/*
 * Block list maintenace routines
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


#ifndef _FSS_BLIST_H
#define _FSS_BLIST_H

#include "flist.h"
#include "utils.h"

typedef struct blockinfo binfo;

struct blockinfo {
  uint64_t offset;
  uint64_t size;

  unsigned char *digest;
  finfo *fi;
};

int binfo_of_fd(int fd, finfo *fi, void (*fn)(binfo*), uint64_t blksz,
		bool overlap);
void cleanup_binfo(binfo *bi);



#endif
