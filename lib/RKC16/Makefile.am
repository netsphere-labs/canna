
lib_LTLIBRARIES = libRKC16.la

libRKC16_la_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -DCPP='"$(CPP)"' -DCANNA_WCHAR16
# current:revision:age
# => *.so.(current-age).age.revision
libRKC16_la_LDFLAGS = -version-info 3:1:2
libRKC16_la_SOURCES = ../RKC/rkc.c ../RKC/convert.c ../RKC/wconvert.c ../RKC/wutil.c ../RKC/conf.c ../RKindep/cfuncs.c ../RKindep/file.c ../RKindep/strops.c
