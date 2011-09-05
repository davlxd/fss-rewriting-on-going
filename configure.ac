#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([fss], [2.0], [i@lxd.me])
AM_INIT_AUTOMAKE([fss], [2.0])
AC_CONFIG_SRCDIR([exit.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h netdb.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h syslog.h])

if test "$prefix" = "NONE"; then
  prefix="/usr/local"
fi

AC_DEFINE_UNQUOTED(SYSCONFDIR, "$(eval echo ${sysconfdir})",
    [Location of configuration files])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_FSEEKO
AC_CHECK_FUNCS([strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
