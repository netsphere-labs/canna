﻿// -*- coding:utf-8-with-signature -*-

/* Copyright 1992 NEC Corporation, Tokyo, Japan.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of NEC
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  NEC Corporation makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * NEC CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NEC CORPORATION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTUOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* filedef

  util.c -- ユーティリティ関数を集めた。

  以下の関数がある。(追加した人はちゃんと書いといてよ)

  GlineClear         ガイドラインが消されるようなリターン値を作る
  Gline2echostr      ガイドラインで返そうとしたものをその場で返す
  echostrClear       その場が全く消されるようなリターン値を作る
  checkGLineLen      ガイドラインに表示しきれるかどうかのチェック
  NothingChanged     何も変化がないことを示すリターン値を作る
  NothingForGLine    ガイドラインに関しては何も変化がない
  NothingChangedWithBeep
                     NothingChange をしてさらにビープ音を鳴らす
  NothingForGLineWithBeep
                     NothingForGLine をしてさらにビープ音を鳴らす
  CannaBeep          ビープ音をならす。
  makeGLineMessage   引数の文字列をGLineに表示するようなリターン値を作る
  makeGLineMessageFromString
  		     引数のeuc文字列をGLineに表示するようなリターン値を作る
  setWStrings	     文字配列の初期化を行う
  NoMoreMemory       メモリがないからエラーだよというエラー値を返す
  GLineNGReturn      エラーメッセージをガイドラインに移す
  GLineNGReturnFI    一覧モードを抜けて GLineNGReturn をする。
  GLineNGReturnTK    登録モードを抜けて GLineNGReturn をする。
  WStrlen            ワイドキャラクタ文字列の長さを求める (cf. strlen)
  WStrcat            ワイドキャラクタ文字列を加える。(cf. strcat)
  WStrcpy            ワイドキャラクタ文字列をコピーする。(cf. strcpy)
  WStrncpy           ワイドキャラクタ文字列をｎ文字コピーする。(cf. strncpy)
  WStraddbcpy        ワイドキャラクタ文字列を空白文字、タブ、バックスラッシュ
                     の前にバックスラッシュを入れながらコピーする。
  WStrcmp	     ワイドキャラクタ文字列を比較する。(cf. strcmp)
  WStrncmp	     ワイドキャラクタ文字列をｎ文字比較する。(cf. strncmp)
  WWhatGPlain	     ワイドキャラクタ１文字の属するグラフィックプレーンを返す
  WIsG0              G0のワイドキャラクタ文字か？
  WIsG1              G1のワイドキャラクタ文字か？
  WIsG2              G2のワイドキャラクタ文字か？
  WIsG3              G3のワイドキャラクタ文字か？
  CANNA_mbstowcs     EUC をワイドキャラクタ文字列に変換
  CNvW2E             ワイドキャラクタを EUC に変換(チェック付き)
  CANNA_wcstombs     ワイドキャラクタを EUC に変換
  WSfree	     WStringで確保した領域を開放する
  WString            EUC をワイドに変換して malloc までして返す(free 不要)
  WStringOpen        上記関数の初期化処理
  WStringClose       上記関数の終了処理
  WToupper           引数の文字を大文字にする
  WTolower           引数の文字を小文字にする
  key2wchar          キーボード入力をワイドキャラクタにする。
  US2WS              Ushort を wchar_t に変換する。
  WS2US              wchar_t を Ushort に変換する。
  confirmContext     yc->context が使えるものか確認する
  makeRkError        Rk の関数でエラーがでたときの処理をする。
  canna_alert        メッセージを Gline に書いて key を待つ。

 */

#if !defined(lint) && !defined(__CODECENTER__)
static char rcs_id[] = "@(#) 102.1 $Id: util.c,v 1.6 2003/09/17 08:50:53 aida_s Exp $";
#endif	/* lint */

#include "canna.h"
#include <errno.h>

#ifdef luna88k
extern int errno;
#endif

/*********************************************************************
 *                      wchar_t replace begin                        *
 *********************************************************************/
#ifdef wchar_t
# error "wchar_t is already defined"
#endif
#define wchar_t cannawc

/* arraydef

  tmpbuf -- ちょっと仮に使われるバッファ

 */

/*
 * Gline をクリアする
 *
 * 引き数	uiContext
 * 戻り値	なし
 */
void
GlineClear(d)
uiContext d;
{
  d->kanji_status_return->info |= KanjiGLineInfo;
  d->kanji_status_return->gline.line = (wchar_t *)NULL;
  d->kanji_status_return->gline.length = 0;
  d->kanji_status_return->gline.revPos = 0;
  d->kanji_status_return->gline.revLen = 0;
}

/* cfuncdef

  Gline2echostr -- 一覧行の内容をその場に移動

 */

static void
Gline2echostr(d)
uiContext d;
{
  d->kanji_status_return->echoStr =
    d->kanji_status_return->gline.line;
  d->kanji_status_return->length =
    d->kanji_status_return->gline.length;
  d->kanji_status_return->revPos =
    d->kanji_status_return->gline.revPos;
  d->kanji_status_return->revLen =
    d->kanji_status_return->gline.revLen;
  GlineClear(d);
}

