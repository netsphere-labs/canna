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
static char rcs_id[] = "@(#) 102.1 $Id: uiutil.c,v 1.3 2003/09/17 08:50:53 aida_s Exp $";
#endif

#include "canna.h"
#include "patchlevel.h"

/*********************************************************************
 *                      wchar_t replace begin                        *
 *********************************************************************/
#ifdef wchar_t
# error "wchar_t is already defined"
#endif
#define wchar_t cannawc

#ifndef NO_EXTEND_MENU

typedef struct {
  char *title;
  int func;
  int funcd;
} e_menuitem;

#define MENU_NEXT_MENU 0 /* ����ȥ�ϥ�˥塼�Ǥ��� */
#define MENU_FUNC_NUM  1 /* ����ȥ�ϡؤ���ʡ٤ε�ǽ�ֹ�Ǥ��� */

#ifdef STANDALONE /* This is not used in Windows environment 1996.7.30 kon */
#define MT_HELP   0
#define MT_SONOTA 1
#define MT_KIGO   2
#define MT_TANGO  3
#define MT_HENKAN 4
#define MT_FILE   5
#else
#define MT_HELP   0
#define MT_SONOTA 1
#define MT_KIGO   2
#define MT_SERV   3
#define MT_TANGO  4
#define MT_HENKAN 5
#define MT_FILE   6
#endif

static e_menuitem e_helptable[] = { 
  /* �������� */
  {"\265\255\271\346\306\376\316\317",   MENU_NEXT_MENU, MT_KIGO}, 
  /* ���������� */
  {"\245\263\241\274\245\311\306\376\316\317", MENU_FUNC_NUM,  CANNA_FN_HexMode},  
  /* �������� */
  {"\311\364\274\363\306\376\316\317",   MENU_FUNC_NUM,  CANNA_FN_BushuMode}, 
  /* ñ����Ͽ */
  {"\303\261\270\354\305\320\317\277",   MENU_NEXT_MENU, MT_TANGO},
  /* �Ķ����� */
  {"\264\304\266\255\300\337\304\352",   MENU_NEXT_MENU, MT_SONOTA},
};

static e_menuitem e_uusonotatable[] = { 
#ifndef CODED_MESSAGE
  {"�Ѵ�����",       MENU_NEXT_MENU, MT_HENKAN},
#ifndef STANDALONE /* This is not used in Windows environment */
  {"���������",     MENU_NEXT_MENU, MT_SERV},
#endif
  {"����ޥ���ȡ�����ޥ����", MENU_FUNC_NUM, CANNA_FN_DicMountMode},
  {"�ؽ�����ɽ��",   MENU_FUNC_NUM,  CANNA_FN_ShowGakushu},
  {"�С������ɽ��", MENU_FUNC_NUM,  CANNA_FN_ShowVersion},
  {"�ե�����ɽ��",   MENU_NEXT_MENU, MT_FILE},
#else
  /* �Ѵ����� */
  {"\312\321\264\271\312\375\274\260",       MENU_NEXT_MENU, MT_HENKAN},
#ifndef STANDALONE /* This is not used in Windows environment 1996.7.30 kon */
  /* ��������� */
  {"\245\265\241\274\245\320\301\340\272\356",     MENU_NEXT_MENU, MT_SERV},
#endif
  /* ����ޥ���ȡ�����ޥ���� */
  {"\274\255\275\361\245\336\245\246\245\363\245\310\241\277\245\242\245\363\245\336\245\246\245\363\245\310", MENU_FUNC_NUM, CANNA_FN_DicMountMode},
  /* �ؽ�����ɽ�� */
  {"\263\330\275\254\276\365\302\326\311\275\274\250",   MENU_FUNC_NUM,  CANNA_FN_ShowGakushu},
  /* �С������ɽ�� */
  {"\245\320\241\274\245\270\245\347\245\363\311\275\274\250", MENU_FUNC_NUM,  CANNA_FN_ShowVersion},
  /* �ե�����ɽ�� */
  {"\245\325\245\241\245\244\245\353\311\275\274\250",   MENU_NEXT_MENU, MT_FILE},
#endif
};

