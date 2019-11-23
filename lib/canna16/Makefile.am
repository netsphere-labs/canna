
lib_LTLIBRARIES = libcanna16.la

libcanna16_la_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -DCANNA_WCHAR16
libcanna16_la_LDFLAGS = -version-info 3:1:2
libcanna16_la_SOURCES = ../canna/lisp.c ../canna/defaultmap.c \
    ../canna/bushu.c ../canna/chikuji.c ../canna/commondata.c ../canna/empty.c ../canna/henkan.c ../canna/hex.c \
    ../canna/ichiran.c ../canna/jishu.c ../canna/kctrl.c ../canna/keydef.c \
    ../canna/kigo.c ../canna/mode.c ../canna/multi.c ../canna/onoff.c ../canna/parse.c ../canna/romaji.c \
    ../canna/uiutil.c ../canna/uldefine.c ../canna/uldelete.c ../canna/ulhinshi.c ../canna/ulkigo.c \
    ../canna/ulmount.c ../canna/ulserver.c ../canna/util.c ../canna/yesno.c ../canna/bunsetsu.c \
    ../canna/engine.c ../canna/RKroma.c ../canna/RKkana.c ../canna/obind.c
libcanna16_la_LIBADD = ../RK/libRK.a
