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
static char rcs_id[] = "@(#) 102.1 $Id: uldefine.c,v 1.6 2003/09/17 08:50:53 aida_s Exp $";
#endif

#include	<errno.h>
#include 	"canna.h"

/*********************************************************************
 *                      wchar_t replace begin                        *
 *********************************************************************/
#ifdef wchar_t
# error "wchar_t is already defined"
#endif
#define wchar_t cannawc

#if !defined(NO_EXTEND_MENU)
#ifdef luna88k
extern int errno;
#endif

exp(int) RkwCreateDic();

static int dicTourokuDo pro((uiContext)),
           checkUsrDic pro((uiContext)),
           dicTourokuYomi pro((uiContext)),
           dicTourokuYomiDo pro((uiContext, canna_callback_t));


static char *shinshitbl1[] = 
{
  "\314\276\273\354",                 /* ̾�� */
  "\270\307\315\255\314\276\273\354", /* ��ͭ̾�� */
  "\306\260\273\354",                 /* ư�� */
  "\267\301\315\306\273\354",         /* ���ƻ� */
  "\267\301\315\306\306\260\273\354", /* ����ư�� */
  "\311\373\273\354",                 /* ���� */
  "\244\275\244\316\302\276",         /* ����¾ */
};

static char *shinshitbl2[] = 
{
  "\303\261\264\301\273\372",           /* ñ���� */
  "\277\364\273\354",                   /* ���� */
  "\317\242\302\316\273\354",           /* Ϣ�λ� */
  "\300\334\302\263\273\354\241\246\264\266\306\260\273\354",/* ��³�졦��ư�� */
};


static int tblflag;

#define TABLE1 1
#define TABLE2 2

#define HINSHI1_SZ (sizeof(shinshitbl1) / sizeof(char *))
#define HINSHI2_SZ (sizeof(shinshitbl2) / sizeof(char *))

#define SONOTA       HINSHI1_SZ - 1

static wchar_t *hinshitbl1[HINSHI1_SZ];
static wchar_t *hinshitbl2[HINSHI2_SZ];

static wchar_t *b1, *b2;

int
initHinshiTable()
{
  int retval = 0;

  retval = setWStrings(hinshitbl1, shinshitbl1, HINSHI1_SZ);
  if (retval != NG) {
    retval = setWStrings(hinshitbl2, shinshitbl2, HINSHI2_SZ);
    b1 = WString("\303\261\270\354?[");
                  /* ñ�� */
    b2 = WString("]");
    if (!b1 || !b2) {
      retval = NG;
    }
  }
  return retval;
}

static
clearTango(d)
uiContext d;
{
  tourokuContext tc = (tourokuContext)d->modec;

  tc->tango_buffer[0] = 0;
  tc->tango_len = 0;
}

clearYomi(d)
uiContext d;
{
  tourokuContext tc = (tourokuContext)d->modec;

  tc->yomi_buffer[0] = 0;
  tc->yomi_len = 0;
}

static
clearTourokuContext(p)
tourokuContext p;
{
  p->id = TOUROKU_CONTEXT;
  p->genbuf[0] = 0;
  p->qbuf[0] = 0;
  p->tango_buffer[0] = 0;
  p->tango_len = 0;
  p->yomi_buffer[0] = 0;
  p->yomi_len = 0;
  p->curHinshi = 0;
  p->newDic = (struct dicname *)0;
  p->hcode[0] = 0;
  p->katsuyou = 0;
  p->workDic2 = (deldicinfo *)0;
  p->workDic3 = (deldicinfo *)0;
  p->udic = (wchar_t **)0;
  p->delContext = 0;

  return(0);
}
  
static tourokuContext
newTourokuContext()
{
  tourokuContext tcxt;

  if ((tcxt = (tourokuContext)malloc(sizeof(tourokuContextRec)))
                                          == (tourokuContext)NULL) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (newTourokuContext) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (newTourokuContext) \244\307\244\255\244\336"
	"\244\273\244\363\244\307\244\267\244\277";
#endif
    return (tourokuContext)NULL;
  }
  clearTourokuContext(tcxt);

  return tcxt;
}

