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
static	char	rcs_id[] = "@(#) 102.1 $Id: ichiran.c,v 1.3 2003/09/17 08:50:53 aida_s Exp $";
#endif /* lint */

#include	<errno.h>
#include	"canna.h"

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

extern int TanNextKouho();

static void clearIchiranContext();
static int IchiranKakutei();
static void getIchiranPreviousKouhoretsu();
static void getIchiranNextKouhoretsu();


#define ICHISIZE 9
static char *sbango = 
  "\243\261\241\241\243\262\241\241\243\263\241\241\243\264\241\241\243\265"
  "\241\241\243\266\241\241\243\267\241\241\243\270\241\241\243\271\241\241"
  "\243\341\241\241\243\342\241\241\243\343\241\241\243\344\241\241\243\345"
  "\241\241\243\346";
     /* �������������������������������������ᡡ�⡡�㡡�䡡�塡�� */
                                                /* ����Ԥθ����ֹ��ʸ���� */
static wchar_t *bango;

/*  "1.","��2.","��3.","��4.","��5.","��6.","��7.","��8.","��9.",*/
static char  *sbango2[] = {
  "1","\241\2412","\241\2413","\241\2414","\241\2415",
  "\241\2416","\241\2417","\241\2418","\241\2419",
  };

static wchar_t *bango2[ICHISIZE];

static char *skuuhaku = "\241\241"; 
			/* �� */
static wchar_t *kuuhaku;

initIchiran()
{
  int i, retval = 0;
  char buf[16];

  retval = setWStrings(&bango, &sbango, 1);
  if (retval != NG) {
    for(i = 0; i < ICHISIZE; i++) {

      /* ���ѥ졼�����ν��� */
      if (cannaconf.indexSeparator &&
	  0x20 <= cannaconf.indexSeparator &&
	  0x80 > cannaconf.indexSeparator)
        sprintf(buf, "%s%c", sbango2[i], (char)cannaconf.indexSeparator);
      else
        sprintf(buf, "%s%c", sbango2[i], (char)DEFAULTINDEXSEPARATOR);
      
      bango2[i] = WString(buf);
    }

    retval = setWStrings(&kuuhaku, &skuuhaku, 1);
  }
  return retval;
}


/*
 * ������ɽ����Υ�����ʸ��θ���򹹿�����
 *
 * �������ȸ�����Ѥ��롣
 * ������ˤȤ�ʤ� kugiri �⹹�������
 *
 * ������	uiContext
 *              yomiContext
 */
static void
makeIchiranEchoStrCurChange(yc)
yomiContext yc;
{
  RkwXfer(yc->context, yc->curIkouho);
}

/*
 * ���ʴ����Ѵ��Ѥι�¤�Τ����Ƥ򹹿�����(���ξ�Τ�)
 *
 * ��������ƤӽФ����ξ��֤ˤĤ��Ƥ�ɽ��ʸ�������
 *
 * ������	uiContext
 *              yomiContext
 */
static void
makeIchiranKanjiStatusReturn(d, env, yc)
uiContext	d;
mode_context env;
yomiContext yc;
{
  mode_context sv;

  sv = d->modec;
  d->modec = env;
  makeKanjiStatusReturn(d, yc);
  d->modec = sv;
}

#define DEC_COLUMNS(n) ((n) < 10 ? 1 : (n) < 100 ? 2 : (n) < 1000 ? 3 : 4)

/*
 * ����Ԥ˴ؤ��빽¤�Τ����Ƥ򹹿�����
 *
 * ��glineinfo �� kouhoinfo �������Ԥ�������������ȸ����ֹ��ȿž����
 *
 * ������	uiContext
 * �����	�ʤ�
 */
void
makeGlineStatus(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;
  wchar_t *p;
  char str[16];
  int i, cur;

  if (cannaconf.kCount) {
    cur = *(ic->curIkouho) + 1;
  }

  d->kanji_status_return->info |= KanjiGLineInfo;
  d->kanji_status_return->gline.line =
    ic->glineifp[ic->kouhoifp[*(ic->curIkouho)].khretsu].gldata;
  d->kanji_status_return->gline.length = 
    ic->glineifp[ic->kouhoifp[*(ic->curIkouho)].khretsu].gllen;
  d->kanji_status_return->gline.revPos = 
    ic->kouhoifp[*(ic->curIkouho)].khpoint;
  if (cannaconf.ReverseWord && ic->inhibit & NUMBERING) {
    p = ic->glineifp[ic->kouhoifp[*(ic->curIkouho)].khretsu].gldata +
      ic->kouhoifp[*(ic->curIkouho)].khpoint;
    for (i = 0;
         *p != *kuuhaku && *p != ((wchar_t)' ') && *p != ((wchar_t)0)
	 && i < ic->glineifp[ic->kouhoifp[*(ic->curIkouho)].khretsu].gllen;
	 i++) {
      p++;
    }
    d->kanji_status_return->gline.revLen = i;
  } else
    d->kanji_status_return->gline.revLen = 1;

  if (cannaconf.kCount && d->kanji_status_return->gline.length) {
    register int a = ic->nIkouho, b = DEC_COLUMNS(cur) + DEC_COLUMNS(a) + 2;
    sprintf(str, " %d/%d", cur, a);
    MBstowcs(d->kanji_status_return->gline.line + 
	     d->kanji_status_return->gline.length - b, str, b + 1);
    /* �ʲ��Ϥ���ʤ��ΤǤϡ� */
    d->kanji_status_return->gline.length
      = WStrlen(d->kanji_status_return->gline.line);
  }
}

static ichiranEveryTimeCatch pro((uiContext, int, mode_context));

static
ichiranEveryTimeCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
{
  yomiContext yc;

  yc = (yomiContext)env;

  makeIchiranEchoStrCurChange(yc);
  makeIchiranKanjiStatusReturn(d, env, yc);

  return(retval);
}

static ichiranExitCatch pro((uiContext, int, mode_context));

