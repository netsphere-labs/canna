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
 * suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
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
static char rcs_id[] = "$Id: engine.c,v 1.6 2003/09/21 10:16:49 aida_s Exp $";
#endif

#include "canna.h"

#define CANNA_SERVER_NAME_LEN 128
static char iroha_server_name[CANNA_SERVER_NAME_LEN] = {0, 0};

RkSetServerName(s)
char *s;
{
  if (s)
    (void)strncpy(iroha_server_name, s, CANNA_SERVER_NAME_LEN);
  else
    iroha_server_name[0] = '\0';
  return 0;
}

char *
RkGetServerHost()
{
  if (iroha_server_name[0]) {
    return iroha_server_name;
  }
  else {
    return (char *)0;
  }
}


void
close_engine()
{
#ifdef ENGINE_SWITCH
#ifdef DL
  if (dlh) {
    (void)dlclose(dlh);
    dlh = (DSOHANDLE)0;
  }
#endif /* DL */
  Rk = (struct rkfuncs *)0;
  current_engine = -1;
#endif /* ENGINE_SWITCH */
}
