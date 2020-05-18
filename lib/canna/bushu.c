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
static char rcs_id[] = "@(#) 102.1 $Id: bushu.c,v 1.3 2003/09/17 08:50:53 aida_s Exp $";
#endif /* lint */

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

extern wchar_t *WString();

extern int uuslQuitCatch();
extern int uuslIchiranQuitCatch();
static int bushuHenkan(), makeBushuIchiranQuit();
static int vBushuExitCatch(), bushuQuitCatch();


#define	BUSHU_SZ	150

static
char *bushu_schar[] = 
{ 
  /* "��", "Ш", "��", "��", "��", "��", */
  "\260\354", "\320\250", "\321\341", "\275\275", "\322\307", "\305\341",
  
  /* "���ʤ�äȤ���", "��", "��", "ұ", "��ҹ��", "е", */
  "\264\242\241\312\244\352\244\303\244\310\244\246\241\313", "\316\317", "\322\314", "\322\261", "\321\304\322\271\323\370", "\320\265",
  
  /* "��", "�͡��Ρʤˤ�٤��", "��", "��", "Ȭ", "ѹ", */
  "\321\322", "\277\315\241\277\277\316\241\312\244\313\244\363\244\331\244\363\241\313", "\313\364", "\321\334", "\310\254", "\321\271",
  
  /* "��", "��", "׮", "��ʤ�������)", "��", "��", */
  "\321\314", "\325\337", "\327\256", "\260\352\241\312\244\252\244\252\244\266\244\310\51", "\270\312", "\275\367",
  
  /* "��", "��", "��ʤ���������)", "�ȡʤ���Τؤ��", */
  "\327\306", "\270\375", "\301\360\241\312\244\257\244\265\244\253\244\363\244\340\244\352\51", "\306\310\241\312\244\261\244\342\244\316\244\330\244\363\241\313",

  /* "��", "���ʤ����ȡ�", "��", "���ʤ��󤺤���", "׵", */
  "\273\322", "\357\372\241\312\244\263\244\266\244\310\241\313", "\273\316", "\271\276\241\312\244\265\244\363\244\272\244\244\241\313", "\327\265",
  
  /* "��", "����ñ�ʤġ�", "��ʤ���ˤ礦��", "��", "��", */
  "\325\371", "\276\256\241\277\303\261\241\312\244\304\241\313", "\355\350\241\312\244\267\244\363\244\313\244\347\244\246\241\313", "\300\243", "\302\347",
  
  /* "��", "��ʤƤؤ��", "��", "��", "��", "ͼ", */
  "\305\332", "\274\352\241\312\244\306\244\330\244\363\241\313", "\266\322", "\326\370", "\273\263", "\315\274",
  
  /* "��", "˻�ʤ�ä���٤��", "��", "��", "��", */
  "\265\335", "\313\273\241\312\244\352\244\303\244\267\244\363\244\331\244\363\241\313", "\267\347", "\335\306", "\270\244",
  
  /* "�����ʤ����ؤ��", "��", "��", "��", "��", "��", */
  "\265\355\241\277\262\264\241\312\244\246\244\267\244\330\244\363\241\313", "\312\322", "\314\332", "\335\343", "\314\323", "\277\264",
  
  /* "��", "��", "��", "��", "ھ", "��", */
  "\277\345", "\267\356", "\304\336", "\306\374", "\332\276", "\262\320",
  
  /* "��", "��", "���ʤ�ä���", "��", "��", "��", */
  "\312\375", "\330\371", "\305\300\241\312\244\354\244\303\244\253\241\313", "\335\325", "\267\352", "\300\320",

  /* "��", "��", "��", "��", "��", "���ʤ��᤹�ؤ��", "��", */
  "\266\314", "\310\351", "\264\244", "\273\256", "\274\250", "\277\300\241\312\244\267\244\341\244\271\244\330\244\363\241\313", "\307\362",
  
  /* "��", "Ω", "��", "��", "�", "��", */
  "\305\304", "\316\251", "\262\323", "\314\334", "\342\242", "\314\360",
  
  /* "�ˡʤ�ޤ������", "��", "��", "��", "��", "Ϸ", */
  "\341\313\241\312\244\344\244\336\244\244\244\300\244\354\241\313", "\273\315", "\273\345", "\261\261", "\261\273", "\317\267",
  
  /* "��", "��", "��ʤ����ؤ��", "��", "��", "��", */
  "\264\314", "\260\341", "\275\351\241\312\244\263\244\355\244\342\244\330\244\363\241\313", "\312\306", "\300\345", "\346\320",
  
  /* "�ݡʤ����������", "��", "�סʤȤ餫�����", "��", */
  "\303\335\241\312\244\277\244\261\244\253\244\363\244\340\244\352\241\313", "\267\354", "\270\327\241\312\244\310\244\351\244\253\244\363\244\340\244\352\241\313", "\306\371",
  
  /* "��", "��", "��", "��", "��", "��", */
  "\300\276", "\261\251", "\315\323", "\346\346", "\275\256", "\274\252",
  
  /* "��", "��", "­��ɥ", "�", "��", */
  "\303\356", "\300\326", "\302\255\241\277\311\245", "\354\265", "\277\303",
  
  /* "��", "��", "��", "��", "��", "��", "��", "ë", */
  "\263\255", "\277\311", "\274\326", "\270\253", "\270\300", "\306\323", "\301\366", "\303\253",
  
  /* "��", "��", "��", "Ʀ", "��", "�", "��", "��", */
  "\263\321", "\310\320", "\307\376", "\306\246", "\277\310", "\354\270", "\261\253", "\310\363",
  
  /* "��", "��", "�", "��", "��", "��", "��", "��", */
  "\266\342", "\314\347", "\360\262", "\312\307", "\262\273", "\271\341", "\263\327", "\311\367",
  
  /* "��", "��", "��", "��", "��", "��", "��", "��", */
  "\274\363", "\277\251", "\360\352", "\314\314", "\307\317", "\265\264", "\361\365", "\271\342",
  
  /* "�", "��", "��", "��", "Ļ", "��", "��", "ɡ", */
  "\362\250", "\271\374", "\265\373", "\265\265", "\304\273", "\271\365", "\274\257", "\311\241",

  /* "��", "����", "����¾" */
  "\363\357", "\265\255\271\346", "\244\275\244\316\302\276"
};