static
ichiranExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
{
  yomiContext yc;

  yc = (yomiContext)env;
  yc->kouhoCount = 0;
  /* d->curIkouho�򥫥��ȸ���Ȥ��� */
  if ((retval = RkwXfer(yc->context, yc->curIkouho)) == NG) {
    if (errno == EPIPE) {
      jrKanjiPipeError();
    }
    jrKanjiError = "\245\253\245\354\245\363\245\310\270\365\312\344\244\362"
	"\274\350\244\352\275\320\244\273\244\336\244\273\244\363\244\307"
	"\244\267\244\277";             
      /* �����ȸ������Ф��ޤ���Ǥ��� */
    /* �����ȸ��䤬���Ф��ʤ����餤�Ǥϲ��Ȥ�ʤ��� */
  }
  else {
    retval = d->nbytes = 0;
  }

  makeIchiranEchoStrCurChange(yc);
  makeIchiranKanjiStatusReturn(d, env, yc);

  freeGetIchiranList(yc->allkouho);
  
  popCallback(d);

  if (!cannaconf.stayAfterValidate && !d->more.todo) {
    d->more.todo = 1;
    d->more.ch = 0;
    d->more.fnum = CANNA_FN_Forward;
  }
  currentModeInfo(d);

  return(retval);
}

static ichiranQuitCatch pro((uiContext, int, mode_context));

static
ichiranQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
{
  yomiContext yc;

  yc = (yomiContext)env;
  yc->kouhoCount = 0;

  if ((retval = RkwXfer(yc->context, yc->curIkouho)) == NG) {
    if(errno == EPIPE) {
      jrKanjiPipeError();
    }
    jrKanjiError = "\245\253\245\354\245\363\245\310\270\365\312\344\244\362"
	"\274\350\244\352\275\320\244\273\244\336\244\273\244\363\244\307"
	"\244\267\244\277";               
           /* �����ȸ������Ф��ޤ���Ǥ��� */
    /* �����ȸ��䤬���Ф��ʤ����餤�Ǥϲ��Ȥ�ʤ��� */
  }
  else {
    retval = d->nbytes = 0;
  }

  makeIchiranEchoStrCurChange(yc);
  makeIchiranKanjiStatusReturn(d, env, yc);

  freeGetIchiranList(yc->allkouho);

  popCallback(d);
  currentModeInfo(d);
  return(retval);
}

void
freeIchiranBuf(ic)
ichiranContext ic;
{
  if(ic->glinebufp)
    free(ic->glinebufp);
  if(ic->kouhoifp)
    free(ic->kouhoifp);
  if(ic->glineifp)
    free(ic->glineifp);
}

void
freeGetIchiranList(buf)
wchar_t **buf;
{
  /* �������ɽ�����ѤΥ��ꥢ��ե꡼���� */
  if(buf) {
    if(*buf) {
      free(*buf);
    }
    free(buf);
  }
}

static void
popIchiranMode(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  d->modec = ic->next;
  d->current_mode = ic->prevMode;
  freeIchiranContext(ic);
}

/*
 * ���٤Ƥθ������Ф��ơ�����ˤ���
 */

static int makeKouhoIchiran();

wchar_t **
getIchiranList(context, nelem, currentkouho)
int context;
int *nelem, *currentkouho;
{
  wchar_t *work, *wptr, **bptr, **buf;
  RkStat st;
  int i;

  /* RkwGetKanjiList �����롢���٤Ƥθ���Τ�����ΰ������ */
  if ((work = (wchar_t *)malloc(ROMEBUFSIZE * sizeof(wchar_t)))
                                               == (wchar_t *)NULL) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (getIchiranList) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (getIchiranList) \244\307\244\255\244\336\244\273\244\363\244\307\244\267\244\277";
#endif
    return (wchar_t **)NULL;
  }

  /* ���٤Ƥθ�������롣
     ��: �������� �� �ٴ�@�ʴ�@�ݴ�@@ (@��NULL) */
  if((*nelem = RkwGetKanjiList(context, work, ROMEBUFSIZE)) < 0) {
    jrKanjiError = "\244\271\244\331\244\306\244\316\270\365\312\344\244\316"
	"\274\350\244\352\275\320\244\267\244\313\274\272\307\324\244\267"
	"\244\336\244\267\244\277";
                   /* ���٤Ƥθ���μ��Ф��˼��Ԥ��ޤ��� */
    free(work);
    return (wchar_t **)NULL;
  }

#ifdef	INHIBIT_DUPLICATION
  if (*nelem == 3) {
    wchar_t *w1, *w2, *w3;

    w1 = work;
    w2 = w1 + WStrlen(w1);
    w3 = w2 + WStrlen(w2);
    if (!WStrcmp(w1, ++w3)) {
      if (!WStrcmp(w1, ++w2))
	*nelem = 1;
      else
	*nelem = 2;
    }
  }
#endif	/* INHIBIT_DUPLICATION */

  /* makeKouhoIchiran()���Ϥ��ǡ��� */
  if((buf = (wchar_t **)calloc
      (*nelem + 1, sizeof(wchar_t *))) == (wchar_t **)NULL) {
    jrKanjiError = "malloc (getIchiranList) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
                                            /* �Ǥ��ޤ���Ǥ��� */
    free(work);
    return (wchar_t **)NULL;
  }
  for(wptr = work, bptr = buf, i = 0; *wptr && i++ < *nelem; bptr++) {
    *bptr = wptr;
    while(*wptr++)
      /* EMPTY */
      ;
  }
  *bptr = (wchar_t *)0;

  if(RkwGetStat(context, &st) == -1) {
    jrKanjiError = "\245\271\245\306\245\244\245\277\245\271\244\362\274\350"
	"\244\352\275\320\244\273\244\336\244\273\244\363\244\307\244\267"
	"\244\277";
                   /* ���ƥ���������Ф��ޤ���Ǥ��� */
    free(work);
    free(buf);
    return (wchar_t **)NULL;
  }
  *currentkouho = st.candnum; /* �����ȸ���ϲ����ܡ� */

  return(buf);
}

/* cfunc ichiranContext
 *
 * ichiranContext ��������Ѥι�¤�Τ�����������
 *
 */
ichiranContext
newIchiranContext()
{
  ichiranContext icxt;

  if ((icxt = (ichiranContext)malloc(sizeof(ichiranContextRec)))
                                          == (ichiranContext)NULL) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (newIchiranContext) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (newIchiranContext) \244\307\244\255\244\336"
	"\244\273\244\363\244\307\244\267\244\277";
#endif
    return (ichiranContext)NULL;
  }
  clearIchiranContext(icxt);

  return icxt;
}

