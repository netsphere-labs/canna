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
static char rcs_id[] = "$Id: util.c,v 1.8 2003/09/21 12:56:29 aida_s Exp $";
#endif

#include "server.h"
#include "canna/widedef.h"


/*
  WidenClientContext

  クライアント構造体で管理しているコンテキスト配列をちょっと大きくする。

  cl -- クライアント構造体へのポインタ
  n  -- どのくらい大きくするか

  返り値
   1  -- 成功
   0  -- 失敗

 */

int
WidenClientContext(cl, n)
ClientPtr cl;
size_t n;
{
  int *new, *old, i;

  new = (int *)malloc((cl->cfsize + n) * sizeof(int));
  if (new) {
    old = cl->context_flag;
    for (i = 0 ; i < cl->ncon ; i++) {
      new[i] = old[i];
    }
    if (cl->ncon > 0)
      free((char *)old);
    cl->context_flag = new;
    cl->cfsize += n;
    return 1;
  }
  return 0;
}

/*
  set_cxt -- そのクライアントが指定されたコンテキストを使っていることを記録

  返り値:
    1 -- 成功
    0 -- 失敗
 */

#define N_ADD_CONTEXTS	    4
int
set_cxt(cl, n)
ClientPtr cl;
int n;
{
  if (!(cl->ncon < cl->cfsize) && !WidenClientContext(cl, N_ADD_CONTEXTS)) {
    return 0;
  }
  else {
    cl->context_flag[cl->ncon++] = n;
    return 1;
  }
}

/*
  off_cxt -- そのクライアントで、指定されたコンテキストをもう使わなくなった
 */

void
off_cxt(cl, cn)
ClientPtr cl;
int cn;
{
  int i, n = cl->ncon, *contexts = cl->context_flag;

  for (i = 0 ; i < n ; i++) {
    if (contexts[i] == cn) {
      break;
    }
  }
  n--;
  for (; i < n ; i++) {
    contexts[i] = contexts[i + 1];
  }
  cl->ncon = n;
}

/*
  chk_cxt -- そのクライアントで、指定されたコンテキストを使ってもいいの？

   1 -- 使ってもいい
   0 -- いけない
 */

int
chk_cxt(cl, cn)
ClientPtr cl;
int cn;
{
  int i, n = cl->ncon, *contexts = cl->context_flag;

  for (i = 0 ; i < n ; i++) {
    if (contexts[i] == cn) {
      return 1;
    }
  }
  return 0;
}
