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

// This is function comes from APUE/UNP
void (*setsignal(int signo, void (*handler)(int))) (int)
{
  struct sigaction act, oact;
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  
  if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif
  } else {
#ifdef SA_RESTART
    act.sa_flags |= SA_RESTART;
#endif
  }

  if (sigaction(signo, &act, &oact) < 0)
    return SIG_ERR;
  return oact.sa_handler; // return prev sig handler
}

static void sig_int_term(int signo)
{
  Log_die(DIE_SUCCESS, LOG_NOTICE, "Bye~");
}


void set_int_term()
{
  if (setsignal(SIGINT, sig_int_term) == SIG_ERR) {
    Log_die(DIE_FAILURE, LOG_ERR, "init_signal_handler failed");
  }
  if (setsignal(SIGTERM, sig_int_term) == SIG_ERR) {
    Log_die(DIE_FAILURE, LOG_ERR, "init_signal_handler failed");
  }

}
