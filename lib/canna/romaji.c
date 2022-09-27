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

#if !defined(lint) && !defined(__CODECENTER__)
static char rcs_id[] = "@(#) 102.1 $Id: romaji.c,v 1.11 2004/03/15 04:33:23 aida_s Exp $";
#endif /* lint */

#include "canna.h"
#include <ctype.h>
#include <errno.h>
#ifdef MEASURE_TIME
#include <sys/types.h>
#include <sys/times.h>
#endif

/* Now canna have only cbp files. */
#if 1
#define DEFAULT_ROMKANA_TABLE "/dic/default.cbp"
#else
#define DEFAULT_ROMKANA_TABLE "/dic/default.kp"
#endif

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

int forceRomajiFlushYomi pro((uiContext));
static int KanaYomiInsert pro((uiContext));
static int chikujiEndBun pro((uiContext));
extern void EWStrcat pro((wchar_t *, char *));

extern int yomiInfoLevel;

extern struct RkRxDic *englishdic;

/*
 * int d->rStartp;     ro shu c|h    shi f   ローマ字 スタート インデックス
 * int d->rEndp;       ro shu ch     shi f|  ローマ字 バッファ インデックス
 * int d->rCurs;       ro shu ch|    shi f   ローマ字 エンド   インデックス
 * int d->rAttr[1024]; 10 100 11     100 1   ローマ字かな変換先頭フラグバッファ
 * int d->kEndp;       ろ し  ch  ゅ し  f|  かな     バッファ インデックス
 * int d->kRStartp;    ろ し  c|h ゅ し  f   カーソル スタート インデックス
 * int d->kCurs;      ろ し  ch| ゅ し  f   カーソル エンド   インデックス
 * int d->kAttr[1024]; 11 11  00  11 11  0   カナ変換したフラグバッファ
 * int d->nrf;                1              ローマ字変換しませんフラグ
 */

/*
 * フラグやポインタの動き
 *
 *           ひゃく           hyaku
 *  先       100010           10010
 *  変       111111
 *  禁       000000
 * rStartp                         1
 * rCurs                           1
 * rEndp                           1
 * kRstartp        1
 * kCurs           1
 * kEndp           1
 *
 * ←
 *           ひゃく           hyaku
 *  先       100010           10010
 *  変       111111
 *  禁       000000
 * rStartp                       1
 * rCurs                         1
 * rEndp                           1
 * kRstartp      1
 * kCurs         1
 * kEndp           1
 *
 * ←
 *           ひゃく           hyaku
 *  先       100010           10010
 *  変       111111
 *  禁       000000
 * rStartp                       1
 * rCurs                         1
 * rEndp                           1
 * kRstartp    1
 * kCurs       1
 * kEndp           1
 *
 * ←
 *           ひゃく           hyaku
 *  先       100010           10010
 *  変       111111
 *  禁       000000
 * rStartp                    1
 * rCurs                      1
 * rEndp                           1
 * kRstartp  1
 * kCurs     1
 * kEndp           1
 *
 * →
 *           ひゃく           hyaku
 *  先       100010           10010
 *  変       111111
 *  禁       000000
 * rStartp                       1
 * rCurs                         1
 * rEndp                           1
 * kRstartp    1
 * kCurs       1
 * kEndp           1
 *
 * 'k'
 *           ひkゃく           hyakku
 *  先       1010010           100110
 *  変       1101111
 *  禁       0010000
 * rStartp                        1
 * rCurs                           1
 * rEndp                             1
 * kRstartp    1
 * kCurs        1
 * kEndp            1
 *
 * 'i'
 *           ひきゃく           hyakiku
 *  先       10100010           1001010
 *  変       11111111
 *  禁       00110000
 * rStartp                           1
 * rCurs                             1
 * rEndp                               1
 * kRstartp      1
 * kCurs         1
 * kEndp             1
 */

#ifndef KANALIMIT
#define KANALIMIT 255
#endif
#define ROMAJILIMIT 255

#define  doubleByteP(x) ((x) & 0x80)

#ifdef DEBUG
void debug_yomi(x)
yomiContext x;
{
  char foo[1024];
  int len, i;

  if (iroha_debug) {
    len = WCstombs(foo, x->romaji_buffer, 1024);
    foo[len] = '\0';
    printf("    %s\n先: ", foo);
    for (i = 0 ; i <= x->rEndp ; i++) {
      printf("%s", (x->rAttr[i] & SENTOU) ? "1" : " ");
    }
    printf("\nポ: ");
    for (i = 0 ; i < x->rStartp ; i++) {
      printf(" ");
    }
    printf("^\n");

    len = WCstombs(foo, x->kana_buffer, 1024);
    foo[len] = '\0';
    printf("    %s\n先: ", foo);
    for (i = 0 ; i <= x->kEndp ; i++) {
      printf("%s ", (x->kAttr[i] & SENTOU) ? "1" : " ");
    }
    printf("\n済: ");
    for (i = 0 ; i <= x->kEndp ; i++) {
      printf("%s", (x->kAttr[i] & HENKANSUMI) ? "済" : "未");
    }
    printf("\nポ: ");
    for (i = 0 ; i < x->kRStartp ; i++) {
      printf("  ");
    }
    printf("↑\n");

  }
}
#else /* !DEBUG */
# define debug_yomi(x)
#endif /* !DEBUG */

#ifndef CALLBACK
#define kanaReplace(where, insert, insertlen, mask) \
kanaRepl(d, where, insert, insertlen, mask)

static void
kanaRepl(d, where, insert, insertlen, mask)
uiContext d;
int where, insertlen, mask;
wchar_t *insert;
{
  yomiContext yc = (yomiContext)d->modec;

  generalReplace(yc->kana_buffer, yc->kAttr, &yc->kRStartp,
		 &yc->kCurs, &yc->kEndp,
		 where, insert, insertlen, mask);
}
#else /* CALLBACK */
#define kanaReplace(where, insert, insertlen, mask) \
kanaRepl(d, where, insert, insertlen, mask)

static void
kanaRepl(d, where, insert, insertlen, mask)
uiContext d;
int where, insertlen, mask;
wchar_t *insert;
{
  yomiContext yc = (yomiContext)d->modec;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  wchar_t buf[256];
#else
  wchar_t *buf = (wchar_t *)malloc(sizeof(wchar_t) * 256);
  if (!buf) {
    return;
  }
#endif

  WStrncpy(buf, insert, insertlen);
  buf[insertlen] = '\0';

  generalReplace(yc->kana_buffer, yc->kAttr, &yc->kRStartp,
		 &yc->kCurs, &yc->kEndp,
		 where, insert, insertlen, mask);
#ifdef USE_MALLOC_FOR_BIG_ARRAY
  (void)free((char *)buf);
#endif
}
#endif /* CALLBACK */

#define  romajiReplace(where, insert, insertlen, mask) \
romajiRepl(d, where, insert, insertlen, mask)

static void
romajiRepl(uiContext d,
           int where,
           wchar_t* insert,
           int insertlen,
           int mask)
{
  yomiContext yc = (yomiContext)d->modec;

  generalReplace(yc->romaji_buffer, yc->rAttr,
		 &yc->rStartp, &yc->rCurs, &yc->rEndp,
		 where, insert, insertlen, mask);
}

/* cfuncdef

   kPos2rPos -- かなバッファのリージョンからローマ字バッファのリージョンを得る

   yc : 読みコンテクスト
   s  : かなバッファのリージョンの開始位置
   e  : かなバッファのリージョンの終了位置
   rs : ローマ字バッファの対応する開始位置を格納する変数へのポインタ
   rs : ローマ字バッファの対応する終了位置を格納する変数へのポインタ
 */

void
kPos2rPos(yc, s, e, rs, re)
yomiContext yc;
int s, e, *rs, *re;
{
  int i, j, k;

  for (i = 0, j = 0 ; i < s ; i++) {
    if (yc->kAttr[i] & SENTOU) {
      do {
	j++;
      } while (!(yc->rAttr[j] & SENTOU));
    }
  }
  for (i = s, k = j ; i < e ; i++) {
    if (yc->kAttr[i] & SENTOU) {
      do {
	k++;
      } while (!(yc->rAttr[k] & SENTOU));
    }
  }
  if (rs) *rs = j;
  if (re) *re = k;
}

/*
  makeYomiReturnStruct-- 読みをアプリケーションに返す時の構造体を作る関数

  makeYomiReturnStruct は kana_buffer を調べて適当な値を組み立てる。そ
  の時にリバースの領域も設定するが、リバースをどのくらいするかは、
  ReverseWidely という変数を見て決定する。

  */

void
makeYomiReturnStruct(uiContext d)
{
  yomiContext yc = (yomiContext)d->modec;

  makeKanjiStatusReturn(d, yc);
}

extern ckverbose;

static struct RkRxDic *
OpenRoma(const char* table)
{
    struct RkRxDic *retval = NULL, *RkwOpenRoma();
  char *p, *getenv();
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  char rdic[1024];
#else
    char *rdic = (char*) malloc(1024);
    if (!rdic)
        return NULL;
#endif

  if (table || *table) {
    retval = RkwOpenRoma(table);

    if (ckverbose == CANNA_FULL_VERBOSE) {
      if (retval != (struct RkRxDic *)NULL) { /* 辞書がオープンできた */
        printf("ローマ字かな変換テーブルは \"%s\" を用います。\n", table);
      }
    }

    if (retval == (struct RkRxDic *)NULL) {
      /* もし辞書がオープンできなければエラー */
      extern jrUserInfoStruct *uinfo;

      rdic[0] = '\0';
      if (uinfo && uinfo->topdir && uinfo->uname) {
	strcpy(rdic, uinfo->topdir);
	strcat(rdic, "/dic/user/");
	strcat(rdic, uinfo->uname);
	strcat(rdic, "/");
	strcat(rdic, table);
	retval = RkwOpenRoma(rdic);
      }
      else {
        p = getenv("HOME");
        if (p) {
          (void)strcpy(rdic, p);
          (void)strcat(rdic, "/");
          (void)strcat(rdic, table);
          retval = RkwOpenRoma(rdic);
        }
      }

      if (ckverbose == CANNA_FULL_VERBOSE) {
	if (retval != (struct RkRxDic *)NULL) {
          printf("ローマ字かな変換テーブルは \"%s\" を用います。\n", rdic);
	}
      }

      if (retval == (struct RkRxDic *)NULL) { /* これもオープンできない */
        extern jrUserInfoStruct *uinfo;

        rdic[0] = '\0';
        if (uinfo && uinfo->topdir) {
	  strcpy(rdic, uinfo->topdir);
        }
        else {
          strcpy(rdic, CANNALIBDIR);
        }
	strcat(rdic, "/dic/");
	strcat(rdic, table);
	retval = RkwOpenRoma(rdic);

	if (ckverbose) {
	  if (retval != (struct RkRxDic *)NULL) {
	    if (ckverbose == CANNA_FULL_VERBOSE) {
              printf("ローマ字かな変換テーブルは \"%s\" を用います。\n", rdic);
	    }
	  }
	}
      }

      if (retval == (struct RkRxDic *)NULL) { /* added for Debian by ISHIKAWA Mutsumi <ishikawa@linux.or.jp> */
        extern jrUserInfoStruct *uinfo;

        rdic[0] = '\0';
        if (uinfo && uinfo->topdir) {
	  strcpy(rdic, uinfo->topdir);
        }
        else {
          strcpy(rdic, CANNALIBDIR);
        }
	strcat(rdic, "/");
	strcat(rdic, table);
	retval = RkwOpenRoma(rdic);

	if (ckverbose) {
	  if (retval != (struct RkRxDic *)NULL) {
	    if (ckverbose == CANNA_FULL_VERBOSE) {
              printf("ローマ字かな変換テーブルは \"%s\" を用います。\n", rdic);
	    }
	  }
	}
      }

      if (retval == (struct RkRxDic *)NULL) { /* added for Debian by ISHIKAWA Mutsumi <ishikawa@linux.or.jp> */
        extern jrUserInfoStruct *uinfo;

        rdic[0] = '\0';
        if (uinfo && uinfo->topdir) {
	  strcpy(rdic, uinfo->topdir);
        }
        else {
          strcpy(rdic, PKGDATADIR);
        }
	strcat(rdic, "/");
	strcat(rdic, table);
	retval = RkwOpenRoma(rdic);

	if (ckverbose) {
	  if (retval != (struct RkRxDic *)NULL) {
	    if (ckverbose == CANNA_FULL_VERBOSE) {
              printf("ローマ字かな変換テーブルは \"%s\" を用います。\n", rdic);
	    }
	  }
	}
      }

      if (retval == (struct RkRxDic *)NULL) { /* 全部オープンできない */
	sprintf(rdic,
#ifndef CODED_MESSAGE
		"ローマ字かな変換テーブル(%s)がオープンできません。",
#else
		"\245\355\241\274\245\336\273\372\244\253\244\312"
		"\312\321\264\271\245\306\241\274\245\326\245\353\50\45\163\51\244\254"
		"\245\252\241\274\245\327\245\363\244\307\244\255\244\336\244\273"
		"\244\363\241\243",
#endif
		table);
	/* ローマ字かな変換テーブル(%s)がオープンできません。 */
	addWarningMesg(rdic);
	retval = (struct RkRxDic *)0;
      }
    }
  }
#ifdef USE_MALLOC_FOR_BIG_ARRAY
  (void)free((char *)rdic);
#endif
  return retval;
}

RomkanaInit()
{
  extern char *RomkanaTable, *EnglishTable;
  extern extraFunc *extrafuncp;
  extraFunc *extrafunc1, *extrafunc2;
  extern jrUserInfoStruct *uinfo;

  /* ローマ字かな変換テーブルのオープン */
  if (uinfo) {
    if (uinfo->romkanatable) {
      if (RomkanaTable) {
        free(RomkanaTable);
      }
      RomkanaTable = malloc(strlen(uinfo->romkanatable) + 1);
      if (RomkanaTable) {
        strcpy(RomkanaTable, uinfo->romkanatable);
      }
    }
  }
  if (RomkanaTable) {
    romajidic = OpenRoma(RomkanaTable);
  }
  else {
#ifndef USE_MALLOC_FOR_BIG_ARRAY
    char buf[1024];
#else
    char *buf = malloc(1024);
    if (!buf) {
      return 0;
    }
#endif

    buf[0] = '\0';
    if (uinfo && uinfo->topdir) {
      strcpy(buf, uinfo->topdir);
    }
    else {
      strcpy(buf, CANNALIBDIR);
    }
    strcat(buf, DEFAULT_ROMKANA_TABLE);
    romajidic = RkwOpenRoma(buf);

    if (romajidic != (struct RkRxDic *)NULL) {
      int len = strlen(buf);
      RomkanaTable = malloc(len + 1);
      if (RomkanaTable) {
	strcpy(RomkanaTable, buf);
      }
      if (ckverbose == CANNA_FULL_VERBOSE) {
        printf("ローマ字かな変換テーブルは \"%s\" を用います。\n", buf);
      }
    }
    else { /* オープンできなかった */
      if (ckverbose) {
        printf("ローマ字かな変換テーブル \"%s\" がオープンできません。\n",
               buf);
      }
      sprintf(buf, "\245\267\245\271\245\306\245\340\244\316\245\355\241\274"
	"\245\336\273\372\244\253\244\312\312\321\264\271\245\306\241\274"
	"\245\326\245\353\244\254\245\252\241\274\245\327\245\363\244\307"
	"\244\255\244\336\244\273\244\363\241\243");
         /* システムのローマ字かな変換テーブルがオープンできません。 */
      addWarningMesg(buf);
    }
#ifdef USE_MALLOC_FOR_BIG_ARRAY
    (void)free(buf);
#endif
  }

#ifndef NOT_ENGLISH_TABLE
  if (EnglishTable && (!RomkanaTable || strcmp(RomkanaTable, EnglishTable))) {
    /* RomkanaTable と EnglishTable が一緒だったらだめ */
    englishdic = OpenRoma(EnglishTable);
  }
#endif

  /* ユーザモードの初期化 */
  for (extrafunc1 = extrafuncp ; extrafunc1 ; extrafunc1 = extrafunc1->next) {
    /* ローマ字かな変換テーブルのオープン */
    if (extrafunc1->keyword == EXTRA_FUNC_DEFMODE) {
      if (extrafunc1->u.modeptr->romaji_table) {
        if (RomkanaTable &&
            !strcmp(RomkanaTable,
		    (char *)extrafunc1->u.modeptr->romaji_table)) {
	  extrafunc1->u.modeptr->romdic = romajidic;
	  extrafunc1->u.modeptr->romdic_owner = 0;
        }
#ifndef NOT_ENGLISH_TABLE
        else if (EnglishTable &&
	         !strcmp(EnglishTable,
			 (char *)extrafunc1->u.modeptr->romaji_table)) {
	  extrafunc1->u.modeptr->romdic = englishdic;
	  extrafunc1->u.modeptr->romdic_owner = 0;
        }
#endif
        else {
	  for (extrafunc2 = extrafuncp ; extrafunc1 != extrafunc2 ;
					extrafunc2 = extrafunc2->next) {
	    if (extrafunc2->keyword == EXTRA_FUNC_DEFMODE &&
		extrafunc2->u.modeptr->romaji_table) {
	      if (!strcmp((char *)extrafunc1->u.modeptr->romaji_table,
			  (char *)extrafunc2->u.modeptr->romaji_table)) {
	        extrafunc1->u.modeptr->romdic = extrafunc2->u.modeptr->romdic;
	        extrafunc1->u.modeptr->romdic_owner = 0;
	        break;
	      }
	    }
	  }
	  if (extrafunc2 == extrafunc1) {
	    extrafunc1->u.modeptr->romdic =
              OpenRoma(extrafunc1->u.modeptr->romaji_table);
	    extrafunc1->u.modeptr->romdic_owner = 1;
	  }
        }
      }
      else {
        extrafunc1->u.modeptr->romdic = (struct RkRxDic *)0; /* nilですよ！ */
        extrafunc1->u.modeptr->romdic_owner = 0;
      }
    }
  }

  return 0;
}

/* ローマ字かな変換テーブルのクローズ */

extern keySupplement keysup[];
extern exp(void) RkwCloseRoma pro((struct RkRxDic *));

void
RomkanaFin()
{
  extern char *RomkanaTable, *EnglishTable;
  extern nkeysup;
  int i;

  /* ローマ字かな変換テーブルのクローズ */
  if (romajidic != (struct RkRxDic *)NULL) {
    RkwCloseRoma(romajidic);
  }
  if (RomkanaTable) {
    free(RomkanaTable);
    RomkanaTable = (char *)NULL;
  }
#ifndef NOT_ENGLISH_TABLE
  if (englishdic != (struct RkRxDic *)NULL) {
    RkwCloseRoma(englishdic);
  }
  if (EnglishTable) {
    free(EnglishTable);
    EnglishTable = (char *)NULL;
  }
#endif
  /* ローマ字かな変換ルールの補足のための領域の解放 */
  for (i = 0 ; i < nkeysup ; i++) {
    if (keysup[i].cand) {
      free((char *)keysup[i].cand);
      keysup[i].cand = (wchar_t **)NULL;
    }
    if (keysup[i].fullword) {
      free((char *)keysup[i].fullword);
      keysup[i].fullword = (wchar_t *)NULL;
    }
  }
  nkeysup = 0;
}

/* cfunc newYomiContext

  yomiContext 構造体を一つ作り返す。

 */

