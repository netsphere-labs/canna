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
static char rcs_id[] = "@(#) 102.1 $Id: empty.c,v 1.2 2003/09/17 08:50:53 aida_s Exp $";
#endif /* lint */

#include "canna.h"
#include "patchlevel.h"

extern KanjiModeRec yomi_mode, cy_mode;

/* EmptySelfInsert -- ��ʬ���Ȥ����ʸ����Ȥ����֤��ؿ���
 * 
 */

static
inEmptySelfInsert(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;
  int res = 0;

  d->kanji_status_return->info |= KanjiThroughInfo | KanjiEmptyInfo;
  if (!(yc->generalFlags & CANNA_YOMI_END_IF_KAKUTEI)) {
    res = d->nbytes;
  }
  /* else { ����ǡ����������ԤäƤ���ͤˤϳ���ǡ������Ϥ��ʤ� } */

  return res;
}

static EmptySelfInsert pro((uiContext));

static
EmptySelfInsert(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;
  int res = inEmptySelfInsert(d);

/* ñ����Ͽ�ΤȤ��� yomi mode �γ��ꥭ���� empty mode �Ǥϳ��ꥭ���Ǥ�
   ���ä��ꤹ��ȡ����Υ����β����ǻ��Ǥ��ޤä��ꤹ��Τεߺѡ�yomi
   mode �ξ�� yomi mode ����äƤ���Τ�ñ����Ͽ�λ����餤�����ȸ�
   �����Ȥ�Ƚ�Ǥκ����ˤ��Ƥ��롣�����Ϥ���ʤ��Ȥ�ꤿ���ʤ��� */

  if (yc->next && yc->next->id == YOMI_CONTEXT &&
      yomi_mode.keytbl[d->buffer_return[0]] == CANNA_FN_Kakutei) {
    d->status = EXIT_CALLBACK;
    if (d->cb->func[EXIT_CALLBACK] != NO_CALLBACK) {
      d->kanji_status_return->info &= ~KanjiThroughInfo; /* �Ż����� */
      popYomiMode(d);
    }
  }
  return res;
}

/* EmptyYomiInsert -- ���⡼�ɤ˰ܹԤ����ɤߤ����Ϥ���ؿ�
 *
 */

static EmptyYomiInsert pro((uiContext));

static
EmptyYomiInsert(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  d->current_mode = (yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) ?
    &cy_mode : &yomi_mode;
  RomajiClearYomi(d);
  return YomiInsert(d); /* ������Хå��Υ����å��� YomiInsert �Ǥ���� */
}

/* EmptyQuotedInsert -- ���ΰ�����ɤΤ褦��ʸ���Ǥ⥹�롼���̤��ؿ���
 *
 */

/* 
  Empty �⡼�ɤǤ� quotedInset �� ^Q �Τ褦��ʸ������� Emacs �ʤɤ���
  ���̤äƤ��ޤ��Хޥåפ��֤��Ƥ��ޤ��Τǡ����ʴ����Ѵ������ǲ�����
  ����ʤ�Ƥ��Ȥ�ɬ�פʤ��ΤǤϤʤ��Τ��ʤ���
 */

static EmptyQuotedInsert pro((uiContext));

static
EmptyQuotedInsert(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  d->current_mode = (yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) ?
    &cy_mode : &yomi_mode;
  return YomiQuotedInsert(d);
}

/* 
  AlphaSelfInsert -- ��ʬ���Ȥ����ʸ����Ȥ����֤��ؿ���
 */

static AlphaSelfInsert pro((uiContext));

static
AlphaSelfInsert(d)
uiContext d;
{
  unsigned kanap = (unsigned)d->ch;

  d->kanji_status_return->length = 0;
  d->kanji_status_return->info |= KanjiEmptyInfo;
  d->kanji_status_return->info |= KanjiThroughInfo;
  if ( d->nbytes != 1 || kanap <= 0xa0 || 0xe0 <= kanap ) {
    return d->nbytes;
  }
  else { /* ��̾�������Ϥξ�� */
    if (d->n_buffer > 1) {
      return 1;
    }
    else {
      return 0;
    }
  }
}

static AlphaNop pro((uiContext));

static
AlphaNop(d)
uiContext d;
{
  /* currentModeInfo �ǥ⡼�ɾ���ɬ���֤�褦�˥��ߡ��Υ⡼�ɤ�����Ƥ��� */
  d->majorMode = d->minorMode = CANNA_MODE_KigoMode;
  currentModeInfo(d);
  return 0;
}

