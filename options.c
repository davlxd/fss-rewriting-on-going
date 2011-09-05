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
#include "options.h"
#include "log.h"
#include "exit.h"

extern int errno;

static const char usage_message[] =
  "General Options:\n"
  "--help          : Show this.\n"
  "--version       : Show version information.\n"
  "-v, --verbose   : Show verbose output.\n"
  "--log [file]    : Output logging records to specific file.\n"
  "--syslog        : Output logging reoords to syslog.\n"
  "\n"
  "Server side options:\n"
  "--server        : Work as server side.\n"
  "--port p        : Server's listening port number (default 3375).\n"
  "--path p        : Directory path in which synced files and subdirectories save\n"
  "\n"
  "Client side options:\n"
  "--client        : Work as client side (default).\n"
  "--port p        : Client's port number (default undefined).\n"
  "--path p        : Client's monitored path.\n"
  "--server-addr h : Server's host name or ip address.\n"
  "--server-port p : Server's listening port number (default 3375).\n"
  ;

static void print_usage()
{
  fprintf(stdout, usage_message);
  fflush(stdout);

  die(DIE_SUCCESS);
}

static void print_version()
{
  fprintf(stdout, "%s\n", PACKAGE_STRING);
  fflush(stdout);

  die(DIE_SUCCESS);
}

int load_default_options(struct options *o)
{
  o->log_file = NULL;
  o->syslog = false;
  o->verbose = false;
  
  o->mode = true;
  o->server_addr = NULL;
  o->path = NULL;
  
  o->fss_dir = DEFAULT_FSS_DIR; // ".fss"
  
  o->local_port = o->remote_port = DEFAULT_PORT;  // 3375

}


int parse_config_file(struct options *o)
{

}

static uint16_t str2port(const char *str)
{
  uint16_t port;
  if (sscanf(str, "%" SCNu16, &port) == EOF)
    Log_die(DIE_FAILURE, LOG_ERR, "sscanf() failed");
  return port;
}

static void set_option(char *o[], struct options *oo)
{
  if (streq("--help", o[0]))
    print_usage();
  
  else if (streq("--version", o[0]))
    print_version();

  else if (streq("-v", o[0]) || streq("--verbose", o[0]))
    oo->verbose = true;

  else if (streq("--log", o[0]))
    oo->log_file = o[1];

  else if (streq("--syslog", o[0]))
    oo->syslog = true;

  else if (streq("--server", o[0]))
    oo->mode = MODE_SERVER;

  else if (streq("--client", o[0]))
    oo->mode = MODE_CLIENT;

  else if (streq("--port", o[0])) {
    if (!o[1])
      Log_die(DIE_FAILURE, LOG_ERR, "Parsing --port, port num doesn't exist");
    
    if (!(oo->local_port = str2port(o[1])))
      Log_die(DIE_FAILURE, LOG_ERR, "Parsing --port, port num %s cannot be parsed", o[1]);
  }

  else if (streq("--path", o[0])) {
    if (!verify_dir(o[1]))
      Log_die(DIE_FAILURE, LOG_ERR, "Parsing --path, %s is invalid", o[1]);
    else
      oo->path = o[1];
  }

  else if (streq("--server-addr", o[0])) {
    oo->server_addr = o[1];
  }

  else if (streq("--server-port", o[0])) {
    if (!o[1])
      Log_die(DIE_FAILURE, LOG_ERR, "Parsing --server-port, port num dosen't exist");

    if (!(oo->remote_port = str2port(o[1])))
      Log_die(DIE_FAILURE, LOG_ERR, "Parsing --server-port, port num %s cannot be parsed", o[1]);
  }
  
}



static void examine_options(struct options *o)
{

}


int parse_argv(const int argc, char *argv[], struct options *o)
{
  int i, j, k;
  
  if (argc <= 1)
    print_usage();

  for (i = 1; i < argc; i++) {
    char *option[MAX_VALUE_NUM];
    set0(&option);

    if (strncmp(argv[i], "-", 1))
      Log_die(DIE_FAILURE, LOG_ERR, "Parsing %s, without leading -", argv[i]);
    
    option[0] = argv[i++];

    for (j = 1;
	 (j < MAX_VALUE_NUM) && (i < argc) && (strncmp(argv[i], "-", 1));
	 j++, i++) 
      option[j] = argv[i];

    set_option(option, o);
  }

  examine_options(o);
}


