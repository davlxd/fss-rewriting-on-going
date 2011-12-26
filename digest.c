/*
 * wrap functions of sha1.c
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

#include "sha1.h"
#include "digest.h"
#include "utils.h"
#include "log.h"
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <endian.h>
#include <errno.h>

extern int errno;

const char *digest2hex(const char *digest)
{
  int i;
  static char hexstr[DIGEST_STR_LEN];
  uint32_t *t = (uint32_t*)digest;

  for(i = 0; i < 5; i++) // `be32toh()' defined in endian.h
    snprintf(hexstr+8*i, 9, "%08x", be32toh(t[i]));

  return hexstr;
}

const char *hex2digest(const char *hexstr)
{
  int i;
  uint32_t tmp;
  static unsigned char digest[DIGEST_BYTES];

  for (i = 0;i < 5; i++) {
    sscanf(hexstr + 8*i, "%08x", &tmp); // `htobe32()' defined in endian.h
    *(uint32_t*)(digest + i*4) = htobe32(tmp);
  }
  
  return digest;
}

// cast the bytes at the end `digest' to uint64_t as hash key
uint64_t digest2hashkey(const char *digest, uint64_t mask)
{
  assert(digest);
  uint64_t keyspan = 0;
  
  size_t offset = DIGEST_BYTES > 8 ? (DIGEST_BYTES - 8) : 0;
  memcpy(&keyspan, digest+offset, min(8, DIGEST_BYTES));
  

  return be64toh(keyspan) & mask; // `be64toh()' defined in endian.h
}


unsigned char* file_digest(FILE* fp, unsigned char *digest)
{
  assert(fp);
  off_t save_pos;
  blk_SHA_CTX c;;
  int len;
  unsigned char buf[SHA1_BUF_LEN];

  save_pos = ftello(fp);
  if (save_pos)
    Log_die(DIE_FAILURE, LOG_ERR, "ftello() failed");

  blk_SHA1_Init(&c);

  fseeko(fp, 0, SEEK_SET);
  while((len = fread(buf, sizeof(unsigned char), SHA1_BUF_LEN, fp)) > 0)
    blk_SHA1_Update(&c, buf, len);

  blk_SHA1_Final(digest, &c);
  fseeko(fp, save_pos, SEEK_SET);

  return digest;
}

unsigned char* file_digest_name(const char *fullname, unsigned char *digest)
{
  FILE *fp = NULL;
  
  fp = fopen(fullname, "rb");
  if (!fp)
    Log_die(DIE_FAILURE, LOG_ERR, "fopen() %s failed", fullname);
    
  file_digest(fp, digest);
  fclose(fp);
  
  return digest;
}

unsigned char* str_digest(const char *text)
{
  assert(text);
  static unsigned char digest[DIGEST_BYTES];
  blk_SHA_CTX c;
  
  blk_SHA1_Init(&c);
  blk_SHA1_Update(&c, text, strlen(text));
  blk_SHA1_Final(digest, &c);

  return digest;
}

unsigned char *mem_digest(const void *addr, size_t size,
			  unsigned char *digest)
{
  assert(addr);
  blk_SHA_CTX c;

  blk_SHA1_Init(&c);
  blk_SHA1_Update(&c, addr, size);
  blk_SHA1_Final(digest, &c);

  return digest;
}