static e_menuitem e_uukigotable[] = {
  /* �������� */
  {"\265\255\271\346\301\264\310\314",     MENU_FUNC_NUM, CANNA_FN_KigouMode},
  /* ����ʸ�� */
  {"\245\355\245\267\245\242\312\270\273\372",   MENU_FUNC_NUM, CANNA_FN_RussianMode},
  /* ���ꥷ��ʸ�� */
  {"\245\256\245\352\245\267\245\343\312\270\273\372", MENU_FUNC_NUM, CANNA_FN_GreekMode},
  /* ���� */
  {"\267\323\300\376",         MENU_FUNC_NUM, CANNA_FN_LineMode},
};

#ifndef STANDALONE /* This is not used in Windows environment 1996.7.30 kon */
static e_menuitem e_uuservertable[] = {
  /* �����Ф��ڤ�Υ�� */
  {"\245\265\241\274\245\320\244\316\300\332\244\352\316\245\244\267", MENU_FUNC_NUM, CANNA_FN_DisconnectServer},
  /* �����Ф��ڤ��ؤ� */
  {"\245\265\241\274\245\320\244\316\300\332\244\352\302\330\244\250", MENU_FUNC_NUM, CANNA_FN_ChangeServerMode},
  /* �����Ф�ɽ�� */
  {"\245\265\241\274\245\320\244\316\311\275\274\250",     MENU_FUNC_NUM, CANNA_FN_ShowServer},
};
#endif /* STANDALONE */

static e_menuitem e_uutangotable[] = {
  /* ñ����Ͽ */
  {"\303\261\270\354\305\320\317\277", MENU_FUNC_NUM, CANNA_FN_DefineDicMode},
  /* ñ���� */
  {"\303\261\270\354\272\357\275\374", MENU_FUNC_NUM, CANNA_FN_DeleteDicMode},
  /* ����ޥ���ȡ�����ޥ���� */
  {"\274\255\275\361\245\336\245\246\245\363\245\310\241\277\245\242\245\363\245\336\245\246\245\363\245\310", MENU_FUNC_NUM, CANNA_FN_DicMountMode},
  };

static e_menuitem e_uuhenkantable[] = {
  /* Ϣʸ���Ѵ� */
  {"\317\242\312\270\300\341\312\321\264\271",   MENU_FUNC_NUM, CANNA_FN_EnterRenbunMode},
  /* �༡��ư�Ѵ� */
  {"\303\340\274\241\274\253\306\260\312\321\264\271", MENU_FUNC_NUM, CANNA_FN_EnterChikujiMode},
};

static e_menuitem e_uufiletable[] = {
  /* ���޻������Ѵ��ơ��֥� */
  {"\245\355\241\274\245\336\273\372\244\253\244\312\312\321\264\271\245\306\241\274\245\326\245\353", MENU_FUNC_NUM, CANNA_FN_ShowPhonogramFile},
  /* �������ޥ����ե����� */
  {"\245\253\245\271\245\277\245\336\245\244\245\272\245\325\245\241\245\244\245\353", MENU_FUNC_NUM, CANNA_FN_ShowCannaFile},
};


#define numitems(x) ((sizeof(x)) / sizeof(e_menuitem))

static struct _e_menu {
  e_menuitem *mi;
  int ni;
} e_me[] = {                                    /* MT_ �ν�ȹ�碌�뤳�� */
  {e_helptable,     numitems(e_helptable)},     /* MT_HELP */
  {e_uusonotatable, numitems(e_uusonotatable)}, /* MT_SONOTA */
  {e_uukigotable,   numitems(e_uukigotable)},   /* MT_KIGO */
#ifndef STANDALONE /* This is not used in Windows environment 1996.7.30 kon */
  {e_uuservertable, numitems(e_uuservertable)}, /* MT_SERV */
#endif /* STANDALONE */
  {e_uutangotable,  numitems(e_uutangotable)},  /* MT_TANGO */
  {e_uuhenkantable, numitems(e_uuhenkantable)}, /* MT_HENKAN */
  {e_uufiletable,   numitems(e_uufiletable)},   /* MT_FILE */
};

