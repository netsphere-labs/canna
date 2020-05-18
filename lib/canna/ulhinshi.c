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
static char rcs_id[] = "@(#) 102.1 $Id: ulhinshi.c,v 1.3 2003/09/17 08:50:53 aida_s Exp $";
#endif

#include <errno.h>
#include "canna.h"

/*********************************************************************
 *                      wchar_t replace begin                        *
 *********************************************************************/
#ifdef wchar_t
# error "wchar_t is already defined"
#endif
#define wchar_t cannawc

#ifndef NO_EXTEND_MENU
#ifdef luna88k
extern int errno;
#endif

static int tourokuYes pro((uiContext)),
           tourokuNo pro((uiContext)),
           makeDoushi pro((uiContext)),
           uuTDicExitCatch pro((uiContext, int, mode_context)),
           uuTDicQuitCatch pro((uiContext, int, mode_context)),
           tangoTouroku pro((uiContext));

static char *e_message[] = {
#ifndef CODED_MESSAGE
  /*0*/"����˺٤����ʻ�ʬ���Τ���μ���򤷤Ƥ��ɤ��Ǥ���?(y/n)",
  /*1*/"�ɤߤȸ���� ���߷������Ϥ��Ƥ���������",
  /*2*/"�ɤߤȸ���� ���Ѥ��㤤�ޤ������Ϥ��ʤ����Ƥ���������",
  /*3*/"�ɤߤȸ���� ���߷������Ϥ��Ƥ�����������) �ᤤ",
  /*4*/"�ɤߤȸ���� ���߷������Ϥ��Ƥ�����������) �Ť���",
  /*5*/"��",
  /*6*/"����פ��������Ǥ���?(y/n)",
  /*7*/"�ʡפ��������Ǥ���?(y/n)",
  /*8*/"�פϿ�̾�Ǥ���?(y/n)",
  /*9*/"�פ���̾�Ǥ���?(y/n)",
  /*10*/"�ʤ��פ��������Ǥ���?(y/n)",
  /*11*/"�פ�̾��Ȥ��ƻȤ��ޤ���?(y/n)",
  /*12*/"�פ��������Ǥ���?(y/n)",
  /*13*/"�ȡפ��������Ǥ���?(y/n)",
#ifdef STANDALONE
  /*14*/"���ʴ����Ѵ��Ǥ��ޤ���",
#else
  /*14*/"���ʴ����Ѵ������Ф��̿��Ǥ��ޤ���",
#endif
  /*15*/"ñ����Ͽ�Ǥ��ޤ���Ǥ���",
  /*16*/"��",
  /*17*/"��",
  /*18*/"��",
  /*19*/"�ˤ���Ͽ���ޤ���",
  /*20*/"ñ����Ͽ�˼��Ԥ��ޤ���",
#else
  /*0*/"\244\265\244\351\244\313\272\331\244\253\244\244\311\312\273\354\312\254\244\261\244\316\244\277\244\341\244\316\274\301\314\344\244\362\244\267\244\306\244\342\316\311\244\244\244\307\244\271\244\253?(y/n)",
       /* ����˺٤����ʻ�ʬ���Τ���μ���򤷤Ƥ��ɤ��Ǥ��� */

  /*1*/"\306\311\244\337\244\310\270\365\312\344\244\362\40\275\252\273\337\267\301\244\307\306\376\316\317\244\267\244\306\244\257\244\300\244\265\244\244\241\243",
       /* �ɤߤȸ���� ���߷������Ϥ��Ƥ���������*/

  /*2*/"\306\311\244\337\244\310\270\365\312\344\244\316\40\263\350\315\321\244\254\260\343\244\244\244\336\244\271\241\243\306\376\316\317\244\267\244\312\244\252\244\267\244\306\244\257\244\300\244\265\244\244\241\243",
       /* �ɤߤȸ���� ���Ѥ��㤤�ޤ������Ϥ��ʤ����Ƥ���������*/

  /*3*/"\306\311\244\337\244\310\270\365\312\344\244\362\40\275\252\273\337\267\301\244\307\306\376\316\317\244\267\244\306\244\257\244\300\244\265\244\244\241\243\316\343) \301\341\244\244",
       /* �ɤߤȸ���� ���߷������Ϥ��Ƥ�����������) �ᤤ */

  /*4*/"\306\311\244\337\244\310\270\365\312\344\244\362\40\275\252\273\337\267\301\244\307\306\376\316\317\244\267\244\306\244\257\244\300\244\265\244\244\241\243\316\343) \300\305\244\253\244\300",
       /* �ɤߤȸ���� ���߷������Ϥ��Ƥ�����������) �Ť��� */

  /*5*/"\241\326",  /* �� */

  /*6*/"\244\271\244\353\241\327\244\317\300\265\244\267\244\244\244\307\244\271\244\253?(y/n)",
       /* ����פ��������Ǥ��� */

  /*7*/"\244\312\241\327\244\317\300\265\244\267\244\244\244\307\244\271\244\253?(y/n)",
       /* �ʡפ��������Ǥ��� */

  /*8*/"\241\327\244\317\277\315\314\276\244\307\244\271\244\253?(y/n)",
       /* �פϿ�̾�Ǥ��� */

  /*9*/"\241\327\244\317\303\317\314\276\244\307\244\271\244\253?(y/n)",
       /* �פ���̾�Ǥ��� */

  /*10*/"\244\312\244\244\241\327\244\317\300\265\244\267\244\244\244\307\244\271\244\253?(y/n)",
       /* �ʤ��פ��������Ǥ��� */

  /*11*/"\241\327\244\317\314\276\273\354\244\310\244\267\244\306\273\310\244\244\244\336\244\271\244\253?(y/n)",
       /* �פ�̾��Ȥ��ƻȤ��ޤ��� */

  /*12*/"\241\327\244\317\300\265\244\267\244\244\244\307\244\271\244\253?(y/n)",
       /* �פ��������Ǥ��� */

  /*13*/"\244\310\241\327\244\317\300\265\244\267\244\244\244\307\244\271\244\253?(y/n)",
       /* �ȡפ��������Ǥ��� */

#ifdef STANDALONE
  /*14*/"\244\253\244\312\264\301\273\372\312\321\264\271\244\307\244\255\244\336\244\273\244\363",
       /* ���ʴ����Ѵ��Ǥ��ޤ��� */
#else
  /*14*/"\244\253\244\312\264\301\273\372\312\321\264\271\245\265\241\274\245\320\244\310\304\314\277\256\244\307\244\255\244\336\244\273\244\363",
       /* ���ʴ����Ѵ������Ф��̿��Ǥ��ޤ��� */
#endif

  /*15*/"\303\261\270\354\305\320\317\277\244\307\244\255\244\336\244\273\244\363\244\307\244\267\244\277",
       /* ñ����Ͽ�Ǥ��ޤ���Ǥ��� */

  /*16*/"\241\330", /* �� */

  /*17*/"\241\331", /* �� */

  /*18*/"\241\312", /* �� */

  /*19*/"\241\313\244\362\305\320\317\277\244\267\244\336\244\267\244\277",
       /* �ˤ���Ͽ���ޤ��� */

  /*20*/"\303\261\270\354\305\320\317\277\244\313\274\272\307\324\244\267\244\336\244\267\244\277",
       /* ñ����Ͽ�˼��Ԥ��ޤ��� */
#endif
};

