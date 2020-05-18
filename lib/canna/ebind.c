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
static char rcsid[] = "$Id: ebind.c,v 1.6 2003/09/17 08:50:53 aida_s Exp $";
#endif

#include "canna.h"

#define MAX_BYTE_PER_CHAR 4

/*********************************************************************
 *                      wchar_t replace begin                        *
 *********************************************************************/
#ifdef wchar_t
# error "wchar_t is already defined"
#endif
#define wchar_t cannawc

extern int howToReturnModeInfo;

static wchar_t *inbuf = 0;
static int inbufsize = 0;

static
StoreWCtoEUC(wbuf, wbuflen, wks, ebuf, maxebuf, ks, ch, nbytes)
wchar_t *wbuf;
int wbuflen;
wcKanjiStatus *wks;
char *ebuf;
int maxebuf;
jrKanjiStatus *ks;
int ch;
int nbytes;
{
  int ret, rest, totallen = 0, len;
  char *p;

  /* info */

  ks->info = wks->info;
    
  /* ��� */

  if (ks->info & KanjiThroughInfo) {
    if (nbytes) {
      ebuf[0] = ch;
    }
    ret = nbytes;
  }
  else {
    ret = (wbuflen > 0) ? WCstombs(ebuf, wbuf, maxebuf) : 0;
    if (ks->info & KanjiYomiInfo) {
      wchar_t *ep;
      len = WCstombs(ebuf + ret + 1, wbuf + wbuflen + 1,
		     maxebuf - ret - 1);
      ep = wbuf + wbuflen + 1;
      while (*ep) ep++;
      WCstombs(ebuf + ret + 1 + len + 1, ep + 1,
	       maxebuf - ret - 1 - len - 1);
    }
  }

  if (wks->length > 0) {
    totallen = wks->length;
  }
  if (wks->info & KanjiModeInfo) {
    totallen += WStrlen(wks->mode);
  }
  if (wks->info & KanjiGLineInfo) {
    totallen += wks->gline.length;
  }

  if (inbufsize < totallen) {
    inbufsize = totallen; /* inbufsize will be greater than 0 */
    if (inbuf) free(inbuf);
    inbuf = (wchar_t *)malloc(inbufsize * sizeof(wchar_t));
    if (!inbuf) {
      inbufsize = 0;
      jrKanjiError = "\245\341\245\342\245\352\244\254\302\255\244\352\244\336\244\273\244\363";
                     /* ���꤬­��ޤ��� */
      return -1;
    }
  }

  rest = inbufsize * sizeof(wchar_t);
  p = (char *)inbuf;

  if (wks->length < 0) {
    ks->length = -1;
  }
  else {
    /* ������ʸ�� */

    ks->length = ks->revLen = ks->revPos = 0;

    if (wks->length > 0) {
      ks->echoStr = (unsigned char *)p;
      if (wks->revPos > 0) {
	len = ks->revPos = CNvW2E(wks->echoStr, wks->revPos, p, rest);
	p += len;
	rest -= len;
      }
      if (wks->revLen > 0) {
	len = ks->revLen 
	  = CNvW2E(wks->echoStr + wks->revPos, wks->revLen, p, rest);
	p += len;
	rest -= len;
      }
      len = 0;
      if (wks->length - wks->revPos - wks->revLen > 0) {
	len = CNvW2E(wks->echoStr + wks->revPos + wks->revLen,
		     wks->length - wks->revPos - wks->revLen, p, rest);
	p += len;
	rest -= len;
      }
      ks->length = ks->revLen + ks->revPos + len;
      *p++ = '\0';
      rest--;
    }
  }

  /* �⡼��ɽ�� */

  if (wks->info & KanjiModeInfo) {
    len = WCstombs(p, wks->mode, rest);
    ks->mode = (unsigned char *)p;
    p[len] = '\0';
    p += len + 1;
    rest -= len + 1;
  }

  /* ������ɽ�� */

  if (wks->info & KanjiGLineInfo) {
    ks->gline.length = ks->gline.revLen = ks->gline.revPos = 0;

    if (wks->gline.length > 0) {
      ks->gline.line = (unsigned char *)p;
      if (wks->gline.revPos > 0) {
	len = ks->gline.revPos 
	  = CNvW2E(wks->gline.line, wks->gline.revPos, p, rest);
	p += len;
	rest -= len;
      }
      if (wks->gline.revLen > 0) {
	len = ks->gline.revLen
	  = CNvW2E(wks->gline.line + wks->gline.revPos, wks->gline.revLen,
		   p, rest);
	p += len;
	rest -= len;
      }
      len = 0;
      if (wks->gline.length - wks->gline.revPos - wks->gline.revLen > 0) {
	len = CNvW2E(wks->gline.line + wks->gline.revPos +
		     wks->gline.revLen,
		     wks->gline.length -
		     wks->gline.revPos - wks->gline.revLen,
		     p, rest);
	p += len;
	rest -= len;
      }
      ks->gline.length = ks->gline.revLen + ks->gline.revPos + len;
      *p++ = '\0';
      rest--;
    }
  }
  return ret;
}

