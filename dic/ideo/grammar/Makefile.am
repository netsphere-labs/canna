
# localstatedir = ${prefix}/var
dicdir = ${localstatedir}/canna/dic
dist_dic_DATA = fuzokugo.cbd

SORT = sort

CANNACMDDIR = ../../../cmd
CRXGRAM = $(CANNACMDDIR)/crxgram/crxgram
CRXDIC = $(CANNACMDDIR)/crxdic/crxdic
CRFREQ = $(CANNACMDDIR)/crfreq/crfreq
MERGE = $(CANNACMDDIR)/mergewd/mergeword
FORSORT = $(CANNACMDDIR)/forsort/forsort
FORCPP = $(CANNACMDDIR)/forcpp/forcpp

cpp.gram: gram.code gram.goto main.code
	$(FORCPP) -7 < $<  | \
		$(CPP) $(DEFINES) -I$(top_srcdir)/dic/ideo/grammar - | \
		$(FORCPP) -8 > $@

cnj.swd cnj.bits cnj.h cnj.mac: cpp.gram 
		$(CRXGRAM) -f $< 

fuzokugo.swd: cnj.swd
	$(FORSORT) -7 < $<  | \
		$(SORT) | \
		$(FORSORT) -8 | \
		$(MERGE) > $@

fuzokugo.cbd: fuzokugo.swd cnj.bits
	$(CRXDIC) $(COMPATFLAG) -g -s -o $@ -D cnj.bits -n fuzokugo $<

