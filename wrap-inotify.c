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
#define _XOPEN_SOURCE 500

#include "path.h"
#include "wrap-inotify.h"
#include "ptrstack.h"
#include "hashtable.h"
#include "digest.h"
#include "log.h"
#include "utils.h" // for OFFSETOF
#include <pthread.h>
#include <assert.h>
#include <ftw.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
extern int errno;

static int pfd[2]; // pipe fd
static int ifd; // inotify fd
static pthread_t tid;
static uint32_t mask;

static const char *basepath;
static char fssdir_path[MAX_PATH_LEN];
static bool include_hidden;
static watcher *INFO_ROOT;

static ptrstack* wtstack;
static hashtable *hashtb_wd;
static hashtable *hashtb_path;

void print_watchers_as_hashtb();
void print_watchers_as_tree();



static uint64_t hashing_wd(watcher *w)
{
  return w->wd % hashtb_wd->size;
}

static uint64_t hashing_path(watcher *w)
{
  assert(w && w->path);
  return digest2hashkey(str_digest(w->path), (hashtb_path->size - 1));
}

static watcher* alloc_watcher(const char *path)
{
  size_t pathlen = strlen(path) + 1;
  void *ptr = calloc(1, sizeof(watcher) + pathlen);
  watcher *w = (watcher*)ptr;

  w->path = (char*)(ptr + sizeof(watcher));
  strncpy(w->path, path, pathlen);
  
  return w;
}


static int insert_watcher(const char *fullname,
			  const struct stat *sb,
			  int info,
			  struct FTW *ftw)
{
  if (!S_ISDIR(sb->st_mode))
    return 0;
  if (streq(fssdir_path, fullname))
    return 0;
  if (!include_hidden && IS_HIDDEN(fullname))
    return 0;
  

  watcher *w = alloc_watcher(fullname);
  w->wd = inotify_add_watch(ifd, fullname, mask);
  w->level = ftw->level;

  hashtable_insert(hashtb_wd, hashing_wd(w), w);
  hashtable_insert(hashtb_path, hashing_path(w), w);

  // the following code build tree based on the pattern of level number
  // of post-order traverse(nftw)
  while (1) {
    watcher *top = (watcher*)ptrstack_top(wtstack);
    if (top == NULL || top->level < ftw->level) {
      ptrstack_push(wtstack, w);
      return 0;
    
    }else if (top->level == ftw->level) {
      w->sibling = top;
      top->prev = w;
      ptrstack_pop(wtstack);
      continue;
      
    } else {
      w->child = top;
      top->prev = w;
      ptrstack_pop(wtstack);
      continue;
    }
  }

}

static void remove_watcher(void *_w)
{
  watcher *w = (watcher *)_w;
  
  if (w->prev && w->prev->child == w)
    w->prev->child = w->sibling;
  else if (w->prev && w->prev->sibling == w)
    w->prev->sibling = w->sibling;
  
  hashtable_remove(hashtb_wd, hashing_wd(w), w);
  free(hashtable_remove(hashtb_path, hashing_path(w), w));
}

// post-order
static void post_traverse_watchers(watcher *w, void (*fn)(void *))
{
  if (!w) return ;

  post_traverse_watchers(w->child, fn);
  post_traverse_watchers(w->sibling, fn);
  fn(w);
}

static void insert_watchers_recur(const char *path)
{
  int nftw_flags = 0;
  nftw_flags |= FTW_DEPTH; // Post-order traverse
  
  if (nftw(path, insert_watcher, DEPTH_OF_NFTW, nftw_flags) != 0)
    Log_die(DIE_FAILURE, LOG_ERR, "nftw() on %s failed", path);
  
  assert(wtstack->stacktop == 1);
}

static void insert_watcher_child(watcher *father, watcher *w)
{
  assert(father && w);
  
  w->sibling = father->child;
  if (father->child)
    father->child->prev = w;
  father->child = w;
  w->prev = father;
  
}

static void remove_watchers_recur(watcher *w)
{
  post_traverse_watchers(w->child, remove_watcher);
  remove_watcher(w);
}