void
echostrClear(d)
uiContext d;
{
  d->kanji_status_return->echoStr = (wchar_t *)NULL;
  d->kanji_status_return->length =
    d->kanji_status_return->revPos = d->kanji_status_return->revLen = 0;
}

/*
 * 文字列からコラム幅を取っ手来る関数
 */

static
colwidth(s, len)
wchar_t *s;
int     len;
{
  int ret = 0;
  wchar_t *es = s + len;

  for (; s < es ; s++) {
    switch (WWhatGPlain(*s)) {
    case 0:
    case 2:
      ret ++;
      break;
    case 1:
    case 3:
      ret += 2;
      break;
    }
  }
  return ret;
}


/**
 * 一覧行に表示できる長さを越えているかをチェック

  長さが越えていたら、カーソル部分に表示されるようにする。
 */
int checkGLineLen( uiContext d )
{
  if (d->kanji_status_return->info & KanjiGLineInfo) {
    if (colwidth(d->kanji_status_return->gline.line,
		 d->kanji_status_return->gline.length) > d->ncolumns) {
      Gline2echostr(d);
      return -1;
    }
  }
  return 0;
}

/* cfuncdef

  NothingChanged -- 読みについては何も変えないようにする

 */

NothingChanged(d)
uiContext d;
{
  d->kanji_status_return->length = -1; /* 変わらない。 */
  d->kanji_status_return->revPos
    = d->kanji_status_return->revLen = 0;
  d->kanji_status_return->info = 0;
  return 0;
}

NothingForGLine(d)
uiContext d;
{
  d->kanji_status_return->length = -1; /* 変わらない。 */
  d->kanji_status_return->revPos
    = d->kanji_status_return->revLen = 0;
  return 0;
}

void
CannaBeep()
{
  extern int (*jrBeepFunc) pro((void));

  if (jrBeepFunc) {
    jrBeepFunc();
  }
}

NothingChangedWithBeep(d)
uiContext d;
{
  CannaBeep();
  return NothingChanged(d);
}

NothingForGLineWithBeep(d)
uiContext d;
{
  CannaBeep();
  return NothingForGLine(d);
}

#ifdef SOMEONE_USE_THIS
/* 誰も使っていないみたい。 */
Insertable(ch)
unsigned char ch;
{
  if ((0x20 <= ch && ch <= 0x7f) || (0xa0 <= ch && ch <= 0xff)) {
    return 1;
  }
  else {
    return 0;
  }
}
#endif /* SOMEONE_USE_THIS */

extern extractJishuString pro((yomiContext, wchar_t *,  wchar_t *,
                               wchar_t **,  wchar_t **));

/*
  extractSimpleYomiString -- yomiContext の読み部分だけを取り出す

  引数
     yc  -- yomiContext
     s   -- 取り出す先のアドレス
     e   -- ここを越えて取り出してはならない、と言うアドレス
     sr  -- 反転領域の開始位置を返すアドレス
     er  -- 反転領域の終了位置を返すアドレス
     pat -- pointer to an attribute buffer.
     focused -- indicates yc is focused or not
 */

static int extractSimpleYomiString
  pro((yomiContext, wchar_t *, wchar_t *, wchar_t **, wchar_t **,
       wcKanjiAttributeInternal *, int));

static int
extractSimpleYomiString(yc, s, e, sr, er, pat, focused)
yomiContext yc;
wchar_t *s, *e, **sr, **er;
wcKanjiAttributeInternal *pat;
int focused;
{
  int len = yc->kEndp - yc->cStartp;

  if (yc->jishu_kEndp) {
    int len = extractJishuString(yc, s, e, sr, er);
    char target = focused ?
      CANNA_ATTR_TARGET_NOTCONVERTED : CANNA_ATTR_CONVERTED;
    if (pat && pat->sp + len < pat->ep) {
      char *ap = pat->sp, *ep = ap + len;
      char *mp1 = ap + (*sr - s), *mp2 = ap + (*er - s);
      while (ap < mp1) {
	*ap++ = CANNA_ATTR_INPUT;
      }
      while (ap < mp2) {
	*ap++ = target;
      }
      while (ap < ep) {
	*ap++ = CANNA_ATTR_INPUT;
      }
      pat->sp = ap;
    }
    return len;
  }

  if (s + len >= e) {
    len = (int)(e - s);
  }
  WStrncpy(s, yc->kana_buffer + yc->cStartp, len);
  if (pat && pat->sp + len < pat->ep) {
    char *ap = pat->sp, *ep = ap + len;

    if (focused) {
      pat->u.caretpos = (ap - pat->u.attr) + yc->kCurs - yc->cStartp;
      /* 上記の計算の解説: キャレットの位置は、今から書き込みをしようと
	 している位置からの相対で、計算し、yc->kCurs - yc->cStartp の位
	 置である。 */
    }

    while (ap < ep) {
      *ap++ = CANNA_ATTR_INPUT;
    }
    pat->sp = ap;
  }
  if (cannaconf.ReverseWidely) {
    *sr = s;
    *er = s + yc->kCurs - yc->cStartp;
  }
  else if (yc->kCurs == yc->kEndp && !yc->right) {
    *sr = *er = s + yc->kCurs - yc->cStartp;
  }
  else {
    *sr = s + yc->kCurs - yc->cStartp;
    *er = *sr + 1;
  }
  return len;
}

