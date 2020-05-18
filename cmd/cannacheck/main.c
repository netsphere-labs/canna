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

static char rcs_id[] = "@(#) 102.1 $Id: main.c,v 1.6.2.2 2003/12/27 17:15:21 aida_s Exp $";

/* ��ưȽ�̻ٱ祳����: �����EUC-JP���������Ȥ����������������ס� */

#include <stdio.h>
#define CANNA_NEW_WCHAR_AWARE
#include <canna/jrkanji.h>
#include <canna/RK.h>
#include "ccompat.h"
#include "rkcapi.h"

int IROHA_verbose = 0;
  
main(argc, argv,envp)
int argc;
char *argv[], *envp[];
{
  char **warn, *cannafile = (char *)0, *servername = (char *)0;
  int i;

  for (i = 1 ; i < argc ; i++) {
    if (!strcmp(argv[i], "-v")) {
      IROHA_verbose = 1;
    }
    else if (!strcmp(argv[i], "-f") && i + 1 < argc) {
      cannafile = argv[i + 1];
      i++;
    }
    else if (!strcmp(argv[i], "-cs") && i + 1 < argc) {
      servername = argv[i + 1];
      i++;
    }
    else {
      fprintf(stderr, "usage: %s [-v] [-f cannafile] [-cs cannahost]\n",
	      argv[0]);
      exit (1);
    }
  }
  if (cannafile) {
    jrKanjiControl(0, KC_SETINITFILENAME, cannafile);
  }
  if (servername) {
    jrKanjiControl(0, KC_SETSERVERNAME, servername);
  }
  jrKanjiControl(0, KC_SETVERBOSE,
		 (char *)(canna_intptr_t)(IROHA_verbose ?
				CANNA_FULL_VERBOSE : CANNA_HALF_VERBOSE));
  if (jrKanjiControl(0, KC_INITIALIZE, (char *)&warn) != -1) {
    if (IROHA_verbose) {
      char *p;

      p = RkwGetServerName();
      if (p)
	  printf("������ \"%s\" ����³���ޤ���\n", p);
      else
	  printf("��³��Υ����Ф������Ǥ�\n");
    }
    if (warn) {
      char **p;

      for (p = warn; *p ; p++) {
	printf("%s\n", *p);
      }
    }
    jrKanjiControl(0, KC_FINALIZE, (char *)0);
  }
}