getTourokuContext(d)
uiContext d;
{
  tourokuContext tc;
  int retval = 0;

  if (pushCallback(d, d->modec, NO_CALLBACK, NO_CALLBACK,
                                  NO_CALLBACK, NO_CALLBACK) == 0) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (pushCallback) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (pushCallback) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
#endif
    return(NG);
  }

  if((tc = newTourokuContext()) == (tourokuContext)NULL) {
    popCallback(d);
    return(NG);
  }
  tc->majorMode = d->majorMode;
  tc->next = d->modec;
  d->modec = (mode_context)tc;

  tc->prevMode = d->current_mode;

  return(retval);
}

void
popTourokuMode(d)
uiContext d;
{
  tourokuContext tc = (tourokuContext)d->modec;

  d->modec = tc->next;
  d->current_mode = tc->prevMode;
  free((char *)tc);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ñ����Ͽ��ñ�������                                                      *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
uuTTangoEveryTimeCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
{
  tourokuContext tc = (tourokuContext)env;
  int len, echoLen, revPos;
  wchar_t tmpbuf[ROMEBUFSIZE];
  /* BIGARRAY */

  retval = d->nbytes = 0;

#ifdef DEBUG
  checkModec(d);
#endif
  if((echoLen = d->kanji_status_return->length) < 0 || d->more.todo)
    return(retval);

  if (echoLen == 0) {
    d->kanji_status_return->revPos = 0;
    d->kanji_status_return->revLen = 0;
  }

  if(d->kanji_status_return->info & KanjiGLineInfo &&
     d->kanji_status_return->gline.length > 0) {
    echostrClear(d);
    return 0;
  }

  WStrncpy(tmpbuf, d->kanji_status_return->echoStr, echoLen);
  tmpbuf[echoLen] = (wchar_t)'\0';

  WStrcpy(d->genbuf, b1);
  WStrcat(d->genbuf, tmpbuf);
  WStrcat(d->genbuf, b2);

  revPos = WStrlen(b1);

  len = revPos + echoLen + 1;
  WStrcpy(d->genbuf + len, tc->genbuf); /* ��å����� */

  len += WStrlen(tc->genbuf);
  tc->genbuf[0] = 0;
  d->kanji_status_return->gline.line = d->genbuf;
  d->kanji_status_return->gline.length = len;
  if (d->kanji_status_return->revLen) {
    d->kanji_status_return->gline.revPos =
      d->kanji_status_return->revPos + revPos;
    d->kanji_status_return->gline.revLen = d->kanji_status_return->revLen;
  }
  else { /* ȿž�ΰ褬�ʤ���� */
    d->kanji_status_return->gline.revPos = len - WStrlen(b2);
    d->kanji_status_return->gline.revLen = 1;
  }
  d->kanji_status_return->info |= KanjiGLineInfo;
  d->kanji_status_return->length = 0;

  echostrClear(d);
  checkGLineLen(d);

  return retval;
}

static
uuTTangoExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  tourokuContext tc;

  popCallback(d); /* �ɤߤ� pop */

  tc = (tourokuContext)d->modec;

  WStrncpy(tc->tango_buffer, d->buffer_return, retval);
  tc->tango_buffer[retval] = (wchar_t)'\0';
  tc->tango_len = retval;

  return(dicTourokuYomi(d));
}

uuTTangoQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* �ɤߤ� pop */

  freeAndPopTouroku(d);
  GlineClear(d);
  currentModeInfo(d);
  return prevMenuIfExist(d);
}

