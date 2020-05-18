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
static char rcs_id[] = "@(#) 102.1 $Id: jrbind.c,v 6.10 1996/10/22 11:52:55 kon Exp $";
#endif /* lint */

#include "canna.h"
#include <canna/mfdef.h>
#include <sys/types.h>

#define ACTHASHTABLESIZE 64
#define KEYHASHTABLESIZE 16
   
/*

  jrKanjiString は TTY のキー入力を受け取り、そのキーにしたがって必要
  ならカナ漢字変換を行い、そのキー入力の結果として得られる文字列を 
  buffer_return で返す。buffer_return はアプリケーション側に用意するバッ
  ファであり、アプリケーションはそのバッファの長さを bytes_buffer で渡
  す。

  kanji_status_return は確定していない入力文字列を表示するためのデータ
  であり、未確定の読みや候補漢字などが返される。kanji_status_returnの
  メンバには、 echoStr, length, revPos, revLen がありそれぞれ、未確定
  文字列へのポインタ、その長さ、未確定文字列のうち、強調する部分へのオ
  フセット、強調する部分の長さを返す。未確定文字列を格納する領域は 
  jrKanjiString で自動的に用意される。

 */

extern int FirstTime;

extern BYTE *actFromHash();

exp(int)
wcKanjiString(context_id, ch, buffer_return, nbuffer, kanji_status_return)
const int context_id, ch, nbuffer;
wchar_t        *buffer_return;
wcKanjiStatus  *kanji_status_return;
{
  int res;

  *buffer_return = (wchar_t)ch;

  res = XwcLookupKanji2((unsigned int)0, (unsigned int)context_id,
			buffer_return, nbuffer,
			1/* byte */, 1/* functional char*/,
			kanji_status_return);
  return res;
}

/* jrKanjiControl -- カナ漢字変換の制御を行う */

exp(int)
wcKanjiControl(context, request, arg)
     const int context;
     const int request;
     char *arg;
{
  return XwcKanjiControl2((unsigned int)0, (unsigned int)context,
			  (unsigned int)request, (BYTE *)arg);
}

static uiContext
newUiContext(dpy, win)
unsigned int dpy, win;
{
  extern struct CannaConfig cannaconf;
  uiContext d;

  if ((d = (uiContext)malloc(sizeof(uiContextRec))) != (uiContext)0) {
    if (initRomeStruct(d, cannaconf.chikuji) == 0) {
      if (internContext(dpy, win, d)) {
	return d;
      }
      freeRomeStruct(d);
    }
    free((char *)d);
  }
  return (uiContext)0;
}

extern int kanjiControl pro((int, uiContext, caddr_t));

XwcLookupKanji2(dpy, win, buffer_return, nbuffer, nbytes, functionalChar,
		kanji_status_return)
unsigned int dpy, win;
int functionalChar, nbytes;
wchar_t *buffer_return;
int nbuffer;
wcKanjiStatus *kanji_status_return;
{
  uiContext d, keyToContext();
  int retval;
  extern int locale_insufficient;

  /* locale データベースが不十分で wchar_t との変換処理ができない場合は
     『かんな』にとって大打撃。もう変換はできない！ */
  if (locale_insufficient) {
    kanji_status_return->info = KanjiEmptyInfo | KanjiThroughInfo;
    if (nbytes) { /* キャラクタコードがとれた場合 */
      kanji_status_return->length =
	kanji_status_return->revPos =
	  kanji_status_return->revLen = 0;
      return nbytes;
    }
    else { /* キャラクタコードがとれなかった場合（シフトキーなど）... */
      kanji_status_return->length = -1;
      return 0;
    }
  }

  /* 初めて XLookupKanjiString が呼ばれた時は辞書の初期化などの処理が
     行われる。 */

  if (FirstTime) {
    if (kanjiControl(KC_INITIALIZE, (uiContext)NULL, (char *)NULL) == -1) {
      return -1;
    }
    FirstTime = 0;
  }

  d = keyToContext(dpy, win);

  if (d == (uiContext)NULL) {
    /* このウィンドウからイベントが来たのが始めてだったりするわけよ */
    d = newUiContext(dpy, win);
    if (d == (uiContext)NULL) {
      return NoMoreMemory();
    }
  }


  bzero(kanji_status_return, sizeof(wcKanjiStatus));
  
  d->ch = (unsigned)*buffer_return;
  d->buffer_return = buffer_return;
  d->n_buffer = nbuffer;
  d->kanji_status_return = kanji_status_return;

  debug_message("current_mode(0x%x)\n", d->current_mode,0,0);

  if ( nbytes || functionalChar ) { /* キャラクタコードがとれた場合 */
    int check;

    *buffer_return = key2wchar(d->ch, &check);
    if (!check) {
      return NothingChangedWithBeep(d);
    }

    d->nbytes = nbytes;

    retval = doFunc(d, 0);
#ifdef DEBUG
    checkModec(d);
#endif /* DEBUG */
    return(retval);
  }
  else { /* キャラクタコードがとれなかった場合（シフトキーなど）... */
    d->kanji_status_return->length = -1;
    return 0;
  }
}

uiContext keyToContext();

