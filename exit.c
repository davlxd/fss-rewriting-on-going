/*
 * Program exit routines
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

#include "exit.h"
#include "log.h"
//#include "flist.h"
#include <stdlib.h>

void die(const int status)
{
  //TODO: add cleanup routines here
  
  /* cleanup_flist(); */
  /* if (status == DIE_SUCCESS) */
  /*   unload_flist(); */
  
  close_log();
  exit(status);
}