/*
  extractKanjiString -- yomiContext の漢字候補を取り出す

  引数
     yc  -- yomiContext
     s   -- 取り出す先のアドレス
     e   -- ここを越えて取り出してはならない、と言うアドレス
     b   -- 文節区切りをするかどうか
     sr  -- 反転領域の開始位置を返すアドレス
     er  -- 反転領域の終了位置を返すアドレス
     pat -- wcKanjiAttributeInternal structure to store attribute information
     focused -- focus is on this yc.
 */

static int extractKanjiString
  pro((yomiContext, wchar_t *, wchar_t *, int, wchar_t **, wchar_t **,
       wcKanjiAttributeInternal *, int));

static int
extractKanjiString(yc, s, e, b, sr, er, pat, focused)
yomiContext yc;
wchar_t *s, *e, **sr, **er;
int b;
wcKanjiAttributeInternal *pat;
int focused;
{
  wchar_t *ss = s;
  int i, len, nbun;

  nbun = yc->bunlen ? yc->curbun : yc->nbunsetsu;

  for (i = 0 ; i < nbun ; i++) {
    if (i && b && s < e) {
      *s++ = (wchar_t)' ';
      if (pat && pat->sp < pat->ep) {
	*pat->sp++ = CANNA_ATTR_CONVERTED;
      }
    }
    RkwGoTo(yc->context, i);
    len = RkwGetKanji(yc->context, s, (int)(e - s));
    if (len < 0) {
      if (errno == EPIPE) {
	jrKanjiPipeError();
      }
      jrKanjiError = "カレント候補を取り出せませんでした";
    }
    else {
      char curattr;
      if (i == yc->curbun && !yc->bunlen && focused) { /* focused */
	*sr = s; *er = s + len;
	curattr = CANNA_ATTR_TARGET_CONVERTED;
      }
      else {
	curattr = CANNA_ATTR_CONVERTED;
      }
      if (pat && pat->sp + len < pat->ep) {
	char *ap = pat->sp, *ep = ap + len;
	while (ap < ep) {
	  *ap++ = curattr;
	}
	pat->sp = ap;
      }
      s += len;
    }
  }

  if (yc->bunlen) {
    if (i && b && s < e) {
      *s++ = (wchar_t)' ';
      if (pat && pat->sp < pat->ep) {
	*pat->sp++ = CANNA_ATTR_CONVERTED;
      }
    }
    len = yc->kEndp - yc->kanjilen;
    if ((int)(e - s) < len) {
      len = (int)(e - s);
    }
    WStrncpy(s, yc->kana_buffer + yc->kanjilen, len);
    if (pat && pat->sp + len < pat->ep) {
      char *ap = pat->sp, *mp = ap + yc->bunlen, *ep = ap + len;
      char target = focused ?
	CANNA_ATTR_TARGET_NOTCONVERTED : CANNA_ATTR_CONVERTED;
      while (ap < mp) {
	*ap++ = target;
      }
      while (ap < ep) {
	*ap++ = CANNA_ATTR_INPUT;
      }
      pat->sp = ap;
    }
    if (b) {
      *er = (*sr = s + yc->bunlen) +
	((yc->kanjilen + yc->bunlen == yc->kEndp) ? 0 : 1);
    }
    else {
      *sr = s; *er = s + yc->bunlen;
    }
    s += len;
  }

  if (s < e) {
    *s = (wchar_t)'\0';
  }

  RkwGoTo(yc->context, yc->curbun);
  return (int)(s - ss);
}

/*
  extractYomiString -- yomiContext の文字を取り出す

  引数
     yc  -- yomiContext
     s   -- 取り出す先のアドレス
     e   -- ここを越えて取り出してはならない、と言うアドレス
     b   -- 文節区切りをするかどうか
     sr  -- 反転領域の開始位置を返すアドレス
     er  -- 反転領域の終了位置を返すアドレス
     pat -- wcKanjiAttributeInternal structure to store attribute information
     focused -- The yc is now focused.
 */

static int extractYomiString
  pro((yomiContext, wchar_t *, wchar_t *, int, wchar_t **, wchar_t **,
       wcKanjiAttributeInternal *, int));

