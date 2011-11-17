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

#include "hashtable.h"
#include <stdlib.h>
#include <assert.h>
#include "log.h"
#include "flist.h"
#include "utils.h"

static uint32_t legalize_tablesize(uint64_t x)
{
  uint32_t legal_size = 1;

  if (x <= MIN_HASHTABLE_SIZE)
    return MIN_HASHTABLE_SIZE;
  if (x >= MAX_HASHTABLE_SIZE)
    return MAX_HASHTABLE_SIZE;

  while (legal_size < x)
    legal_size <<= 1;

  return legal_size;
}

hashtable* init_hashtable(uint64_t raw_size)
{
  hashtable *htb = (hashtable*)calloc(1, sizeof(hashtable));
  
  htb->size = legalize_tablesize(raw_size);
  htb->bucket = calloc(htb->size, sizeof(void*));

  return htb;
}

/* void build_hashtable(hashtable *htb, uint32_t key, void *node) */
/* { */
/*   assert(htb->node_type == TYPE_FINFO || htb->node_type == TYPE_BINFO); */
/*   struct finfo *fn; */
/*   struct binfo *bn; */
  
/*   if (htb->node_type == TYPE_FINFO) { */
/*     struct finfo *node_new = (struct finfo*)node; */
/*     struct finfo *node_old = (struct finfo*)(*(htb->node+key)); */
/*     node_new->chain = node_old; */
/*     *(htb->node+key) = node_new; */
    
/*   } else { */
/*     //struct binfo ... */
/*   } */
  
/* } */

void free_hashtable(hashtable *htb)
{
  assert(htb && htb->bucket);
  free(htb->bucket);
  free(htb);

}
