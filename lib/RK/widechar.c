
#include "RKintern.h"
#include <assert.h>
#include "canna/sglobal.h"

size_t
WStrlen(const cannawc* ws)
{
    assert(ws);

    int res = 0;
    while (*ws++)
        res++;
    return res;
}


// @return 見つからなかった場合, NULL.
cannawc*
ushortmemchr(const cannawc* ws, cannawc ch, size_t len)
{
    const cannawc *p, *end;
    for (p = ws, end = ws + len; p < end; ++p) {
        if (*p == ch)
            return (cannawc*) p;
    }
    return NULL;
}


// シグネチャが違う.
// @return 書き込んだワイド文字数 (ナル終端を含まない). WStrlen(wsrc) と同じ.
int
ushortstrcpy(cannawc* wdest, const cannawc* wsrc)
{
    assert(wdest);
    assert(wsrc);

    int ret;
    // ws1 と ws2 が重なっている場合を考慮.
    if ( wsrc < wdest ) {
        int len = WStrlen(wsrc);
        ret = len;
        wdest[len] = 0;
        while (len--)
            wdest[len] = wsrc[len];
    }
    else if ( wdest < wsrc ) {
        ret = 0;
        while (*wsrc) {
            *wdest++ = *wsrc++;
            ret++;
        }
        *wdest = 0;
    }

    return ret;
}


cannawc*
WStrcpy(cannawc* ws1, const cannawc* ws2)
{
    assert(ws2);
    assert(ws1);

    ushortstrcpy(ws1, ws2);
    return ws1;
}

// @param n wdest の大きさ.
// @return 書き込んだワイド文字数. ナル終端を含まない.
int
ushortstrncpy(cannawc* wdest, const cannawc* wsrc, int n)
{
    assert(wsrc);

    int res = 0;
    while (res < n - 1 && (*wdest = *wsrc) != 0) {
        wdest++; wsrc++; res++;
    }
    *wdest = 0;
    return res;
}


cannawc*
WStrncpy(cannawc* ws1, const cannawc* ws2, size_t destsize)
{
    assert(ws1);
    assert(ws2);

    cannawc* ws;

    if (ws1 == ws2)
        ws1[destsize - 1] = 0;
    else if ( ws2 < ws1 ) {
        int len = WStrlen(ws2);
        if (destsize - 1 < len) len = destsize - 1;
        ws1[len] = 0;
        while (len--)
            ws1[len] = ws2[len];
    }
    else if (ws1 < ws2) {
        int i = 0;
        ws = ws1;
        while ( i++ < destsize - 1 && *ws2)
            *ws++ = *ws2++;
        *ws = 0;
    }

    return ws1;
}


cannawc*
WStrcat(cannawc* ws1, const cannawc* ws2)
{
    cannawc* ws;

  ws = ws1;
  while (*ws) {
    ws++;
  }
  WStrcpy(ws, ws2);
  return ws1;
}

int
WStrcmp(const cannawc* w1, const cannawc* w2)
{
  while (*w1 && *w1 == *w2) {
    w1++;
    w2++;
  }
  return(*w1 - *w2);
}

int
WStrncmp(const cannawc* w1, const cannawc* w2, size_t n)
{
    if (n == 0) return 0;

  while (--n && *w1 && *w1 == *w2) {
    w1++;
    w2++;
  }
  return *w1 - *w2;
}


/**
 * WWhatGPlain -- どのグラフィックプレーンの文字か？
 * @return 0 : G0 ASCII
           1 : G1 漢字(JISX0208)
           2 : G2 半角カタカナ(JISX0201)
           3 : G3 外字(補助漢字 JISX0212)
 */
int
WWhatGPlain(cannawc wc)
{
    switch (((unsigned long)wc) & 0x8080)
    {
    case 0x0000:
        return 0;
    case 0x8080:
        return 1;
    case 0x0080:
        return 2;
    case 0x8000:
        return 3;
    }
}

// いくつかの場所で使われている.
int
WIsG0(cannawc wc)
{
    return (WWhatGPlain(wc) == 0);
}

int
WIsG1(cannawc wc)
{
    return (WWhatGPlain(wc) == 1);
}

int
WIsG2(cannawc wc)
{
    return (WWhatGPlain(wc) == 2);
}

int
WIsG3(cannawc wc)
{
    return (WWhatGPlain(wc) == 3);
}


/**
 * RkCvtWide - EUC-JP -> cannawc
 * @return ワイド文字の数. ナル終端を含まない.
 *         不正なシーケンスだった場合, -1.
 * @param dst NULLの場合、文字数カウントのみを行う. この場合, maxdst は無視す
 *            る.
 */
