
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

#bin_PROGRAMS = dpbindic

#dpbindic_SOURCES = dpbindic.cpp

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	  $(LN_S) -f dpxdic$(EXEEXT) dpbindic$(EXEEXT)