XLookupKanji2(dpy, win, buffer_return, bytes_buffer, nbytes, functionalChar,
	      kanji_status_return)
unsigned int dpy, win;
int functionalChar, nbytes;
char *buffer_return;
int bytes_buffer;
jrKanjiStatus *kanji_status_return;
{
  int ret;
  wcKanjiStatus wks;
  int ch;
  int i;

  /* �����Хåե��򥢥����Ȥ��� */
  if (inbufsize < bytes_buffer) {
    inbufsize = bytes_buffer; /* inbufsize will be greater than 0 */
    if (inbuf) free(inbuf);
    inbuf = (wchar_t *)malloc(inbufsize * sizeof(wchar_t));
    if (!inbuf) {
      inbufsize = 0;
      jrKanjiError = "\245\341\245\342\245\352\244\254\302\255\244\352\244\336\244\273\244\363";
                     /* ���꤬­��ޤ��� */
      return -1;
    }
  }

  inbuf[0] = (wchar_t)(unsigned char)buffer_return[0];
  for (i = 1 ; i < nbytes ; i++) {
    inbuf[i] = (wchar_t)(unsigned char)buffer_return[i];
  }
  ch = buffer_return[0] & 0xff;
  ret = XwcLookupKanji2(dpy, win, inbuf, inbufsize, nbytes, functionalChar,
			&wks);
  if (ret >= inbufsize)
    ret = inbufsize - 1;
  inbuf[ret] = (wchar_t)0;

  return StoreWCtoEUC(inbuf, ret, &wks,
		      (char *)buffer_return, bytes_buffer, kanji_status_return,
		      ch, nbytes);
}
		      
int
EUCListCallback(client_data, func, items, nitems, cur_item)
char *client_data;
int func;
wchar_t **items;
int nitems, *cur_item;
{
  const jrEUCListCallbackStruct *elistcb;
  int r = -1;
  char **eitems = NULL;
  char *ebuf = NULL;
  char *ep;
  size_t buflen = 0;
  int i;

  elistcb = (const jrEUCListCallbackStruct *)client_data;
  if (!items) /* CANNA_LIST_Insert sets 'nitems' to the pressed key (!=0) */
    return elistcb->callback_func(elistcb->client_data,
	func, NULL, nitems, cur_item);
  for (i = 0; i < nitems; i++) {
    /* EUC(����3�Х���) + ��ü�̥� */
    buflen += WStrlen(items[i]) * 3 + 1;
  }
  ebuf = (char *)malloc(buflen);
  eitems = (char **)malloc((nitems + 1) * sizeof(char **));
  if (!ebuf || !eitems)
    goto last;	/* XXX: ñ��-1���֤��Ƥ����Τ��� */
  ep = ebuf;
  for (i = 0; i < nitems; i++) {
    size_t len = WCstombs(ep, items[i], ebuf + buflen - ep);
    eitems[i] = ep;
    ep += len + 1;  /* �Хåե��Ͼ��­��Ƥ��ƥ̥뽪ü������ */
  }
  eitems[nitems] = NULL;
  r = elistcb->callback_func(elistcb->client_data,
      func, eitems, nitems, cur_item);
last:
  free(ebuf);
  free(eitems);
  return r;
}