static
char *bushu_skey[] =  
{ 
/* "����", "��", "�����Ф�", "���夦", "�դ�", "������", */
"\244\244\244\301", "\244\316", "\244\246\244\261\244\320\244\263", "\244\270\244\345\244\246", "\244\325\244\267", "\244\253\244\277\244\312",

/* "��äȤ�", "��", "����", "��", "���ޤ�", "�ʤ�", "��", */
"\244\352\244\303\244\310\244\246", "\244\253", "\244\254\244\363", "\244\257", "\244\253\244\336\244\250", "\244\312\244\331", "\244\313",

/* "�Ҥ�", "��", "�Ĥ���", "�Ϥ�", "��", "��", */
"\244\322\244\310", "\244\314", "\244\304\244\257\244\250", "\244\317\244\301", "\244\353", "\244\357",

/* "��", "����", "��������", "���Τ�", "�����", "���礦", */
"\244\246", "\244\250\244\363", "\244\252\244\252\244\266\244\310", "\244\252\244\316\244\354", "\244\252\244\363\244\312", "\244\256\244\347\244\246",

/* "��", "����", "�����", "��", "������", "����餤", */
"\244\355", "\244\257\244\265", "\244\261\244\342\244\316", "\244\263", "\244\263\244\266\244\310", "\244\265\244\340\244\351\244\244",

/* "��", "����", "���㤯", "��", "����", "����", */
"\244\267", "\244\267\244\255", "\244\267\244\343\244\257", "\244\304", "\244\267\244\363", "\244\271\244\363",

/* "����", "��", "��", "�Ϥ�", "��", "���", */
"\244\300\244\244", "\244\311", "\244\306", "\244\317\244\320", "\244\336", "\244\344\244\336",

/* "�椦", "���", "��ä���", "����", "������", "����", */
"\244\346\244\246", "\244\346\244\337", "\244\352\244\303\244\267\244\363", "\244\261\244\304", "\244\244\244\301\244\277", "\244\244\244\314",

/* "����", "����", "��", "�����ޤ�", "��", "������", */
"\244\246\244\267", "\244\253\244\277", "\244\255", "\244\255\244\254\244\336\244\250", "\244\261", "\244\263\244\263\244\355",

/* "����", "�Ĥ�", "�Ĥ�", "�ˤ�", "�Τ֤�", "��", */
"\244\271\244\244", "\244\304\244\255", "\244\304\244\341", "\244\313\244\301", "\244\316\244\326\244\363", "\244\322",

/* "�ۤ�", "�ۤ�", "��ĤƤ�", "��ޤ�", "����", "����", */
"\244\333\244\246", "\244\333\244\263", "\244\350\244\304\244\306\244\363", "\244\353\244\336\244\277", "\244\242\244\312", "\244\244\244\267",

/* "����", "����", "�����", "����", "���᤹", "��", */
"\244\252\244\246", "\244\253\244\357", "\244\253\244\357\244\351", "\244\265\244\351", "\244\267\244\341\244\271", "\244\315",

/* "����", "��", "����", "�Τ�", "��", "�Ϥ�", "��", */
"\244\267\244\355", "\244\277", "\244\277\244\304", "\244\316\244\256", "\244\341", "\244\317\244\304", "\244\344",

/* "��ޤ�", "���", "����", "����", "����", "����", */
"\244\344\244\336\244\244", "\244\350\244\363", "\244\244\244\310", "\244\246\244\271", "\244\246\244\352", "\244\252\244\244",

/* "����", "����", "�����", "����", "����", "����", */
"\244\253\244\363", "\244\255\244\314", "\244\263\244\355\244\342", "\244\263\244\341", "\244\267\244\277", "\244\271\244\255",

/* "����", "��", "�Ȥ�", "�ˤ�", "�ˤ�", "�Ϥ�", "�ҤĤ�", */
"\244\277\244\261", "\244\301", "\244\310\244\351", "\244\313\244\257", "\244\313\244\267", "\244\317\244\315", "\244\322\244\304\244\270",

/* "�դ�", "�դ�", "�ߤ�", "�ष", "����", "����", */
"\244\325\244\307", "\244\325\244\315", "\244\337\244\337", "\244\340\244\267", "\244\242\244\253", "\244\242\244\267",

/* "���Τ�", "����", "����", "���餤", "�����", "����", */
"\244\244\244\316\244\263", "\244\252\244\337", "\244\253\244\244", "\244\253\244\351\244\244", "\244\257\244\353\244\336", "\244\261\244\363",

/* "����", "����", "����", "����", "�Ĥ�", "�Τ���", */
"\244\264\244\363", "\244\265\244\261", "\244\275\244\246", "\244\277\244\313", "\244\304\244\316", "\244\316\244\264\244\341",

/* "�Ф�", "�ޤ�", "��", "�स��", "����", "���餺", */
"\244\320\244\257", "\244\336\244\341", "\244\337", "\244\340\244\270\244\312", "\244\242\244\341", "\244\242\244\351\244\272",

/* "����", "���", "�դ�Ȥ�", "�ڡ���", "����", "����", */
"\244\253\244\315", "\244\342\244\363", "\244\325\244\353\244\310\244\352", "\244\332\241\274\244\270", "\244\252\244\310", "\244\263\244\246",

/* "����", "����", "����", "���礯", "�ʤᤷ", "���", */
"\244\253\244\257", "\244\253\244\274", "\244\257\244\323", "\244\267\244\347\244\257", "\244\312\244\341\244\267", "\244\341\244\363",

/* "����", "����", "����", "������", "�Ȥ�", "�ۤ�", */
"\244\246\244\336", "\244\252\244\313", "\244\253\244\337", "\244\277\244\253\244\244", "\244\310\244\246", "\244\333\244\315",

/* "����", "����", "�Ȥ�", "����", "����", "�Ϥ�", */
"\244\246\244\252", "\244\253\244\341", "\244\310\244\352", "\244\257\244\355", "\244\267\244\253", "\244\317\244\312",

/* "��", "������", "���Τ�" */
"\244\317", "\244\255\244\264\244\246", "\244\275\244\316\244\277"
};


