#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])

# [FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS]
AC_INIT([canna],[3.7p3-q1],[hisashi dot horikawa at gmail dot com])
AC_CONFIG_SRCDIR([WHATIS.jp])
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

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
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_SETPGRP
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([bzero dup2 gethostbyaddr gethostbyname gethostname inet_ntoa memchr memset mkdir munmap select setlocale socket strchr strdup strrchr strspn strtol strtoul])

AC_CHECK_SIZEOF([void *])
AC_CHECK_MEMBERS([struct hostent.h_addr_list],,,[#include <netdb.h>])
AC_CONFIG_FILES([
lib/RK/Makefile
lib/canna/Makefile
lib/RKC16/Makefile
lib/Makefile
lib/RKindep/Makefile
lib/RKC/Makefile
lib/canna16/Makefile
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
Makefile
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

dnl PUBCONF_DEFINE(NAME,VALUE)
define([PUBCONF_PREFIX],[CANNA_])
AC_DEFUN([PUBCONF_DEFINE],[dnl
cat << PUBCONF_EOF >> "$pubconf_tmp"
[#]define PUBCONF_PREFIX[$1] [$2]
PUBCONF_EOF[]dnl
])
dnl PUBCONF_NOTDEF(NAME)
AC_DEFUN([PUBCONF_NOTDEF],[dnl
echo "/* [#]undef PUBCONF_PREFIX[$1] */" >> "$pubconf_tmp"[]dnl
])
dnl PUBCONF_DEFINE_IF(NAME,COND)
AC_DEFUN([PUBCONF_DEFINE_IF],[dnl
if [$2]; then
  PUBCONF_DEFINE([$1],1)
else
  PUBCONF_NOTDEF([$1])
fi
])
dnl PUBCONF_DEFINE_YESNO(NAME,VAR)
AC_DEFUN([PUBCONF_DEFINE_YESNO],[PUBCONF_DEFINE_IF([$1],[test "x$][$2" = xyes])])

AC_CONFIG_COMMANDS([pubconf.h],[
pubconf_hdr="pubconf.h"
pubconf_tmp="$pubconf_hdr.tmp"
AC_MSG_NOTICE([creating "$pubconf_hdr"])

cat << PUBCONF_EOF > "$pubconf_tmp"
/* Generated file. Modify "configure.ac" instead of this file. */
#ifndef CANNA_PUBCONF_H
#define CANNA_PUBCONF_H

PUBCONF_EOF

PUBCONF_DEFINE_YESNO([HAVE_INTTYPES_H],[pubconf_header_inttypes_h])
PUBCONF_DEFINE_YESNO([HAVE_STDINT_H],[pubconf_header_stdint_h])
PUBCONF_DEFINE_YESNO([HAVE_INT32_T],[pubconf_type_int32_t])

cat << PUBCONF_EOF >> "$pubconf_tmp"

#endif /* CANNA_PUBCONF_H */
PUBCONF_EOF

if cmp -s "$pubconf_hdr" "$pubconf_tmp"; then
  AC_MSG_NOTICE(["$pubconf_hdr" is unchanged])
  rm "$pubconf_tmp"
else
  mv "$pubconf_tmp" "$pubconf_hdr"
  fi
],[
  m4_foreach([Var],[header_inttypes_h,header_stdint_h,type_int32_t],[dnl
    [pubconf_]Var="[$ac_cv_]Var"
  ])
])

AC_CONFIG_SUBDIRS([canuum])
AC_OUTPUT
# $Id: configure.ac,v 1.12.2.5 2004/05/19 15:47:33 aida_s Exp $
