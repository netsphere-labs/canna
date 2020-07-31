
bin_PROGRAMS = catdic 

AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

catdic_SOURCES = can.c rutil.c RKdelline.c
catdic_LDADD = ../../lib/RKC/libRKC.la

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	$(LN_S) -f catdic$(EXEEXT) addwords$(EXEEXT) && \
	$(LN_S) -f catdic$(EXEEXT) chmoddic$(EXEEXT) && \
	$(LN_S) -f catdic$(EXEEXT) cpdic$(EXEEXT) && \
	$(LN_S) -f catdic$(EXEEXT) delwords$(EXEEXT) && \
	$(LN_S) -f catdic$(EXEEXT) lsdic$(EXEEXT) && \
	$(LN_S) -f catdic$(EXEEXT) mkdic$(EXEEXT) && \
	$(LN_S) -f catdic$(EXEEXT) mvdic$(EXEEXT) && \
	$(LN_S) -f catdic$(EXEEXT) rmdic$(EXEEXT) && \
	$(LN_S) -f catdic$(EXEEXT) syncdic$(EXEEXT) && \
	cd $(DESTDIR)$(sbindir) && \
	$(LN_S) -f ../bin/catdic$(EXEEXT) cannakill$(EXEEXT)

