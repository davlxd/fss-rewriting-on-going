/*
 * Command-line options and config file parse 
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

#include "path.h"
#include "fss.h"
#include "log.h"
#include "exit.h"
#include <dirent.h>
#include <assert.h>
#include <errno.h>
extern int errno;


static const char *basepath; // basepath is client's monitored path
                             // or server's storage path, may end with '/'
void set_basepath(const char *p)
{
  if (!p)
    Log_die(DIE_FAILURE, LOG_ERR, "Monitored Direcotry is NULL");
  
  struct stat sb;
  set0(sb);
  int rv = stat(p, &sb);

  if (rv == 0 && !S_ISDIR(sb.st_mode))
    Log_die(DIE_FAILURE, LOG_ERR, "Monitored directory is a file, exit!");
  else if (rv < 0 && errno == ENOENT)
    Log_die(DIE_FAILURE, LOG_ERR, "Monitored directory dosen't exist, exit!");
  else if (rv < 0)
    Log_die(DIE_FAILURE, LOG_ERR, "stat(%s) failed", p);

  basepath = p;
}

const char *get_tail(const char *path)
{
  assert(path);

  const char *ptr = path + strlen(path);
  assert(*ptr == '\0');

  while (ptr != path && *ptr != *PATH_SEP)
    ptr = ptr - 1;

  if (*ptr == *PATH_SEP)
    ptr = ptr + 1;
  
  return ptr;
}

char *pathncat(char *path0, const char *path1, size_t size)
{
  size_t path0_len = strlen(path0);
  size_t path1_len = strlen(path1);
  char *ptr0 = path0 + path0_len;
  const char *ptr1 = path1;
  
  if (size <= path0_len + path1_len)
    Log_die(DIE_FAILURE, LOG_ERR, "Dst char array is not big enough when copy %s to", path1);

  if (path0_len == 0) {
    if (!strncpy(ptr0, path1, path1_len+1))
      Log_die(DIE_FAILURE, LOG_ERR, "strncpy() failed");
    return path0;
  }

  if (*(ptr0 - 1) == *PATH_SEP && *ptr1 == *PATH_SEP)
    *(--ptr0) = 0;
  else if ( *(ptr0 - 1) != *PATH_SEP && *ptr1 != *PATH_SEP)
    *ptr0++ = *PATH_SEP;

  if (!strncpy(ptr0, path1, path1_len+1))
    Log_die(DIE_FAILURE, LOG_ERR, "strncpy() failed");

  return path0;
}

char *pathncat2(char *path0, const char *path1, const char *path2,
		size_t size)
{
  pathncat(path0, path1, size);
  return pathncat(path0, path2, size);
}


char *full2rela(char *fullname)
{
  if (!strprefix(basepath, fullname))
    Log_die(DIE_FAILURE, LOG_ERR, "basepath %s is not a prefix of fullname %s", basepath, fullname);

  size_t start_pos = strlen(basepath);
					
  while (*(fullname + start_pos) == *PATH_SEP)
    start_pos++;

  return fullname + start_pos;
}


char *rela2full(const char *relaname, char *fullname, size_t size)
{
  if (size <= strlen(relaname) + strlen(basepath)) 
    Log_die(DIE_FAILURE, LOG_ERR, "fullname char array is not big enough to put %s plus %s", basepath, relaname);

  return pathncat2(fullname, basepath, relaname, size);
}