static EmptyQuit pro((uiContext));

static
EmptyQuit(d)
uiContext d;
{
  int res;

  res = inEmptySelfInsert(d);
  d->status = QUIT_CALLBACK;
  if (d->cb->func[QUIT_CALLBACK] != NO_CALLBACK) {
    d->kanji_status_return->info &= ~KanjiThroughInfo; /* �Ż����� */
    popYomiMode(d);
  }
  return res;
}

static EmptyKakutei pro((uiContext));

static
EmptyKakutei(d)
uiContext d;
{
  int res;

  res = inEmptySelfInsert(d);
  d->status = EXIT_CALLBACK;
  if (d->cb->func[EXIT_CALLBACK] != NO_CALLBACK) {
    d->kanji_status_return->info &= ~KanjiThroughInfo; /* �Ż����� */
    popYomiMode(d);
  }
  return res;
}

static EmptyDeletePrevious pro((uiContext));

static
EmptyDeletePrevious(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_DELETE_DONT_QUIT) {
    /* Delete �� QUIT ���ʤ��ΤǤ���С�selfInsert */
    return inEmptySelfInsert(d);
  }
  else {
    return EmptyQuit(d);
  }
}

extraFunc *
FindExtraFunc(fnum)
int fnum;
{
  extern extraFunc *extrafuncp;
  extraFunc *extrafunc;

  for (extrafunc = extrafuncp; extrafunc; extrafunc = extrafunc->next) {
    if (extrafunc->fnum == fnum) {
      return extrafunc;
    }
  }
  return (extraFunc *)0;
}

static
UserMode(d, estruct)
uiContext d;
extraFunc *estruct;
{
  newmode *nmode = estruct->u.modeptr;
  yomiContext yc = (yomiContext)d->modec;
  int modeid
    = estruct->fnum - CANNA_FN_MAX_FUNC + CANNA_MODE_MAX_IMAGINARY_MODE;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }

  yc->generalFlags &= ~CANNA_YOMI_ATTRFUNCS;
  yc->generalFlags |= nmode->flags;
  if (yc->generalFlags & CANNA_YOMI_END_IF_KAKUTEI) {
    /* ����ǽ����褦�ʥ⡼�ɤ��ä������⡼�ɤˤʤ�ʤ� */
    yc->generalFlags &= ~CANNA_YOMI_KAKUTEI;
  }
  yc->romdic = nmode->romdic;
  d->current_mode = yc->myEmptyMode = nmode->emode;

  yc->majorMode = yc->minorMode = yc->myMinorMode = (BYTE)modeid;

  currentModeInfo(d);

  d->kanji_status_return->length = 0;
  return 0;
}

#ifndef NO_EXTEND_MENU /* continues to the bottom of this file */
static
UserSelect(d, estruct)
uiContext d;
extraFunc *estruct;
{
  int curkigo = 0, *posp = (int *)0;
  kigoIchiran *kigop = (kigoIchiran *)0;
  extern int uuKigoGeneralExitCatch(), uuKigoMake();
  selectinfo *selinfo = (selectinfo *)0, *info;
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  info = d->selinfo;
  while (info) {
    if (info->ichiran == estruct->u.kigoptr) {
      selinfo = info;
      break;
    }
    info = info->next;
  }

  if (!selinfo) {
    selinfo = (selectinfo *)malloc(sizeof(selectinfo));
    /* malloc �˼��Ԥ������ϡ��������򤷤��ֹ椬��¸����ʤ� */
    if (selinfo) {
      selinfo->ichiran = estruct->u.kigoptr;
      selinfo->curnum = 0;
      selinfo->next = d->selinfo;
      d->selinfo = selinfo;
    }
  }

  if (selinfo) {
    curkigo = selinfo->curnum;
    posp = &selinfo->curnum;
  }

  kigop = estruct->u.kigoptr;
  if (!kigop) {
    return NothingChangedWithBeep(d);
  }
  return uuKigoMake(d, kigop->kigo_data, kigop->kigo_size, 
                    curkigo, kigop->kigo_mode, uuKigoGeneralExitCatch, posp);
}
  
static
UserMenu(d, estruct)
uiContext d;
extraFunc *estruct;
{
  return showmenu(d, estruct->u.menuptr);
}
#endif /* NO_EXTEND_MENU */

/* �ǥե���Ȱʳ��Υ⡼�ɻ��ѻ��˸ƤӽФ��ؿ����ڤ�ʬ���� */