#define	BUSHU_CNT	(sizeof(bushu_schar)/sizeof(char *))

static wchar_t *bushu_char[BUSHU_CNT];
static wchar_t *bushu_key[BUSHU_CNT];

int
initBushuTable()
{
  int retval = 0;

  retval = setWStrings(bushu_char, bushu_schar, BUSHU_CNT);
  if (retval != NG) {
    retval = setWStrings(bushu_key, bushu_skey, BUSHU_CNT);
  }
  return retval;
}


/*
 * �������Υ������Ѥ�ʸ�������
 *
 * ������	RomeStruct
 * �����	���ｪλ�� 0
 */
static int
makeBushuEchoStr(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  d->kanji_status_return->echoStr = ic->allkouho[*(ic->curIkouho)];
  d->kanji_status_return->length = 1;
  d->kanji_status_return->revPos = 0;
  d->kanji_status_return->revLen = 1;

  return(0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * forichiranContext�Ѵؿ�                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * forichiranContext �ν����
 */
static
clearForIchiranContext(p)
forichiranContext p;
{
  p->id = FORICHIRAN_CONTEXT;
  p->curIkouho = 0;
  p->allkouho = 0;

  return(0);
}
  
static forichiranContext
newForIchiranContext()
{
  forichiranContext fcxt;

  if ((fcxt = (forichiranContext)malloc(sizeof(forichiranContextRec)))
                                             == (forichiranContext)NULL) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (newForIchiranContext) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (newForIchiranContext) \244\307\244\255\244\336\244\273\244\363\244\307\244\267\244\277";  /* �Ǥ��ޤ���Ǥ��� */
#endif
    return (forichiranContext)NULL;
  }
  clearForIchiranContext(fcxt);

  return fcxt;
}

getForIchiranContext(d)
uiContext d;
{
  forichiranContext fc;
  int retval = 0;

  if (pushCallback(d, d->modec, NO_CALLBACK, NO_CALLBACK,
                                  NO_CALLBACK, NO_CALLBACK) == 0) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (pushCallback) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (pushCallback) \244\307\244\255\244\336\244\273\244\363\244\307\244\267\244\277"; /* �Ǥ��ޤ���Ǥ��� */
#endif
    return(NG);
  }
  
  if((fc = newForIchiranContext()) == NULL) {
    popCallback(d);
    return(NG);
  }
  fc->next = d->modec;
  d->modec = (mode_context)fc;

  fc->prevMode = d->current_mode;
  fc->majorMode = d->majorMode;

  return(retval);
}

