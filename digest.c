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

#include "sha1.h"
#include "digest.h"
#include "log.h"
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
extern int errno;

static char* export_to_str(SHA1Context *sha1, char *digest, size_t size)
{
  int i;
  char temp[SHA1_STR_LEN];

  if (size < 2)
    return 0;
  
  for (i = 0; i < 5 && 8*i < size; i++)
    snprintf(temp+8*i, 8+1, "%08x", sha1->Message_Digest[i]);
      
  temp[SHA1_STR_LEN-1] = 0;

  strncpy(digest, temp, size-1);
  digest[size-1] = 0;
  
  return digest;
}

char* file_digest(FILE* fp, char *digest, size_t size)
{
  assert(fp);
  off_t save_pos;
  SHA1Context sha;
  int len;
  unsigned char buf[SHA1_BUF_LEN];

  save_pos = ftello(fp);
  if (save_pos)
    Log_die(DIE_FAILURE, LOG_ERR, "ftello() failed");

  SHA1Reset(&sha);

  fseeko(fp, 0, SEEK_SET);
  while(len = fread(buf, sizeof(unsigned char), SHA1_BUF_LEN, fp))
    SHA1Input(&sha, buf, len);

  SHA1Result(&sha);
  fseeko(fp, save_pos, SEEK_SET);

  return export_to_str(&sha, digest, size);
}

char* file_digest_name(const char *fullname, char *digest, size_t size)
{
  FILE *fp = NULL;
  
  fp = fopen(fullname, "rb");
  if (!fp)
    Log_die(DIE_FAILURE, LOG_ERR, "fopen() %s failed", fullname);
    
  file_digest(fp, digest, size);
  fclose(fp);
  
  return digest;
}

char* str_digest(const char *text, char* digest, size_t size)
{
  assert(text);
  SHA1Context sha;
  SHA1Reset(&sha);

  SHA1Input(&sha, (const unsigned char*)text, strlen(text));
  if (!SHA1Result(&sha))
    Log_die(DIE_FAILURE, LOG_ERR, "SHA1Result() failed");

  return export_to_str(&sha, digest, size);
}

/* int get_sha1(const char *fullname, char *digest, size_t size) */
/* { */
/*   SHA1Context sha; */
/*   struct stat sb; */
/*   FILE *file; */

/*   if (stat(fullname, &sb) < 0) { */
/*     if (errno == ENOENT) */
/*       return ENOENT; */
/*     else { */
/*       perror("@get_sha1(): stat() failed"); */
/*       return 1; */
/*     } */
/*   } */

/*   // Normally it's DIR */
/*   if (!S_ISREG(sb.st_mode)) { */
/*     SHA1Reset(&sha); */
/*     SHA1Result(&sha); */
/*     export_to_str(&sha, digest, size); */
/*     return 0; */
/*   } */
  
/*   if (!(file = fopen(fullname, "rb"))) { */
/*     perror("@get_sha1(): fopen() failed"); */
/*     return 1; */
/*   } */
  
/*   if (sha1_file(file, digest, size)) { */
/*     fprintf(stderr, "@get_sha1(): sha1_file() failed\n"); */
/*     return 1; */
/*   } */
  
/*   fclose(file); */
/*   return 0; */

/* } */
