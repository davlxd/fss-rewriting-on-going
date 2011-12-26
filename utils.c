/*
 * Utility routines
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

#include "utils.h"
#include "log.h"
#include <ftw.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
extern int errno;

static mode_t mask;

bool strpostfix(const char *post, const char *str)
{
  assert(post && str);
  
  size_t i;
  const char *post_ptr = post + strlen(post);
  const char *str_ptr = str + strlen(str);

  for (i = 1; i <= min(strlen(post), strlen(str)); i++)
    if (*(post_ptr - i) != *(str_ptr - i))
      return false;

  if (i < strlen(post))
    return false;
  
  return true;
  
}

bool strprefix(const char *pre, const char *str)
{
  assert(pre && str);
  return !strncmp(pre, str, strlen(pre));
}

mode_t get_proc_umask()
{
  mode_t mask = umask(0);
  umask(mask);
  return mask;
}


uint16_t str2uint16(const char *str)
{
  uint16_t u16 = 0;
  
  if (sscanf(str, "%" SCNu16, &u16) == EOF)
    Log_die(DIE_FAILURE, LOG_ERR, "sscanf(%s) to u16t failed", str);
  return u16;
}

uint32_t str2uint32(const char *str)
{
  uint32_t u32 = 0;

  if (sscanf(str, "%" SCNu32, &u32) == EOF)
    Log_die(DIE_FAILURE, LOG_ERR, "sscanf(%s) to u32t failed", str);
  return u32;
}

uint64_t str2uint64(const char *str)
{
  uint64_t u64 = 0;

  if (sscanf(str, "%" SCNu64, &u64) == EOF)
    Log_die(DIE_FAILURE, LOG_ERR, "sscanf(%s) to u64t failed", str);
  return u64;
}

int uint642str(uint64_t u64, char *buf, size_t sz)
{
  int rv = 0;
  
  if ((rv = snprintf(buf, sz, "%" PRIu64, u64)) < 0)
    Log_die(DIE_FAILURE, LOG_ERR, "snprintf() failed");
  
  return rv;
}

mode_t str2modet(const char *str)
{
  return (mode_t)str2uint32(str);
}

int64_t str2int64(const char *str)
{
  int64_t i64 = 0;
  
  if (sscanf(str, "%" SCNd64, &i64) == EOF)
    Log_die(DIE_FAILURE, LOG_ERR, "sscanf() failed");

  return i64;
}

const char *uint162str(uint16_t u16)
{
  static char str[6];
  set0(str);

  if (snprintf(str, 6, "%" PRIu16, u16) < 0)
    Log_die(DIE_FAILURE, LOG_ERR, "snprintf() failed");

  return str;
}
  
bool verify_dir(const char *p)
{
  struct stat sb;
  if (stat(p, &sb) < 0 || !S_ISDIR(sb.st_mode))
    return false;
  return true;
}

bool verify_addr(const char *addr)
{
  return false;
}

void set_umask(mode_t m)
{
  if (m >> 9)
    Log_die(DIE_FAILURE, LOG_ERR, "umask value is invalid, exit!");
  
  mask = m;
}


static void do_mkdir(const char *path)
{
  struct stat sb;
  int rv = stat(path, &sb);
 
  if (rv < 0 && errno == ENOENT)
    if (mkdir(path, DMODE) < 0)
      Log_die(DIE_FAILURE, LOG_ERR, "mkdir() %s failed", path);
  
  if (rv < 0 && errno != ENOENT)
    Log_die(DIE_FAILURE, LOG_ERR, "stat(%s) failed", path);
  
  if (S_ISDIR(sb.st_mode))
    return;

  if (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode)) {
    Log(LOG_WARNING,
	"%s exists as a regular/symbolic-link file, remove it", path);
    if (remove(path) < 0)
      Log_die(DIE_FAILURE, LOG_ERR, "remove() %s failed", path);
    
  } else if (S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode) ||
	     S_ISFIFO(sb.st_mode) || S_ISSOCK(sb.st_mode)) {
    Log_die(DIE_SUCCESS, LOG_ERR,
	    "%s exists as a special/FIFO/socket file, cannot be removed, exit", path);
  } 
}

// mkdir() recursively
void mkdir_p(const char *fullname)
{ 
  char *ptr = NULL;
  size_t len = strlen(fullname) + 1;
  char fname[len];
  
  strncpy(fname, fullname, len);

  ptr = full2rela(fname);
  while((ptr = strchr(ptr, '/'))) {
    *ptr = 0;
    do_mkdir(fname);
    *ptr++ = '/';
    while (*ptr == '/')
      ptr++;
  }
  do_mkdir(fname);
  
}

static int do_rm(const char *fullname, const struct stat *sb,
		 int typeflag, struct FTW *ftwbuf)
{
  int rv;
  if ((rv = remove(fullname)) < 0)
    Log_die(DIE_FAILURE, LOG_ERR, "remove() %s failed during nftw", fullname);
  return rv;
}

void rm_rf(const char *fullname)
{
  int nftw_flags = 0;
  nftw_flags |= FTW_DEPTH; //FTW_DEPTH is crucial here, make sure post-order
  nftw_flags |= FTW_PHYS; // Not follow symbolic links
  
  if (nftw(fullname, do_rm, DEPTH_OF_NFTW, nftw_flags) != 0)
    Log_die(DIE_FAILURE, LOG_ERR, "nftw() failed");
  
}
