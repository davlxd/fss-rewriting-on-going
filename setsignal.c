/*
 * Signal handlers
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

#include "signal.h"
#include "log.h"
#include <signal.h>


static void sig_int_term(int signo)
{
  Log_die(DIE_SUCCESS, LOG_NOTICE, "Bye~");
}


void init_signal_handler()
{
  if (signal(SIGINT, sig_int_term) == SIG_ERR) {
    Log_die(DIE_FAILURE, LOG_ERR, "init_signal_handler failed");
  }
  if (signal(SIGTERM, sig_int_term) == SIG_ERR) {
    Log_die(DIE_FAILURE, LOG_ERR, "init_signal_handler failed");
  }

}
