/*
 * File/Standard I/O wrappers
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

#include "io.h"
#include "fss.h"
#include "log.h"
#include "utils.h"
#include "path.h"
#include "digest.h" // for fi->digest, DIGEST_BYTES
#include <ftw.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <endian.h>
#include <errno.h>
extern int errno;

off_t file_size(FILE *fp)
{
  assert(fp);
  
  struct stat sb;
  fstat(fileno(fp), &sb);
  return sb.st_size;

}

char* file_getline(FILE *fp, char *buf, size_t sz)
{
  assert(fp && buf);
  char *rv = fgets(buf, sz, fp);

  if (strlen(buf) > 0 && *(buf + strlen(buf) - 1) == LF)
    *(buf + strlen(buf) - 1) = 0;

  return rv;
}

int file_putline(FILE *fp, const char *buf)
{
  assert(fp && buf);
  
  int num0 = fputs(buf, fp);
  int num1 = fputc(LF, fp);
  
  return num0 + num1;
}


void set_nonblock(int fd)
{
  int rv;
  if ((rv = fcntl(fd, F_GETFL, 0)) < 0)
    Log_die(DIE_FAILURE, LOG_ERR, "fcntl() F_GETFL fd %d failed", fd);
  if (fcntl(fd, F_SETFL, rv | O_NONBLOCK) < 0)
    Log_die(DIE_FAILURE, LOG_ERR, "fcntl() F_SETFL fd %d NONBLOCK failed", fd);
}

static void sig_timeout(int signo)
{
  assert(signo == SIGALRM);
  Log(LOG_DEBUG, "@sig_timeout");
  return ;
}


// If `timeo' != 0, `fss_write()' set a timeout on syscall `write()'
// return 1 if than happens
int fss_write(int fd, const void *buf, uint64_t _len, uint32_t timeo)
{
  Log(LOG_DEBUG, "@fss_write, fd = %d, _len = %" PRIu64 "", fd, _len);
  ssize_t rv;
  const void *ptr = buf;
  size_t len = _len;
  void (*ohanlder)(int);

  if (timeo) {
    if ((ohanlder = setsignal(SIGALRM, sig_timeout)) == SIG_ERR)
      Log_die(DIE_FAILURE, LOG_ERR, "setsignal failed");
  
    if (alarm(timeo) != 0)
      Log(LOG_WARNING, "alarm already set");
  }
  
  while ((rv = write(fd, ptr, len)) < len) {
    if (rv < 0 && (errno == ECONNRESET || errno == EPIPE))
      return 1;
    if (rv < 0 && errno == EINTR && timeo)
      return 1;
    else if (rv < 0 && errno == EINTR)
      continue;
    else if (rv < 0)
      Log_die(DIE_FAILURE, LOG_ERR, "write() failed");
    
    ptr += rv;
    len -= rv;
  }

  if (timeo) {
    alarm(0);
    setsignal(SIGALRM, ohanlder);
  }

  return 0;
}

// Same as `fss_write()'
int fss_read(int fd, void *buf, uint64_t _len, uint32_t timeo)
{
  ssize_t rv;
  void *ptr = buf;
  uint64_t len = _len;
  void (*ohanlder)(int);

  if (timeo) {
    if ((ohanlder = setsignal(SIGALRM, sig_timeout)) == SIG_ERR)
      Log_die(DIE_FAILURE, LOG_ERR, "setsignal failed");
  
    if (alarm(DEFAULT_TIMEOUT) != 0)
      Log(LOG_WARNING, "alarm already set");
  }

  while ((rv = read(fd, ptr, len)) < len) {
    if (rv == 0)
      return 1;
    else if (rv < 0 && errno == ECONNRESET)
      return 1;
    else if (rv < 0 && errno == EINTR && timeo)
      return 1;
    else if (rv < 0 && errno == EINTR)
      continue;
    else if (rv < 0)
      Log_die(DIE_FAILURE, LOG_ERR, "read failed");

    ptr += rv;
    len -= rv;
  }
  
  if (timeo) {
    alarm(0);
    setsignal(SIGALRM, ohanlder);
  }
  return 0;
}


// connect `fss_read()' and `fss_write()'
int fss_redirect(int fd0, int fd1, uint64_t size, uint32_t t0, uint32_t t1)
{
  Log(LOG_DEBUG, "@fss_rw, fd0 = %d, fd1 = %d, size = %" PRIu64 ", t0 = %" PRIu32 ", t1 = %" PRIu32"", fd0, fd1, size, t0, t1);
  unsigned char buf[BUF_LEN]; set0(buf);
  off_t curoff;
  uint64_t len;

  while (size) {
    len = min(size, BUF_LEN);
    assert(size > 0);
    
    if (fss_read(fd0, buf, len, t0))
      return 1;
    if (fss_write(fd1, buf, len, t1))
      return 1;
    
    size -= len;
  }
}

int write_uint64(int fd, uint64_t u64, uint32_t timeo)
{
  uint64_t u64_be = htobe64(u64);
  return fss_write(fd, &u64_be, sizeof(u64), timeo);
}

int read_uint64(int fd, uint64_t *u64, uint32_t timeo)
{
  if (fss_read(fd, u64, sizeof(*u64), timeo))
    return 1;
  be64toh(*u64);
  return 0;
}
  
int write_uint32(int fd, uint32_t u32, uint32_t timeo)
{
  uint32_t u32_be = htobe32(u32);
  return fss_write(fd, &u32_be, sizeof(u32), timeo);
}

int read_uint32(int fd, uint32_t *u32, uint32_t timeo)
{
  if (fss_read(fd, u32, sizeof(*u32), timeo))
    return 1;
  be32toh(*u32);
  return 0;
}

int write_uint16(int fd, uint16_t u16, uint32_t timeo)
{
  uint16_t u16_be = htobe16(u16);
  return fss_write(fd, &u16_be, sizeof(u16), timeo);
}

int read_uint16(int fd, uint16_t *u16, uint32_t timeo)
{
  if (fss_read(fd, u16, sizeof(*u16), timeo))
    return 1;
  be16toh(*u16);
  return 0;
}

uint64_t read_tag(int fd, uint32_t timeo)
{
  uint64_t tag;
  return read_uint64(fd, &tag, timeo) ? (uint64_t)0 : tag;
}

int write_finfo(int fd, const finfo *fi, uint32_t timeo)
{
  assert(fi);
  
  if (write_uint32(fd, fi->mode, timeo))    return 1;
  if (write_uint64(fd, fi->size, timeo))    return 1;
  if (write_uint64(fd, fi->mtime, timeo))   return 1;
  if (write_uint16(fd, fi->pathlen, timeo)) return 1;

  if (write_bytes(fd, fi->digest, DIGEST_BYTES, timeo)) return 1;
  if (write_buf(fd, fi->relapath, fi->pathlen, timeo))  return 1;

  return 0;
}

finfo* read_finfo(int fd, uint32_t timeo)
{
  uint32_t mode;
  uint64_t size;
  uint64_t mtime;
  uint16_t pathlen;

  if (read_uint32(fd, &mode, timeo))    return NULL;
  if (read_uint64(fd, &size, timeo))    return NULL;
  if (read_uint64(fd, &mtime, timeo))   return NULL;
  if (read_uint16(fd, &pathlen, timeo)) return NULL;

  finfo *fi = alloc_finfo(pathlen);
  fi->mode = mode;
  fi->size = size;
  fi->mtime = mtime;
  fi->pathlen = pathlen;

  if (read_bytes(fd, fi->digest, DIGEST_BYTES, timeo)) {
    cleanup_finfo(fi);
    return NULL;
  }
  
  if (read_buf(fd, fi->relapath, pathlen, timeo)) {
    cleanup_finfo(fi);
    return NULL;
  }

  return fi;
}
		    