/*
 * ��������Ԥ���
 */

#ifdef __STDC__
int
selectOne(uiContext d, wchar_t **buf, int *ck, int nelem, int bangomax,
	  unsigned inhibit, int currentkouho, int allowcallback,
	  canna_callback_t everyTimeCallback, canna_callback_t exitCallback,
	  canna_callback_t quitCallback, canna_callback_t auxCallback)
#else
selectOne(d, buf, ck, nelem, bangomax, inhibit, currentkouho, allowcallback,
	  everyTimeCallback, exitCallback, quitCallback, auxCallback)
uiContext d;
wchar_t **buf;
int *ck;
int nelem, bangomax;
unsigned inhibit;
int currentkouho;
int allowcallback;
canna_callback_t everyTimeCallback, exitCallback, quitCallback, auxCallback;
#endif
{
  extern KanjiModeRec ichiran_mode;
  ichiranContext ic;

  if (allowcallback != WITHOUT_LIST_CALLBACK &&
      d->list_func == (int (*) pro((char *, int, wchar_t **, int, int *)))0) {
    allowcallback = WITHOUT_LIST_CALLBACK;
  }

  if(pushCallback(d, d->modec,
	everyTimeCallback, exitCallback, quitCallback, auxCallback) == 0) {
    jrKanjiError = "malloc (pushCallback) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
                                          /* �Ǥ��ޤ���Ǥ��� */
    return(NG);
  }
  
  if((ic = newIchiranContext()) == (ichiranContext)NULL) {
    popCallback(d);
    return(NG);
  }
  ic->majorMode = d->majorMode;
  ic->next = d->modec;
  d->modec = (mode_context)ic;

  ic->prevMode = d->current_mode;
  d->current_mode = &ichiran_mode;
  d->flags &= ~(PLEASE_CLEAR_GLINE | PCG_RECOGNIZED);
  /* �����ˤ���ľ���� C-t �Ȥ��� Gline ��ɽ������Ƥ������Σ��Ԥ�
     ���ɬ�פ��ФƤ��롣 */

  ic->allkouho = buf;
  ic->curIkouho = ck;
  ic->inhibit = inhibit;
  ic->nIkouho = nelem;

  if (allowcallback != WITHOUT_LIST_CALLBACK) {
    ic->flags |= ICHIRAN_ALLOW_CALLBACK;
    /* listcallback �Ǥ��ֹ�ϤĤ��ʤ� */
    ic->inhibit |= NUMBERING;
  }

  if (allowcallback == WITHOUT_LIST_CALLBACK) {
    if (makeKouhoIchiran(d, nelem, bangomax, inhibit, currentkouho)   == NG) {
      popIchiranMode(d);
      popCallback(d);
      return(NG);
    }
  }
  else {
    if (cannaconf.kCount) {
      *(ic->curIkouho) += currentkouho;
      if (*(ic->curIkouho) >= ic->nIkouho)
        ic->svIkouho = *(ic->curIkouho) = 0;
    }
    d->list_func(d->client_data, CANNA_LIST_Start, buf, nelem, ic->curIkouho);
  }

  return(0);
}

/*
 * IchiranContext �ν����
 */
static void
clearIchiranContext(p)
ichiranContext p;
{
  p->id = ICHIRAN_CONTEXT;
  p->svIkouho = 0;
  p->curIkouho = 0;
  p->nIkouho = 0;
  p->tooSmall = 0;
  p->curIchar = 0;
  p->allkouho = 0;
  p->glinebufp = 0;
  p->kouhoifp = (kouhoinfo *)0;
  p->glineifp = (glineinfo *)0;
  p->flags = (unsigned char)0;
}
  
/*
 * ��������Υǡ�����¤�Τ��뤿����ΰ����ݤ���
 */
allocIchiranBuf(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;
  int size;

  /* ��������ʬ���ֹ��ʬ���ΰ������*/
  size = ic->nIkouho * (d->ncolumns + 1) * WCHARSIZE; /* ������� */
  if((ic->glinebufp = (wchar_t *)malloc(size)) ==  (wchar_t *)NULL) {
    jrKanjiError = "malloc (allocIchiranBuf) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
                                             /* �Ǥ��ޤ���Ǥ��� */
    return(NG);
  }

  /* kouhoinfo���ΰ������ */
  size = (ic->nIkouho + 1) * sizeof(kouhoinfo);
  if((ic->kouhoifp = (kouhoinfo *)malloc(size)) == (kouhoinfo *)NULL) {
    jrKanjiError = "malloc (allocIchiranBuf) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
                                             /* �Ǥ��ޤ���Ǥ��� */
    free(ic->glinebufp);
    return(NG);
  }

  /* glineinfo���ΰ������ */
  size = (ic->nIkouho + 1) * sizeof(glineinfo);
  if((ic->glineifp = (glineinfo *)malloc(size)) == (glineinfo *)NULL) {
    jrKanjiError = "malloc (allocIchiranBuf) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
                                             /* �Ǥ��ޤ���Ǥ��� */
    free(ic->glinebufp);
    free(ic->kouhoifp);
    return(NG);
  }
  return(0);
}

