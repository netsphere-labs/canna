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

#if !defined(lint) && !defined(__CODECENTER__)
static char rcsid[]="@(#) 102.1 $Id: RKroma.c,v 1.4.2.1 2004/04/26 22:49:21 aida_s Exp $";
#endif

/* LINTLIBRARY */
#include "canna.h"

#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>

#define S2TOS(s2)	(((unsigned short)(s2)[0]<<8)|(s2)[1])
#define L4TOL(l4)\
        ((((((((uint32_t) ((unsigned char)(l4)[0])) << 8) | \
                ((uint32_t) ((unsigned char)(l4)[1])))  << 8)  | \
                ((uint32_t) ((unsigned char)(l4)[2])))  << 8)      | \
                ((uint32_t) ((unsigned char)(l4)[3])))

#ifdef JAPANESE_SORT

struct romaRec {
  unsigned char *roma;
  unsigned char bang;
};


// for qsort()
static int compar(const void* p, const void* q)
{
    const unsigned char* s = ((const struct romaRec*) p)->roma;
    const unsigned char* t = ((const struct romaRec*) q)->roma;

    while ( *s == *t ){
        if (*s)
            s++, t++;
        else
            return 0;
    }
    return ((int)*s) - ((int)*t);
}
#endif /* JAPANESE_SORT */


// @return 成功 = 0, エラー = -1
static int readHeader(struct RkRxDic* rdic, FILE* dicfd)
{
    char magic[3];
    unsigned char hdrbuf[8];
    int hdrsize;

    if (fread(magic, 1, 2, dicfd) != 2)
        return -1;
    magic[2] = '\0';
    if (!strcmp(magic, "KP")) {
        rdic->dic = RX_KPDIC;
        hdrsize = 4;
    }
/*  else if (!strcmp(magic, "RD")) {  // Old format. もはや使われていない.
        rdic->dic = RX_RXDIC;
        hdrsize = 4;
    } */
    else if (!strcmp(magic, "PT")) {  // flag_large
        rdic->dic = RX_PTDIC;
        hdrsize = 8;
    }
    else
        return -1;

    if ( fread(hdrbuf, 1, hdrsize, dicfd) != hdrsize)
        return -1;
    if (hdrsize == 4) {
	rdic->nr_strsz = S2TOS(hdrbuf);
	rdic->nr_nkey = S2TOS(hdrbuf + 2);
    } else {
	rdic->nr_strsz = L4TOL(hdrbuf);
	rdic->nr_nkey = L4TOL(hdrbuf + 4);
    }
    return 0;
}


