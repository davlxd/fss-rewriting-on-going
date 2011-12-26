/*
 * File list maintenance routines
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


#ifndef _FSS_FLIST_H
#define _FSS_FLIST_H

#include "options.h"
#include "fss.h" // for DEPTH_OF_NFTW


#define STATINFO_FSS        "statinfo.fss"
#define RELAPATH_FSS        "relapath.fss"
#define DIGEST_FSS          "digest.fss"

typedef struct fileinfo finfo;

struct fileinfo {
  uint64_t ucount; // Updated count time

  uint32_t mode;
  uint64_t size;
  uint64_t mtime;
  uint16_t pathlen; // Lenght of relapath (including '\0')
  
  unsigned char *digest;
  char *relapath;

  finfo *orig;  //ptr to original (moved to .trash) finfo
  finfo *chain; //ptr for hashtable chain
};

void init_flist(struct options *o);

finfo* alloc_finfo(uint16_t relapath_len);
void cleanup_finfo(finfo *fi);
uint64_t hashing(finfo *fi);
void load_flist(); // Load flist from *.fss
void update_flist(); // Update flist by traversing monitored directory
void unload_flist(); // Unload(Write) flist to *.fss
void cleanup_flist();
void unload_cleanup_flist();




// Test functions
void print_flist();
void print_flist_hashtb();


#endif
