/*
* Command-line options and config file parse 
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

#include <stdio.h>

#ifndef _FSS_OPTIONS_H_
#define _FSS_OPTIONS_H_

#include "fss.h"

struct options {

#define MOD_SERVER 0
#define MOD_CLIENT 1
  
  int mode;  //0 - run as server, 1 - run as client
  char server_addr[MAX_PATH_LEN];
  char path[MAX_PATH_LEN]; // monitored path
  char dir[MAX_PATH_LEN];  // fss's dir

  int local_port;
  int remote_port;
  
  



};




int load_default_options(struct options *o);
int parse_config_file(struct options *o);
int parse_argv(struct options *o);
  


#endif

  
    