static int
extractYomiString(yc, s, e, b, sr, er, pat, focused)
yomiContext yc;
wchar_t *s, *e, **sr, **er;
int b;
wcKanjiAttributeInternal *pat;
int focused;
{
  int autoconvert = yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE, len;
  wchar_t *ss = s;

  if (autoconvert) {
    int OnBunsetsu = ((yc->status & CHIKUJI_ON_BUNSETSU) ||
		      (yc->nbunsetsu && !(yc->status & CHIKUJI_OVERWRAP)));
    len = extractKanjiString(yc, s, e, b, sr, er, pat, focused && OnBunsetsu);
    s += len;
    if (yc->kEndp - yc->cStartp > 0) {
      wchar_t *ssr, *eer;

      if (b && len && s < e) {
	*s++ = (wchar_t)' ';
	if (pat && pat->sp < pat->ep) {
	  *pat->sp++ = CANNA_ATTR_CONVERTED;
	}
      }
      len = extractSimpleYomiString(yc, s, e, &ssr, &eer, pat,
				    focused && !OnBunsetsu);
/* 最後の !OnBunsetsu ってところは、以下のようにした方が表示がキャレット
   つきで、反転文節が出てもキャレットがカーソルポジションであることがわ
   かりやすいのだが、OVERWRAP フラグがちゃんとモード等との対応がされてい
   ないようなので、とりあえず上記のままとする。
				    (!yc->nbunsetsu ||
				     (yc->status & CHIKUJI_OVERWRAP)));
 */
      s += len;
      if (!OnBunsetsu) {
	*sr = ssr; *er = eer;
	if (pat && focused) {
	  pat->u.caretpos = pat->sp - pat->u.attr - (s - *sr);
	  /* 上記の計算の解説: キャレット位置は、今後アトリビュート
	     を書き込む位置から戻った位置にある。どのくらい戻るかと
	     言うと、次に文字列を書き込む位置から、反転開始位置まで
	     戻る量だけ戻る */
	}
      }
    }
  }
  else if (yc->nbunsetsu) { /* 単候補モード */
    len = extractKanjiString(yc, s, e, b, sr, er, pat, focused);
    s += len;
  }
  else {
    len = extractSimpleYomiString(yc, s, e, sr, er, pat, focused);
    s += len;
  }
  if (s < e) {
    *s = (wchar_t)'\0';
  }
  return (int)(s - ss);
}

static
extractString(str, s, e)
wchar_t *str, *s, *e;
{
  int len;

  len = WStrlen(str);
  if (s + len < e) {
    WStrcpy(s, str);
    return len;
  }
  else {
    WStrncpy(s, str, (int)(e - s));
    return (int)(e - s);
  }
}

/*
  extractTanString -- tanContext の文字を取り出す

  引数
     tan -- tanContext
     s   -- 取り出す先のアドレス
     e   -- ここを越えて取り出してはならない、と言うアドレス
 */

int
extractTanString(tan, s, e)
tanContext tan;
wchar_t *s, *e;
{
  return extractString(tan->kanji, s, e);
}

/*
  extractTanYomi -- tanContext の文字を取り出す

  引数
     tan -- tanContext
     s   -- 取り出す先のアドレス
     e   -- ここを越えて取り出してはならない、と言うアドレス
 */

int
extractTanYomi(tan, s, e)
tanContext tan;
wchar_t *s, *e;
{
  return extractString(tan->yomi, s, e);
}

/*
  extractTanRomaji -- tanContext の文字を取り出す

  引数
     tan -- tanContext
     s   -- 取り出す先のアドレス
     e   -- ここを越えて取り出してはならない、と言うアドレス
 */

int
extractTanRomaji(tan, s, e)
tanContext tan;
wchar_t *s, *e;
{
  return extractString(tan->roma, s, e);
}

void
makeKanjiStatusReturn(d, yc)
uiContext d;
yomiContext yc;
{
  int len;
  wchar_t *s = d->genbuf, *e = s + ROMEBUFSIZE, *sr, *er, *sk, *ek;
  tanContext tan = (tanContext)yc;
  long truecaret = -1;

  if (d->attr) {
    d->attr->sp = d->attr->u.attr;
    d->attr->ep = d->attr->u.attr + d->attr->len;
  }

  /* 最初は変換されている部分を取り出す */
  while (tan->left) {
    tan = tan->left;
  }

  while (tan) {
    if (d->attr) d->attr->u.caretpos = -1;
    switch (tan->id) {
    case TAN_CONTEXT:
      len = extractTanString(tan, s, e);
      sk = s; ek = s + len;
      if (d->attr &&
	  d->attr->sp + len < d->attr->ep) {
	char *ap = d->attr->sp, *ep = ap + len;
	char curattr =
	  ((mode_context)tan == (mode_context)yc) ?
	    CANNA_ATTR_TARGET_CONVERTED : CANNA_ATTR_CONVERTED;
	for (; ap < ep ; ap++) {
	  *ap = curattr;
	}
	d->attr->sp = ap;
      }
      break;
    case YOMI_CONTEXT:
      len = extractYomiString((yomiContext)tan, s, e, cannaconf.BunsetsuKugiri,
			      &sk, &ek, d->attr,
			      (mode_context)tan == (mode_context)yc);
      break;
    default:
      break;
    }

    if ((mode_context)tan == (mode_context)yc) {
      sr = sk;
      er = ek;
      if (d->attr) truecaret = d->attr->u.caretpos;
    }
    s += len;
    tan = tan->right;
    if (cannaconf.BunsetsuKugiri && tan && s < e) {
      *s++ = (wchar_t)' ';
      if (d->attr && d->attr->sp < d->attr->ep) {
	*d->attr->sp++ = CANNA_ATTR_CONVERTED;
      }
    }
  }

  if (s < e) {
    *s = (wchar_t)'\0';
  }

  d->kanji_status_return->length = (int)(s - d->genbuf);
  d->kanji_status_return->echoStr = d->genbuf;
  d->kanji_status_return->revPos = (int)(sr - d->genbuf);
  d->kanji_status_return->revLen = (int)(er - sr);
  if (d->attr) {
    d->attr->u.caretpos = truecaret;
    if (d->kanji_status_return->length < d->attr->len) {
      d->attr->u.attr[d->kanji_status_return->length] = '\0';
    }
    d->kanji_status_return->info |= KanjiAttributeInfo;
  }
}