#define message_num (sizeof(e_message) / sizeof(char *))
static wchar_t *message[message_num];

#ifndef CODED_MESSAGE
static char sgyouA[] = "���������ʤФޤ��";
static char sgyouI[] = "���������ˤӤߤꤤ";
static char sgyouU[] = "�������Ĥ̤֤�뤦";
#else
static char sgyouA[] = "\244\253\244\254\244\265\244\277\244\312\244\320\244\336\244\351\244\357";
                       /* ���������ʤФޤ�� */

static char sgyouI[] = "\244\255\244\256\244\267\244\301\244\313\244\323\244\337\244\352\244\244";
                       /* ���������ˤӤߤꤤ */

static char sgyouU[] = "\244\257\244\260\244\271\244\304\244\314\244\326\244\340\244\353\244\246";
                       /* �������Ĥ̤֤�뤦 */
#endif


#define KAGYOU 0
#define GAGYOU 1
#define SAGYOU 2
#define TAGYOU 3
#define NAGYOU 4
#define BAGYOU 5
#define MAGYOU 6
#define RAGYOU 7
#define WAGYOU 8

static wchar_t *gyouA;
static wchar_t *gyouI;
static wchar_t *gyouU;

/* ���ƤΥ�å�������"unsigned char"����"wchar_t"���Ѵ����� */
int
initHinshiMessage()
{
  int i;

  for(i = 0; i < message_num; i++) {
    message[i] = WString(e_message[i]);
    if(!message[i]) {
      return(-1);
    }
  }
  return 0;
}

/* WSprintf(to_buf, x1, x2, from_buf)
   :WSprintf(to_buf,"x1%sx2",from_buf);
 */
static void
WSprintf(to_buf, x1, x2, from_buf)
wchar_t *to_buf, *x1, *x2, *from_buf;
{
    WStrcpy(to_buf, x1);
    WStrcat(to_buf, from_buf);
    WStrcat(to_buf, x2);
}
#endif /* NO_EXTEND_MENU */

void
EWStrcat(buf, xxxx)
wchar_t *buf;
char *xxxx;
{
  wchar_t x[1024];

  MBstowcs(x, xxxx, 1024);
  WStrcat(buf, x);
}

