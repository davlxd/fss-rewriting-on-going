/*
 * Entry function of server side 
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

#include "server.h"
#include "flist.h"
#include "core.h"
#include "log.h"
#include <poll.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <endian.h> // for be64toh
#include <unistd.h>
#include <errno.h>
extern int errno;

struct pollfd *client;
static int client_idx;

static void kill_client(struct pollfd *cli, int i)
{
  close((cli + i)->fd);
  (cli + i)->fd = -1;
}

void time_out(int signo)
{
  Log(LOG_DEBUG, "@time_out()");
  kill_client(client, client_idx);
}

void server_polling(int listener)
{
  int i, fd, res_num;
  ssize_t n;
  uint64_t tag;
  nfds_t pfd_num = MAX_CLIENTS + 1; // + 1 for listener itself
  struct pollfd pfd[pfd_num];
  client = pfd;
  struct sockaddr cliaddr;
  socklen_t cliaddrlen;
  
  for (i = 0; i < pfd_num; i++)
    pfd[i].fd = -1;

  pfd[0].fd = listener;
  pfd[0].events = POLLIN;

  while (1) {
    Log(LOG_DEBUG, "polling.......");
    if ((res_num = poll(pfd, pfd_num, 10000)) < 0)
      Log_die(DIE_FAILURE, LOG_ERR, "poll() failed");

    if (pfd[0].revents & POLLIN) { // acceptable
      cliaddrlen = sizeof(struct sockaddr);
      if ((fd = accept(listener, &cliaddr, &cliaddrlen)) < 0)
	Log_die(DIE_FAILURE, LOG_ERR, "accept() failed");

      for (i = 1; i < pfd_num; i++)
	if (pfd[i].fd < 0) {
	  pfd[i].fd = fd;
	  //set_nonblock(pfd[i].fd);
	  break;
	}
      
      if (i == pfd_num)
	Log(LOG_WARNING, "client number over upper limit %d", MAX_CLIENTS);
      else {
	pfd[i].events = POLLIN;
	Log(LOG_NOTICE, "client[%d] connected", i);
      }

      if (--res_num <= 0)
	continue;
    }

    for (i = 1; i < pfd_num; i++) {
      if ((fd = pfd[i].fd) < 0)
	continue;

      if (pfd[i].revents & (POLLIN | POLLERR)) {
	tag = read_tag(fd, NULL);
	//read_tag(fd, &tag, sizeof(tag), NULL);
	if ((n < 0 && errno == ECONNRESET) || n == 0) {
	  Log(LOG_NOTICE, "client[%d] reseted/closed connection", i);
	  kill_client(pfd, i);
	} else if (n < 0)
	  Log_die(DIE_FAILURE, LOG_ERR, "read from client[i] failed", i);
	else {
	  client_idx = i;
	  tag_switch(tag, fd);
	}
      }

      if (--res_num <= 0)
	break;
    }
    
  } // end for while(1) ..
  
}

void entry_server(const struct options *o)
{

  int listener = tcp_listen(o->local_port);
  server_polling(listener);
  
}

