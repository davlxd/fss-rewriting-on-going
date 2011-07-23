/*
 * Path string operate routines
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

#ifndef _FSS_PATH_H_
#define _FSS_PATH_H_

#include <stdio.h>

// append str1 to str0, size refers to total size of str0
int pathncat(const char *path1, char *path0, size_t size);

int full2rela(const char *fullname, char *relaname, size_t size);
int rela2full(const char *relaname, char *fullname, size_t size);


#endif

  
    