yomiContext
newYomiContext(buf, bufsize, allowedc, chmodinhibit,
	       quitTiming, hinhibit)
     wchar_t *buf;
     int bufsize;
     int allowedc, chmodinhibit, quitTiming, hinhibit;
{
  yomiContext ycxt;

  ycxt = (yomiContext)malloc(sizeof(yomiContextRec));
  if (ycxt) {
    bzero(ycxt, sizeof(yomiContextRec));
    ycxt->id = YOMI_CONTEXT;
    ycxt->allowedChars = allowedc;
    ycxt->generalFlags = chmodinhibit ? CANNA_YOMI_CHGMODE_INHIBITTED : 0;
    ycxt->generalFlags |= quitTiming ? CANNA_YOMI_END_IF_KAKUTEI : 0;
    ycxt->savedFlags = (long)0;
    ycxt->henkanInhibition = hinhibit;
    ycxt->n_susp_chars = 0;
    ycxt->retbufp = ycxt->retbuf = buf;
    ycxt->romdic = (struct RkRxDic *)0;
    ycxt->myEmptyMode = (KanjiMode)0;
    ycxt->last_rule = 0;
    if ((ycxt->retbufsize = bufsize) == 0) {
      ycxt->retbufp = 0;
    }
    ycxt->right = ycxt->left = (tanContext)0;
    ycxt->next = (mode_context)0;
    ycxt->prevMode = 0;

    /* 変換の分 */
    ycxt->nbunsetsu = 0;  /* 文節の数、これで読みモードかどうかの判定もする */
    ycxt->context = -1;
    ycxt->kouhoCount = 0;
    ycxt->allkouho = (wchar_t **)0;
    ycxt->curbun = 0;
    ycxt->curIkouho = 0;  /* カレント候補 */
    ycxt->proctime = ycxt->rktime = 0;

    /* 逐次の分 */
    ycxt->ys = ycxt->ye = ycxt->cStartp = ycxt->cRStartp = ycxt->status = 0;
  }
  return ycxt;
}

/*

  GetKanjiString は漢字かな混じり文を取ってくる関数である。実際には
  empty モードを設定するだけでリターンする。最終的な結果が buf で指定
  されたバッファに格納され exitCallback が呼び出されることによって呼び
  出し側は漢字かな混じり文字を得ることができる。

  第２引数の ycxt は通常は０を指定する。アルファベットモードから日本語
  モードへの切り替えに際してのみは uiContext の底に保存してあるコンテ
  キストを用いる。アルファベットモードと日本語モードとの切り替えはスタッ
  ク上に積み込まれたモードの push/pop 操作ではなく、スワップ上のモード
  の一番上の要素の入れ替えになる。

  ３つの Callback のうち、exitCallback はひょっとしたら使われないで、
  everyTimeCallback と quitCallback しか用いないかも知れない。

 */

yomiContext
GetKanjiString(d, buf, bufsize, allowedc, chmodinhibit,
	       quitTiming, hinhibit,
	       everyTimeCallback, exitCallback, quitCallback)
     uiContext d;
     wchar_t *buf;
     int bufsize, allowedc, chmodinhibit, quitTiming, hinhibit;
     canna_callback_t everyTimeCallback, exitCallback, quitCallback;
{
  extern KanjiModeRec empty_mode;
  yomiContext yc;

  if ((pushCallback(d, d->modec, everyTimeCallback, exitCallback, quitCallback,
		    NO_CALLBACK)) == (struct callback *)0) {
    return (yomiContext)0;
  }

  yc = newYomiContext(buf, bufsize, allowedc, chmodinhibit,
		      quitTiming, hinhibit);
  if (yc == (yomiContext)0) {
    popCallback(d);
    return (yomiContext)0;
  }
  yc->romdic = romajidic;
  yc->majorMode = d->majorMode;
  yc->minorMode = CANNA_MODE_HenkanMode;
  yc->next = d->modec;
  d->modec = (mode_context)yc;
  /* 前のモードの保存 */
  yc->prevMode = d->current_mode;
  /* モード変更 */
  d->current_mode = yc->curMode = yc->myEmptyMode = &empty_mode;
  return yc;
}

/* cfuncdef

   popYomiMode -- 読みモードをポップアップする。

 */

void
popYomiMode(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  d->modec = yc->next;
  d->current_mode = yc->prevMode;

  if (yc->context >= 0) {
    RkwCloseContext(yc->context);
    yc->context = -1;
  }

  freeYomiContext(yc);
}

/* cfuncdef

  checkIfYomiExit -- 読みモードが終了かどうかを調べて値を返すフィルタ

  このフィルタは読みモードの各関数で値を返そうとする時に呼ぶ。読みモー
  ドでの処理が終了するところであれば、読みモードを終了し、uiContext に
  プッシュされていたローカルデータやモード構造体がポップされる。

  ローカルデータに exitCallback が定義されていなければいかなる場合にも
  構造体のポップアップは行われない。

  今のところ、読みモードの終了は次のような場合が考えられる。

  (1) C-m が確定読みの最後の文字として返された時。(変換許可の時)

  (2) 確定文字列が存在する場合。(変換禁止の時)

  quit で読みモードを終了する時は?他の関数?を呼ぶ。

 */

static
checkIfYomiExit(d, retval)
uiContext d;
int retval;
{
  yomiContext yc = (yomiContext)d->modec;

  if (retval <= 0) {
    /* 確定文字列がないかエラーの場合 ≡ exit ではない */
    return retval;
  }
  if (yc->retbufp && yc->retbufsize - (yc->retbufp - yc->retbuf) > retval) {
    /* 文字列格納バッファがあって、確定した文字列よりもあまっている領
       域が長いのであれば格納バッファに確定した文字列をコピーする */
    WStrncpy(yc->retbufp, d->buffer_return, retval);
    yc->retbufp[retval] = (wchar_t)0;
    yc->retbufp += retval;
  }
  if (yc->generalFlags & CANNA_YOMI_END_IF_KAKUTEI
      || d->buffer_return[retval - 1] == '\n') {
    /* 変換が禁止されているとしたら exit */
    /* そうでない場合は、\n が入っていたら exit */
    d->status = EXIT_CALLBACK;
    if (!(d->cb && d->cb->func[EXIT_CALLBACK] == NO_CALLBACK)) {
      d->status = EXIT_CALLBACK;
      popYomiMode(d);
    }
  }
  return retval;
}

static
checkIfYomiQuit(d, retval)
uiContext d;
int retval;
/* ARGSUSED */
{
#ifdef QUIT_IN_YOMI /* コメントアウトする目的の ifdef */
  yomiContext yc = (yomiContext)d->modec;

  if (d->cb && d->cb->func[QUIT_CALLBACK] == NO_CALLBACK) {
    /* コールバックがない場合

       こんなチェックを親切に行うのは、読みモードが非常に基本的なモード
       であり、完全に抜けるときにわざわざポップアップしてもすぐにプッシュ
       する場合が多いと考えられて処理が無駄だからである。

     */
  }
  else {
    d->status = QUIT_CALLBACK;
    popYomiMode(d);
  }
#endif /* QUIT_IN_YOMI */
  return retval;
}

#ifdef __STDC__
void fitmarks(yomiContext);
#endif

void
fitmarks(yc)
yomiContext yc;
{
  if (yc->kRStartp < yc->pmark) {
    yc->pmark = yc->kRStartp;
  }
  if (yc->kRStartp < yc->cmark) {
    yc->cmark = yc->kRStartp;
  }
}

/* 直前に未変換文字列がないかどうか確認 */
void
ReCheckStartp(yc)
yomiContext yc;
{
  int r = yc->rStartp, k = yc->kRStartp, i;

  do {
    yc->kRStartp--;
    yc->rStartp--;
  } while ( yc->kRStartp >= 0
	   && !(yc->kAttr[yc->kRStartp] & HENKANSUMI)
	   );
  yc->kRStartp++;
  yc->rStartp++;

  /* 未変換部に先頭マークが付いていた場合は先頭マークをはずす。

     未変換部の先頭に関しては先頭マークを付けておく。
     未変換部があった場合(kRStartp < k)、それが、kCurs よりも
     左側であれば先頭フラグを落とす。 */

  if (yc->kRStartp < k && k < yc->kCurs) {
    yc->kAttr[k] &= ~SENTOU;
    yc->rAttr[r] &= ~SENTOU;
  }
  for (i = yc->kRStartp + 1 ; i < k ; i++) {
    yc->kAttr[i] &= ~SENTOU;
  }
  for (i = yc->rStartp + 1 ; i < r ; i++) {
    yc->rAttr[i] &= ~SENTOU;
  }
}

extern void setMode pro((uiContext d, tanContext tan, int forw));

void
removeCurrentBunsetsu(d, tan)
uiContext d;
tanContext tan;
{
  if (tan->left) {
    tan->left->right = tan->right;
    d->modec = (mode_context)tan->left;
    d->current_mode = tan->left->curMode;
    setMode(d, tan->left, 0);
  }
  if (tan->right) {
    tan->right->left = tan->left;
    d->modec = (mode_context)tan->right;
    d->current_mode = tan->right->curMode;
    setMode(d, tan->right, 1);
  }
  switch (tan->id) {
  case YOMI_CONTEXT:
    freeYomiContext((yomiContext)tan);
    break;
  case TAN_CONTEXT:
    freeTanContext(tan);
    break;
  }
}

/* tabledef

 charKind -- キャラクタの種類のテーブル

 0x20 から 0x7f までのキャラクタの種類を表すテーブルである。

 3: 数字
 2: １６進数として用いられる英字
 1: それ以外の英字
 0: その他

 となる。

 */

static BYTE charKind[] = {
/*sp !  "  #  $  %  &  '  (  )  *  +  ,  -  .  / */
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/*0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ? */
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 1, 1, 1,
/*@  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O */
  1, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2,
/*P  Q  R  S  T  U  V  W  X  Y  X  [  \  ]  ^  _ */
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
/*`  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o */
  1, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2,
/*p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  DEL */
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1,
};

/*
  YomiInsert -- ローマ字を１文字挿入する関数

  */

static makePhonoOnBuffer();

void
restoreChikujiIfBaseChikuji(yc)
yomiContext yc;
{
  if (!chikujip(yc) && (yc->generalFlags & CANNA_YOMI_BASE_CHIKUJI)) {
    yc->generalFlags &= ~CANNA_YOMI_BASE_CHIKUJI;
    yc->generalFlags |= CANNA_YOMI_CHIKUJI_MODE;
    yc->minorMode = getBaseMode(yc);
  }
}

int YomiInsert pro((uiContext));

YomiInsert(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;
  int subst, autoconvert = (yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE);
  int kugiri = 0;
#ifdef USE_ROMKANATABLE_FOR_KANAKEY
  wchar_t key = 0;
#endif

  d->nbytes = 0;
  if (autoconvert) {
    if (yc->status & CHIKUJI_ON_BUNSETSU) {
      yc->status &= ~CHIKUJI_OVERWRAP;
      if (yc->kCurs != yc->kEndp) {
	yc->rStartp = yc->rCurs = yc->rEndp;
	yc->kRStartp = yc->kCurs = yc->kEndp;
      }
    }
    else {
      if (yc->rEndp == yc->rCurs) {
	yc->status &= ~CHIKUJI_OVERWRAP;
      }
      if (yc->kCurs < yc->ys) {
	yc->ys = yc->kCurs;
      }
    }
  }

  if (yc->allowedChars == CANNA_NOTHING_ALLOWED)/* どのキーも受付けない */
    return NothingChangedWithBeep(d);
  if  (yc->rEndp >= ROMAJILIMIT
       || yc->kEndp >= KANALIMIT
       /* 渋い計算をしている
       || (chc && yc->rEndp + chc->hc->ycx->rEndp > ROMAJILIMIT)*/) {
    return NothingChangedWithBeep(d);
  }

  fitmarks(yc);

  if (0xa0 < d->ch && d->ch < 0xe0) {
#ifdef USE_ROMKANATABLE_FOR_KANAKEY
    key = d->buffer_return[0];
#else
    if (yc->allowedChars == CANNA_NOTHING_RESTRICTED) {
      return KanaYomiInsert(d); /* callback のチェックは KanaYomiInsert で! */
    }
    else {
      return NothingChangedWithBeep(d);
    }
#endif
  }

  /*   (d->ch & ~0x1f) == 0x1f < (unsigned char)d->ch */
  if (!(d->ch & ~0x1f) && yc->allowedChars != CANNA_NOTHING_RESTRICTED
      || (d->ch < 0x80 ? charKind[d->ch - 0x20] : 1) < yc->allowedChars) {
    /* 前の行、USE_ROMKANATABLE_FOR_KANAKEY のときにまずい */
    /* 0x20 はコントロールキャラクタの分 */
    return NothingChangedWithBeep(d);
  }

  if (yc->allowedChars != CANNA_NOTHING_RESTRICTED) {
    /* allowed all 以外ではローマ字かな変換を行わない */
    wchar_t romanBuf[4]; /* ２バイトで十分だと思うけどね */
    int len;
#ifdef USE_ROMKANATABLE_FOR_KANAKEY
    wchar_t tempc = key ? key : (wchar_t)d->ch;
#else
    wchar_t tempc = (wchar_t)d->ch;
#endif
    romajiReplace(0, &tempc, 1, SENTOU);

    len = RkwCvtNone(romanBuf, 4, &tempc, 1);

    if (yc->generalFlags & CANNA_YOMI_KAKUTEI) { /* 確定しちゃう */
      WStrncpy(d->buffer_return + d->nbytes, yc->kana_buffer, yc->kCurs);
      /* ローマ字の断片が残っていることはないので、yc->kRStartp でなくて、
	 yc->kCurs が使える */
      d->nbytes += yc->kCurs;
      romajiReplace(-yc->rCurs, (wchar_t *)0, 0, 0);
      kanaReplace(-yc->kCurs, (wchar_t *)0, 0, 0);

      WStrncpy(d->buffer_return + d->nbytes, romanBuf, len);
      d->nbytes += len;
      len = 0;
    }

    kanaReplace(0, romanBuf, len, HENKANSUMI);
    yc->kAttr[yc->kRStartp] |= SENTOU;
    yc->rStartp = yc->rCurs;
    yc->kRStartp = yc->kCurs;
  }
  else { /* ローマ字カナ変換する場合 */
#ifdef USE_ROMKANATABLE_FOR_KANAKEY
    wchar_t tempc = key ? key : (wchar_t)d->ch;
#else
    wchar_t tempc = (wchar_t)d->ch;
#endif
    int ppos;
    if (cannaconf.BreakIntoRoman)
      yc->generalFlags |= CANNA_YOMI_BREAK_ROMAN;

    /* 直前に未変換文字列がないかどうか確認 */

    if (yc->kCurs == yc->kRStartp) {
      ReCheckStartp(yc);
    }

    /* まずカーソル部分にローマ字を１文字入れる */

    romajiReplace(0, &tempc, 1, (yc->rStartp == yc->rCurs) ? SENTOU : 0);

    ppos = yc->kRStartp;
    kanaReplace(0, &tempc, 1, (yc->kRStartp == yc->kCurs) ? SENTOU : 0);

#ifdef USE_ROMKANATABLE_FOR_KANAKEY
    kugiri = makePhonoOnBuffer(d, yc, key ? key : (unsigned char)d->ch, 0, 0);
#else
    kugiri = makePhonoOnBuffer(d, yc, (unsigned char)d->ch, 0, 0);
#endif

    if (kugiri && autoconvert) {
      if (ppos < yc->ys) {
	yc->ys = ppos;
      }
      if ((subst = ChikujiSubstYomi(d)) < 0) {
	makeGLineMessageFromString(d, jrKanjiError);
	if (subst == -2) {
	  TanMuhenkan(d);
	}
	else {
	  makeYomiReturnStruct(d);
	}
	return 0; /* 下まで行かなくていいのかなあ */
      }
    }
  }

  debug_yomi(yc);
  makeYomiReturnStruct(d);

  if (!yc->kEndp && !(autoconvert && yc->nbunsetsu)) {
    if (yc->left || yc->right) {
      removeCurrentBunsetsu(d, (tanContext)yc);
    }
    else {
      /* 未確定文字列が全くなくなったのなら、φモードに遷移する */
      restoreChikujiIfBaseChikuji(yc);
      d->current_mode = yc->curMode = yc->myEmptyMode;
      d->kanji_status_return->info |= KanjiEmptyInfo;
    }
    currentModeInfo(d);
  }

  return d->nbytes;
}

/* cfuncdef

   findSup -- supkey の中からキーに一致するものを探す。

   返る値は supkey の中で key が一致するものが何番目に入っているかを表す。
   何番目と言うのは１から始まる値。

   見つからない時は０を返す。
 */

int findSup pro((wchar_t));

#ifdef __STDC__
findSup(wchar_t key)
#else
findSup(key)
wchar_t key;
#endif
{
  int i;
  extern nkeysup;

  for (i = 0 ; i < nkeysup ; i++) {
    if (key == keysup[i].key) {
      return i + 1;
    }
  }
  return 0;
}

/* cfuncdef

   makePhonoOnBuffer -- yomiContext のバッファ上でキー入力→表音文字変換を
   する

   変換にひと区切りが付いた時点で 1 を返す。それ以外の場合には 0 を返す。

   最後から２つめの flag は RkwMapPhonogram に渡すフラグで、
   最後の english と言うのは英単語カナ変換をするかどうかを表すフラグ

 */