static
uuT2TangoEveryTimeCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
{
  yomiContext nyc;
  int echoLen, pos, offset;
  wchar_t tmpbuf[ROMEBUFSIZE];
  /* BIGARRAY */

  nyc = (yomiContext)env;

#ifdef DEBUG
  checkModec(d);
#endif
  if(d->kanji_status_return->info & KanjiThroughInfo) {
    extern KanjiModeRec yomi_mode;
    _do_func_slightly(d, 0, (mode_context)nyc, &yomi_mode);
  } else if(retval > 0){
    /* �������� */
    generalReplace(nyc->kana_buffer, nyc->kAttr, &nyc->kRStartp,
		   &nyc->kCurs, &nyc->kEndp, 0, d->buffer_return,
		   retval, HENKANSUMI | SENTOU);
    generalReplace(nyc->romaji_buffer, nyc->rAttr, &nyc->rStartp,
		   &nyc->rCurs, &nyc->rEndp, 0, d->buffer_return,
		   retval, SENTOU);
    nyc->rStartp = nyc->rCurs;
    nyc->kRStartp = nyc->kCurs;
  }

  d->kanji_status_return->info &= ~(KanjiThroughInfo | KanjiEmptyInfo);
  if((echoLen = d->kanji_status_return->length) < 0)
    return(retval);

  WStrncpy(tmpbuf, d->kanji_status_return->echoStr, echoLen);

  WStrncpy(d->genbuf, nyc->kana_buffer, pos = offset = nyc->kCurs);

  WStrncpy(d->genbuf + pos, tmpbuf, echoLen);
  pos += echoLen;
  WStrncpy(d->genbuf + pos, nyc->kana_buffer + offset, nyc->kEndp - offset);
  pos += nyc->kEndp - offset;
  if (d->kanji_status_return->revLen == 0 && /* ȿžɽ����ʬ�ʤ���... */
      nyc->kEndp - offset) { /* ���ˤ��äĤ�����ʬ������Τʤ� */
    d->kanji_status_return->revLen = 1;
    d->kanji_status_return->revPos = offset + echoLen;
  }
  else {
    d->kanji_status_return->revPos += offset;
  }
  d->kanji_status_return->echoStr = d->genbuf;
  d->kanji_status_return->length = pos;

  return retval;
}

/************************************************
 *  ñ����Ͽ�⡼�ɤ�ȴ����ݤ�ɬ�פʽ�����Ԥ�  *
 ************************************************/
static
uuT2TangoExitCatch(d, retval, nyc)
uiContext d;
int retval;
mode_context nyc;
/* ARGSUSED */
{
  yomiContext yc;

  popCallback(d); /* �ɤߤ� pop */

  yc = (yomiContext)d->modec;
  d->nbytes = retval = yc->kEndp;
  WStrncpy(d->buffer_return, yc->kana_buffer, retval);
  d->buffer_return[retval] = (wchar_t)'\0';

  RomajiClearYomi(d);
  popYomiMode(d);
  d->status = EXIT_CALLBACK;

  return retval;
}

