/*
 * fss - File Synchronization System , detect file changes under
 *       specific directory on one client then synchronize to other
 *       client via a centralized server
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

#ifndef _F_SS_H_
#define _F_SS_H_

#include <limits.h>

#ifdef	PATH_MAX
#define MAX_PATH_LEN		PATH_MAX
#else
#define MAX_PATH_LEN		1024
#endif

#define DEPTH_OF_NFTW           10

#define BUF_LEN			4096
#define LISTEN_BACKLOG          10
#define MAX_CLIENTS             10
#define POLL_TIMEOUT            (1000 * 60)

// Default configuration
#define DEFAULT_BLOCK_LEN	700
#define DEFAULT_PORT		3375
#define DEFAULT_FSS_DIR		"." PACKAGE_TARNAME
#define DEFAULT_TRASH_DIR       ".trash"

// retry seconds if cannot establish tcp connect
#define DEFAULT_RETRY_SEC       5
// alarm timeout for block syscall read()/write()
#define DEFAULT_TIMEOUT         5


#endif