static
ProcExtraFunc(d, fnum)
uiContext d;
int fnum;
{
  extraFunc *extrafunc;

  extrafunc = FindExtraFunc(fnum);
  if (extrafunc) {
    switch (extrafunc->keyword) {
      case EXTRA_FUNC_DEFMODE:
        return UserMode(d, extrafunc);
#ifndef NO_EXTEND_MENU
      case EXTRA_FUNC_DEFSELECTION:
        return UserSelect(d, extrafunc);
      case EXTRA_FUNC_DEFMENU:
        return UserMenu(d, extrafunc);
#endif
      default:
        break;
    }
  }
  return NothingChangedWithBeep(d);
}

getBaseMode(yc)
yomiContext yc;
{
  int res;
  long fl = yc->generalFlags;

  if (yc->myMinorMode) {
    return yc->myMinorMode;
  }
  else if (fl & CANNA_YOMI_ROMAJI) {
    res = CANNA_MODE_ZenAlphaHenkanMode;
  }
  else if (fl & CANNA_YOMI_KATAKANA) {
    res = CANNA_MODE_ZenKataHenkanMode;
  }
  else {
    res = CANNA_MODE_ZenHiraHenkanMode;
  }
  if (fl & CANNA_YOMI_BASE_HANKAKU) {
    res++;
  }
  if (fl & CANNA_YOMI_KAKUTEI) {
    res += CANNA_MODE_ZenHiraKakuteiMode - CANNA_MODE_ZenHiraHenkanMode;
  }
  if (res == CANNA_MODE_ZenHiraHenkanMode) {
    if (chikujip(yc)) {
      res = CANNA_MODE_ChikujiYomiMode;
    }
    else {
      res = CANNA_MODE_HenkanMode;
    }
  }
  return res;
}

/* �١���ʸ�����ڤ��ؤ� */

void
EmptyBaseModeInfo(d, yc)
uiContext d;
yomiContext yc;
{
  coreContext cc = (coreContext)d->modec;

  cc->minorMode = getBaseMode(yc);
  currentModeInfo(d);
}

EmptyBaseHira(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }
  yc->generalFlags &= ~(CANNA_YOMI_KATAKANA | CANNA_YOMI_HANKAKU |
			CANNA_YOMI_ROMAJI | CANNA_YOMI_ZENKAKU);
  EmptyBaseModeInfo(d, yc);
  return 0;
}

EmptyBaseKata(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if ((yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED)
      || (cannaconf.InhibitHankakuKana
	  && (yc->generalFlags & CANNA_YOMI_BASE_HANKAKU))) {
    return NothingChangedWithBeep(d);
  }
  yc->generalFlags &= ~(CANNA_YOMI_ROMAJI | CANNA_YOMI_ZENKAKU);
  yc->generalFlags |= CANNA_YOMI_KATAKANA |
    ((yc->generalFlags & CANNA_YOMI_BASE_HANKAKU) ? CANNA_YOMI_HANKAKU : 0);
  EmptyBaseModeInfo(d, yc);
  return 0;
}

EmptyBaseEisu(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }
/*  yc->generalFlags &= ~CANNA_YOMI_ATTRFUNCS; ���ꥢ����Τ�� */
  yc->generalFlags |= CANNA_YOMI_ROMAJI |
    ((yc->generalFlags & CANNA_YOMI_BASE_HANKAKU) ? 0 : CANNA_YOMI_ZENKAKU);
  EmptyBaseModeInfo(d, yc);
  return 0;
}

EmptyBaseZen(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }
  yc->generalFlags &= ~CANNA_YOMI_BASE_HANKAKU;
  if (yc->generalFlags & CANNA_YOMI_ROMAJI) {
    yc->generalFlags |= CANNA_YOMI_ZENKAKU;
  }
  /* ���� ���޻��⡼�ɤǤ��ĥ������ʥ⡼�ɤλ�������
          (���ξ��ɽ�̾�ϥ��޻��⡼��) */
  if (yc->generalFlags & CANNA_YOMI_KATAKANA) {
    yc->generalFlags &= ~CANNA_YOMI_HANKAKU;
  }
  EmptyBaseModeInfo(d, yc);
  return 0;
}

