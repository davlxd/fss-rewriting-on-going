/*
 * File I/O wrappers
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


#ifndef _FSS_IO_H
#define _FSS_IO_H

#include <inttypes.h>
#include <stdio.h>
#include <sys/stat.h>
#include "flist.h"


off_t file_size(FILE *fp);

/* the following 2 functions trim/append LF comparing to fgets()/fputs()
 * but reserve their original return values
 * for file_getline(), `buf' has to big enough to hold LF
 */
char* file_getline(FILE *fp, char* buf, size_t sz);
int file_putline(FILE *fp, const char *buf);


void set_nonblock(int fd);

// If `timeo' != 0, `fss_write()' set a timeout on syscall `write()'
// return 1 if than happens
int fss_write(int fd, const void *buf, uint64_t len, uint32_t timeo);
int fss_read(int fd, void *buf, uint64_t len, uint32_t timeo);
int fss_redirect(int fd0, int fd1, uint64_t sz, uint32_t t0, uint32_t t1);

int write_uint64(int fd, uint64_t u64, uint32_t timeo);
int read_uint64(int fd, uint64_t *u64, uint32_t timeo);
int write_uint32(int fd, uint32_t u32, uint32_t timeo);
int read_uint32(int fd, uint32_t *u32, uint32_t timeo);
int write_uint16(int fd, uint16_t u16, uint32_t timeo);
int read_uint16(int fd, uint16_t *u16, uint32_t timeo);

#define write_tag(a, b, c) write_uint64(a, b, c)
uint64_t read_tag(int fd, uint32_t timeo);


#define write_bytes(fd, buf, sz, t) fss_write(fd, buf, sz, t)
#define read_bytes(fd, buf, sz, t) fss_read(fd, buf, sz, t)

#define write_buf(fd, buf, sz, t) fss_write(fd, buf, sz, t)
#define read_buf(fd, buf, sz, t) fss_read(fd, buf, sz, t)

int write_finfo(int fd, const finfo *fi, uint32_t timeo);
finfo* read_finfo(int fd, uint32_t timeo);


  




#endif
