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

#include <stdio.h>

#ifndef _WRAP_SHA1_H_
#define _WRAP_SHA1_H_

#ifndef SHA1_BUF_LEN
#define SHA1_BUF_LEN 1024
#endif

#ifndef SHA1_STR_LEN
#define SHA1_STR_LEN 41
#endif

// sha1_digest refers to original sha1 digest of file's content
// hash_digest refers to sha1_checksum(sha1_digest+[relaname])

// if fullname doesn't exist, following 2 funtions will return ENOENT
int get_sha1(const char *fullname, char *digest, size_t size);
int get_hash(const char *fullname, const char *relaname,
	     char *sha1_digest, size_t sha1_size,
	     char *hash_digest, size_t hash_size);

int sha1_file(FILE *file, char *digest, size_t size);
int sha1_str(char *text, char *digest, size_t size); 




#endif

  
  
  
    
    