int
XKanjiControl2(display, window, request, arg)
unsigned int display, window, request;
BYTE *arg;
{
  int ret = -1, len1, len2;
  wcKanjiStatusWithValue wksv;
  wcKanjiStatus wks;
  jrListCallbackStruct list_cb;
  int ch;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  wchar_t arg2[256];
  wchar_t wbuf[320], wbuf1[320], wbuf2[320];
#else
  wchar_t *arg2, *wbuf, *wbuf1, *wbuf2;
  arg2 = (wchar_t *)malloc(sizeof(wchar_t) * 256);
  wbuf = (wchar_t *)malloc(sizeof(wchar_t) * 320);
  wbuf1 = (wchar_t *)malloc(sizeof(wchar_t) * 320);
  wbuf2 = (wchar_t *)malloc(sizeof(wchar_t) * 320);
  if (!arg2 || !wbuf || !wbuf1 || !wbuf2) {
    if (arg2) {
      (void)free((char *)arg2);
    }
    if (wbuf) {
      (void)free((char *)wbuf);
    }
    if (wbuf1) {
      (void)free((char *)wbuf1);
    }
    if (wbuf2) {
      (void)free((char *)wbuf2);
    }
    return ret;
  }
#endif

  wksv.buffer = wbuf;
  wksv.n_buffer = 320;
  wksv.ks = &wks;

  switch (request) {
  case KC_DO: /* val �� buffer_return ������륿���� */
    wbuf[0] = ((jrKanjiStatusWithValue *)arg)->buffer[0];
    /* ����³�� */
  case KC_CHANGEMODE: /* val ��Ϳ���륿���� */
    wksv.val = ((jrKanjiStatusWithValue *)arg)->val;
    goto withksv;
  case KC_STOREYOMI: /* echoStr �� length �� mode ��Ϳ���륿���� */
    /* �ޤ� mode ��磻�ɤˤ��Ƥߤ褦 */
    if (((jrKanjiStatusWithValue *)arg)->ks->mode) {
      len2 = MBstowcs(wbuf2, (char *)((jrKanjiStatusWithValue *)arg)->ks->mode,
		      320);
      wbuf2[len2] = (wchar_t)0;
      wks.mode = wbuf2;
    }
    else {
      wks.mode = (wchar_t *)0;
    }
    /* ����³�� */
  case KC_DEFINEKANJI: /* echoStr �� length ��Ϳ���륿���� */
    /* echoStr ��磻�ɤˤ���Ϳ���Ƥߤ褦 */
    len1 = MBstowcs(wbuf1,
		    (char *)((jrKanjiStatusWithValue *)arg)->ks->echoStr, 320);
    wbuf1[len1] = (wchar_t)0;
    wks.echoStr = wbuf1;
    wks.length = len1;
    /* ����³�� */
  case KC_KAKUTEI: /* ����ñ��Ϳ�����֤ä���륿���� */
  case KC_KILL:
    goto withksv;
  case KC_CLOSEUICONTEXT:
    goto closecont;
  case KC_QUERYMODE: /* querymode */
    ret = XwcKanjiControl2(display, window, request, (BYTE *)arg2);
    if (!ret) {
      switch (howToReturnModeInfo) {
      case ModeInfoStyleIsString:
	WCstombs((char *)arg, arg2, 256);
	break;
      case ModeInfoStyleIsBaseNumeric:
        arg[2] = (unsigned char)arg2[2];
      case ModeInfoStyleIsExtendedNumeric:
	arg[1] = (unsigned char)arg2[1];
      case ModeInfoStyleIsNumeric:
	arg[0] = (unsigned char)arg2[0];
	break;
      }
    }
    goto return_ret;
  case KC_SETLISTCALLBACK: /* dirty, dirty hack */
    /* list_cb��KC_setListCallback��d->elistcb�˰��ñۤ� */
    list_cb.client_data = (char *)arg;
    list_cb.callback_func = &EUCListCallback;
    ret = XwcKanjiControl2(display, window, request, (char *)&list_cb);
    goto return_ret;
    /* FALLTHROUGH */
  default: /* �磻�ɤǤ�EUC�Ǥ��Ѥ��ʤ���� */
    ret = XwcKanjiControl2(display, window, request, arg);
    goto return_ret;
  }
 withksv:
  ch = ((jrKanjiStatusWithValue *)arg)->buffer[0];
  ret = XwcKanjiControl2(display, window, request, (BYTE *)&wksv);
  if (ret < 0) {
    goto return_ret;
  }
  else {
    wksv.buffer[ret] = (wchar_t)0;
    ((jrKanjiStatusWithValue *)arg)->val =
      StoreWCtoEUC(wksv.buffer, wksv.val, wksv.ks,
		   (char *)((jrKanjiStatusWithValue *)arg)->buffer,
		   ((jrKanjiStatusWithValue *)arg)->bytes_buffer,
		   ((jrKanjiStatusWithValue *)arg)->ks,
		   ch, ((jrKanjiStatusWithValue *)arg)->val);
    ret = ((jrKanjiStatusWithValue *)arg)->val;
    goto return_ret;
  }
 closecont:
  ch = ((jrKanjiStatusWithValue *)arg)->buffer[0];
  ret = XwcKanjiControl2(display, window, request, (BYTE *)&wksv);
  if (ret < 0) {
    goto return_ret;
  }
  else {
    wksv.val = 0;
    ((jrKanjiStatusWithValue *)arg)->val =
      StoreWCtoEUC(wksv.buffer, wksv.val, wksv.ks,
		   (char *)((jrKanjiStatusWithValue *)arg)->buffer,
		   ((jrKanjiStatusWithValue *)arg)->bytes_buffer,
		   ((jrKanjiStatusWithValue *)arg)->ks,
		   ch, ((jrKanjiStatusWithValue *)arg)->val);
    goto return_ret;
  }
 return_ret:
#ifdef USE_MALLOC_FOR_BIG_ARRAY
  (void)free((char *)wbuf2);
  (void)free((char *)wbuf1);
  (void)free((char *)wbuf);
  (void)free((char *)arg2);
#endif
  return ret;
}

exp(int)
jrKanjiString(context_id, ch, buffer_return, nbuffer, kanji_status_return)
const int context_id, ch, nbuffer;
char  *buffer_return;
jrKanjiStatus  *kanji_status_return;
{
  *buffer_return = ch;

  return XLookupKanji2((unsigned int)0, (unsigned int)context_id,
		       buffer_return, nbuffer,
		       1/* byte */, 1/* functional char*/,
		       kanji_status_return);
}

/* jrKanjiControl -- ���ʴ����Ѵ��������Ԥ� */

exp(int)
jrKanjiControl(context, request, arg)
     const int context;
     const int request;
     char *arg;
{
  return XKanjiControl2((unsigned int)0, (unsigned int)context,
			request, (BYTE *)arg);
}

#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/
