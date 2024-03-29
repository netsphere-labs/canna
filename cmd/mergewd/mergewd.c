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

#ifndef lint
static char rcsid[]="@(#) 112.1 $Id: mergewd.c,v 1.1.1.1.4.2 2003/12/27 17:15:23 aida_s Exp $";
#endif

#include "RK/RKintern.h"
#include <stdio.h>
#include <locale.h>

// SVR4
#define	gettxt(x,y)  (y)

#define RkwIsGraphicChar(x) ((unsigned long)(x) > (unsigned long)' ')
#define RkwIsControlChar(x) ((unsigned long)(x) < (unsigned long)' ')

/* rec  = klen + hinshi + kanji */
/*        1    + 1      + ?     */
/* wrec = ylen  + yomi  + knum  + fqoffset + rec */
/*        1byte + ?byte + 2byte + 3byte    + rec */
#define ckrecSiz(k)		(NW_PREFIX + WStrlen(k))
#define ckwrecSiz(y,s)		(1 + WStrlen(y) +2+3+s)
#ifndef AIXV3
typedef unsigned char	uchar;
#endif


//cannawc*
//euctous(const unsigned char* src, int srclen, cannawc* dest, int destlen)


int main(n, args)
int	n;
char	*args[];
{
    uchar	Yomi[RK_LINE_BMAX], Pair[RK_LINE_BMAX], Kanji[RK_LINE_BMAX*10];
    uchar	yomi[RK_LINE_BMAX], pair[RK_LINE_BMAX], kanji[RK_LINE_BMAX*10];
    uchar	S[RK_LINE_BMAX*10], *s, *d;
    uchar	fkanji[RK_LINE_BMAX*10];
    Wchar	wyomi[RK_LINE_BMAX], wkanji[RK_LINE_BMAX*10];
    int		kouho = 0;
    int		wrec;
    int		krec = 0;
    int		rec = 0;
    int		first = 0;
    FILE *fp;

#if defined(__STDC__) || defined(SVR4)
    (void)setlocale(LC_ALL,"");
#endif
#ifdef __EMX__
  _fsetmode(stdout, "b");
#endif
    /*  first = !strcmp(args[1], "-f"); */
    Yomi[0] = Pair[0] = Kanji[0] = 0;

    if (n < 2 || !(fp = fopen(args[1], "r"))) {
      fp = stdin;
    }

    while (fgets((char *)(s = S), sizeof(S), fp)) {
      s[strlen((char *)s) - 1] = '\0';
      if (!RkwIsGraphicChar(S[0]) || S[0] == '#' )
	continue;
      d = yomi;
      while (RkwIsGraphicChar(*d = *s++)) d++;
      *d = 0;
      while (*s && !RkwIsGraphicChar(*s)) s++;
      d = pair;
      while (RkwIsGraphicChar(*d = *s++)) d++;
      *d = 0;
      while (*s && !RkwIsGraphicChar(*s)) s++;
      d = kanji;
      while ( (*d = *s++) != 0 )	d++;

      euctous(yomi, strlen((char *)yomi), wyomi, sizeof(yomi)/sizeof(Wchar));
      euctous(kanji, strlen((char *)kanji), wkanji, sizeof(kanji)/sizeof(Wchar));
      if (!strcmp((char *)Yomi, (char *)yomi)) {
	if ( !first ) {
	  if (strcmp((char *)Pair, (char *)pair)) {     /* 品詞が異なる */
	    rec += ckrecSiz(wkanji);
	    wrec = ckwrecSiz(wyomi, rec);
	    /* ここはワードレコードの大きさは無視したい */
	    if( (kouho < RK_CAND_NMAX) && (wrec <= RK_WREC_BMAX) ) {
	      strcpy((char *)Pair, (char *)pair);
	      strcat((char *)Kanji, " ");
	      strcat((char *)Kanji, (char *)Pair);

	      strcat((char *)Kanji, " ");
	      strcat((char *)Kanji, (char *)kanji);
	      strcpy((char *)fkanji, (char *)kanji);
	      krec += ckrecSiz(wkanji);
	    }
	    else
	      (void)fprintf(stderr, gettxt("cannacmd:32",
			   "%s: over word [%d %d]\n"), Yomi, kouho, rec);
	    kouho++;
	  }
	  else if (strcmp((char *)fkanji, (char *)kanji)) {
	    rec += ckrecSiz(wkanji);
	    wrec = ckwrecSiz(wyomi, rec);
	    /* ここはワードレコードの大きさは無視したい */
	    if( (kouho < RK_CAND_NMAX) && (wrec <= RK_WREC_BMAX*2) ) {
	      strcat((char *)Kanji, " ");
	      strcat((char *)Kanji, (char *)kanji);
	      strcpy((char *)fkanji, (char *)kanji);
	      krec += ckrecSiz(wkanji);
	    }
	    else
	      (void)fprintf(stderr, gettxt("cannacmd:33",
			   "%s: *over word [%d %d]\n"), Yomi, kouho, rec);
	    kouho++;
	  }
	}
      }
      else {
	if ( Yomi[0] ) {                 /* 読みが異なる */
	  wrec = ckwrecSiz(wyomi, rec);
	  printf("%s%s\n", Yomi, Kanji);
	  /* ここはチェックのため */
	  if( (kouho > RK_CAND_NMAX) || (wrec > RK_WREC_BMAX) )
	    (void)fprintf(stderr, gettxt("cannacmd:34",
			 "%s: over word [%d %d]\n"), Yomi, kouho, rec);
	  if (strlen((char *)Yomi)+strlen((char *)Kanji)+1 >= RK_LINE_BMAX ) {
	    (void)fprintf(stderr,gettxt("cannacmd:35", "too long line\n"));
	  }
	}
	strcpy((char *)Yomi, (char *)yomi);
	strcpy((char *)Pair, (char *)pair);
	strcpy((char *)fkanji, (char *)kanji);

	strcpy((char *)Kanji, " ");
	strcat((char *)Kanji, (char *)Pair);
	strcat((char *)Kanji, " ");
	strcat((char *)Kanji, (char *)kanji);
	krec += ckrecSiz(wkanji);
	rec = 0;                    /* 修正 = krec */
	krec = 0;
	kouho = 1;
      }
    }

    if (fp != stdin) {
      fclose(fp);
    }

    if ( Yomi[0] ) {
      wrec = ckwrecSiz(wyomi, rec);
      printf("%s%s\n", Yomi, Kanji);
      /* ここはチェックのため */
      if( (kouho > RK_CAND_NMAX) || (wrec > RK_WREC_BMAX) )
	(void)fprintf(stderr, gettxt("cannacmd:36", "%s: over word [%d %d]\n"), Yomi, kouho, rec);
      if (strlen((char *)Yomi)+strlen((char *)Kanji)+1 >= RK_LINE_BMAX ) {
	(void)fprintf(stderr,gettxt("cannacmd:37", "too long line\n"));
      }
    }
    exit(0);
}
