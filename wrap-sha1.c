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

#include "fss.h"
#include "sha1.h"
#include "wrap-sha1.h"

extern int errno;

static int export_to_str(SHA1Context *sha1, char *str, size_t size);


int sha1_file(FILE *file, char *digest, size_t size)
{
  off_t pos;
  SHA1Context sha;
  int len;
  unsigned char buf[SHA1_BUF_LEN];

  pos = ftello(file);
  if (pos < 0) {
    perror("@sha1_file(): ftello failed");
    return 1;
  }
  
  SHA1Reset(&sha);
  
  len = fread(buf, sizeof(unsigned char), SHA1_BUF_LEN, file);
  while(len) {
    SHA1Input(&sha, buf, len);
    len = fread(buf, sizeof(unsigned char), SHA1_BUF_LEN, file);
  }

  SHA1Result(&sha);

  export_to_str(&sha, digest, size);
  fseeko(file, pos, SEEK_SET);
  
  return 0;
}

int sha1_str(char *text, char *digest, size_t size)
{
  SHA1Context sha;
  
  SHA1Reset(&sha);
  SHA1Input(&sha, (const unsigned char*)text, strlen(text));
  if (!SHA1Result(&sha)) {
    fprintf(stderr,
	    "@sha1_str(): SHA1Result() fails.\n");
    return 1;
  }
  export_to_str(&sha, digest, size);
  
  return 0;
}


int get_sha1(const char *fullname, char *digest, size_t size)
{
  SHA1Context sha;
  struct stat sb;
  FILE *file;

  if (stat(fullname, &sb) < 0) {
    if (errno == ENOENT)
      return ENOENT;
    else {
      perror("@get_sha1(): stat() failed");
      return 1;
    }
  }

  // Normally it's DIR
  if (!S_ISREG(sb.st_mode)) {
    SHA1Reset(&sha);
    SHA1Result(&sha);
    export_to_str(&sha, digest, size);
    return 0;
  }
  
  if (!(file = fopen(fullname, "rb"))) {
    perror("@get_sha1(): fopen() failed");
    return 1;
  }
  
  if (sha1_file(file, digest, size)) {
    fprintf(stderr, "@get_sha1(): sha1_file() failed\n");
    return 1;
  }
  
  fclose(file);
  return 0;

}

int get_hash(const char *fullname, const char *relaname,
	     char *sha1_digest, size_t sha1_size, 
	     char *hash_digest ,size_t hash_size)
{
  SHA1Context sha;
  char content_digest[SHA1_STR_LEN];
  char path_digest[SHA1_STR_LEN];
  char both_digest[2*SHA1_STR_LEN-1];
  char relaname_arr[MAX_PATH_LEN];

  if (get_sha1(fullname, content_digest, SHA1_STR_LEN)) {
    if (errno == ENOENT)
      return ENOENT;
    else {
      fprintf(stderr, "@get_hash(): get_sha1() failed\n");
      return 1;
    }
  }

  // if sha1 digest is asked
  if (sha1_digest){
    if (!strncpy(sha1_digest, content_digest, sha1_size-1)) {
      perror("get_hash(): strncpy() failed");
      return 1;
    }
    sha1_digest[sha1_size-1] = 0;
  }
    
  strncpy(relaname_arr, relaname, strlen(relaname));
  relaname_arr[strlen(relaname)] = 0;

  // if hash digest is asked
  if (relaname && hash_digest) {
    char *token;
    token = strtok(relaname_arr, "/");
  
    while(token) {
      printf(">>>> current token is --%s--\n", token);
      SHA1Reset(&sha);
      SHA1Input(&sha, (unsigned char *)token, strlen(token));
      SHA1Result(&sha);

      export_to_str(&sha, path_digest, SHA1_STR_LEN);

      strncpy(both_digest, content_digest, SHA1_STR_LEN-1);
      strncpy(both_digest+SHA1_STR_LEN-1, path_digest, SHA1_STR_LEN-1);
      both_digest[2*SHA1_STR_LEN-2] = 0;

      SHA1Reset(&sha);
      SHA1Input(&sha, (unsigned char *)both_digest, 2*SHA1_STR_LEN-2);
      SHA1Result(&sha);

      export_to_str(&sha, content_digest, SHA1_STR_LEN);
      token = strtok(NULL, "/");
    }

    if (!strncpy(hash_digest, content_digest, hash_size-1)) {
      perror("@get_hash(): strncpy() failed");
      return 1;
    }
    
    hash_digest[hash_size-1] = 0;
  }
  
  return 0;
}


static int export_to_str(SHA1Context *sha1, char *str, size_t size)
{
  int i;
  char temp[SHA1_STR_LEN];

  if (size < 2)
    return 0;
  
  for (i = 0; i < 5 && 8*i < size; i++)
    snprintf(temp+8*i, 8+1, "%08X", sha1->Message_Digest[i]);
      
  temp[SHA1_STR_LEN-1] = 0;

  strncpy(str, temp, size-1);
  str[size-1] = 0;

  return 0;
}
  
  
