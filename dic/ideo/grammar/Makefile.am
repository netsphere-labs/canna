
cpp.gram: gram.code gram.goto main.code
	$(FORCPP) -7 < gram.code | \
		$(CPP) $(DEFINES) | \
		$(FORCPP) -8 > cpp.gram

cnj.swd cnj.bits cnj.h cnj.mac: cpp.gram 
		$(CRXGRAM) -f cpp.gram

fuzokugo.swd: cnj.swd
	$(FORSORT) -7 < cnj.swd | \
		$(SORT) | \
		$(FORSORT) -8 | \
		$(MERGE) > fuzokugo.swd

fuzokugo.cbd: fuzokugo.swd cnj.bits
	$(CRXDIC) $(COMPATFLAG) -g -s -o fuzokugo.cbd -D cnj.bits -n fuzokugo fuzokugo.swd

