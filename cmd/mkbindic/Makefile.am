
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

bin_PROGRAMS = mkbindic$(EXEEXT)

mkbindic$(EXEEXT): mkbindic.cpp
	$(CANNA_PP) $< -o $@ -I../..
