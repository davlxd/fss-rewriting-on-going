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
#include "io.h"
#include "setsignal.h"
#include <stdlib.h>
#include "server.h"
#include "client.h"
#include <errno.h>

extern int errno;

static void test_network(struct options* o)
{
  if (o->mode == MODE_CLIENT)
    entry_client(o);
  else
    entry_server(o);
}

int main(int argc, char *argv[])
{

  struct options o;
  set0(o);
  init_log();
  
  load_default_options(&o);
  parse_config_file(&o);
  parse_argv(argc, argv, &o);
  reinit_log(&o);
  
  set_basepath(o.path); // for path.c/h
  set_umask(o.umask);   // for fs.c/h
  
  /* set SIGINT SIGTREM to die routine */
  set_int_term();

  test_network(&o);
  
  die(DIE_SUCCESS);

}
