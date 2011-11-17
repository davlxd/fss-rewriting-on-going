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

#include "fss.h"
#include "flist.h"
#include "path.h"
#include "hashtable.h"
#include "fs.h"
#include "log.h"
#include "digest.h"
#include <time.h>
#include <ftw.h>
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

static void print_finfo(finfo** head, finfo *fi); // Test function


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



static bool finfo_valid(finfo *fi)
{
  return fi->ucount == g_ucount ? true : false;
}

static uint32_t get_hash_key(const char *relapath)
{
  uint32_t keyspan = 0;
  char digest[DIGEST_STR_LEN]; set0(digest);
  char *ptr = digest;

  str_digest(relapath, digest, DIGEST_STR_LEN);
  
  int offset = DIGEST_STR_LEN > 9 ? (DIGEST_STR_LEN - 9) : 0;// 9 = (32/4 + 1)
  
  ptr += offset;
  sscanf(ptr, "%" SCNx32, &keyspan);
  
  return (keyspan & (hashtb->size - 1));
}

static finfo* alloc_finfo(uint16_t relapath_len)
{
  void* ptr = calloc(1, sizeof(finfo) + DIGEST_STR_LEN + relapath_len);
  finfo* fi = (finfo*)ptr;
  
  fi->digest = (char*)(ptr + sizeof(finfo));
  fi->relapath = (char*)(ptr + sizeof(finfo) + DIGEST_STR_LEN);

  return fi;
}


static void hashtable_insert(finfo* fi)
{
  uint32_t key = get_hash_key(fi->relapath);
  finfo *head = (finfo*)(*(hashtb->bucket + key));
  
  Log(LOG_INFO, "Inserting \"%s\" to hashtable, key = %"PRIu32, fi->relapath, key);
  //INFO, DEBUG
  print_finfo(NULL, fi);

  fi->chain = head;
  *(hashtb->bucket + key) = fi;
}

static bool compa_file_digest(const char *f0, const char *f1)
{
  char digest0[DIGEST_STR_LEN]; set0(digest0);
  char digest1[DIGEST_STR_LEN]; set0(digest1);

  file_digest_name(f0, digest0, DIGEST_STR_LEN);
  file_digest_name(f1, digest1, DIGEST_STR_LEN);

  return streq(digest0, digest1);
}

static bool hit(finfo *fi, const char *fullname, const struct stat *sb)
{
  const char *relapath = full2rela(fullname, NULL, 0);

  // Check if same file type(FILE|DIR)
  if (!SAME_FILE_TYPE(fi->mode, sb->st_mode))
    return false;

  // Check if same relative path string
  if (!streq(relapath, fi->relapath))
    return false;

  // If --force-use-digest is set, check if same digest string
  if (S_ISREG(fi->mode) && force_use_digest) {
    char full[MAX_PATH_LEN]; set0(full);
    rela2full(fi->relapath, full, MAX_PATH_LEN);

    if (!compa_file_digest(full, fullname))
      return false;

    // If --force-use-digest is not set, check if last modifed time is same
  } else if (fi->mtime != sb->st_mtime)
    return false;

  return true;
}

static finfo* hashtable_search(const char *fullname, const struct stat *sb)
{
  const char *relapath = full2rela(fullname, NULL, 0);
  uint32_t key = get_hash_key(relapath);
  finfo **head = (finfo **) (hashtb->bucket + key);
  finfo *fi = *head;

  while (fi && !hit(fi, fullname, sb))
    fi = fi->chain;
  
  return fi;
}

static void hashtable_remove(finfo **head, finfo *target)
{
  assert(*head && target);

  finfo *prev = NULL;
  finfo **cur = head;
  finfo *next = (*cur)->chain;

  while (next && *cur != target) { 
    prev = *cur;
    *cur = next;
    next = next->chain;
  }
  assert(*cur == target);

  free(*cur);
  
  if (prev)
    prev->chain = next;
  else
    *cur = next;
}

