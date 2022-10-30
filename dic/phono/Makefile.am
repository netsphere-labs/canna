
KPDEF = default.kpdef jdaemon.kpdef just.kpdef kaisoku.kpdef kana.kpdef \
        lan5.kpdef matsu.kpdef newjis.kpdef romaji.kpdef sokuon.kpdef \
        english.kpdef tut.kpdef vje.kpdef wx2+.kpdef

# .kp は旧形式.
CBP   = default.cbp jdaemon.cbp just.cbp kaisoku.cbp kana.cbp \
        lan5.cbp matsu.cbp newjis.cbp romaji.cbp sokuon.cbp \
        english.cbp tut.cbp vje.cbp wx2+.cbp

# Automake では指定してはいけない.
#.SUFFIXES: .cbp .kpdef

FORCPP = ../../cmd/forcpp/forcpp
KPDIC = ../../cmd/kpdic/kpdic
ROMAJI_DIC_DEF = -DSHIFT

.kpdef.cbp:
	$(CANNA_PP) $(ROMAJI_DIC_DEF) $< > t
	$(KPDIC) < t > $@

dist_pkgdata_DATA = $(CBP)

clean-local:
	rm -f *.cbp