/*
 * ��������Ԥ�ɽ���ѤΥǡ�����ơ��֥�˺�������
 *
 * ��glineinfo �� kouhoinfo���������
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
static int
makeKouhoIchiran(d, nelem, bangomax, inhibit, currentkouho)
uiContext d;
int nelem, bangomax;
unsigned char inhibit;
int currentkouho;
{
  ichiranContext ic = (ichiranContext)d->modec;
  wchar_t **kkptr, *kptr, *gptr, *svgptr;
  int           ko, lnko, cn = 0, svcn, line = 0, dn = 0, svdn;
  int netwidth;

  netwidth = d->ncolumns -
    (cannaconf.kCount ? (DEC_COLUMNS(nelem) * 2 + 2/* 2��/��SP��ʬ */) : 0);

  ic->nIkouho = nelem;	/* ����ο� */

  /* �����ȸ���򥻥åȤ��� */
  ic->svIkouho = *(ic->curIkouho);
  *(ic->curIkouho) += currentkouho;
  if(*(ic->curIkouho) >= ic->nIkouho)
    ic->svIkouho = *(ic->curIkouho) = 0;

  if(allocIchiranBuf(d) == NG)
    return(NG);

  if(d->ncolumns < 1) {
    ic->tooSmall = 1;
    return(0);
  }

  /* glineinfo��kouhoinfo���� */
  /* 
   ��glineinfo��
      int glkosu   : int glhead     : int gllen  : wchar_t *gldata
      ���Ԥθ���� : ��Ƭ���䤬     : ���Ԥ�Ĺ�� : ��������Ԥ�ʸ����
                   : �����ܤθ��䤫 :
   -------------------------------------------------------------------------
   0 | 6           : 0              : 24         : �����������ʣ�����������
   1 | 4           : 6              : 16         : ���ã�����������

    ��kouhoinfo��
      int khretsu  : int khpoint  : wchar_t *khdata
      �ʤ����ܤ�   : �Ԥ���Ƭ���� : �����ʸ����
      ������䤫   : ���Х����ܤ� :
   -------------------------------------------------------------------------
   0 | 0           : 0            : ��
   1 | 0           : 4            : ��
             :                :             :
   7 | 1           : 0            : ��
   8 | 1           : 4            : ��
  */

  kkptr = ic->allkouho;
  kptr = *(ic->allkouho);
  gptr = ic->glinebufp;

  /* line -- �����ܤ�
     ko   -- ���Τ���Ƭ���鲿���ܤθ��䤫
     lnko -- �����Ƭ���鲿���ܤθ��䤫
     cn   -- �����Ƭ���鲿�Х����ܤ� */

  for(line=0, ko=0; ko<ic->nIkouho; line++) {
    ic->glineifp[line].gldata = gptr; /* ����Ԥ�ɽ�����뤿���ʸ���� */
    ic->glineifp[line].glhead = ko;   /* ���ιԤ���Ƭ����ϡ����ΤǤ�ko���� */

    ic->tooSmall = 1;
    for (lnko = cn = dn = 0 ;
	dn < netwidth && lnko < bangomax && ko < ic->nIkouho ; lnko++, ko++) {
      ic->tooSmall = 0;
      kptr = kkptr[ko];
      ic->kouhoifp[ko].khretsu = line; /* �����ܤ�¸�ߤ��뤫��Ͽ */
      ic->kouhoifp[ko].khpoint = cn + (lnko ? 1 : 0);
      ic->kouhoifp[ko].khdata = kptr;  /* ����ʸ����ؤΥݥ��� */
      svgptr = gptr;
      svcn = cn;
      svdn = dn;
      /* �������ɽ����ʬ���� */
      if(!(inhibit & (unsigned char)NUMBERING)) {
	/* �ֹ�򥳥ԡ����� */
	if (!cannaconf.indexHankaku) {/* ���� */
	  if(lnko == 0) {
	    *gptr++ = *bango; cn ++; dn +=2;
	  } else {
	    WStrncpy(gptr, bango + 1 + BANGOSIZE * (lnko - 1), BANGOSIZE);
	    cn += BANGOSIZE; gptr += BANGOSIZE, dn += BANGOSIZE*2;
	  }
	}
	else{ /* Ⱦ�� */
	  WStrcpy(gptr, bango2[lnko]);
	  if(lnko == 0) {
	    dn +=2;
	  } else {
	    dn +=4;
	  }
	  cn += WStrlen(bango2[lnko]);
	  gptr += WStrlen(bango2[lnko]);
	}
      } else {
	/* ����򥳥ԡ����� */
	if(lnko) {
	  *gptr++ = *kuuhaku; cn ++; dn +=2;
	}
      }
      /* ����򥳥ԡ����� */
      for (; *kptr && dn < netwidth ; gptr++, kptr++, cn++) {
	*gptr = *kptr;
	if (WIsG0(*gptr))
	  dn++;
	else if (WIsG1(*gptr))
	  dn += 2;
	else if (WIsG2(*gptr))
	  dn ++;
	else if (WIsG3(*gptr))
	  dn += 2;
      }

      /* ���������Ϥߤ����Ƥ��ޤ������ˤʤä��Τǣ����᤹ */
      if (dn >= netwidth) {
	if (lnko) {
	  gptr = svgptr;
	  cn = svcn;
	  dn = svdn;
	}
	else {
	  ic->tooSmall = 1;
	}
	break;
      }
    }
    if (ic->tooSmall) {
      return 0;
    }
    if (cannaconf.kCount) {
      for (;dn < d->ncolumns - 1; dn++)
	*gptr++ = ' ';
    }
    /* ���Խ���� */
    *gptr++ = 0;
    ic->glineifp[line].glkosu = lnko;
    ic->glineifp[line].gllen = WStrlen(ic->glineifp[line].gldata);
  }

  /* �Ǹ��NULL������� */
  ic->kouhoifp[ko].khretsu = 0;
  ic->kouhoifp[ko].khpoint = 0;
  ic->kouhoifp[ko].khdata  = (wchar_t *)NULL;
  ic->glineifp[line].glkosu  = 0;
  ic->glineifp[line].glhead  = 0;
  ic->glineifp[line].gllen   = 0;
  ic->glineifp[line].gldata  = (wchar_t *)NULL;

#if defined(DEBUG)
  if (iroha_debug) {
    int i;
    for(i=0; ic->glineifp[i].glkosu; i++)
      printf("%d: %s\n", i, ic->glineifp[i].gldata);
  }
#endif

  return(0);
}

