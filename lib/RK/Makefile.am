
noinst_LTLIBRARIES = libRK.la

libRK_la_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib
libRK_la_SOURCES = dicsw.c permdic.c tempdic.c ngram.c \
    context.c bun.c ncache.c \
    nword.c dic.c util.c kana.c \
    bits.c dd.c fq.c widechar.c
libRK_la_LDFLAGS = -static

man3_MANS = RkBgnBun.man RkDefineDic.man RkDeleteDic.man RkEndBun.man \
    RkEnlarge.man RkFinalize.man