#ifndef NO_EXTEND_MENU
static void
EWStrcpy(buf, xxxx)
wchar_t *buf;
char *xxxx;
{
  wchar_t x[1024];
  int len;

  len = MBstowcs(x, xxxx, 1024);
  WStrncpy(buf, x, len);
  buf[len] = (wchar_t)0;
}

static int
EWStrcmp(buf, xxxx)
wchar_t *buf;
char *xxxx;
{
  wchar_t x[1024];

  MBstowcs(x, xxxx, 1024);
  return(WStrncmp(buf, x, WStrlen(x)));
}

static int
EWStrncmp(buf, xxxx, len)
wchar_t *buf;
char *xxxx;
int len;
/* ARGSUSED */
{
  wchar_t x[1024];

  MBstowcs(x, xxxx, 1024);
  return(WStrncmp(buf, x, WStrlen(x)));
}

int
initGyouTable()
{
  gyouA = WString(sgyouA);
  gyouI = WString(sgyouI);
  gyouU = WString(sgyouU);

  if (!gyouA || !gyouI || !gyouU) {
    return NG;
  }
  return 0;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ñ����Ͽ���ʻ����� ��Yes/No ���� Quit��                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
uuTHinshiYNQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d);
  
  return(dicTourokuHinshi(d));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ñ����Ͽ���ʻ����� ��Yes/No �裲�ʳ� ���̥�����Хå���                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
uuTHinshi2YesCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  tourokuContext tc;

  popCallback(d); /* yesNo ��ݥå� */

  tourokuYes(d);   /* �ʻ줬��ޤ�� tc->hcode �˥��åȤ��� */

  tc = (tourokuContext)d->modec;

  if (!tc->qbuf[0]) {
    if (tc->hcode[0]) {
      /* �ʻ줬��ޤä��Τǡ���Ͽ����桼������λ����Ԥ� */
      return(dicTourokuDictionary(d, uuTDicExitCatch, uuTDicQuitCatch));
    }
  }
  return(retval);
}

static
uuTHinshi2NoCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  tourokuContext tc;

  popCallback(d); /* yesNo ��ݥå� */

  tourokuNo(d);   /* �ʻ줬��ޤ�� tc->hcode �˥��åȤ��� */

  tc = (tourokuContext)d->modec;

  if (!tc->qbuf[0]) {
    if (tc->hcode[0]) {
      /* �ʻ줬��ޤä��Τǡ���Ͽ����桼������λ����Ԥ� */
      return(dicTourokuDictionary(d, uuTDicExitCatch, uuTDicQuitCatch));
    }
  }

  return(retval);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ñ����Ͽ���ʻ����� ��Yes/No �裱�ʳ� ������Хå���                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
uuTHinshi1YesCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  tourokuContext tc;
  coreContext ync;
  
  popCallback(d); /* yesNo ��ݥå� */

  tourokuYes(d);   /* �ʻ줬��ޤ�� tc->hcode �˥��åȤ��� */

  tc = (tourokuContext)d->modec;

  if(tc->qbuf[0]) {
    /* ���䤹�� */
    makeGLineMessage(d, tc->qbuf, WStrlen(tc->qbuf));
    if((retval = getYesNoContext(d,
		 NO_CALLBACK, uuTHinshi2YesCatch,
		 uuTHinshiYNQuitCatch, uuTHinshi2NoCatch)) == NG) {
      defineEnd(d);
      return(GLineNGReturnTK(d));
    }
    ync = (coreContext)d->modec;
    ync->majorMode = CANNA_MODE_ExtendMode;
    ync->minorMode = CANNA_MODE_TourokuHinshiMode;
  } else if(tc->hcode[0]) {
    /* �ʻ줬��ޤä��Τǡ���Ͽ����桼������λ����Ԥ� */
    return(dicTourokuDictionary(d, uuTDicExitCatch, uuTDicQuitCatch));
  }

  return(retval);
}

static
uuTHinshi1NoCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  tourokuContext tc;
  coreContext ync;

  popCallback(d); /* yesNo ��ݥå� */

  tourokuNo(d);   /* �ʻ줬��ޤ�� tc->hcode �˥��åȤ��� */

  tc = (tourokuContext)d->modec;

  if(tc->qbuf[0]) {
    /* ���䤹�� */
    makeGLineMessage(d, tc->qbuf, WStrlen(tc->qbuf));
    if((retval = getYesNoContext(d,
		 NO_CALLBACK, uuTHinshi2YesCatch,
		 uuTHinshiYNQuitCatch, uuTHinshi2NoCatch)) == NG) {
      defineEnd(d); 
      return(GLineNGReturnTK(d));
    }
    ync = (coreContext)d->modec;
    ync->majorMode = CANNA_MODE_ExtendMode;
    ync->minorMode = CANNA_MODE_TourokuHinshiMode;
  } else if(tc->hcode[0]) {
    /* �ʻ줬��ޤä��Τǡ���Ͽ����桼������λ����Ԥ� */
    return(dicTourokuDictionary(d, uuTDicExitCatch, uuTDicQuitCatch));
  }

  return(retval);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ñ����Ͽ���ʻ�ʬ�����롩                                                  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
uuTHinshiQYesCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  tourokuContext tc;
  coreContext ync;

  popCallback(d); /* yesNo ��ݥå� */

  tc = (tourokuContext)d->modec;

  makeGLineMessage(d, tc->qbuf, WStrlen(tc->qbuf)); /* ���� */
  if((retval = getYesNoContext(d,
	 NO_CALLBACK, uuTHinshi1YesCatch,
	 uuTHinshiYNQuitCatch, uuTHinshi1NoCatch)) == NG) {
    defineEnd(d);
    return(GLineNGReturnTK(d));
  }
  ync = (coreContext)d->modec;
  ync->majorMode = CANNA_MODE_ExtendMode;
  ync->minorMode = CANNA_MODE_TourokuHinshiMode;

  return(retval);
}

static
uuTHinshiQNoCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* yesNo ��ݥå� */

  return(dicTourokuDictionary(d, uuTDicExitCatch, uuTDicQuitCatch));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ñ����Ͽ���ʻ�����                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int makeHinshi();

dicTourokuHinshiDelivery(d)
uiContext	d;
{
  tourokuContext tc = (tourokuContext)d->modec;
  coreContext ync;
  int retval = 0;

  makeHinshi(d); /* �ʻ졢���顼��å�����������򥻥åȤ��Ƥ��� */

#if defined(DEBUG)
  if(iroha_debug) {
    printf("tc->genbuf=%s, tc->qbuf=%s, tc->hcode=%s\n", tc->genbuf, tc->qbuf,
	   tc->hcode);
  }
#endif
  if(tc->genbuf[0]) {
    /* ���Ϥ��줿�ǡ����˸�꤬���ä��Τǡ�
       ��å�������ɽ�������ɤ����Ϥ���� */
    clearYomi(d);
    return(dicTourokuTango(d, uuTTangoQuitCatch));
  } else if(tc->qbuf[0] && cannaconf.grammaticalQuestion) {
    /* �٤����ʻ�ʬ���Τ���μ���򤹤� */
    WStrcpy(d->genbuf, message[0]);
    if((retval = getYesNoContext(d,
		 NO_CALLBACK, uuTHinshiQYesCatch,
		 uuTHinshiYNQuitCatch, uuTHinshiQNoCatch)) == NG) {
      defineEnd(d);
      return(GLineNGReturnTK(d));
    }
    makeGLineMessage(d, d->genbuf, WStrlen(d->genbuf));
    ync = (coreContext)d->modec;
    ync->majorMode = CANNA_MODE_ExtendMode;
    ync->minorMode = CANNA_MODE_TourokuHinshiMode;
    return(retval);
  } else if(tc->hcode[0]) {
    /* �ʻ줬��ޤä��Τǡ���Ͽ����桼������λ����Ԥ� */
    return(dicTourokuDictionary(d, uuTDicExitCatch, uuTDicQuitCatch));
  }
  return 0;
}

/*
 * ���򤵤줿�ʻ줫�鼡��ư���Ԥ�
 * 
 * tc->hcode	�ʻ�
 * tc->qbuf	����
 * tc->genbuf	���顼
 */
