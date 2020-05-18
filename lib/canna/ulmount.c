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
static char rcs_id[] = "@(#) 102.1 $Id: ulmount.c,v 1.3 2003/09/17 08:50:53 aida_s Exp $";
#endif

#ifndef NO_EXTEND_MENU
#include	<errno.h>
#include 	"canna.h"

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

/* cfunc mountContext
 *
 * mountContext
 *
 */
static mountContext
newMountContext()
{
  mountContext mcxt;

  if ((mcxt = (mountContext)calloc(1, sizeof(mountContextRec)))
                                           == (mountContext)NULL) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (newMountContext) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (newMountContext) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
#endif
    return (mountContext)NULL;
  }
  mcxt->id = MOUNT_CONTEXT;

  return mcxt;
}

static void
freeMountContext(mc)
mountContext mc;
{
  if (mc) {
    if (mc->mountList) {
      if (*(mc->mountList)) {
	free(*(mc->mountList));
      }
      free(mc->mountList);
    }
    if (mc->mountOldStatus) {
      free(mc->mountOldStatus);
    }
    if (mc->mountNewStatus) {
      free(mc->mountNewStatus);
    }
    free(mc);
  }
}

/*
 * ��������Ԥ���
 */
getMountContext(d)
uiContext d;
{
  mountContext mc;
  int retval = 0;

  if (pushCallback(d, d->modec,
                   NO_CALLBACK, NO_CALLBACK,
                   NO_CALLBACK, NO_CALLBACK) == 0) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (pushCallback) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (pushCallback) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
#endif
    return(NG);
  }
  
  if((mc = newMountContext()) == (mountContext)NULL) {
    popCallback(d);
    return(NG);
  }
  mc->majorMode = d->majorMode;
  mc->next = d->modec;
  d->modec = (mode_context)mc;

  mc->prevMode = d->current_mode;

  return(retval);
}

void
popMountMode(d)
uiContext d;
{
  mountContext mc = (mountContext)d->modec;

  d->modec = mc->next;
  d->current_mode = mc->prevMode;
  freeMountContext(mc);
}