// 定義されているのはここだけ。RK/tempdic.c で使われている.
int
RkCvtWide(cannawc* dest, int destlen, const unsigned char* src, int srclen)
{
    assert( src );

    cannawc* d = dest;
    const unsigned char* s = src;
    const unsigned char* const S = src + srclen;
    int count = 0;
    unsigned long code;

    for ( count = 0; (code = *s) != 0 && s < S; count++ ) {
        if (dest) {
            if (count + 1 >= destlen)
                break;
        }

        s++;
        if (code == RK_SS2 && s < S) { /* G2: hankaku katakana */
            if ( !(*s & 0x80) )
                return -1;
            code = 0x0080 | (*s++ & 0x7f);
        }
        else if (code == RK_SS3 && s + 1 < S) { /* G3: gaiji */
            if ( !(s[0] & 0x80) || !(s[1] & 0x80) )
                return -1;
            code = 0x8000 | (((s[0] << 8) | s[1]) & 0x7f7f);
            s += 2;
        }
        else if ( (code & 0x80) != 0) { // G1: Kanji
            if ( !(*s & 0x80) )
                return -1;
            code = 0x8080 | (((code << 8) | *s++) & 0x7f7f);
        }

        if (dest)
            *d++ = code;
    }

    if ( dest && (count < destlen) )
        *d = 0;
    return count;
}


// それなりに使われている.
cannawc*
euctous(const unsigned char* src, int srclen, cannawc* dest, int destlen)
{
    int r = RkCvtWide(dest, destlen, src, srclen);
    if (r == -1)
        return NULL;
    return dest + r;
}

// 引数の並びが違う
int euc2ushort(const unsigned char* src, int srclen, cannawc* dest, int destlen)
{
    return RkCvtWide(dest, destlen, src, srclen);
}

// @param dest  If dest is NULL, destlen is ignored.
// @return 変換された wide-character の数. ナル終端は含まない.
//         Invalid なシーケンスだった場合は, -1.
size_t
CANNA_mbstowcs(cannawc* dest, const unsigned char* src, size_t destlen)
{
    return RkCvtWide(dest, destlen, src, strlen(src));
}


// cannawc -> EUC-JP.
// 引数の順序が違う.
size_t
CNvW2E(const cannawc* src, int srclen, unsigned char* dest, size_t destlen)
{
    assert(src);

    int j = 0;
    cannawc wc;
    const cannawc* s = src;

    while ( (wc = *s++) && (s - src) < srclen ) {
        switch (wc & 0x8080)
        {
        case 0:  /* G0: ASCII */
            if (dest) {
                if ( j + 1 >= destlen)
                    goto ret;
                dest[j] = wc & 0xff;
            }
            j++;
            break;
        case 0x8080: /* G1: 漢字 */
            if (dest) {
                if ( j + 2 >= destlen)
                    goto ret;
                dest[j + 0] = (wc >> 8) & 0xff;
                dest[j + 1] = wc & 0xff;
            }
            j += 2;
            break;
        case 0x0080:  /* G2: 半角カナ */
            if (dest) {
                if ( j + 2 >= destlen)
                    goto ret;
                dest[j + 0] = 0x8e; /* SS2 */
                dest[j + 1] = wc & 0xff;
            }
            j += 2;
            break;
        case 0x8000: /* G3: Hojo-kanji */
            if (dest) {
                if ( j + 3 >= destlen)
                    goto ret;
                dest[j + 0] = 0x8f; /* SS3 */
                dest[j + 1] = (wc >> 8) & 0xff;
                dest[j + 2] = (wc & 0xff) | 0x80;
            }
            j += 3;
            break;
        }
    }
 ret:
    if (dest && j < destlen) dest[j] = 0;

    return j;
}


/* RkCvtNarrow
 *
 */
int
RkCvtNarrow(unsigned char* dest, int maxdst, const cannawc* src, int maxsrc)
{
    return CNvW2E(src, maxsrc, dest, maxdst);
}


size_t
CANNA_wcstombs(unsigned char* dest, const cannawc* src, size_t destlen)
{
    assert(src);
    return CNvW2E(src, WStrlen(src), dest, destlen);
}


// CNvW2E() とほとんど同じだが、戻り値だけ違う。
// それなりに使われている.
unsigned char *
ustoeuc(const cannawc* src, int srclen, unsigned char* dest, int destlen)
{
    size_t r = CNvW2E(src, srclen, dest, destlen);
    return dest + r;
}


// @return min(ワイド文字の文字数, len). ナル終端は含まない.
int
HowManyChars(const cannawc* yomi, int len)
{
    assert(yomi);

    int res = 0;
    while (res < len && *yomi++)
        res++;

    return res;
}

// ワイド文字を多バイトに変換するときに, 何バイト必要か.
int
ushort2eucsize(const cannawc* yomi, int len)
{
    int chlen, bytelen;

    for (chlen = 0, bytelen = 0; chlen < len; chlen++) {
        Wchar ch = yomi[chlen];

        if (us_iscodeG0(ch))
            bytelen++;
        else if (us_iscodeG3(ch))
            bytelen += 3;
        else   // G1, G2
            bytelen += 2;
    }
    return bytelen;
}