// ローマ字かなテーブルファイルを読み込む.
// @return If failed, NULL.
struct RkRxDic *
RkwOpenRoma(const char* romaji)
{
    struct RkRxDic	*rdic;
#ifdef JAPANESE_SORT
    struct romaRec *tmp_rdic;
#endif

    rdic = (struct RkRxDic*) malloc(sizeof(struct RkRxDic));
    if (!rdic)
        return NULL;

    FILE* dic;
    unsigned char	*s;
    int	i, sz;

    fprintf(stderr, "romaji path = %s\n", romaji);
    if ( !(dic = fopen(romaji, "rb")) ) {
        free(rdic);
        return NULL;
    }
    if ( readHeader(rdic, dic) ) {
        fclose(dic);
        free( rdic);
        return NULL;
    }

    if (rdic->nr_strsz > 0) {
        rdic->nr_string = (unsigned char*) malloc(rdic->nr_strsz);
        if ( !rdic->nr_string ) {
            fclose(dic);
            free( rdic);
            return NULL;
        }

        sz = fread((char*)rdic->nr_string, 1, rdic->nr_strsz, dic);
        fclose(dic);
        if ( sz != rdic->nr_strsz ) {
            free( rdic->nr_string);
            free( rdic);
            return NULL;
        }
    }
    else {
        fprintf(stderr, "warning: nr_strsz = 0\n");
        rdic->nr_string = NULL;
    }

    if (rdic->nr_nkey > 0) {
        rdic->nr_keyaddr =
	    (unsigned char **)calloc(rdic->nr_nkey,
				     sizeof(unsigned char *));
        if ( !rdic->nr_keyaddr ) {
            free( rdic->nr_string);
            free( rdic);
            return NULL;
        }
    }
    else {
        fprintf(stderr, "warning: nr_nkey = 0\n");
        rdic->nr_keyaddr = NULL;
    }

    s = rdic->nr_string;

    /* バックトラックのトリガー文字のポインタ */
//  if (rdic->dic != RX_RXDIC) {
	  /* RXDIC以外 で nr_string が無いことはない */
	  rdic->nr_bchars = s;
	  while (*s++)
	    /* EMPTY */
	    ;

	  /* トリガー文字があるのなら、トリガールールもあるはず */
	  if (*rdic->nr_string && rdic->nr_nkey > 0) {
	    rdic->nr_brules = (unsigned char *)calloc((unsigned)rdic->nr_nkey,
                                                       sizeof(unsigned char));
	  }
	  else {
	    rdic->nr_brules = (unsigned char *)0;
	  }
//  }
//  else {
//	  rdic->nr_brules = (unsigned char *)0;
//  }

    /* ルールの読み込み */
    for ( i = 0; i < rdic->nr_nkey; i++ ) {
        rdic->nr_keyaddr[i] = s;
        while (*s++)
                /* EMPTY */   // ローマ字
            ;
        while (*s++)
                /* EMPTY */     // 出力かな
            ;
//	    if (rdic->dic != RX_RXDIC) {
        while ( *s > 0x19 )
            s++;      // temp部
        if (*s) { /* トリガールール */
            if (rdic->nr_brules) {
		  rdic->nr_brules[i] = (unsigned char)1;
            }
            *s = (unsigned char)'\0';  // temp をナル終端させておく
        }
        s++;
//	    }
    }

#ifdef JAPANESE_SORT
	tmp_rdic = (struct romaRec *)calloc((unsigned)rdic->nr_nkey,
                                              sizeof(struct romaRec));
	if (!tmp_rdic) {
          if (rdic->nr_string)
            (void)free((char *)rdic->nr_string);
          if (rdic->nr_keyaddr)
            (void)free((char *)rdic->nr_keyaddr);
	  if (rdic->nr_brules)
            (void)free((char *)rdic->nr_brules);
	  (void)free((char *)rdic);
	  return (struct RkRxDic *)NULL;
	}

        for (i = 0; i < rdic->nr_nkey; i++) {
	  tmp_rdic[i].roma = rdic->nr_keyaddr[i];
	  if (rdic->nr_brules)
	    tmp_rdic[i].bang = rdic->nr_brules[i];
	}

        qsort(tmp_rdic, rdic->nr_nkey, sizeof(struct romaRec), compar);

        for (i = 0; i < rdic->nr_nkey; i++) {
	  rdic->nr_keyaddr[i] = tmp_rdic[i].roma;
	  if (rdic->nr_brules)
	    rdic->nr_brules[i]  = tmp_rdic[i].bang;
	}
	free ((char *)tmp_rdic);
#endif /* JAPANESE_SORT */

    return rdic;
}


/* RkCloseRoma
 *	romaji henkan table wo tojiru
 */
void
RkwCloseRoma(struct RkRxDic* rdic)
{
    if ( rdic ) {
        if (rdic->nr_string) (void)free((char *)rdic->nr_string);
        if (rdic->nr_keyaddr) (void)free((char *)rdic->nr_keyaddr);
	if (rdic->nr_brules) (void)free((char *)rdic->nr_brules);
	(void)free((char *)rdic);
    };
}

struct RkRxDic *
RkOpenRoma(char* romaji)
{
    return RkwOpenRoma(romaji);
}

void
RkCloseRoma(struct RkRxDic* rdic)
{
    RkwCloseRoma(rdic);
}

