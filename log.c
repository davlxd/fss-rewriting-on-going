/*
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

#include "log.h"
#include "exit.h"
#include "fss.h"

extern int errno;

static FILE *_logfp = NULL;
static bool _syslog = false;
static bool _verbose = false;

static void open_syslog();
static void close_syslog();


void init_log(const struct options *o)
{
  if (o->log_file) {
    if (!(_logfp = fopen(o->log_file, "a+")))
	Log_die(DIE_FAILURE, LOG_ERR, "fopen(%s) failed", o->log_file);
    
  } else
      _logfp = stdout;
    

  o->syslog ? open_syslog() : close_syslog();

  _verbose = o->verbose;
}


static void open_syslog()
{
  if (!_syslog) {
    openlog(PACKAGE_NAME, LOG_PID, LOG_DAEMON);
    _syslog = true;
  }
}

static void close_syslog()
{
  if (_syslog) {
    closelog();
    _syslog = false;
  }

}


void close_log()
{
  close_syslog();
  
  if (_logfp && _logfp != stdout)
    if (fclose(_logfp) == EOF)
      Log_die(DIE_FAILURE, LOG_ERR, "fclose() _logfp failed");
  
  _logfp = NULL;
}



void Log(int priority, const char *fmt, ...)
{
  char custom_msg[MAX_LOG_LEN];
  char log_record[MAX_LOG_LEN];
  set0(&log_record);
  set0(&custom_msg);

  va_list ap;
  va_start(ap, fmt);
  vsnprintf(custom_msg, MAX_LOG_LEN, fmt, ap);
  va_end(ap);

  char *flag;
  int syslog_priority = 0;

  switch (priority) {
  case LOG_EMERG:
    flag = "EMERG";
    break;
    
  case LOG_ALERT:
    flag = "ALERT";
    break;

  case LOG_CRIT:
    flag = "CRIT";
    break;

  case LOG_ERR:
    flag = "ERROR";
    break;

  case LOG_WARNING:
    flag = "WARNING";
    break;

  case LOG_NOTICE:
    flag = "NOTICE";
    break;

  case LOG_INFO:
    if (!_verbose)
      return;
    flag = "INFO";
    break;

  case LOG_DEBUG:
    flag = "DEBUG";
    break;
  }

  if (_syslog) {
    if (errno) // append standard error message string if errno is set
      syslog(priority, "%s: %s", custom_msg, strerror(errno));
    else
      syslog(priority, "%s", custom_msg);
  }

  // Just if _syslog == true and _logfp == stdout
  // so we don't print log messages to _logfp
  if (_logfp != stdout || !_syslog) {
    if (errno)
      fprintf(_logfp, "%s: %s: %s\n", flag, custom_msg, strerror(errno));
    else
      fprintf(_logfp, "%s: %s\n", flag, custom_msg);
    fflush(_logfp);
  }

  //strncpy(log_record, MAX_LOG_LEN, "%s: %s: %s\n", flag, custom_msg, strerr_msg);

}