EmptyBaseHan(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if ((yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) ||
      /* �⡼���ѹ����ػߤ���Ƥ���Ȥ� */
      (cannaconf.InhibitHankakuKana &&
       (yc->generalFlags & CANNA_YOMI_KATAKANA) &&
       !(yc->generalFlags & CANNA_YOMI_ROMAJI))) {
    /* Ⱦ�ѥ��ʤ��ػߤ���Ƥ���Τ�Ⱦ�ѥ��ʤˤ��������ʤȤ� */
    return NothingChangedWithBeep(d);
  }
  yc->generalFlags |= CANNA_YOMI_BASE_HANKAKU;
  if (yc->generalFlags & CANNA_YOMI_ROMAJI) {
    yc->generalFlags &= ~CANNA_YOMI_ZENKAKU;
  }
  /* ���� ���޻��⡼�ɤǤ��ĥ������ʥ⡼�ɤλ�������
          (���ξ��ɽ�̾�ϥ��޻��⡼��) */
  if (yc->generalFlags & CANNA_YOMI_KATAKANA) {
    if (!cannaconf.InhibitHankakuKana) {
      yc->generalFlags |= CANNA_YOMI_HANKAKU;
    }
  }
  EmptyBaseModeInfo(d, yc);
  return 0;
}

EmptyBaseKana(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if ((yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) ||
      /* �⡼���ѹ����ػߤ���Ƥ����� */
      (!cannaconf.InhibitHankakuKana &&
       (yc->generalFlags & CANNA_YOMI_KATAKANA) &&
       (yc->generalFlags & CANNA_YOMI_BASE_HANKAKU))) {
    /* Ⱦ�ѥ��ʤ��ػߤ���Ƥ���Τ�Ⱦ�ѥ��ʤˤʤäƤ��ޤ������ʾ�� */
    return NothingChangedWithBeep(d);
  }
  yc->generalFlags &= ~(CANNA_YOMI_ROMAJI | CANNA_YOMI_ZENKAKU);

  if ((yc->generalFlags & CANNA_YOMI_BASE_HANKAKU) &&
      (yc->generalFlags & CANNA_YOMI_KATAKANA)) {
    yc->generalFlags |= CANNA_YOMI_HANKAKU;
  }
  EmptyBaseModeInfo(d, yc);
  return 0;
}

EmptyBaseKakutei(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }
  yc->generalFlags |= CANNA_YOMI_KAKUTEI;

  EmptyBaseModeInfo(d, yc);
  return 0;
}

EmptyBaseHenkan(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }
  yc->generalFlags &= ~CANNA_YOMI_KAKUTEI;

  EmptyBaseModeInfo(d, yc);
  return 0;
}

#ifndef NO_EXTEND_MENU
static int
renbunInit(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  d->status = 0;
  killmenu(d);
  if (ToggleChikuji(d, 0) == -1) {
    jrKanjiError = "\317\242\312\270\300\341\312\321\264\271\244\313\300\332"
	"\302\330\244\250\244\353\244\263\244\310\244\254\244\307\244\255"
	"\244\336\244\273\244\363";
                   /* Ϣʸ���Ѵ������ؤ��뤳�Ȥ��Ǥ��ޤ��� */
    makeGLineMessageFromString(d, jrKanjiError);
    currentModeInfo(d);
    return(-1);
  }
  else {
    makeGLineMessageFromString(d, "\317\242\312\270\300\341\312\321\264\271"
	"\244\313\300\332\302\330\244\250\244\336\244\267\244\277");
                   /* Ϣʸ���Ѵ������ؤ��ޤ��� */
    currentModeInfo(d);
    return 0;
  }
}

static int
showVersion(d)
uiContext d;
{
  int retval = 0;
  char s[512];
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  d->status = 0;
  killmenu(d);

  sprintf(s, "\306\374\313\334\270\354\306\376\316\317\245\267\245\271\245\306"
	"\245\340\241\330\244\253\244\363\244\312\241\331Version %d.%d",
	  cannaconf.CannaVersion / 1000, cannaconf.CannaVersion % 1000);
             /* ���ܸ����ϥ����ƥ�ؤ���ʡ� */
  strcat(s, CANNA_PATCH_LEVEL);
  makeGLineMessageFromString(d, s);
  currentModeInfo(d);

  return (retval);
}