static
makePhonoOnBuffer(d, yc, key, flag, english)
uiContext d;
yomiContext yc;
unsigned char key;
int flag, english;
{
  int i, n, m, t, sm, henkanflag, prevflag, cond;
  int retval = 0;
  int sup = 0;
  int engflag = (english && englishdic);
  int engdone = 0;
  wchar_t *subp;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  wchar_t kana_char[1024], sub_buf[1024];
#else
  wchar_t *kana_char, *sub_buf;

  kana_char = (wchar_t *)malloc(sizeof(wchar_t) * 1024);
  sub_buf = (wchar_t *)malloc(sizeof(wchar_t) * 1024);
  if (!kana_char || !sub_buf) {
    if (kana_char) {
      (void)free((char *)kana_char);
    }
    if (sub_buf) {
      (void)free((char *)sub_buf);
    }
    return 0;
  }
#endif

  if (cannaconf.ignore_case) flag |= RK_IGNORECASE;

  /* 未変換ローマ文字列のかな変換 */
  for (;;) {
#ifndef USE_ROMKANATABLE_FOR_KANAKEY
    if ((flag & RK_FLUSH) &&
	yc->kRStartp != yc->kCurs &&
	!WIsG0(yc->kana_buffer[yc->kCurs - 1])) {
      /* アスキー文字が入っているわけでなかったら */
      kana_char[0] = yc->kana_buffer[yc->kRStartp];
      n = m = 1; t = 0;
      henkanflag = HENKANSUMI;
    }
    /* 補助マッピングの調査 */
    else
#endif
      if ((cond = (!(yc->generalFlags & CANNA_YOMI_ROMAJI) &&
		   !(yc->generalFlags & CANNA_YOMI_IGNORE_USERSYMBOLS) &&
		   (yc->kCurs - yc->kRStartp) == 1 &&
		   (sup = findSup(yc->kana_buffer[yc->kRStartp]))) )
	  && keysup[sup - 1].ncand > 0) {
      n = 1; t = 0;
      WStrcpy(kana_char, keysup[sup - 1].cand[0]);
      m = WStrlen(kana_char);
      /* defsymbol の新しい機能に対応した処理 */
      yc->romaji_buffer[yc->rStartp] = keysup[sup - 1].xkey;
      henkanflag = HENKANSUMI | SUPKEY;
    }
    else {
      if (cond) { /* && keysup[sup - 1].ncand == 0 */
      /* defsymbol の新しい機能に対応した処理。入力文字自身を置き換える */
	yc->kana_buffer[yc->kRStartp] =
	  yc->romaji_buffer[yc->rStartp] = keysup[sup - 1].xkey;
      }
      if (yc->romdic != (struct RkRxDic *)NULL
	  && !(yc->generalFlags & CANNA_YOMI_ROMAJI)) {
	if (engflag &&
	    RkwMapPhonogram(englishdic, kana_char, 1024,
			    yc->kana_buffer + yc->kRStartp,
			    yc->kCurs - yc->kRStartp,
			    (wchar_t)key,
			    flag, &n, &m, &t, &yc->last_rule) &&
	    n > 0) {
	  henkanflag = HENKANSUMI | GAIRAIGO;
	  engdone = 1;
	}
	else if (engflag && 0 == n /* 上の RkwMapPhonogram で得た値 */ &&
		 RkwMapPhonogram(englishdic, kana_char, 1024,
				 yc->kana_buffer + yc->kRStartp,
				 yc->kCurs - yc->kRStartp,
				 (wchar_t)key,
				 flag | RK_FLUSH,
				 &n, &m, &t, &yc->last_rule) &&
		 n > 0) {
	  henkanflag = HENKANSUMI | GAIRAIGO;
	  engdone = 1;
	}
	else {
	  engflag = 0;
	  if (RkwMapPhonogram(yc->romdic, kana_char, 1024,
			      yc->kana_buffer + yc->kRStartp,
			      yc->kCurs - yc->kRStartp,
			      (wchar_t) key,
			      flag | RK_SOKON, &n, &m, &t, &yc->last_rule)) {
	    /* RK_SOKON を付けるのは旧辞書用 */
	    henkanflag = HENKANSUMI;
	  }
	  else {
	    henkanflag = 0;
	  }
	  if (n > 0 && !engdone) {
	    engflag = (english && englishdic);
	  }
	}
	if (n == yc->kCurs - yc->kRStartp) {
	  key = (unsigned char)0;
	}
      }
      else {
	t = 0;
	henkanflag = (yc->generalFlags & CANNA_YOMI_ROMAJI) ?
	  (HENKANSUMI | STAYROMAJI) : 0;
	m = n = (yc->kCurs - yc->kRStartp) ? 1 : 0;
	WStrncpy(kana_char, yc->kana_buffer + yc->kRStartp, n);
      }
    }

    /* ローマ字のうち n 文字分カナに変換された */

    if (n <= 0) {
      break;
    }
    else {
      int unchanged;

      /* ローマ字かな変換の結果を加工する */
      if (cannaconf.abandonIllegalPhono && !henkanflag && !yc->n_susp_chars) {
	/* 変なローマ字は捨てる */
	sm = 0; subp = sub_buf;
	/* t があるのに henkanflag が 0 のことってないんだけどね */
	/* WStrncpy(subp, kana_char + m, t); */
      }
      else {
	sm = m; subp = kana_char;
	if (yc->generalFlags & (CANNA_YOMI_KATAKANA | CANNA_YOMI_HIRAGANA)) {
	  int tempm;

	  if (yc->generalFlags & CANNA_YOMI_KATAKANA) {
	    tempm = RkwCvtKana(sub_buf, 1024, subp, sm);
	  }
	  else {
	    tempm = RkwCvtHira(sub_buf, 1024, subp, sm);
	  }
	  /* 長さチェックが本当はいるが、駄目のときの処理を考えたくない */
	  WStrncpy(sub_buf + tempm, subp + sm, t);
	  subp = sub_buf;
	  sm = tempm;
	}
	if (yc->generalFlags & (CANNA_YOMI_ZENKAKU | CANNA_YOMI_HANKAKU)) {
	  int tempm;
	  wchar_t *otherp = (subp == sub_buf) ? kana_char : sub_buf;

	  if (yc->generalFlags & CANNA_YOMI_ZENKAKU) {
	    tempm = RkwCvtZen(otherp, 1024, subp, sm);
	  }
	  else {
	    tempm = RkwCvtHan(otherp, 1024, subp, sm);
	  }
	  WStrncpy(otherp + tempm, subp + sm, t);
	  subp = otherp;
	  sm = tempm;
	}

	if (yc->generalFlags & CANNA_YOMI_KAKUTEI) { /* 確定しちゃう */
	  int off;

	  chikujiEndBun(d);
	  WStrncpy(d->buffer_return + d->nbytes,
		   yc->kana_buffer, yc->kRStartp);
	  d->nbytes += yc->kRStartp;

	  off = yc->kCurs - yc->kRStartp;
	  yc->kRStartp = 0;
	  yc->kCurs -= off;
	  kanaReplace(-yc->kCurs, (wchar_t *)0, 0, 0);
	  yc->kCurs += off;

	  WStrncpy(d->buffer_return + d->nbytes, subp, sm);
	  d->nbytes += sm;
	  subp += sm;
	  sm = 0;
	}
      }
      /* ローマ字かな変換の結果をカナバッファに入れる。 */

      unchanged = yc->kCurs - yc->kRStartp - n;
      yc->kCurs -= unchanged;
      prevflag = (yc->kAttr[yc->kRStartp] & SENTOU);
      kanaReplace(-n, subp, sm + t, henkanflag);
      if ( prevflag ) {
	yc->kAttr[yc->kRStartp] |= SENTOU;
      }
      yc->kRStartp += sm;
      if (t == 0 && m > 0 && unchanged) {
	yc->kAttr[yc->kRStartp] |= SENTOU;
      }
      for (i = yc->kRStartp ; i < yc->kCurs ; i++) {
	yc->kAttr[i] &= ~HENKANSUMI; /* HENKANSUMI フラグを取り除く */
      }
      yc->kCurs += unchanged;

      if (t > 0) {
	/* suspend している文字長はローマ字バッファとかなバッファとの
           各文字の対応付けに影響するが、その調整をするための計算 */

	if (yc->n_susp_chars) {
	  yc->n_susp_chars += t - n;
	}
	else {
	  yc->n_susp_chars = SUSPCHARBIAS + t - n;
	}

	/* ついでに次のローマ字かな変換用に key を考えてみる。 */
	key = (unsigned char)yc->kana_buffer[yc->kRStartp + t];
      }
      else if (m > 0) { /* ローマ字とかなの対応を付けるための処理 */
	int n_cor_keys = n -
	  (yc->n_susp_chars ? yc->n_susp_chars - SUSPCHARBIAS : 0);

	retval = 1; /* ひと区切りがついた */
	yc->rStartp += n_cor_keys;
	if (cannaconf.abandonIllegalPhono &&
	    !henkanflag && !yc->n_susp_chars) {
	  yc->rStartp -= n;
	  unchanged = yc->rCurs - yc->rStartp - n;
	  yc->rCurs -= unchanged;
	  romajiReplace(-n, (wchar_t *)0, 0, 0);
	  yc->rCurs += unchanged;
	  retval = 0; /* やっぱり区切りがついていない */
	}
	else if (yc->generalFlags & CANNA_YOMI_KAKUTEI) {
	  int offset = yc->rCurs - yc->rStartp;

	  yc->rCurs -= offset;
	  romajiReplace(-yc->rCurs, (wchar_t *)0, 0, 0);
	  yc->rCurs += offset;
	  retval = 0; /* やっぱり区切りがついていない */
	}
	yc->rAttr[yc->rStartp] |= SENTOU;
	yc->n_susp_chars = /* t ? SUSPCHARBIAS + t : (t は必ず 0)*/ 0;
      }
    }
  }
#ifdef USE_MALLOC_FOR_BIG_ARRAY
  (void)free((char *)kana_char);
  (void)free((char *)sub_buf);
#endif
  return retval;
}

#define KANAYOMIINSERT_BUFLEN 10

/* 以下のいくつかの関数は非常に日本語に依存している。
   かな入力についてもテーブルを使うようにして、依存部分を
   排除するようにしたいものだ */

/*
  dakuonP -- predicate for Japanese voiced sounds (Japanese specific)

  argument:
            ch(wchar_t): character to be inspected

  return value:
            0: Not a voiced sound.
	    1: Semi voiced sound.
	    2: Full voiced sound.
 */

#define DAKUON_HV 1
#define DAKUON_FV 2

static
dakuonP(ch)
wchar_t ch;
{
  static dakuon_first_time = 1;
  static wchar_t hv, fv;

  if (dakuon_first_time) { /* 日本語固有の処理 */
    wchar_t buf[2];

    dakuon_first_time = 0;

    MBstowcs(buf, "\216\336"/* 濁音 */, 2);
    fv = buf[0];
    MBstowcs(buf, "\216\337"/* 半濁音 */, 2);
    hv = buf[0];
  }

  if (ch == hv) {
    return DAKUON_HV;
  }
  else if (ch == fv) {
    return DAKUON_FV;
  }
  else {
    return 0;
  }
}

/*
  growDakuonP -- 濁音が付くかどうか

  引数:
       ch(wchar_t): 調べる対象の文字

  返り値:
       0: 付かない
       1: 「う」
       2: 濁音だけが付く
       3: 半濁音と濁音が付く
 */

#define GROW_U  1
#define GROW_FV 2
#define GROW_HV 3

static
growDakuonP(ch)
wchar_t ch;
{
  /* 濁点が続く可能性がある文字の処理 (う、か〜と、は〜ほ) */
  static dakuon_first_time = 1;
  static wchar_t wu, wka, wto, wha, who;

  if (dakuon_first_time) { /* 日本語固有の処理 */
    wchar_t buf[2];

    dakuon_first_time = 0;

    MBstowcs(buf, "\216\263"/* ウ */, 2);
    wu = buf[0];
    MBstowcs(buf, "\216\266"/* カ */, 2);
    wka = buf[0];
    MBstowcs(buf, "\216\304"/* ト */, 2);
    wto = buf[0];
    MBstowcs(buf, "\216\312"/* ハ */, 2);
    wha = buf[0];
    MBstowcs(buf, "\216\316"/* ホ */, 2);
    who = buf[0];
  }

  if (ch == wu) {
    return GROW_U;
  }
  else if (wka <= ch && ch <= wto) {
    return GROW_FV;
  }
  else if (wha <= ch && ch <= who) {
    return GROW_HV;
  }
  else {
    return 0;
  }
}

static
KanaYomiInsert(d)
uiContext d;
{
  static wchar_t kana[3], *kanap;
  wchar_t buf1[KANAYOMIINSERT_BUFLEN], buf2[KANAYOMIINSERT_BUFLEN];
  /* The array above is not so big (10 wchar_t length) 1996.6.5 kon */
  wchar_t *bufp, *nextbufp;
  int len, replacelen, spos;
  yomiContext yc = (yomiContext)d->modec;
  int dakuon, grow_dakuon;

  yc->generalFlags &= ~CANNA_YOMI_BREAK_ROMAN;
  kana[0] = (wchar_t)0;
  kana[1] = d->buffer_return[0];
  kana[2] = (wchar_t)0;
  kanap = kana + 1;
  replacelen = 0; len = 1;
  romajiReplace(0, kanap, 1, SENTOU);
  yc->rStartp = yc->rCurs;
  if ((dakuon = dakuonP(kanap[0])) != 0) { /* 濁点の処理 */
    if (yc->rCurs > 1) {
      kana[0] = yc->romaji_buffer[yc->rCurs - 2];
      if ((grow_dakuon = growDakuonP(kana[0])) == GROW_HV ||
	  (grow_dakuon && dakuon == DAKUON_FV)) {
	kanap = kana; len = 2; replacelen = -1;
	yc->rAttr[yc->rCurs - 1] &= ~SENTOU;
      }
    }
  }
#ifdef DEBUG
  if (iroha_debug) {
    wchar_t aho[200];

    WStrncpy(aho, kana, len);
    aho[len] = 0;
    fprintf(stderr, "\312\321\264\271\301\260(%s)", aho);
                   /* 変換前 */
  }
#endif
  bufp = kanap; nextbufp = buf1;
  if (yc->generalFlags & CANNA_YOMI_ZENKAKU ||
      !(yc->generalFlags & (CANNA_YOMI_ROMAJI | CANNA_YOMI_HANKAKU))) {
    len = RkwCvtZen(nextbufp, KANAYOMIINSERT_BUFLEN, bufp, len);
    bufp = nextbufp;
    if (bufp == buf1) {
      nextbufp = buf2;
    }
    else {
      nextbufp = buf1;
    }
  }
  if (!(yc->generalFlags & (CANNA_YOMI_ROMAJI | CANNA_YOMI_KATAKANA))) {
    /* ひらがなにする */
    len = RkwCvtHira(nextbufp, KANAYOMIINSERT_BUFLEN, bufp, len);
    bufp = nextbufp;
    if (bufp == buf1) {
      nextbufp = buf2;
    }
    else {
      nextbufp = buf1;
    }
  }

  spos = yc->kCurs + replacelen;
  kanaReplace(replacelen, bufp, len, HENKANSUMI);
  yc->kAttr[spos] |= SENTOU;

  yc->kRStartp = yc->kCurs;
  yc->rStartp = yc->rCurs;
  if (growDakuonP(yc->romaji_buffer[yc->rCurs - 1])) {
    yc->kRStartp--;
    yc->rStartp--;
  }

  if (yc->generalFlags & CANNA_YOMI_KAKUTEI) { /* 確定モードなら */
    int off, i;

    for (i = len = 0 ; i < yc->kRStartp ; i++) {
      if (yc->kAttr[i] & SENTOU) {
	do {
	  len++;
	} while (!(yc->rAttr[len] & SENTOU));
      }
    }

    if (yc->kRStartp < d->n_buffer) {
      WStrncpy(d->buffer_return, yc->kana_buffer, yc->kRStartp);
      d->nbytes = yc->kRStartp;
    }
    else {
      d->nbytes = 0;
    }
    off = yc->kCurs - yc->kRStartp;
    yc->kCurs -= off;
    kanaReplace(-yc->kCurs, (wchar_t *)0, 0, 0);
    yc->kCurs += off;
    off = yc->rCurs - len;
    yc->rCurs -= off;
    romajiReplace(-yc->rCurs, (wchar_t *)0, 0, 0);
    yc->rCurs += off;
  }
  else {
    d->nbytes = 0;
  }

  if (yc->rStartp == yc->rCurs && yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE &&
      ChikujiSubstYomi(d) == -1) {
    makeRkError(d, "\303\340\274\241\312\321\264\271\244\313\274\272\307\324"
	"\244\267\244\336\244\267\244\277");
                   /* 逐次変換に失敗しました */
    return 0;
  }

  makeYomiReturnStruct(d);

  if (yc->kEndp <= yc->cStartp &&
      !((yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) && yc->nbunsetsu)) {
    if (yc->left || yc->right) {
      removeCurrentBunsetsu(d, (tanContext)yc);
    }
    else {
      /* 未確定文字列が全くなくなったのなら、φモードに遷移する */
      restoreChikujiIfBaseChikuji(yc);
      d->current_mode = yc->curMode = yc->myEmptyMode;
      d->kanji_status_return->info |= KanjiEmptyInfo;
    }
    currentModeInfo(d);
  }

  return d->nbytes;
}

#undef KANAYOMIINSERT_BUFLEN

void
moveStrings(str, attr, start, end, distance)
wchar_t *str;
BYTE *attr;
int  start, end, distance;
{
  int i;

  if (distance > 0) { /* 後ろにずれれば */
    for (i = end ; start <= i ; i--) { /* 後ろからずらす */
      str[i + distance]  = str[i];
      attr[i + distance] = attr[i];
    }
  }
  else if (distance < 0) { /* 前にずれれば */
    for (i = start ; i <= end ; i++) {     /* 前からずらす */
      str[i + distance]  = str[i];
      attr[i + distance] = attr[i];
    }
  }
  /* else { なにもしない } */
}

static
howFarToGoBackward(yc)
yomiContext yc;
{
  if (yc->kCurs <= yc->cStartp) {
    return 0;
  }
  if (!cannaconf.ChBasedMove) {
    BYTE *st = yc->kAttr;
    BYTE *cur = yc->kAttr + yc->kCurs;
    BYTE *p = cur;

    for (--p ; p > st && !(*p & SENTOU) ;) {
      --p;
    }
    if (yc->kAttr + yc->cStartp > p) {
      p = yc->kAttr + yc->cStartp;
    }
    return cur - p;
  }
  return 1;
}

static
howFarToGoForward(yc)
yomiContext yc;
{
  if (yc->kCurs == yc->kEndp) {
    return 0;
  }
  if (!cannaconf.ChBasedMove) {
    BYTE *end = yc->kAttr + yc->kEndp;
    BYTE *cur = yc->kAttr + yc->kCurs;
    BYTE *p = cur;

    for (++p ; p < end && !(*p & SENTOU) ;) {
      p++;
    }
    return p - cur;
  }
  return 1;
}

static int YomiBackward pro((uiContext));

static int
YomiBackward(d) /* カーソルの左移動 */
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;
  int howManyMove;

  d->nbytes = 0;
  if (forceRomajiFlushYomi(d))
    return(d->nbytes);

  if ((yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) &&
      !(yc->status & CHIKUJI_OVERWRAP) && yc->nbunsetsu) {
    /* オーバラップじゃないなら */
    yc->status |= CHIKUJI_OVERWRAP;
    moveToChikujiTanMode(d);
    return TanBackwardBunsetsu(d);
  }

  howManyMove = howFarToGoBackward(yc);
  if (howManyMove) {
    yc->kCurs -= howManyMove;

    if (yc->kCurs < yc->kRStartp)
      yc->kRStartp = yc->kCurs;   /* 未確定ローマ字カーソルもずらす */

    /* かなのポインタが変換されたときの途中のデータでない場合
       (つまり変換の時に先頭のデータだった場合)にはローマ字の
       カーソルもずらす */

    if (yc->kAttr[yc->kCurs] & SENTOU) {
      while ( yc->rCurs > 0 && !(yc->rAttr[--yc->rCurs] & SENTOU) )
	/* EMPTY */
	;
      if (yc->rCurs < yc->rStartp)
	yc->rStartp = yc->rCurs;
    }
  }
  else if (yc->nbunsetsu) { /* 文節があるなら(逐次) */
    yc->curbun = yc->nbunsetsu - 1;
    if (RkwGoTo(yc->context, yc->nbunsetsu - 1) == -1) { /* 最後尾文節へ */
      return makeRkError(d, "\312\270\300\341\244\316\260\334\306\260\244\313"
	"\274\272\307\324\244\267\244\336\244\267\244\277");
                            /* 文節の移動に失敗しました */
    }
    yc->kouhoCount = 0;
    moveToChikujiTanMode(d);
  }
  else if (yc->left) {
    return TbBackward(d);
  }
  else if (!cannaconf.CursorWrap) {
    return NothingChanged(d);
  }
  else if (yc->right) {
    return TbEndOfLine(d);
  }
  else {
    yc->kCurs = yc->kRStartp = yc->kEndp;
    yc->rCurs = yc->rStartp = yc->rEndp;
  }
  yc->status |= CHIKUJI_OVERWRAP;
  makeYomiReturnStruct(d);

  return 0;
}

static YomiNop pro((uiContext));

