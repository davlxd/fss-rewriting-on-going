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
#include "config.h"
#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

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


int close_log()
{
  close_syslog();
  
  if (_logfp && _logfp != stdout)
    if (fclose(_logfp) == EOF) {
      Log(LOG_ERR, "fclose() _logfp failed");
      return -1;
    }
  
  _logfp = NULL;
  return 0;
}



void do_log(int priority,
	    const char* _file, const char* _func, int _line,
	    const char *fmt, ...)
{
  size_t msg_len = 0;
  char log_record[MAX_LOG_LEN];
  set0(log_record);

  va_list ap;
  va_start(ap, fmt);
  vsnprintf(log_record, MAX_LOG_LEN, fmt, ap);
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

  if (_file && _func && _line) {
    msg_len = strlen(log_record);
    snprintf(log_record+msg_len, MAX_LOG_LEN-msg_len-1, 
	     " (%s, %s(), %d)", _file, _func, _line);
  }

  if (errno && (priority == LOG_ERR || priority == LOG_WARNING)) {
    msg_len = strlen(log_record);
    snprintf(log_record+msg_len, MAX_LOG_LEN-msg_len-1, 
	     ": %s", strerror(errno));
  }

  if (_syslog)
    syslog(priority, "%s.", log_record);

  // if _syslog == true and _logfp == stdout
  // so we don't print log messages to _logfp
  if (_logfp != stdout || !_syslog) {
    fprintf(_logfp, "%s: %s.\n", flag, log_record);
    fflush(_logfp);
  }

}