static int
makeHinshi(d)
uiContext	d;
{
  tourokuContext tc = (tourokuContext)d->modec;
  int tlen, ylen, yomi_katsuyou;
  wchar_t tmpbuf[256];

  tc->hcode[0] = 0;
  tc->qbuf[0] = 0;
  tc->genbuf[0] = 0;

  tlen = tc->tango_len;
  ylen = tc->yomi_len;

  switch(tc->curHinshi) {
  case MEISHI:
    EWStrcpy(tc->hcode, "#T35");
    tc->katsuyou = 0;
    WSprintf(tc->qbuf, message[5], message[6], tc->tango_buffer);
    break;

  case KOYUMEISHI:
    EWStrcpy(tc->hcode, "#KK");
    WSprintf(tc->qbuf, message[5], message[8], tc->tango_buffer);
    break;
    
  case DOSHI:

    /* ���Ϥ����߷����� */
    tc->katsuyou = 0;
    while (tc->katsuyou < GOBISUU &&
	   tc->tango_buffer[tlen - 1] != gyouU[tc->katsuyou]) {
      tc->katsuyou++;
    }
    yomi_katsuyou = 0;
    while (yomi_katsuyou < GOBISUU &&
	   tc->yomi_buffer[ylen - 1] != gyouU[yomi_katsuyou]) {
      yomi_katsuyou++;
    }
    if((tc->katsuyou == GOBISUU) || (yomi_katsuyou == GOBISUU)){
      WStrcpy(tc->genbuf, message[1]);
      return(0);
    }
    if(tc->katsuyou != yomi_katsuyou){
      WStrcpy(tc->genbuf, message[2]);
      return(0);
    }

    makeDoushi(d);  /* �ܺ٤��ʻ��ɬ�פȤ��ʤ���� */
    if (tc->katsuyou == RAGYOU) {
      tc->curHinshi = RAGYODOSHI;
      /* ̤������Ĥ��� */
      WStrncpy(tmpbuf, tc->tango_buffer, tlen-1);  
      tmpbuf[tlen - 1] = gyouA[tc->katsuyou];
      tmpbuf[tlen] = (wchar_t)0;
      WSprintf(tc->qbuf, message[5], message[10], tmpbuf);
    }
    else {
      tc->curHinshi = GODAN;
      WStrncpy(tmpbuf, tc->tango_buffer, tlen - 1);
      tmpbuf[tlen - 1] = gyouI[tc->katsuyou];
      tmpbuf[tlen] = (wchar_t)'\0';
      WSprintf(tc->qbuf, message[5], message[11], tmpbuf);
    }
    break;

  case KEIYOSHI:
    tc->katsuyou = 1;
    if(tlen >= 1 && ylen >= 1 &&
       ((EWStrncmp(tc->tango_buffer+tlen-1, "\244\244", 1) != 0) ||
	(EWStrncmp(tc->yomi_buffer+ylen-1, "\244\244", 1) != 0))) {
                                           /* �� */
      WStrcpy(tc->genbuf, message[3]);
      return(0);
    }

    EWStrcpy(tc->hcode, "#KY"); /* �ܺ٤��ʻ��ɬ�פȤ��ʤ���� */
    WStrncpy(tmpbuf, tc->tango_buffer, tlen-1);  
    tmpbuf[tlen-1] = 0;
    WSprintf(tc->qbuf, message[5], message[11], tmpbuf);
    break;

  case KEIYODOSHI:
    tc->katsuyou = 1;
    if(tlen >= 1 && ylen >= 1 &&
       ((EWStrncmp(tc->tango_buffer+tlen-1, "\244\300", 1)) ||
	(EWStrncmp(tc->yomi_buffer+ylen-1, "\244\300", 1)))) {
                                           /* �� */
      WStrcpy(tc->genbuf, message[4]);
      return(0);
    }
    EWStrcpy(tc->hcode, "#T05"); /* �ܺ٤��ʻ��ɬ�פȤ��ʤ���� */
    WStrncpy(tmpbuf, tc->tango_buffer, tlen-1);  
    tmpbuf[tlen-1] = 0;  
    WSprintf(tc->qbuf, message[5], message[6], tmpbuf);
    break;

  case FUKUSHI:
    EWStrcpy(tc->hcode, "#F14"); /* �ܺ٤��ʻ��ɬ�פȤ��ʤ���� */
    tc->katsuyou = 0;
    WSprintf(tc->qbuf, message[5], message[6], tc->tango_buffer);
    break;

  case TANKANJI:
    EWStrcpy(tc->hcode, "#KJ");
    break;

  case SUSHI:
    EWStrcpy(tc->hcode, "#NN");
    break;

  case RENTAISHI:
    EWStrcpy(tc->hcode, "#RT");
    break;

  case SETSUZOKUSHI:  /* ��³�졦��ư�� */
    EWStrcpy(tc->hcode, "#CJ");
    break;

  case SAHENMEISHI:
  case MEISHIN:
    tc->katsuyou = 0;
    WSprintf(tc->qbuf, message[5], message[7], tc->tango_buffer);
    break;

  case JINMEI:
  case KOYUMEISHIN:
    WSprintf(tc->qbuf, message[5], message[9], tc->tango_buffer);
    break;

  case RAGYOGODAN:
    WStrncpy(tmpbuf, tc->tango_buffer, tlen - 1);
    tmpbuf[tlen - 1] = gyouI[tc->katsuyou];
    tmpbuf[tlen] = (wchar_t)'\0';
    WSprintf(tc->qbuf, message[5], message[11], tmpbuf);
    break;

  case KAMISHIMO:
    WStrncpy(tmpbuf, tc->tango_buffer, tlen - 1);
    tmpbuf[tlen - 1] = (wchar_t)'\0';
    WSprintf(tc->qbuf, message[5], message[11], tmpbuf);
    break;

  case KEIYODOSHIY:
  case KEIYODOSHIN: 
    WStrncpy(tmpbuf, tc->tango_buffer, tlen - 1);
    tmpbuf[tlen - 1] = 0;
    WSprintf(tc->qbuf, message[5], message[11], tmpbuf);
    break;

  case FUKUSHIY:
  case FUKUSHIN:
    WSprintf(tc->qbuf, message[5], message[13], tc->tango_buffer);
    break;
  }

  return(0);
}