void
popForIchiranMode(d)
uiContext d;
{
  forichiranContext fc = (forichiranContext)d->modec;

  d->modec = fc->next;
  d->current_mode = fc->prevMode;
  freeForIchiranContext(fc);
}

#ifndef NO_EXTEND_MENU
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ����⡼������                                                            *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
vBushuMode(d, major_mode)
uiContext d;
int major_mode;
{
  forichiranContext fc;
  ichiranContext ic;
  unsigned inhibit = 0;
  int retval = 0;

  d->status = 0;

  if((retval = getForIchiranContext(d)) == NG) {
    killmenu(d);
    return(GLineNGReturn(d));
  }

  fc = (forichiranContext)d->modec;

  /* selectOne ��Ƥ֤���ν��� */
  fc->allkouho = bushu_char;
  fc->curIkouho = 0;
  if (!cannaconf.HexkeySelect)
    inhibit |= ((unsigned char)NUMBERING | (unsigned char)CHARINSERT);
  else
    inhibit |= (unsigned char)CHARINSERT;

  if((retval = selectOne(d, fc->allkouho, &fc->curIkouho, BUSHU_SZ,
		 BANGOMAX, inhibit, 0, WITH_LIST_CALLBACK,
		 NO_CALLBACK, vBushuExitCatch,
		 bushuQuitCatch, uiUtilIchiranTooSmall)) == NG) {
    killmenu(d);
    return(GLineNGReturnFI(d));
  }

  ic = (ichiranContext)d->modec;
  ic->majorMode = major_mode;
  ic->minorMode = CANNA_MODE_BushuMode;
  currentModeInfo(d);

  *(ic->curIkouho) = d->curbushu;

  /* ��������Ԥ������Ƹ���������Ф��ʤ� */
  if(ic->tooSmall) {
    d->status = AUX_CALLBACK;
    killmenu(d);
    return(retval);
  }

  if ( !(ic->flags & ICHIRAN_ALLOW_CALLBACK) ) {
    makeGlineStatus(d);
  }
  /* d->status = ICHIRAN_EVERYTIME; */

  return(retval);
}