static
YomiNop(d)
uiContext d;
{
  /* currentModeInfo でモード情報が必ず返るようにダミーのモードを入れておく */
  d->majorMode = d->minorMode = CANNA_MODE_AlphaMode;
  currentModeInfo(d);
  makeYomiReturnStruct(d);
  return 0;
}

static YomiForward pro((uiContext));

static
YomiForward(d) /* カーソルの右移動 */
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;
  int howManyMove;

  d->nbytes = 0;
  if (forceRomajiFlushYomi(d))
    return(d->nbytes);

  if ((yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) &&
      !(yc->status & CHIKUJI_OVERWRAP) && yc->nbunsetsu) {
    yc->status |= CHIKUJI_OVERWRAP;
    moveToChikujiTanMode(d);
    return TanForwardBunsetsu(d);
  }

  howManyMove = howFarToGoForward(yc);
  if (howManyMove) {
    if (yc->kAttr[yc->kCurs] & SENTOU) { /* ローマ字かな変換時先頭だった */
      while ( !yc->rAttr[++yc->rCurs] )
	/* EMPTY */
	; /* 次の先頭までずらす */
      yc->rStartp = yc->rCurs;
    }

    yc->kCurs += howManyMove;   /* 画面の入力位置 カーソルを右にずらす */
    yc->kRStartp = yc->kCurs;
    yc->status &= ~CHIKUJI_ON_BUNSETSU;
  }
  else if (yc->right) {
    return TbForward(d);
  }
  else if (!cannaconf.CursorWrap) {
    return NothingChanged(d);
  }
  else if (yc->left) {
    return TbBeginningOfLine(d);
  }
  else if (yc->nbunsetsu) { /* 文節がある(逐次) */
    yc->kouhoCount = 0;
    yc->curbun = 0;
    if (RkwGoTo(yc->context, 0) == -1) {
      return makeRkError(d, "\312\270\300\341\244\316\260\334\306\260\244\313"
	"\274\272\307\324\244\267\244\336\244\267\244\277");
                            /* 文節の移動に失敗しました */
    }
    moveToChikujiTanMode(d);
  }
  else {
    yc->kRStartp = yc->kCurs = yc->rStartp = yc->rCurs = 0;
  }

  yc->status |= CHIKUJI_OVERWRAP;
  makeYomiReturnStruct(d);
  return 0;
}

static YomiBeginningOfLine pro((uiContext));

static
YomiBeginningOfLine(d) /* カーソルの左端移動 */
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  d->nbytes = 0;
  if (forceRomajiFlushYomi(d))
    return(d->nbytes);

  if (yc->left) {
    return TbBeginningOfLine(d);
  }
  else if (yc->nbunsetsu) { /* 逐次で左側に文節があるなら */
    yc->kouhoCount = 0;
    if (RkwGoTo(yc->context, 0) < 0) {
      return makeRkError(d, "\312\270\300\341\244\316\260\334\306\260\244\313"
	"\274\272\307\324\244\267\244\336\244\267\244\277");
                            /* 文節の移動に失敗しました */
    }
    yc->curbun = 0;
    moveToChikujiTanMode(d);
  }
  else {
    yc->kRStartp = yc->kCurs = yc->cStartp;
    yc->rStartp = yc->rCurs = yc->cRStartp;
  }
  yc->status |= CHIKUJI_OVERWRAP;
  makeYomiReturnStruct(d);
  return(0);
}

static YomiEndOfLine pro((uiContext));

static
YomiEndOfLine(d) /* カーソルの右端移動 */
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  d->nbytes = 0;
  if (forceRomajiFlushYomi(d))
    return(d->nbytes);

  if (yc->right) {
    return TbEndOfLine(d);
  }
  else {
    yc->kRStartp = yc-> kCurs = yc->kEndp;
    yc->rStartp = yc-> rCurs = yc->rEndp;
    yc->status &= ~CHIKUJI_ON_BUNSETSU;
    yc->status |= CHIKUJI_OVERWRAP;
  }
  makeYomiReturnStruct(d);
  return 0;
}

int
forceRomajiFlushYomi(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->kCurs != yc->kRStartp) {
    d->nbytes = 0;
    if (RomajiFlushYomi(d, (wchar_t *)NULL, 0) == 0) { /* empty mode */
      d->more.todo = 1;
      d->more.ch = d->ch;
      d->more.fnum = 0;    /* 上の ch で示される処理をせよ */
      return(1);
    }
  }
  return(0);
}

/* RomajiFlushYomi(d, buffer, bufsize) ユーティリティ関数
 *
 * この関数は、(uiContext)d に蓄えられている読みの情報
 * (yc->romaji_buffer と yc->kana_buffer)を用いて、buffer にその読みをフ
 * ラッシュした結果を返す関数である。フラッシュした結果の文字列の長さ
 * はこの関数の返り値として返される。
 *
 * buffer として NULL が指定された時は、バッファに対する格納は行わない
 *
 * 【作用】
 *
 *    読みを確定する
 *
 * 【引数】
 *
 *    d  (uiContext)  カナ漢字変換構造体
 *    buffer (char *)    読みを返すためのバッファ (NULL 可)
 *
 * 【戻り値】
 *
 *    buffer に格納した文字列の長さ(バイト長)
 *
 * 【副作用】
 *
 */

RomajiFlushYomi(d, b, bsize)
uiContext d;
wchar_t *b;
int bsize;
{
  int ret;
  yomiContext yc = (yomiContext)d->modec;

  yc->generalFlags &= ~CANNA_YOMI_BREAK_ROMAN;

  makePhonoOnBuffer(d, yc, (unsigned char)0, RK_FLUSH, 0);
  yc->n_susp_chars = 0; /* 上の行で保証されるかも知れない */
  yc->last_rule = 0;

  ret = yc->kEndp - yc->cStartp; /* その結果がこの関数の返り値になる */
  if (b) {
    if (bsize > ret) {
      WStrncpy(b, yc->kana_buffer + yc->cStartp, ret);
      b[ret] = '\0';
    }
    else {
      WStrncpy(b, yc->kana_buffer + yc->cStartp, bsize);
      ret = bsize;
    }
  }
  if (ret == 0) { /* 読みが無くなったのならエンプティモードへ */
    d->current_mode = yc->curMode = yc->myEmptyMode;
    /* もっといろいろクリアした方が良いんじゃないの */
  }
  return ret;
}

static int saveFlags pro((yomiContext));

static int
saveFlags(yc)
yomiContext yc;
{
  if (!(yc->savedFlags & CANNA_YOMI_MODE_SAVED)) {
    yc->savedFlags = (yc->generalFlags &
		      (CANNA_YOMI_ATTRFUNCS | CANNA_YOMI_BASE_HANKAKU)) |
			CANNA_YOMI_MODE_SAVED;
    yc->savedMinorMode = yc->minorMode;
    return 1;
  }
  else {
    return 0;
  }
}

void
restoreFlags(yc)
yomiContext yc;
{
  yc->generalFlags &= ~(CANNA_YOMI_ATTRFUNCS | CANNA_YOMI_BASE_HANKAKU);
  yc->generalFlags |= yc->savedFlags
    & (CANNA_YOMI_ATTRFUNCS | CANNA_YOMI_BASE_HANKAKU);
  yc->savedFlags = (long)0;
  yc->minorMode = yc->savedMinorMode;
}

/*
 doYomiKakutei -- 読みを確定させる動作をする。

  retval 0 -- 問題無く確定した。
         1 -- 確定したらなくなった。
        -1 -- エラー？
 */

static int
doYomiKakutei(d)
uiContext d;
{
  int len;

  len = RomajiFlushYomi(d, (wchar_t *)0, 0);
  if (len == 0) {
    return 1;
  }

  return 0;
}

int
xString(str, len, s, e)
wchar_t *str, *s, *e;
int len;
{
  if (e < s + len) {
    len = e - s;
  }
  WStrncpy(s, str, len);
  return len;
}

static int
xYomiKakuteiString(yc, s, e)
yomiContext yc;
wchar_t *s, *e;
{
  return xString(yc->kana_buffer + yc->cStartp, yc->kEndp - yc->cStartp, s, e);
}

static int
xYomiYomi(yc, s, e)
yomiContext yc;
wchar_t *s, *e;
{
  return xString(yc->kana_buffer, yc->kEndp, s, e);
}

static int
xYomiRomaji(yc, s, e)
yomiContext yc;
wchar_t *s, *e;
{
  return xString(yc->romaji_buffer, yc->rEndp, s, e);
}

static void
finishYomiKakutei(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->savedFlags & CANNA_YOMI_MODE_SAVED) {
    restoreFlags(yc);
  }
}

int
appendTan2Yomi(tan, yc)
tanContext tan;
yomiContext yc;
{
  int klen, rlen;

  klen = WStrlen(tan->yomi);
  rlen = WStrlen(tan->roma);

  if (yc->kEndp + klen < ROMEBUFSIZE && yc->rEndp + rlen < ROMEBUFSIZE) {
    WStrcpy(yc->kana_buffer + yc->kEndp, tan->yomi);
    WStrcpy(yc->romaji_buffer + yc->rEndp, tan->roma);
    bcopy(tan->kAttr, yc->kAttr + yc->kEndp, (klen + 1) * sizeof(BYTE));
    bcopy(tan->rAttr, yc->rAttr + yc->rEndp, (rlen + 1) * sizeof(BYTE));
    yc->rEndp += rlen;
    yc->kEndp += klen;
    return 1;
  }
  return 0;
}

static
appendYomi2Yomi(yom, yc)
yomiContext yom, yc;
{
  int rlen, klen;

  rlen = yom->rEndp;
  klen = yom->kEndp;
  if (yc->kEndp + klen < ROMEBUFSIZE && yc->rEndp + rlen < ROMEBUFSIZE) {
    yom->romaji_buffer[rlen] = (wchar_t)'\0';
    yom->kana_buffer[klen] = (wchar_t)'\0';
    WStrcpy(yc->romaji_buffer + yc->rEndp, yom->romaji_buffer);
    WStrcpy(yc->kana_buffer + yc->kEndp, yom->kana_buffer);
    bcopy(yom->kAttr, yc->kAttr + yc->kEndp, (klen + 1) * sizeof(BYTE));
    bcopy(yom->rAttr, yc->rAttr + yc->rEndp, (rlen + 1) * sizeof(BYTE));
    yc->rEndp += rlen;
    yc->kEndp += klen;
    return 1;
  }
  return 0;
}

yomiContext
dupYomiContext(yc)
yomiContext yc;
{
  yomiContext res;

  res = newYomiContext((wchar_t *)NULL, 0, /* 結果は格納しない */
		       CANNA_NOTHING_RESTRICTED,
		       (int)!CANNA_YOMI_CHGMODE_INHIBITTED,
		       (int)!CANNA_YOMI_END_IF_KAKUTEI,
		       CANNA_YOMI_INHIBIT_NONE);
  if (res) {
    res->generalFlags = yc->generalFlags;
    res->status = yc->status;
    res->majorMode = yc->majorMode;
    res->minorMode = yc->minorMode;
    res->myMinorMode = yc->myMinorMode;
    res->curMode = yc->curMode;
    res->myEmptyMode = yc->myEmptyMode;
    res->romdic = yc->romdic;
    res->next = yc->next;
    res->prevMode = yc->prevMode;
    appendYomi2Yomi(yc, res);
  }
  return res;
}


/*
  doMuhenkan -- 無変換処理をする。

  yc から右の tanContext/yomiContext をボツにして、そのなかに格納されていた
  読みを yc にくっつける。
 */

void
doMuhenkan(d, yc)
uiContext d;
yomiContext yc;
{
  tanContext tan, netan, st = (tanContext)yc;
  yomiContext yom;

  /* まず無変換準備処理をする */
  for (tan = st ; tan ; tan = tan->right) {
    if (tan->id == YOMI_CONTEXT) {
      yom = (yomiContext)tan;
      d->modec = (mode_context)yom;
      if (yom->nbunsetsu || (yom->generalFlags & CANNA_YOMI_CHIKUJI_MODE)) {
	tanMuhenkan(d, -1);
      }
      if (yom->jishu_kEndp) {
	leaveJishuMode(d, yom);
      }
      /* else 読みモードではなにもする必要がない。 */
    }
  }

  /* 次に読みなどの文字を取り出す */
  for (tan = st ; tan ; tan = netan) {
    netan = tan->right;
    if (tan->id == TAN_CONTEXT) {
      appendTan2Yomi(tan, yc);
      freeTanContext(tan);
    }
    else if (tan->id == YOMI_CONTEXT) {
      if ((yomiContext)tan != yc) {
	appendYomi2Yomi((yomiContext)tan, yc);
	freeYomiContext((yomiContext)tan);
      }
    }
  }
  yc->rCurs = yc->rStartp = yc->rEndp;
  yc->kCurs = yc->kRStartp = yc->kEndp;
  yc->right = (tanContext)0;
  d->modec = (mode_context)yc;
}

static int
xTanKakuteiString(yc, s, e)
yomiContext yc;
wchar_t *s, *e;
{
  wchar_t *ss = s;
  int i, len, nbun;

  nbun = yc->bunlen ? yc->curbun : yc->nbunsetsu;

  for (i = 0 ; i < nbun ; i++) {
    RkwGoTo(yc->context, i);
    len = RkwGetKanji(yc->context, s, (int)(e - s));
    if (len < 0) {
      if (errno == EPIPE) {
	jrKanjiPipeError();
      }
      jrKanjiError = "\245\253\245\354\245\363\245\310\270\365\312\344\244\362"
	"\274\350\244\352\275\320\244\273\244\336\244\273\244\363\244\307"
	"\244\267\244\277";
                    /* カレント候補を取り出せませんでした */
    }
    else {
      s += len;
    }
  }
  RkwGoTo(yc->context, yc->curbun);

  if (yc->bunlen) {
    len = yc->kEndp - yc->kanjilen;
    if (((int)(e - s)) < len) {
      len = (int)(e - s);
    }
    WStrncpy(s, yc->kana_buffer + yc->kanjilen, len);
    s += len;
  }

  if ((yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) &&
      yc->cStartp < yc->kEndp) {
    len = xYomiKakuteiString(yc, s, e);
    s += len;
  }
  return (int)(s - ss);
}

static int
doJishuKakutei(d, yc)
uiContext d;
yomiContext yc;
{
  exitJishu(d);
  yc->jishu_kEndp = 0;
  return 0;
}


typedef struct _autoDefRec {
    struct _autoDefRec *next;
    int ishira;
    wchar_t yomibuf[ROMEBUFSIZE];
    wchar_t kanabuf[ROMEBUFSIZE];
} autoDefRec, *autoDef;

/*
  doKakutei -- 確定処理をする。

    st から et の直前までの tanContext/yomiContext を確定させる
    s から e の範囲に確定結果が格納される。
    yc_return は yomiContext を一つ残して欲しい場合に、残った yomiContext
    を格納して返すためのアドレス。yc_return がヌルなら、何も残さず free
    する。
    et->left は呼び出したところで 0 にすること。

    確定した文字の長さが返される。

    この関数を呼んだら d->modec が壊れているので入れ直さなければならない

 */

int
doKakutei(d, st, et, s, e, yc_return)
uiContext d;
tanContext st, et;
wchar_t *s, *e;
yomiContext *yc_return;
{
  tanContext tan, netan;
  yomiContext yc;
  int len, res;
  wchar_t *ss = s;
  extern int auto_define;
  autoDef autotop = NULL, autocur;
  KanjiMode kmsv = d->current_mode;

  /* まず確定準備処理をする */
  for (tan = st ; tan != et ; tan = tan->right) {
    if (tan->id == YOMI_CONTEXT) {
      yc = (yomiContext)tan;
      d->modec = (mode_context)yc;
      if (yc->jishu_kEndp) {
	autocur = NULL;
        if (auto_define &&
	    (yc->jishu_kc == JISHU_ZEN_KATA
#ifdef HIRAGANAAUTO
	     || yc->jishu_kc == JISHU_HIRA
#endif
	    ))
	  autocur = (autoDef)malloc(sizeof(autoDefRec));
	if (autocur) {
	  WStrcpy(autocur->yomibuf, yc->kana_buffer);
	  autocur->ishira = (yc->jishu_kc == JISHU_HIRA);
	}
	doJishuKakutei(d, yc);
	if (autocur) {
	  WStrcpy(autocur->kanabuf, yc->kana_buffer);
	  autocur->next = autotop;
	  autotop = autocur;
	}
      }
      else if (!yc->bunlen && /* 文節伸ばし縮め中 */
	       (!yc->nbunsetsu || /* 漢字がないか... */
		(yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE &&
		 yc->cStartp < yc->kEndp))) { /* 読みがまだある .. */
	long savedFlag = yc->generalFlags;
	yc->generalFlags &= ~CANNA_YOMI_KAKUTEI;
	/* base-kakutei だと doYomiKakutei() が呼び出している
	   RomajiFlushYomi() の中で確定文字列が発生し、
	   処理が面倒になるのでとりあえず base-kakutei を寝せる */
	doYomiKakutei(d);
	yc->generalFlags = savedFlag;
      }
    }
  }
  /* doJishuKakutei,doYomiKakuteiでempty_modeに入ることがある */
  d->current_mode = kmsv;

  /* 次に確定文字を取り出す */
  for (tan = st ; tan != et ; tan = tan->right) {
    if (tan->id == TAN_CONTEXT) {
      len = extractTanString(tan, s, e);
    }
    else if (tan->id == YOMI_CONTEXT) {
      yc = (yomiContext)tan;
      d->modec = (mode_context)yc;
      if (yc->nbunsetsu || (yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE)) {
	len = xTanKakuteiString(yc, s, e);
      }
      else { /* else ってことは、読み状態しかない */
	len = xYomiKakuteiString(yc, s, e);
      }
    }
    s += len;
  }
  res = (int)(s - ss);
  if (s < e) {
    *s++ = (wchar_t)'\0';
  }

  /* yomiInfo の処理をする */
  if (yomiInfoLevel > 0) {
    d->kanji_status_return->info |= KanjiYomiInfo;
    for (tan = st ; tan != et ; tan = tan->right) {
      if (tan->id == TAN_CONTEXT) {
	len = extractTanYomi(tan, s, e);
      }
      else if (tan->id == YOMI_CONTEXT) {
	len = xYomiYomi((yomiContext)tan, s, e);
      }
      s += len;
    }
    if (s < e) {
      *s++ = (wchar_t)'\0';
    }

    if (yomiInfoLevel > 1) {
      for (tan = st ; tan != et ; tan = tan->right) {
	if (tan->id == TAN_CONTEXT) {
	  len = extractTanRomaji(tan, s, e);
	}
	else if (tan->id == YOMI_CONTEXT) {
	  len = xYomiRomaji((yomiContext)tan, s, e);
	}
	s += len;
      }
    }
    if (s < e) {
      *s++ = (wchar_t)'\0';
    }
  }

  /* 確定の残処理を行う */
  if (yc_return) {
    *yc_return = (yomiContext)0;
  }
  for (tan = st ; tan != et ; tan = netan) {
    netan = tan->right;
    if (tan->id == TAN_CONTEXT) {
      freeTanContext(tan);
    }
    else { /* tan->id == YOMI_CONTEXT */
      yc = (yomiContext)tan;
      d->modec = (mode_context)yc;

      if (yc->nbunsetsu || (yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE)) {
	if (yc->bunlen) {
	  leaveAdjustMode(d, yc);
	}
	finishTanKakutei(d);
      }
      else { /* ってことは、読み状態しかない */
	finishYomiKakutei(d);
      }
      if (yc_return && !*yc_return) {
	*yc_return = yc;
      }
      else {
	/* とっておくやつがもうあるか、いらないなら、今のは捨てる */
	/* yc->context の close はいらないのかなあ。1996.10.30 今 */
	freeYomiContext(yc);
      }
    }
  }

  if (yc_return) {
    yc = *yc_return;
    if (yc) {
      yc->left = yc->right = (tanContext)0;
    }
  }
  d->modec = (mode_context)0;
  /* 壊れているかも知れないので使い間違わないように壊し尽くしておく */

  /* 字種変換で全角カタカナを確定したら、自動登録する */
  for (autocur = autotop; autocur; autocur = autocur->next) {
    wchar_t line[ROMEBUFSIZE];
    int cnt;
    extern int defaultContext;
    extern char *kataautodic;
#ifdef HIRAGANAAUTO
    extern char *hiraautodic;
#endif

    WStraddbcpy(line, autocur->yomibuf, ROMEBUFSIZE);
    EWStrcat(line, " ");
    EWStrcat(line, "#T30");
    EWStrcat(line, " ");
    cnt = WStrlen(line);
    WStraddbcpy(line + cnt, autocur->kanabuf, ROMEBUFSIZE - cnt);

    if (defaultContext == -1) {
      if ((KanjiInit() < 0) || (defaultContext == -1)) {
	jrKanjiError = KanjiInitError();
        makeGLineMessageFromString(d, jrKanjiError);
        goto return_res;
      }
    }

    if (!autocur->ishira) {
      if (RkwDefineDic(defaultContext, kataautodic, line) != 0) {
        jrKanjiError = "\274\253\306\260\305\320\317\277\244\307\244\255"
                       "\244\336\244\273\244\363\244\307\244\267\244\277";
                         /* 自動登録できませんでした */
        makeGLineMessageFromString(d, jrKanjiError);
	goto return_res;
      }
      else {
        if (cannaconf.auto_sync) {
          (void)RkwSync(defaultContext, kataautodic);
        }
      }
    } else {
#ifdef HIRAGANAAUTO
      if (RkwDefineDic(defaultContext, hiraautodic, line) != 0) {
        jrKanjiError = "\274\253\306\260\305\320\317\277\244\307\244\255"
                       "\244\336\244\273\244\363\244\307\244\267\244\277";
                         /* 自動登録できませんでした */
        makeGLineMessageFromString(d, jrKanjiError);
	goto return_res;
      }
      else {
        if (cannaconf.auto_sync) {
          (void)RkwSync(defaultContext, hiraautodic);
        }
      }
#endif
    }
  }
 return_res:
  while (autotop) {
    autocur = autotop->next;
    free(autotop);
    autotop = autocur;
  }
  return res;
}

