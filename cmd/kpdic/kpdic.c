// -*- coding:utf-8-with-signature -*-
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

// lib/canna/RKroma.c:RkwOpenRoma() 関数で読み込める形式に、ローマ字かなテーブル
// ファイルを変換する.
// => そもそも何でわざわざ変換するのか? テキストのままでいいやん

#ifndef lint
static char rcsid[]="@(#) 102.1 $Id: kpdic.c,v 1.4.2.2 2003/12/27 17:15:23 aida_s Exp $";
#endif

#if defined(__STDC__) || defined(SVR4)
#include <locale.h>
#endif

// SVR4
#define	gettxt(x,y)  (y)

#include "canna/ccompat.h"

#ifdef __CYGWIN32__
#include <fcntl.h> /* for O_BINARY */
#endif

#include	<stdio.h>
#include	<ctype.h>
#include <stdarg.h>

#define		MAXKEY	((1 << 16) / 4)
#define		MAXSIZE	(1 << 16)

#define		LMAXKEY		(1 << 16)
#define		LMAXSIZE	(1 << 20)

#define LOMASK(x)	((x)&255)

static char	fileName[256];
static int	lineNum;
static int	errCount = 0;

/* flag_old == 1 の場合にしか使われない.
   Old format `RX_RXDIC` -> temp列がなく, 促音「っ」が決め打ち。上手くない.
struct  def_tbl {
	    int   used  ;
	    char  *roma ;
	    char  *kana ;
	    char  *intr ;
        }    ;
static struct def_tbl def [] = {
	    {0,"kk","っ","k"},
	    {0,"ss","っ","s"},
	    {0,"tt","っ","t"},
	    {0,"hh","っ","h"},
	    {0,"mm","っ","m"},
	    {0,"yy","っ","y"},
	    {0,"rr","っ","r"},
	    {0,"ww","っ","w"},
	    {0,"gg","っ","g"},
	    {0,"zz","っ","z"},
	    {0,"dd","っ","d"},
	    {0,"bb","っ","b"},
	    {0,"pp","っ","p"},
	    {0,"cc","っ","c"},
	    {0,"ff","っ","f"},
	    {0,"jj","っ","j"},
	    {0,"qq","っ","q"},
	    {0,"vv","っ","v"}
	}  ;
*/

static void alert(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "#line %d %s: (WARNING) ", lineNum, fileName);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);

    ++errCount;
}


static void fatal(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "#line %d %s: (FATAL) ", lineNum, fileName);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);

    exit(1);
}


// 文字列から語を切り出す
// @param word 語を書き込む領域. 呼び出し側が確保すること.
// @param maxword word の大きさ.
// @return 語があれば 1以上 (長さ).
static int
getWORD(const unsigned char* s, const unsigned char** news,
        unsigned char* word, int maxword)
{
    unsigned char c;
    int	 	i;

    i = 0;
    while ( *s && *s <= ' ' )
	s++;
    while ( (c = *s) > ' ' ) {
	s++;
	if ( c == '\\' ) {
	    switch(*s) {
            case '\0':
		break;
            case '0': // 8進数3桁
                /*
		if ( s[1] == 'x' && isxdigit(s[2]) && isxdigit(s[3]) ) {
		    unsigned char   xx[3];

		    s += 2;
		    xx[0] = *s++; xx[1] = *s++; xx[2] = 0;
		    sscanf((char *)xx, "%x", &c);
                    } */
		{
                    c = 0;
                    int j = 0;
                    while ( (++j) <= 3 && isdigit(*s) )
                        c = 8 * c + (*s++ - '0');
                }
		break;
            case 'x':  // 16進数2桁
		{
		    unsigned char   xx[3];
		    unsigned char   *xxp = xx;
		    s++;
		    if ( isxdigit(*s) )
			*xxp++ = *s++;
		    if ( isxdigit(*s) )
			*xxp++ = *s++;
		    *xxp = '\0';
		    sscanf((char *)xx, "%hhx", &c);
		}
		break;
	    default:
		c = *s++;
		break;
            }
        }
	if ( i < maxword - 1 )
	    word[i++] = c;
    }
    word[i] = '\0';
    *news = s;
    return i;
}


