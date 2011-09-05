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


static const char *basepath; // basepath is client's monitored path
                             // or server's storage path

void set_basepath(const char *p)
{
  basepath = p;
}


char *pathncat(char *path0, const char *path1, size_t size)
{
  size_t path0_len = strlen(path0);
  size_t path1_len = strlen(path1);
  char *ptr0 = path0 + path0_len;
  const char *ptr1 = path1;
  
  if (size <= path0_len + path1_len)
    Log_die(DIE_FAILURE, LOG_ERR, "Dst char array is not big enough when copy %s to", path1);


  if (*(ptr0 - 1) == '/' && *ptr1 == '/')
    *(--ptr0) = 0;
  else if ( *(ptr0 - 1) != '/' && *ptr1 != '/')
    *ptr0++ = '/';

  if (!strncpy(ptr0, path1, path1_len+1))
    Log_die(DIE_FAILURE, LOG_ERR, "strncpy() %s to %s failed", path1, path0);

  return path0;
}


char *full2rela(const char *fullname, char *relaname, size_t size)
{
  size_t monitored_path_len = strlen(basepath);
  if (size <= strlen(fullname) - monitored_path_len)
    Log_die(DIE_FAILURE, LOG_ERR, "relaname char array is not big enough to put %s minus %s", fullname, basepath);

  size_t start_pos = monitored_path_len;

  if (!strneq(basepath, fullname))
    Log_die(DIE_FAILURE, LOG_ERR, "basepath %s is not a prefix of fullname %s", basepath, fullname);
  
  if (*(fullname + monitored_path_len) == '/')
    start_pos++;

  if (!strncpy(relaname, fullname + start_pos, size))
    Log_die(DIE_FAILURE, LOG_ERR, "strncpy() failed when copy %s to %s", fullname+start_pos, relaname);

  return relaname;
  
}

char *rela2full(const char *relaname, char *fullname, size_t size)
{
  if (size <= strlen(relaname) + strlen(basepath)) 
    Log_die(DIE_FAILURE, LOG_ERR, "fullname char array is not big enough to put %s plus %s", basepath, relaname);
  
  if (!strncpy(fullname, basepath, strlen(basepath)+1))
    Log_die(DIE_FAILURE, LOG_ERR, "strncpy() failed when copy %s to %s", basepath, fullname);
  
  return pathncat(fullname, relaname, size);
  
}