/*
  cutOffLeftSide -- 左の方の tanContext を確定させる。

  n -- 左に n 個残して確定する。

 */

int
cutOffLeftSide(d, yc, n)
uiContext d;
yomiContext yc;
int n;
{
  int i;
  tanContext tan = (tanContext)yc, st;

  for (i = 0 ; i < n && tan ; i++) {
    tan = tan->left;
  }
  if (tan && tan->left) {
    st = tan->left;
    while (st->left) {
      st = st->left;
    }
    d->nbytes = doKakutei(d, st, tan, d->buffer_return,
			  d->buffer_return + d->n_buffer, (yomiContext *)0);
    d->modec = (mode_context)yc;
    tan->left = (tanContext)0;
    return 1;
  }
  return 0;
}

extern KanjiModeRec cy_mode;

int YomiKakutei pro((uiContext));

int
YomiKakutei(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec, newFilledYomiContext();
  tanContext leftmost;
  int len, res;
  wchar_t *s = d->buffer_return, *e = s + d->n_buffer;
  mode_context next = yc->next;
  KanjiMode prev = yc->prevMode;
  long prevflags;

  prevflags = (yc->id == YOMI_CONTEXT) ?
    yc->generalFlags : ((tanContext)yc)->generalFlags;

  d->kanji_status_return->length = 0;
  d->nbytes = 0;

  leftmost = (tanContext)yc;
  while (leftmost->left) {
    leftmost = leftmost->left;
  }

  len = doKakutei(d, leftmost, (tanContext)0, s, e, &yc);

  if (!yc) {
    yc = newFilledYomiContext(next, prev);
    yc->generalFlags = prevflags;
    yc->minorMode = getBaseMode(yc);
  }
  d->modec = (mode_context)yc;
  if (!yc) {
    freeRomeStruct(d);
    return -1; /* 本当にこれでいいのか？→いい 1994.2.23 kon */
  }
  d->current_mode = yc->curMode;
  d->nbytes = len;

  res = YomiExit(d, d->nbytes);
  currentModeInfo(d);
  return res;
}

/* 全く 0 にするわけではないので注意 */

void
clearYomiContext(yc)
yomiContext yc;
{
  yc->rStartp = 0;
  yc->rCurs = 0;
  yc->rEndp = 0;
  yc->romaji_buffer[0] = (wchar_t)0;
  yc->rAttr[0] = SENTOU;
  yc->kRStartp = 0;
  yc->kCurs = 0;
  yc->kEndp = 0;
  yc->kana_buffer[0] = (wchar_t)0;
  yc->kAttr[0] = SENTOU;
  yc->pmark = yc->cmark = 0;
  yc->englishtype = CANNA_ENG_KANA;
  yc->cStartp = yc->cRStartp = 0;
  yc->jishu_kEndp = 0;
  yc->n_susp_chars = 0;
}

static int
clearChikujiContext(yc)
     yomiContext yc;
{
  clearYomiContext(yc);
  yc->status &= CHIKUJI_NULL_STATUS;
  yc->ys = yc->ye = yc->cStartp;
  clearHenkanContext(yc);
  return 0;
}


/* RomajiClearYomi(d) ユーティリティ関数
 *
 * この関数は、(uiContext)d に蓄えられている読みの情報
 * をクリアする。
 *
 * 【作用】
 *
 *    読みをクリアする。
 *
 * 【引数】
 *
 *    d  (uiContext)  カナ漢字変換構造体
 *
 * 【戻り値】
 *
 *    なし。
 *
 * 【副作用】
 *
 *    yc->rEndp = 0;
 *    yc->kEndp = 0; 等
 */

void
RomajiClearYomi(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) {
    if (yc->context >= 0) {
      RkwEndBun(yc->context, 0);
      abandonContext(d, yc);
    }
    clearChikujiContext(yc);
  }
  else {
    clearYomiContext(yc);
  }
}

YomiExit(d, retval)
uiContext d;
int retval;
{
  yomiContext yc = (yomiContext)d->modec;

  RomajiClearYomi(d);

  /* 確定してしまったら、読みがなくなるのでφモードに遷移する。 */
  restoreChikujiIfBaseChikuji(yc);
  d->current_mode = yc->curMode = yc->myEmptyMode;
  d->kanji_status_return->info |= KanjiEmptyInfo;

  return checkIfYomiExit(d, retval);
}

/* RomajiStoreYomi(d, kana) ユーティリティ関数
 *
 * この関数は、(uiContext)d に読みの情報をストアする。
 *
 * 【作用】
 *
 *    読みを格納する。
 *
 * 【引数】
 *
 *    d    (uiContext)  カナ漢字変換構造体
 *    kana (wchar_t *) かな文字列
 *    roma (wchar_t *) ローマ字文字列
 * 【戻り値】
 *
 *    なし。
 *
 * 【副作用】
 *
 *    yc->rEndp = WStrlen(kana);
 *    yc->kEndp = WStrlen(kana); 等
 */

void
RomajiStoreYomi(d, kana, roma)
uiContext d;
wchar_t *kana, *roma;
{
  int i, ylen, rlen, additionalflag;
  yomiContext yc = (yomiContext)d->modec;

  rlen = ylen = WStrlen(kana);
  if (roma) {
    rlen = WStrlen(roma);
    additionalflag = 0;
  }
  else {
    additionalflag = SENTOU;
  }
  WStrcpy(yc->romaji_buffer, (roma ? roma : kana));
  yc->rStartp = rlen;
  yc->rCurs = rlen;
  yc->rEndp = rlen;
  WStrcpy(yc->kana_buffer, kana);
  yc->kRStartp = ylen;
  yc->kCurs = ylen;
  yc->kEndp = ylen;
  for (i = 0 ; i < rlen ; i++) {
    yc->rAttr[i] = additionalflag;
  }
  yc->rAttr[0] |= SENTOU;
  yc->rAttr[i] = SENTOU;
  for (i = 0 ; i < ylen ; i++) {
    yc->kAttr[i] = HENKANSUMI | additionalflag;
  }
  yc->kAttr[0] |= SENTOU;
  yc->kAttr[i] = SENTOU;
}

/*
  KanaDeletePrevious -- 色々なところから使われる。

*/

KanaDeletePrevious(d)/* カーソルの左の文字の削除 */
uiContext d;
{
  int howManyDelete;
  int prevflag;
  yomiContext yc = (yomiContext)d->modec;

  /* カーソルの左側を削除するのだが、カーソルの左側が

    (1) ローマ字かな変換の途中の状態であり、アルファベットになっている時、
    (2) 先頭であるとき

    などが考えられる。(要は整理されていないのでもっとありそう)
   */

  if (!yc->kCurs) { /* 左端のとき */
    d->kanji_status_return->length = -1;
    return 0;
  }
  yc->last_rule = 0;
  howManyDelete = howFarToGoBackward(yc);
  if (howManyDelete > 0 && (yc->generalFlags & CANNA_YOMI_BREAK_ROMAN)
      && (yc->kAttr[yc->kCurs] & SENTOU)) {
    /*
     * ローマ字1文字に対応する仮名を消した時はローマ字、仮名とも
     * SENTOUフラグが1個減る。
     * そうでないときはSENTOUフラグの個数は変わらない
     */
    yc->rStartp = yc->rCurs - 1;
    while ( yc->rStartp > 0 && !(yc->rAttr[yc->rStartp] & SENTOU) ) {
      yc->rStartp--;
    }
    romajiReplace (-1, (wchar_t *)NULL, 0, 0);
    yc->kRStartp = yc->kCurs - 1;
    while ( yc->kRStartp > 0 && !(yc->kAttr[yc->kRStartp] & SENTOU) )
      yc->kRStartp--;
    /* これ必ず真では? */
    prevflag = (yc->kAttr[yc->kRStartp] & SENTOU);
    kanaReplace(yc->kRStartp - yc->kCurs,
		yc->romaji_buffer + yc->rStartp,
		yc->rCurs - yc->rStartp,
		0);
    /* ローマ字1文字に対応する仮名を消したときは最初からSENTOUである */
    yc->kAttr[yc->kRStartp] |= prevflag;
    yc->n_susp_chars = 0; /* とりあえずクリアしておく */
    makePhonoOnBuffer(d, yc, (unsigned char)0, 0, 0);
    /* 以前は常にフラグを下げていたが、未変換ローマ字が残っているときは
     * フラグを下げないことにする */
    if (yc->kRStartp == yc->kCurs)
      yc->generalFlags &= ~CANNA_YOMI_BREAK_ROMAN;
  }
  else {
    yc->generalFlags &= ~CANNA_YOMI_BREAK_ROMAN;
    if ( yc->kAttr[yc->kCurs - howManyDelete] & HENKANSUMI ) {
      if (yc->kAttr[yc->kCurs - howManyDelete] & SENTOU) {
	/* ローマ字かな変換の先頭だったら */
	if (yc->kAttr[yc->kCurs] & SENTOU) {
	  int n;

	  /* 先頭だったらローマ字も先頭マークが立っているところまで戻す */

	  for (n = 1 ; yc->rCurs > 0 && !(yc->rAttr[--yc->rCurs] & SENTOU) ;) {
	    n++;
	  }
	  moveStrings(yc->romaji_buffer, yc->rAttr,
		      yc->rCurs + n, yc->rEndp,-n);
	  if (yc->rCurs < yc->rStartp) {
	    yc->rStartp = yc->rCurs;
	  }
	  yc->rEndp -= n;
	}
	else {
	  /* 仮名のカーソル位置は先頭になるのでローマ字のカーソルも動かす*/
	  while ( yc->rCurs > 0 && !(yc->rAttr[--yc->rCurs] & SENTOU) )
	    ;
	  if (yc->rCurs < yc->rStartp) {
	    yc->rStartp = yc->rCurs;
	  }
	  yc->kAttr[yc->kCurs] |= SENTOU;
	}
      }
    }
    else {
      romajiReplace(-howManyDelete, (wchar_t *)NULL, 0, 0);
    }
    kanaReplace(-howManyDelete, (wchar_t *)NULL, 0, 0);
    if ((yc->rAttr[yc->rCurs] & SENTOU) && yc->kRStartp == yc->kCurs) {
      /* 未変換のローマ字を消してしまったので、次に入力したローマ字は
       * SENTOUになる方が自然だろう
       */
      yc->rStartp = yc->rCurs;
    }
  }
  debug_yomi(yc);
  return(0);
}

static YomiDeletePrevious pro((uiContext));

static int
YomiDeletePrevious(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  KanaDeletePrevious(d);
  if (!yc->kEndp) {
    if (yc->savedFlags & CANNA_YOMI_MODE_SAVED) {
      restoreFlags(yc);
    }
    if (yc->left || yc->right) {
      removeCurrentBunsetsu(d, (tanContext)yc);
      yc = (yomiContext)0;
    }
    else {
      /* 未確定文字列が全くなくなったのなら、φモードに遷移する */
      restoreChikujiIfBaseChikuji(yc);
      d->current_mode = yc->curMode = yc->myEmptyMode;
      d->kanji_status_return->info |= KanjiEmptyInfo;
    }
    currentModeInfo(d);
  }
  else {
    if (yc->kCurs != yc->kRStartp) {
      ReCheckStartp(yc);
    }
  }

  if (yc) {
    fitmarks(yc);
  }

  makeYomiReturnStruct(d);
  return 0;
}

static YomiDeleteNext pro((uiContext));

static int
YomiDeleteNext(d)/* カーソル上の文字の削除 */
uiContext d;
{
  int howManyDelete;
  yomiContext yc = (yomiContext)d->modec;

  if (chikujip(yc) && (yc->status & CHIKUJI_ON_BUNSETSU)) {
    return NothingChangedWithBeep(d);
  }

  if (yc->kCurs == yc->kEndp) {
    /* 右端だからなにもしないのでしょうねぇ */
    d->kanji_status_return->length = -1;
    return 0;
  }

  fitmarks(yc);

  yc->last_rule = 0;
  howManyDelete = howFarToGoForward(yc);

  if (yc->kAttr[yc->kCurs] & SENTOU) {
    if (yc->kAttr[yc->kCurs + howManyDelete] & SENTOU) {
      int n = 1;
      while ( !(yc->rAttr[++yc->rCurs] & SENTOU) )
	n++;
      moveStrings(yc->romaji_buffer, yc->rAttr, yc->rCurs, yc->rEndp, -n);
      yc->rCurs -= n;
      yc->rEndp -= n;
    }
    else {
      yc->kAttr[yc->kCurs + howManyDelete] |= SENTOU;
    }
  }
  kanaReplace(howManyDelete, (wchar_t *)NULL, 0, 0);
  /* ここまで削除処理 */

  if (yc->cStartp < yc->kEndp) { /* 読みがまだある */
    if (yc->kCurs < yc->ys) {
      yc->ys = yc->kCurs; /* こんなもんでいいのでしょうか？ */
    }
  }
  else if (yc->nbunsetsu) { /* 読みはないが文節はある */
    if (RkwGoTo(yc->context, yc->nbunsetsu - 1) == -1) {
      return makeRkError(d, "\312\270\300\341\244\316\260\334\306\260\244\313"
	"\274\272\307\324\244\267\244\336\244\267\244\277");
                            /* 文節の移動に失敗しました */
    }
    yc->kouhoCount = 0;
    yc->curbun = yc->nbunsetsu - 1;
    moveToChikujiTanMode(d);
  }
  else { /* 読みも文節もない */
    if (yc->savedFlags & CANNA_YOMI_MODE_SAVED) {
      restoreFlags(yc);
    }
    if (yc->left || yc->right) {
      removeCurrentBunsetsu(d, (tanContext)yc);
    }
    else {
      /* 未確定文字列が全くなくなったのなら、φモードに遷移する */
      restoreChikujiIfBaseChikuji(yc);
      d->current_mode = yc->curMode = yc->myEmptyMode;
      d->kanji_status_return->info |= KanjiEmptyInfo;
    }
    currentModeInfo(d);
  }
  makeYomiReturnStruct(d);
  debug_yomi(yc);
  return 0;
}

static YomiKillToEndOfLine pro((uiContext));

static int
YomiKillToEndOfLine(d)  /* カーソルから右のすべての文字の削除 */
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  romajiReplace (yc->rEndp - yc->rCurs, (wchar_t *)NULL, 0, 0);
  kanaReplace   (yc->kEndp - yc->kCurs, (wchar_t *)NULL, 0, 0);

  fitmarks(yc);

  if (!yc->kEndp) {
    if (yc->savedFlags & CANNA_YOMI_MODE_SAVED) {
      restoreFlags(yc);
    }
    if (yc->left || yc->right) {
      removeCurrentBunsetsu(d, (tanContext)yc);
    }
    else {
      /* 未確定文字列が全くなくなったのなら、φモードに遷移する */
      restoreChikujiIfBaseChikuji(yc);
      d->current_mode = yc->curMode = yc->myEmptyMode;
      d->kanji_status_return->info |= KanjiEmptyInfo;
    }
    currentModeInfo(d);
  }
  makeYomiReturnStruct(d);
  return 0;
}

static YomiQuit pro((uiContext));

static int
YomiQuit(d)/* 読みの取り消し */
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  /* 未確定文字列を削除する */
  RomajiClearYomi(d);

  if (yc->left || yc->right) {
    removeCurrentBunsetsu(d, (tanContext)yc);
  }
  else {
    /* 未確定文字列が全くなくなったので、φモードに遷移する */
    restoreChikujiIfBaseChikuji(yc);
    d->current_mode = yc->curMode = yc->myEmptyMode;
    d->kanji_status_return->info |= KanjiEmptyInfo;
  }
  makeYomiReturnStruct(d);
  currentModeInfo(d);
  return checkIfYomiQuit(d, 0);
}

coreContext
newCoreContext()
{
  coreContext cc;

  cc = (coreContext)malloc(sizeof(coreContextRec));
  if (cc) {
    cc->id = CORE_CONTEXT;
  }
  return cc;
}

static simplePopCallback pro((uiContext, int, mode_context));

static
simplePopCallback(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d);
  currentModeInfo(d);
  return retval;
}

alphaMode(d)
uiContext d;
{
  extern KanjiModeRec alpha_mode;
  coreContext cc;
  char *bad = "\245\341\245\342\245\352\244\254\302\255\244\352\244\336"
	"\244\273\244\363";
              /* メモリが足りません */

  cc = newCoreContext();
  if (cc == (coreContext)0) {
    makeGLineMessageFromString(d, bad);
    return 0;
  }
  if (pushCallback(d, d->modec,
		   NO_CALLBACK, simplePopCallback,
                   simplePopCallback, NO_CALLBACK) == 0) {
    freeCoreContext(cc);
    makeGLineMessageFromString(d, bad);
    return 0;
  }
  cc->prevMode = d->current_mode;
  cc->next = d->modec;
  cc->majorMode =
    cc->minorMode = CANNA_MODE_AlphaMode;
  d->current_mode = &alpha_mode;
  d->modec = (mode_context)cc;
  return 0;
}

