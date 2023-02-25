

## cmd/canlisp

LISPインタプリタ.

課題:
`lambda` はあるのに `funcall` がない。`doc/lisp/canlisp.tex` にあるとおり。




## ローマ字かなテーブル

ローマ字かなだけではなく、カナ配列, TUT-Code もこのレイヤー. ローマ字かな変換はクライアント側で行われる.

読み込む場所が, いろんな場所を検索していたが, 次に整理した。
  romaji path = $(HOME)/.config/canna/default.cbp
  romaji path = /usr/share/canna/default.cbp  -- Debian版, EPEL版と同じ.

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



## クライアントユーティリティ

### cmd/cannacheck

2022.11.3
```
$ /opt/canna/bin/cannacheck -v
カスタマイズファイルとして "/opt/canna/etc/canna/default.canna" を用います。romaji path = /home/hori/.config/canna/default.cbp
romaji path = /opt/canna/share/canna/default.cbp
ローマ字かな変換テーブル: `/opt/canna/share/canna/default.cbp'
RKC config path=/home/hori/.config/canna/rkc.conf, preproc=LANG=C /usr/bin/cpp -E -fdirectives-only -nostdinc
サーバ "localhost" に接続します。
RKC�������ե����뤬�����ޤ���
bushu���ޥ����ȤǤ��ޤ����Ǥ���
keishiki���ޥ����ȤǤ��ޤ����Ǥ���
hojoswd���ޥ����ȤǤ��ޤ����Ǥ���
hojomwd���ޥ����ȤǤ��ޤ����Ǥ���
fuzokugo���ޥ����ȤǤ��ޤ����Ǥ���
iroha���ޥ����ȤǤ��ޤ����Ǥ���
```

下 7行は次の文字列:
```
RKCの設定ファイルがありません
bushuをマウントできませんでした
keishikiをマウントできませんでした
hojoswdをマウントできませんでした
hojomwdをマウントできませんでした
fuzokugoをマウントできませんでした
irohaをマウントできませんでした
```

環境変数 `CANNA_RKC_PREPROCESSOR`

TODO:
 - <s>カスタマイズファイルの場所が `/var/lib/` 以下ではおかしい。`/etc/` 以下でないと.</s> 修正済み. `default.canna` のインストール先も変更.


### cmd/cannastat

●●未了


### cmd/cshost

●●未了





## server/

2022.11.3
```
$ <kbd>sudo /opt/canna/sbin/cannaserver -d 100 -inet</kbd>
辞書ホームディレクトリィ = /opt/canna/var/lib/canna/dic
My name is kiwi2.fruits
今からソケットを作る
UNIX Socket path: /opt/canna/var/run/canna/IROHA.sock
UNIX ドメインはできた
Service: [canna]
INET ドメインはできた
ソケットの準備はできた
select()で待ちを開始
```

root で動かせば, 起動はできる.
 - <s>cannacheck から接続できない.</s> IPソケットでの接続はできた。●●UNIX ドメインソケット未了.

UNIX ドメインソケット
  -p オプションで, ソケットファイルに ":1" などを追加.
IPソケット
  Original はポート番号に加算する数字. 謎過ぎる. 次のようにした:
    - ポート番号を指定  <kbd>-p port</kbd>
    - 指定がない場合, `/etc/services` からポート番号を取得.
    - それもない場合, default port number 5680





## クライアントライブラリ

### lib/RKC

TODO: RKindep のソースコードファイルを取り込んでいるが、単に同ライブラリに依存すればいいのでは?
 - cmd/canlisp が不味い. ほかは問題なさそう.


接続先サーバの決定方法

cmd/cannastat.c -> RkwInitialize(hostname)

<s>lib/RK/context.c:RkwInitialize( const char* ddhome )</s>  // ×これはサーバ側

lib/RKC/rkc.c:RkwInitialize( const char* hostname )
  -> 大域変数 ServerNameSpecified にホスト名保存
     -> rkc_Connect_Iroha_Server(ConnectIrohaServerName)

lib/RKC/wconvert.c:SOCKET rkc_Connect_Iroha_Server( const char* hostname_port )
  -> rkc_build_cannaserver_list( listp ) ;
     ●●接続先を複数、収集して、リストを作る。複数の接続先というのはどういう意味か?
