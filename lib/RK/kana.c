// -*- coding:utf-8-with-signature -*-

/* Copyright 1994 NEC Corporation, Tokyo, Japan.
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
static char rcsid[]="@(#) 102.1 $Id: kana.c,v 1.2 2003/09/17 08:50:52 aida_s Exp $";
#endif

/* LINTLIBRARY */
#include "RKintern.h"
#include "canna/canna.h"
#include <assert.h>

#define SUUJI_THROUGH		0
#define SUUJI_HANKAKU		1
#define SUUJI_ZENKAKU		2
#define SUUJI_SIMPLEKANJI	3
#define SUUJI_FULLKANJI		4
#define SUUJI_FULLKANJITRAD	5
#define SUUJI_WITHKANJIUNIT	6
#define SUUJI_WITHCOMMA		7

/* RkCvtZen
 * 半角文字を全角文字にする.
 * 値は EUC-JP. 半角 => 全角では, SS3 (0x8F) に続いて3バイトになる組み合わせは
 * ない.
 */
static cannawc hiragana[] =
{
/* 0x00 */
  0x0000,	0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
/* 0x10 */
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
/* 0x20 */
	0xa1a1, 0xa1aa, 0xa1ed, 0xa1f4,		0xa1f0, 0xa1f3, 0xa1f5, 0xa1c7,
	0xa1ca, 0xa1cb, 0xa1f6, 0xa1dc,		0xa1a4, 0xa1dd, 0xa1a5, 0xa1bf,
/* 0x30 */
	0xa3b0, 0xa3b1, 0xa3b2, 0xa3b3,		0xa3b4, 0xa3b5, 0xa3b6, 0xa3b7,
	0xa3b8, 0xa3b9, 0xa1a7, 0xa1a8,		0xa1e3, 0xa1e1, 0xa1e4, 0xa1a9,
/* 0x40 */
	0xa1f7, 0xa3c1, 0xa3c2, 0xa3c3,		0xa3c4, 0xa3c5, 0xa3c6, 0xa3c7,
	0xa3c8, 0xa3c9, 0xa3ca, 0xa3cb,		0xa3cc, 0xa3cd, 0xa3ce, 0xa3cf,
/* 0x50 */
	0xa3d0, 0xa3d1, 0xa3d2, 0xa3d3,		0xa3d4, 0xa3d5, 0xa3d6, 0xa3d7,
	0xa3d8, 0xa3d9, 0xa3da, 0xa1ce,		0xa1ef, 0xa1cf, 0xa1b0, 0xa1b2,
/* 0x60 */
	0xa1c6, 0xa3e1, 0xa3e2, 0xa3e3,		0xa3e4, 0xa3e5, 0xa3e6, 0xa3e7,
	0xa3e8, 0xa3e9, 0xa3ea, 0xa3eb,		0xa3ec, 0xa3ed, 0xa3ee, 0xa3ef,
/* 0x70 */
	0xa3f0, 0xa3f1, 0xa3f2, 0xa3f3,		0xa3f4, 0xa3f5, 0xa3f6, 0xa3f7,
	0xa3f8, 0xa3f9, 0xa3fa, 0xa1d0,		0xa1c3, 0xa1d1, 0xa1c1, 0xa2a2,
/*0x80 */
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
/*0x90 */
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
/*0xa0 */
	0xa1a1, 0xa1a3, 0xa1d6, 0xa1d7,		0xa1a2, 0xa1a6, 0xa4f2, 0xa4a1,
	0xa4a3, 0xa4a5, 0xa4a7, 0xa4a9,		0xa4e3, 0xa4e5, 0xa4e7, 0xa4c3,
/*0xb0 */
	0xa1bc, 0xa4a2, 0xa4a4, 0xa4a6,		0xa4a8, 0xa4aa, 0xa4ab, 0xa4ad,
	0xa4af, 0xa4b1, 0xa4b3, 0xa4b5,		0xa4b7, 0xa4b9, 0xa4bb, 0xa4bd,
/*0xc0 */
	0xa4bf, 0xa4c1, 0xa4c4, 0xa4c6,		0xa4c8, 0xa4ca, 0xa4cb, 0xa4cc,
	0xa4cd, 0xa4ce, 0xa4cf, 0xa4d2,		0xa4d5, 0xa4d8, 0xa4db, 0xa4de,
/*0xd0 */
	0xa4df, 0xa4e0, 0xa4e1, 0xa4e2,		0xa4e4, 0xa4e6, 0xa4e8, 0xa4e9,
	0xa4ea, 0xa4eb, 0xa4ec, 0xa4ed,		0xa4ef, 0xa4f3, 0xa1ab, 0xa1ac,
/* 0xe0 */
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
/* 0xf0 */
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
};

