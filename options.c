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

#include "options.h"
#include "fss.h"
#include "log.h"
#include "exit.h"
#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <getopt.h>
#include <errno.h>

extern int errno;

static const char usage_message[] =
  "General Options:\n"
  "-h, --help       : Show this.\n"
  "--version        : Show version information.\n"
  "-v, --verbose    : Show verbose output.\n"
  "--log [file]     : Output logging records to specific file (default is stdout).\n"
  "--syslog         : Output logging reoords to syslog.\n"
  "--umask value    : Set the umask for received files/directories, default is umask of current process.\n"
  "\n"
  
  "Server side options:\n"
  "--server         : Work as server side.\n"
  "--port p         : Server's listening port number (default %d).\n"//arg1
  "--path p         : Directory path in which synced files and subdirectories save\n"
  "\n"
  
  "Client side options:\n"
  "--client          : Work as client side (default).\n"
  "--port p          : Client's port number (default undefined).\n"
  "--path p          : Client's monitored path.\n"
  "--server-addr h   : Server's host name or ip address.\n"
  "--server-port p   : Server's listening port number (default %d).\n"//arg2
  "--follow-symbolic : Unset if you want to skip symbolic link files( default unset).\n"
  "--include-hidden  : Set if you want to monitored file/directories( default unset).\n"
  "--force-use-digest: Using strong digest instead of Last Modification Time to decide if a file was modified locally, you may want to set this when you need to change mtime manually under certain circumstances or your system clock is very unstable , but it is very slow to compute strong digest of a very large file (default unset).\n"
  "\n\n"
  ;

static struct option long_options[]= {
  {"help",		no_argument,		0,	'h' },	
  {"version",		no_argument,		0,	0 },	
  {"verbose",		no_argument,		0,	'v' },	
  {"log",		required_argument,	0,	0 },	
  {"syslog",		no_argument,		0,	0 },	
  {"umask",		required_argument,	0,	0 },	
  
  {"server",		no_argument,		0,	0 },	
  {"port",		required_argument,	0,	0 },	
  {"path",		required_argument,	0,	0 },	
  
  {"client",		no_argument,		0,	0 },	
  {"server-addr",	required_argument,	0,	0 },	
  {"server-port",	required_argument,	0,	0 },	
  {"force-use-digest",	no_argument,		0,	0 },	
  {"follow-symbolic",	no_argument,		0,	0 },	
  {"include-hidden",	no_argument,		0,	0 },	
  {0,			0,			0,	0 }
  
};


static void print_usage()
{
  fprintf(stdout, usage_message, DEFAULT_PORT, DEFAULT_PORT);
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
  o->umask = get_proc_umask();
  o->verbose = false;
  
  o->mode = MODE_CLIENT;
  o->force_use_digest = false;
  o->follow_symbolic = false;
  o->include_hidden = false;
  o->server_addr = NULL;
  o->path = NULL;
  
  o->fss_dir = DEFAULT_FSS_DIR; // ".fss"
  
  o->local_port = o->remote_port = DEFAULT_PORT;  // 3375

}


int parse_config_file(struct options *o)
{

}


/*
static void set_option(char *o[], struct options *oo)
{
  if (streq("-h", o[0]) || streq("--help", o[0]))
    print_usage();
  
  else if (streq("--version", o[0]))
    print_version();

  else if (streq("-v", o[0]) || streq("--verbose", o[0]))
    oo->verbose = true;

  else if (streq("--log", o[0]))
    oo->log_file = o[1];

  else if (streq("--syslog", o[0]))
    oo->syslog = true;
  else if (streq("--umask", o[0])) {
    if (!o[1])
      Log_die(DIE_FAILURE, LOG_ERR, "umask value doesn't exist");
    if (!(oo->umask = str2modet(o[1])))
      Log_die(DIE_FAILURE, LOG_ERR, "umask cannot be parsed");
  }

  else if (streq("--server", o[0]))
    oo->mode = MODE_SERVER;

  else if (streq("--client", o[0]))
    oo->mode = MODE_CLIENT;

  else if (streq("--force-use-digest", o[0]))
    oo->force_use_digest = true;

  else if (streq("--port", o[0])) {
    if (!o[1])
      Log_die(DIE_FAILURE, LOG_ERR, "port num doesn't exit");
    
    if (!(oo->local_port = str2port(o[1])))
      Log_die(DIE_FAILURE, LOG_ERR, "port num %s cannot be parsed", o[1]);
  }

  else if (streq("--path", o[0])) {
    if (!o[1])
      Log_die(DIE_FAILURE, LOG_ERR, "path dosen't exist");
    else
      oo->path = o[1];
  }

  else if (streq("--server-addr", o[0])) {
    oo->server_addr = o[1];
  }

  else if (streq("--server-port", o[0])) {
    if (!o[1])
      Log_die(DIE_FAILURE, LOG_ERR, "server-port num dosen't exist");

    if (!(oo->remote_port = str2port(o[1])))
      Log_die(DIE_FAILURE, LOG_ERR, "server-port num %s cannot be parsed", o[1]);
  }
  
}
*/

