/*
 * Network handling functions
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

#define _FILE_OFFSET_BITS 64

#include "fss.h"
#include "sock.h"
#include "log.h"
#include "setsignal.h"
#include "utils.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <endian.h>

extern int errno;

int tcp_connect(const char *host, uint16_t port)
{
  int rv, rv1;
  struct addrinfo hints, *res, *ai;
  set0(hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  rv = getaddrinfo(host, port2str(port), &hints, &res);
  if (rv != 0 ) {
    if (rv != EAI_AGAIN && rv != EAI_NONAME)
      Log_die(DIE_FAILURE, LOG_ERR, "getaddrinfo() failed: %s",
	      gai_strerror(rv));
    else
      return rv; // EAI_AGAIN = -3 EAI_NONAME = -2, defined in <netdb.h>
  }
  
  ai = res;
  while (ai) {
    if ((rv = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0){
      Log(LOG_WARNING, "socket() failed");
      continue;
    }
    if ((rv1 = connect(rv, ai->ai_addr, ai->ai_addrlen)) == 0)
      goto end;

    rv = rv1;
    ai = ai->ai_next;
  }

 end:
  freeaddrinfo(res);
  return rv;
}

int tcp_listen(uint16_t port)
{
  int fd, rv;
  struct addrinfo hints, *res, *ai;
  set0(hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(NULL, port2str(port), &hints, &res)) < 0)
    Log_die(DIE_FAILURE, LOG_ERR, "getaddrinfo() failed");

  ai = res;
  while (ai) {
    if ((fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0){
      Log(LOG_WARNING, "socket() failed");
      continue;
    }
    const int set = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set)) < 0)
      Log_die(DIE_FAILURE, LOG_ERR, "setsockopt REUSEADDR on fd failed");
    
    if (bind(fd, ai->ai_addr, ai->ai_addrlen) == 0)
      goto end;
    
    close(fd);
    ai = ai->ai_next;
  }
  freeaddrinfo(res);
  Log_die(DIE_FAILURE, LOG_ERR, "socket() or bind() failed");

 end:

  if (listen(fd, LISTEN_BACKLOG) < 0)
    Log_die(DIE_FAILURE, LOG_ERR, "listen() on fd failed");
  
  freeaddrinfo(res);
  return fd;
}