static struct dicname *
findDic(s)
char *s;
{
  extern struct dicname *kanjidicnames;
  struct dicname *dp;

  for (dp = kanjidicnames ; dp ; dp = dp->next) {
    if (!strcmp(s, dp->name)) {
      return dp;
    }
  }
  return (struct dicname *)0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ����Υޥ���ȡ�����ޥ����                                              *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
uuMountExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  mountContext mc;
  int i, nmount = 0;
  extern defaultContext;
  struct dicname *dp;

  killmenu(d);
  popCallback(d); /* OnOff ��ݥå� */

  if(defaultContext == -1) {
    if((KanjiInit() != 0) || (defaultContext == -1)) {
#ifdef STANDALONE
#ifndef CODED_MESSAGE
      jrKanjiError = "���ʴ����Ѵ��Ǥ��ޤ���";
#else
      jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271\244\307\244\255\244\336\244\273\244\363";
#endif
#else
#ifndef CODED_MESSAGE
      jrKanjiError = "���ʴ����Ѵ������Ф��̿��Ǥ��ޤ���";
#else
      jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271\245\265"
	"\241\274\245\320\244\310\304\314\277\256\244\307\244\255\244\336"
	"\244\273\244\363";
#endif
#endif
      popMountMode(d);
      popCallback(d);
      return(GLineNGReturn(d));
    }
  }

  mc = (mountContext)d->modec;
  for(i=0; mc->mountList[i]; i++) {
    if(mc->mountOldStatus[i] != mc->mountNewStatus[i]) {
      if(mc->mountNewStatus[i]) {
	/* �ޥ���Ȥ��� */
	nmount++;
	if((retval = RkwMountDic(defaultContext, (char *)mc->mountList[i],
			    cannaconf.kojin ? PL_ALLOW : PL_INHIBIT)) == NG) {
	  if (errno == EPIPE) {
	    jrKanjiPipeError();
	  }
	  MBstowcs(d->genbuf, "\274\255\275\361\244\316\245\336\245\246"
		"\245\363\245\310\244\313\274\272\307\324\244\267\244\336"
		"\244\267\244\277", 512);
                       /* ����Υޥ���Ȥ˼��Ԥ��ޤ��� */
	}
	else if(d->contextCache != -1 &&
	  (retval = RkwMountDic(d->contextCache, (char *)mc->mountList[i],
			    cannaconf.kojin ? PL_ALLOW : PL_INHIBIT)) == NG) {
	  if (errno == EPIPE) {
	    jrKanjiPipeError();
	  }
	  MBstowcs(d->genbuf, "\274\255\275\361\244\316\245\336\245\246"
		"\245\363\245\310\244\313\274\272\307\324\244\267\244\336"
		"\244\267\244\277", 512);
                              /* ����Υޥ���Ȥ˼��Ԥ��ޤ��� */
	}
	else { /* ���� */
	  dp = findDic((char *)mc->mountList[i]);
	  if (!dp) {
	    dp = (struct dicname *)malloc(sizeof(struct dicname));
	    if (dp) {
	      dp->name = malloc(strlen((char *)mc->mountList[i]) + 1);
	      if (dp->name) {
		/* �ޥ���Ȥ�����Ĥϥꥹ�ȤˤĤʤ� */
		strcpy(dp->name, (char *)mc->mountList[i]);
		dp->dictype = DIC_PLAIN;
		/* dp->dicflag = DIC_NOT_MOUNTED; will be rewritten below */
		dp->next = kanjidicnames;
		kanjidicnames = dp;
	      }
	      else { /* malloc failed */
		free((char *)dp);
		dp = (struct dicname *)0;
	      }
	    }
	  }
	  if (dp) {
	    dp->dicflag = DIC_MOUNTED;
	  }
	}
      } else {
	/* ����ޥ���Ȥ��� */
	nmount++;
	if((retval = RkwUnmountDic(defaultContext, (char *)mc->mountList[i]))
	   == NG) {
	  if (errno == EPIPE) {
	    jrKanjiPipeError();
	  }
	  MBstowcs(d->genbuf, "\274\255\275\361\244\316\245\242\245\363"
		"\245\336\245\246\245\363\245\310\244\313\274\272\307\324"
		"\244\267\244\336\244\267\244\277", 512);
                             /* ����Υ���ޥ���Ȥ˼��Ԥ��ޤ��� */
	}
	else if(d->contextCache != -1 &&
	  (retval = RkwUnmountDic(d->contextCache, (char *)mc->mountList[i]))
		== NG) {
	  if (errno == EPIPE) {
	    jrKanjiPipeError();
	  }
	  MBstowcs(d->genbuf, "\274\255\275\361\244\316\245\242\245\363"
		"\245\336\245\246\245\363\245\310\244\313\274\272\307\324"
		"\244\267\244\336\244\267\244\277", 512);
                             /* ����Υ���ޥ���Ȥ˼��Ԥ��ޤ��� */
	}
	else {
	  dp = findDic((char *)mc->mountList[i]);
	  if (dp) { /* ���ʤ餺�ʲ����̤�Ϥ� */
	    dp->dicflag = DIC_NOT_MOUNTED;
	  }
	}
      }
    }
  }

  if(nmount)
    makeAllContextToBeClosed(1);

  if(retval != NG)
    MBstowcs(d->genbuf, "\274\255\275\361\244\316\245\336\245\246\245\363"
	"\245\310\241\277\245\242\245\363\245\336\245\246\245\363\245\310"
	"\244\362\271\324\244\244\244\336\244\267\244\277", 512);
           /* ����Υޥ���ȡ�����ޥ���Ȥ�Ԥ��ޤ��� */ 
  else
    MBstowcs(d->genbuf, "\274\255\275\361\244\316\245\336\245\246\245\363"
	"\245\310\241\277\245\242\245\363\245\336\245\246\245\363\245\310"
	"\244\313\274\272\307\324\244\267\244\336\244\267\244\277", 512);
           /* ����Υޥ���ȡ�����ޥ���Ȥ˼��Ԥ��ޤ��� */
  makeGLineMessage(d, d->genbuf, WStrlen(d->genbuf));

  popMountMode(d);
  popCallback(d);
  currentModeInfo(d);

  return(0);
}

static
uuMountQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* OnOff ��ݥå� */

  popMountMode(d);
  popCallback(d);
  currentModeInfo(d);

  return prevMenuIfExist(d);
}

/*
 * dicLbuf                dicLp       soldp   snewp
 * ���������������������� ����������  ������  ������
 * ��iroha\@fuzokugo\@k�� ��*iroha��  ��1 ��  ��1 ��
 * ��atakana\@satoko\@s�� ��*fuzo ��  ��1 ��  ��1 ��
 * ��oft\@\@...        �� ��*kata ��  ��0 ��  ��0 ��
 * ��                  �� ��  :   ��  ��: ��  ��: ��
 * ���������������������� ����������  ������  ������
 * dicMbuf                dicMp
 * ���������������������� ����������
 * ��iroha\@fuzokugo\@s�� ��*iroha��
 * ��atoko\@\@...      �� ��*fuzo ��
 * ��                  �� ��*sato ��
 * ��                  �� ��  :   ��
 * ���������������������� ����������
 */