// 文字列をコピーして作成する。呼び出し側が解放すること。
static unsigned char* allocs(const unsigned char* s)
{
    unsigned char	*d;

    if ( (d = (unsigned char *)malloc(strlen((char *)s) + 1)) != NULL )
	 strcpy((char *)d, (char *)s);
    else {
	fprintf(stderr, "Out of memory\n");
	exit(1);
    }
    return d;
}

// 入力行
struct roman {
    unsigned char	*roma; // ローマ字
    unsigned char	*kana; // 出力かな
    unsigned char	*temp; // ローマ字バッファに残す
    int                 bang;  // 列4 これは何?
};


static void
freeallocs(struct roman* roman, int nKey)
{
    int i;

    for (i = 0 ; i < nKey ; i++) {
        /* free them */
        free(roman[i].roma); roman[i].roma = NULL;
        free(roman[i].kana); roman[i].kana = NULL;
        if (roman[i].temp) {
            free(roman[i].temp); roman[i].temp = NULL;
        }
    }
}


// for qsort()
static int compar(const void* p, const void* q)
{
    const unsigned char* s = ((const struct roman*) p)->roma;
    const unsigned char* t = ((const struct roman*) q)->roma;

    while ( *s == *t ) {
        if ( *s )
            s++, t++;
        else
            return 0;
    }
    return ((int)*s) - ((int)*t);
}


/*
static int chk_dflt(int c)
{
    int  i,n ;
    char cc = (char)c;
    n = sizeof(def) / sizeof(struct def_tbl) ;
    for (i=0; i < n ; i++) {
	if (cc == def[i].intr[0]) {
	    return(i+1) ;
	}
    }
    return(0);
}
*/

static unsigned char* skip_sp(unsigned char* s)
{
    while (*s == ' ' || *s == '\t')
        s++;
    return s;
}