static
uuT2TangoQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* �ɤߤ� pop */

  popYomiMode(d);

  d->status = QUIT_CALLBACK;

  return(0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ñ����Ͽ�μ������                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
uuTMakeDicYesCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  int err = 0, perr = 0;
  tourokuContext tc;
  wchar_t **dp;
  extern defaultContext;

  popCallback(d); /* yesNo ��ݥå� */

  tc = (tourokuContext)d->modec;

  if(defaultContext < 0) {
    if((KanjiInit() < 0) || (defaultContext < 0)) {
      jrKanjiError = KanjiInitError();
      freeAndPopTouroku(d);
      defineEnd(d);
      return(GLineNGReturn(d));
    }
  }
  /* ������� */
  if (RkwCreateDic(defaultContext, tc->newDic->name, 0x80) < 0) {
    err++; if (errno == EPIPE) perr++;
    MBstowcs(d->genbuf, "\274\255\275\361\244\316\300\270\300\256\244\313"
	"\274\272\307\324\244\267\244\336\244\267\244\277", 256);
                          /* ����������˼��Ԥ��ޤ��� */
  } else if(RkwMountDic(defaultContext, tc->newDic->name, 0) < 0) {
    err++; if (errno == EPIPE) perr++;
    MBstowcs(d->genbuf, "\274\255\275\361\244\316\245\336\245\246\245\363"
	"\245\310\244\313\274\272\307\324\244\267\244\336\244\267\244\277", 256);
                          /* ����Υޥ���Ȥ˼��Ԥ��ޤ��� */ 
  } else if(d->contextCache != -1 && 
    RkwMountDic(d->contextCache, tc->newDic->name, 0) < 0) {
    err++; if (errno == EPIPE) perr++;
    MBstowcs(d->genbuf, "\274\255\275\361\244\316\245\336\245\246\245\363"
	"\245\310\244\313\274\272\307\324\244\267\244\336\244\267\244\277", 256);
                          /* ����Υޥ���Ȥ˼��Ԥ��ޤ��� */
  }

  if(err) {
    if (perr) {
      jrKanjiPipeError();
    }
    makeGLineMessage(d, d->genbuf, WStrlen(d->genbuf));
    freeAndPopTouroku(d);
    defineEnd(d);
    currentModeInfo(d);
    return(0);
  }

  tc->newDic->dicflag = DIC_MOUNTED;

  /* ����θ���κǸ���ɲä��� */
  dp = tc->udic;
  if (dp) {
    while (*dp) {
      dp++;
    }
    *dp++ = WString(tc->newDic->name);
    *dp = 0;
  }

  return(dicTourokuTango(d, uuTTangoQuitCatch));
}

static
uuTMakeDicQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* yesNo ��ݥå� */

  freeAndPopTouroku(d);

  return prevMenuIfExist(d);
}

static
uuTMakeDicNoCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* yesNo ��ݥå� */

  freeAndPopTouroku(d);
  defineEnd(d);
  currentModeInfo(d);

  GlineClear(d);
  defineEnd(d);
  return(retval);
}

/*
  �桼������ǥޥ���Ȥ���Ƥ����Τ���Ф�����
 */