// It is so lucky `wd' start from 1, if not, better split `hit()'
static bool hit(void *_preqw, void *_w)
{
  watcher *preqw = (watcher*)_preqw;
  watcher *w = (watcher*)_w;
  
  if (preqw->wd == w->wd)
    return true;
  if (preqw->path && w->path && streq(preqw->path, w->path))
    return true;
  return false;
}

static void watchers_polling()
{
  int i;
  watcher preqw, *viewer, *actor, *wtop;
  char buf[BUF_LEN]; set0(buf);
  char fullname[MAX_PATH_LEN]; set0(fullname);
  ssize_t len;
  
  while (1) {
    if ((len = read(ifd, buf, BUF_LEN)) < 0)
      Log_die(DIE_FAILURE, LOG_ERR, "read from inotify fd failed");

    // TODO: write raw message to fd, should be refined
    write(pfd[1], buf, len);

    i = 0;
    while (i < len) {
      struct inotify_event *event = (struct inotify_event*)&buf[i];
	
      if (event->mask & IN_ISDIR) {
	set0(preqw);
	preqw.wd = event->wd;
	viewer = (watcher*)hashtable_search(hashtb_wd, hashing_wd(&preqw),
					   &preqw, hit);
	assert(viewer);
	
	set0(fullname);
	pathncat2(fullname, viewer->path, event->name, MAX_PATH_LEN);


	if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO) {
	  insert_watchers_recur(fullname);
	  insert_watcher_child(viewer, (watcher*)ptrstack_pop(wtstack));
	  
	}else if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM) {
	  set0(preqw);
	  preqw.path = fullname;
	  actor = (watcher*)hashtable_search(hashtb_path,
					     hashing_path(&preqw),
					     &preqw, hit);
	  assert(actor);
	  remove_watchers_recur(actor);
	}
      }

      i += sizeof(struct inotify_event) + event->len;
    } // end for `while (i < len) {'

    print_watchers_as_hashtb();
    print_watchers_as_tree();
    
  } // end for `while (1) {'
}

static void *thread_new(void *arg)
{
  watchers_polling();
  pthread_exit((void*)0);
}

int init_watchers(struct options *o, uint32_t m, uint64_t htbsz)
{
  mask = m;
  pipe(pfd);
  basepath = o->path;
  pathncat2(fssdir_path, basepath, o->fss_dir, MAX_PATH_LEN);
  include_hidden = o->include_hidden;
  
  
  wtstack = init_ptrstack(MAX_DIR_LEVEL);
  hashtb_wd = init_hashtable(htbsz, OFFSETOF(watcher, chain_wd));
  hashtb_path = init_hashtable(power_of_2_ceiling(htbsz),
			       OFFSETOF(watcher, chain_path));
  
  if ((ifd = inotify_init()) < 0)
    Log_die(DIE_FAILURE, LOG_ERR, "inotify_init() failed");

  insert_watchers_recur(basepath);
  INFO_ROOT = (watcher*)ptrstack_pop(wtstack);
  print_watchers_as_hashtb();
  print_watchers_as_tree();

  if (pthread_create(&tid, NULL, thread_new, NULL) < 0 )
    Log_die(DIE_FAILURE, LOG_ERR, "pthread_create failed");
  
  return pfd[0];
}

void cleanup_watchers()
{
  traverse_hashtable(hashtb_wd, remove_watcher);
  free_hashtable(hashtb_wd);
  free_hashtable(hashtb_path);
}


// TEST FUNCTIONS
void print_watcher(void *_w)
{
  assert(_w);
  watcher *w = (watcher*)_w;
  Log(LOG_INFO, "watcher %2d  %9p  %9p  %9p  %9p  %9p  %9p  %s ", w->wd, w, w->chain_wd, w->chain_path, w->prev, w->child, w->sibling, w->path);
}

void print_watchers_as_hashtb()
{
  Log(LOG_INFO, "\n\n");
  Log(LOG_INFO, "print as wd hashtable");
  traverse_hashtable(hashtb_wd, print_watcher);

  Log(LOG_INFO, "\n\n");
  Log(LOG_INFO, "print as path hashtable");
  traverse_hashtable(hashtb_path, print_watcher);

}

void print_watchers_as_tree()
{
  Log(LOG_INFO, "\n\n");
  Log(LOG_INFO, "print as tree");
  post_traverse_watchers(INFO_ROOT, print_watcher);
}


  