static int
showServer(d)
uiContext d;
{
#ifndef STANDALONE /* This is not used in Windows environment 1996.7.30 kon */
  int retval = 0;
  char s[512];
  extern defaultContext;
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  d->status = 0;
  killmenu(d);

  if(defaultContext == -1) {
    sprintf(s, "\244\253\244\312\264\301\273\372\312\321\264\271\245\265"
	"\241\274\245\320\244\310\244\316\300\334\302\263\244\254\300\332"
	"\244\354\244\306\244\244\244\336\244\271");
               /* ���ʴ����Ѵ������ФȤ���³���ڤ�Ƥ��ޤ� */
  }
  else {
    sprintf(s, "%s \244\316\244\253\244\312\264\301\273\372\312\321\264\271"
	"\245\265\241\274\245\320\244\313\300\334\302\263\244\267\244\306"
	"\244\244\244\336\244\271", RkwGetServerName());
               /* �Τ��ʴ����Ѵ������Ф���³���Ƥ��ޤ� */
  }
  makeGLineMessageFromString(d, s);
  currentModeInfo(d);

  return (retval);
#else
  return (0);
#endif /* STANDALONE */
}

static int
showGakushu(d)
uiContext d;
{
  int retval = 0;
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  d->status = 0;
  killmenu(d);
  
  if (cannaconf.Gakushu == 1) {
    makeGLineMessageFromString(d, "\263\330\275\254\244\254\243\317\243\316"
	"\244\316\276\365\302\326\244\307\244\271");
                                  /* �ؽ����ϣΤξ��֤Ǥ� */
  }
  else {
    makeGLineMessageFromString(d, "\263\330\275\254\244\254\243\317\243\306"
	"\243\306\244\316\276\365\302\326\244\307\244\271");
                                  /* �ؽ����ϣƣƤξ��֤Ǥ� */
  }
    currentModeInfo(d);

  return (retval);
}

static int
showInitFile(d)
uiContext d;
{
  int retval = 0;
  char s[512];
  extern char *CANNA_initfilename;
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  d->status = 0;
  killmenu(d);

  if (CANNA_initfilename && strlen(CANNA_initfilename)) {
    sprintf(s, "\245\253\245\271\245\277\245\336\245\244\245\272\245\325"
	"\245\241\245\244\245\353\244\317 %s \244\362\273\310\315\321\244\267"
	"\244\306\244\244\244\336\244\271", CANNA_initfilename);
               /* �������ޥ����ե������ %s ����Ѥ��Ƥ��ޤ� */
    makeGLineMessageFromString(d, s);
  }
  else {
    sprintf(s, "\245\253\245\271\245\277\245\336\245\244\245\272\245\325"
	"\245\241\245\244\245\353\244\317\300\337\304\352\244\265\244\354"
	"\244\306\244\244\244\336\244\273\244\363");
               /* �������ޥ����ե���������ꤵ��Ƥ��ޤ��� */
    makeGLineMessageFromString(d, s);
  }
    currentModeInfo(d);

  return (retval);
}

static int
showRomkanaFile(d)
uiContext d;
{
  int retval = 0;
  char s[512];
  extern char *RomkanaTable;
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  d->status = 0;
  killmenu(d);
  
  if (RomkanaTable && romajidic) {
    sprintf(s, "\245\355\241\274\245\336\273\372\244\253\244\312\312\321"
	"\264\271\245\306\241\274\245\326\245\353\244\317 %s \244\362\273\310"
	"\315\321\244\267\244\306\244\244\244\336\244\271",
	    RomkanaTable);
               /* ���޻������Ѵ��ơ��֥�� %s ����Ѥ��Ƥ��ޤ� */
    makeGLineMessageFromString(d, s);
  }
  else {
    sprintf(s, "\245\355\241\274\245\336\273\372\244\253\244\312\312\321"
	"\264\271\245\306\241\274\245\326\245\353\244\317\273\310\315\321"
	"\244\265\244\354\244\306\244\244\244\336\244\273\244\363");
               /* ���޻������Ѵ��ơ��֥�ϻ��Ѥ���Ƥ��ޤ��� */
    makeGLineMessageFromString(d, s);
  }
    currentModeInfo(d);

  return (retval);
}

static int
dicSync(d)
uiContext d;
{
  int retval = 0;
  char s[512];
  extern defaultContext;
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }    
  d->status = 0;
  killmenu(d);

  retval = RkwSync(defaultContext, "");
  sprintf(s, "\274\255\275\361\244\316 Sync \275\350\315\375%s",
          retval < 0 ? "\244\313\274\272\307\324\244\267\244\336\244\267"
	"\244\277" : "\244\362\271\324\244\244\244\336\244\267\244\277");
          /* ����� Sync ����%s",
                retval < 0 ? "�˼��Ԥ��ޤ���" : "��Ԥ��ޤ��� */
  makeGLineMessageFromString(d, s);
  currentModeInfo(d);

  return 0;
}
#endif /* not NO_EXTEND_MENU */

#include "emptymap.h"
#include "alphamap.h"