static
getDicList(d)
uiContext d;
{
  mountContext mc = (mountContext)d->modec;
  char *dicLbuf, dicMbuf[ROMEBUFSIZE];
  char **dicLp, *dicMp[ROMEBUFSIZE/2];
  char *wptr, **Lp, **Mp;
  BYTE *sop, *snp, *soldp, *snewp;
  int dicLc, dicMc, i;
  extern defaultContext;

  if((dicLbuf = malloc(ROMEBUFSIZE)) == (char *)NULL) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (getDicList) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (getDicList) \244\307\244\255\244\336\244\273";
#endif
    return(NG);
  }
  if(defaultContext == -1) {
    if((KanjiInit() != 0) || (defaultContext == -1)) {
#ifdef STANDALONE
#ifndef CODED_MESSAGE
      jrKanjiError = "���ʴ����Ѵ��Ǥ��ޤ���";
#else
      jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271\244\307\244\255\244\336\244\273\244\363";
#endif
#else
#ifndef CODED_MESSAGE
      jrKanjiError = "���ʴ����Ѵ������Ф��̿��Ǥ��ޤ���";
#else
      jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271"
	"\245\265\241\274\245\320\244\310\304\314\277\256\244\307\244\255"
	"\244\336\244\273\244\363";
#endif
#endif
      free(dicLbuf);
      return(NG);
    }
  }
  if((dicLc = RkwGetDicList(defaultContext, (char *)dicLbuf, ROMEBUFSIZE))
     < 0) {
    if(errno == EPIPE)
      jrKanjiPipeError();
    jrKanjiError = "\245\336\245\246\245\363\245\310\262\304\307\275\244\312"
	"\274\255\275\361\244\316\274\350\244\352\275\320\244\267\244\313"
	"\274\272\307\324\244\267\244\336\244\267\244\277";
                   /* �ޥ���Ȳ�ǽ�ʼ���μ��Ф��˼��Ԥ��ޤ��� */
    free(dicLbuf);
    return(NG);
  }
  if (dicLc == 0) {
    jrKanjiError = "\245\336\245\246\245\363\245\310\262\304\307\275\244\312"
	"\274\255\275\361\244\254\302\270\272\337\244\267\244\336\244\273"
	"\244\363";
                   /* �ޥ���Ȳ�ǽ�ʼ���¸�ߤ��ޤ��� */
    free(dicLbuf);
    return NG;
  }
  if((dicLp = (char **)calloc(dicLc + 1, sizeof(char *))) == (char **)NULL) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (getDicList) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (getDicList) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
#endif
    free(dicLbuf);
    return(NG);
  }
  if((soldp = (BYTE *)malloc(dicLc + 1)) == (BYTE *)NULL) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (getDicList) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (getDicList) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
#endif
    free(dicLbuf);
    free(dicLp);
    return(NG);
  }
  if((snewp = (BYTE *)malloc(dicLc + 1)) == (BYTE *)NULL) {
#ifndef CODED_MESSAGE
    jrKanjiError = "malloc (getDicList) �Ǥ��ޤ���Ǥ���";
#else
    jrKanjiError = "malloc (getDicList) \244\307\244\255\244\336\244\273"
	"\244\363\244\307\244\267\244\277";
#endif
    free(dicLbuf);
    free(dicLp);
    free((char *)soldp);
    return(NG);
  }
  for(i = 0, wptr = dicLbuf; i < dicLc; i++) { /* buf ���� */
    dicLp[i] = wptr;
    while(*wptr++)
      /* EMPTY */
      ; /* NULL �ޤǥ����åפ���NULL �μ��ޤǥݥ��󥿤�ʤ�� */
  }
  dicLp[i] = (char *)NULL;

  if(defaultContext == -1) {
    if((KanjiInit() != 0) || (defaultContext == -1)) {
#ifdef STANDALONE
#ifndef CODED_MESSAGE
      jrKanjiError = "���ʴ����Ѵ��Ǥ��ޤ���";
#else
      jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271\244\307\244\255\244\336\244\273\244\363";
#endif
#else
#ifndef CODED_MESSAGE
      jrKanjiError = "���ʴ����Ѵ������Ф��̿��Ǥ��ޤ���";
#else
      jrKanjiError = "\244\253\244\312\264\301\273\372\312\321\264\271"
	"\245\265\241\274\245\320\244\310\304\314\277\256\244\307\244\255"
	"\244\336\244\273\244\363";
#endif
#endif
      free(dicLbuf);
      free(dicLp);
      free((char *)soldp);
      return(NG);
    }
  }
  if((dicMc = RkwGetMountList(defaultContext, (char *)dicMbuf, ROMEBUFSIZE)) <
     0) {
    if(errno == EPIPE)
      jrKanjiPipeError();
    jrKanjiError = "\245\336\245\246\245\363\245\310\244\267\244\306\244\244"
	"\244\353\274\255\275\361\244\316\274\350\244\352\275\320\244\267"
	"\244\313\274\272\307\324\244\267\244\336\244\267\244\277";
                   /* �ޥ���Ȥ��Ƥ��뼭��μ��Ф��˼��Ԥ��ޤ��� */
    free(dicLbuf);
    free(dicLp);
    free((char *)soldp);
    free((char *)snewp);
    return(NG);
  }

  for(i = 0, wptr = dicMbuf ; i < dicMc ; i++) { /* buf ���� */
    dicMp[i] = wptr;
    while (*wptr++)
      /* EMPTY */
      ; /* NULL �ޤǥ����åפ���NULL �μ��ޤǥݥ��󥿤�ʤ�� */
  }
  dicMp[i] = (char *)NULL;

  for(i=0, sop=soldp, snp=snewp; i<dicLc; i++, sop++, snp++) {
    *sop = 0;
    *snp = 0;
  }
  for(Lp=dicLp, sop=soldp, snp=snewp; *Lp; Lp++, sop++, snp++) {
    for(Mp=dicMp; *Mp; Mp++) {
      if(!strcmp((char *)*Lp, (char *)*Mp)) {
	*sop = *snp = 1;
	break;
      }
    }
  }
  mc->mountList = dicLp;
  mc->mountOldStatus = soldp;
  mc->mountNewStatus = snewp;

  return(dicLc);
}

