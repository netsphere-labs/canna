
bin_PROGRAMS = catdic 

AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

catdic_SOURCES = can.c rutil.c RKdelline.c
catdic_LDADD = ../../lib/RKC/libRKC.la

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	$(LN_S) catdic$(EXEEXT) addwords$(EXEEXT) && \
	$(LN_S) catdic$(EXEEXT) chmoddic$(EXEEXT) && \
	$(LN_S) catdic$(EXEEXT) cpdic$(EXEEXT) && \
	$(LN_S) catdic$(EXEEXT) delwords$(EXEEXT) && \
	$(LN_S) catdic$(EXEEXT) lsdic$(EXEEXT) && \
	$(LN_S) catdic$(EXEEXT) mkdic$(EXEEXT) && \
	$(LN_S) catdic$(EXEEXT) mvdic$(EXEEXT) && \
	$(LN_S) catdic$(EXEEXT) rmdic$(EXEEXT) && \
	$(LN_S) catdic$(EXEEXT) syncdic$(EXEEXT) && \
	cd $(DESTDIR)$(sbindir) && \
	$(LN_S) ../bin/catdic$(EXEEXT) cannakill$(EXEEXT)

