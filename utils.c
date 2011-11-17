/*
 * Utility routines
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

#include "utils.h"
#include "log.h"
#include <stdio.h>
#include <assert.h>


// I meant to implement following 2 functions as macro, but ...
bool streq(const char *s0, const char *s1)
{
  assert(s0 && s1);
  return !strcmp(s0, s1);
}

bool strprefix(const char *pre, const char *str)
{
  assert(pre && str);
  return !strncmp(pre, str, strlen(pre));
}

mode_t get_proc_umask()
{
  mode_t mask = umask(0);
  umask(mask);
  return mask;
}


uint16_t str2uint16(const char *str)
{
  uint16_t u16 = 0;
  
  if (sscanf(str, "%" SCNu16, &u16) == EOF)
    Log_die(DIE_FAILURE, LOG_ERR, "sscanf(%s) to u16t failed", str);
  return u16;
}

uint32_t str2uint32(const char *str)
{
  uint32_t u32 = 0;

  if (sscanf(str, "%" SCNu32, &u32) == EOF)
    Log_die(DIE_FAILURE, LOG_ERR, "sscanf(%s) to u32t failed", str);
  return u32;
}

uint64_t str2uint64(const char *str)
{
  uint64_t u64 = 0;

  if (sscanf(str, "%" SCNu64, &u64) == EOF)
    Log_die(DIE_FAILURE, LOG_ERR, "sscanf(%s) to u64t failed", str);
  return u64;
}

int uint642str(uint64_t u64, char *buf, size_t sz)
{
  int rv = 0;
  
  if ((rv = snprintf(buf, sz, "%" PRIu64, u64)) < 0)
    Log_die(DIE_FAILURE, LOG_ERR, "snprintf() failed");
  
  return rv;
}

mode_t str2modet(const char *str)
{
  return (mode_t)str2uint32(str);
}

int64_t str2int64(const char *str)
{
  int64_t i64 = 0;
  
  if (sscanf(str, "%" SCNd64, &i64) == EOF)
    Log_die(DIE_FAILURE, LOG_ERR, "sscanf() failed");

  return i64;
}


/* time_t str2timet(const char *str) */
/* { */
/*   return (time_t)str2int64(str); */
/* } */


/* char* timet2str(time_t t, char *buf, size_t sz) */
/* { */
/*   int num = 0; */
  
/*   //TODO: This is ugly */
/*   if (sizeof(time_t) == 4) { */
/*     if ((num = snprintf(buf, "%" PRId32, t)) < 0) */
/*       Log_die(DIE_FAILURE, LOG_ERR, "snprintf() failed"); */
    
/*   } else if (sizeof(time_t) == 8) { */
/*     if ((num = snprintf(buf, "%" PRId64, t)) < 0) */
/*       Log_die(DIE_FAILURE, LOG_ERR, "snprintf() failed"); */
    
/*   } else */
/*     Log_die(DIE_FAILURE, LOG_ERR, "unknown time_t size %d", sizeof(time_t)); */

/*   return buf + num; */
/* } */

/* char* offt2str(off_t off, char *buf, size_t sz) */
/* { */
/*   int num = 0; */

/*   //TODO: dirty */
/*   if (sizeof(off_t) == 4) { */
/*     if ((num = snprintf(buf, "%" PRId32, off)) < 0) */
/*       Log_die(DIE_FAILURE, LOG_ERR, "snprintf() failed"); */

/*   } else if (sizeof(off_t) == 8) { */
/*     if ((num = snprintf(buf, "%" PRId64, off)) < 0) */
/*       Log_die(DIE_FAILURE, LOG_ERR, "snprintf() failed"); */

/*   } else */
/*     Log_die(DIE_FAILURE, LOG_ERR, "unkonw off_t size %d", sizeof(off_t)); */

/*   return buf+num; */
/* } */
  
      
  
  
bool verify_dir(const char *p)
{
  struct stat sb;
  if (stat(p, &sb) < 0 || !S_ISDIR(sb.st_mode))
    return false;
  return true;
}

bool verify_addr(const char *addr)
{
  return false;
}
