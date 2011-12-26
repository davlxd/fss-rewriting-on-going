/*
 * Entry function of client side 
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

#include "client.h"
#include "sock.h"
#include "io.h"
#include "fss.h" // for DEFAULT_RETRY_SEC DEFAULT_TIMEOUT
#include "log.h"
#include "core.h"
#include <errno.h>
#include <sys/stat.h> // DEBUG
#include <fcntl.h> // DEBUG
#include <sys/mman.h> // DEBUG
#include <netdb.h> // for gai_strerror()
#include <unistd.h> // for sleep()

extern int errno;

static const char *server_addr_str;
static uint16_t server_port;

#define send_tag(fd, tag) write_tag(fd, tag, DEFAULT_TIMEOUT)
#define send_file(fd, ffd, sz) fss_redirect((ffd), (fd), (sz), 0, DEFAULT_TIMEOUT)
#define recv_tag(fd) read_tag(fd, DEFAULT_TIMEOUT)

void reconnect(int sockfd)
{
  Log(LOG_DEBUG, "@reconnect()");
  //close(sockfd);
  //unload_flist();
  //sockfd = connect_to_server();
  
}


void entry_client(const struct options *o)
{
  int rv;
  int sockfd;
  server_addr_str = o->server_addr;
  server_port = o->remote_port;

  sockfd = connect_to_server();
  send_tag(sockfd, FINFO);

  sleep(3);

  /* char buf[4]; */
  /* set0(buf); */
  /* Log(LOG_DEBUG, "ready to read"); */
  /* rv = read(sockfd, buf, 4); */
  /* Log(LOG_DEBUG, "rv = %d, errno = %d", rv, errno); */
  /* Log(LOG_DEBUG, "strerror = %s", strerror(errno)); */
  /* Log(LOG_ERR, "my error msg:"); */
  
  
  const char *file = "/home/i/demonstrate_after.mp4";
  struct stat sb;
  int ffd = open(file, O_RDONLY);
  fstat(ffd, &sb);

  if (send_file(sockfd, ffd, sb.st_size))
    reconnect(sockfd);
  
  /* void *ptr = mmap(0, sb.st_size, PROT_READ, MAP_SHARED, ffd, 0); */
  /* Log(LOG_DEBUG, "sb.st_size = %d, errno = %d", sb.st_size, errno); */
  /* Log(LOG_DEBUG, "strerror = %s", strerror(errno)); */
  /* off_t bytes = sb.st_size; */
  /* while ((rv = write(sockfd, ptr, bytes)) < bytes && rv > 0) { */
  /*   ptr += rv; */
  /*   bytes -= rv; */
  /*   Log(LOG_DEBUG, "within write loop, bytes = %d", bytes); */
  /* } */
    
}


int connect_to_server()
{
  int rv;
  const char *errmsg;
  
  while ((rv = tcp_connect(server_addr_str, server_port)) < 0) {
    errmsg = rv == -1 ? strerror(errno) : gai_strerror(rv);
    Log(LOG_NOTICE, "connect to server failed: %s, retry %d sec later",
	errmsg, DEFAULT_RETRY_SEC);
    
    sleep(DEFAULT_RETRY_SEC);
  }

  return rv;
}