#define MESSBUFSIZE 256

/*
 * リバースなしのメッセージをガイドラインに表示する
 * 次の入力があったときに消えるようにフラグを設定する
 */
void
makeGLineMessage(d, msg, sz)
uiContext d;
wchar_t *msg;
int sz;
{
  static wchar_t messbuf[MESSBUFSIZE];
  int len = sz < MESSBUFSIZE ? sz : MESSBUFSIZE - 1;

  WStrncpy(messbuf, msg, len);
  messbuf[len] = (wchar_t)0;
  d->kanji_status_return->gline.line = messbuf;
  d->kanji_status_return->gline.length = len;
  d->kanji_status_return->gline.revPos = 0;
  d->kanji_status_return->gline.revLen = 0;
  d->kanji_status_return->info |= KanjiGLineInfo;

  d->flags &= ~PCG_RECOGNIZED;
  d->flags |= PLEASE_CLEAR_GLINE;
  checkGLineLen(d);
}


void makeGLineMessageFromString( uiContext d, const char  *msg )
{
  int len;

  len = MBstowcs(d->genbuf, msg, ROMEBUFSIZE);
  makeGLineMessage(d, d->genbuf, len);
}


int setWStrings(cannawc** ws, const char** s, int sz)
{
  int f = sz;
  wchar_t *WString();

  for (; (f && sz) || (!f && *s); ws++, s++, sz--) {
    *ws = WString(*s);
    if (!*ws) {
      return NG;
    }
  }
  return 0;
}

#ifdef DEBUG
dbg_msg(fmt, x, y, z)
char *fmt;
int x, y, z;
{
  if (iroha_debug) {
    fprintf(stderr, fmt, x, y, z);
  }
}

checkModec(d)
uiContext d;
{
  coreContext c;
  struct callback *cb;
  int depth = 0, cbDepth = 0;
  int callbacks = 0;

  for (c = (coreContext)d->modec ; c ; c = (coreContext)c->next)
    depth++;
  for (cb = d->cb ; cb ; cb = cb->next) {
    int i;

    cbDepth++;
    for (i = 0 ; i < 4 ; i++) {
      callbacks <<= 1;
      if (cb->func[i]) {
	callbacks++;
      }
    }
  }
  if (depth != cbDepth) {
    fprintf(stderr, "■■■■■！！！深さが違うぞ！！！■■■■■\n");
  }
  debug_message("\242\243\40\277\274\244\265: d->modec:%d d->cb:%d callbacks:0x%08x ",
		depth, cbDepth, callbacks);
                /* ■ 深さ */
  debug_message("EXIT_CALLBACK = 0x%x\n", d->cb->func[EXIT_CALLBACK],0,0);
  {
    extern KanjiModeRec yomi_mode;
    if (d->current_mode == &yomi_mode) {
      yomiContext yc = (yomiContext)d->modec;
      if (yc->kana_buffer[yc->kEndp]) {
        fprintf(stderr, "■■■■■ カナバッファにゴミが入っているぞ！\n");
      }
    }
  }
}

static char pbufstr[] = " o|do?b%";

showRomeStruct(dpy, win)
unsigned int dpy, win;
{
  uiContext d, keyToContext();
  extern defaultContext;
  static int n = 0;
  int i;
  char buf[1024];

  n++;
  fprintf(stderr, "\n【デバグメッセージ(%d)】\n", n);
  d = keyToContext((unsigned int)dpy, (unsigned int)win);
  fprintf(stderr, "buffer(0x%x), bytes(%d)\n",
	  d->buffer_return, d->n_buffer);
  fprintf(stderr, "nbytes(%d), ch(0x%x)\n", d->nbytes, d->ch);
  fprintf(stderr, "モード: %d\n", ((coreContext)d->modec)->minorMode);
  /* コンテクスト */
  fprintf(stderr, "コンテクスト(%d)\n", d->contextCache);
  fprintf(stderr, "デフォルトコンテクスト(%d)\n", defaultContext);

  /* ローマ字かな関連 */
  if (((coreContext)d->modec)->id == YOMI_CONTEXT) {
    yomiContext yc = (yomiContext)d->modec;

    fprintf(stderr, "r:       Start(%d), Cursor(%d), End(%d)\n",
	    yc->rStartp, yc->rCurs, yc->rEndp);
    fprintf(stderr, "k: 未変換Start(%d), Cursor(%d), End(%d)\n",
	    yc->kRStartp, yc->kCurs, yc->kEndp);
    WStrncpy(buf, yc->romaji_buffer, yc->rEndp);
    buf[yc->rEndp] = '\0';
    fprintf(stderr, "romaji_buffer(%s)\n", buf);
    fprintf(stderr, "romaji_attrib(");
    for (i = 0 ; i <= yc->rEndp ; i++) {
      fprintf(stderr, "%1x", yc->rAttr[i]);
    }
    fprintf(stderr, ")\n");
    fprintf(stderr, "romaji_pointr(");
    for (i = 0 ; i <= yc->rEndp ; i++) {
      int n = 0;
      if (i == yc->rStartp)
	n |= 1;
      if (i == yc->rCurs)
	n |= 2;
      if (i == yc->rEndp)
	n |= 4;
      fprintf(stderr, "%c", pbufstr[n]);
    }
    fprintf(stderr, ")\n");
    WStrncpy(buf, yc->kana_buffer, yc->kEndp);
    buf[yc->kEndp] = '\0';
    fprintf(stderr, "kana_buffer(%s)\n", buf);
    fprintf(stderr, "kana_attrib(");
    for (i = 0 ; i <= yc->kEndp ; i++) {
      fprintf(stderr, "%1x", yc->kAttr[i]);
    }
    fprintf(stderr, ")\n");
    fprintf(stderr, "kana_pointr(");
    for (i = 0 ; i <= yc->kEndp ; i++) {
      int n = 0;
      if (i == yc->kRStartp)
	n |= 1;
      if (i == yc->kCurs)
	n |= 2;
      if (i == yc->kEndp)
	n |= 4;
      fprintf(stderr, "%c", pbufstr[n]);
    }
    fprintf(stderr, ")\n");
    fprintf(stderr, "\n");
  }
/*  RkPrintDic(0, "kon"); */
}
#endif /* DEBUG */