int main(int argc, char* argv[])
{
    struct roman *roman;
    unsigned char	rule[256], *r;
  int			nKey, size;
  int			i, p;
  //  int                   flag_old ;
  int                   flag_large = 0;
  int                   werr ;
  long maxkey, maxsize;
  unsigned char	l4[4], *bangchars = NULL, *pp;

#if defined(__STDC__)  || defined(SVR4)
    setlocale(LC_ALL,"");
#endif
    // もっともポータブルなのは, C標準関数を使う方法. setmode() は UNIX のみ.
    freopen(NULL, "wb", stdout);

/* option */
//    flag_old =  0 ;
    werr = 0 ;
    while(--argc) {
    	argv++ ;
        //        if (!strcmp(*argv,"-m")) {
        //		flag_old = 1 ;
        //}
        if (!strcmp(*argv,"-x")) {
    		flag_large = 1 ;
        }
    }

  if (flag_large) {
    maxkey = LMAXKEY;
    maxsize = LMAXSIZE;
  }
  else {
    maxkey = MAXKEY;
    maxsize = MAXSIZE;
  }

  roman = (struct roman *)malloc(sizeof(struct roman) * maxkey);
  if (!roman) {
    fatal(gettxt("cannacmd:8", "No more memory\n"), 0);
  }

    nKey = 0;
    size  = 0;
    // .kpdef ファイル: 文字コードはEUC-JP. 各行は次のいずれか
    //    空行は無視
    //    "#" 以下はコメント
    //    @568 作者? コメント?
    // 定義:
    //    <ローマ字> <空白またはタブ> <出力かな> (<空白またはタブ> <残す>)opt
    //    <ローマ字>には、かな文字を買いてもよい(!)
    //       kk    っ   k    .. 2番目「っ」を出力し, 3番目「k」を残す
    //       か@   が        .. バッファの末尾が「か」のときに"@"を打つと「が」
    //       b     \0   こ   .. "b" -> 出力なし. ローマ字バッファに「こ」残す
    //                          濁点に備える.
    // 語について:
    //       \x27 16進数2桁。キーコードではなく文字で書くため. "'" の意味.
    //       \010 8進数3桁.
    //       \#   エスケープ. "#" など.
    while (fgets((char *)(r = rule), sizeof(rule), stdin)) {
        unsigned char roma[256];

        lineNum++;
        r = skip_sp(r);
        if ( *r == '\r' || *r == '\n' || *r == '#' )
            continue;

        // ローマ字
        if ( getWORD(r, &r, roma, sizeof(roma)) ) {
            if (nKey >= maxkey) {
                freeallocs(roman, nKey);
                free( roman );
                fatal("More than %d romaji rules are given.", maxkey);
            }

            for ( i = 0; i < nKey; i++ ) {
                if ( !strcmp((char *)roman[i].roma, (char *)roma) )
                    break;
            }
            if ( i < nKey ) {
                alert("multiply defined key <%s>. skip.", roma);
                continue;
            }
            roman[nKey].roma = allocs(roma);

            // 出力かな
            if ( getWORD(r, &r, roma, sizeof(roma)) ) {
                roman[nKey].kana = allocs(roma);
                roman[nKey].temp = NULL;
                roman[nKey].bang = 0;

                // temp
                if ( getWORD(r, &r, roma, sizeof(roma)) ) {
                    roman[nKey].temp = allocs(roma);
                    if ( getWORD(r, &r, roma, sizeof(roma)) ) {
                        roman[nKey].bang = 1;
                    }
                }
                size += strlen((char *)roman[nKey].roma) + 1 +
                     strlen((char *)roman[nKey].kana) + 1 +
                     (roman[nKey].temp ? strlen((char *)roman[nKey].temp) : 0) +
                    1;  // bang分

                nKey++;
            }
            else {
                // バックトラックのトリガー文字
                if (roman[nKey].roma &&
                    roman[nKey].roma[0] == '!' &&
                    roman[nKey].roma[1] != '\0' ) {
                    abort(); // 使われていないように見えるが...
                    if (bangchars) {
                        free((char *)bangchars);
                    }
                    bangchars = allocs(roman[nKey].roma + 1);
                }
                else {
                    alert("syntax error: %s", rule);
                }

                free(roman[nKey].roma);
                roman[nKey].roma = NULL;
            }
        }
    }

    if ( errCount ) {
        freeallocs(roman, nKey);
        free(roman);
        fatal(gettxt("cannacmd:29", "Romaji dictionary is not produced."), 0);
    }

    size += (bangchars ? strlen((char *)bangchars) : 0) + 1;
    if (size >= maxsize) {
        freeallocs(roman, nKey);
        free(roman);
        fatal(gettxt("cannacmd:32", "Too much rules.  Size exhausted."), 0);
    }

    qsort(roman, nKey, sizeof(struct roman), compar);

    // 出力 - header部
    if (!flag_large) {
        putchar('K'); putchar('P'); // RX_KPDIC

        l4[0] = LOMASK(size >> 8); l4[1] = LOMASK(size);
        l4[2] = LOMASK(nKey >> 8); l4[3] = LOMASK(nKey);
        putchar(l4[0]); putchar(l4[1]); putchar(l4[2]); putchar(l4[3]);
    }
    else {
        putchar('P'); putchar('T'); // RX_PTDIC

        l4[0] = LOMASK(size >> 24); l4[1] = LOMASK(size >> 16);
        l4[2] = LOMASK(size >> 8); l4[3] = LOMASK(size);
        putchar(l4[0]); putchar(l4[1]); putchar(l4[2]); putchar(l4[3]);
        l4[0] = LOMASK(nKey >> 24); l4[1] = LOMASK(nKey >> 16);
        l4[2] = LOMASK(nKey >> 8); l4[3] = LOMASK(nKey);
        putchar(l4[0]); putchar(l4[1]); putchar(l4[2]); putchar(l4[3]);
    }

    // バックトラックのトリガー文字
    if (bangchars) {
        for (pp = bangchars ; pp && *pp ; pp++) {
            putchar(*pp);
        }
        free( bangchars);
    }
    putchar('\0');

    // ルール部
    for ( i = 0; i < nKey; i++ ) {
        r = roman[i].roma; do { putchar(*r); } while (*r++);
        r = roman[i].kana; do { putchar(*r); } while (*r++);
        if (roman[i].temp) {
            r = roman[i].temp; while (*r) putchar(*r++); // ナル文字書かない.
        }
        putchar(roman[i].bang); /* temp がなくて、bang が１はありえない */
    }

    freeallocs(roman, nKey);
    free( roman);
    fprintf(stderr, "STR SIZE = %d, KEYS = %d\n", size, nKey);
    if (werr == 1 ) {
        fprintf(stderr,gettxt("cannacmd:31",
	  "warning: Option -m is specified for new dictionary format.\n")) ;
    }

    exit(0);
}
