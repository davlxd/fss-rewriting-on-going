/*
 * A Simple interface to inotify Linux subsystem, focusing on recursively
 * monitoring.
 *
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

#ifndef _FSS_WRAP_INOTIFY_H_
#define _FSS_WRAP_INOTIFY_H_

#include "fss.h"
#include "options.h"
#include <inttypes.h>
#include <sys/inotify.h>

#define MAX_DIR_LEVEL  MAX_PATH_LEN

typedef struct _watcher watcher;
struct _watcher {
  int wd; // defined by inotify
  int level; // temporary level in current nftw()
  char *path;

  // hashtable linked list chain
  watcher *chain_wd;
  watcher *chain_path;

  // watcher's connected as tree according to monitored directory structure,
  // so when sub directores are removed, their corresponding wd(s) can be
  // removed directly
  watcher *prev; // refers to father or left-sibling
  watcher *child;
  watcher *sibling; // refers to right-sibling

};

typedef struct {
  watcher* head;
  watcher* tail;
  
} dirlist;


// return a fd from which messages can be read when something happen
int init_watchers(struct options *o, uint32_t mask, uint64_t htbsz);
void cleanup_watchers();

#endif
