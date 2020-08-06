
# libRKC は libcanna に組み込まれる。libRK と同時に使われることはない.

lib_LTLIBRARIES = libRKC16.la

libRKC16_la_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -DCPP='"$(CPP)"'
# current:revision:age
# => *.so.(current-age).age.revision
libRKC16_la_LDFLAGS = -version-info 2:0:0
libRKC16_la_SOURCES = rkc.c convert.c wconvert.c ../RK/widechar.c conf.c ../RKindep/cfuncs.c ../RKindep/file.c ../RKindep/strops.c ../RK/kana.c
