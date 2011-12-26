/*
 * Read messages from socket file descriptor, different tagged incoming
 * messages will be handled in different functions via a function table.
 * This is where the main business logic lies.
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


#ifndef _FSS_CORE_H
#define _FSS_CORE_H

#include <inttypes.h>

#define FINFO ((uint64_t)1 << 0)
#define REQ_FLIST ((uint64_t)1 << 1)
#define REQ_BINFO ((uint64_t)1 << 2)

typedef struct {
  uint64_t tag;
  void (*func)(int fd);
  
}tagtable;

void tag_switch(uint64_t tag, int fd);



#endif
