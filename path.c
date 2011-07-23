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
#include <string.h>

extern struct options o;

int pathncat(const char *path1, char *path0, size_t size)
{
  size_t path0_len = strlen(path0);
  size_t path1_len = strlen(path1);
  char *ptr0 = path0 + path0_len;
  const char *ptr1 = path1;
  
  if (size <= path0_len + path1_len) {
    fprintf(stderr, "@pathncat: dst char array is not big enough\n");
    return 1;
  }

  if (*(ptr0 - 1) == '/' && *ptr1 == '/')
    *(--ptr0) = 0;
  else if ( *(ptr0 - 1) != '/' && *ptr1 != '/')
    *ptr0++ = '/';

  if (!strncpy(ptr0, path1, path1_len+1)) {
    perror("@pathncat(): strncpy() failed");
    return 1;
  }

  return 0;
}


int full2rela(const char *fullname, char *relaname, size_t size)
{
  size_t monitored_path_len = strlen(o.monitored_path);
  if (size <= strlen(fullname) - monitored_path_len) {
    fprintf(stderr, "@full2rela(): relaname is not big enough\n");
    return 1;
  }
   
  size_t start_pos = monitored_path_len;
  if (*(fullname + monitored_path_len) == '/')
    start_pos++;

  if (!strncpy(relaname, fullname + start_pos, size)) {
    perror("@full2rela(): strncpy() failed");
    return 1;
  }

  return 0;
  
}

void rela2full(const char *relaname, char *fullname, size_t size)
{
  if (size <= strlen(relaname) + strlen(o.monitored_path)) {
    fprintf(stderr, "@rela2full(): fullname is not big enought\n");
    return 1;
  }
  
  if (!strncpy(fullname, o.monitored_path, strlen(o.monitored_path)+1)) {
    perror("@rela2full(): strncpy() failed");
    return 1;
  }
  
  return pathncat(relaname, fullname, size);
  
}

