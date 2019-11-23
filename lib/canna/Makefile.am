
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

lib_LTLIBRARIES = libcanna.la

libcanna_la_LDFLAGS = -version-info 3:1:2
libcanna_la_SOURCES = lisp.c defaultmap.c \
    bushu.c chikuji.c commondata.c empty.c henkan.c hex.c \
    ichiran.c jishu.c kctrl.c keydef.c \
    kigo.c mode.c multi.c onoff.c parse.c romaji.c \
    uiutil.c uldefine.c uldelete.c ulhinshi.c ulkigo.c \
    ulmount.c ulserver.c util.c yesno.c bunsetsu.c \
    engine.c RKroma.c RKkana.c obind.c
libcanna_la_LIBADD = ../RK/libRK.a