tanKouhoIchiran(d, step)
uiContext d;
int step;
{
  yomiContext yc = (yomiContext)d->modec;
  ichiranContext ic;
  int nelem, currentkouho, retval = 0;
  unsigned inhibit = 0;
  unsigned char listcallback = (unsigned char)(d->list_func ? 1 : 0);
  int netwidth;

  netwidth = d->ncolumns -
    (cannaconf.kCount ? (DEC_COLUMNS(9999) * 2 + 2/* 2�� / �� SP ��ʬ */) : 0);

  /* ��������Ԥ������Ƹ���������Ф��ʤ� */
  if (listcallback == 0 && netwidth < 2) {
    /* tooSmall */
    return TanNextKouho(d);
  }

  /* �༡��Ϣ */
  yc->status |= CHIKUJI_OVERWRAP;

  /* ���٤Ƥθ������Ф� */
  yc->allkouho = getIchiranList(yc->context, &nelem, &currentkouho);
  if (yc->allkouho == 0) {
    if (errno == EPIPE) {
      jrKanjiPipeError();
    }
    TanMuhenkan(d);
    makeGLineMessageFromString(d, jrKanjiError);
    return 0;
  }

  if (!cannaconf.HexkeySelect) {
    inhibit |= (unsigned char)NUMBERING;
  }

  yc->curIkouho = currentkouho;	/* ���ߤΥ����ȸ����ֹ����¸���� */
  currentkouho = step;	/* �����ȸ��䤫�鲿���ܤ򥫥��ȸ���Ȥ��뤫 */

  /* ��������˰ܹԤ��� */
  if ((retval = selectOne(d, yc->allkouho, &yc->curIkouho, nelem, BANGOMAX,
			  inhibit, currentkouho, WITH_LIST_CALLBACK,
			  ichiranEveryTimeCatch, ichiranExitCatch,
			  ichiranQuitCatch, NO_CALLBACK)) == NG) {
    freeGetIchiranList(yc->allkouho);
    return GLineNGReturn(d);
  }

  ic = (ichiranContext)d->modec;
  if (ic->tooSmall) {
    freeGetIchiranList(yc->allkouho);
    popIchiranMode(d);
    popCallback(d);
    return TanNextKouho(d);
  }

  ic->minorMode = CANNA_MODE_IchiranMode;
  currentModeInfo(d);

  if ( !(ic->flags & ICHIRAN_ALLOW_CALLBACK) ) {
    makeGlineStatus(d);
  }
  /* d->status = EVERYTIME_CALLBACK; */

  return(retval);
}

/*
 * ��������Ԥ�ɽ��������λ����
 */
IchiranQuit(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;
  int retval = 0;

  if (ic->flags & ICHIRAN_ALLOW_CALLBACK &&
      d->list_func) {
    if (ic->flags & ICHIRAN_NEXT_EXIT) {
      d->list_func(d->client_data,
                     CANNA_LIST_Select, (wchar_t **)0, 0, (int *)0);
    }
    else {
      d->list_func(d->client_data,
                     CANNA_LIST_Quit, (wchar_t **)0, 0, (int *)0);
    }
  }
  
  if (ic->flags & ICHIRAN_NEXT_EXIT) {
    ichiranFin(d); 
    d->status = EXIT_CALLBACK;
  }
  else {
    *(ic->curIkouho) = ic->nIkouho - 1; /* �Ҥ餬�ʸ���ˤ��� */
    ichiranFin(d); 
    d->status = QUIT_CALLBACK;
  }
  return(retval);
}

int
IchiranNop(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  if ((ic->flags & ICHIRAN_ALLOW_CALLBACK) && d->list_func) {
    (*d->list_func)
      (d->client_data, CANNA_LIST_Query, (wchar_t **)0, 0, (int *)0);
  }

  /* currentModeInfo �ǥ⡼�ɾ���ɬ���֤�褦�˥��ߡ��Υ⡼�ɤ�����Ƥ��� */
  d->majorMode = d->minorMode = CANNA_MODE_AlphaMode;
  currentModeInfo(d);

  if (!(ic->flags & ICHIRAN_ALLOW_CALLBACK)) {
    makeGlineStatus(d);
  }
  return 0;
}

/*
   IchiranKakuteiThenDo

     -- Do determine from the candidate list, then do one more function.
 */

static
IchiranKakuteiThenDo(d, func)
uiContext d;
int func;
{
  ichiranContext ic = (ichiranContext)d->modec;
  int retval;
  BYTE ifl = ic->flags;

  if (!ic->prevMode || !ic->prevMode->func ||
      !(*ic->prevMode->func)((uiContext)0/*dummy*/, ic->prevMode, KEY_CHECK,
			     0/*dummy*/, func)) {
    return NothingChangedWithBeep(d);
  }
  retval = IchiranKakutei(d);
  if (ifl & ICHIRAN_STAY_LONG) {
    (void)IchiranQuit(d);
  }
  d->more.todo = 1;
  d->more.ch = d->ch;
  d->more.fnum = func;
  return retval;
}

static
IchiranQuitThenDo(d, func)
uiContext d;
int func;
{
  ichiranContext ic = (ichiranContext)d->modec;
  int retval;

  if (!ic->prevMode || !ic->prevMode->func ||
      !(*ic->prevMode->func)((uiContext)0/*dummy*/, ic->prevMode, KEY_CHECK,
			     0/*dummy*/, func)) {
    return NothingChangedWithBeep(d);
  }
  retval = IchiranQuit(d);
  d->more.todo = 1;
  d->more.ch = d->ch;
  d->more.fnum = func;
  return retval;
}

/*
 * ������˰�ư����
 *
 * �������ȸ��䤬�ǽ�������ä�����Ƭ����򥫥��ȸ���Ȥ���
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
IchiranForwardKouho(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  if (ic->flags & ICHIRAN_ALLOW_CALLBACK &&
      d->list_func) {
    int res;
    
    res = (*d->list_func)
      (d->client_data, CANNA_LIST_Forward, (wchar_t **)0, 0, (int *)0);
    if (res) {
      return 0;
    }
    else { /* CANNA_LIST_Forward was not prepared at the callback func */
      return IchiranKakuteiThenDo(d, CANNA_FN_Forward);
    }
  }

  /* ������ˤ��� (ñ�����������֤ǡ��Ǹ�θ�����ä�����������) */
  *(ic->curIkouho) += 1;
  if(*(ic->curIkouho) >= ic->nIkouho) {
    if (cannaconf.QuitIchiranIfEnd
       && (((coreContext)d->modec)->minorMode == CANNA_MODE_IchiranMode)) {
      return(IchiranQuit(d));
    }
    else if (cannaconf.CursorWrap) {
      *(ic->curIkouho) = 0;
    } else {
      *(ic->curIkouho) -= 1;
      return NothingChangedWithBeep(d);
    }
  }

  if(ic->tooSmall) { /* for bushu */
    d->status = AUX_CALLBACK;
    return 0;
  }

  makeGlineStatus(d);
  /* d->status = EVERYTIME_CALLBACK; */

  return 0;
}

