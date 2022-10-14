
KPDEF = default.kpdef jdaemon.kpdef just.kpdef kaisoku.kpdef kana.kpdef \
        lan5.kpdef matsu.kpdef newjis.kpdef romaji.kpdef sokuon.kpdef \
        english.kpdef tut.kpdef vje.kpdef wx2+.kpdef

CBP   = default.cbp jdaemon.cbp just.cbp kaisoku.cbp kana.cbp \
        lan5.cbp matsu.cbp newjis.cbp romaji.cbp sokuon.cbp \
        english.cbp tut.cbp vje.cbp wx2+.cbp

# Automake では指定してはいけない.
#.SUFFIXES: .cbp .kpdef

FORCPP = ../../cmd/forcpp/forcpp
KPDIC = ../../cmd/kpdic/kpdic
ROMAJI_DIC_DEF = -DSHIFT

# gcc -E プリプロセスのみ実行
.kpdef.cbp:
	$(CPP) $(ROMAJI_DIC_DEF) - < $< | $(KPDIC) > $@

dist_pkgdata_DATA = $(CBP)