static
vBushuIchiranQuitCatch(d, retval, env)
     uiContext d;
     int retval;
     mode_context env;
     /* ARGSUSED */
{
  popCallback(d); /* ������ݥå� */

  if (((forichiranContext)env)->allkouho != (wchar_t **)bushu_char) {
    /* bushu_char �� static ����������� free ���ƤϤ����ʤ���
       ���������ΤäƤʤ󤫱����ʤ� */
    freeGetIchiranList(((forichiranContext)env)->allkouho);
  }
  popForIchiranMode(d);
  popCallback(d);

  return(vBushuMode(d, CANNA_MODE_BushuMode));
}

static
vBushuExitCatch(d, retval, env)
     uiContext d;
     int retval;
     mode_context env;
     /* ARGSUSED */
{
  forichiranContext fc;
  int cur, res;

  popCallback(d); /* ������ݥå� */

  fc = (forichiranContext)d->modec;
  cur = fc->curIkouho;

  popForIchiranMode(d);
  popCallback(d);

  res = bushuHenkan(d, 1, 1, cur, vBushuIchiranQuitCatch);
  if (res < 0) {
    makeYomiReturnStruct(d);
    return 0;
  }
  return res;
}

BushuMode(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    killmenu(d);
    return NothingChangedWithBeep(d);
  }    

  return(vBushuMode(d, CANNA_MODE_BushuMode));
}
#endif /* not NO_EXTEND_MENU */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ����⡼�����Ϥΰ���ɽ��                                                  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static bushuEveryTimeCatch pro((uiContext, int, mode_context));

static
bushuEveryTimeCatch(d, retval, env)
     uiContext d;
     int retval;
     mode_context env;
     /* ARGSUSED */
{
  makeBushuEchoStr(d);

  return(retval);
}

static bushuExitCatch pro((uiContext, int, mode_context));

static
bushuExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
{
  yomiContext yc;

  popCallback(d); /* ������ݥå� */

  if (((forichiranContext)env)->allkouho != bushu_char) {
    /* bushu_char �� static ����������� free ���ƤϤ����ʤ���
       ���������ΤäƤʤ󤫱����ʤ� */
    freeGetIchiranList(((forichiranContext)env)->allkouho);
  }
  popForIchiranMode(d);
  popCallback(d);
  yc = (yomiContext)d->modec;
  if (yc->savedFlags & CANNA_YOMI_MODE_SAVED) {
    restoreFlags(yc);
  }
  retval = YomiExit(d, retval);
  killmenu(d);
  currentModeInfo(d);

  return retval;
}

#ifndef NO_EXTEND_MENU
static
bushuQuitCatch(d, retval, env)
     uiContext d;
     int retval;
     mode_context env;
     /* ARGSUSED */
{
  popCallback(d); /* ������ݥå� */

  if (((forichiranContext)env)->allkouho != (wchar_t **)bushu_char) {
    /* bushu_char �� static ����������� free ���ƤϤ����ʤ���
       ���������ΤäƤʤ󤫱����ʤ� */
    freeGetIchiranList(((forichiranContext)env)->allkouho);
  }
  popForIchiranMode(d);
  popCallback(d);
  currentModeInfo(d);
  GlineClear(d);

  return prevMenuIfExist(d);
}
#endif /* not NO_EXTEND_MENU */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ����Ȥ��Ƥ��Ѵ��ΰ���ɽ��                                                *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
convBushuQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
{
  popCallback(d); /* ������ݥå� */

  if (((forichiranContext)env)->allkouho != (wchar_t **)bushu_char) {
    /* bushu_char �� static ����������� free ���ƤϤ����ʤ���
       ���������ΤäƤʤ󤫱����ʤ� */
    freeGetIchiranList(((forichiranContext)env)->allkouho);
  }
  popForIchiranMode(d);
  popCallback(d);

  makeYomiReturnStruct(d);
  currentModeInfo(d);

  return(retval);
}

