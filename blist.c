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

#include "blist.h"
#include "digest.h"
#include "log.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

extern int errno;

static binfo* alloc_binfo()
{
  void *ptr = calloc(1, sizeof(binfo) + DIGEST_BYTES);
  binfo* bi = (binfo*)ptr;

  bi->digest = (unsigned char*)(ptr + sizeof(binfo));
  return bi;
}

int binfo_of_fd(int fd, finfo* fi, void (*fn)(binfo*), uint64_t blksz,
		bool overlap)
{
  int rv;
  binfo *bi;
  void *ptr;
  size_t i, filesize, incre;
  struct stat sb;
  
  if ((rv = fstat(fd, &sb)) < 0 && errno != ENOENT)
    Log_die(DIE_FAILURE, LOG_ERR, "fstat() failed");
  else if (rv < 0)
    return ENOENT;
  
  filesize = sb.st_size;

  incre = overlap ? 1 : blksz;

  if ((ptr = mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED)
    Log_die(DIE_FAILURE, LOG_ERR, "mmap() failed");


  for (i = 0; i < filesize; i += incre) {
    bi = alloc_binfo();
    
    bi->offset = i;
    if (i + blksz >= filesize) {
      bi->size = filesize - i;
      incre = blksz;
    } else
      bi->size = blksz;
    mem_digest(ptr+i, bi->size, bi->digest);
    bi->fi = fi;
    
    fn(bi);
  }
  
  if (munmap(ptr, filesize) < 0)
    Log_die(DIE_FAILURE, LOG_ERR, "munmap() failed");

  return 0;
}


void cleanup_binfo(binfo* bi)
{
  free(bi);
}