static void traverse_flist(void (*fn)(finfo**, finfo* ))
{
  Log(LOG_INFO, "Traversing flist ...");
  uint32_t i = 1;
  //next refers to the next finfo after cur in bucket linked list
  finfo *cur, *next;
  cur = next = NULL;

  for (i = 0; i < hashtb->size; i++)
    if (*(hashtb->bucket + i)) {
      Log(LOG_INFO, "Index = %"PRIu32, i);
      cur = (hashtb->bucket)[i];
      while (cur) {
	next = cur->chain;
      
	if (!finfo_valid(cur))
	  hashtable_remove((finfo**)(hashtb->bucket + i), cur);
	else
	  fn((finfo**)(hashtb->bucket + i), cur);
      
	cur = next;
      }//end while(...
    }//end if 
  
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
  strncpy(fi->digest, _digest, DIGEST_STR_LEN);
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
  char digest_line[DIGEST_STR_LEN + 1]; // Have to handle LF
  set0(statinfo_line);
  set0(relapath_line);
  set0(digest_line);


  while(file_getline(statinfo_fp, statinfo_line, MAX_PATH_LEN) &&
	file_getline(relapath_fp, relapath_line, MAX_PATH_LEN) &&
	file_getline(digest_fp, digest_line, DIGEST_STR_LEN + 1) ) {
    Log(LOG_INFO, "Loading finfo:\n\t\"%s\"\n\t\"%s\"\n\t\"%s\"", relapath_line, digest_line, statinfo_line);
    
    fi = str2finfo(statinfo_line, relapath_line, digest_line);
    fi->ucount = now;
    hashtable_insert(fi);
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
    
  hashtb = init_hashtable(line_num);
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
  if (streq(fullname, basepath))
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

  
  // If this fullname-file is searchable, which means he's already
  // in flist, so I just update its ucount
  if ((fi = hashtable_search(fullname, sb))) {
    Log(LOG_INFO, "\t\t\"%s\" searchable, update its ucount", fullname);
    fi->ucount = g_ucount;
  }
  
  // If not, Append a new one to flist, But:
  // TODO: if force-use-digest is set, fullname's digest will be calculated
  // twice
  else {
    Log(LOG_INFO, "Updating flist: \"%s\" is new, create a new finfo");
    const char* rela_ptr = full2rela(fullname, NULL, 0);
    size_t rela_len = strlen(rela_ptr) + 1;
    finfo* fi = alloc_finfo(rela_len);

    fi->ucount = g_ucount;
    fi->mode = sb->st_mode;
    fi->size = sb->st_size;
    fi->mtime = sb->st_mtime;
    fi->pathlen = rela_len;

    file_digest_name(fullname, fi->digest, DIGEST_STR_LEN);
    strncpy(fi->relapath, rela_ptr, rela_len);
    
    hashtable_insert(fi);
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


static void do_unload_flist(finfo **head, finfo *fi)
{
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
  file_putline(digest_fp, fi->digest);

  Log(LOG_INFO, "Unloading finfo \"%s\" ...", fi->relapath);
  //DEBUG, INFO
  print_finfo(head, fi);
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
  traverse_flist(hashtable_remove);
  free_hashtable(hashtb);
  hashtb = NULL;
}


static void do_unload_free_flist(finfo **head, finfo* fi)
{
  do_unload_flist(head, fi);
  hashtable_remove(head, fi);
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

static void print_finfo(finfo **head, finfo* fi)
{
  if (!fi)  {// NULL
    Log(LOG_INFO, "NULL\n");
    return ;
  }
  Log(LOG_INFO, "\t%p (chain->%p): "
	 "ucount=%" PRIu64 " " 
	 "mode=0%06" PRIo32 " "
	 "size=%" PRIu64 " "
	 "mtime=%" PRIu64 " "
	 "pathlen=%" PRIu16 " "
	 "digest=\"%c%c%c%c...%c%c%c%c\" "
	 "relapath=\"%s\""
	 "\n"
	 ,
	 fi, fi->chain, 
	 fi->ucount,
	 fi->mode,
	 fi->size,
	 fi->mtime,
	 fi->pathlen,
	 *(fi->digest), *(fi->digest+1), *(fi->digest+2), *(fi->digest+3),
	 *(fi->digest+36), *(fi->digest+37), *(fi->digest+38), *(fi->digest+39),
	 fi->relapath
      );

}
void print_flist()
{
  Log(LOG_INFO, "\nPrint flist (g_ucount = %" PRIu64 "):\n", g_ucount);
  traverse_flist(print_finfo);
  fflush(stdout);
}
