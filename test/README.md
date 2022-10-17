

## cmd/canlisp

LISPインタプリタ.

課題:
`lambda` はあるのに `funcall` がない。`doc/lisp/canlisp.tex` にあるとおり。




## ローマ字かなテーブル

ローマ字かなだけではなく、カナ配列, TUT-Code もこのレイヤー.

### cmd/forcpp 

プリプロセッサに通すために, テキストを 7bit <-> 8bit 変換する。しかし, そもそも 8bit clean なプリプロセッサなら、これを通す必要がない。
Round-trip を確認. Checked.


### cmd/kpdic, cmd/dpromdic

 * kpdic - テキストの .kpdef ファイル -> バイナリの .cbp ファイル. 
 * dpromdic - バイナリ .cbp ファイルをテキストに戻す.

バイナリ -> テキスト -> バイナリで round-trip. Checked.

ドキュメントにある .kp ファイルは古い形式で、促音「っ」が決め打ちで使えない。


### cmd/mkromdic

シェルスクリプトで, `forcpp`, プリプロセッサ, `kpdic` を呼び出して, バイナリファイルに変換する。
全然まともに動かない. ●未了




## cmd/cannacheck

ローマ字かな変換テーブル(default.cbp)がオープンできません。
RKCの設定ファイルがありません
かな漢字変換サーバと通信できません