/*
 * �ɤߤ�����Ȥ����Ѵ�����
 *
 * ������	uiContext
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
int ConvertAsBushu pro((uiContext));

ConvertAsBushu(d)
uiContext	d;
{
  yomiContext yc = (yomiContext)d->modec;
  int res;

  d->status = 0; /* clear status */
  
  if (yc->henkanInhibition & CANNA_YOMI_INHIBIT_ASBUSHU ||
      yc->right || yc->left) {
    return NothingChangedWithBeep(d);
  }

  if (yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) {
    if (!(yc->status & CHIKUJI_OVERWRAP) && yc->nbunsetsu) {
      moveToChikujiTanMode(d);
      return TanKouhoIchiran(d);
    }
    else if (yc->nbunsetsu) {
      return NothingChanged(d);
    }
  }

  d->nbytes = yc->kEndp;
  WStrncpy(d->buffer_return, yc->kana_buffer, d->nbytes);

  /* 0 �ϡ�ConvertAsBushu ����ƤФ줿���Ȥ򼨤� */
  res = bushuHenkan(d, 0, 1, 0, convBushuQuitCatch);
  if (res < 0) {
    makeYomiReturnStruct(d);
    return 0;
  }
  return res;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ������                                                                    *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * �ɤߤ����󼭽񤫤������Ѵ�����
 */
static
bushuBgnBun(st, yomi, length)
RkStat *st;
wchar_t *yomi;
int length;
{
  int nbunsetsu;
  extern defaultBushuContext;

  /* Ϣʸ���Ѵ��򳫻Ϥ��� *//* ����ˤ������Τ߼��Ф� */
  if ((defaultBushuContext == -1)) {
    if (KanjiInit() == -1 || defaultBushuContext == -1) {
      jrKanjiError = KanjiInitError();
      return(NG);
    }
  }

  nbunsetsu = RkwBgnBun(defaultBushuContext, yomi, length, RK_CTRLHENKAN);
  if(nbunsetsu == -1) {
    if(errno == EPIPE)
      jrKanjiPipeError();
    jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271\244\313\274\272\307\324\244\267\244\336\244\267\244\277"; 
	    /* ���ʴ����Ѵ��˼��Ԥ��ޤ��� */
    return(NG);
  }
  
  if(RkwGetStat(defaultBushuContext, st) == -1) {
    if(errno == EPIPE)
      jrKanjiPipeError();
    jrKanjiError = "\245\271\245\306\245\244\245\277\245\271\244\362\274\350\244\352\275\320\244\273\244\336\244\273\244\363\244\307\244\267\244\277";
                  /* ���ƥ���������Ф��ޤ���Ǥ��� */
    return(NG);
  }

  return(nbunsetsu);
}

/*
 * �ɤߤ�Ⱦ�������ղä��Ƹ�������Ԥ�ɽ������
 *
 * ������	uiContext
 *		flag	ConvertAsBushu����ƤФ줿 0
 *			BushuYomiHenkan����ƤФ줿 1
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 *
 *
 * �����������Ϥޤ� getForIchiranContext ���ƤФ�Ƥ��ʤ���ΤȤ���
 */