extern const char* jrKanjiError;

int NoMoreMemory()
{
  jrKanjiError = "\245\341\245\342\245\352\244\254\311\324\302\255\244\267\244\306\244\244\244\336\244\271\241\243";
                /* メモリが不足しています。 */
  return NG;
}

GLineNGReturn(d)
uiContext d;
{
  int len;
  len = MBstowcs(d->genbuf, jrKanjiError, ROMEBUFSIZE);
  makeGLineMessage(d, d->genbuf, len);
  currentModeInfo(d);

  return(0);
}

GLineNGReturnFI(d)
uiContext d;
{
  popForIchiranMode(d);
  popCallback(d);
  GLineNGReturn(d);
  return(0);
}

#ifndef NO_EXTEND_MENU

GLineNGReturnTK(d)
uiContext d;
{
  extern void popTourokuMode pro((uiContext));
  popTourokuMode(d);
  popCallback(d);
  GLineNGReturn(d);
  return(0);
}

#endif /* NO_EXTEND_MENU */

#ifdef USE_COPY_ATTRIBUTE
copyAttribute(dest, src, n)
     BYTE	*dest;
     BYTE	*src;
     int	n;
{
  if (dest > src && dest < src + n) {
    dest += n;
    src += n;
    while (n-- > 0) {
      *--dest = *--src;
    }
  }
  else {
    while (n-- > 0) {
      *dest++ = *src++;
    }
  }
}
#endif

#ifdef DEBUG_ALLOC
int fail_malloc = 0;

#undef malloc

char *
debug_malloc(n)
int n;
{
  if (fail_malloc)
    return (char *)0;
  else
    return malloc(n);
}
#endif /* DEBUG_ALLOC */

/*
 * ワイドキャラクタオペレーション
 *
 */


wchar_t *
WStraddbcpy(ws1, ws2, cnt)
wchar_t	*ws1, *ws2;
int cnt;
{
  wchar_t *strp = ws1, *endp = ws1 + cnt - 1;

  while (*ws2 != (wchar_t)'\0' && ws1 < endp) {
    if (*ws2 == (wchar_t)' ' || *ws2 == (wchar_t)'\t' || *ws2 == (wchar_t)'\\')
      *ws1++ = (wchar_t)'\\';
    *ws1++ = *ws2++;
  }
  if (ws1 == endp) {
    ws1--;
  }
  *ws1 = (wchar_t)'\0';
  return(strp);
}



/* cfuncdef

  WString -- EUCからワイドキャラクタへのマッピングおよび malloc

  WString は引数の文字列をワイドキャラクタに変換し、その文字列が収まる
  だけのメモリを malloc し、その文字列を納め返す。

  利用者はこの関数で得たポインタを free する必要はあまりない。

  すなわち、この関数で得たメモリは後で WStringClose を呼び出したときに
  free される。

  そういう事情なのでこの関数を頻繁に呼び出してはいけない。今までEUCで
  初期定義できていた文字列などに留めるべきである。

  この機能を使う人は最初に WStringOpen を呼び出さなければならないが、
  ユーザインタフェースライブラリではシステムが自動的に読んでくれるの
  でその必要はない。

 */

static wchar_t **wsmemories = (wchar_t **)NULL;
static int nwsmemories = 0;

#define WSBLOCKSIZE 128

int
WStringOpen()
{
  return 0;
}