#define N_BUILTIN_MENU (sizeof(e_me) / sizeof(struct _e_menu))

static menustruct *me[N_BUILTIN_MENU];

#define MBUFSIZE 512

void
freeMenu(m)
menustruct *m;
{
  free((char *)m->titles);
  free((char *)m->titledata);
  free((char *)m->body);
  free((char *)m);
}

menustruct *
allocMenu(n, nc)
int n, nc;
{
  wchar_t *wctab, **wcs;
  menuitem *menubody;
  menustruct *res;

  res = (menustruct *)malloc(sizeof(menustruct));
  if (res) {
    wctab = (wchar_t *)malloc(sizeof(wchar_t) * nc);
    if (wctab) {
      wcs = (wchar_t **)malloc(sizeof(wchar_t *) * n);
      if (wcs) {
	menubody = (menuitem *)malloc(sizeof(menuitem) * n);
	if (menubody) {
	  res->titles = wcs;
	  res->titledata = wctab;
	  res->body = menubody;
	  return res;
	}
	free((char *)wcs);
      }
      free((char *)wctab);
    }
    free((char *)res);
  }
  return (menustruct *)0;
}

static menustruct *
copystruct(eucmenu)
struct _e_menu *eucmenu;
{
  int i, nc, len, n = eucmenu->ni;
  e_menuitem *euctable = eucmenu->mi;
  menuitem *menubody;
  wchar_t *wp, **wpp;
  menustruct *res = (menustruct *)0;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  wchar_t buf[MBUFSIZE];
#else
  wchar_t *buf = (wchar_t *)malloc(sizeof(wchar_t) * MBUFSIZE);
  if (!buf) {
    return res;
  }
#endif

  /* �����ȥ��ʸ�����򥫥���� */
  for (i = 0, nc = 0 ; i < n ; i++) {
    len = MBstowcs(buf, euctable[i].title, MBUFSIZE);
    nc += len + 1;
  }

  res = allocMenu(n, nc);
  if (res) {
    menubody = res->body;
    /* �����ȥ�ʸ����ǡ����Хåե��˥��ԡ� */
    for (i = 0, wp = res->titledata, wpp = res->titles ; i < n ; i++) {
      len = MBstowcs(wp, euctable[i].title, MBUFSIZE);
      *wpp++ = wp;
      wp += len + 1;

      /* ��¤�Τ򥳥ԡ����� */
      switch (euctable[i].func) {
      case MENU_NEXT_MENU:
	menubody[i].flag = MENU_MENU;
	menubody[i].u.fnum = euctable[i].funcd;
	break;
      case MENU_FUNC_NUM:
	menubody[i].flag = MENU_FUNC;
	menubody[i].u.fnum = euctable[i].funcd;
	break;
      }
    }
    res->nentries = n;
    res->modeid = CANNA_MODE_ExtendMode;
  }
#ifdef USE_MALLOC_FOR_BIG_ARRAY
  (void)free((char *)buf);
#endif
  return res;
}

/*
 * menuitem�������Ƥι�¤�Τ�"unsigned char"����"wchar_t"���Ѵ�����
 */
int
initExtMenu()
{
  int i, j;

  for (i = 0 ; i < N_BUILTIN_MENU ; i++) {
    me[i] = copystruct(e_me + i);
    if (!me[i]) {
      for (j = 0 ; j < i ; j++) {
	freeMenu(me[j]);
      }
      return -1;
    }
  }
  for (i = 0 ; i < N_BUILTIN_MENU ; i++) {
    menustruct *m = me[i];
    for (j = 0 ; j < m->nentries ; j++) {
      if (m->body[j].flag == MENU_MENU) {
	m->body[j].u.menu_next = me[m->body[j].u.fnum];
      }
    }
  }

  return 0;
}

