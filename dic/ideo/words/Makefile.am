
iroha.cbd iroha.cld: $(pubdicDir)/iroha.t bushu.cbd $(FUZOKUGODIC)
	$(CRXDIC) $(COMPATFLAG) -D $(FUZOKUGODIC) -m -o iroha.cbd -n iroha $(pubdicDir)/iroha.t
	$(CRFREQ) -div 512 -o iroha.cld iroha.cbd iroha.mwd
	cat bushu.cbd >> iroha.cbd

bushu.cbd bushu.cld: bushu.t $(FUZOKUGODIC)
	$(CRXDIC) $(COMPATFLAG) -D $(FUZOKUGODIC) -m -o bushu.cbd -n bushu bushu.t
	$(CRFREQ) -o bushu.cld bushu.cbd bushu.mwd

