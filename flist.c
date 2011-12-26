/*
 * File list maintenace routines
 *
 * Copyright (c) 2010, 2011 lxd <i@lxd.me>
 * 
 * This file is part of File Synchronization System(fss).
 *
 * fss is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, 
 * (at your option) any later version.
 *
 * fss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fss.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _XOPEN_SOURCE 500

#include <ftw.h>
#include "fss.h"
#include "flist.h"
#include "path.h"
#include "hashtable.h"
#include "io.h"
#include "log.h"
#include "digest.h"
#include "utils.h" //for OFFSETOF
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

extern int errno;

static uint64_t g_ucount;
static hashtable *hashtb;
static const char *basepath;

static bool force_use_digest;
static bool follow_symbolic;
static bool include_hidden;

static char fssdir_path[MAX_PATH_LEN];
static char statinfo_path[MAX_PATH_LEN];
static char relapath_path[MAX_PATH_LEN];
static char digest_path[MAX_PATH_LEN];

static FILE *statinfo_fp;
static FILE *relapath_fp;
static FILE *digest_fp;


static void set_path(struct options *o)
{
  basepath = o->path;
  set0(fssdir_path);
  set0(statinfo_path);
  set0(relapath_path);
  set0(digest_path);

  pathncat2(fssdir_path, basepath, o->fss_dir, MAX_PATH_LEN);
  pathncat2(statinfo_path, fssdir_path, STATINFO_FSS, MAX_PATH_LEN);
  pathncat2(relapath_path, fssdir_path, RELAPATH_FSS, MAX_PATH_LEN);
  pathncat2(digest_path, fssdir_path, DIGEST_FSS, MAX_PATH_LEN);
}

void init_flist(struct options *o)
{
  g_ucount = 0;
  hashtb = NULL;

  set_path(o);
  force_use_digest = o->force_use_digest;
  follow_symbolic = o->follow_symbolic;
  include_hidden = o->include_hidden;
  mkdir_p(fssdir_path); // mkdir(/foo/.../.fss) if dosen't exist
  
  Log(LOG_INFO, "flist initiated.\n\tfssdir_path: %s\n\tstatinfo_path: %s\n\trelapath_path: %s\n\tdigest_path: %s", fssdir_path, statinfo_path, relapath_path, digest_path);
}

// Attention: statinfo_fp and digest_fp are all NULL if target files
// doesn't exist
static void open_fp(const char *type)
{
  statinfo_fp = fopen(statinfo_path, type);
  relapath_fp = fopen(relapath_path, type);
  digest_fp = fopen(digest_path, type);


  if (!(statinfo_fp && relapath_fp && digest_fp) && errno != ENOENT)
    Log_die(DIE_FAILURE, LOG_ERR, "fopen() %s or %s or %s failed",
	    statinfo_path, relapath_path, digest_path);
}

static void do_flush_fp(FILE *fp)
{
  assert(fp);
  if (fflush(fp) == EOF)
    Log_die(DIE_FAILURE, LOG_ERR, "fflush() failed");
}

static void flush_fp()
{
  do_flush_fp(statinfo_fp);
  do_flush_fp(relapath_fp);
  do_flush_fp(digest_fp);
}

static void do_close_fp(FILE **fp)
{
  if (*fp && (fclose(*fp) != EOF))
    *fp = NULL;
  
  if (*fp)
    Log_die(DIE_FAILURE, LOG_ERR, "fclose() failed");
}
  
static void close_fp()
{
  do_close_fp(&statinfo_fp);
  do_close_fp(&relapath_fp);
  do_close_fp(&digest_fp);
}

finfo* alloc_finfo(uint16_t relapath_len)
{
  void* ptr = calloc(1, sizeof(finfo) + DIGEST_BYTES + relapath_len);
  finfo* fi = (finfo*)ptr;
  
  fi->digest = (unsigned char*)(ptr + sizeof(finfo));
  fi->relapath = (char*)(ptr + sizeof(finfo) + DIGEST_BYTES);

  fi->pathlen = relapath_len;
  return fi;
}

void cleanup_finfo(finfo *fi)
{
  free(fi);
}

uint64_t hashing(finfo* fi)
{
  assert(fi && fi->relapath);
  return digest2hashkey(str_digest(fi->relapath), (hashtb->size - 1));
}

static void remove_finfo(void *fi)
{
  cleanup_finfo(hashtable_remove(hashtb, hashing((finfo*)fi), fi));
}

static bool finfo_valid(finfo *fi)
{
  return fi->ucount == g_ucount ? true : false;
}


static bool compar_file_digest(const char *f0, const char *f1)
{
  unsigned char digest0[DIGEST_BYTES]; set0(digest0);
  unsigned char digest1[DIGEST_BYTES]; set0(digest1);

  file_digest_name(f0, digest0);
  file_digest_name(f1, digest1);

  return !memcmp(digest0, digest1, DIGEST_BYTES);
}


static bool hit(void *_preq, void *_fi)
{
  finfo *preq = (finfo*)_preq;
  finfo *fi = (finfo*)_fi;

  // Check if same file type(FILE|DIR)
  if (!SAME_FILE_TYPE(preq->mode, fi->mode))
    return false;

  // Check if same relative path string
  if (!streq(preq->relapath, fi->relapath))
    return false;

  // If --force-use-digest is set, check if same digest string
  if (S_ISREG(fi->mode) && force_use_digest) {
    char full_preq[MAX_PATH_LEN]; set0(full_preq);
    char full_fi[MAX_PATH_LEN]; set0(full_fi);
    
    rela2full(preq->relapath, full_preq, MAX_PATH_LEN);
    rela2full(fi->relapath, full_fi, MAX_PATH_LEN);

    if (!compar_file_digest(full_preq, full_fi))
      return false;

    // If --force-use-digest is not set, check if last modifed time is same
    // and last modified time should not be applied to directories
  } else if (S_ISREG(fi->mode) && preq->mtime != fi->mtime)
    return false;

  return true;
}

static void traverse_flist(void (*fn)(void* ))
{
  Log(LOG_INFO, "Traversing flist ...");
  traverse_hashtable(hashtb, fn);
}


// The following functions and do_unload_flist() define format of statinfo.fss:
//
//   u32       u64       u64         u16
// [mode][\t][size][\t][mtime][\t][pathlen]
//
static finfo* str2finfo(char* _statinfo, char* _relapath, char* _digest)
{
  assert(_statinfo && _relapath && _digest);
  
  uint16_t _pathlen = strlen(_relapath) + 1;
  finfo* fi = alloc_finfo(_pathlen);

  sscanf(_statinfo,
	 "%" SCNu32 "\t" //fi->mode
	 "%" SCNu64 "\t" //fi->size
	 "%" SCNu64 "\t" //fi->mtime
	 "%" SCNu16      //fi->pathlen
	 "\n", 
	 &fi->mode,
	 &fi->size,
	 &fi->mtime,
	 &fi->pathlen);
  
  assert(_pathlen == fi->pathlen);
  memcpy(fi->digest, (unsigned char*)hex2digest(_digest), DIGEST_BYTES);
  strncpy(fi->relapath, _relapath, _pathlen); 

  return fi;
}

static void do_load_flist()
{
  assert((statinfo_fp && relapath_fp && digest_fp) ||
	 (!statinfo_fp && !relapath_fp && !digest_fp));

  finfo* fi;
  time_t now = time(NULL);
  g_ucount = now;

  if (!statinfo_fp)
    return ;

  char statinfo_line[MAX_PATH_LEN]; // Just borrow MAX_PATH_LEN
  char relapath_line[MAX_PATH_LEN];
  char digest_line[DIGEST_STR_LEN + 1]; // read LF in digest_line then trimmed by file_getline
  set0(statinfo_line);
  set0(relapath_line);
  set0(digest_line);


  while(file_getline(statinfo_fp, statinfo_line, MAX_PATH_LEN) &&
	file_getline(relapath_fp, relapath_line, MAX_PATH_LEN) &&
	file_getline(digest_fp, digest_line, DIGEST_STR_LEN + 1) ) {
    Log(LOG_INFO, "Loading finfo:\n\t\"%s\"\n\t\"%s\"\n\t\"%s\"", relapath_line, digest_line, statinfo_line);
    
    fi = str2finfo(statinfo_line, relapath_line, digest_line);
    fi->ucount = now;
    hashtable_insert(hashtb, hashing(fi), fi);
  }
}

// Load flist from statinfo.fss/digest.fss
void load_flist()
{
  Log(LOG_INFO, "Loading flist from *.fss ...");
  off_t line_num = 0;
  open_fp("rb");

  // Calculate line number for digest.fss which is FILE|DIR number stored
  // in *.fss
  if (!digest_fp)
    line_num = 0;
  else {
    off_t filesize = file_size(digest_fp);
    assert(filesize % DIGEST_STR_LEN == 0);
    line_num = filesize/DIGEST_STR_LEN;
  }

  hashtb = init_hashtable(power_of_2_ceiling(line_num),
			  OFFSETOF(finfo, chain));
  Log(LOG_INFO, "HASH TABLE SIZE = %" PRIu32, hashtb->size);

  do_load_flist();
  close_fp();
}


// Call-back functio for update_flist
static int do_update_flist(const char *fullname,
			   const struct stat *sb,
			   int typeflag,
			   struct FTW *ftwbuf)
{
  finfo* fi;

  Log(LOG_INFO, "Updating flist: checking on \"%s\" ...", fullname);
  // -----------------------filters---------------------------
  
  // Skip monitored directory itself
  if (strncmp(fullname, basepath, strlen(fullname)) == 0)
    return 0;
    
  // Skip *.fss files under /foo/.fss directory
  if (strprefix(fssdir_path, fullname))
    return 0;
  
  // Skip hidden files and directories if --include_hidden is set
  if (!include_hidden && IS_HIDDEN(fullname))
    return 0;
    
  // Skip block/char special files, FIFO, socket symbolic files
  // nftw() says he won't report symbolic files if follow them
  if (!(S_ISDIR(sb->st_mode) || S_ISREG(sb->st_mode)))
    return 0;

  //---------------------End--of--filters----------------------

  finfo preq;
  preq.mode = sb->st_mode;
  preq.size = sb->st_size;
  preq.mtime = sb->st_mtime;
  preq.relapath = full2rela(fullname); // Attention! const violated
  preq.pathlen = strlen(preq.relapath);
  
  if ((fi = hashtable_search(hashtb, hashing(&preq), &preq, hit))) {
    Log(LOG_INFO, "\t\t\"%s\" searchable, update its ucount", fullname);
    fi->ucount = g_ucount;

    if (S_ISDIR(sb->st_mode)) // Directories' mtime could be altered, update it
      fi->mtime = sb->st_mtime;
  }
  
  // If not, Append a new one to flist, But:
  // TODO: if force-use-digest is set, fullname's digest will be calculated
  // twice
  else {
    Log(LOG_INFO, "Updating flist: \"%s\" is new, create a new finfo");
    const char* rela_ptr = full2rela(fullname);
    size_t rela_len = strlen(rela_ptr) + 1;
    finfo* fi = alloc_finfo(rela_len);

    fi->ucount = g_ucount;
    fi->mode = sb->st_mode;
    fi->size = sb->st_size;
    fi->mtime = sb->st_mtime;
    fi->pathlen = rela_len;

    file_digest_name(fullname, fi->digest);
    strncpy(fi->relapath, rela_ptr, rela_len);
    
    hashtable_insert(hashtb, hashing(fi), fi);
  }
      
  return 0;
}


void update_flist()
{
  Log(LOG_INFO, "UPDATING flist.......");
  int nftw_flags = 0;
  g_ucount += 1;

  if (!follow_symbolic)
    nftw_flags |= FTW_PHYS;
    
  if (nftw(basepath, do_update_flist, DEPTH_OF_NFTW, nftw_flags) != 0)
    Log_die(DIE_FAILURE, LOG_ERR, "nftw failed");
  
}


static void do_unload_flist(void *_fi)
{
  finfo* fi = (finfo*)_fi;
  if (!finfo_valid(fi)) {
    remove_finfo(_fi);
    return ;
  }
  
  fprintf(statinfo_fp,
	  "%" PRIu32 "\t"
	  "%" PRIu64 "\t"
	  "%" PRIu64 "\t"
	  "%" PRIu16
	  "\n",
	  fi->mode,
	  fi->size,
	  fi->mtime,
	  fi->pathlen );
  
  file_putline(relapath_fp, fi->relapath);
  file_putline(digest_fp, digest2hex(fi->digest));

  Log(LOG_INFO, "Unloading finfo \"%s\" ...", fi->relapath);
}

 

void unload_flist()
{
  close_fp();
  open_fp("wb");

  traverse_flist(do_unload_flist);

  flush_fp();
  close_fp();
}


void cleanup_flist()
{
  traverse_flist(remove_finfo);
  free_hashtable(hashtb);
  hashtb = NULL;
}


static void do_unload_free_flist(void* fi)
{
  if (finfo_valid((finfo*)fi))
    do_unload_flist(fi);
  remove_finfo(fi);
}
   

void unload_cleanup_flist()
{
  close_fp();
  open_fp("wb");
  
  traverse_flist(do_unload_free_flist);

  free_hashtable(hashtb);
  hashtb = NULL;
  
  flush_fp();
  close_fp();
}


//--------------------------------------------------------------------
//                          |
//                          | test functions
//                          v

static void print_finfo(void* _fi)
{
  finfo *fi = (finfo*)_fi;
  if (!finfo_valid(fi)) {
    remove_finfo(_fi);
    return ;
  }
  
  if (fi == NULL)  {// NULL
    Log(LOG_INFO, "K%" PRIu32 " NULL", hashing(fi));
    return ;
  }

  const char *digesthex = digest2hex(fi->digest);
  
  Log(LOG_INFO, "\tK%" PRIu64
      "%p (chain->%p): "
      "ucount=%" PRIu64 " " 
      "mode=0%06" PRIo32 " "
      "size=%" PRIu64 " "
      "mtime=%" PRIu64 " "
      "pathlen=%" PRIu16 " "
      "digest=\"%c%c%c%c...%c%c%c%c\" "
      "relapath=\"%s\""
      "\n"
      ,
      hashing(fi),
      fi, fi->chain, 
      fi->ucount,
      fi->mode,
      fi->size,
      fi->mtime,
      fi->pathlen,
      *(digesthex), *(digesthex+1), *(digesthex+2), *(digesthex+3),
      *(digesthex+36), *(digesthex+37), *(digesthex+38), *(digesthex+39),
      fi->relapath
      );

}
void print_flist()
{
  Log(LOG_INFO, "\nPrint flist (g_ucount = %" PRIu64 "):\n", g_ucount);
  traverse_flist(print_finfo);
  fflush(stdout);
}