static cannawc hankaku[] = {
/*0x00*/
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
/*0x10*/
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
/*0x20*/
	0x0000,    ' ', 0x8ea4, 0x8ea1,		   ',',    '.', 0x8ea5,    ':',
	   ';',    '?',    '!', 0x8ede,		0x8edf, 0x0000, 0x0000, 0x0000,
/*0x30*/
	   '^', 0x0000,    '_', 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x8eb0, 0x0000, 0x0000,    '/',
/*0x40*/
	0x0000,    '~', 0x0000,    '|',		0x0000, 0x0000,   '\'',   '\'',
	   '"',    '"',    '(',    ')',		   '[',    ']',    '[',    ']',
/*0x50*/
	'{',    '}', 0x0000, 0x0000,		0x0000, 0x0000, 0x8ea2, 0x8ea3,
	0x0000, 0x0000, 0x0000, 0x0000,		   '+',    '-', 0x0000, 0x0000,
/*0x60*/
	0x0000,    '=', 0x0000,    '<',		   '>', 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000,   '\\',
/*0x70*/
	    '$',0x0000, 0x0000,    '%',		   '#',    '&',    '*',    '@',
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
/*0x80*/
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
/*0x90*/
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
/*0xa0*/
	0x0020, 0x00a7, 0x00b1, 0x00a8, 	0x00b2, 0x00a9, 0x00b3, 0x00aa,
	0x00b4, 0x00ab, 0x00b5, 0x00b6, 	0xb6de, 0x00b7, 0xb7de, 0x00b8,
/*0xb0*/
	0xb8de, 0x00b9, 0xb9de, 0x00ba, 	0xbade, 0x00bb, 0xbbde, 0x00bc,
	0xbcde, 0x00bd, 0xbdde, 0x00be, 	0xbede, 0x00bf, 0xbfde, 0x00c0,
/*0xc0*/
	0xc0de, 0x00c1, 0xc1de, 0x00af, 	0x00c2, 0xc2de, 0x00c3, 0xc3de,
	0x00c4, 0xc4de, 0x00c5, 0x00c6, 	0x00c7, 0x00c8, 0x00c9, 0x00ca,
/*0xd0*/
	0xcade, 0xcadf, 0x00cb, 0xcbde, 	0xcbdf, 0x00cc, 0xccde, 0xccdf,
	0x00cd, 0xcdde, 0xcddf, 0x00ce, 	0xcede, 0xcedf, 0x00cf, 0x00d0,
/*0xe0*/
	0x00d1, 0x00d2, 0x00d3, 0x00ac, 	0x00d4, 0x00ad, 0x00d5, 0x00ae,
	0x00d6, 0x00d7, 0x00d8, 0x00d9, 	0x00da, 0x00db, 0x00dc, 0x00dc,
/*0xf0*/
	0x00b2, 0x00b4, 0x00a6, 0x00dd,		0xb3de, 0x00b6, 0x00b9, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,		0x0000, 0x0000, 0x0000, 0x0000,
};


// @param count lengthだけ加算.
#define	ADDCODE(dst, maxdst, count, code, length) {\
    if ( (unsigned long)(length) <= (unsigned long)(maxdst) ) {\
	(maxdst) -= (length); (count) += (length);\
	if ( (dst) ) {\
	    (dst) += (length);\
	    switch((length)) {\
	    case 4:	*--(dst) = (code)&255; (code) >>= 8;\
	    case 3:	*--(dst) = (code)&255; (code) >>= 8;\
	    case 2:	*--(dst) = (code)&255; (code) >>= 8;\
	    case 1:	*--(dst) = (code)&255; (code) >>= 8;\
	    };\
	    (dst) += (length);\
	};\
    };\
}

#define	ADDWCODE(dst, maxdst, count, code) {\
    if ( (maxdst) > 0 ) {\
	(maxdst)-- ; (count)++ ;\
	if ( (dst) ) {\
	    *(dst)++ = (code);\
	}\
    }\
}


// @return ワイド文字数.
static int
euccharlen(const unsigned char* s, int bytelen)
{
    assert(s);

  unsigned char	ch;
  int		res = 0;

  while ((ch = *s++) && bytelen--) {
    res++;
    if (ch & 0x80) {
      if (ch == RK_SS3) {
	s++;
	bytelen--;
      }
      s++;
      bytelen--;
    }
  }
  return res;
}

/* RkCvtZen
 *	hankaku moji(ASCII+katakana) wo taiou suru zenkaku moji ni suru
 *	dakuten,handakuten shori mo okonau.
 * @return 書き込んだバイト数. ナル終端を含まない.
 */