/* Quoted Insert Mode -- 引用入力モード。

   このモードでは次の一文字は否応無しにそのまま入力される。

 */

static exitYomiQuotedInsert pro((uiContext, int, mode_context));

static
exitYomiQuotedInsert(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d);
  return retval;
}

static
YomiInsertQuoted(d)
uiContext d;
{
  unsigned char ch;
  coreContext cc = (coreContext)d->modec;
  yomiContext yc;

  ch = (unsigned char)*(d->buffer_return);

  if (IrohaFunctionKey(ch)) {
    d->kanji_status_return->length = -1;
    d->kanji_status_return->info = 0;
    return 0;
  } else {
    d->current_mode = cc->prevMode;
    d->modec = cc->next;
    free(cc);

    yc = (yomiContext)d->modec;

    romajiReplace (0, d->buffer_return, d->nbytes, 0);
    kanaReplace   (0, d->buffer_return, d->nbytes, HENKANSUMI);
    yc->rStartp = yc->rCurs;
    yc->kRStartp = yc->kCurs;
    makeYomiReturnStruct(d);
    currentModeInfo(d);
    d->status = EXIT_CALLBACK;
    return 0;
  }
}

static yomiquotedfunc pro((uiContext, KanjiMode, int, int, int));

static
yomiquotedfunc(d, mode, whattodo, key, fnum)
     uiContext d;
     KanjiMode mode;
     int whattodo;
     int key;
     int fnum;
     /* ARGSUSED */
{
  switch (whattodo) {
  case KEY_CALL:
    return YomiInsertQuoted(d);
  case KEY_CHECK:
    return 1;
  case KEY_SET:
    return 0;
  }
  /* NOTREACHED */
}

static KanjiModeRec yomi_quoted_insert_mode = {
  yomiquotedfunc,
  0, 0, 0,
};

static void
yomiQuotedInsertMode(d)
uiContext d;
{
  coreContext cc;

  cc = newCoreContext();
  if (cc == 0) {
    NothingChangedWithBeep(d);
    return;
  }
  cc->prevMode = d->current_mode;
  cc->next = d->modec;
  cc->majorMode = d->majorMode;
  cc->minorMode = CANNA_MODE_QuotedInsertMode;
  if (pushCallback(d, d->modec,
                   NO_CALLBACK, exitYomiQuotedInsert,
                   NO_CALLBACK, NO_CALLBACK) == (struct callback *)0) {
    freeCoreContext(cc);
    NothingChangedWithBeep(d);
    return;
  }
  d->modec = (mode_context)cc;
  d->current_mode = &yomi_quoted_insert_mode;
  currentModeInfo(d);
  return;
}

YomiQuotedInsert(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  d->nbytes = 0;

  if (yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) {
    if (yc->status & CHIKUJI_ON_BUNSETSU) {
      if (yc->kEndp != yc->kCurs) {
	yc->rStartp = yc->rCurs = yc->rEndp;
	yc->kRStartp = yc->kCurs = yc->kEndp;
      }
      yc->status &= ~CHIKUJI_ON_BUNSETSU;
      yc->status |= CHIKUJI_OVERWRAP;
    }
    else if (yc->rEndp == yc->rCurs) {
      yc->status &= ~CHIKUJI_OVERWRAP;
    }
  }

  if (forceRomajiFlushYomi(d))
    return(d->nbytes);

  fitmarks(yc);

  yomiQuotedInsertMode(d);
  d->kanji_status_return->length = -1;
  return 0;
}

static int
mapAsKuten(d)
     uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;
  int i, j, ch, len, clen, kanalen, pos;
  char tmpbuf[4];
  wchar_t *hexbuf;
  wchar_t buf[2];
  static allowTwoByte = 1;

  tmpbuf[0] = tmpbuf[1] = tmpbuf[2] = tmpbuf[3] = '\0';

  if (yc->kCurs < yc->cmark) {
    int tmp = yc->kCurs;
    yc->kCurs = yc->cmark;
    yc->cmark = tmp;
    kPos2rPos(yc, 0, yc->kCurs, (int *) 0, &tmp);
    yc->rCurs = tmp;
  }
  else if (yc->kCurs == yc->cmark) {
    yc->kCurs = yc->kRStartp = yc->kEndp;
    yc->rCurs = yc->rStartp = yc->rEndp;
  }

  if (*yc->romaji_buffer == 'x' || *yc->romaji_buffer == 'X')
    len = yc->rCurs - 1;
  else
    len = yc->rCurs;
  if (len > 6) {
    return 0;
  }
  hexbuf = yc->romaji_buffer + yc->rCurs - len;

  kPos2rPos(yc, 0, yc->cmark, (int *) 0, &pos);

  if (hexbuf < yc->romaji_buffer + pos) {
    if (hexbuf + 6 < yc->romaji_buffer + pos) {
      return 0;
    }
  }
  for (i = 0, j = 1; i < len; i++) {
    ch = *(hexbuf + i);
    if ('0' <= ch && ch <= '9')
      tmpbuf[j] = tmpbuf[j] * 10 + (ch - '0');
    else if (ch == '-' && j == 1)
      j++;
    else
      return 0;
  }
  tmpbuf[2] = (char)((0x80 | tmpbuf[2]) + 0x20);
  if (tmpbuf[1] < 0x5f) {
    tmpbuf[1] = (char)((0x80 | tmpbuf[1]) + 0x20);
  }
  else {
    tmpbuf[1] = (char)((0x80 | tmpbuf[1]) - 0x5e + 0x20);
    tmpbuf[0] = (char)0x8f; /* SS3 */
  }
  if ((unsigned char)tmpbuf[1] < 0xa1 ||
      0xfe < (unsigned char)tmpbuf[1] ||
      (len > 2 && ((unsigned char)tmpbuf[2] < 0xa1 ||
                   0xfe < (unsigned char)tmpbuf[2]))) {
    return 0;
  }
  if (hexbuf[-1] == 'x' || hexbuf[-1] == 'X') {
    tmpbuf[0] = (char)0x8f;/*SS3*/
    len++;
  }
  if (tmpbuf[0]) {
    clen = MBstowcs(buf, tmpbuf, 2);
  }
  else {
    clen = MBstowcs(buf, tmpbuf + 1, 2);
  }
  for (i = 0, kanalen = 0 ; i < len ; i++) {
    if (yc->rAttr[yc->rCurs - len + i] & SENTOU) {
      do {
	kanalen++;
      } while (!(yc->kAttr[yc->kCurs - kanalen] & SENTOU));
      yc->rAttr[yc->rCurs - len + i] &= ~SENTOU;
    }
  }
  yc->rAttr[yc->rCurs - len] |= SENTOU;
  kanaReplace(-kanalen, buf, clen, HENKANSUMI);
  yc->kAttr[yc->kCurs - clen] |= SENTOU;
  yc->kRStartp = yc->kCurs;
  yc->rStartp = yc->rCurs;
  yc->pmark = yc->cmark;
  yc->cmark = yc->kCurs;
  yc->n_susp_chars = 0; /* サスペンドしている文字がある場合があるのでクリア */
  return 1;
}

static int
mapAsHex(d)
     uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;
  int i, ch, len = 4, clen, kanalen, pos;
  char tmpbuf[8], *a;
  wchar_t *hexbuf;
  wchar_t buf[2];
  static allowTwoByte = 1;
  extern struct CannaConfig cannaconf;

  if (yc->kCurs < yc->cmark) {
    int tmp = yc->kCurs;
    yc->kCurs = yc->cmark;
    yc->cmark = tmp;
    kPos2rPos(yc, 0, yc->kCurs, (int *)0, &tmp);
    yc->rCurs = tmp;
  }
  else if (yc->kCurs == yc->cmark) {
    yc->kCurs = yc->kRStartp = yc->kEndp;
    yc->rCurs = yc->rStartp = yc->rEndp;
  }

  hexbuf = yc->romaji_buffer + yc->rCurs - 4;

  kPos2rPos(yc, 0, yc->cmark, (int *)0, &pos);

  if (hexbuf < yc->romaji_buffer + pos) {
    if (!allowTwoByte || hexbuf + 2 < yc->romaji_buffer + pos) {
      return 0;
    }
    hexbuf += 2;
    len = 2;
  }
 retry:
  for (i = 0, a = tmpbuf + 1; i < len ; i++) {
    ch = *(hexbuf + i);
    if ('0' <= ch && ch <= '9')
      ch -= '0';
    else if ('A' <= ch && ch <= 'F')
      ch -= 'A' - 10;
    else if ('a' <= ch && ch <= 'f')
      ch -= 'a' - 10;
    else if (allowTwoByte && i < 2 && 2 < len) {
      hexbuf += 2;
      len = 2;
      goto retry;
    }
    else {
      return 0;
    }
    *a++ = ch;
  }
  if (cannaconf.code_input == CANNA_CODE_SJIS) { /* sjis コードだったら */
    char eucbuf[4];  /* SS3 のことがあるため */

    tmpbuf[1] = tmpbuf[1] * 16 + tmpbuf[2];
    if (len > 2) {
      tmpbuf[2] = tmpbuf[3] * 16 + tmpbuf[4];
      tmpbuf[3] = '\0';
    }
    else {
      tmpbuf[2] = '\0';
    }
    if ((unsigned char)tmpbuf[1] < 0x81 ||
        (0x9f < (unsigned char)tmpbuf[1] && (unsigned char)tmpbuf[1] < 0xe0) ||
        0xfc < (unsigned char)tmpbuf[1] ||
        (len > 2 && ((unsigned char)tmpbuf[2] < 0x40 ||
                     0xfc < (unsigned char)tmpbuf[2] ||
                     (unsigned char)tmpbuf[2] == 0x7f))) {
      return 0;
    }
    RkCvtEuc((unsigned char *)eucbuf, sizeof(eucbuf),
                        (unsigned char *)tmpbuf + 1, 2);
    clen = MBstowcs(buf, eucbuf, 2);
  }
  else {
    tmpbuf[1] = 0x80 | (tmpbuf[1] * 16 + tmpbuf[2]);
    if (len > 2) {
      tmpbuf[2] = 0x80 | (tmpbuf[3] * 16 + tmpbuf[4]);
      tmpbuf[3] = '\0';
    }
    else {
      tmpbuf[2] = '\0';
    }
    if ((unsigned char)tmpbuf[1] < 0xa1 ||
        0xfe < (unsigned char)tmpbuf[1] ||
        (len > 2 && ((unsigned char)tmpbuf[2] < 0xa1 ||
                      0xfe < (unsigned char)tmpbuf[2]))) {
      return 0;
    }
    if (len == 2) {
      tmpbuf[1] &= 0x7f;
    }
    if (hexbuf > yc->romaji_buffer
        && len > 2 && (hexbuf[-1] == 'x' || hexbuf[-1] == 'X')) {
      tmpbuf[0] = (char)0x8f;/*SS3*/
      len++;
      clen = MBstowcs(buf, tmpbuf, 2);
    }
    else {
      clen = MBstowcs(buf, tmpbuf + 1, 2);
    }
  }
  for (i = 0, kanalen = 0 ; i < len ; i++) {
    if (yc->rAttr[yc->rCurs - len + i] & SENTOU) {
      do {
	kanalen++;
      } while (!(yc->kAttr[yc->kCurs - kanalen] & SENTOU));
      yc->rAttr[yc->rCurs - len + i] &= ~SENTOU;
    }
  }
  yc->rAttr[yc->rCurs - len] |= SENTOU;
  kanaReplace(-kanalen, buf, clen, HENKANSUMI);
  yc->kAttr[yc->kCurs - clen] |= SENTOU;
  yc->kRStartp = yc->kCurs;
  yc->rStartp = yc->rCurs;
  yc->pmark = yc->cmark;
  yc->cmark = yc->kCurs;
  yc->n_susp_chars = 0; /* サスペンドしている文字がある場合があるのでクリア */
  return 1;
}

/* ConvertAsHex -- １６進とみなしての変換

  ローマ字入力されて反転表示されている文字列を１６進で表示されているコードと
  みなして変換する。

  (MSBは０でも１でも良い)

  */

static ConvertAsHex pro((uiContext));

static
ConvertAsHex(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;
  extern struct CannaConfig cannaconf;

  if (yc->henkanInhibition & CANNA_YOMI_INHIBIT_ASHEX) {
    return NothingChangedWithBeep(d);
  }
  if (yc->savedFlags & CANNA_YOMI_MODE_SAVED) {
    restoreFlags(yc);
    currentModeInfo(d);
  }
  if (cannaconf.code_input != CANNA_CODE_KUTEN) {
    if (!mapAsHex(d)) {
      return NothingChangedWithBeep(d);
    }
  }
  else {
    if (!mapAsKuten(d)) {
      return NothingChangedWithBeep(d);
    }
  }
  if (yc->kCurs - 1 < yc->ys) {
    yc->ys = yc->kCurs - 1;
  }
  makeYomiReturnStruct(d);
  return 0;
}

/*
  convertAsHex  １６進の数字を漢字文字に変換

  これは内部的に使用するためのルーチンである。d->romaji_buffer に含ま
  れる文字列を１６進で表された漢字コードであるとみなして、そのコードに
  よって表現される漢字文字に変換する。変換した文字列は buffer_return
  に格納する。リターン値はエラーがなければ buffer_return に格納した文
  字列の長さである(通常は２である)。エラーが発生している時は−１が格納
  される。

  モードの変更等の処理はこの関数では行われない。

  またバッファのクリアなども行わないので注意するべきである。

  <戻り値>
    正しく１６進に変換できた場合は１そうでない時は０が返る。
*/

int
cvtAsHex(d, buf, hexbuf, hexlen)
uiContext d;
wchar_t *buf, *hexbuf;
int hexlen;
{
  int i;
  char tmpbuf[5], *a, *b;
  wchar_t rch;

  if (hexlen != 4) { /* 入力された文字列の長さが４文字でないのであれば変換
			してあげない */
    d->kanji_status_return->length = -1;
    return 0;
  }
  for (i = 0, a = tmpbuf; i < 4 ; i++) {
    rch = hexbuf[i]; /* まず一文字取り出し、１６進の数字にする。 */
    if ('0' <= rch && rch <= '9') {
      rch -= '0';
    }
    else if ('A' <= rch && rch <= 'F') {
      rch -= 'A' - 10;
    }
    else if ('a' <= rch && rch <= 'f') {
      rch -= 'a' - 10;
    }
    else {
      d->kanji_status_return->length = -1;
      return 0;
    }
    *a++ = (char)rch; /* 取り敢えず保存しておく */
  }
  b = (a = tmpbuf) + 1;
  *a = (char)(0x80 | (*a * 16 + *b));
  *(tmpbuf+1) = 0x80 | (*(a += 2) * 16 + *(b += 2));
  *a = '\0';
  if ((unsigned char)*tmpbuf < 0xa1 ||
      0xfe < (unsigned char)*tmpbuf ||
      (unsigned char)*--a < 0xa1 ||
      0xfe < (unsigned char)*a) {
    return 0;
  } else {
    MBstowcs(buf, tmpbuf, 2);
    return 1;
  }
}

convertAsHex(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  return cvtAsHex(d, d->buffer_return, yc->romaji_buffer, yc->rEndp);
}

/*
   ローマ字かな変換補足ルーチン関連
 */

static void
replaceSup2(ind, n)
int ind, n;
{
  int i;
  wchar_t *temp, **p;

  if (ind < 0)
    return;

  temp = (p = keysup[ind].cand)[n];
  for (i = n ; i > 0 ; i--) {
    p[i] = p[i - 1];
  }
  p[0] = temp;
}

static void
replaceSup(ind, n)
int ind, n;
{
  int i, group;
  extern nkeysup;

  group = keysup[ind].groupid;
  for (i = 0 ; i < nkeysup ; i++) {
    if (keysup[i].groupid == group) {
      replaceSup2(i, n);
    }
  }
}

static everySupkey pro((uiContext, int, mode_context));

static
everySupkey(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  ichiranContext ic = (ichiranContext)d->modec;
  wchar_t *cur;

  cur = ic->allkouho[*(ic->curIkouho)];
  d->kanji_status_return->revPos = 0;
  d->kanji_status_return->revLen = 0;
  d->kanji_status_return->echoStr = cur;
  d->kanji_status_return->length = WStrlen(cur);

  return retval;
}

static exitSupkey pro((uiContext, int, mode_context));

static
exitSupkey(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  yomiContext yc;

  popCallback(d); /* 一覧をポップ */

  yc = (yomiContext)d->modec;

  replaceSup(findSup(yc->romaji_buffer[0]) - 1, yc->cursup);

#ifdef NOT_KAKUTEI
  yc->rCurs = yc->rStartp = yc->rEndp;
  yc->kCurs = yc->kEndp;
  kanaReplace(-yc->kEndp, d->buffer_return, retval, HENKANSUMI | SUPKEY);
  yc->kRStartp = yc->kCurs;
  yc->kAttr[0] |= SENTOU;
  yc->rAttr[0] |= SENTOU | HENKANSUMI;
  for (i = 1 ; i < retval ; i++) {
    yc->kAttr[i] &= ~SENTOU;
  }
  currentModeInfo(d);
  makeYomiReturnStruct(d);
  return 0;
#else
  /* 未確定文字列を削除する */
  RomajiClearYomi(d);

  /* 未確定文字列が全くなくなったので、φモードに遷移する */
  restoreChikujiIfBaseChikuji(yc);
  d->current_mode = yc->curMode = yc->myEmptyMode;
  d->kanji_status_return->info |= KanjiEmptyInfo;
  currentModeInfo(d);
  makeYomiReturnStruct(d);
  return checkIfYomiQuit(d, retval);
#endif
}

static quitSupkey pro((uiContext, int, mode_context));

static
quitSupkey(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* 一覧をポップ */
  makeYomiReturnStruct(d);
  currentModeInfo(d);
  return retval;
}

selectKeysup(d, yc, ind)
uiContext d;
yomiContext yc;
int ind;
{
  int retval;
  ichiranContext ic;
  extern nkeysup;

  yc->cursup = 0;
  retval = selectOne(d, keysup[ind].cand, &(yc->cursup), keysup[ind].ncand,
		     BANGOMAX,
		     (unsigned)(!cannaconf.HexkeySelect ? NUMBERING : 0),
		     0, WITH_LIST_CALLBACK,
		     everySupkey, exitSupkey, quitSupkey, NO_CALLBACK);

  ic = (ichiranContext)d->modec;
  ic->majorMode = CANNA_MODE_IchiranMode;
  ic->minorMode = CANNA_MODE_IchiranMode;

  currentModeInfo(d);
  *(ic->curIkouho) = 0;

  /* 候補一覧行が狭くて候補一覧が出せない */
  if(ic->tooSmall) {
    d->status = AUX_CALLBACK;
    return(retval);
  }

  if ( !(ic->flags & ICHIRAN_ALLOW_CALLBACK) ) {
    makeGlineStatus(d);
  }

  return retval;
}

/*
  外来語変換をするようなリージョンになっているか？

  どんなことを調べるかと言うと、まず、リージョン内が外来語変換されてい
  るかどうかを調べる。次に、リージョンの両端が先頭文字になっていること
  を調べたいところだが、これはやっぱりはずした。

  外来語の途中からとか途中までとかで mark を行った時にさらに外来語変換
  を行うことを抑制する。

 */

