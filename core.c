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

#include "core.h"
#include "utils.h"
#include "log.h"


static void FINFO_hander(int fd)
{
  Log(LOG_DEBUG, "In FINFO_handler");
}

static void REQ_FLIST_handler(int fd)
{
}

static void REQ_BINFO_handler(int fd)
{
}



static const tagtable tagtbl[] = {
  {FINFO, FINFO_hander},
  {REQ_FLIST, REQ_FLIST_handler},
  {REQ_BINFO, REQ_BINFO_handler}
};

void tag_switch(uint64_t tag, int fd)
{
  int i;
  size_t n = ELEM_NUM(tagtbl);
  
  for (i = 0; i < n; i++)
    if (tag & tagtbl[i].tag && tagtbl[i].func) {
      (*tagtbl[i].func)(fd);
      break;
    }
  
  if (i == n)
    Log_die(DIE_FAILURE, LOG_ERR, "unkonwn tag 0x%" PRIx64, tag);
}
