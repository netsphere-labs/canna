
lib_LTLIBRARIES = libRKC.la

libRKC_la_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -DCPP='"$(CPP)"'
# current:revision:age
# => *.so.(current-age).age.revision
libRKC_la_LDFLAGS = -version-info 3:1:2   
libRKC_la_SOURCES = rkc.c convert.c wconvert.c wutil.c conf.c ../RKindep/cfuncs.c ../RKindep/file.c ../RKindep/strops.c
