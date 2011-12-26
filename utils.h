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

#ifndef _FSS_UTILS_H_
#define _FSS_UTILS_H_

#include <sys/stat.h>
#include <inttypes.h>
#include <string.h>

#ifndef LF
#define LF '\n'
#endif
#ifndef CR
#define CR '\r'
#endif
#ifndef CRLF
#define CRLF "\r\n"
#endif

#ifndef ACCESSPERMS
#define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO)
#endif
#ifndef DEFFILEMODE
#define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#endif

#define FMODE        ((DEFFILEPERMS)&(~mask))
#define DMODE        ((ACCESSPERMS)&(~mask))

// get offset of given field within struct
#define OFFSETOF(type, field) ((size_t)(&(((type*)0)->field)))
#define ELEM_NUM(array) (sizeof(array)/sizeof((array)[0]))

//S_IFMT is defined in <sys/stat.h>
#define SAME_FILE_TYPE(m0, m1) (((m0) & S_IFMT) == ((m1) & S_IFMT))

#define DEPTH_OF_NFTW   10

#define streq(a, b) (!strcmp((a), (b)))
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define set0(x) (memset(&(x), 0, sizeof((x))))

#define str2port(s) str2uint16((s))
#define port2str(p) uint162str((p))

typedef enum { false = 0, true = 1 } bool;

bool strpostfix(const char *post, const char *str);
bool strprefix(const char *pre, const char *str);
mode_t get_proc_umask();

uint32_t str2uint32(const char *str);
uint16_t str2uint16(const char *str);
const char *uint162str(uint16_t u16);
mode_t str2modet(const char *str);

bool verify_dir(const char *path);
bool verify_addr(const char *addr);

void set_umask(mode_t m);
void mkdir_p(const char *fullname); // simple reimplement of `mkdir -p /foo`
void rm_rf(const char *fullname); // simple reimplement of `rm -rf /foo`
#endif