int RkCvtZen(
    unsigned char* zen, int maxzen, const unsigned char* han, int maxhan)
{
    unsigned char	*z = zen;
    const unsigned char* h = han;
    const unsigned char* H = han + maxhan;
  unsigned short       hi, lo;
  int byte;
  int 		count = 0;
  unsigned long	code;

    if ( --maxzen <= 0 )
        return count;
    while ( h < H ) {
        hi = *h++;
        byte = 2;
        if ( hi == 0x8e ) {	/* hankaku katakana */
            if ( !(code = hiragana[lo = *h++]) )
                code = (hi<<8)|lo;
            byte = (code>>8) ? 2 : 1;
            if ( (code>>8) == 0xa4 ) {
	code |= 0x100;
	/* dakuten/handakuten ga tuku baai */
	if ( h + 1 < H && h[0] == 0x8e ) {
	  lo = h[1];
          switch( LOMASK(code) )
            {
	  case 0xa6: /* u */
	    if ( lo == 0xde ) code = 0xa5f4, h += 2;
	    break;
	    /* ha */case 0xcf: case 0xd2: case 0xd5: case 0xd8: case 0xdb:
	    if ( lo == 0xdf ) {
	      code += 2, h += 2;
	      break;
	    }
          case 0xab: case 0xad: case 0xaf: case 0xb1: case 0xb3: /* ka */
          case 0xb5: case 0xb7: case 0xb9: case 0xbb: case 0xbd: /* sa */
          case 0xbf: case 0xc1: case 0xc4: case 0xc6: case 0xc8: /* ta */
            if ( lo == 0xde ) {
	      code += 1; h += 2;
	      break;
	    };
	  };
	};
      };
    }
    else if ( hi == 0x8f ) { // 補助漢字
      ADDCODE(z, maxzen, count, hi, 1);
      code = (((unsigned long) h[0]) << 8) | ((unsigned long) h[1]); h += 2;
      byte = 2;
    }
    else if ( hi & 0x80 ) // JIS X 0208
      code = (hi<<8) | *h++;
    else {
      if ( !(code = hiragana[hi]) )
	code = hi;
      byte = (code>>8) ? 2 : 1;
    }
    ADDCODE(z, maxzen, count, code, byte);
  } // while

  if ( z )
    *z = 0;
  return count;
}

/* RkCvtHan
 *	zenkaku kana moji wo hankaku moji ni suru
 */
int
RkCvtHan(unsigned char* han, int maxhan, const unsigned char* zen, int maxzen)
{
    unsigned char	*h = han;
    const unsigned char* z = zen;
    const unsigned char* Z = zen + maxzen;
  unsigned short hi, lo;
  int byte;
  int 		count = 0;
    uint32_t code; // 4バイト必要.

  if ( --maxhan <= 0 )
    return 0;
  while ( z < Z ) {
    hi = *z++;
    byte = 1;
    switch(hi)
    {
    case	0xa1:	/* kigou */
      lo = *z++;
      if ( !(code = hankaku[lo&0x7f]) )
	code = (hi<<8)|lo;
      byte = (code>>8) ? 2 : 1;
      break;
    case	0xa3:	/* eisuuji */
      lo = *z++;
      if ( 0xb0 <= lo && lo <= 0xb9 ) code = (lo - 0xb0) + '0';
      else
	if ( 0xc1 <= lo && lo <= 0xda ) code = (lo - 0xc1) + 'A';
	else
	  if ( 0xe1 <= lo && lo <= 0xfa ) code = (lo - 0xe1) + 'a';
	  else
	    code = (hi<<8)|lo, byte = 2;
      break;
    case	0xa4:	/* hiragana */
    case	0xa5:	/* katakana */
      lo = *z++;
      if ( (code = hankaku[lo]) &&
	  (lo <= (unsigned short)(hi == 0xa4 ? 0xf3 : 0xf6)) ) {
	if ( code>>8 ) {
	  code = 0x8e000000|((code>>8)<<16)|0x00008e00|LOMASK(code);
	  byte = 4;
	}
	else {
	  code = 0x00008e00|LOMASK(code);
	  byte = 2;
        }
      }
      else
	code = (hi<<8)|lo, byte = 2;
      break;
    default:
      if (hi == 0x8f) {
	ADDCODE(h, maxhan, count, hi, 1);
	code = (((unsigned long) z[0]) << 8) | ((unsigned long) z[1]); z += 2;
	byte = 2;
      }
      else if ( hi & 0x80 ) { 	/* kanji */
	code = (hi<<8)|(*z++);
	byte = 2;
      }
      else
	switch(hi) {
	  /*
	    case	',':	code = 0x8ea4; byte = 2; break;
	    case	'-': 	code = 0x8eb0; byte = 2; break;
	    case	'.': 	code = 0x8ea1; byte = 2; break;
	    */
	default:	code = hi; break;
	};
      break;
    };
    ADDCODE(h, maxhan, count, code, byte);
  }

  if ( h )
    *h = 0;
  return count;
}