#undef numitems

void
finExtMenu()
{
  int i;
  for (i = 0 ; i < N_BUILTIN_MENU ; i++) {
    freeMenu(me[i]);
  }
}
#endif /* NO_EXTEND_MENU */

static
makeUiUtilEchoStr(d)
uiContext d;
{
  ichiranContext ic = (ichiranContext)d->modec;

  d->kanji_status_return->echoStr = ic->allkouho[*(ic->curIkouho)];
  d->kanji_status_return->length = WStrlen(ic->allkouho[*(ic->curIkouho)]);
  d->kanji_status_return->revPos = 0;
  d->kanji_status_return->revLen = 0;

  return(0);
}

int
uiUtilIchiranTooSmall(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  makeUiUtilEchoStr(d);
  return 0;
}

#ifndef NO_EXTEND_MENU
static void
pushmenu(d, tab)
uiContext d;
menustruct *tab;
{
  tab->prev = d->prevMenu;
  d->prevMenu = tab;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * UI�桼�ƥ���ƥ��ΰ���ɽ��(FirstLine)                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static
uuflExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  forichiranContext fc;
  menustruct *mtab, *ptab;
  menuitem *men;
  int cur;

  d->nbytes = 0;

  popCallback(d); /* ������ pop */

  fc = (forichiranContext)d->modec;
  cur = fc->curIkouho;
  if (fc->prevcurp) {
    *(fc->prevcurp) = cur;
  }
  men = fc->table->body + cur;
  ptab = fc->table;

  popForIchiranMode(d);
  popCallback(d);

  pushmenu(d, ptab);
  switch (men->flag) {
  case MENU_MENU:
    for (mtab = d->prevMenu ; mtab ; mtab = mtab->prev) {
      if (mtab == men->u.menu_next) {
	killmenu(d);
	jrKanjiError = "\244\263\244\316\271\340\314\334\244\316\245\341"
	"\245\313\245\345\241\274\244\317\272\306\265\242\305\252\244\313"
	"\301\252\302\362\244\265\244\354\244\306\244\244\244\336\244\271";
                     /* ���ι��ܤΥ�˥塼�ϺƵ�Ū�����򤵤�Ƥ��ޤ� */
	makeGLineMessageFromString(d, jrKanjiError);
	currentModeInfo(d);
	return 0;
      }
    }
    return showmenu(d, men->u.menu_next);
  case MENU_FUNC:
    if (men->u.fnum < 0) {
      jrKanjiError = "\244\263\244\316\271\340\314\334\244\317\300\265\244\267"
	"\244\257\304\352\265\301\244\265\244\354\244\306\244\244\244\336"
	"\244\273\244\363";                
	    /* ���ι��ܤ��������������Ƥ��ޤ��� */
      killmenu(d);
      makeGLineMessageFromString(d, jrKanjiError);
      currentModeInfo(d);
      return 0;
    }
    else {
      d->more.todo = 1;
      d->more.fnum = men->u.fnum;
      /* �ʲ��Σ��Ĥ�ɬ�פ��ɤ����ɤ�ʬ����ʤ� */
      GlineClear(d);
      echostrClear(d);
      return 0;
    }
  }
  return NothingChangedWithBeep(d); /* �����ˤ���ʤ��Ϥ� */
}

prevMenuIfExist(d)
uiContext d;
{
  menustruct *m = d->prevMenu;

  if (m) {
    d->prevMenu = m->prev;
    d->kanji_status_return->info &= ~KanjiEmptyInfo;

    return showmenu(d, m);
  }
  else {
    return 0;
  }
}