wchar_t **
getUserDicName(d)
uiContext d;
/* ARGSUSED */
{
  int nmudic; /* �ޥ���Ȥ���Ƥ���桼������ο� */
  struct dicname *p;
  wchar_t **tourokup, **tp;
  extern defaultContext;

  if(defaultContext < 0) {
    if((KanjiInit() < 0) || (defaultContext < 0)) {
      jrKanjiError = KanjiInitError();
      return (wchar_t **)0;
    }
  }

  for (nmudic = 0, p = kanjidicnames ; p ; p = p->next) {
    if (p->dictype == DIC_USER && p->dicflag == DIC_MOUNTED) {
      nmudic++;
    }
  }

  /* return BUFFER �� alloc */
  if ((tourokup = (wchar_t **)calloc(nmudic + 2, sizeof(wchar_t *)))
                                                  == (wchar_t **)NULL) {
    /* + 2 �ʤΤ� 1 ���������ǽ��������Τ��Ǥ��ߤ�ޡ����򤤤�뤿�� */
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (getUserDicName) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (getUserDicName) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
#endif
    return (wchar_t **)0;
  }

  for (tp = tourokup + nmudic, p = kanjidicnames ; p ; p = p->next) {
    if (p->dictype == DIC_USER && p->dicflag == DIC_MOUNTED) {
      *--tp = WString(p->name);
    }
  }
  tourokup[nmudic] = (wchar_t *)0;

  return (wchar_t **)tourokup;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ñ����Ͽ��ñ�������                                                      *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* �Ϥ�˸ƤФ��ؿ� */

dicTouroku(d)
uiContext d;
{
  tourokuContext tc;
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  if(dicTourokuDo(d) < 0) {
    defineEnd(d);
    return(GLineNGReturn(d));
  }

  tc = (tourokuContext)d->modec;

  /* ����̵����м������ */
  if(!*tc->udic) {
    checkUsrDic(d);
    return(0); /* ������뤫�ɤ�������䤹��⡼�ɤ�����(����)��
		  �֤���פäƤΤϡ�checkUsrDic �Ǥʤˤ����������꤬
		  ȯ��������������ʤ����Ȥ�ɽ���� */
  }
  tblflag = TABLE1;
  return(dicTourokuTango(d, uuTTangoQuitCatch));
}

static
dicTourokuDo(d)
uiContext d;
{
  tourokuContext tc;
  wchar_t **up;
  wchar_t **getUserDicName();

  d->status = 0;

  /* �桼������ǥޥ���Ȥ���Ƥ����Τ��äƤ��� */
  if((up = getUserDicName(d)) == 0) {
    return(NG);
  }

  if (getTourokuContext(d) < 0) {
    if (up) {
      wchar_t **p = up;

      for ( ; *p; p++) {
        WSfree(*p);
      }
      free((char *)up);
    }
    return(NG);
  }

  tc = (tourokuContext)d->modec;

  tc->udic = up;

  return(0);
}

/*
 * ñ����Ͽ�Ѽ���Υ�����
 *  �������ޥ����ե�����ǰ��ֻϤ�˻��ꤵ��Ƥ���
 *  ñ����Ͽ�Ѽ�����դ���
 */
static struct dicname *
findUsrDic()
{
  struct dicname *res = (struct dicname *)0, *p;

  for (p = kanjidicnames ; p ; p = p->next) {
    if (p->dictype == DIC_USER) {
      res = p;
    }
  }
  return res;
}

/* 
 * �ޥ���Ȥ���Ƥ��뼭��Υ����å�
 * ���������ޥ����ե������ñ����Ͽ�Ѽ���Ȥ��ƻ��ꤵ��Ƥ��ơ�
 *   �ޥ���Ȥ˼��Ԥ��Ƥ��뼭�񤬤���
 *     �� �������(��뼭��ϣ��Ĥ���)
 * ���������ޥ����ե������ñ����Ͽ�Ѽ���Ȥ��ƻ��ꤵ��Ƥ��뼭�񤬤ʤ�
 * ���������ޥ����ե������ñ����Ͽ�Ѽ���Ȥ��ƻ��ꤵ��Ƥ��ơ�
 *   �ޥ���Ȥ���Ƥ��뼭�񤬤ʤ�
 */
static
checkUsrDic(d)
uiContext d;
{
  tourokuContext tc = (tourokuContext)d->modec;
  coreContext ync;
  struct dicname *u;
  wchar_t xxxx[512];
  /* BIGARRAY */

  u = findUsrDic();

  /* �������ޥ����ե�����ǡ�ñ����Ͽ�Ѽ���ϻ��ꤵ��Ƥ��뤬��
     �ޥ���Ȥ˼��Ԥ��Ƥ���Ȥ�                                */
  if (u) {
    if (u->dicflag == DIC_MOUNT_FAILED) {
      char tmpbuf[1024];
      sprintf(tmpbuf, "\303\261\270\354\305\320\317\277\315\321\274\255"
	"\275\361\244\254\244\242\244\352\244\336\244\273\244\363\241\243"
	"\274\255\275\361(%s)\244\362\272\356\300\256\244\267\244\336\244"
	"\271\244\253?(y/n)",
	      u->name);
                /* ñ����Ͽ�Ѽ��񤬤���ޤ��󡣼���(%s)��������ޤ��� */
      makeGLineMessageFromString(d, tmpbuf);
      tc->newDic = u; /* ��뼭�� */
      if(getYesNoContext(d,
			 NO_CALLBACK, uuTMakeDicYesCatch,
			 uuTMakeDicQuitCatch, uuTMakeDicNoCatch) < 0) {
	defineEnd(d);
	return(GLineNGReturn(d));
      }
      makeGLineMessage(d, d->genbuf, WStrlen(d->genbuf));
      ync = (coreContext)d->modec;
      ync->majorMode = CANNA_MODE_ExtendMode;
      ync->minorMode = CANNA_MODE_TourokuMode;
    }
  }

  /* �������ޥ����ե�����ǡ�ñ����Ͽ�Ѽ��񤬻��ꤵ��Ƥ��ʤ�����
     ����Ϥ���Ƥ��뤬�ޥ���Ȥ���Ƥ��ʤ��Ȥ�                  */
  if (!u || u->dicflag == DIC_NOT_MOUNTED) {
    MBstowcs(xxxx, "\303\261\270\354\305\320\317\277\315\321\274\255\275\361"
	"\244\254\273\330\304\352\244\265\244\354\244\306\244\244\244\336"
	"\244\273\244\363", 512);
                 /* ñ����Ͽ�Ѽ��񤬻��ꤵ��Ƥ��ޤ��� */
    WStrcpy(d->genbuf, xxxx);
    makeGLineMessage(d, d->genbuf, WStrlen(d->genbuf));
    freeAndPopTouroku(d);
    defineEnd(d);
    currentModeInfo(d);
  }

  return(0);
}

dicTourokuTango(d, quitfunc)
uiContext d;
canna_callback_t quitfunc;
{
  tourokuContext tc = (tourokuContext)d->modec;
  yomiContext yc, yc2;
  int retval = 0;

  yc = GetKanjiString(d, (wchar_t *)0, 0,
		      CANNA_NOTHING_RESTRICTED,
		      (int)CANNA_YOMI_CHGMODE_INHIBITTED,
		      (int)CANNA_YOMI_END_IF_KAKUTEI,
		      CANNA_YOMI_INHIBIT_NONE,
		      uuTTangoEveryTimeCatch, uuTTangoExitCatch,
		      quitfunc);
  if (yc == (yomiContext)0) {
    freeAndPopTouroku(d);
    defineEnd(d);
    currentModeInfo(d);
    return NoMoreMemory();
  }
  yc2 = GetKanjiString(d, (wchar_t *)0, 0,
		      CANNA_NOTHING_RESTRICTED,
		      (int)CANNA_YOMI_CHGMODE_INHIBITTED,
		      (int)!CANNA_YOMI_END_IF_KAKUTEI,
		      CANNA_YOMI_INHIBIT_NONE,
		      uuT2TangoEveryTimeCatch, uuT2TangoExitCatch,
		      uuT2TangoQuitCatch);
  if (yc2 == (yomiContext)0) {
    popYomiMode(d);  /* yc1 �� pop ���� */
    popCallback(d);
    freeAndPopTouroku(d);
    defineEnd(d);
    currentModeInfo(d);
    return NoMoreMemory();
  }
  yc2->generalFlags |= CANNA_YOMI_DELETE_DONT_QUIT;

  yc2->majorMode = CANNA_MODE_ExtendMode;
  yc2->minorMode = CANNA_MODE_TourokuMode;
  currentModeInfo(d);

  return(retval);
}

static
dicTourokuTangoPre(d)
uiContext d;
{
  return dicTourokuTango(d, uuTTangoQuitCatch);
}

static
acDicTourokuTangoPre(d, dn, dm) /* ac means "alert continuation" */
uiContext d;
int dn;
mode_context dm;
/* ARGSUSED */
{
  popCallback(d);
  return dicTourokuTangoPre(d);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ñ����Ͽ���ɤߤ�����                                                      *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
uuTYomiEveryTimeCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
{
  tourokuContext tc = (tourokuContext)env;
  int len, echoLen, revPos;
  wchar_t tmpbuf[ROMEBUFSIZE];

  retval = d->nbytes = 0;

  if((echoLen = d->kanji_status_return->length) < 0)
    return(retval);

  if (echoLen == 0) {
    d->kanji_status_return->revPos = 0;
    d->kanji_status_return->revLen = 0;
  }

  /* ��ꤢ���� echoStr �� d->genbuf ���⤷��ʤ��Τ� copy ���Ƥ��� */
  WStrncpy(tmpbuf, d->kanji_status_return->echoStr, echoLen);

  d->kanji_status_return->info &= ~(KanjiThroughInfo | KanjiEmptyInfo);
  revPos = MBstowcs(d->genbuf, "\303\261\270\354[", ROMEBUFSIZE);
                               /* ñ�� */
  WStrcpy(d->genbuf + revPos, tc->tango_buffer);
  revPos += WStrlen(tc->tango_buffer);
  revPos += MBstowcs(d->genbuf + revPos, "] \306\311\244\337?[", ROMEBUFSIZE - revPos);
                                           /* �ɤ� */
  WStrncpy(d->genbuf + revPos, tmpbuf, echoLen);
  len = echoLen + revPos;
  d->genbuf[len++] = (wchar_t) ']';
  WStrcpy(d->genbuf + len, tc->genbuf);
  len += WStrlen(tc->genbuf);
  tc->genbuf[0] = 0;
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
  d->kanji_status_return->info |= KanjiGLineInfo;
  echostrClear(d);
  checkGLineLen(d);

  return retval;
}

static
uuTYomiExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  tourokuContext tc;

  popCallback(d); /* �ɤߤ� pop */

  tc = (tourokuContext)d->modec;

  WStrncpy(tc->yomi_buffer, d->buffer_return, retval);
  tc->yomi_buffer[retval] = (wchar_t)'\0';
  tc->yomi_len = retval;

  return(dicTourokuHinshi(d));
}

static uuTYomiQuitCatch pro((uiContext, int, mode_context));

static
uuTYomiQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* �ɤߤ� pop */

  clearTango(d);
  clearYomi(d);

  return(dicTourokuTango(d, uuTTangoQuitCatch));
}