/* RkCvtKana/RkCvtHira
 *	zenkaku hiragana wo katakana ni suru
 */
int
RkCvtKana(
    unsigned char* kana, int maxkana, const unsigned char* hira, int maxhira)
{
  unsigned char	*k = kana;
  const unsigned char	*h = hira;
  const unsigned char	*H = hira + maxhira;
  unsigned short hi;
  int byte;
  int 		count = 0;
  unsigned long	code;

  if ( --maxkana <= 0 )
    return 0;
  while ( h < H ) {
    hi = *h++;
    if (hi == 0x8f) {
      ADDCODE(k, maxkana, count, hi, 1);
      code = (((unsigned long) h[0]) << 8) | ((unsigned long) h[1]); h += 2;
      byte = 2;
    }
    else if ( hi & 0x80 ) {
      int	dakuon;

      code = (hi == 0xa4) ? (0xa500|(*h++)) : ((hi<<8)|(*h++));
      byte = 2;
      /* hiragana U + " */
      dakuon = ( h + 1 < H &&
            ((((unsigned long) h[0])<<8)| ((unsigned long) h[1])) == 0xa1ab );
      if ( hi == 0xa4 && code == 0xa5a6 && dakuon ) {
        code = 0xa5f4;
        h += 2;
      }
    }
    else {
      code = hi; byte = 1;
    }
    ADDCODE(k, maxkana, count, code, byte);
  }

  if ( k )
    *k = 0;
  return count;
}

int
RkCvtHira(unsigned char* hira, int maxhira, const unsigned char* kana, int maxkana)
{
    unsigned char	*h = hira;
    const unsigned char	*k = kana;
    const unsigned char	*K = kana + maxkana;
    unsigned short	hi;
    unsigned short	byte;
  int 			count = 0;
  unsigned long		code;

    if ( --maxhira <= 0 )
        return 0;
    while ( k < K ) {
        hi = *k++;
        if (hi == 0x8f) {
            ADDCODE(h, maxhira, count, hi, 1);
            code = (((unsigned long) k[0]) << 8) | ((unsigned long) k[1]);
            k += 2;
            byte = 2;
        }
        else if ( hi & 0x80 ) {
            code = (hi == 0xa5) ? (0xa400|(*k++)) : ((hi<<8)|(*k++));
            byte = 2;
            /* katakana U + " */
            if ( code == 0xa4f4 ) {	/* u no dakuon */
                code = 0xa4a6a1ab;
                byte = 4;
            }
            else if ( code == 0xa4f5 )
                code = 0xa4ab;
            else if ( code == 0xa4f6 )
                code = 0xa4b1;
        }
        else {
            code = hi; byte = 1;
        }
    ADDCODE(h, maxhira, count, code, byte);
  };
  if ( h )
    *h = 0;
  return count;
}

int
RkCvtNone(unsigned char* dst, int maxdst, const unsigned char* src, int maxsrc)
{
    unsigned char	*d = dst;
    const unsigned char	*s = src;
    const unsigned char	*S = src + maxsrc;
    unsigned short	byte;
    int 		count = 0;
    unsigned long	code;

    if ( --maxdst <= 0 )
        return 0;
    while ( s < S ) {
        code = *s++;
        byte = 1;
        if (code == 0x8f) {
            ADDCODE(d, maxdst, count, code, 1);
            code = (((unsigned long) s[0]) << 8) | ((unsigned long) s[1]);
            s += 2;
            byte = 2;
        }
        else if ( code & 0x80 ) {
            code = (code<<8)|(*s++); byte = 2;
        }
    ADDCODE(d, maxdst, count, code, byte);
    }
  if ( d )
    *d = 0;
  return count;
}


