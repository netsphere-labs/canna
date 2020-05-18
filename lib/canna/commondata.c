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
static char rcs_id[] = "@(#) 102.1 $Id: commondata.c,v 1.4.2.1 2004/04/26 22:53:02 aida_s Exp $";
#endif /* lint */

#include "canna.h"
#include <canna/mfdef.h>
#include "patchlevel.h"

struct CannaConfig cannaconf;
     
/* �ǥե���ȤΥ��޻������Ѵ��ѤΥХåե� */

int defaultContext = -1;
int defaultBushuContext = -1;

/* ���޻������Ѵ��ơ��֥� */
/*
 * ���޻������Ѵ��ơ��֥�ϣ��Ĥ�����ɤ��Ǥ��礦��ʣ����ɬ�פʤΤ�
 * ����� RomeStruct �Υ��ФȤ�������Ƥ���ɬ�פ⤢��ޤ��礦��...��
 * �λ��Ϥ��λ��ǹͤ��ޤ��礦��
 */
     
struct RkRxDic *romajidic, *englishdic, *RkwOpenRoma();

/* ̤��������Ǹ����ν����Τ����� */

int howToBehaveInCaseOfUndefKey = kc_normal;

/*
 * �����̾��������Ƥ����ѿ�
 */

char saveapname[CANNA_MAXAPPNAME]; /* �����ФȤ���³���ڤ�Ȥ���AP̾ */

/*
 * irohacheck ���ޥ�ɤˤ�äƻȤ��Ƥ��뤫�Ȥ���
 * irohacheck �ʤ��Ǥ� verbose ��ɽ���͡�
 */

int ckverbose = 0;

/*
 * ���顼�Υ�å�����������Ƥ����ѿ�
 */

char *jrKanjiError = "";

/*
 * �ǥХ���å�������Ф����ɤ����Υե饰
 */

int iroha_debug = 0;

/*
 * �Ϥ�Ƥλ��Ѥ��ɤ����򼨤��ե饰
 */

int FirstTime = 1;

/*
 * �ӡ��ײ����Ĥ餹�ؿ����Ǽ����Ȥ���
 */

int (*jrBeepFunc)() = (int (*)())NULL;

/*
 * KC_INITIALIZE ľ��˼¹Ԥ��뵡ǽ����
 */

BYTE *initfunc = (BYTE *)0;
int howToReturnModeInfo = ModeInfoStyleIsString;
char *RomkanaTable = (char *)NULL;
char *EnglishTable = (char *)NULL;
/* char *Dictionary = (char *)NULL; */
struct dicname *RengoGakushu = (struct dicname *)NULL;
struct dicname *KatakanaGakushu = (struct dicname *)NULL;
struct dicname *HiraganaGakushu = (struct dicname *)NULL;

int nKouhoBunsetsu = 16;

int KeepCursorPosition = 0;

int nothermodes = 0;

keySupplement keysup[MAX_KEY_SUP];
int nkeysup = 0;

/*
 * ������κݻ��Ѥ���������ե�����̾�����ƤȤäƤ����Хåե���
 * �ե�����̾��","�Ƕ��ڤ��롣(��ĥ��ǽ�ǻ���)
 */

char *CANNA_initfilename = (char *)NULL;

/*
 * �С������
 */

int protocol_version = -1;
int server_version = -1;
char *server_name = (char *)NULL;

int chikuji_debug = 0;
int auto_define = 0;

void (*keyconvCallback)() = (void (*)())0;

extraFunc *extrafuncp = (extraFunc *)NULL;
struct dicname *kanjidicnames; /* .canna �ǻ��ꤷ�Ƥ��뼭��ꥹ�� */
char *kataautodic = (char *)NULL; /* �������ʸ켫ư��Ͽ�Ѽ��� */
#ifdef HIRAGANAAUTO
char *hiraautodic = (char *)NULL; /* �Ҥ餬�ʸ켫ư��Ͽ�Ѽ��� */
#endif

/* �桼������ */
jrUserInfoStruct *uinfo = (jrUserInfoStruct *)NULL;

 /* �ޥ���Ƚ�����ԤäƤ��뤫�ɤ��� */
int mountnottry = 1;

void
InitCannaConfig(cf)
struct CannaConfig *cf;
{
  bzero(cf, sizeof(struct CannaConfig));
  cf->CannaVersion = CANNA_MAJOR_MINOR;
  cf->kouho_threshold = 2;
  cf->strokelimit = STROKE_LIMIT;
  cf->CursorWrap = 1;
  cf->SelectDirect = 1;
  cf->HexkeySelect = 1;
  cf->ChBasedMove = 1;
  cf->Gakushu = 1;
  cf->grammaticalQuestion = 1;
  cf->stayAfterValidate = 1;
  cf->kCount = 1;
  cf->hexCharacterDefiningStyle = HEX_USUAL;
  cf->ChikujiContinue = 1;
  cf->RenbunContinue = 1;
  cf->MojishuContinue = 1;
  cf->kojin = 1;
  cf->indexSeparator = DEFAULTINDEXSEPARATOR;
  cf->allowNextInput = 1;
  cf->chikujiRealBackspace = 1;
  cf->BackspaceBehavesAsQuit = 1;
  cf->doKatakanaGakushu = 1;
  cf->doHiraganaGakushu = 1;
  cf->auto_sync = 1;
}

static void freeUInfo pro((void));

static void
freeUInfo()
{
  if (uinfo) {
    if (uinfo->uname)
      free(uinfo->uname);
    if (uinfo->gname)
      free(uinfo->gname);
    if (uinfo->srvname)
      free(uinfo->srvname);
    if (uinfo->topdir)
      free(uinfo->topdir);
    if (uinfo->cannafile)
      free(uinfo->cannafile);
    if (uinfo->romkanatable)
      free(uinfo->romkanatable);
    free((char *)uinfo);
    uinfo = (jrUserInfoStruct *)NULL;
  }
}

/*
  �ǥե�������ͤˤ�ɤ���
*/
void
restoreBindings()
{
  InitCannaConfig(&cannaconf);

  if (initfunc) free(initfunc);
  initfunc = (BYTE *)NULL;

  if (server_name) free(server_name);
  server_name = (char *)NULL;

  if (RomkanaTable) {
    free(RomkanaTable);
    RomkanaTable = (char *)NULL;
  }
  if (EnglishTable) {
    free(EnglishTable);
    EnglishTable = (char *)NULL;
  }
  romajidic = (struct RkRxDic *)NULL;
  englishdic = (struct RkRxDic *)NULL;
  RengoGakushu = (struct dicname *)NULL;
  KatakanaGakushu = (struct dicname *)NULL;
  HiraganaGakushu = (struct dicname *)NULL;
  howToBehaveInCaseOfUndefKey = kc_normal;
/*  kanjidicname[nkanjidics = 0] = (char *)NULL; ����Τ��Ȥ򤷤ʤ���� */
  kanjidicnames = (struct dicname *)NULL;
  kataautodic = (char *)NULL;
#ifdef HIRAGANAAUTO
  hiraautodic = (char *)NULL;
#endif
  auto_define = 0;
  saveapname[0] = '\0';
  KeepCursorPosition = 0;

  nothermodes = 0;
  protocol_version = server_version = -1;
  nKouhoBunsetsu = 16;
  nkeysup = 0;
  chikuji_debug = 0;
  keyconvCallback = (void (*)())0;
  freeUInfo();
}