static
tourokuYes(d)
uiContext	d;
{
  tourokuContext tc = (tourokuContext)d->modec;

  tc->hcode[0] = 0;
  tc->qbuf[0] = 0;
  tc->genbuf[0] = 0;

  switch(tc->curHinshi) {
  case MEISHI:
    tc->curHinshi = SAHENMEISHI;
    makeHinshi(d);
    break;

  case KOYUMEISHI:
    tc->curHinshi = JINMEI;
    makeHinshi(d);
    break;

  case GODAN:  /* ��԰ʳ��θ��ʳ���ư�� */
    makeDoushi(d);
    EWStrcat(tc->hcode, "r");              /* �񤯡��ޤ����ܤ� */
    break;

  case RAGYODOSHI:
    tc->curHinshi = RAGYOGODAN;
    makeHinshi(d);
    break;

  case KEIYOSHI:
    EWStrcpy(tc->hcode, "#KYT");           /* ������ */
    break;

  case KEIYODOSHI:
    tc->curHinshi = KEIYODOSHIY;
    makeHinshi(d);
    break;

  case FUKUSHI:
    tc->curHinshi = FUKUSHIY;
    makeHinshi(d);
    break;

  case MEISHIN:
    EWStrcpy(tc->hcode, "#T15");          /* ���������� */
    break;

  case SAHENMEISHI:
    EWStrcpy(tc->hcode, "#T10");          /* �¿����ⵤ */
    break;

  case KOYUMEISHIN:
    EWStrcpy(tc->hcode, "#CN");	          /* ��� */
    break;

  case JINMEI:
    EWStrcpy(tc->hcode, "#JCN");          /* ʡ�� */
    break;

  case RAGYOGODAN:
    EWStrcpy(tc->hcode, "#R5r");          /* �դ� */
    break;

  case KAMISHIMO:
    EWStrcpy(tc->hcode, "#KSr");          /* �����롢�¤��� */
    break;

  case KEIYODOSHIY:
    EWStrcpy(tc->hcode, "#T10");          /* �ؿ��� */
    break;

  case KEIYODOSHIN:
    EWStrcpy(tc->hcode, "#T15");          /* �ճ�������ǽ�� */
    break;

  case FUKUSHIY:
    EWStrcpy(tc->hcode, "#F04");          /* �դä��� */
    break;

  case FUKUSHIN:
    EWStrcpy(tc->hcode, "#F06");          /* ���� */
    break;
  }

  return(0);
}

static
tourokuNo(d)
uiContext	d;
{
  tourokuContext tc = (tourokuContext)d->modec;
  int ylen;

  tc->hcode[0] = 0;
  tc->qbuf[0] = 0;
  tc->genbuf[0] = 0;

  switch( tc->curHinshi ) {
  case MEISHI:
    tc->curHinshi = MEISHIN;
    makeHinshi(d);
    break;

  case KOYUMEISHI:
    tc->curHinshi = KOYUMEISHIN;
    makeHinshi(d);
    break;

  case GODAN:  /* ��԰ʳ��θ��ʳ���ư�� */
    makeDoushi(d);
    break;

  case RAGYODOSHI:
    ylen = tc->yomi_len;
    if (ylen >= 2 && !(EWStrcmp(tc->yomi_buffer + ylen - 2, "\244\257\244\353"))) {   /* ���� */
      EWStrcpy(tc->hcode, "#KX");         /* ��� */
    }
    else if (ylen >=2 && !(EWStrcmp(tc->yomi_buffer + ylen - 2, "\244\271\244\353"))) { /* ���� */
      EWStrcpy(tc->hcode, "#SX");         /* ���� */
    }
    else if (ylen >=2 && !(EWStrcmp(tc->yomi_buffer + ylen - 2, "\244\272\244\353"))) {  /* ���� */
      EWStrcpy(tc->hcode, "#ZX");         /* �ऺ�� */
    }
    else {
      tc->curHinshi = KAMISHIMO;
      makeHinshi(d);
    }
    break;

  case KEIYOSHI:
    EWStrcpy(tc->hcode, "#KY");           /* ���������ᤤ */
    break;

  case KEIYODOSHI:
    tc->curHinshi = KEIYODOSHIN;
    makeHinshi(d);
    break;

  case FUKUSHI:
    tc->curHinshi = FUKUSHIN;
    makeHinshi(d);
    break;

  case MEISHIN:
    EWStrcpy(tc->hcode, "#T35");          /* ������ */
    break;

  case SAHENMEISHI:
    EWStrcpy(tc->hcode, "#T30");          /* ���ϡ����� */
    break;

  case KOYUMEISHIN:
    EWStrcpy(tc->hcode, "#KK");           /* �����ŵ� */
    break;

  case JINMEI:
    EWStrcpy(tc->hcode, "#JN");           /* ���� */
    break;

  case RAGYOGODAN:
    EWStrcpy(tc->hcode, "#R5");           /* ��ĥ�� */
    break;

  case KAMISHIMO:
    EWStrcpy(tc->hcode, "#KS");           /* �ߤ�롢Ϳ���� */
    break;

  case KEIYODOSHIY:
    EWStrcpy(tc->hcode, "#T13");          /* ¿���Ƥ� */
    break;

  case KEIYODOSHIN:
    EWStrcpy(tc->hcode, "#T18");          /* ���������Ť��� */
    break;

  case FUKUSHIY:
    EWStrcpy(tc->hcode, "#F12");          /* ���ä� */
    break;

  case FUKUSHIN:
    EWStrcpy(tc->hcode, "#F14");          /* ˰���ޤ� */
    break;
  }
  return(0);
}