// 使われているのは この kana.c 内だけ.
int
SJistowcs(cannawc* wc_return, int maxwc, const unsigned char* sj, int maxsj)
{
    cannawc* e = wc_return;
    cannawc* ee = wc_return + maxwc;
    const unsigned char* s = sj;
    const unsigned char* S = sj + maxsj;
    unsigned short	hi, lo;
    unsigned short	byte;
    int 		count = 0;
    unsigned long	code;

    if ( --maxwc <= 0 )
	return 0;

    while ( s < S ) {
	hi = *s++;
	if ( hi <= 0x7f )  			/* ascii */
	    code = hi, byte = 1;
	else
	if ( 0xa0 <= hi && hi <= 0xdf ) 	/* hankaku katakana */
	    code = hi, byte = 2;
        else
        if (0xf0 <= hi && hi <= 0xfc) {		/* gaiji */
            hi -= 0xf0;
            hi = 2*hi + 0x21;
            if ((lo = *s++) <= 0x9e) {
                if (lo < 0x80)
                    lo++;
                lo -= 0x20;
            }
            else {
                hi++;
                lo -= 0x7e;
            }
            code = 0x8000 | (hi<<8) | lo, byte = 3;
        }
	else {
	    hi -= (hi <= 0x9f) ?  0x80 : 0xc0;
	    hi = 2*hi + 0x20;
	    if ( (lo = *s++) <= 0x9e ) {	/* kisuu ku */
		hi--;
		if ( 0x80 <= lo ) lo--;
		lo -= (0x40 - 0x21);
	    }
	    else 			/* guusuu ku */
		lo -= (0x9f - 0x21);
	    code = 0x8080|(hi<<8)|lo, byte = 2;
	};
	if (wc_return && e < ee) {
	  *e++ = (Wchar)code;
	}
    };
    if (wc_return && e && e < ee) {
      *e = 0;
    }
    return count;
}


/*
  Wcstosjis -- To convert Wchar string to SJIS string.
   This function should not copy after NULL character even if
   the srclen is too large
 */
int
Wcstosjis(unsigned char* dst, int dstlen, const cannawc* src, int srclen)
{
    int i, j;
    unsigned char *sjise_kanjip, sjise_area[2];
    Wchar codeset;
    Wchar wc;

  sjise_kanjip = sjise_area;

  for (i = 0, j = 0; src[i] != (Wchar)0 && j < dstlen && i < srclen ; i++) {
    wc = src[i];
    codeset = wc & 0x8080; /* to know what codeset is `wc' */

    switch (codeset) {
    case 0x0000: /* ASCII */
      /* continue to the following line ... */
    case 0x0080: /* Codeset 2, that is Katakana */
      if (dst) dst[j++] = (unsigned char)wc;
      break;

    case 0x8080: /* Codeset 1, that is Kanji */
      if (j + 2 > dstlen) {
	return j; /* overflow */
      }
      sjise_kanjip[0] = ((wc >> 8) & 0x7f);
      sjise_kanjip[1] = (wc & 0x7f);
      sjise_kanjip[1] = ((sjise_kanjip[0] % 2) ?
			 (sjise_kanjip[1] + 0x1f +
			  ((sjise_kanjip[1] < 0x60) ? 0 : 1)) :
			 (sjise_kanjip[1] + 0x7e));
      sjise_kanjip[0] = ((sjise_kanjip[0] < 0x5f) ?
			 ((sjise_kanjip[0] - 0x21) / 2 + 0x81) :
			 ((sjise_kanjip[0] - 0x5f) / 2 + 0xe0));
      if (dst) {
	dst[j++] = sjise_kanjip[0];
	dst[j++] = sjise_kanjip[1];
      }
      break;

    case 0x8000: /* Codeset 3 */
      if (j + 2 > dstlen) {
	return j; /* overflow */
      }
      sjise_kanjip[0] = ((wc >> 8) & 0x7f);
      sjise_kanjip[1] = (wc & 0x7fb);
      sjise_kanjip[1] = ((sjise_kanjip[0] % 2) ?
			 (sjise_kanjip[1] + 0x1f +
			  ((sjise_kanjip[1] < 0x60) ? 0 : 1)) :
			 (sjise_kanjip[1] + 0x7e));
      sjise_kanjip[0] = ((sjise_kanjip[0] - 0x21) / 2 + 0xf0);
      if (dst) {
	dst[j++] = sjise_kanjip[0];
	dst[j++] = sjise_kanjip[1];
      }
      break;
    }
  }
  if (j < dstlen && dst) {
    dst[j] = '\0';
  }
  return j;
}


/* RkEuc
 * 	shift jis --> euc
 */
int
RkCvtEuc(unsigned char* euc, int maxeuc, const unsigned char* sj, int maxsj)
{
    unsigned char	*e = euc;
    const unsigned char	*s = sj;
    const unsigned char	*S = sj + maxsj;
    unsigned short	hi, lo;
    unsigned short	byte;
  int 		count = 0;
  unsigned long	code;

  if ( --maxeuc <= 0 )
    return 0;

    while ( s < S ) {
        hi = *s++;
        if ( hi <= 0x7f ) { 			/* ascii */
            code = hi; byte = 1;
        }
        else if ( 0xa0 <= hi && hi <= 0xdf ) {	/* hankaku katakana */
            code = 0x8e00|hi; byte = 2;
        }
        else if (0xf0 <= hi && hi <= 0xfc) {		/* gaiji */
            hi -= 0xf0;
            hi = 2*hi + 0x21;
            if ((lo = *s++) <= 0x9e) {
                if (lo < 0x80)
                    lo++;
                lo -= 0x20;
            }
            else {
                hi++;
                lo -= 0x7e;
            }
            code = 0x8f8080 | (hi<<8) | lo; byte = 3;
        }
        else {
            hi -= (hi <= 0x9f) ?  0x80 : 0xc0;
            hi = 2*hi + 0x20;
            if ( (lo = *s++) <= 0x9e ) {	/* kisuu ku */
                hi--;
                if ( 0x80 <= lo ) lo--;
                lo -= (0x40 - 0x21);
            }
            else 			/* guusuu ku */
                lo -= (0x9f - 0x21);
	code = 0x8080|(hi<<8)|lo, byte = 2;
      };
    ADDCODE(e, maxeuc, count, code, byte);
  };
  if ( e )
    *e = 0;
  return count;
}


