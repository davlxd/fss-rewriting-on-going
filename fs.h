/*
 * Local file system related functions
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


#ifndef _FSS_FS_H
#define _FSS_FS_H

#include <stdio.h>
#include <sys/stat.h>

#ifndef LF
#define LF '\n'
#endif

//S_IFMT is defined in sys/stat.h
#define SAME_FILE_TYPE(m0, m1) (((m0) & S_IFMT) == ((m1) & S_IFMT))

#define DEPTH_OF_NFTW   10


off_t file_size(FILE *fp);

// The following 2 functions trim/append LF comparing to fgets()/fputs
// But reserve their original return values
char* file_getline(FILE *fp, char* buf, size_t sz);
int file_putline(FILE *fp, const char *buf);

void set_umask(mode_t m);
void mkdir_p(const char *fullname); // reimplement of `mkdir -p /foo`
void rm_rf(const char *fullname); // reimplement of `rm -rf /foo`


#endif