static
dicTourokuYomi(d)
uiContext d;
{
  return(dicTourokuYomiDo(d, uuTYomiQuitCatch));
}

static
acDicTourokuYomi(d, dn, dm)
uiContext d;
int dn;
mode_context dm;
/* ARGSUSED */
{
  popCallback(d);
  return dicTourokuYomi(d);
}

static
dicTourokuYomiDo(d, quitfunc)
uiContext d;
canna_callback_t quitfunc;
{
  yomiContext yc;
  tourokuContext tc = (tourokuContext)d->modec;
  int retval = 0;

  if(tc->tango_len < 1) {
    clearTango(d);
    return canna_alert(d, "\303\261\270\354\244\362\306\376\316\317\244\267\244\306\244\257\244\300\244\265\244\244", acDicTourokuTangoPre);
                         /* ñ������Ϥ��Ƥ������� */
  }

  yc = GetKanjiString(d, (wchar_t *)0, 0,
		      CANNA_NOTHING_RESTRICTED,
		      (int)CANNA_YOMI_CHGMODE_INHIBITTED,
		      (int)CANNA_YOMI_END_IF_KAKUTEI,
		      (CANNA_YOMI_INHIBIT_HENKAN | CANNA_YOMI_INHIBIT_ASHEX |
		      CANNA_YOMI_INHIBIT_ASBUSHU),
		      uuTYomiEveryTimeCatch, uuTYomiExitCatch,
		      quitfunc);
  if (yc == (yomiContext)0) {
    freeAndPopTouroku(d);
    defineEnd(d);
    currentModeInfo(d);
    return NoMoreMemory();
  }
  yc->majorMode = CANNA_MODE_ExtendMode;
  yc->minorMode = CANNA_MODE_TourokuMode;
  currentModeInfo(d);

  return(retval);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ñ����Ͽ���ʻ������                                                      *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
uuTHinshiExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  forichiranContext fc;
  tourokuContext tc;
  int cur;

  d->nbytes = 0;

  popCallback(d); /* ������ pop */

  fc = (forichiranContext)d->modec;
  cur = fc->curIkouho;

  popForIchiranMode(d);
  popCallback(d);

  if (tblflag == TABLE1 && cur == SONOTA) {
    tblflag = TABLE2;
    return dicTourokuHinshi(d);
  }

  if (tblflag == TABLE2) {
    cur += SONOTA;
  }

  tc = (tourokuContext)d->modec;

  tc->curHinshi = cur;

  return(dicTourokuHinshiDelivery(d));
}

