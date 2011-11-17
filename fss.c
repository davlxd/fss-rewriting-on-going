/*
 * fss - File Synchronization System , detect file changes under
 *       specific directory on one client then synchronize to other
 *       client via a centralized server
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


#include "log.h"
#include "utils.h"
#include "options.h"
#include "path.h"
#include "fs.h"
#include "setsignal.h"
#include <stdlib.h>
//#include "server.h"
//#include "client.h"
#include <unistd.h>
#include <errno.h>

extern int errno;

static void test_flist(struct options* o)
{
  char buf[2];
  memset(buf, 0, 2);
  ssize_t num;
  init_flist(o);
  load_flist();

  while ((num = read(STDIN_FILENO, buf, 2)) > 0)
    switch (*buf) {
    case 'q':
      unload_cleanup_flist();
      printf("Exit\n");
      exit(EXIT_SUCCESS);
    case 'u':
      printf("\n");
      update_flist();
      print_flist();
      break;
    case 'w':
      unload_flist();
      print_flist();
      break;
    default:
      fprintf(stderr, "TEST: unknown input %c: %s\n", *buf, strerror(errno));
    }
  
  if (num < 0) {
    perror("TEST: read from STDIN failed");
    exit(EXIT_FAILURE);
  }
    
}

int main(int argc, char *argv[])
{

  struct options o;
  set0(o);

  load_default_options(&o);
  parse_config_file(&o);
  parse_argv(argc, argv, &o);
  init_log(&o);

  set_basepath(o.path); // for path.c/h
  set_umask(o.umask);   // for fs.c/h
  
  /* set SIGINT SIGTREM to die routine */
  init_signal_handler();

  test_flist(&o);
  
  die(DIE_SUCCESS);

}
