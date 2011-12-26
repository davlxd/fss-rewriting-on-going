 #
 # fss - File Synchronization System , detect file changes under
 #       specific directory on one client then synchronize to other
 #       client via a centralized server
 #
 # Copyright (c) 2010, 2011 lxd <i@lxd.me>
 # 
 # This file is part of File Synchronization System(fss).
 #
 # fss is free software: you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation, either version 3 of the License, 
 # (at your option) any later version.
 #
 # fss is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 # You should have received a copy of the GNU General Public License
 # along with fss.  If not, see <http://www.gnu.org/licenses/>.
 #

bin_PROGRAMS = fss

dist_doc_DATA = README COPYING

dist_sysconf_DATA = fss.conf

AUTOMAKE_OPTIONS = foreign

fss_SOURCES = \
	blist.c blist.h \
	client.c client.h \
	core.c core.h \
	digest.c digest.h \
	exit.c exit.h \
	flist.c flist.h \
	fss.c fss.h \
	hashtable.c hashtable.h \
	io.c io.h \
	log.c log.h \
	options.c options.h \
	path.c path.h \
	ptrstack.c ptrstack.h \
	server.c server.h \
	setsignal.c setsignal.h \
	sha1.c sha1.h \
	sock.c sock.h \
	utils.c utils.h \
	wrap-inotify.c wrap-inotify.h