/*
 * ������˰�ư����
 *
 * �������ȸ��䤬��Ƭ������ä���ǽ�����򥫥��ȸ���Ȥ���
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
IchiranBackwardKouho(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;
  BYTE mode;

  if (ic->flags & ICHIRAN_ALLOW_CALLBACK &&
      d->list_func) {
    int res;
    res = (*d->list_func)
      (d->client_data, CANNA_LIST_Backward, (wchar_t **)0, 0, (int *)0);
    if (res) {
      return 0;
    }
    else { /* CANNA_LIST_Backward was not prepared at the callback func */
      return IchiranKakuteiThenDo(d, CANNA_FN_Backward);
    }
  }

  /* ���ߤΥ⡼�ɤ���� */
  if (cannaconf.QuitIchiranIfEnd)
    mode = ((coreContext)d->modec)->minorMode;

  /* ������ˤ��� (ñ�����������֤ǡ��ǽ�θ�����ä�����������) */
  if(*(ic->curIkouho))
    *(ic->curIkouho) -= 1;
  else {
    if (cannaconf.QuitIchiranIfEnd && (mode == CANNA_MODE_IchiranMode)) {
      return(IchiranQuit(d));
    }
    else if (cannaconf.CursorWrap) {
      *(ic->curIkouho) = ic->nIkouho - 1;
    } else {
      *(ic->curIkouho) = 0;
      return NothingChangedWithBeep(d);
    }
  }

  if(ic->tooSmall) { /* for bushu */
    d->status = AUX_CALLBACK;
    return 0;
  }

  makeGlineStatus(d);
  /* d->status = EVERYTIME_CALLBACK; */

  return 0;
}

/*
   IchiranConvert() will be called when `convert' key is pressed
 */

static IchiranConvert pro((uiContext));

static
IchiranConvert(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  if (ic->flags & ICHIRAN_ALLOW_CALLBACK && d->list_func) {
    (*d->list_func)
      (d->client_data, CANNA_LIST_Convert, (wchar_t **)0, 0, (int *)0);
    return 0;
  }
  else {
    return IchiranForwardKouho(d);
  }
}

/*
 * ��������˰�ư����
 *
 * �������ȸ������Ƹ�������Ȥ��ξ�θ����ɽ������
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
IchiranPreviousKouhoretsu(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  if (ic->flags & ICHIRAN_ALLOW_CALLBACK &&
      d->list_func) {
    int res;
    res = (*d->list_func)
      (d->client_data, CANNA_LIST_Prev, (wchar_t **)0, 0, (int *)0);
    if (res) {
      return 0;
    }
    else { /* CANNA_LIST_Backward was not prepared at the callback func */
      return IchiranKakuteiThenDo(d, CANNA_FN_Prev);
    }
  }

  if(ic->tooSmall) { /* for bushu */
    return(IchiranBackwardKouho(d));
  }

  /* ��������ˤ��� (*(ic->curIkouho)�����)*/
  getIchiranPreviousKouhoretsu(d);

  makeGlineStatus(d);
  /* d->status = EVERYTIME_CALLBACK; */

  return 0;
}

/*
 * ��������Υ����ȸ�������
 *
 * �������������Ʊ�������ֹ�Τ�Τ򥫥��ȸ���Ȥ���
 * �������ֹ��Ʊ����Τ��ʤ����ϡ����θ�����κǽ�����򥫥��ȸ���Ȥ���
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
static void
getIchiranPreviousKouhoretsu(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;
  int kindex;
  int curretsu, nretsu;

  /* �����ȸ���ԤΤʤ��ǲ����ܤθ��䤫�ʤΤ������� */
  kindex = *(ic->curIkouho) - 
    ic->glineifp[ic->kouhoifp[*(ic->curIkouho)].khretsu].glhead;
  /* ������������� */
  curretsu = ic->kouhoifp[*(ic->curIkouho)].khretsu;
  nretsu = ic->kouhoifp[ic->nIkouho - 1].khretsu + 1;
  if(curretsu == 0) {
    if (cannaconf.CursorWrap)
      curretsu = nretsu;
    else {
      NothingChangedWithBeep(d);
      return;
    }
  }
  curretsu -= 1;
  /* kindex �������ȸ�����θ��������礭���ʤäƤ��ޤä���
     �Ǳ�����򥫥��ȸ���Ȥ��� */
  if(ic->glineifp[curretsu].glkosu <= kindex) 
    kindex = ic->glineifp[curretsu].glkosu - 1;
  /* ���������Ʊ���ֹ�˰�ư���� */
  *(ic->curIkouho) = kindex + ic->glineifp[curretsu].glhead;
  return;
}

/*
 * ��������˰�ư����
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
IchiranNextKouhoretsu(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  if (ic->flags & ICHIRAN_ALLOW_CALLBACK &&
      d->list_func) {
    int res;
    res = (*d->list_func)
      (d->client_data, CANNA_LIST_Next, (wchar_t **)0, 0, (int *)0);
    if (res) {
      return 0;
    }
    else { /* CANNA_LIST_Backward was not prepared at the callback func */
      return IchiranKakuteiThenDo(d, CANNA_FN_Next);
    }
  }

  if(ic->tooSmall) {
    return(IchiranForwardKouho(d));
  }

  /* ��������ˤ��� (*(ic->curIkouho) �����) */
  getIchiranNextKouhoretsu(d);

  makeGlineStatus(d);
  /* d->status = EVERYTIME_CALLBACK; */

  return 0;
}

/*
 * �������Ǥ˰�ư����
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */

static IchiranNextPage pro((uiContext));

static
IchiranNextPage(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  if (ic->flags & ICHIRAN_ALLOW_CALLBACK &&
      d->list_func) {
    int res;
    res = (*d->list_func)
      (d->client_data, CANNA_LIST_PageDown, (wchar_t **)0, 0, (int *)0);
    if (res) {
      return 0;
    }
    else { /* CANNA_LIST_Backward was not prepared at the callback func */
      return IchiranKakuteiThenDo(d, CANNA_FN_PageDown);
    }
  }

  return IchiranNextKouhoretsu(d);
}

/*
 * �������Ǥ˰�ư����
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */

static IchiranPreviousPage pro((uiContext));