/* RkMapRoma
 *	key no sentou wo saichou itti hou ni yori,henkan suru
 */
#define	xkey(roma, line, n) 	((roma)->nr_keyaddr[line][n])

struct rstat {
    int	start, end;	/* match sury key no hanni */
};

static
int
findRoma(rdic, m, c, n, flg)
struct RkRxDic	*rdic;
struct rstat	*m;
unsigned char	c;
int		n;
int		flg;
{
    register int	s, e;

    if (flg && 'A' <= c && c <= 'Z') {
      c += 'a' - 'A';
    }
    for(s = m->start; s < m->end; s++)
	if( c == xkey(rdic, s, n) )
	    break;
    for(e = s; e < m->end; e++)
	if( c != xkey(rdic, e, n) )
	    break;
    m->start	= s;
    m->end	= e;
    return e - s;
}
static
unsigned char	*
getKana(rdic, p, flags)
struct RkRxDic	*rdic;
int		p;
int		flags;
{
    register unsigned char	*kana;
    int				klen;
    static unsigned  char	tmp[256];

    for (kana = rdic->nr_keyaddr[p] ; *kana++ ; )
      /* EMPTY */
      ;

    klen = strlen((char *)kana);
    switch(flags&RK_XFERMASK) {
    default:
	(void)RkCvtNone(tmp, sizeof(tmp), kana, klen);
	return tmp;
    case RK_XFER:
	(void)RkCvtHira(tmp, sizeof(tmp), kana, klen);
	return tmp;
    case RK_HFER:
	(void)RkCvtHan(tmp, sizeof(tmp), kana, klen);
	return tmp;
    case RK_KFER:
	(void)RkCvtKana(tmp, sizeof(tmp), kana, klen);
	return tmp;
    case RK_ZFER:
	(void)RkCvtZen(tmp, sizeof(tmp), kana, klen);
	return tmp;
    };
}
static
unsigned char	*
getRoma(rdic, p)
struct RkRxDic	*rdic;
int		p;
{
    return rdic->nr_keyaddr[p];
}
/*ARGSUSED*/
static
unsigned char	*
getTSU(rdic, flags)
struct RkRxDic	*rdic;
int		flags;
{
    static unsigned char  hira_tsu[] = {0xa4, 0xc3, 0};
    static unsigned char  kana_tsu[] = {0xa5, 0xc3, 0};
    static unsigned char  han_tsu[] =  {0x8e, 0xaf, 0};

    switch(flags&RK_XFERMASK) {
    default:	  return hira_tsu;
    case RK_HFER: return han_tsu;
    case RK_KFER: return kana_tsu;
    };
}

