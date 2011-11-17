/*
 * Build hashtable for flist and blist
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


#ifndef _FSS_HASHTABLE_H
#define _FSS_HASHTABLE_H

#include <inttypes.h>

#define MIN_HASHTABLE_SIZE ((uint32_t)1 << 8)
#define MAX_HASHTABLE_SIZE ((uint32_t)1 << 16)

typedef struct {
  uint32_t size;
  void **bucket;
  
} hashtable;

// raw_size is file/directories number come from "statinfo.fss"
hashtable* init_hashtable(uint64_t raw_size);
void free_hashtable(hashtable *htb);




#endif