static
regionGairaigo(yc, s, e)
yomiContext yc;
int s, e;
{
  if ((yc->kAttr[s] & SENTOU) && (yc->kAttr[e] & SENTOU)) {
    return 1;
  }
  else {
    return 0;
  }
}


/*
  外来語変換済の字が入っているか？
 */

static int
containGairaigo(yc)
yomiContext yc;
{
  int i;

  for (i = 0 ; i < yc->kEndp ; i++) {
    if (yc->kAttr[i] & GAIRAIGO) {
      return 1;
    }
  }
  return 0;
}

containUnconvertedKey(yc)
yomiContext yc;
{
  int i, s, e;

  if (containGairaigo(yc)) {
    return 0;
  }

  if ((s = yc->cmark) > yc->kCurs) {
    e = s;
    s = yc->kCurs;
  }
  else {
    e = yc->kCurs;
  }

  for (i = s ; i < e ; i++) {
    if ( !(yc->kAttr[i] & HENKANSUMI) ) {
      return 1;
    }
  }
  return 0;
}

/*
 * かな漢字変換を行い(変換キーが初めて押された)、TanKouhoModeに移行する
 *
 * 引き数	uiContext
 * 戻り値	正常終了時 0	異常終了時 -1
 */

static YomiHenkan pro((uiContext));

static int
YomiHenkan(d)
uiContext	d;
{
  yomiContext yc = (yomiContext)d->modec;
  int len, idx;

#ifdef MEASURE_TIME
  struct tms timebuf;
  long   currenttime, times();

  currenttime = times(&timebuf);
#endif

  if (yc->henkanInhibition & CANNA_YOMI_INHIBIT_HENKAN) {
    return NothingChangedWithBeep(d);
  }

  d->nbytes = 0;
  len = RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);

  if (containUnconvertedKey(yc)) {
    YomiMark(d);
    len = RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);
  }

  yc->kRStartp = yc->kCurs = yc->kEndp;
  yc->rStartp  = yc->rCurs = yc->rEndp;

  if (len == 0) { /* empty モードに行ってしまった */
    d->more.todo = 1;
    d->more.ch = d->ch;
    d->more.fnum = 0;    /* 上の ch で示される処理をせよ */
    return d->nbytes;
  }

  if (yc->rEndp == 1 && (yc->kAttr[0] & SUPKEY) &&
      !yc->left && !yc->right &&
      (idx = findSup(yc->romaji_buffer[0])) &&
      keysup[idx - 1].ncand > 1) {
    return selectKeysup(d, yc, idx - 1);
  }

  if (!prepareHenkanMode(d)) {
    makeGLineMessageFromString(d, jrKanjiError);
    makeYomiReturnStruct(d);
    return 0;
  }
  yc->minorMode = CANNA_MODE_TankouhoMode;
  yc->kouhoCount = 1;
  if (doHenkan(d, 0, (wchar_t *)0) < 0) {
    makeGLineMessageFromString(d, jrKanjiError);
    return TanMuhenkan(d);
  }
  if (cannaconf.kouho_threshold > 0 &&
      yc->kouhoCount >= cannaconf.kouho_threshold) {
    return tanKouhoIchiran(d, 0);
  }
  currentModeInfo(d);

#ifdef MEASURE_TIME
  hc->proctime = times(&timebuf);
  hc->proctime -= currenttime;
#endif

  return 0;
}

static YomiHenkanNaive pro((uiContext));

static int
YomiHenkanNaive(d)
uiContext	d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags &
      (CANNA_YOMI_HANKAKU | CANNA_YOMI_ROMAJI | CANNA_YOMI_BASE_HANKAKU)) {
    return YomiInsert(d);
  }
  else {
    return YomiHenkan(d);
  }
}

static YomiHenkanOrNothing pro((uiContext));

static int
YomiHenkanOrNothing(d)
uiContext	d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags &
      (CANNA_YOMI_HANKAKU | CANNA_YOMI_ROMAJI | CANNA_YOMI_BASE_HANKAKU)) {
    return NothingChanged(d);
  }
  else {
    return YomiHenkan(d);
  }
}

/* ベース文字の切り替え */

extern EmptyBaseHira pro((uiContext)), EmptyBaseKata pro((uiContext));
extern EmptyBaseEisu pro((uiContext));
extern EmptyBaseZen pro((uiContext)), EmptyBaseHan pro((uiContext));

static YomiBaseHira pro((uiContext));

static
YomiBaseHira(d)
uiContext d;
{
  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);
  (void)EmptyBaseHira(d);
  makeYomiReturnStruct(d);
  return 0;
}

static YomiBaseKata pro((uiContext));

static
YomiBaseKata(d)
uiContext d;
{
  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);
  (void)EmptyBaseKata(d);
  makeYomiReturnStruct(d);
  return 0;
}

static YomiBaseEisu pro((uiContext));

static
YomiBaseEisu(d)
uiContext d;
{
  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);
  (void)EmptyBaseEisu(d);
  makeYomiReturnStruct(d);
  return 0;
}

static YomiBaseZen pro((uiContext));

static
YomiBaseZen(d)
uiContext d;
{
  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);
  (void)EmptyBaseZen(d);
  makeYomiReturnStruct(d);
  return 0;
}

static YomiBaseHan pro((uiContext));

static
YomiBaseHan(d)
uiContext d;
{
  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);
  (void)EmptyBaseHan(d);
  makeYomiReturnStruct(d);
  return 0;
}

static YomiBaseKana pro((uiContext));

static
YomiBaseKana(d)
uiContext d;
{
  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);
  (void)EmptyBaseKana(d);
  makeYomiReturnStruct(d);
  return 0;
}

static YomiBaseKakutei pro((uiContext));

static
YomiBaseKakutei(d)
uiContext d;
{
  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);
  (void)EmptyBaseKakutei(d);
  makeYomiReturnStruct(d);
  return 0;
}

static YomiBaseHenkan pro((uiContext));

static
YomiBaseHenkan(d)
uiContext d;
{
  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);
  (void)EmptyBaseHenkan(d);
  makeYomiReturnStruct(d);
  return 0;
}

int YomiBaseHiraKataToggle pro((uiContext));

YomiBaseHiraKataToggle(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);

  if (yc->generalFlags & CANNA_YOMI_KATAKANA) {
    (void)EmptyBaseHira(d);
  }
  else {
    (void)EmptyBaseKata(d);
  }
  makeYomiReturnStruct(d);
  return 0;
}

int YomiBaseZenHanToggle pro((uiContext));

YomiBaseZenHanToggle(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);

  if (yc->generalFlags & CANNA_YOMI_BASE_HANKAKU) {
    (void)EmptyBaseZen(d);
  }
  else {
    (void)EmptyBaseHan(d);
  }
  makeYomiReturnStruct(d);
  return 0;
}

int YomiBaseRotateForw pro((uiContext));

YomiBaseRotateForw(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);

  if (!(yc->generalFlags & CANNA_YOMI_BASE_HANKAKU) &&
      ((yc->generalFlags & CANNA_YOMI_ROMAJI) ||
       ((yc->generalFlags & CANNA_YOMI_KATAKANA) &&
	!cannaconf.InhibitHankakuKana) )) {
    (void)EmptyBaseHan(d);
  }
  else {
    yc->generalFlags &= ~CANNA_YOMI_BASE_HANKAKU;
    if (yc->generalFlags & CANNA_YOMI_ROMAJI) {
      (void)EmptyBaseHira(d);
    }
    else if (yc->generalFlags & CANNA_YOMI_KATAKANA) {
      (void)EmptyBaseEisu(d);
    }
    else {
      (void)EmptyBaseKata(d);
    }
  }
  makeYomiReturnStruct(d);
  return 0;
}

int YomiBaseRotateBack pro((uiContext));

YomiBaseRotateBack(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);

  if (yc->generalFlags & CANNA_YOMI_BASE_HANKAKU) {
    (void)EmptyBaseZen(d);
  }
  else if (yc->generalFlags & CANNA_YOMI_KATAKANA) {
    (void)EmptyBaseHira(d);
  }
  else if (yc->generalFlags & CANNA_YOMI_ROMAJI) {
    if (!cannaconf.InhibitHankakuKana) {
      yc->generalFlags |= CANNA_YOMI_BASE_HANKAKU;
    }
    (void)EmptyBaseKata(d);
  }
  else {
    yc->generalFlags &= ~CANNA_YOMI_ZENKAKU;
    yc->generalFlags |= CANNA_YOMI_BASE_HANKAKU;
    (void)EmptyBaseEisu(d);
  }
  makeYomiReturnStruct(d);
  return 0;
}

int YomiBaseKanaEisuToggle pro((uiContext));

YomiBaseKanaEisuToggle(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);

  if (yc->generalFlags & CANNA_YOMI_ROMAJI) {
    (void)EmptyBaseKana(d);
  }
  else {
    (void)EmptyBaseEisu(d);
  }
  makeYomiReturnStruct(d);
  return 0;
}

int YomiBaseKakuteiHenkanToggle pro((uiContext));

YomiBaseKakuteiHenkanToggle(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  (void)RomajiFlushYomi(d, d->genbuf, ROMEBUFSIZE);

  if (yc->generalFlags & CANNA_YOMI_KAKUTEI) {
    (void)EmptyBaseHenkan(d);
  }
  else { /* 本当は一筋縄では行かない */
    (void)EmptyBaseKakutei(d);
  }
  makeYomiReturnStruct(d);
  return 0;
}

int YomiModeBackup pro((uiContext));

YomiModeBackup(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  (void)saveFlags(yc);
  return NothingChanged(d);
}

/* 字種変換関連 */

/* cfuncdef

   exitJishu -- 字種変換を確定させる

   この関数は字種変換を確定させて読みモードに戻ったところで実行される
   関数である。

   【字種変換とのお約束事】

   この関数は jishu.c に書いてある JishuKakutei が呼び出されたと
   きなどに呼び出される関数である。JishuKakutei では最終的な字種
   の指定やその範囲の指定をして来るだけで実際の目的字種への変換は
   この関数で行わなければならない。なぜかと言うとローマ字との対応
   づけをきちんと保持しておきたいからである JishuKakutei との間の
   お約束は以下の通り

   (1) 最終的な字種は yc の字種関連のメンバに返される
   (2) 具体的には以下に入る。
       jishu_kc    最終的な字種の種類 (JISHU_ZEN_KATA など)
       jishu_case  最終的な字種のケース (CANNA_JISHU_UPPER など)
       jishu_kEndp 字種変換の対象範囲
       jishu_rEndp 字種変換の対象範囲のローマ字バッファでの位置
   (3) yc->cmark までは字種が置き変わらないのに注意する。
   (4) yc->kana_buffer の置き換えは exitJishu が行う。
   (5) yc->kana_buffer で字種変換範囲以外のものは yc->romaji_buffer
       をもう一度コピーしてローマ字かな変換されることで付け加えられる。
   (6) ただし、yc->kRStartp == yc->jishu_kEndp ならば上記の処理は行わな
       い。
   (7) 上記で返されない部分のローマ字は yc->jishu_rEndp 以降である。
   (8) exitJishu はその部分を yc->kana_buffer に移動しもう一度ローマ字
       かな変換を行う。
 */

exitJishu(d)
uiContext d;
{
  yomiContext yc;
  int len, srclen, i, pos;
  BYTE jishu, jishu_case, head = 1;
  int jishu_kEndp, jishu_rEndp;
  int (*func1)(), (*func2)();
  int RkwCvtZen(), RkwCvtKana(), RkwCvtHira(), RkwCvtHan();
  long savedgf;
  wchar_t *buf, *p;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  wchar_t xxxx[1024];
#else
  wchar_t *xxxx = (wchar_t *)malloc(sizeof(wchar_t) * 1024);
  if (!xxxx) {
    return 0;
  }
#endif

  /* ここから下は完全な『読み』モード */

  yc = (yomiContext)d->modec;

  jishu = yc->jishu_kc;
  jishu_case = yc->jishu_case;
  jishu_kEndp = yc->jishu_kEndp;
  jishu_rEndp = yc->jishu_rEndp;

  leaveJishuMode(d, yc);

  /* テンポラリモードだったら元に戻す */
  if (yc->savedFlags & CANNA_YOMI_MODE_SAVED) {
    restoreFlags(yc);
  }
  /* 逐次の読みポインタをクリア */
  yc->ys = yc->cStartp;

  /* まず、字種変換された部分を変換 */
  buf = d->genbuf;
  switch (jishu) {
  case JISHU_ZEN_KATA: /* 全角カタカナに変換する */
    func1 = RkwCvtZen;
    func2 = RkwCvtKana;
    goto jishuKakuteiKana;

  case JISHU_HAN_KATA: /* 半角カタカナに変換する */
    func1 = RkwCvtKana;
    func2 = RkwCvtHan;
    goto jishuKakuteiKana;

  case JISHU_HIRA: /* ひらがなに変換する */
    func1 = RkwCvtZen;
    func2 = RkwCvtHira;

  jishuKakuteiKana:
    /* まず、ベースがローマ字のときに入力されたものがあればかなに変換する */
    savedgf = yc->generalFlags;
    yc->generalFlags = savedgf & CANNA_YOMI_IGNORE_USERSYMBOLS;
    for (i = yc->cmark ; i < jishu_kEndp ;) {
      int j = i;
      while (i < jishu_kEndp && yc->kAttr[i] & STAYROMAJI) {
	yc->kAttr[i++] &= ~(HENKANSUMI | STAYROMAJI);
      }
      if (j < i) {
	kPos2rPos(yc, j, i, &yc->rStartp, &yc->rCurs);
	yc->kRStartp = j;
	yc->kCurs = i;
	makePhonoOnBuffer(d, yc, (unsigned char)0, RK_FLUSH, 0);
	jishu_kEndp += yc->kCurs - i;
	i = yc->kCurs;
      }
      else {
	i++;
      }
    }
    yc->generalFlags = savedgf;

    /* ここで、ローマ字かな変換単位で字種変換する */
    for (i = yc->cmark ; i < jishu_kEndp ; i = yc->kCurs) {
      int j;

      for (j = i + 1 ; !(yc->kAttr[j] & SENTOU) ;) {
	j++;
      }
      if(j > jishu_kEndp) {
	j = jishu_kEndp;
      }
      srclen = j - i;

      len = (*func1)(xxxx, 1024, yc->kana_buffer + i, srclen);
      len = (*func2)(buf, ROMEBUFSIZE, xxxx, len);
      yc->kCurs = j;
      kanaReplace(-srclen, buf, len, 0);
      jishu_kEndp += len - srclen; /* yc->kCurs - j と同じ値 */

      for (j = yc->kCurs - len ; j < yc->kCurs ; j++) {
	yc->kAttr[j] = HENKANSUMI;
      }
      yc->kAttr[yc->kCurs - len] |= SENTOU;
    }
    break;

  case JISHU_ZEN_ALPHA: /* 全角英数に変換する */
  case JISHU_HAN_ALPHA: /* 半角英数に変換する */
    p = yc->romaji_buffer;
    kPos2rPos(yc, 0, yc->cmark, (int *)0, &pos);

    for (i = pos ; i < jishu_rEndp ; i++) {
      xxxx[i - pos] =
	(jishu_case == CANNA_JISHU_UPPER) ? WToupper(p[i]) :
        (jishu_case == CANNA_JISHU_LOWER) ? WTolower(p[i]) : p[i];
      if (jishu_case == CANNA_JISHU_CAPITALIZE) {
	if (p[i] <= ' ') {
	  head = 1;
	}
	else if (head) {
	  head = 0;
	  xxxx[i - pos] = WToupper(p[i]);
	}
      }
    }
    xxxx[i - pos] = (wchar_t)0;
#if 0
    if (jishu_case == CANNA_JISHU_CAPITALIZE) {
      xxxx[0] = WToupper(xxxx[0]);
    }
#endif
    if (jishu == JISHU_ZEN_ALPHA) {
      len = RkwCvtZen(buf, ROMEBUFSIZE, xxxx, jishu_rEndp - pos);
    }
    else {
      len = RkwCvtNone(buf, ROMEBUFSIZE, xxxx, jishu_rEndp - pos);
    }
    yc->rCurs = jishu_rEndp;
    yc->kCurs = jishu_kEndp;
    kanaReplace(yc->cmark - yc->kCurs, buf, len, 0);
    jishu_kEndp = yc->kCurs;

    /* ここで先頭ビットを立てる */
    for (i = pos ; i < yc->rCurs ; i++) {
      yc->rAttr[i] = SENTOU;
    }

    len = yc->kCurs;
    for (i = yc->cmark ; i < len ; i++) {
      yc->kAttr[i] = HENKANSUMI | SENTOU;
    }

    /* 後ろの部分 */
    for (i = jishu_rEndp ; i < yc->rEndp ; i++) {
      yc->rAttr[i] = 0;
    }
    yc->rAttr[jishu_rEndp] = SENTOU;

    kanaReplace(yc->kEndp - jishu_kEndp, yc->romaji_buffer + jishu_rEndp,
		yc->rEndp - jishu_rEndp, 0);
    yc->rAttr[jishu_rEndp] |= SENTOU;
    yc->kAttr[jishu_kEndp] |= SENTOU;
    yc->rStartp = jishu_rEndp;
    yc->kRStartp = jishu_kEndp;

    for (yc->kCurs = jishu_kEndp, yc->rCurs = jishu_rEndp ;
	 yc->kCurs < yc->kEndp ;) {
      yc->kCurs++; yc->rCurs++;
      if (yc->kRStartp == yc->kCurs - 1) {
	yc->kAttr[yc->kRStartp] |= SENTOU;
      }
      makePhonoOnBuffer(d, yc,
                          (unsigned char)yc->kana_buffer[yc->kCurs - 1], 0, 0);
    }
    if (yc->kRStartp != yc->kEndp) {
      if (yc->kRStartp == yc->kCurs - 1) {
	yc->kAttr[yc->kRStartp] |= SENTOU;
      }
      makePhonoOnBuffer(d, yc, (unsigned char)0, RK_FLUSH, 0);
    }
    break;

  default:/* どれでもなかったら変換出来ないので何もしない */
    jishu_rEndp = jishu_kEndp = 0;
    break;
  }
  yc->kCurs = yc->kRStartp = yc->kEndp;
  yc->rCurs = yc->rStartp = yc->rEndp;
  yc->pmark = yc->cmark;
  yc->cmark = yc->kCurs;
  yc->jishu_kEndp = 0;
#ifdef USE_MALLOC_FOR_BIG_ARRAY
  (void)free((char *)xxxx);
#endif
  return 0;
}

static
YomiJishu(d, fn) /* 読みモードから直接字種モードへ */
uiContext d;
int fn;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->henkanInhibition & CANNA_YOMI_INHIBIT_JISHU) {
    return NothingChangedWithBeep(d);
  }
  d->nbytes = 0;
  if ((yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) &&
      !(yc->status & CHIKUJI_OVERWRAP) && yc->nbunsetsu) {
    yc->status |= CHIKUJI_OVERWRAP;
    moveToChikujiTanMode(d);
  }
  else if (! RomajiFlushYomi(d, (wchar_t *)NULL, 0)) {
    d->more.todo = 1;
    d->more.ch = d->ch;
    d->more.fnum = 0;    /* 上の ch で示される処理をせよ */
    return d->nbytes;
  }
  else {
    enterJishuMode(d, yc);
    yc->minorMode = CANNA_MODE_JishuMode;
  }
  currentModeInfo(d);
  d->more.todo = 1;
  d->more.ch = d->ch;
  d->more.fnum = fn;
  return 0;
}