int
RkMapRoma(rdic, dst, maxdst, src, maxsrc, flags, status)
struct RkRxDic	*rdic;
unsigned char	*dst;
int		maxdst;
unsigned char	*src;
int		maxsrc;
int		flags;
int		*status;
{
    register int	i;
    unsigned char	*roma;
    unsigned char	*kana = src;
    int			count = 0;
    int			byte;
    int			found = 1;
    struct rstat *m;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
    struct rstat match[256];
#else
    struct rstat *match;

    match = (struct rstat *)malloc(sizeof(struct rstat) * 256);
    if (!match) {
      return count;
    }
#endif

    if ( rdic ) {
	m = match;
	m->start = 0;
	m->end = rdic->nr_nkey;
	for (i = 0; (flags & RK_FLUSH) || i < maxsrc;  i++) {
	    m[1] = m[0];
	    m++;
	    switch((i < maxsrc) ? findRoma(rdic, m, src[i], i, 0) : 0) {
	    case	0:
		while (--m > match && xkey(rdic, m->start, m - match))
		  /* EMPTY */
		  ;
		if(m == match) { /* table ni nakatta tokino shori */
		    kana = src;
		    count = (maxsrc <= 0)? 0 : (*src & 0x80)? 2 : 1;
		    if( (flags & RK_SOKON) &&
			(match[1].start < rdic->nr_nkey) &&
			(2 <= maxsrc) &&
			(src[0] == src[1]) &&
			(i == 1)) {
			kana = getTSU(rdic, flags);
		    /* tsu ha jisho ni aru kao wo suru */
			byte = strlen((char *)kana);
		    }
		    else {
			static unsigned char	tmp[256];

			switch(flags&RK_XFERMASK) {
			default:
			    byte = RkCvtNone(tmp, sizeof(tmp), src, count);
			    break;
			case RK_XFER:
			    byte = RkCvtHira(tmp, sizeof(tmp), src, count);
			    break;
			case RK_HFER:
			    byte = RkCvtHan(tmp, sizeof(tmp), src, count);
			    break;
			case RK_KFER:
			    byte = RkCvtKana(tmp, sizeof(tmp), src, count);
			    break;
			case RK_ZFER:
			    byte = RkCvtZen(tmp, sizeof(tmp), src, count);
			    break;
			};
			kana = tmp;
			found = -1;
		    };
		}
		else {  /* 'n' nado no shori: saitan no monowo toru */
		    kana = getKana(rdic, m->start, flags);
		    byte = strlen((char *)kana);
		    count = m - match;
		}
		goto done;
	    case	1:	/* determined uniquely */
	    /* key no hou ga nagai baai */
		roma = getRoma(rdic, m->start);
		if ( roma[i + 1] ) 	/* waiting suffix */
		    continue;
		kana = getKana(rdic, m->start, flags);
		byte = strlen((char *)kana);
		count = i + 1;
		goto done;
	    };
	};
	byte = 0;
    }
    else
	byte = (maxsrc <= 0) ? 0 : (*src & 0x80) ? 2 : 1;
done:
    *status = found*byte;
    if ( byte + 1 <= maxdst ) {
	if ( dst ) {
	    while ( byte-- )
		*dst++ = *kana++;
	    *dst = 0;
	};
    };
#ifdef USE_MALLOC_FOR_BIG_ARRAY
    (void)free((char *)match);
#endif
    return count;
}

static
unsigned char	*
getrawKana(rdic, p)
struct RkRxDic	*rdic;
int		p;
{
  register unsigned char	*kana;

  for (kana = rdic->nr_keyaddr[p] ; *kana++ ; )
    /* EMPTY */
    ;

  return kana;
}

static
unsigned char	*
getTemp(rdic, p)
struct RkRxDic	*rdic;
int		p;
{
  register unsigned char	*kana;

//  if (rdic->dic == RX_RXDIC) {
//    return (unsigned char *)0;
//  }
  kana = rdic->nr_keyaddr[p];
  while (*kana++)
    /* EMPTY */
    ;
  while (*kana++)
    /* EMPTY */
    ;

  return kana;
}


int
RkMapPhonogram(rdic, dst, maxdst, src, srclen, key, flags,
	       used_len_return, dst_len_return, tmp_len_return,
	       rule_id_inout)