static
makeDoushi(d)
uiContext	d;
{
  tourokuContext tc = (tourokuContext)d->modec;

    switch(tc->katsuyou){
    case  KAGYOU:
      EWStrcpy( tc->hcode, "#K5" );     /* �֤� */
      break;
    case  GAGYOU:
      EWStrcpy( tc->hcode, "#G5" );     /* �Ĥ� */
      break;
    case  SAGYOU:
      EWStrcpy( tc->hcode, "#S5" );     /* �֤� */
      break;
    case  TAGYOU:
      EWStrcpy( tc->hcode, "#T5" );     /* ��� */
      break;
    case  NAGYOU:
      EWStrcpy( tc->hcode, "#N5" );     /* ��� */
      break;
    case  BAGYOU:
      EWStrcpy( tc->hcode, "#B5" );     /* ž�� */
      break;
    case  MAGYOU:
      EWStrcpy( tc->hcode, "#M5" );     /* ���� */
      break;
    case  RAGYOU:
      EWStrcpy( tc->hcode, "#R5" );     /* ��ĥ�� */
      break;
    case  WAGYOU:
      EWStrcpy( tc->hcode, "#W5" );     /* ���� */
      break;
    }
}    

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ����ΰ���                                                                *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
uuTDicExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  forichiranContext fc;
  int cur;
  tourokuContext tc;

  d->nbytes = 0;

  popCallback(d); /* ������ pop */

  fc = (forichiranContext)d->modec;
  cur = fc->curIkouho;

  popForIchiranMode(d);
  popCallback(d);

  tc = (tourokuContext)d->modec;

  tc->workDic = cur;

  return(tangoTouroku(d));
}

static
uuTDicQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* ������ pop */

  popForIchiranMode(d);
  popCallback(d);

  return(dicTourokuHinshi(d));
}

dicTourokuDictionary(d, exitfunc, quitfunc)
uiContext d;
int (*exitfunc)();
int (*quitfunc)();
{
  tourokuContext tc = (tourokuContext)d->modec;
  forichiranContext fc;
  ichiranContext ic;
  wchar_t **work;
  unsigned inhibit = 0;
  int retval, upnelem = 0;

  retval = d->nbytes = 0;
  d->status = 0;

  for(work = tc->udic; *work; work++)
    upnelem++;

  if((retval = getForIchiranContext(d)) == NG) {
    freeDic(tc);
    defineEnd(d);
    return(GLineNGReturnTK(d));
  }
  fc = (forichiranContext)d->modec;

  /* selectOne ��Ƥ֤���ν��� */

  fc->allkouho = tc->udic;

  fc->curIkouho = 0;
  if (!cannaconf.HexkeySelect)
    inhibit |= ((unsigned char)NUMBERING | (unsigned char)CHARINSERT); 
  else
    inhibit |= (unsigned char)CHARINSERT;

   if((retval = selectOne(d, fc->allkouho, &fc->curIkouho, upnelem,
		 BANGOMAX, inhibit, 0, WITHOUT_LIST_CALLBACK,
		 NO_CALLBACK, exitfunc, quitfunc, uiUtilIchiranTooSmall)) 
                 == NG) {
    if(fc->allkouho)
      free(fc->allkouho);
    popForIchiranMode(d);
    popCallback(d);
    defineEnd(d);
    return(GLineNGReturnTK(d));
  }

  ic = (ichiranContext)d->modec;
  ic->majorMode = CANNA_MODE_ExtendMode;
  ic->minorMode = CANNA_MODE_TourokuDicMode;
  currentModeInfo(d);

  /* ��������Ԥ������Ƹ���������Ф��ʤ� */
  if(ic->tooSmall) {
    d->status = AUX_CALLBACK;
    return(retval);
  }

  makeGlineStatus(d);
  /* d->status = ICHIRAN_EVERYTIME; */

  return(retval);
}

/*
 * ñ����Ͽ��Ԥ�
 */
