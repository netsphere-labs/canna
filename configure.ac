#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# mingw: autoconf 2.71
# Fedora 37: autoconf 2.71
AC_PREREQ([2.71])

# [FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS]
AC_INIT([canna],[3.7p3-q1],[hisashi dot horikawa at gmail dot com])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([WHATIS.jp])
# Manual によれば <code>-W<var>category</var></code> が書けるはずだが, 反映されない.
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

############################
# Checks for programs.

AC_PROG_CXX([clang++ g++ c++])
AC_PROG_CC([clang gcc cc])
AC_PROG_CPP
AM_PROG_AR
LT_INIT
AC_PROG_LN_S

############################
# Checks for libraries.

# Checks for header files.
# The preprocessor macro `STDC_HEADERS' is obsolete.
# 標準ヘッダは確認しない.
# AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h locale.h  netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h   sys/socket.h sys/time.h syslog.h unistd.h])
AC_CHECK_HEADERS([memory.h sys/ioctl.h sys/param.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_HEADERS_ONCE([sys/time.h])

AC_STRUCT_TM
#AC_TYPE_INT16_T
#AC_TYPE_INT32_T
#AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SSIZE_T
#AC_TYPE_UINT16_T
#AC_TYPE_UINT32_T
#AC_TYPE_UINT8_T

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

# BSD only
AC_CHECK_FUNCS([strlcpy strlcat vsyslog initgroups bzero])

AC_CHECK_SIZEOF([void *])
AC_CHECK_SIZEOF(wchar_t)
AC_CHECK_MEMBERS([struct hostent.h_addr_list],,,[#include <netdb.h>])

AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    native_win32=yes
    LDFLAGS="$LDFLAGS -no-undefined"
    ;;
  *)
    native_win32=no
    ;;
esac
AC_MSG_RESULT([$native_win32])
AM_CONDITIONAL(OS_WIN32, test "$native_win32" = "yes")

# CPP は C言語用の処理が多い。最小限の処理だけをさせる.
# gcc -E プリプロセスのみ実行ではない. アカン
# cpp -fdirectives-only -E .. directives such as "#define", "#ifdef", and
#                             "#error" のみ処理
#     -nostdinc
AS_CASE(["$CC"],
  [clang*], [
    AC_SUBST(CANNA_PP, "LANG=C /usr/bin/cpp -E -nostdinc") ],
  [gcc*], [
    AC_SUBST(CANNA_PP, "LANG=C /usr/bin/cpp -E -fdirectives-only -nostdinc")
    # ICE with -fstack-clash-protection. gcc 10.2 on mingw.
    # -Wformat=2 includes -Wformat-security
    CFLAGS="$CFLAGS -std=c11 -Wall -Wextra -Wshadow -Wformat=2 -Wformat-truncation -Wundef -fno-common -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wno-unused-parameter -fstack-protector-strong "  
    ],
  [AC_MSG_ERROR([Unknown C compiler.])]
  )

AC_ENABLE_STATIC(no)


dnl ----------------------------------------------------------------------------
dnl setup various package/compilation settings
dnl ----------------------------------------------------------------------------

# <kbd>autoheader</kbd> uses `AH_TEMPLATE` to generate `config.h.in`. In older releases, <kbd>autoheader</kbd> copied from `acconfig.h`.
AH_TEMPLATE(LOCALSTATE_DIR, [The localstate directory.])
# $ac_default_prefix=/usr/local; $prefix=NONE; $localstatedir='${prefix}/var'
#  => 単に $prefix を使えない. うーむ.
if test "x${localstatedir}" = 'x${prefix}/var'; then
  if test "x${prefix}" = "xNONE"; then
    LOCALSTATE_DIR="${ac_default_prefix}/var"
  else
    LOCALSTATE_DIR="${prefix}/var"
  fi
else
  LOCALSTATE_DIR="${localstatedir}"
fi
# `config.h` に値を埋め込むには, `AC_DEFINE()`.
AC_DEFINE_UNQUOTED(LOCALSTATE_DIR, "${LOCALSTATE_DIR}")


# AS_IF(), AS_CASE() などは, autoconf が提供する, シェル制御構造のﾎﾟｰﾀﾌﾞﾙな代替.
# ソケットファイル: Linux = /run 以下, FreeBSD = /var/run
AC_ARG_ENABLE([unix-socket],
  AS_HELP_STRING([--disable-unix-socket],
      [Disable UNIX domain socket. If use, the argument is the directory they are in. (default:${LOCALSTATE_DIR}/run/canna)]))
AS_IF([test "x$enable_unix_socket" != "xno" -a "x$native_win32" = "xno"], [
  AC_DEFINE(USE_UNIX_SOCKET, 1, [Enable UNIX domain socket.])
  AS_IF([test "x$enable_unix_socket" = "x"], [
    AC_DEFINE_UNQUOTED(IR_UNIX_DIR, ["${LOCALSTATE_DIR}/run/canna"], [UNIX domain socket directory.])
  ], [
    AC_DEFINE_UNQUOTED(IR_UNIX_DIR, ["$enable_unix_socket"])
  ])
])

AC_DEFINE(USE_INET_SOCKET, 1, [Enable IPv4/IPv6 socket.])


dnl Set PACKAGE_SYSCONF_DIR in config.h.
AH_TEMPLATE(PACKAGE_SYSCONF_DIR, [Configuration directory of the package.])
# --sysconfdir=DIR        read-only single-machine data [PREFIX/etc]
#   => 実際に保存するのは /etc/canna 以下.
if test "x${sysconfdir}" = 'x${prefix}/etc'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_SYSCONF_DIR, "${ac_default_prefix}/etc/${PACKAGE}")
  else
    AC_DEFINE_UNQUOTED(PACKAGE_SYSCONF_DIR, "${prefix}/etc/${PACKAGE}")
  fi
else
  AC_DEFINE_UNQUOTED(PACKAGE_SYSCONF_DIR, "${sysconfdir}/${PACKAGE}")
fi

dnl Set PACKAGE_DATA_DIR in config.h.
dnl Makefile.am ファイルで次のようにしてもよいが, 定義を1ヶ所に集める.
dnl     AM_CFLAGS = ... -DPKGDATADIR='"$(pkgdatadir)"'
AH_TEMPLATE(PACKAGE_DATA_DIR, [Data directory of the package.])
if test "x${datadir}" = 'x${prefix}/share'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}")
  else
    AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}")
  fi
else
  # デフォルト値は ${datarootdir}. もう一段展開が必要.
  if test "x${datadir}" = 'x${datarootdir}'; then
    if test "x${datarootdir}" = 'x${prefix}/share'; then
      if test "x${prefix}" = "xNONE"; then
        AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/${PACKAGE}")
      else
        AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/${PACKAGE}")
      fi
    else
      AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${datarootdir}/${PACKAGE}")
    fi
  else
    AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${datadir}/${PACKAGE}")
  fi
fi


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

#AC_CONFIG_SUBDIRS([canuum])
AC_OUTPUT
# $Id: configure.ac,v 1.12.2.5 2004/05/19 15:47:33 aida_s Exp $