static
uuflQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* ������ pop */

  popForIchiranMode(d);
  popCallback(d);
  currentModeInfo(d);

  return prevMenuIfExist(d);
}
#endif /* NO_EXTEND_MENU */

/* cfuncdef

  UiUtilMode -- UI�桼�ƥ���ƥ��⡼�ɤˤʤ�Ȥ��˸ƤФ�롣

 */
UiUtilMode(d)
uiContext d;
{
#ifdef NO_EXTEND_MENU
  d->kanji_status_return->info |= KanjiExtendInfo;
  return 0;
#else
  return showmenu(d, me[MT_HELP]);
#endif
}

#ifndef NO_EXTEND_MENU
/*
 * newMenuInfo() -- ��������˥塼����μ���
 */

static menuinfo *
newMenuInfo(tab)
menustruct *tab;
{
  menuinfo *res;

  res = (menuinfo *)malloc(sizeof(menuinfo));
  if (res) {
    res->mstruct = tab;
    res->curnum = 0;
  }
  return res;
}

void
freeAllMenuInfo(p)
menuinfo *p;
{
  menuinfo *q;

  while (p) {
    q = p->next;
    free((char *)p);
    p = q;
  }
}

static menuinfo *
findMenuInfo(p, ms)
menuinfo *p;
menustruct *ms;
{
  while (p) {
    if (p->mstruct == ms) {
      return p;
    }
    p = p->next;
  }
  return (menuinfo *)0;
}

/*
 * showmenu -- ��˥塼��ɽ��
 *
 * ����
 *   d         : uiContext
 *   table     : ��˥塼����(menustruct �ؤΥݥ���)
 */

int
showmenu(d, table)
uiContext d;
menustruct *table;
{
  yomiContext yc = (yomiContext)d->modec;
  forichiranContext fc;
  ichiranContext ic;
  unsigned inhibit = 0;
  int retval = 0;
  menuinfo *minfo;
  int *prevcurp = (int *)0;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    

  minfo = findMenuInfo(d->minfo, table);
  if (!minfo) {
    minfo = newMenuInfo(table);
    if (minfo) {
      minfo->next = d->minfo;
      d->minfo = minfo;
    }
  }

  if (minfo) {
    prevcurp = &(minfo->curnum);
  }

  d->status = 0;

  if((retval = getForIchiranContext(d)) == NG)
     return(GLineNGReturn(d));
  fc = (forichiranContext)d->modec;

  fc->prevcurp = prevcurp;
  fc->table = table;

  /* selectOne ��Ƥ֤���ν��� */
  fc->allkouho = table->titles;
  fc->curIkouho = 0;
  if (!cannaconf.HexkeySelect)
    inhibit |= ((unsigned char)NUMBERING | (unsigned char)CHARINSERT);
  else
    inhibit |= (unsigned char)CHARINSERT;
  if((retval = selectOne(d, fc->allkouho, &fc->curIkouho, table->nentries,
			 BANGOMAX, inhibit, 0, WITHOUT_LIST_CALLBACK,
			 NO_CALLBACK, uuflExitCatch,
			 uuflQuitCatch, uiUtilIchiranTooSmall)) == NG) {
    return(GLineNGReturnFI(d));
  }

  ic = (ichiranContext)d->modec;
  ic->majorMode = CANNA_MODE_ExtendMode;
  ic->minorMode = table->modeid;
  currentModeInfo(d);

  if (prevcurp) {
    *(ic->curIkouho) = *prevcurp;
  }
  else {
    *(ic->curIkouho) = 0;
  }

  /* ��������Ԥ������Ƹ���������Ф��ʤ� */
  if(ic->tooSmall) {
    d->status = AUX_CALLBACK;
    return(retval);
  }

  makeGlineStatus(d);
  /* d->status = ICHIRAN_EVERYTIME; */

  return(retval);
}
#endif /* NO_EXTEND_MENU */

#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/
