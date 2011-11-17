/*
 * Local files system related functions 
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

#include "log.h"
#include <ftw.h>
#include "fs.h"
#include "path.h"
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>
extern int errno;

static mode_t mask;

#ifndef ACCESSPERMS
#define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO)
#endif
#ifndef DEFFILEMODE
#define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif

#define FMODE        ((DEFFILEPERMS)&(~mask))
#define DMODE        ((ACCESSPERMS)&(~mask))


off_t file_size(FILE *fp)
{
  assert(fp);
  
  struct stat sb;
  fstat(fileno(fp), &sb);
  return sb.st_size;

}

char* file_getline(FILE *fp, char *buf, size_t sz)
{
  assert(fp && buf);
  char *rv = fgets(buf, sz, fp);

  if (strlen(buf) > 0 && *(buf + strlen(buf) - 1) == LF)
    *(buf + strlen(buf) - 1) = 0;

  return rv;
}

int file_putline(FILE *fp, const char *buf)
{
  assert(fp && buf);
  
  int num0 = fputs(buf, fp);
  int num1 = fputc(LF, fp);
  
  return num0 + num1;
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

  ptr = full2rela(fname, NULL, 0);
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
