
sbin_PROGRAMS = cannaserver

AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

cannaserver_LDADD = ../lib/RK/libRK.la ../lib/RKindep/libRKindep.la
cannaserver_SOURCES = comm.c session.c convert.c wconvert.c main.c \
    misc.c util.c