static
tangoTouroku(d)
uiContext	d;
{
  tourokuContext tc = (tourokuContext)d->modec;
  wchar_t ktmpbuf[256];
  wchar_t ttmpbuf[256];
  wchar_t line[ROMEBUFSIZE], line2[ROMEBUFSIZE];
  wchar_t xxxx[1024];
  char dicname[1024];
  extern int defaultContext;
  int linecnt;
  wchar_t *WStraddbcpy();

  defineEnd(d);
  if(tc->katsuyou || (EWStrncmp(tc->hcode, "#K5", 3) == 0)) {
    WStrncpy(ttmpbuf, tc->tango_buffer, tc->tango_len - 1);
    ttmpbuf[tc->tango_len - 1] = (wchar_t)0;
    WStrncpy(ktmpbuf, tc->yomi_buffer, tc->yomi_len - 1);
    ktmpbuf[tc->yomi_len - 1] = 0;
  } else {
    WStrcpy(ttmpbuf, tc->tango_buffer);
    WStrcpy(ktmpbuf, tc->yomi_buffer);
  }

  /* ����񤭹����Ѥΰ�Ԥ��� */
  WStraddbcpy(line, ktmpbuf, ROMEBUFSIZE);
  linecnt = WStrlen(line);
  line[linecnt] = (wchar_t)' ';
  linecnt++;
  WStrcpy(line + linecnt, tc->hcode);
  linecnt += WStrlen(tc->hcode);
  line[linecnt] = (wchar_t)' ';
  linecnt++;
  WStraddbcpy(line + linecnt, ttmpbuf, ROMEBUFSIZE - linecnt);

  if(defaultContext == -1) {
    if((KanjiInit() < 0) || (defaultContext == -1)) {
      jrKanjiError = (char *)e_message[14];
      freeAndPopTouroku(d);
      return(GLineNGReturn(d));
    }
  }
  /* �������Ͽ���� */
  WCstombs(dicname, tc->udic[tc->workDic], sizeof(dicname));

  if (RkwDefineDic(defaultContext, dicname, line) != 0) {
    /* �ʻ줬 #JCN �ΤȤ��ϡ���Ͽ�˼��Ԥ����顢#JN �� #CN ����Ͽ���� */
    if (EWStrncmp(tc->hcode, "#JCN", 4) == 0) {
      wchar_t xxx[3];

      /* �ޤ� #JN ����Ͽ���� */
      EWStrcpy(xxx, "#JN");
      WStraddbcpy(line, ktmpbuf, ROMEBUFSIZE);
      EWStrcat(line, " ");
      WStrcat(line, xxx);
      EWStrcat(line, " ");
      linecnt = WStrlen(line);
      WStraddbcpy(line + linecnt, ttmpbuf, ROMEBUFSIZE - linecnt);

      if (RkwDefineDic(defaultContext, dicname, line) == 0) {
        /* #JN ����Ͽ�Ǥ����Ȥ������� #CN ����Ͽ���� */
        EWStrcpy(xxx, "#CN");
        WStraddbcpy(line2, ktmpbuf, ROMEBUFSIZE);
        EWStrcat(line2, " ");
        WStrcat(line2, xxx);
        EWStrcat(line2, " ");
        linecnt = WStrlen(line2);
        WStraddbcpy(line2 + linecnt, ttmpbuf, ROMEBUFSIZE - linecnt);

        if (RkwDefineDic(defaultContext, dicname, line2) == 0) {
          goto success;
        }

        /* #CN ����Ͽ�Ǥ��ʤ��ä��Ȥ���#JN �������� */
        if (RkwDeleteDic(defaultContext, dicname, line) == NG) {
          /* #JN ������Ǥ��ʤ��ä��顢"���Ԥ��ޤ���" */
          if (errno == EPIPE)
            jrKanjiPipeError();
          WStrcpy(d->genbuf, message[20]);
          goto close;
        }
      }
    }
    /* #JCN �ʳ��ΤȤ�
       #JN ����Ͽ�Ǥ��ʤ��ä��Ȥ�
       #CN ����Ͽ�Ǥ�����#JN ������Ǥ����Ȥ� */
    if (errno == EPIPE)
      jrKanjiPipeError();
    WStrcpy(d->genbuf, message[15]);
    goto close;
  }

 success:
  if (cannaconf.auto_sync) {
    RkwSync(defaultContext, dicname);
  }
  /* ��Ͽ�δ�λ��ɽ������ */
  WSprintf(d->genbuf, message[16], message[17], tc->tango_buffer);
  WSprintf(xxxx, message[18], message[19], tc->yomi_buffer);
  WStrcat(d->genbuf, xxxx);

 close:
  makeGLineMessage(d, d->genbuf, WStrlen(d->genbuf));

  freeAndPopTouroku(d);
  currentModeInfo(d);

  return(0); /* ñ����Ͽ��λ */
}
#endif /* NO_EXTEND_MENU */

#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/