/* RkCvtSuuji
 * 	arabia suuji wo kansuuji ni kaeru
 */
static cannawc suujinew[] = {
    0xa1bb, 0xb0ec, 0xc6f3, 0xbbb0, 0xbbcd,
    0xb8de, 0xcfbb, 0xbcb7, 0xc8ac, 0xb6e5,
};
static cannawc suujiold[] = {
  0xa1bb, 0xb0ed, 0xc6f5, 0xbbb2, 0xbbcd,
  0xb8e0, 0xcfbb, 0xbcb7, 0xc8ac, 0xb6e5,
};
static cannawc kurai4[] = {
  0, 0xcbfc, 0xb2af, 0xc3fb, 0xb5fe, 0,
};

static cannawc kurai3new[] = { 0, 0xbdbd, 0xc9b4, 0xc0e9, };
static cannawc kurai3old[] = { 0, 0xbdbd, 0xc9b4, 0xc0e9, };

int
RkwCvtSuuji(cannawc* dst, int maxdst, const cannawc* src, int maxsrc, int format)
{
    int	count;
    int	i, j, k;
  int	digit[4], pend;
  Wchar	code, tmp;
    cannawc* d = dst;
    const cannawc* s = src + maxsrc - 1;

  if ( --maxdst <= 0 )
    return 0;
  /* 有効な桁数を数える */
  pend = 0;
  for ( count = k = 0; s >= src; k++ ) {
    int	dec, thru = *s;

    if ( thru & 0x8080 ) {
      if ( !((Wchar)0xa3b0 <= *s && *s <= (Wchar)0xa3b9) )
	break;
      dec = *s-- - 0xa3b0;
    }
    else {
      if ( !((Wchar)'0' <= *s && *s <= (Wchar)'9') )
	break;
      dec = *s-- - '0';
    }

    switch(format) {
      /* simple */
    case SUUJI_THROUGH:	/* sanyou suuji */
      code = thru;
      ADDWCODE(d, maxdst, count, code);
      break;
    case SUUJI_HANKAKU:	/* sanyou suuji */
      code = dec + '0';
      if (code == thru) {
	return 0;
      }
      ADDWCODE(d, maxdst, count, code);
      break;
    case SUUJI_ZENKAKU:	/* sanyou suuji */
      code = hiragana[dec + '0'];
      if (code == thru) {
	return 0;
      }
      ADDWCODE(d, maxdst, count, code);
      break;
      /* kanji kurai dori */
    case SUUJI_SIMPLEKANJI:	/* kanji suuji */
      code = suujinew[dec];
      ADDWCODE(d, maxdst, count, code);
      break;
    case SUUJI_FULLKANJI:
    case SUUJI_FULLKANJITRAD:
    case SUUJI_WITHKANJIUNIT:	/* 12 O 3456 M 7890 */
      digit[pend++] = dec;
      if ( pend == 4 ) {
	while ( pend > 0 && digit[pend - 1] == 0 )
	  pend--;
	if ( pend ) {
	  /* kurai wo shuturyoku */
	  code = kurai4[k/4];
	  if (code)
	    ADDWCODE(d, maxdst, count, code)
	  else
	    if ( k >= 4 )
	      return 0;

	  for ( i = 0; i < pend; i++ )
	    switch(format) {
	    case SUUJI_FULLKANJI:
	      if ( digit[i] ) {
		code = kurai3new[i];
		if (code)
		  ADDWCODE(d, maxdst, count, code);
		if ( i == 0 || (digit[i] > 1) ) {
		  code = suujinew[digit[i]];
		  ADDWCODE(d, maxdst, count, code);
		}
	      }
	      break;
	    case SUUJI_FULLKANJITRAD:
	      if ( digit[i] ) {
		code = kurai3old[i];
		if (code)
		  ADDWCODE(d, maxdst, count, code);
		code = suujiold[digit[i]];
		ADDWCODE(d, maxdst, count, code);
	      };
	      break;
	    case SUUJI_WITHKANJIUNIT:
	      code = hiragana[digit[i]+'0'];
	      ADDWCODE(d, maxdst, count, code);
	      break;
	    }
	}
	pend = 0;
      }
      break;
    case SUUJI_WITHCOMMA: /* 1,234,567,890 */
      if ( k && k%3 == 0 ) {
	code = hiragana[','];
	ADDWCODE(d, maxdst, count, code);
      }
      code = hiragana[dec + '0'];
      ADDWCODE(d, maxdst, count, code);
      break;
    default:
      return 0;
    };
  };

  if (format == SUUJI_FULLKANJI || format == SUUJI_FULLKANJITRAD ||
      format == SUUJI_WITHKANJIUNIT) {
    while ( pend > 0 && digit[pend - 1] == 0 )
      pend--;
    if ( pend ) {
      code = kurai4[k/4];
      if (code)
	ADDWCODE(d, maxdst, count, code)
      else
	if ( k >= 4 )
	  return 0;
      for ( i = 0; i < pend; i++ )
	switch(format) {
	case SUUJI_FULLKANJI:
	  if ( digit[i] ) {
	    code = kurai3new[i];
	    if (code)
	      ADDWCODE(d, maxdst, count, code);
	    if ( i == 0 || (digit[i] > 1) ) {
	      code = suujinew[digit[i]];
	      ADDWCODE(d, maxdst, count, code);
	    };
	  };
	  break;
	case SUUJI_FULLKANJITRAD:
	  if ( digit[i] ) {
	    code = kurai3old[i];
	    if (code)
	      ADDWCODE(d, maxdst, count, code);
	    code = suujiold[digit[i]];
	    ADDWCODE(d, maxdst, count, code);
	  };
	  break;
	case SUUJI_WITHKANJIUNIT:
	  code = hiragana[digit[i]+'0'];
	  ADDWCODE(d, maxdst, count, code);
	  break;
	}
    }
  }

  if ( dst ) {
    *d = 0;
    for ( i = 0, j = count - 1; i < j; i++, j-- ) {
      tmp = dst[i]; dst[i] = dst[j]; dst[j] = tmp;
    }
  }
  return count;
}