wchar_t *
WString(s)
char *s;
{
  int i, len;
  wchar_t *temp, **wm;

  if (wsmemories == (wchar_t **)NULL) {
    nwsmemories = WSBLOCKSIZE;
    if (!(wsmemories = (wchar_t **)calloc(nwsmemories, sizeof(wchar_t *))))
      return((wchar_t *)0) ;
    /* calloc されたメモリはクリアされている */
  }

  for (i = 0 ; i < nwsmemories && wsmemories[i] ;) {
    i++;
  }

  if (i == nwsmemories) { /* 使い切ったので増やす */
    if (!(wm = (wchar_t **)realloc(wsmemories,
				 (nwsmemories + WSBLOCKSIZE)
				 * sizeof(wchar_t *))))
      return((wchar_t *)0);
    wsmemories = wm;
    for (; i < nwsmemories + WSBLOCKSIZE ; i++)
      wsmemories[i] = (wchar_t *)0;
    i = nwsmemories;
    nwsmemories += WSBLOCKSIZE;
  }

  /* とりあえず大きくとっておいて、そのサイズを見て丁度のサイズに
     直して返す */

  len = strlen(s);
  if (!(temp = (wchar_t *)malloc((len + 1) * WCHARSIZE)))
    return((wchar_t *)0);
  len = MBstowcs(temp, s, len + 1);
  if (!(wsmemories[i] = (wchar_t *)malloc((len + 1) * WCHARSIZE))) {
    free(temp);
    return((wchar_t *) 0);
  }
  WStrncpy(wsmemories[i], temp, len);
  wsmemories[i][len] = (wchar_t)0;
  free(temp);
  return(wsmemories[i]);
}

void
WStringClose()
{
  int i;

  for (i = 0 ; i < nwsmemories ; i++)
    if (wsmemories[i])
      free(wsmemories[i]);
  free(wsmemories);
  wsmemories = (wchar_t **)0;
  nwsmemories = 0;
}

WSfree(s)
     wchar_t *s;
{
  int	i;
  wchar_t **t;

  for (t = wsmemories, i = nwsmemories; s != *t && i;) {
    t++;
    i--;
  }
  if (s != *t)
    return(-1);
  free(*t);
  *t = (wchar_t *) 0;
  return(0);
}

/*
 generalReplace -- カナバッファにもローマ字バッファにも使える置換ルーチン

  この置換ルーチンは文字列のメモリ上の置換を行うためのライブラリルーチ
  ンである。メモリ上に文字列を保持しておく仕組みは次のようになっている
  ものとする。

    ・文字列用のバッファ
    ・文字の属性用のバッファ
    ・カーソル(インデックス(ポインタではない))
    ・文字列の終わりを指すインデックス
    ・必ずカーソルより左に存在するインデックス(未変換文字へのインデッ
      クスに使ったりする)

  上記に示されるバッファ上のカーソルの前か後ろの指定された長さの文字列
  を別に指定される文字列で置き換える処理をする。

  トータルのバイト数が変化する場合は文字列の終わりを指すインデックスの
  値も変化させる。また、カーソルの前の部分に対して文字列の置換を行う場
  合にはカーソルポジションの値も変化させる。カーソルを変化させた結果、
  未変換文字等へのインデックスよりも小さくなった場合には、未変換文字等
  へのインデックスの値をカーソルの値に合わせて小さくする。

  この関数の最終引数には新たに挿入する文字列の属性に関するヒントが記述
  できる。新たに挿入される文字列の各文字に対して、ヒントで与えられた値
  自身が属性値として格納される。

  【引数】
     buf      バッファへのポインタ
     attr     属性バッファへのポインタ
     startp   バッファの未確定文字列などへのインデックスを収めている変
              数へのポインタ
     cursor   カーソル位置を収めている変数へのポインタ
     endp     文字列の最終位置を指し示している変数へのポインタ

     bytes    何バイト置換するか？負の数が指定されるとカーソルの前の部
              分の |bytes| 分の文字列が置換の対象となり、正の数が指定
              されるとカーソルの後ろの部分の bytes 分の文字列が対象と
              なる。
     rplastr  新しく置く文字列へのポインタ
     len      新しく置く文字列の長さ
     attrmask 新しく置く文字列の属性のヒント

  実際にはこの関数を直接に使わずに、bytes, rplastr, len, attrmask だけ
  を与えるだけですむマクロ、kanaReplace, romajiReplace を使うのが良い。
*/

void
generalReplace(buf, attr, startp, cursor, endp, bytes, rplastr, len, attrmask)
wchar_t *buf, *rplastr;
BYTE *attr;
int *startp, *cursor, *endp,  bytes, len, attrmask;
{
  int idou, begin, end, i;
  int cursorMove;

  if (bytes > 0) {
    cursorMove = 0;
    begin = *cursor;
    end = *endp;
  }
  else {
    bytes = -bytes;
    cursorMove = 1;
    begin = *cursor - bytes;
    end = *endp;
  }

  idou = len - bytes;

  moveStrings(buf, attr, begin + bytes, end, idou);
  *endp += idou;
  if (cursorMove) {
    *cursor += idou;
    if (*cursor < *startp)
      *startp = *cursor;
  }

  WStrncpy(buf + begin, rplastr, len);
  for (i = 0 ; i < len ; i++) {
    attr[begin + i] = attrmask;
  }
/*  if (len)
    attr[begin] |= attrmask; */
}

#ifdef __STDC__
WToupper(wchar_t w)
#else
WToupper(w)
wchar_t w;
#endif
{
  if ('a' <= w && w <= 'z')
    return((wchar_t) (w - 'a' + 'A'));
  else
    return(w);
}

