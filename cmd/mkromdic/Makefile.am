
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

bin_PROGRAMS = mkromdic$(EXEEXT)

mkromdic$(EXEEXT): mkromdic.cpp
	$(CANNA_PP) $< -o $@ -I../..