static
IchiranPreviousPage(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  if (ic->flags & ICHIRAN_ALLOW_CALLBACK &&
      d->list_func) {
    int res;
    res = (*d->list_func)
      (d->client_data, CANNA_LIST_PageUp, (wchar_t **)0, 0, (int *)0);
    if (res) {
      return 0;
    }
    else { /* CANNA_LIST_Backward was not prepared at the callback func */
      return IchiranKakuteiThenDo(d, CANNA_FN_PageUp);
    }
  }

  return IchiranPreviousKouhoretsu(d);
}

/*
 * ��������˰�ư����
 *
 * �������������Ʊ�������ֹ�Τ�Τ򥫥��ȸ���Ȥ���
 * �������ֹ��Ʊ����Τ��ʤ����ϡ����θ�����κǽ�����򥫥��ȸ���Ȥ���
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
static void
getIchiranNextKouhoretsu(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;
  int kindex;
  int curretsu, nretsu;

  /* �����ȸ���ԤΤʤ��ǲ����ܤθ��䤫�ʤΤ������� */
  kindex = *(ic->curIkouho) - 
    ic->glineifp[ic->kouhoifp[*(ic->curIkouho)].khretsu].glhead;
  /* ������������� */
  curretsu = ic->kouhoifp[*(ic->curIkouho)].khretsu;
  nretsu = ic->kouhoifp[ic->nIkouho - 1].khretsu + 1;
  curretsu += 1;
  if(curretsu >= nretsu) {
    if (cannaconf.CursorWrap)
      curretsu = 0;
    else {
      NothingChangedWithBeep(d);
      return;
    }
  }
  /* kindex �������ȸ�����θ��������礭���ʤäƤ��ޤä���
     �Ǳ�����򥫥��ȸ���Ȥ��� */
  if(ic->glineifp[curretsu].glkosu <= kindex) 
    kindex = ic->glineifp[curretsu].glkosu - 1;
  /* ���������Ʊ���ֹ�˰�ư���� */
  *(ic->curIkouho) = kindex + ic->glineifp[curretsu].glhead;
  return;
}

/*
 * ����������Ƭ����˰�ư����
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
IchiranBeginningOfKouho(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  if (ic->flags & ICHIRAN_ALLOW_CALLBACK &&
      d->list_func) {
    int res;
    res = (*d->list_func)
      (d->client_data, CANNA_LIST_BeginningOfLine, (wchar_t **)0, 0,(int *)0);
    if (res) {
      return 0;
    }
    else { /* CANNA_LIST_Backward was not prepared at the callback func */
      return IchiranKakuteiThenDo(d, CANNA_FN_BeginningOfLine);
    }
  }

  if(ic->tooSmall) {
    d->status = AUX_CALLBACK;
    return 0;
  }

  /* ���������Ƭ����򥫥��ȸ���ˤ��� */
  *(ic->curIkouho) = 
    ic->glineifp[ic->kouhoifp[*(ic->curIkouho)].khretsu].glhead;

  makeGlineStatus(d);
  /* d->status = EVERYTIME_CALLBACK; */

  return 0;
}

/*
 * �������κǱ�����˰�ư����
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
IchiranEndOfKouho(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  if (ic->flags & ICHIRAN_ALLOW_CALLBACK &&
      d->list_func) {
    int res;
    res = (*d->list_func)
      (d->client_data, CANNA_LIST_EndOfLine, (wchar_t **)0, 0, (int *)0);
    if (res) {
      return 0;
    }
    else { /* CANNA_LIST_Backward was not prepared at the callback func */
      return IchiranKakuteiThenDo(d, CANNA_FN_EndOfLine);
    }
  }

  if(ic->tooSmall) {
    d->status = AUX_CALLBACK;
    return 0;
  }

  /* ������κǱ�����򥫥��ȸ���ˤ��� */
  *(ic->curIkouho) = 
    ic->glineifp[ic->kouhoifp[*(ic->curIkouho)].khretsu].glhead
    + ic->glineifp[ic->kouhoifp[*(ic->curIkouho)].khretsu].glkosu - 1;

  makeGlineStatus(d);
  /* d->status = EVERYTIME_CALLBACK; */

  return 0;
}

/*
 * �����������Ϥ��줿�ֹ�θ���˰�ư����
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */

static int getIchiranBangoKouho pro((uiContext));
static IchiranBangoKouho pro((uiContext));

static
IchiranBangoKouho(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;
  int zflag, retval = 0;

  if(ic->tooSmall) {
    d->status = AUX_CALLBACK;
    return(retval);
  }

  /* d->status = EVERYTIME_CALLBACK; */

  if (cannaconf.HexkeySelect && !(ic->inhibit & NUMBERING)) {
    /* ���Ϥ��줿�ֹ�θ���򥫥��ȸ���Ȥ��� */
    if((zflag = getIchiranBangoKouho(d)) == NG)
      goto insert;

    /* SelectDirect �Υ������ޥ����ν��� */
  do_selection:
    if (cannaconf.SelectDirect) /* ON */ {
      if(zflag) /* �������Ϥ��줿 */
	retval = IchiranQuit(d);
      else
	retval = IchiranKakutei(d);
    } else {          /* OFF */
      makeGlineStatus(d);
      /* d->status = EVERYTIME_CALLBACK; */
    }
    return(retval);
  }
  else {
#ifdef CANNA_LIST_Insert /* �����������Ƥ��������ɤ� */
    if (ic->flags & ICHIRAN_ALLOW_CALLBACK && d->list_func) {
      int res = (*d->list_func) /* list_func ��ƤӽФ� */
	(d->client_data, CANNA_LIST_Insert, (wchar_t **)0, d->ch, (int *)0);
      if (res) { /* d->ch �����ץꥱ�������¦�ǽ������줿 */
	if (res == CANNA_FN_FunctionalInsert) {
	  zflag = 1; /* 0 ����ʤ���Ф��� */
	  goto do_selection;
	}
	else if (res != CANNA_FN_Nop) {
	  /* ���ץꥱ�������¦�����׵ᤷ���褿��ǽ��³���Ƽ¹Ԥ��� */
	  d->more.todo = 1;
	  d->more.ch = d->ch;
	  d->more.fnum = CANNA_FN_FunctionalInsert;
	}
	return 0;
      }
      else { /* CANNA_LIST_Insert was not processed at the callback func */
	/* continue to the 'insert:' tag.. */
      }
    }
#endif

  insert:
    if(!(ic->inhibit & CHARINSERT) && cannaconf.allowNextInput) {
      BYTE ifl = ic->flags;
      retval = IchiranKakutei(d);
      if (ifl & ICHIRAN_STAY_LONG) {
	(void)IchiranQuit(d);
      }
      d->more.todo = 1;
      d->more.ch = d->ch;
      d->more.fnum = CANNA_FN_FunctionalInsert;
    } else {
      NothingChangedWithBeep(d);
    }
    return(retval);
  }
}

