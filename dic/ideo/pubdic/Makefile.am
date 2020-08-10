
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib

# iroha.t はインストールしない
noinst_DATA = iroha.t

noinst_PROGRAMS = pod

pod_SOURCES = pod.c
pod_LDADD = ../../../lib/RK/libRK.la ../../../lib/RKindep/libRKindep.la

iroha.t: $(PUBDICS) ProgramTargetName(pod)
	cat $(PUBDICS) | ./pod - -p -i -2 > iroha.spl
	$(MERGE) < iroha.spl > iroha.t
	$(RM) iroha.spl