static
uuTHinshiQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* ������ pop */

  popForIchiranMode(d);
  popCallback(d);

  if (tblflag == TABLE2) {
    tblflag = TABLE1;
    return dicTourokuHinshi(d);
  }

  clearYomi(d);

  return(dicTourokuYomi(d));
}

dicTourokuHinshi(d)
uiContext d;
{
  tourokuContext tc = (tourokuContext)d->modec;
  forichiranContext fc;
  ichiranContext ic;
  unsigned inhibit = 0;
  int currentkouho, retval = 0, numkouho;

  d->status = 0;

  if(tc->yomi_len < 1) {
    return canna_alert(d, "\306\311\244\337\244\362\306\376\316\317\244\267"
	"\244\306\244\257\244\300\244\265\244\244", acDicTourokuYomi);
                          /* �ɤߤ����Ϥ��Ƥ������� */
  }

  if((retval = getForIchiranContext(d)) < 0) {
    freeDic(tc);
    defineEnd(d);
    return(GLineNGReturnTK(d));
  }

  fc = (forichiranContext)d->modec;

  /* selectOne ��Ƥ֤���ν��� */
  if (tblflag == TABLE2) {
    fc->allkouho = hinshitbl2;
    numkouho = HINSHI2_SZ;
  }
  else {
    fc->allkouho = hinshitbl1;
    numkouho = HINSHI1_SZ;
  }

  fc->curIkouho = 0;
  currentkouho = 0;
  if (!cannaconf.HexkeySelect)
    inhibit |= ((unsigned char)NUMBERING | (unsigned char)CHARINSERT); 
  else
    inhibit |= (unsigned char)CHARINSERT; 

  if((retval = selectOne(d, fc->allkouho, &fc->curIkouho, numkouho,
		 BANGOMAX, inhibit, currentkouho, WITH_LIST_CALLBACK,
		 NO_CALLBACK, uuTHinshiExitCatch, 
		 uuTHinshiQuitCatch, uiUtilIchiranTooSmall)) < 0) {
    popForIchiranMode(d);
    popCallback(d);
    freeDic(tc);
    defineEnd(d);
    return(GLineNGReturnTK(d));
  }

  ic = (ichiranContext)d->modec;
  ic->majorMode = CANNA_MODE_ExtendMode;
  ic->minorMode = CANNA_MODE_TourokuHinshiMode;
  currentModeInfo(d);

  /* ��������Ԥ������Ƹ���������Ф��ʤ� */
  if(ic->tooSmall) {
    d->status = AUX_CALLBACK;
    return(retval);
  }

  if ( !(ic->flags & ICHIRAN_ALLOW_CALLBACK) ) {
    makeGlineStatus(d);
  }

  /* d->status = ICHIRAN_EVERYTIME; */

  return(retval);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * jrKanjiControl ��                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

dicTourokuControl(d, tango, quitfunc)
uiContext d;
wchar_t *tango;
canna_callback_t quitfunc;
{
  tourokuContext tc;

  if(dicTourokuDo(d) < 0) {
    return(GLineNGReturn(d));
  }

  tc = (tourokuContext)d->modec;

  if(!*tc->udic) {
    if(checkUsrDic(d) < 0) 
      return(GLineNGReturn(d));
    else
      return(0);
  }

  tblflag = TABLE1;
  if(tango == 0 || tango[0] == 0) {
    return(dicTourokuTango(d, quitfunc));
  }

  WStrcpy(tc->tango_buffer, tango);
  tc->tango_len = WStrlen(tc->tango_buffer);

  return(dicTourokuYomiDo(d, quitfunc));
}
#endif /* NO_EXTEND_MENU */

#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/