struct RkRxDic	*rdic;
unsigned char	*dst;
int		maxdst;
unsigned char	*src;
int		srclen;
unsigned	key;
int		flags;
int		*used_len_return, *dst_len_return, *tmp_len_return;
int		*rule_id_inout;
{
  register int	i;
  unsigned char	*roma, *temp;
  unsigned char	*kana = src;
  int			count = 0;
  int			byte;
  int			found = 1;
  int templen, lastrule;
  struct rstat *m;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  struct rstat match[256];
#else
  struct rstat *match;
  match = (struct rstat *)malloc(sizeof(struct rstat) * 256);
  if (!match) {
    return found;
  }
#endif

  if ( rdic ) {
    if ((rdic->dic == RX_KPDIC || rdic->dic == RX_PTDIC)
	&& rule_id_inout && (lastrule = *rule_id_inout)) {
      if (!key) {
	if (rdic->nr_brules && rdic->nr_brules[lastrule] &&
	    !(flags & RK_FLUSH)) {
	  /* もし、! が付いていた場合には第３フィールドに書かれている
             文字で始まるルールがあると仮想的に考えられるわけであるか
             ら key が与えられていないのであれば与えられた文字列が短か
             すぎるためなんともできないよしのリターン値を返す。 */
	  /* RK_FLUSH は調べるべきかどうか悩むところ */
	  byte = count = 0;
	  templen = 0;
	  found = 0;
	  goto done;
	}
      }
      else {
	lastrule--;
	if (lastrule < rdic->nr_nkey && rdic->nr_brules) {
	  if (rdic->nr_brules[lastrule]) {
	    unsigned char *p;

	    for (p = rdic->nr_bchars ; *p ; p++) {
	      if (key == *p) {
		unsigned char *origin = getTemp(rdic, lastrule), *ret;
		int dstlen = 0, tmplen;

		ret = dst;
		for (i = 0 ; i < maxdst && *origin ; i++) {
		  origin++;
		}
		if (i + 1 == srclen) {
		  /* バックトラックをする */
		  origin = rdic->nr_keyaddr[lastrule];

		  for (i = 0 ; i < maxdst && *origin ; i++) {
		    *dst++ = *origin++;
		  }
		  tmplen = ++i;
		  if (i < maxdst) {
		    *dst++ = key;
		    *dst = (unsigned char)0;
		  }
		  if (used_len_return) *used_len_return = srclen;
		  if (*ret & 0x80) { /* very dependent on Japanese EUC */
		    if (*ret == 0x8f) {
		      dstlen++;
		    }
		    dstlen++;
		  }
		  dstlen++;
		  if (dst_len_return) *dst_len_return = dstlen;
		  if (tmp_len_return) *tmp_len_return = tmplen - dstlen;
		  *rule_id_inout = 0;
		  goto return_found;
		}
	      }
	    }
	  }
	}
      }
    }
    m = match;
    m->start = 0;
    m->end = rdic->nr_nkey;
    for (i = 0; (flags & RK_FLUSH) || i < srclen;  i++) {
      m[1] = m[0];
      m++;
      switch((i < srclen) ?
	     findRoma(rdic, m, src[i], i, flags & RK_IGNORECASE) : 0) {
      case	0:
	while (--m > match && xkey(rdic, m->start, m - match))
	  /* EMPTY */
	  ;
	if(m == match) { /* テーブルになかった時の処理 */
	  count = (*src & 0x80) ? 2 : 1;
	  if (srclen < count) {
	    count = 0;
	  }
/*	  if( (rdic->dic == RX_RXDIC) &&   // tt の救済(旧辞書用)
	     (flags & RK_SOKON) &&
	     (match[1].start < rdic->nr_nkey) &&
	     (2 <= srclen) &&
	     (src[0] == src[1]) &&
	     (i == 1)) {
	    kana = getTSU(rdic, flags);
            // tsu ha jisho ni aru kao wo suru
	    byte = strlen((char *)kana);
	    templen = 0;
	    if (rule_id_inout) *rule_id_inout = 0;
	  }
	  else { */ /* １文字変換されたことにする */
	    byte = count;
	    templen = 0;
	    kana = src;
	    found = 0;
//	  }
	}
	else {  /* 'n' などの処理: 最短のものを取る */
	  kana = getrawKana(rdic, m->start);
	  byte = strlen((char *)kana);
	  temp = getTemp(rdic, m->start);
	  templen = temp ? strlen((char *)temp) : 0;
	  count = m - match;
	  if (rule_id_inout) {
	    if (byte == 0 && templen > 0) {
	      *rule_id_inout = m->start + 1;
	    }
	    else {
	      *rule_id_inout = 0;
	    }
	  }
	}
	goto done;
      case	1: /* 途中でどんぴしゃが見つかった */
	/* key no hou ga nagai baai */
	roma = getRoma(rdic, m->start);
	if ( roma[i + 1] ) 	/* waiting suffix */
	  continue;
	kana = getrawKana(rdic, m->start);
	byte = strlen((char *)kana);
	temp = getTemp(rdic, m->start);
	templen = temp ? strlen((char *)temp) : 0;
	count = i + 1;
	if (rule_id_inout) {
	  if (byte == 0 && templen > 0) {
	    *rule_id_inout = m->start + 1;
	  }
	  else {
	    *rule_id_inout = 0;
	  }
	}
	goto done;
      }
    }
    byte = count = 0;
    templen = 0;
  }
  else {
    byte = (*src & 0x80) ? 2 : 1;
    if (srclen < byte) {
      byte = 0;
    }
    count = byte;
    kana = src;
    templen = 0;
    found = 0;
  }
 done:

  if (dst_len_return) {
    *dst_len_return = byte;
  }
  if (used_len_return) {
    *used_len_return = count;
  }
  if (tmp_len_return) {
    *tmp_len_return = templen;
  }
  if ( byte < maxdst ) {
    if ( dst ) {
      int ii;
      for (ii = 0 ; ii < byte ; ii++)
	*dst++ = *kana++;
      *dst = 0;
    }
    if (byte + templen < maxdst) {
      if (dst) {
	while (templen--) {
	  *dst++ = *temp++;
	}
	*dst = 0;
      }
    }
  }
 return_found:
#ifdef USE_MALLOC_FOR_BIG_ARRAY
  (void)free((char *)match);
#endif
  return found;
}

