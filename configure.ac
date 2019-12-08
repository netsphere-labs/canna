#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])

# [FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS]
AC_INIT([canna],[3.7p3-q1],[hisashi dot horikawa at gmail dot com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([WHATIS.jp])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

############################
# Checks for programs.

AC_PROG_CXX
AC_PROG_CC
# AC_PROG_RANLIB
# AM_PROG_LIBTOOL is deprecated.
LT_INIT
AC_PROG_LN_S

############################
# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h locale.h memory.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
#AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_SETPGRP
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([dup2 gethostname inet_ntoa memchr memset mkdir munmap select setlocale socket strchr strrchr strspn strtol strtoul])

AC_CHECK_SIZEOF([void *])
AC_CHECK_SIZEOF(wchar_t)
AC_CHECK_MEMBERS([struct hostent.h_addr_list],,,[#include <netdb.h>])
AC_CONFIG_FILES([
  Makefile
  lib/RK/Makefile
lib/canna/Makefile
lib/Makefile
lib/RKindep/Makefile
lib/RKC/Makefile
cmd/dicar/Makefile
cmd/dpxdic/Makefile
cmd/crxgram/Makefile
cmd/cannastat/Makefile
cmd/forsort/Makefile
cmd/crfreq/Makefile
cmd/ctow/Makefile
cmd/wtoc/Makefile
cmd/splitwd/Makefile
cmd/dpbindic/Makefile
cmd/dpromdic/Makefile
cmd/mkromdic/Makefile
cmd/cannacheck/Makefile
cmd/mergewd/Makefile
cmd/Makefile
cmd/mkbindic/Makefile
cmd/cshost/Makefile
cmd/chkconc/Makefile
cmd/forcpp/Makefile
cmd/catdic/Makefile
cmd/crxdic/Makefile
cmd/kpdic/Makefile
cmd/canlisp/Makefile
canna/Makefile
doc/Makefile
doc/man/guide/Makefile
doc/man/guide/tex/Makefile
dic/phono/Makefile
dic/ideo/Makefile
dic/ideo/pubdic/Makefile
dic/ideo/grammar/Makefile
dic/ideo/words/Makefile
dic/Makefile
ccustom/Makefile
misc/initfiles/Makefile
misc/Makefile
server/Makefile
  ])

AC_CONFIG_SUBDIRS([canuum])
AC_OUTPUT
# $Id: configure.ac,v 1.12.2.5 2004/05/19 15:47:33 aida_s Exp $