#ifdef __STDC__
WTolower(wchar_t w)
#else
WTolower(w)
wchar_t w;
#endif
{
  if ('A' <= w && w <= 'Z') {
    return (wchar_t)(w - 'A' + 'a');
  }
  else {
    return w;
  }
}

/*
  キーを wchar の文字に変換する。

  引数:
      key         入力されたキー
      check       wchar_t に変換されたかどうかを格納するための変数のアドレス
  返値:
      関数の返値  変換された wchar_t の文字
      check       うまく変換できたかどうか
  注意:
      check は必ず有効な変数のアドレスをポイントすること。
      check のポイント先の有効性は key2wchar ではチェックしない。
 */

wchar_t
key2wchar(key, check)
int key;
int *check;
{
  *check = 1; /* Success as default */
  if (161 <= key && key <= 223) { /* カタカナの範囲だったら */
    char xxxx[4];
    wchar_t yyyy[4];
    int nchars;

    xxxx[0] = (char)0x8e; /* SS2 */
    xxxx[1] = (char)key;
    xxxx[2] = '\0';
    nchars = MBstowcs(yyyy, xxxx, 4);
    if (nchars != 1) {
      *check = 0;
      return 0; /* エラー */
    }
    return yyyy[0];
  }
  else {
    return (wchar_t)key;
  }
}

int
confirmContext(d, yc)
uiContext d;
yomiContext yc;
{
  extern defaultContext;

  if (yc->context < 0) {
    if (d->contextCache >= 0) {
      yc->context = d->contextCache;
      d->contextCache = -1;
    }
    else {
      if (defaultContext == -1) {
	if (KanjiInit() < 0 || defaultContext == -1) {
	  jrKanjiError = KanjiInitError();
	  return -1;
	}
      }
      yc->context = RkwDuplicateContext(defaultContext);
      if (yc->context < 0) {
	if (errno == EPIPE) {
	  jrKanjiPipeError();
	}
	jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271\244\313\274\272\307\324\244\267\244\336\244\267\244\277";
                      /* かな漢字変換に失敗しました */
	return -1;
      }
    }
  }
  return yc->context;
}


int abandonContext( uiContext d, yomiContext yc )
{
  extern defaultContext;

  if (yc->context >= 0) {
    if (d->contextCache >= 0) {
      RkwCloseContext(yc->context);
    }
    else {
      d->contextCache = yc->context;
    }
    yc->context = -1;
  }
  return 0;
}


int makeRkError( uiContext d, const char* str )
{
  if (errno == EPIPE) {
    jrKanjiPipeError();
  }
  jrKanjiError = str;
  makeGLineMessageFromString(d, jrKanjiError);
  return -1;
}

/* 以下メッセージを gline に出すための仕組み */

static int ProcAnyKey( uiContext d )
{
  coreContext cc = (coreContext)d->modec;

  d->current_mode = cc->prevMode;
  d->modec = cc->next;
  freeCoreContext(cc);

  d->status = EXIT_CALLBACK;
  return 0;
}


static int
wait_anykey_func( uiContext d,
                  KanjiMode mode,
                  int whattodo,
                  int key,
                  int fnum )
/* ARGSUSED */
{
  switch (whattodo) {
  case KEY_CALL:
    return ProcAnyKey(d);
  case KEY_CHECK:
    return 1;
  case KEY_SET:
    return 0;
  }
  /* NOTREACHED */
}

static KanjiModeRec canna_message_mode = {
  wait_anykey_func,
  0, 0, 0,
};


static void
cannaMessageMode( uiContext d, canna_callback_t cnt )
{
  coreContext cc;
  extern coreContext newCoreContext pro((void));


  cc = newCoreContext();
  if (cc == 0) {
    NothingChangedWithBeep(d);
    return;
  }
  cc->prevMode = d->current_mode;
  cc->next = d->modec;
  cc->majorMode = d->majorMode;
  cc->minorMode = d->minorMode;
  if (pushCallback(d, d->modec, NO_CALLBACK, cnt,
                     NO_CALLBACK, NO_CALLBACK) == (struct callback *)0) {
    freeCoreContext(cc);
    NothingChangedWithBeep(d);
    return;
  }
  d->modec = (mode_context)cc;
  d->current_mode = &canna_message_mode;
  return;
}


/**
 * メッセージを gline に出す
  何かキーが入力されたら cnt と言う関数を呼び出す。

 * @param d        UI Context
 * @param message  メッセージ
 * @param cnt      次の処理を行う関数

  cnt では popCallback(d) をやらなければならないことに注意！
 */
int canna_alert( uiContext d, char *message, canna_callback_t cnt )
{
  d->nbytes = 0;

  makeGLineMessageFromString(d, message);
  cannaMessageMode(d, cnt);
  return 0;
}

char *
KanjiInitError()
{
  return "\244\253\244\312\264\301\273\372\312\321\264\271\245\265"
    "\241\274\245\320\244\310\304\314\277\256\244\307\244\255\244\336"
      "\244\273\244\363";
  /* "かな漢字変換サーバと通信できません" */
}

#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/