/* RkCvtRoma
 */
int
RkCvtRoma(rdic, dst, maxdst, src, maxsrc, flags)
struct RkRxDic	*rdic;
unsigned char	*dst;
int		maxdst;
unsigned char	*src;
int		maxsrc;
unsigned	flags;
{
    register unsigned char	*d = dst;
    register unsigned char	*s = src;
    register unsigned char	*S = src + maxsrc;
    int count = 0;
    unsigned xp = 0;
    unsigned char key;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
    unsigned char xxxx[64], yyyy[64];
#else
    unsigned char *xxxx, *yyyy;
    xxxx = (unsigned char *)malloc(64);
    yyyy = (unsigned char *)malloc(64);
    if (!xxxx || !yyyy) {
      if (xxxx) {
	(void)free((char *)xxxx);
      }
      if (yyyy) {
	(void)free((char *)yyyy);
      }
      return count;
    }
#endif

    if (!(maxdst <= 0 || maxsrc < 0)) {
      while ( s < S ) {
	int ulen, dlen, tlen, rule = 0;
	unsigned dontflush = RK_FLUSH;

	key = xxxx[xp++] = *s++;
      flush:
	do {
	  RkMapPhonogram(rdic, d, maxdst, xxxx, xp, (unsigned)key,
			 flags & ~dontflush, &ulen, &dlen, &tlen, &rule);

	  if ( dlen + 1 <= maxdst ) {
	    maxdst -= dlen; count += dlen;
	    if ( dst ) {
	      d += dlen;
	      (void)strncpy((char *)yyyy, (char *)d, tlen);
	    }
	  }

	  if (ulen < (int)xp) {
	    strncpy((char *)yyyy + tlen, (char *)xxxx + ulen, xp - ulen);
	  }
	  strncpy((char *)xxxx, (char *)yyyy, tlen + xp - ulen);
	  xp = tlen + xp - ulen;
	  key = 0;
	} while (ulen > 0);
	if (s == S && dontflush) {
	  dontflush = 0;
	  goto flush;
	}
      }
    }
#ifdef USE_MALLOC_FOR_BIG_ARRAY
    (void)free((char *)yyyy);
    (void)free((char *)xxxx);
#endif
    return count;
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
        *ulen = eucchars(cbuf1, fulen); //MBstowcs(wbuf, cbuf1, CBUFSIZE);
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
