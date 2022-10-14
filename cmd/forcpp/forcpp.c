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

#ifndef lint
static char rcsid[]="@(#) 112.1 $Id: forcpp.c,v 1.2 2003/02/01 19:34:21 aida_s Exp $";
#endif

/*
 * forcpp  /usr/bin/cpp を通す時のために, EUC-JP <-> ISO-2022-JP を相互変換する.
 * cpp が 8bit clean だと、このコマンドを通す必要はない.
 *	forcpp -7 < [in-file-name] > [out-file-name]
 *		-7	前処理
 *		-8	後処理
 */
#include	<stdio.h>
#include	<signal.h>
#include	<ctype.h>
#include        <locale.h>
#include <assert.h>
#include "canna/ccompat.h"

// 太古の AT&T UNIX SVR4 では、次のようだったらしい.
// mkmsgs(1) - /usr/lib/locale/<var>locale</var>/LC_MESSAGES/* にメッセージファ
//             イルを生成.
// gettxt(1) - メッセージファイルから文字列を取り出す.
//
// header <nl_types.h>
// gettxt(3C) ライブラリ関数で翻訳された文字列を取り出す. そのために,
// 1. exstr -e コマンドで C ソースファイルから文字列を取り出す
// 2. 翻訳する
// 3. exstr -rd コマンドでソースコードに id を埋め込む。のような運用か?
//
// 実装, 標準:
//     See SYSTEM V RELEASE 4 User's Reference Manual. - 1989年ごろ?
//         SunOS リファレンスマニュアル 2007年7月.
//               -> Oracle Solaris 11にも含まれる.
// System V Interface Definition (SVID) には含まれる
// POSIX には, 2018 edition にいたるまでずっと, 含まれない.
//   -> Canna パッケージ内に翻訳ファイルない。これは意味ない。
#define	gettxt(x,y)  (y)

const char	*hd	= "0123456789abcdef";

/* #define	ESC	'@'*/
#define	ESC 033

// @return エラー時 -1
int e2j()
{
    int c;
    int		kin = 0;

    while ( (c = getchar()) != EOF ) {
        if (c == 0x8e ) { // 半角カナ, SS2 (G2の呼び出し)
            if (kin != 2) {
                printf("\x1b(I"); kin = 2;
            }
            putchar(getchar() & 0x7f);
        }
        else if (c == 0x8f ) { // 補助漢字, SS3 (G3の呼び出し)
            if (kin != 3) {
                printf("\x1b$(D"); kin = 3;
            }
            putchar(getchar() & 0x7f);
            putchar(getchar() & 0x7f);
        }
        else if ( c & 0x80 ) { // 漢字, G1
	    if ( kin != 1 ) {
                printf("\x1b$B"); kin = 1;
            }
            putchar( c & 0x7f );
            putchar( getchar() & 0x7f );
	}
        else { // ASCII, G0
            if ( kin ) {
                printf("\x1b(B"); kin = 0;
            }
            putchar(c);
        }
    }

    return 0;
}

// @return エラー時 -1
int j2e()
{
    int c;
    int		kin = 0;

    while ( (c = getchar()) != EOF ) {
        if ( c == ESC ) {
            switch (getchar()) {
            case '(':
                switch (getchar()) {
                case 'B':
                case 'J': kin = 0; // ASCII, JIS X 0201-Roman
                          break;
                case 'I': kin = 2; // Katakana Character Set JIS C6220-1969
                          break;
                default:  return -1;
                }
                break;
            case '$':
                switch (getchar()) {
                case '@': // 旧JIS
                case 'B': kin = 1; // 新JIS. いくつかの文字が旧JISから交換.
                          break;
                case '(':
                    switch (getchar()) {
                    case 'D': kin = 3;
                              break;
                    case 'Q': kin = 1; // JIS X 0213:2004 面1. かなり追加ある
                              break;
                    default:  return -1;
                    }
                    break;
                default:  return -1;
                }
                break;
            case '.': return -1; // ISO 8859-1右, ISO 8859-7 (Greek) 右
            default:  return -1;
            }
        }
        else  {
            switch (kin) {
            case 0:
                putchar(c);
                break;
            case 1:
                putchar(c | 0x80); putchar(getchar() | 0x80);
                break;
            case 2:
                putchar(0x8e); putchar(c | 0x80);
                break;
            case 3:
                putchar(0x8f); putchar(c | 0x80); putchar(getchar() | 0x80);
                break;
            default:
                assert(0);
            }
        }
    }

    return 0;
}


void catch(int sig)
{
  fprintf(stderr, gettxt("cannacmd:18", "Dictionary format error.\n"));
  exit(1);
}


int main(int argc, char* argv[])
{
    signal(SIGSEGV, catch);
    signal(SIGBUS, catch);

#if defined(__STDC__) || defined(SVR4)
    setlocale(LC_ALL,"");
#endif
#ifdef __EMX__
    _fsetmode(stdout, "b");
#endif


    if( argc == 2 && !strcmp(argv[1], "-7"))
        return e2j();
    else if( argc == 2 && !strcmp(argv[1], "-8"))
        return j2e();
    else {
        fprintf(stderr, gettxt("cannacmd:19", "Usage: forcpp -7 < [file],\n       forcpp -8 < [file]\n"));
        exit( 1 );
    }

    return 0;
}
