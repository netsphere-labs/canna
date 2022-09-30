
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

bin_PROGRAMS = mkbindic$(EXEEXT)

mkbindic$(EXEEXT): mkbindic.cpp
	$(CPP) $< -o $@ -I../..
