﻿// -*- coding:utf-8-with-signature -*-
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

#ifndef	lint
static char rcsid[] = "@(#) 112.1 $Id: splitwd.c,v 1.2.4.2 2003/12/27 17:15:23 aida_s Exp $";
#endif

#include	<stdio.h>
#include	<signal.h>
#include	"canna/ccompat.h"

#if defined(__STDC__) || defined(SVR4)
#include <locale.h>
#endif

// SVR4
#define	gettxt(x,y)  (y)

#define		SIZE			4192
#define		ISSPACE(c)		('\n' == c || ' ' == c || '\t' == c)

#ifndef         AIXV3
typedef unsigned char	uchar;
#endif


struct head{
    uchar	yomi[SIZE];
    struct tango	*next;
}word;

struct tango{
    uchar	*tsuduri;
    uchar	*hinshi;
    struct tango	*next;
};

uchar *getword(p,Word)
uchar	*p;
uchar	*Word;
{
    while( ISSPACE(*p) )	p++;

    while( !ISSPACE(*p) && '\0' != *p )	{
      if (*p == '\\' && *(p + 1)) {
	*Word++ = *p++;
      }
      *Word++ = *p++;
    }
    *Word = '\0';

    return(p);
}

struct tango *newtango(tsuduri,hinshi)
uchar   *tsuduri;
uchar   *hinshi;
{
    struct tango   *tp;
    uchar   *p;

    if( !(tp = (struct tango *)malloc(sizeof(struct tango))) )
	fprintf(stderr, gettxt("cannacmd:41",
	       "cannnot malloc %lu\n"), (unsigned long)sizeof(struct tango) );

    if( !(p = (uchar *)malloc(strlen((char *)tsuduri) + 1)) )
	fprintf(stderr, gettxt("cannacmd:42",
	       "cannnot malloc %lu\n"),
		(unsigned long)strlen((char *)tsuduri)+1 );

    tp->tsuduri = p;
    strcpy((char *)p,(char *)tsuduri);

    if( !(p = (uchar *)malloc(strlen((char *)hinshi) + 1)) )
	fprintf(stderr, gettxt("cannacmd:43",
	       "cannnot malloc %lu\n"),
		(unsigned long)strlen((char *)hinshi)+1 );
    tp->hinshi = p;
    strcpy((char *)p, (char *)hinshi);

    tp->next = NULL;

    return(tp);
}

void savetango(tsuduri, hinshi)
uchar	*tsuduri;
uchar	*hinshi;
{
    struct tango	*tp;

    if( !word.next ){
	word.next = newtango(tsuduri,hinshi);
	return;
    }

    tp = word.next;
    while(tp->next)
	tp = tp->next;

    tp->next = newtango(tsuduri, hinshi);
}

void save_factor(line, nline)
uchar	*line;
int	nline;
{
    uchar	*lp;
    uchar	hinshi[SIZE];
    uchar	tsuduri[SIZE];

    lp = line;

    lp = getword(lp,word.yomi);			/* head に読みを入れる */
    lp = getword(lp,hinshi);			/* hinshi を取り込む */
    if ('#' != word.yomi[0] && '#' != hinshi[0])
        fprintf(stderr, gettxt("cannacmd:48", "No hinshi in line %d\n"),
		nline);

    next:
    while(1){ 		       			/* 表記読み込み loop */
	lp = getword(lp,tsuduri);
	if( '\0' == tsuduri[0] ) 		/* 1 行終わり */
	    break;
	if( '#' == tsuduri[0] ){ 		/* 品詞が読み込まれた */
	    strcpy((char *)hinshi, (char *)tsuduri);
	    goto next;
	}
	savetango(tsuduri,hinshi);
    }
}


void disp_factor()
{
    struct tango	*tp;

    tp = word.next;
    while( tp ){
#ifdef USE_ATMARK
	if(!strcmp((char *)word.yomi, (char *)tp->tsuduri))
	    printf("%s %s @\n", word.yomi, tp->hinshi);
	else
#endif
	    printf("%s %s %s\n", word.yomi, tp->hinshi, tp->tsuduri);
	tp = tp->next;
    }
}

void
free_factor(tp)
struct tango *tp;
{
  struct tango *ftp;

  while (tp) {
    ftp = tp;
    tp = ftp->next;
    free((char *)ftp->tsuduri);
    free((char *)ftp->hinshi);
    free((char *)ftp);
  }
}

void
catch(sig)
int sig;
{
  fprintf(stderr, gettxt("cannacmd:44", "Dictionary format error.\n"));
  exit(1);
}

static void
splitword(fp, name)
FILE *fp;
char *name;
{
  int nline = 0; /* 読み込む行数を数える */
  uchar line[SIZE];

  while (fgets((char *)line, sizeof(line), fp)) {

    nline++;
    if ('\n' != line[strlen((char *)line) - 1]) {
      fprintf(stderr, gettxt("cannacmd:47",
			     "%s:Line %d is too long.\n"), name, nline);
    }
    else {
      line[strlen((char *)line) - 1] = '\0';
    }

    save_factor(line, nline);
    disp_factor();
    free_factor(word.next);
    word.next = NULL;
  }
}

main( argc, argv )
int	argc;
char	*argv[];
{
    FILE	*fp;
    int		i;

    signal(SIGSEGV, catch);
#ifdef SIGBUS
    signal(SIGBUS, catch);
#endif
#if defined(__STDC__) || defined(SVR4)
    (void)setlocale(LC_ALL,"");
#endif

    if( argc == 1 ) {		/* コマンドだけの時 */
      splitword(stdin, argv[0]);
    }

    for( i = 1; i < argc ; i++ ){
	if( !(fp = fopen( argv[i], "r" )) )
	    fprintf(stderr, gettxt("cannacmd:46",
		   "cannot open file %s\n"), argv[i] );

	splitword(fp, argv[0]);

	fclose( fp );
    }
    exit(0);
}
