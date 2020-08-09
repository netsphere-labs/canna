
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -DPKGDATADIR='"$(pkgdatadir)"'

lib_LTLIBRARIES = libcanna16.la

# current:revision:age
# => *.so.(current-age).age.revision
libcanna16_la_LDFLAGS = -version-info 2:0:0
libcanna16_la_SOURCES = lisp.c defaultmap.c \
    bushu.c chikuji.c commondata.c empty.c henkan.c hex.c \
    ichiran.c jishu.c kctrl.c keydef.c \
    kigo.c mode.c multi.c onoff.c parse.c romaji.c \
    uiutil.c uldefine.c uldelete.c ulhinshi.c ulkigo.c \
    ulmount.c ulserver.c util.c yesno.c bunsetsu.c \
    engine.c RKroma.c jrbind.c ebind.c
#libcanna_la_LIBADD = ../RK/libRK.a
libcanna16_la_LIBADD = ../RKC/libRKC16.la

install-exec-hook:
	cd $(DESTDIR)$(libdir) && \
	$(LN_S) -f libcanna16.so.2 libcanna.so.2
