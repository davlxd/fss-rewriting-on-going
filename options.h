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

#ifndef _FSS_OPTIONS_H_
#define _FSS_OPTIONS_H_

#include "utils.h"

// Max value numbers after each option on command line
#define MAX_VALUE_NUM  8

#define MODE_UNSET     0
#define MODE_CLIENT    (1)
#define MODE_SERVER    (1<<1)

struct options {
  
  const char *log_file;
  bool syslog;
  bool verbose;

  unsigned int mode;
  const char *server_addr;
  const char *path; // monitored path
  const char *fss_dir;  // fss's dir

  uint16_t local_port;
  uint16_t remote_port;  

};


int load_default_options(struct options *o);
int parse_config_file(struct options *o);
int parse_argv(const int argc, char *argv[], struct options *o);


#endif

  
    
