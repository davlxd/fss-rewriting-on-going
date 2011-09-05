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

#include "fss.h"

#include "log.h"
#include "options.h"
#include "exit.h"

int main(int argc, char *argv[])
{
  struct options o;
  set0(&o);

  load_default_options(&o);
  init_log(&o);
  parse_config_file(&o);
  parse_argv(argc, argv, &o);

  Log(LOG_INFO, "test");
  set_basepath(o.path);


  char *full_const = "home/a/b/c/";
  char  relaname[MAX_PATH_LEN];
  set0(&relaname);
  printf("full2rela of %s is --%s--\n", full_const, full2rela(full_const, relaname, MAX_PATH_LEN));

  char *rela_const = "/asdf/c/";
  char fullname[MAX_PATH_LEN];
  set0(&fullname);
  printf("rela2full of %s is --%s--\n", rela_const, rela2full(rela_const, fullname, MAX_PATH_LEN));

  close_log();

  die(DIE_SUCCESS);

  


}
