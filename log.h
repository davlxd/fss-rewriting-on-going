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

#ifndef _FSS_LOG_H_
#define _FSS_LOG_H_

#include "options.h"
#include "exit.h"
#include <syslog.h>

#define MAX_LOG_LEN       1024

void init_log();
void reinit_log(const struct options *o);
int close_log();

#define Log(p, args...) do_log(p, NULL, NULL, 0, args)
#define Log_die(status, p, args...) do { do_log(p, __FILE__, __func__, __LINE__, args); die(status);} while (0)


void do_log(int p, const char*, const char*, int, const char *, ...)
#ifdef __GNUC__
  __attribute__((format (printf, 5, 6)))
#endif
  ;

#endif