static void set_option(const char *name, const char *value,
		       struct options *o)

{
  if (streq("h", name))
    print_usage();

  else if (streq("version", name))
    print_version();

  else if (streq("v", name))
    o->verbose = true;

  else if (streq("log", name))
    o->log_file = value;

  else if (streq("syslog", name))
    o->syslog = true;

  else if (streq("umask", name)) {
    if (value)
      Log_die(DIE_FAILURE, LOG_ERR, "umask value doesn't exist");
    if (!(o->umask = str2modet(value)))
      Log_die(DIE_FAILURE, LOG_ERR, "umask cannot be parsed");
  }

  else if (streq("server", name))
    o->mode = MODE_SERVER;

  else if (streq("client", name))
    o->mode = MODE_CLIENT;

  else if (streq("follow-symbolic", name))
    o->follow_symbolic = true;

  else if (streq("include-hidden", name))
    o->include_hidden = true;
  
  else if (streq("force-use-digest", name))
    o->force_use_digest = true;

  else if (streq("port", name)) {
    if (!value)
      Log_die(DIE_FAILURE, LOG_ERR, "port num doesn't exit");
    
    if (!(o->local_port = str2port(value)))
      Log_die(DIE_FAILURE, LOG_ERR, "port num %s cannot be parsed", value);
  }

  else if (streq("path", name)) {
    if (!value)
      Log_die(DIE_FAILURE, LOG_ERR, "path dosen't exist");
    else
      o->path = value;
  }

  else if (streq("server-addr", name)) {
    o->server_addr = value;
  }

  else if (streq("server-port", name)) {
    if (!value)
      Log_die(DIE_FAILURE, LOG_ERR, "server-port num dosen't exist");

    if (!(o->remote_port = str2port(value)))
      Log_die(DIE_FAILURE, LOG_ERR, "server-port num %s cannot be parsed", value);
  }
  
}



static void check_options(struct options *o)
{
  if (!o->path)
    Log_die(DIE_FAILURE, LOG_ERR, "path is unset");
  
  if (!verify_dir(o->path))
    Log_die(DIE_FAILURE, LOG_ERR, "path %s is not a valid directory", o->path);

  if (o->mode == MODE_CLIENT && !o->server_addr)
    Log_die(DIE_FAILURE, LOG_ERR, "server address is unset");
  //verify_dir

}


int parse_argv(const int argc, char *argv[], struct options *o)
{
  int c, opt_index;
  assert(argc && argv && o);
  if (argc <= 1)
    print_usage();

  while(1) {
    opt_index = 0;
    c = getopt_long(argc, argv, "hv", long_options, &opt_index);
    if (c == -1)
      break;

    switch(c) {
    case 0:
      set_option(long_options[opt_index].name, optarg, o);
      break;
      
    case 'h':
      set_option("h", optarg, o);
      break;
      
    case 'v':
      set_option("v", optarg, o);
      break;
      
    case '?':
      break;
      
    default:
      Log_die(DIE_FAILURE, LOG_ERR, "unrecognized opt code: %c", c);
    }
  }

  while (optind < argc)
    Log(LOG_ERR, "unrecognized option: %s ", argv[optind++]);
  
  check_options(o);
}
