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
static char rcs_id[] = "@(#) 102.1 $Id: ulserver.c,v 1.3 2003/09/17 08:50:53 aida_s Exp $";
#endif

#ifndef NO_EXTEND_MENU
#include	<errno.h>
#include "canna.h"

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

static int serverChangeDo();

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * �����Ф��ڤ�Υ��                                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

serverFin(d)
uiContext d;
{
  int retval = 0;
  yomiContext yc = (yomiContext)d->modec;

#ifndef STANDALONE
  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  d->status = 0;
  killmenu(d);

  jrKanjiPipeError();
  
  makeGLineMessageFromString(d, "\244\253\244\312\264\301\273\372\312\321\264\271\245\265\241\274\245\320\244\310\244\316\300\334\302\263\244\362\300\332\244\352\244\336\244\267\244\277");
            /* ���ʴ����Ѵ������ФȤ���³���ڤ�ޤ��� */
  currentModeInfo(d);
#endif /* STANDALONE */

  return(retval);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * �����Ф��ڤ괹��                                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef STANDALONE

static
uuServerChangeEveryTimeCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  int len, echoLen, revPos;
  static int lmachinename;
  static wchar_t *wmachinename;

  if (!wmachinename) {
    wchar_t xxx[30]; /* 30 �äƤΤ� "�ޥ���̾?[" ����Ĺ���٤Ȥ������� */
    lmachinename = MBstowcs(xxx, "\245\336\245\267\245\363\314\276?[", 30);
                              /* �ޥ���̾ */
    wmachinename = (wchar_t *)malloc((lmachinename + 1)* sizeof(wchar_t));
    if (!wmachinename) {
      return -1;
    }
    WStrcpy(wmachinename, xxx);
  }

  if((echoLen = d->kanji_status_return->length) < 0)
    return(retval);

  if (echoLen == 0) {
    d->kanji_status_return->revPos = 0;
    d->kanji_status_return->revLen = 0;
  }

  WStrncpy(d->genbuf + lmachinename,
	   d->kanji_status_return->echoStr, echoLen);
  /* echoStr == d->genbuf ���Ȥޤ����Τ����ư���� */
  WStrncpy(d->genbuf, wmachinename, lmachinename);
  revPos = len = lmachinename;
  len += echoLen;
  d->genbuf[len++] = (wchar_t)']';

  d->kanji_status_return->gline.line = d->genbuf;
  d->kanji_status_return->gline.length = len;
  if (d->kanji_status_return->revLen) {
    d->kanji_status_return->gline.revPos =
      d->kanji_status_return->revPos + revPos;
    d->kanji_status_return->gline.revLen = d->kanji_status_return->revLen;
  }
  else { /* ȿž�ΰ褬�ʤ���� */
    d->kanji_status_return->gline.revPos = len - 1;
    d->kanji_status_return->gline.revLen = 1;
  }
  d->kanji_status_return->info &= ~(KanjiThroughInfo | KanjiEmptyInfo);
  d->kanji_status_return->info |= KanjiGLineInfo;
  echostrClear(d);
  checkGLineLen(d);

  return retval;
}

static
uuServerChangeExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* �ɤߤ� pop */

  return(serverChangeDo(d, retval));
}

static
uuServerChangeQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* �ɤߤ� pop */

  return prevMenuIfExist(d);
}

extern exp(char *) RkwGetServerName();
#endif /* STANDALONE */

serverChange(d)
uiContext d;
{
  int retval = 0;
  wchar_t *w;
  extern KanjiModeRec yomi_mode;
  extern defaultContext;
  yomiContext yc = (yomiContext)d->modec;

#ifndef STANDALONE
  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  d->status = 0;

  if ((yc = GetKanjiString(d, (wchar_t *)NULL, 0,
		     CANNA_ONLY_ASCII,
		     (int)CANNA_YOMI_CHGMODE_INHIBITTED,
		     (int)CANNA_YOMI_END_IF_KAKUTEI,
		     CANNA_YOMI_INHIBIT_ALL,
		     uuServerChangeEveryTimeCatch, uuServerChangeExitCatch,
		     uuServerChangeQuitCatch))
      == (yomiContext)0) {
    killmenu(d);
    return NoMoreMemory();
  }
  yc->minorMode = CANNA_MODE_ChangingServerMode;
  if(defaultContext != -1) {
    char *servname;
    servname = RkwGetServerName();
    if (servname && (w = WString(servname)) != (wchar_t *)0) {
      RomajiStoreYomi(d, w, (wchar_t *)0);
      WSfree(w);
      yc->kRStartp = yc->kCurs = 0;
      yc->rStartp = yc->rCurs = 0;
      d->current_mode = &yomi_mode;
      makeYomiReturnStruct(d);
    }
  }
  currentModeInfo(d);
#endif /* STANDALONE */

  return(retval);
}
		 
#ifndef STANDALONE
static
serverChangeDo(d, len)
uiContext d;
int len;
{
/* wchar_t ���ɤ����� 256 ���ɤ����� */
  wchar_t newServerName[256];
  wchar_t w1[512];
  char tmpServName[256];
  extern defaultContext;
  char *p;

  d->status = 0;

  if(!len)
    return(serverChange(d));

  WStrncpy(newServerName, d->buffer_return, len);
  newServerName[len] = 0;
#if defined(DEBUG)
  if(iroha_debug)
    printf("iroha_server_name = [%s]\n", newServerName);
#endif

  jrKanjiPipeError();
  WCstombs(tmpServName, newServerName, 256);
  if (RkSetServerName(tmpServName) && (p = index((char *)tmpServName, '@'))) {
    char xxxx[1024];
    *p = '\0';
    sprintf(xxxx, "\244\253\244\312\264\301\273\372\312\321\264\271\245\250\245\363\245\270\245\363 %s \244\317\315\370\315\321\244\307\244\255\244\336\244\273\244\363\n",
	    tmpServName);
          /* ���ʴ����Ѵ����󥸥� %s �����ѤǤ��ޤ��� */
    makeGLineMessageFromString(d, xxxx);

    RkSetServerName((char *)0);
    currentModeInfo(d);
    killmenu(d);
    return 0;
  }

  if(defaultContext == -1) {
    if((KanjiInit() != 0) || (defaultContext == -1)) {
      jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271\245\265\241\274\245\320\244\310\304\314\277\256\244\307\244\255\244\336\244\273\244\363";
                   /* ���ʴ����Ѵ������Ф��̿��Ǥ��ޤ��� */
      killmenu(d);
      return(GLineNGReturn(d));
    }
    d->contextCache = -1;
  }

  p = RkwGetServerName();
  if (p) { /* �����������������ɤ� */
    if ((int)strlen(p) < 256) {
      MBstowcs(newServerName, p, 256);
    }
  }

  MBstowcs(w1, " \244\316\244\253\244\312\264\301\273\372\312\321\264\271\245\265\241\274\245\320\244\313\300\334\302\263\244\267\244\336\244\267\244\277", 512);
              /* �Τ��ʴ����Ѵ������Ф���³���ޤ��� */
  WStrcpy((wchar_t *)d->genbuf, (wchar_t *)newServerName);
  WStrcat((wchar_t *)d->genbuf, (wchar_t *)w1);

  makeGLineMessage(d, d->genbuf, WStrlen(d->genbuf));
  killmenu(d);
  currentModeInfo(d);

  return(0);
}

#endif /* STANDALONE */
#endif /* NO_EXTEND_MENU */

#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/
