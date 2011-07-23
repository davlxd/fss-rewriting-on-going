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

#include "fss.h"

extern int errno;

static const char usage_message[] =
  "General Options:\n"
  "--help          : Show this.\n"
  "--version       : Show version information.\n"
  "\n"
  "Server side options:\n"
  "--server        : Work as server side.\n"
  "--port p        : Server's listening port number (default 3375).\n"
  "--path p        : Directory path in which synced files and subdirectories save"
  "\n"
  "Client side options:\n"
  "--client        : Work as client side.\n"
  "--port p        : Client's port number (default undefined).\n"
  "--path p        : Client's monitored path.\n"
  "--server-addr h : Server's host name or ip address.\n"
  "--server-port p : Server's listening port number (default 3375).\n"
  ;

void print_usage()
{
  fprintf(stdout, usage_message);
  fflush(stdout);

}




int load_default_options(struct options *o)
{
  o->mode = MOD_CLIENT;
  o->dir = DEFAULT_FSS_DIR;
  o->local_port = o->remote_port = DEFAULT_PORT;
  


}


int parse_config_file(struct options *o)
{

}

int parse_argv(struct options *o)
{

}