static
bushuHenkan(d, flag, ext, cur, quitfunc)
uiContext	d;
int             flag, cur;
int             (*quitfunc) pro((uiContext, int, mode_context));
{
  forichiranContext fc;
  ichiranContext ic;
  unsigned inhibit = 0;
  wchar_t *yomi, **allBushuCands;
  RkStat	st;
  int nelem, currentkouho, nbunsetsu, length, retval = 0;
  extern defaultBushuContext;
  
  wchar_t **getIchiranList();

  if(flag) {
    yomi = (wchar_t *)bushu_key[cur];
    length = WStrlen(yomi);
    d->curbushu = (short)cur;
  } else {
    d->nbytes = RomajiFlushYomi(d, d->buffer_return, d->n_buffer);
    yomi = d->buffer_return;
    length = d->nbytes;
  }

  if((nbunsetsu = bushuBgnBun(&st, yomi, length)) == NG) {
    killmenu(d);
    (void)GLineNGReturn(d);
    return -1;
  }

  if((nbunsetsu != 1) || (st.klen > 1) || (st.maxcand == 0)) {
    /* ����Ȥ��Ƥθ��䤬�ʤ� */

    d->kanji_status_return->length = -1;

    makeBushuIchiranQuit(d, flag);
    currentModeInfo(d);

    killmenu(d);
    if(flag) {
      makeGLineMessageFromString(d, "\244\263\244\316\311\364\274\363\244\316\270\365\312\344\244\317\244\242\244\352\244\336\244\273\244\363");
                                  /* ��������θ���Ϥ���ޤ��� */
    } else {
      return(NothingChangedWithBeep(d));
    }
    return(0);
  }

  /* ��������Ԥ�ɽ������ */
  /* 0 �ϡ������ȸ��� + 0 �򥫥��ȸ���ˤ��뤳�Ȥ򼨤� */

  if((allBushuCands
      = getIchiranList(defaultBushuContext, &nelem, &currentkouho)) == 0) {
    killmenu(d);
    (void)GLineNGReturn(d);
    return -1;
  }

  /* �����Ѵ��ϳؽ����ʤ��� */
  if(RkwEndBun(defaultBushuContext, 0) == -1) { /* 0:�ؽ����ʤ� */
    if(errno == EPIPE)
      jrKanjiPipeError();
    jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271\244\316\275\252\316\273\244\313\274\272\307\324\244\267\244\336\244\267\244\277";
                   /* ���ʴ����Ѵ��ν�λ�˼��Ԥ��ޤ��� */
    freeGetIchiranList(allBushuCands);
    killmenu(d);
    (void)GLineNGReturn(d);
    return -1;
  }

  if(getForIchiranContext(d) == NG) {
    freeGetIchiranList(allBushuCands);
    killmenu(d);
    (void)GLineNGReturn(d);
    return -1;
  }

  fc = (forichiranContext)d->modec;
  fc->allkouho = allBushuCands;

  if (!cannaconf.HexkeySelect)
    inhibit |= (unsigned char)NUMBERING;
  fc->curIkouho = currentkouho;	/* ���ߤΥ����ȸ����ֹ����¸���� */
  currentkouho = 0;	/* �����ȸ��䤫�鲿���ܤ򥫥��ȸ���Ȥ��뤫 */

  if((retval = selectOne(d, fc->allkouho, &fc->curIkouho, nelem, BANGOMAX,
			 inhibit, currentkouho, WITH_LIST_CALLBACK,
			 bushuEveryTimeCatch, bushuExitCatch,
			 quitfunc, uiUtilIchiranTooSmall)) == NG) {
    freeGetIchiranList(allBushuCands);
    killmenu(d);
    (void)GLineNGReturnFI(d);
    return -1;
  }

  ic = (ichiranContext)d->modec;

  if(!flag) { /* convertAsBushu */
    ic->majorMode = ic->minorMode = CANNA_MODE_BushuMode;
  } else {
    if(ext) {
      ic->majorMode = ic->minorMode = CANNA_MODE_BushuMode;
    } else {
      ic->majorMode = CANNA_MODE_ExtendMode;
      ic->minorMode = CANNA_MODE_BushuMode;
    }
  }
  currentModeInfo(d);

  /* ��������Ԥ������Ƹ���������Ф��ʤ� */
  if(ic->tooSmall) {
    d->status = AUX_CALLBACK;
    killmenu(d);
    return(retval);
  }

  if ( !(ic->flags & ICHIRAN_ALLOW_CALLBACK) ) {
    makeGlineStatus(d);
  }
  /* d->status = EVERYTIME_CALLBACK; */

  return(retval);
}

/*
 * ����Ԥ�õ������⡼�ɤ���ȴ�����ɤߤ��ʤ��⡼�ɤ˰ܹԤ���
 *
 * ������	uiContext
 *		flag	ConvertAsBushu����ƤФ줿 0
 *			BushuYomiHenkan����ƤФ줿 1
 * �����	���ｪλ�� 0	�۾ｪλ�� -1
 */
static
makeBushuIchiranQuit(d, flag)
uiContext	d;
int              flag;
{
  extern defaultBushuContext;

  /* �����Ѵ��ϳؽ����ʤ��� */
  if(RkwEndBun(defaultBushuContext, 0) == -1) { /* 0:�ؽ����ʤ� */
    if(errno == EPIPE)
      jrKanjiPipeError();
    jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271\244\316\275\252\316\273\244\313\274\272\307\324\244\267\244\336\244\267\244\277";
                   /* ���ʴ����Ѵ��ν�λ�˼��Ԥ��ޤ��� */
    return(NG);
  }

  if(flag) {
    /* kanji_status_return �򥯥ꥢ���� */
    d->kanji_status_return->length  = 0;
    d->kanji_status_return->revLen  = 0;
    
/*
    d->status = QUIT_CALLBACK;
*/
  } else {
    makeYomiReturnStruct(d);
  }
  GlineClear(d);
  
  return(0);
}


#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/