///////////////////////////////////////////////////////////////////
/* ワイドキャラクタ対応関数 */

//#define CBUFSIZE     512
// RK/util.c で定義され、それなりに使われている.
//unsigned char	*ustoeuc();
//Wchar		*euctous();

// @return -1 malloc() failed.
int
RkwCvtHan(cannawc* dst, int maxdst, const cannawc* src, int srclen)
{
    // 中間バッファ. 1文字当たり最大3バイト. 濁音1文字は4バイトになる.
    int len;
    unsigned char *cbuf, *cbuf2;
    int buflen = srclen * 3 + 1;
    cbuf = (unsigned char*) malloc( buflen );
    int buf2len = srclen * 4 + 1;
    cbuf2 = (unsigned char*) malloc( buf2len );
    if (!cbuf || !cbuf2) {
        free(cbuf);
        free(cbuf2);
        return -1;
    }

    len = CNvW2E(src, srclen, cbuf, buflen);
    len = RkCvtHan(cbuf2, buf2len, cbuf, len);
    if (len > 0) {
        if (dst)
            len = euctous(cbuf2, len, dst, maxdst) - dst;
        else
            len = euccharlen(cbuf2, len);
    }

    free( cbuf2 );
    free( cbuf );
    return len;
}

// @return -1 malloc() failed.
int
RkwCvtHira(cannawc* dst, int maxdst, const cannawc* src, int srclen)
{
    int len;
    unsigned char *cbuf, *cbuf2;
    int buflen = srclen * 3 + 1;
    cbuf = (unsigned char*) malloc( buflen );
    cbuf2 = (unsigned char*) malloc( buflen );
    if (!cbuf || !cbuf2) {
        free( cbuf);
        free( cbuf2);
        return -1;
    }

    len = CNvW2E(src, srclen, cbuf, buflen);
    len = RkCvtHira(cbuf2, buflen, cbuf, len);
    if (len > 0) {
        if (dst)
            len = euctous(cbuf2, len, dst, maxdst) - dst;
        else
            len = euccharlen(cbuf2, len);
    }

    free( cbuf);
    free( cbuf2);
    return len;
}

int
RkwCvtKana(cannawc* dst, int maxdst, const cannawc* src, int srclen)
{
    assert( src );

    int	len;
    unsigned char *cbuf, *cbuf2;
    int buflen = srclen * 3 + 1;
    cbuf = (unsigned char *)malloc( buflen );
    cbuf2 = (unsigned char *)malloc( buflen );
    if (!cbuf || !cbuf2) {
        free( cbuf);
        free( cbuf2);
        return -1;
    }

    len = CNvW2E(src, srclen, cbuf, buflen);
    len = RkCvtKana(cbuf2, buflen, cbuf, len);
    if (len > 0) {
        if (dst)
      len = euctous(cbuf2, len, dst, maxdst) - dst;
        else
      len = euccharlen(cbuf2, len);
    }

    free( cbuf);
    free( cbuf2);
  return len;
}