/*
 * �����������Ϥ��줿�ֹ�θ���˰�ư����
 *
 *
 * ������	uiContext
 * �����	�������Ϥ��줿��              �����֤�
 * 		������������椬���Ϥ��줿��  �����֤�
 * 		���顼���ä���              �������֤�
 */
static int
getIchiranBangoKouho(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;
  int num, kindex;

  /* ���ϥǡ����� ������ ����� ���� */
  if(((0x30 <= d->ch) && (d->ch <= 0x39))
     || ((0x61 <= d->ch) && (d->ch <= 0x66))) {
    if((0x30 <= d->ch) && (d->ch <= 0x39))
      num = (int)(d->ch & 0x0f);
    else if((0x61 <= d->ch) && (d->ch <= 0x66))
      num = (int)(d->ch - 0x57);
  } 
  else {
    /* ���Ϥ��줿�ֹ������������ޤ��� */
    return(NG);
  }
  /* ���ϥǡ����� ����Ԥ����¸�ߤ�������� */
  if(num > ic->glineifp[ic->kouhoifp[*(ic->curIkouho)].khretsu].glkosu) {
    /* ���Ϥ��줿�ֹ������������ޤ��� */
    return(NG);
  }

  /* ���Ϥ��줿�������� SelectDirect �� ON �ʤ��ɤߤ��ᤷ�ƣ����֤� */
  if(num == 0) {
    if (cannaconf.SelectDirect)
      return(1);
    else {
      /* ���Ϥ��줿�ֹ������������ޤ��� */
      return(NG);
    }  
  } else {
    /* ���������Ƭ��������� */
    kindex = ic->glineifp[ic->kouhoifp[*(ic->curIkouho)].khretsu].glhead;
    *(ic->curIkouho) = kindex + (num - 1);
  }

  return(0);
}

/*
 * ������椫�����򤵤줿����򥫥��ȸ���Ȥ���
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */

static IchiranKakutei pro((uiContext));

static
IchiranKakutei(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;
  int retval = 0;
  wchar_t *kakuteiStrings;

  if ((ic->flags & ICHIRAN_ALLOW_CALLBACK) && d->list_func) {
    if (ic->flags & ICHIRAN_STAY_LONG) {
      d->list_func(d->client_data,
                     CANNA_LIST_Query, (wchar_t **)0, 0, (int *)0);
    }
    else {
      d->list_func(d->client_data,
                     CANNA_LIST_Select, (wchar_t **)0, 0, (int *)0);
    }
  }

  kakuteiStrings = ic->allkouho[*(ic->curIkouho)];
  retval = d->nbytes = WStrlen(kakuteiStrings);
  WStrcpy(d->buffer_return, kakuteiStrings);

  if (ic->flags & ICHIRAN_STAY_LONG) {
    ic->flags |= ICHIRAN_NEXT_EXIT; 
    d->status = EVERYTIME_CALLBACK;
  }
  else {
    ichiranFin(d);

    d->status = EXIT_CALLBACK;
  }

  return(retval);
}

/*
 * �����ɽ���⡼�ɤ���ȴ����
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
void
ichiranFin(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec; 

  /* �������ɽ�����ѤΥ��ꥢ��ե꡼���� */
  freeIchiranBuf(ic);

  popIchiranMode(d);

  /* gline �򥯥ꥢ���� */
  GlineClear(d);
}

static IchiranExtendBunsetsu pro((uiContext));

static
IchiranExtendBunsetsu(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_Extend);
}

static IchiranShrinkBunsetsu pro((uiContext));

static
IchiranShrinkBunsetsu(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_Shrink);
}

static IchiranAdjustBunsetsu pro((uiContext));

static
IchiranAdjustBunsetsu(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_AdjustBunsetsu);
}

static IchiranKillToEndOfLine pro((uiContext));

static
IchiranKillToEndOfLine(d)
uiContext d;
{
  return IchiranKakuteiThenDo(d, CANNA_FN_KillToEndOfLine);
}

static IchiranDeleteNext pro((uiContext));

static
IchiranDeleteNext(d)
uiContext d;
{
  return IchiranKakuteiThenDo(d, CANNA_FN_DeleteNext);
}

static IchiranBubunMuhenkan pro((uiContext));

static
IchiranBubunMuhenkan(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_BubunMuhenkan);
}

static IchiranHiragana pro((uiContext));

static
IchiranHiragana(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_Hiragana);
}

static IchiranKatakana pro((uiContext));

static
IchiranKatakana(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_Katakana);
}

static IchiranZenkaku pro((uiContext));

static
IchiranZenkaku(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_Zenkaku);
}

static IchiranHankaku pro((uiContext));

static
IchiranHankaku(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_Hankaku);
}

static IchiranRomaji pro((uiContext));

static
IchiranRomaji(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_Romaji);
}

static IchiranToUpper pro((uiContext));

static
IchiranToUpper(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_ToUpper);
}

static IchiranToLower pro((uiContext));

static
IchiranToLower(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_ToLower);
}

static IchiranCapitalize pro((uiContext));

static
IchiranCapitalize(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_Capitalize);
}

static IchiranKanaRotate pro((uiContext));

static
IchiranKanaRotate(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_KanaRotate);
}

static IchiranRomajiRotate pro((uiContext));

static
IchiranRomajiRotate(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_RomajiRotate);
}

static IchiranCaseRotateForward pro((uiContext));

static
IchiranCaseRotateForward(d)
uiContext d;
{
  return IchiranQuitThenDo(d, CANNA_FN_CaseRotate);
}

#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/

#include	"ichiranmap.h"