dicMount(d)
uiContext d;
{
  ichiranContext oc;
  mountContext mc;
  int retval = 0, currentkouho = 0, nelem;
  wchar_t *xxxx[100];
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  d->status = 0;

  if((retval = getMountContext(d)) == NG) {
    killmenu(d);
    return(GLineNGReturn(d));
  }

  /* ����ꥹ�Ȥȥޥ���ȡ�����ޥ���Ȥξ��֤� montContext �ˤȤäƤ��� */
  if((nelem = getDicList(d)) == NG) {
    popMountMode(d);
    popCallback(d);
    killmenu(d);
    return(GLineNGReturn(d));
  }

  mc = (mountContext)d->modec;
#if defined(DEBUG)
  if(iroha_debug) {
    int i;

    printf("<��mount>\n");
    for(i= 0; mc->mountList[i]; i++)
      printf("[%s][%x][%x]\n", mc->mountList[i],
	     mc->mountOldStatus[i], mc->mountNewStatus[i]);
    printf("\n");
  }
#endif

  /* selectOnOff ��Ƥ֤���ν��� */
  mc->curIkouho = currentkouho = 0;

  retval = setWStrings(xxxx, mc->mountList, 0);
  if (retval == NG) {
    popMountMode(d);
    popCallback(d);
    killmenu(d);
    return GLineNGReturn(d);
  }
  if((retval = selectOnOff(d, xxxx, &mc->curIkouho, nelem,
		 BANGOMAX, currentkouho, mc->mountOldStatus,
		 NO_CALLBACK, uuMountExitCatch,
		 uuMountQuitCatch, uiUtilIchiranTooSmall)) == NG) {
    popMountMode(d);
    popCallback(d);
    killmenu(d);
    return GLineNGReturn(d);
  }

  oc = (ichiranContext)d->modec;
  oc->majorMode = CANNA_MODE_ExtendMode;
  oc->minorMode = CANNA_MODE_MountDicMode;
  currentModeInfo(d);

  /* ��������Ԥ������Ƹ���������Ф��ʤ� */
  if(oc->tooSmall) {
    wchar_t p[512];

    ichiranFin(d);
    popCallback(d); /* OnOff ��ݥå� */
    popMountMode(d);
    popCallback(d);
    currentModeInfo(d);
    MBstowcs(p ,"\274\255\275\361\260\354\315\367\315\321\244\316\311\375"
		"\244\254\266\271\244\244\244\316\244\307\274\255\275\361"
		"\245\336\245\246\245\363\245\310\241\277\245\242\245\363"
		"\245\336\245\246\245\363\245\310\244\307\244\255\244\336"
		"\244\273\244\363",64);
         /* ��������Ѥ����������ΤǼ���ޥ���ȡ�����ޥ���ȤǤ��ޤ��� */
    makeGLineMessage(d, p, WStrlen(p));
    killmenu(d);
    return(0);
  }

  makeGlineStatus(d);
  /* d->status = ICHIRAN_EVERYTIME; */

  return(retval);
}

#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/

#endif /* NO_EXTEND_MENU */