// Half-width -> Full-width.
// @return -1 malloc() failed.
int
RkwCvtZen(cannawc* dst, int maxdst, const cannawc* src, int srclen)
{
    int len;
    unsigned char *cbuf, *cbuf2;
    int buflen = srclen * 3 + 1;
    cbuf = (unsigned char *)malloc( buflen );
    cbuf2 = (unsigned char *)malloc( buflen );
    if (!cbuf || !cbuf2) {
        free( cbuf);
        free( cbuf2);
        return -1;
    }

    len = CNvW2E(src, srclen, cbuf, buflen);
    len = RkCvtZen(cbuf2, buflen, cbuf, len);
    if (len > 0) {
        if (dst)
            len = euctous(cbuf2, len, dst, maxdst) - dst;
        else
      len = euccharlen(cbuf2, len);
    }

    free(cbuf2);
    free(cbuf);
    return len;
}

int
RkwCvtNone(cannawc* dst, int maxdst, const cannawc* src, int srclen)
{
  int i;
    int len = (maxdst - 1) < srclen ? maxdst - 1 : srclen;

    if (dst) {
        for (i = 0 ; i < len ; i++)
            *dst++ = *src++;
        *dst = 0;
    }
  return len;
}


// romaji -> kana.
int RkwMapPhonogram(struct RkRxDic *romaji, cannawc *dst, int maxdst,
		const cannawc* src, int srclen, cannawc key, int flags,
		int *ulen, int *dlen, int *tlen, int *rule)
{
  int status = 0;
  char tmpch;
  int len, ret, fdlen, fulen, ftlen;
    unsigned char *cbuf1, *cbuf2;
    //cannawc *wbuf;

    int buf1len = srclen * 3 + 1;
    cbuf1 = (unsigned char*) malloc( buf1len ); // wc -> euc-jp
    int buf2len = srclen * 3 * 2 + 1;
    cbuf2 = (unsigned char*) malloc( buf2len ); // romaji -> kana
    //wbuf = (cannawc*) malloc(sizeof(cannawc) * CBUFSIZE);
    if ( !cbuf1 || !cbuf2 /*|| !wbuf*/ ) {
        free(cbuf1);
        free(cbuf2);
        //free( wbuf );
        return -1;
    }

    len = CNvW2E(src, srclen, cbuf1, buf1len );
    status = RkMapPhonogram(romaji, cbuf2, buf2len, cbuf1, len,
			  (unsigned) key, flags,
			  &fulen, &fdlen, &ftlen, rule);
  tmpch = cbuf2[fdlen];
  cbuf2[fdlen] = '\0';
  ret = MBstowcs(dst, cbuf2, maxdst);
  cbuf2[fdlen] = tmpch;
  if (dlen) {
    *dlen = ret;
  }
  cbuf2[fdlen + ftlen] = (unsigned char)0;
  ret = MBstowcs(dst + ret, cbuf2 + fdlen, maxdst - ret);
  if (tlen) {
    *tlen = ret;
  }
    if (ulen) {
        cbuf1[fulen] = '\0';
        *ulen = euccharlen(cbuf1, fulen); //MBstowcs(wbuf, cbuf1, CBUFSIZE);
    }

  free(cbuf2);
  free(cbuf1);
  return status;
}


// 文字列すべてのローマ字 -> Kana.
// @return -1 malloc() failed.
int RkwCvtRoma(struct RkRxDic* romaji, cannawc* dst, int maxdst,
        const cannawc* src, int srclen, int flags)
{
    assert(src);

    int ret = 0, len;
    unsigned char *cbuf1, *cbuf2;

    int buf1len = srclen * 3 + 1;
    cbuf1 = (unsigned char*) malloc( buf1len );
    int buf2len = srclen * 3 * 2 + 1;
    cbuf2 = (unsigned char*) malloc( buf2len );
    if ( !cbuf1 || !cbuf2) {
        free(cbuf1);
        free(cbuf2);
        return -1;
    }

    len = CNvW2E(src, srclen, cbuf1, buf1len);
    len = RkCvtRoma(romaji, cbuf2, buf2len, cbuf1, len, flags);
    cbuf2[len] = (unsigned char)0;
    ret = MBstowcs(dst, cbuf2, maxdst);
    dst[ret] = (cannawc)0;

    free(cbuf2);
    free(cbuf1);
    return ret;
}