static int
chikujiEndBun(d)
     uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;
  int ret = 0;

  if ((yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) && yc->nbunsetsu) {
    KanjiMode mdsv;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
    yomiContextRec ycsv;
#else
    yomiContext ycsv;

    ycsv = (yomiContext)malloc(sizeof(yomiContextRec));
    if (ycsv) {
#endif

    /* 疑問が残る処理 */
#ifdef USE_MALLOC_FOR_BIG_ARRAY
    * /* This is a little bit tricky source code */
#endif
    ycsv = *yc;
    yc->kEndp = yc->rEndp = 0;
    mdsv = d->current_mode;
    ret = TanKakutei(d);
    d->current_mode = mdsv;
    *yc =
#ifdef USE_MALLOC_FOR_BIG_ARRAY
      * /* this is also a little bit trick source code */
#endif
      ycsv;
  }
#ifdef USE_MALLOC_FOR_BIG_ARRAY
    (void)free((char *)ycsv);
  }
#endif
  return(ret);
}

/* cfuncdef

   replaceEnglish -- かなバッファをローマ字に戻して再ローマ字かな変換する

   d, yc      : コンテクスト
   start, end : ローマ字に戻す範囲
   RKflag     : RkwMapPhonogram に与えるフラグ
   engflag    : 英単語カタカナ変換をするかどうかのフラグ

 */

static void
replaceEnglish(d, yc, start, end, RKflag, engflag)
uiContext d;
yomiContext yc;
int start, end, RKflag, engflag;
{
  int i;

  kanaReplace(yc->pmark - yc->cmark,
	      yc->romaji_buffer + start, end - start, 0);
  yc->kRStartp = yc->pmark;
  yc->rStartp = start;
  for (i = start ; i < end ; i++) {
    yc->rAttr[i] &= ~SENTOU;
  }
  yc->rAttr[start] |= SENTOU;
  for (i = yc->pmark ; i < yc->kCurs ; i++) {
    yc->kAttr[i] &= ~(SENTOU | HENKANSUMI);
  }
  yc->kAttr[yc->pmark] |= SENTOU;

  yc->n_susp_chars = 0; /* ローマ字かな変換やり直しなのでクリアする */
  makePhonoOnBuffer(d, yc, 0, (unsigned char)RKflag, engflag);
  yc->kRStartp = yc->kCurs;
  yc->rStartp = yc->rCurs;
}

int YomiMark pro((uiContext));

int
YomiMark(d)
uiContext d;
{
#ifndef NOT_ENGLISH_TABLE
  int rc, rp, i;
#endif
  yomiContext yc = (yomiContext)d->modec;

#if defined(DEBUG)
  if (iroha_debug) {
    fprintf(stderr,"yc->kCurs=%d yc->cmark=%d\n", yc->kCurs,yc->cmark);
  }
#endif /* DEBUG */

  if (yc->kCurs != yc->cmark) { /* お初 */

    if (yc->cmark < yc->kCurs) {
      yc->pmark = yc->cmark;
      yc->cmark = yc->kCurs;
    }
    else {
      /* 以下、pmark < cmark を仮定している処理があるので、
	 cmark < pmark の場合は pmark も cmark と同じ値にしてしまう。
	 ちょっと前までは pmark と cmark の入れ換えをやっていたが、
	 そうしてしまうと、現在のマークよりも左へはマークが付けられない
	 と言うことになってしまう。 */
      yc->pmark = yc->cmark = yc->kCurs;
    }
    yc->englishtype = CANNA_ENG_NO;
  }
#ifndef NOT_ENGLISH_TABLE
  if (englishdic) {
    if (regionGairaigo(yc, yc->pmark, yc->cmark)) {
      yc->englishtype++;
      yc->englishtype = (BYTE)((int)yc->englishtype % (int)(CANNA_ENG_NO + 1));
      if (yc->englishtype == CANNA_ENG_KANA) {
	kPos2rPos(yc, yc->pmark, yc->cmark, &rp, &rc);
	replaceEnglish(d, yc, rp, rc, RK_FLUSH, 1);
	yc->cmark = yc->kCurs;
      }
    }
    else {
      makeYomiReturnStruct(d);
      return 0;
    }

    /* まずは、カナにできる英単語があったかどうかをチェック */
    rp = rc = 0;
    for (i = yc->pmark ; i < yc->cmark ; i++) {
      if (yc->kAttr[i] & GAIRAIGO) {
	rp = i;
	do {
	  i++;
	} while (!(yc->kAttr[i] & SENTOU));
	rc = i;
	break;
      }
    }
    if (rp || rc) {
      int rs, re, offset;
      wchar_t space2[2];

      kPos2rPos(yc, rp, rc, &rs, &re);
      switch (yc->englishtype) {
      case CANNA_ENG_KANA:
	break;
      case CANNA_ENG_ENG1:
	offset = yc->kCurs - rc;
	yc->kCurs -= offset;
	kanaReplace(rp - rc, yc->romaji_buffer + rs, re - rs,
		    HENKANSUMI | GAIRAIGO);
	yc->kAttr[yc->kCurs - re + rs] |= SENTOU;
	yc->kCurs += offset;
	yc->cmark = yc->kRStartp = yc->kCurs;
	break;
      case CANNA_ENG_ENG2:
	offset = yc->kCurs - rc;
	yc->kCurs -= offset;
	space2[0] = (wchar_t)' ';
	space2[1] = (wchar_t)' ';
	kanaReplace(rp - rc, space2, 2, HENKANSUMI | GAIRAIGO);
	yc->kAttr[yc->kCurs - 2] |= SENTOU;
	yc->kCurs--;
	kanaReplace(0, yc->romaji_buffer + rs, re - rs, HENKANSUMI | GAIRAIGO);
	yc->kAttr[yc->kCurs - re + rs] &= ~SENTOU;
	yc->kCurs += offset + 1;
	yc->cmark = yc->kRStartp = yc->kCurs;
	break;
      case CANNA_ENG_NO:
	kPos2rPos(yc, yc->pmark, yc->cmark, &rs, &re);
	replaceEnglish(d, yc, rs, re, 0, 0);
	yc->cmark = yc->kRStartp = yc->kCurs;
	break;
      }
    }
  }
#endif
  makeYomiReturnStruct(d);
  debug_yomi(yc);
  return 0;
}

Yomisearchfunc(d, mode, whattodo, key, fnum)
uiContext d;
KanjiMode mode;
int whattodo;
int key;
int fnum;
{
  yomiContext yc = (yomiContext)0;
  int len;
  extern KanjiModeRec yomi_mode;

  if (d) {
    yc = (yomiContext)d->modec;
  }

  if (yc && yc->id != YOMI_CONTEXT) {
    /* 本来ありえないが、バグっていて、こうなってても core を吐きさえ
       しなければそのうち正しい状態に戻るので念の為いれておく */
    yc = (yomiContext)0;
  }

  if (cannaconf.romaji_yuusen && yc) { /* もし、優先なら */
    len = yc->kCurs - yc->kRStartp;
    if (fnum == 0) {
      fnum = mode->keytbl[key];
    }
    if (fnum != CANNA_FN_FunctionalInsert && len > 0) {
      int n, m, t, flag, prevrule;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
      wchar_t kana[128], roma[128];
#else
      wchar_t *kana, *roma;
      kana = (wchar_t *)malloc(sizeof(wchar_t) * 128);
      roma = (wchar_t *)malloc(sizeof(wchar_t) * 128);
      if (!kana || !roma) {
	if (kana) {
	  (void)free((char *)kana);
	}
	if (roma) {
	  (void)free((char *)roma);
	}
	return 0; /* ? suspicious */
      }
#endif

      flag = cannaconf.ignore_case ? RK_IGNORECASE : 0;

      WStrncpy(roma, yc->kana_buffer + yc->kRStartp, len);
      roma[len++] = (wchar_t)key;

      prevrule = yc->last_rule;
      if ((RkwMapPhonogram(yc->romdic, kana, 128, roma, len, (wchar_t)key,
			   flag | RK_SOKON, &n, &m, &t, &prevrule) &&
	   n == len) || n == 0) {
	/* RK_SOKON を付けるのは旧辞書用 */
	fnum = CANNA_FN_FunctionalInsert;
      }
#ifdef USE_MALLOC_FOR_BIG_ARRAY
      (void)free((char *)kana);
      (void)free((char *)roma);
#endif
    }
  }
  return searchfunc(d, mode, whattodo, key, fnum);
}

/*
  trimYomi -- 読みバッファのある領域以外を削る

   sy ey  かなの部分で残す領域、この外側は削られる。
   sr er  ローマ字             〃
 */

void
trimYomi(d, sy, ey, sr, er)
uiContext d;
int sy, ey, sr, er;
{
  yomiContext yc = (yomiContext)d->modec;

  yc->kCurs = ey;
  yc->rCurs = er;

  romajiReplace (yc->rEndp - er, (wchar_t *)NULL, 0, 0);
  kanaReplace   (yc->kEndp - ey, (wchar_t *)NULL, 0, 0);

  yc->kCurs = sy;
  yc->rCurs = sr;

  romajiReplace (-sr, (wchar_t *)NULL, 0, 0);
  kanaReplace   (-sy, (wchar_t *)NULL, 0, 0);
}

#if 0 /* unused */
static int
TbBubunKakutei(d)
uiContext d;
{
  tanContext tan, tc = (tanContext)d->modec;
  wchar_t *s = d->buffer_return, *e = s + d->n_buffer;
  int len;

  tan = tc;
  while (tan->left) {
    tan = tan->left;
  }

  len = doKakutei(d, tan, tc, s, e, (yomiContext *)0);
  d->modec = (mode_context)tc;
  tc->left = (tanContext)0;
  s += len;
  (void)TanMuhenkan(d);
  return len;
}
#endif

int doTanConvertTb pro((uiContext, yomiContext));

int TanBubunKakutei pro((uiContext));

int
TanBubunKakutei(d)
uiContext d;
{
  int len;
  tanContext tan;
  yomiContext yc = (yomiContext)d->modec;
  wchar_t *s = d->buffer_return, *e = s + d->n_buffer;

  if (yc->id == YOMI_CONTEXT) {
    doTanConvertTb(d, yc);
    yc = (yomiContext)d->modec;
  }
  tan = (tanContext)yc;
  while (tan->left) {
    tan = tan->left;
  }
  len = doKakutei(d, tan, (tanContext)yc, s, e, (yomiContext *)0);
  d->modec = (mode_context)yc;
  yc->left = (tanContext)0;

  makeYomiReturnStruct(d);
  currentModeInfo(d);
  return len;
}

#if 0
/*
 * カレント文節の前まで確定し、カレント以降の文節を読みに戻す
 *
 * 引き数	uiContext
 * 戻り値	正常終了時 0	異常終了時 -1
 */
TanBubunKakutei(d)
uiContext	d;
{
  extern KanjiModeRec cy_mode, yomi_mode;
  wchar_t *ptr = d->buffer_return, *eptr = ptr + d->n_buffer;
  yomiContext yc = (yomiContext)d->modec;
  tanContext tan;
  int i, j, n, l = 0, len, con, ret = 0;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  wchar_t tmpbuf[ROMEBUFSIZE];
#else
  wchar_t *tmpbuf = (wchar_t *)malloc(sizeof(wchar_t) * ROMEBUFSIZE);
  if (!tmpbuf) {
    return 0;
  }
#endif

  if (yc->id != YOMI_CONTEXT) {
    ret = TbBubunKakutei(d);
    goto return_ret;
  }

  tan = (tanContext)yc;
  while (tan->left) {
    tan = tan->left;
  }

  len = doKakutei(d, tan, (tanContext)yc, ptr, eptr, (yomiContext *)0);
  d->modec = (mode_context)yc;
  yc->left = (tanContext)0;
  ptr += len;

  if (yomiInfoLevel > 0) {  /* 面倒なので yomiInfo を捨てる */
    d->kanji_status_return->info &= ~KanjiYomiInfo;
  }

  con = yc->context;

  /* 確定文字列 を作る */
  for (i = 0, n = yc->curbun ; i < n ; i++) {
    if (RkwGoTo(con, i) < 0) {
      ret = makeRkError(d, "\312\270\300\341\244\316\260\334\306\260\244\313"
	"\274\272\307\324\244\267\244\336\244\267\244\277");
                            /* 文節の移動に失敗しました */
      goto return_ret;
    }
    len = RkwGetKanji(con, ptr, (int)(eptr - ptr));
    if (len < 0) {
      (void)makeRkError(d, "\264\301\273\372\244\316\274\350\244\352\275\320"
	"\244\267\244\313\274\272\307\324\244\267\244\336\244\267\244\277");
                           /* 漢字の取り出しに失敗しました */
      ret = TanMuhenkan(d);
      goto return_ret;
    }
    ptr += len;
    j = RkwGetYomi(yc->context, tmpbuf, ROMEBUFSIZE);
    if (j < 0) {
      (void)makeRkError(d, "\245\271\245\306\245\244\245\277\245\271\244\362"
	"\274\350\244\352\275\320\244\273\244\336\244\273\244\363\244\307"
	"\244\267\244\277");
                           /* ステイタスを取り出せませんでした */
      ret = TanMuhenkan(d);
      goto return_ret;
    }
    l += j;
  }
  d->nbytes = ptr - d->buffer_return;

  for (i = j = 0 ; i < l ; i++) {
    if (yc->kAttr[i] & SENTOU) {
      do {
	++j;
      } while (!(yc->rAttr[j] & SENTOU));
    }
  }
  yc->rStartp = yc->rCurs = j;
  romajiReplace(-j, (wchar_t *)NULL, 0, 0);
  yc->kRStartp = yc->kCurs = i;
  kanaReplace(-i, (wchar_t *)NULL, 0, 0);

  if (RkwEndBun(yc->context, cannaconf.Gakushu ? 1 : 0) == -1) {
    jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271\244\316"
	"\275\252\316\273\244\313\274\272\307\324\244\267\244\336\244\267"
	"\244\277";
                   /* かな漢字変換の終了に失敗しました */
    if (errno == EPIPE) {
      jrKanjiPipeError();
    }
  }

  if (yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) {
    yc->status &= CHIKUJI_NULL_STATUS;
    yc->cStartp = yc->cRStartp = 0;
    yc->kCurs = yc->kRStartp = yc->kEndp;
    yc->rCurs = yc->rStartp = yc->rEndp;
    yc->ys = yc->ye = yc->cStartp;
    clearHenkanContext(yc);
    d->current_mode = yc->curMode = yc->rEndp ? &cy_mode : yc->myEmptyMode;
  }
  else {
    d->current_mode = yc->curMode = &yomi_mode;
  }
  yc->minorMode = getBaseMode(yc);

  yc->nbunsetsu = 0;

  /* 単候補状態から読みに戻るときには無条件にmarkを先頭に戻す */
  yc->cmark = yc->pmark = 0;

  abandonContext(d, yc);

  doMuhenkan(d, yc);

  makeYomiReturnStruct(d);
  currentModeInfo(d);

  ret = d->nbytes;

 return_ret:
#ifdef USE_MALLOC_FOR_BIG_ARRAY
  (void)free((char *)tmpbuf);
#endif
  return ret;
}
#endif /* 0 */

/*
  removeKana -- yomiContext の先頭から字を削る(逐次で使う)

  k -- かなの削る数
  r -- ローマ字の削る数
  d はいらないように見えるがマクロで実は使っているので必要。

 */

void
removeKana(d, yc, k, r)
uiContext d;
yomiContext yc;
int k, r;
{
  int offs;

  offs = yc->kCurs - k;
  yc->kCurs = k;
  kanaReplace(-k, (wchar_t *)NULL, 0, 0);
  if (offs > 0) {
    yc->kCurs = offs;
  }
  yc->cmark = yc->kRStartp = yc->kCurs;
  offs = yc->rCurs - r;
  yc->rCurs = r;
  romajiReplace(-r, (wchar_t *)NULL, 0, 0);
  if (offs > 0) {
    yc->rCurs = offs;
  }
  yc->rStartp = yc->rCurs;
}

static YomiNextJishu pro((uiContext));

static
YomiNextJishu(d) /* 読みモードからの順回り文字種変換 */
uiContext d;
{
  return YomiJishu(d, CANNA_FN_Next);
}

static YomiPreviousJishu pro((uiContext));

static
YomiPreviousJishu(d) /* 読みモードからの逆回り文字種変換 */
uiContext d;
{
  return YomiJishu(d, CANNA_FN_Prev);
}

static YomiKanaRotate pro((uiContext));

static
YomiKanaRotate(d) /* 読みモードからの順回りかな文字種変換 */
uiContext d;
{
  return YomiJishu(d, CANNA_FN_KanaRotate);
}

static YomiRomajiRotate pro((uiContext));

static
YomiRomajiRotate(d) /* 読みモードからの順回り英数文字種変換 */
uiContext d;
{
  return YomiJishu(d, CANNA_FN_RomajiRotate);
}

static YomiCaseRotateForward pro((uiContext));

static
YomiCaseRotateForward(d) /* 読みモードからの順回り英数文字種変換 */
uiContext d;
{
  return YomiJishu(d, CANNA_FN_CaseRotate);
}

static YomiZenkaku pro((uiContext));

static
YomiZenkaku(d) /* 読みモードからの全角変換 */
uiContext d;
{
  return YomiJishu(d, CANNA_FN_Zenkaku);
}

static YomiHankaku pro((uiContext));

static
YomiHankaku(d) /* 読みモードからの半角変換 */
uiContext d;
{
  if (cannaconf.InhibitHankakuKana)
    return NothingChangedWithBeep(d);
  else
    return YomiJishu(d, CANNA_FN_Hankaku);
}

static YomiHiraganaJishu pro((uiContext));

static
YomiHiraganaJishu(d) /* 読みモードから字種モードのひらがなへ */
uiContext d;
{
  return YomiJishu(d, CANNA_FN_Hiragana);
}

static YomiKatakanaJishu pro((uiContext));

static
YomiKatakanaJishu(d) /* 読みモードから字種モードのカタカナへ */
uiContext d;
{
  return YomiJishu(d, CANNA_FN_Katakana);
}

static YomiRomajiJishu pro((uiContext));

static
YomiRomajiJishu(d) /* 読みモードから字種モードのローマ字へ */
uiContext d;
{
  return YomiJishu(d, CANNA_FN_Romaji);
}

static YomiToLower pro((uiContext));
static
YomiToLower(d)
uiContext d;
{
  return YomiJishu(d, CANNA_FN_ToLower);
}

static YomiToUpper pro((uiContext));

static
YomiToUpper(d)
uiContext d;
{
  return YomiJishu(d, CANNA_FN_ToUpper);
}

static YomiCapitalize pro((uiContext));

static
YomiCapitalize(d)
uiContext d;
{
  return YomiJishu(d, CANNA_FN_Capitalize);
}

/* 英語カタカナ変換のやり残し

 ・外来語変換は字種変換に取り込みたい

   ついでなのでエンジン切り替えのやり残し

 ・ＤＳＯがない場合に「そのエンジンに切り替えられない」と言いたい
 ・その他エラーチェック

 */

#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/

#include "yomimap.h"
/* vim: set sw=2: */