int
XwcKanjiControl2(display, window, request, arg)
unsigned int display, window, request;
BYTE *arg;
{
  if (request == KC_INITIALIZE || request == KC_FINALIZE ||
      request == KC_SETSERVERNAME || request == KC_SETINITFILENAME ||
      request == KC_SETVERBOSE || request == KC_KEYCONVCALLBACK ||
      request == KC_QUERYCONNECTION || request == KC_SETUSERINFO ||
      request == KC_QUERYCUSTOM) {
    return kanjiControl(request, (uiContext)NULL, (char *)arg);
  }
  else if (/* 0 <= request && (必ず真) */ request < MAX_KC_REQUEST) {
    uiContext d;

    /* 初めて wcKanjiString が呼ばれた時は辞書の初期化などの処理が
       行われる。 */

    if (FirstTime) {
      if (kanjiControl(KC_INITIALIZE, (uiContext)NULL, (char *)NULL) == -1) {
	return -1;
      }
      FirstTime = 0;
    }

    d = keyToContext((unsigned int)display, (unsigned int)window);

    if (d == (uiContext)NULL) {
      d = newUiContext(display, window);
      if (d == (uiContext)NULL) {
	return NoMoreMemory();
      }
    }

    if (request == KC_CLOSEUICONTEXT) {
      rmContext(display, window);
    }
    return kanjiControl(request, d, (char *)arg);
  }
  else {
    return -1;
  }
}

struct map {
  KanjiMode tbl;
  BYTE key;
  KanjiMode mode;
  struct map *next;
} *mapFromHash();

/* cfuncdef

  pushCallback -- コールバックの集合をプッシュする。

  コールバックの集合を格納する配列が malloc されて、それが uiContext に
  プッシュされる。

  malloc された配列が戻り値として返る。

 */

struct callback *
pushCallback(d, env, ev, ex, qu, au)
uiContext d;
mode_context env;
canna_callback_t ev, ex, qu, au;
{
  struct callback *newCB;

  newCB = (struct callback *)malloc(sizeof(struct callback));
  if (newCB) {
    newCB->func[0] = ev;
    newCB->func[1] = ex;
    newCB->func[2] = qu;
    newCB->func[3] = au;
    newCB->env = env;
    newCB->next = d->cb;
    d->cb = newCB;
  }
  return newCB;
}

void
popCallback(d)
uiContext d;
{
  struct callback *oldCB;

  oldCB = d->cb;
  d->cb = oldCB->next;
  free(oldCB);
}

#if defined(WIN) && defined(_RK_h)

extern RkwGetProtocolVersion pro((int *, int *));
extern char *RkwGetServerName pro((void));

exp(struct cannafn) CannaFuncs = {
  {
    RkwGetProtocolVersion,
    RkwGetServerName,
    RkwGetServerVersion,
    RkwInitialize,
    RkwFinalize,
    RkwCreateContext,
    RkwDuplicateContext,
    RkwCloseContext,
    RkwSetDicPath,
    RkwCreateDic,
    RkwSync,
    RkwGetDicList,
    RkwGetMountList,
    RkwMountDic,
    RkwRemountDic,
    RkwUnmountDic,
    RkwDefineDic,
    RkwDeleteDic,
    RkwGetHinshi,
    RkwGetKanji,
    RkwGetYomi,
    RkwGetLex,
    RkwGetStat,
    RkwGetKanjiList,
    RkwFlushYomi,
    RkwGetLastYomi,
    RkwRemoveBun,
    RkwSubstYomi,
    RkwBgnBun,
    RkwEndBun,
    RkwGoTo,
    RkwLeft,
    RkwRight,
    RkwNext,
    RkwPrev,
    RkwNfer,
    RkwXfer,
    RkwResize,
    RkwEnlarge,
    RkwShorten,
    RkwStoreYomi,
    RkwSetAppName,
    RkwSetUserInfo,
    RkwQueryDic,
    RkwCopyDic,
    RkwListDic,
    RkwRemoveDic,
    RkwRenameDic,
    RkwChmodDic,
    RkwGetWordTextDic,
    RkwGetSimpleKanji,
  },
  wcKanjiControl,
  wcKanjiString,
};
#endif

#ifdef WIN
#include "cannacnf.h"

/* Interfaces for CannaGetConfigure/CannaSetConfigure */

#define CANNA_MODE_AllModes 255
#define MAX_KEYS_IN_A_MODE 256

typedef int (*keycallback)(unsigned, unsigned char *, int,
			   unsigned char *, int, char *);

static void
GetCannaKeyOnAMode(unsigned modeid, unsigned char *mode, 
		   keycallback keyfn, char *con)
{
  unsigned char key;
  int i;

  for (i = 0 ; i < MAX_KEYS_IN_A_MODE ; i++) {
    if (mode[i] != CANNA_FN_Undefined) { /* is this required? */
      key = i;
      (*keyfn)(modeid, &key, 1, mode + i, 1, con);
    }
  }
}

static void
GetCannaKeyfunc(keycallback keyfn, char *con)
{
  extern unsigned char default_kmap[], alpha_kmap[], empty_kmap[];

  GetCannaKeyOnAMode(CANNA_MODE_AllModes, default_kmap, keyfn, con);
  GetCannaKeyOnAMode(CANNA_MODE_AlphaMode, alpha_kmap, keyfn, con);
  GetCannaKeyOnAMode(CANNA_MODE_EmptyMode, empty_kmap, keyfn, con);
}

__declspec(dllexport) int
GetConfigure(char *name, struct libconf *conf,
	     struct RegInfo *rinfo, struct metaconf *mconf, char *con)
{
  if (conf) {
    if (conf->cf) {
      InitCannaConfig(conf->cf);
    }
    if (conf->romfn) {
      (*conf->romfn)("", con);
    }
    if (conf->keyfn) {
      GetCannaKeyfunc(conf->keyfn, con);
    }
  }
  return 1;
}

__declspec(dllexport) int
SetConfigure(char *name, struct libconfwrite *conf,
	     struct RegInfo *rinfo, struct metaconf *mconf, char *con)
{
  return 0;
}

#endif
