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

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define set0(x) (memset(&(x), 0, sizeof((x))))

#define str2port(s) str2uint16(s)

typedef enum { false = 0, true = 1 } bool;

bool streq(const char *s0, const char *s1);
bool strprefix(const char *pre, const char *str);

bool verify_dir(const char *path);
bool verify_addr(const char *addr);


uint32_t str2uint32(const char *str);
uint16_t str2uint16(const char *str);
mode_t str2modet(const char *str);

mode_t get_proc_umask();

#endif
