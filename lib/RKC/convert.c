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

#if !defined(lint) && !defined(__CODECENTER__)
static char rcs_id[] = "$Id: convert.c,v 1.9.2.1 2004/04/26 21:48:37 aida_s Exp $";
#endif

/* LINTLIBRARY */

#include "canna/sglobal.h"
#include "rkcw.h"
#include "canna/RK.h"
#include "rkc.h"
#include "server/IRproto.h"
#include "RKindep/file.h"

#include <sys/types.h>
#include <signal.h>

/* 単語登録で辞書が作れなくなるので、とりあえずコメントアウト
#ifdef CANNA_LIGHT
#ifdef EXTENSION
#undef EXTENSION
#endif
#endif
 */

extern int _RkwGetYomi( RkcContext* cx, cannawc* yomi, int maxyomi);
extern int eucchars(const unsigned char* s, int bytelen);
extern int CNvW2E pro((const cannawc* src, int srclen, unsigned char* dest, size_t));
extern int ushort2eucsize(const cannawc* yomi, int len);


#ifdef USE_EUC_PROTOCOL

extern int ServerFD ;
extern unsigned int ServerTimeout ;

#define SENDBUFSIZE 1024
#define RECVBUFSIZE 1024

#define PROTOBUF (16 * 8)

#define TRY_COUNT	    10

#ifdef LESS_SPACE_IS_IMPORTANT
#undef LTOL4
static void
LTOL4(uint32_t l, BYTE* p)
{
  p[0] = (l >> 24) & 0xff;
  p[1] = (l >> 16) & 0xff;
  p[2] = (l >> 8)  & 0xff;
  p[3] = l & 0xff;
}

#undef L4TOL
uint32_t L4TOL(BYTE* p)
{
  return (((((p[0] << 8) | p[1]) << 8) | p[2]) << 8) | p[3];
}
#endif

#ifdef DEBUGPROTO
static void
printproto(char* p, int n)
{
  int i;

  for (i = 0 ; i < n ; i++) {
    if (i) {
      if ((i %  4) == 0) printf(" ");
      if ((i % 32) == 0) printf("\n");
    }
    printf("%02x", (unsigned)((*p++) & 0xff));
  }
  printf("\n");
}

static void
probe(char* format, int n, char* p)
{
  printf(format, n);
  printproto(p, n);
}
#else /* !DEBUGPROTO */
#define probe(a, b, c)
#endif /* !DEBUGPROTO */

/*

  RkcRecvEReply()

   1: Succeed;
   0: Error;

  len_return: データの長さ。NULL を与えれば 格納しない。

  bufsize < requiredsize なら空読みする。

 */

#define ReadServer RkcRecvEReply

int
RkcRecvEReply(BYTE* buf, int bufsize, int requiredsize, int* len_return)
{
  int empty_count = 0, bufcnt = 0, readlen;
  unsigned rest = (unsigned)bufsize;
  BYTE *bufptr = buf;
  struct timeval timeout, timeout2;
  rki_fd_set rfds, rfds2;

  timeout.tv_sec = ServerTimeout / 1000;
  timeout.tv_usec = (ServerTimeout % 1000) * 1000;
  RKI_FD_ZERO(&rfds);
  RKI_FD_SET(ServerFD, &rfds);

  errno = 0;

  empty_count = 0;
  do {
    timeout2 = timeout;
    rfds2 = rfds;
    if (ServerTimeout) {
      int r = select(ServerFD + 1, &rfds2, NULL, NULL, &timeout2);
      if (r == 0) {
	break;
      } else if (r == -1) {
	if (errno == EINTR)
	  continue;
	else
	  break;
      }
    }
    readlen = read(ServerFD, (char *)bufptr, rest);
    if (readlen < 0) {
      if (errno == EINTR) {
	continue;
      }
      else {
	break;
      }
    }
    else if ( readlen == 0 ) {
      empty_count++;
    }
    else { /* 読んだ */
      bufcnt += readlen;
      if (requiredsize <= bufsize) {
	bufptr += readlen;
	rest -= readlen;
      }
    }
  } while (empty_count < TRY_COUNT &&
	   (bufcnt == 0 || (requiredsize && bufcnt < requiredsize)));

  if (bufcnt == 0 || (requiredsize && bufcnt < requiredsize)) {
    errno = EPIPE;
    close(ServerFD);
    return NO;
  }
  else {
    probe("Read: %d\n", bufcnt, buf);
    if (len_return) *len_return = bufcnt;
    return YES;
  }
}

/*
static void DoSomething(int sig)
{
    errno = EPIPE;
    signal(SIGPIPE, DoSomething);
}
*/

int
RkcSendERequest( const BYTE* Buffer, int size )
{
    register int todo, retval = YES;
    register int write_stat;
    register const BYTE *bufindex;
    void (*Sig)(int);
    struct timeval timeout, timeout2;
    rki_fd_set wfds, wfds2;

    timeout.tv_sec = ServerTimeout / 1000;
    timeout.tv_usec = (ServerTimeout % 1000) * 1000;
    RKI_FD_ZERO(&wfds);
    RKI_FD_SET(ServerFD, &wfds);

    errno = 0 ;
    bufindex = Buffer ;
    todo = size ;
    Sig = signal(SIGPIPE, SIG_IGN);
    if ( Sig == SIG_ERR )
        return NO;

    while (size) {
	timeout2 = timeout;
	wfds2 = wfds;
	errno = 0;
	probe("Write: %d\n", todo, (char *)bufindex);
	if (ServerTimeout) {
	  int r = select(ServerFD + 1, NULL, &wfds, NULL, &timeout2);
	  if (r == 0) {
	    goto fail;
	  } else if (r == -1) {
	    if (errno == EINTR)
	      continue;
	    else
	      goto fail;
	  }
	}

        write_stat = write(ServerFD, (char *)bufindex, (unsigned)todo);
	if (write_stat >= 0) {
	    size -= write_stat;
	    todo = size;
	    bufindex += write_stat;
        }
        else if (errno == EWOULDBLOCK) {   /* pc98 */
	    continue ;
	}
	else if (errno == EINTR) {
	    continue;
	}
#ifdef EMSGSIZE
	else if (errno == EMSGSIZE) {
	    if (todo > 1)
		todo >>= 1;
	    else
		continue ;
	}
#endif
	else {
	    goto fail;
	}
    }
    goto last;
fail:
    close( ServerFD ) ;
    retval = NO;
    errno = EPIPE ;
last:
    signal(SIGPIPE, Sig);
    return retval;
}

static
int SendType0Request( long proto, long len, BYTE* name) /* Initialize */
{
  BYTE lbuf[PROTOBUF], *bufp = lbuf, *p;
  int sz = 8 + len;
  int res;

  if (sz <= PROTOBUF || (bufp = (BYTE *)malloc(sz))) {
    p = bufp;
    LTOL4(proto, p); p += SIZEOFLONG;
    LTOL4(len, p);   p += SIZEOFLONG;
    strcpy((char *)p, (char *)name);
    res = RkcSendERequest(bufp, sz);
    if (bufp != lbuf) free((char *)bufp);
    return res;
  }
  else {
    return NO;
  }
}

static
int SendTypeE1Request( int proto) /* Finalize */
{
  BYTE lbuf[4];

  LTOL4(proto, lbuf);
  return RkcSendERequest(lbuf, sizeof(lbuf));
}

static
int SendTypeE2Request( int proto, int con) /* IR_DUP_CON */
{
  BYTE lbuf[8], *p;

  LTOL4(proto, lbuf); p = lbuf + SIZEOFLONG;
  LTOL4(con, p);
  return RkcSendERequest(lbuf, sizeof(lbuf));
}

static
int SendTypeE3Request(int proto, int con, int val) /* IR_DIC_LIST */
{
  BYTE lbuf[12], *p;

  LTOL4(proto, lbuf); p = lbuf + SIZEOFLONG;
  LTOL4(con, p); p += SIZEOFLONG;
  LTOL4(val, p);
  return RkcSendERequest(lbuf, sizeof(lbuf));
}

static
int SendTypeE4Request( int proto, int con, int bun, int val) /* IR_GET_YOMI */
{
  BYTE lbuf[16], *p;

  LTOL4(proto, lbuf); p = lbuf + SIZEOFLONG;
  LTOL4(con, p); p += SIZEOFLONG;
  LTOL4(bun, p); p += SIZEOFLONG;
  LTOL4(val, p);
  return RkcSendERequest(lbuf, sizeof(lbuf));
}

static
int SendTypeE5Request( int proto, int con, int bun, int val, int max_) /* IR_GET_LEX */
{
  BYTE lbuf[5 * SIZEOFLONG], *p;

  LTOL4(proto, lbuf); p = lbuf + SIZEOFLONG;
  LTOL4(con, p); p += SIZEOFLONG;
  LTOL4(bun, p); p += SIZEOFLONG;
  LTOL4(val, p); p += SIZEOFLONG;
  LTOL4(max_, p);
  return RkcSendERequest(lbuf, sizeof(lbuf));
}

static
int SendTypeE6Request( int proto, int con, int bun, BYTE* name, int nlen) /* IR_STO_YOMI */
{
  BYTE lbuf[SENDBUFSIZE], *bufp = lbuf, *p;
  int res, sz = 4 * SIZEOFLONG + nlen;

  if (sz <= SENDBUFSIZE || (bufp = (BYTE *)malloc(sz))) {
    p = bufp;
    LTOL4(proto, p); p += SIZEOFLONG;
    LTOL4(con, p);   p += SIZEOFLONG;
    LTOL4(bun, p);   p += SIZEOFLONG;
    LTOL4(nlen, p);  p += SIZEOFLONG;
    strncpy((char *)p, (char *)name, nlen);
    res = RkcSendERequest(bufp, sz);
    if (bufp != lbuf) free((char *)bufp);
    return res;
  }
  return NO;
}

static
int SendTypeE7Request( int proto, RkcContext* cx, int val) /* IR_CONV_END */
{
  BYTE lbuf[SENDBUFSIZE], *bufp = lbuf, *p;
  int res, con = (int)cx->server, mbun = val * (int)cx->maxbun, i;
  int datalen = mbun * SIZEOFLONG, sz = 3 * SIZEOFLONG + datalen;

  if (sz <= SENDBUFSIZE || (bufp = (BYTE *)malloc(sz))) {
    p = bufp;
    LTOL4(proto, p); p += SIZEOFLONG;
    LTOL4(con, p);   p += SIZEOFLONG;
    LTOL4(mbun, p);  p += SIZEOFLONG;
    for (i = 0 ; i < mbun ; i++) {
      int kn = (int)cx->bun[i].curcand;

      LTOL4(kn, p); p += SIZEOFLONG;
    }
    res = RkcSendERequest(bufp, sz);
    if (bufp != lbuf) free((char *)bufp);
    return res;
  }
  return NO;
}

static
int SendTypeE9Request( int proto, int con, BYTE* name, int nlen, int val) /* IR_MNT_DIC */
{
  BYTE lbuf[SENDBUFSIZE], *bufp = lbuf, *p;
  int res, sz = 4 * SIZEOFLONG + nlen;

  if (sz <= SENDBUFSIZE || (bufp = (BYTE *)malloc(sz))) {
    p = bufp;
    LTOL4(proto, p); p += SIZEOFLONG;
    LTOL4(con, p);   p += SIZEOFLONG;
    LTOL4(nlen, p);  p += SIZEOFLONG;
    strncpy((char *)p, (char *)name, nlen); p += nlen;
    LTOL4(val, p);
    res = RkcSendERequest(bufp, sz);
    if (bufp != lbuf) free((char *)bufp);
    return res;
  }
  return NO;
}

static
int SendTypeE10Request( int proto, int con, BYTE* name, int nlen, BYTE* val, int vlen) /* IR_DEF_DIC */
{
  BYTE lbuf[SENDBUFSIZE], *bufp = lbuf, *p;
  int res, sz = 4 * SIZEOFLONG + nlen + vlen;

  if (sz <= SENDBUFSIZE || (bufp = (BYTE *)malloc(sz))) {
    p = bufp;
    LTOL4(proto, p); p += SIZEOFLONG;
    LTOL4(con, p);   p += SIZEOFLONG;
    LTOL4(nlen, p);  p += SIZEOFLONG;
    strncpy((char *)p, (char *)name, nlen); p += nlen;
    LTOL4(vlen, p);  p += SIZEOFLONG;
    strncpy((char *)p, (char *)val, vlen);
    res = RkcSendERequest(bufp, sz);
    if (bufp != lbuf) free((char *)bufp);
    return res;
  }
  return NO;
}

static
int SendTypeE11Request( int proto, int con, BYTE* name, int nlen, BYTE* dest, int dlen, int val)
{
  BYTE lbuf[SENDBUFSIZE], *bufp = lbuf, *p;
  int res, sz = 5 * SIZEOFLONG + nlen + dlen;

  if (sz <= SENDBUFSIZE || (bufp = (BYTE *)malloc(sz))) {
    p = bufp;
    LTOL4(proto, p); p += SIZEOFLONG;
    LTOL4(con, p);   p += SIZEOFLONG;
    LTOL4(nlen, p);  p += SIZEOFLONG;
    strncpy((char *)p, (char *)name, nlen); p += nlen;
    LTOL4(dlen, p);  p += SIZEOFLONG;
    strncpy((char *)p, (char *)dest, dlen); p += dlen;
    LTOL4(val, p);
    res = RkcSendERequest(bufp, sz);
    if (bufp != lbuf) free((char *)bufp);
    return res;
  }
  return NO;
}

static
int SendTypeE12Request( long proto, BYTE* name, long nlen, long val) /* IR_QUERY_EXT */
{
  BYTE lbuf[SENDBUFSIZE], *bufp = lbuf, *p;
  int res, sz = 3 * SIZEOFLONG + nlen;

  if (sz <= SENDBUFSIZE || (bufp = (BYTE *)malloc(sz))) {
    p = bufp;
    LTOL4(proto, p); p += SIZEOFLONG;
    LTOL4(nlen, p);  p += SIZEOFLONG;
    strncpy((char *)p, (char *)name, nlen); p += nlen;
    LTOL4(val, p);   p += SIZEOFLONG;
    res = RkcSendERequest(bufp, sz);
    if (bufp != lbuf) free((char *)bufp);
    return res;
  }
  return NO;
}

#define RecvType0Reply RecvTypeE1Reply /* Initinalize */

static
int RecvTypeE1Reply(long* rep) /* Finalize */
{
  BYTE lbuf[SIZEOFLONG];

  if (ReadServer(lbuf, sizeof(lbuf), SIZEOFLONG, 0)) {
    *rep = (long)L4TOL(lbuf);
    return YES;
  }
  return NO;
}


// 第4引数は、目的に応じて替える.
typedef int (*StoreFunc)(int n, BYTE* src, int slen, BYTE* dest, int maxn, int unit);

/* GeneralReply
  ４バイト目から４バイトのデータを拾って来ると、そこから後ろのデータ長
  が拾えるようなリプライを拾うためのルーチン。

  rep       呼出しがわに返す値を格納するアドレス。
  storefunc 拾って来たデータの格納ルーチン。
  addr      格納する先のアドレス。
  unit      addr の大きさの単位がバイトで数えていくつかと言うことを表す値。
  maxn      その単位の大きさのデータが addr にはいくつ積めるかと言うこと。
  offset     storefunc にデータを渡す時のオフセット。
 */
static
int GeneralReply(int* rep, StoreFunc storefunc, BYTE* addr, int maxn, int unit, int offset)
{
  BYTE lbuf[RECVBUFSIZE], *bufp = lbuf, *p;
  int res, datalen, readlen, readcnt, requiredsize, retval;

  if (!ReadServer(lbuf, RECVBUFSIZE, SIZEOFLONG, &readlen)) {
    return NO;
  }
  res = L4TOL(lbuf); p = lbuf + SIZEOFLONG;
  if (res >= 0) {
    readcnt = readlen;
    readlen = 0;
    if (readcnt < 2 * SIZEOFLONG) {
      if (!ReadServer(lbuf + readcnt, RECVBUFSIZE - readcnt,
		      2 * SIZEOFLONG - readcnt, &readlen)) {
        return NO;
      }
      readcnt += readlen;
    }

    datalen = L4TOL(p); p += SIZEOFLONG;

    requiredsize = 2 * SIZEOFLONG + datalen;

    if (readcnt < requiredsize) {
      if (RECVBUFSIZE < requiredsize) {
	bufp = (BYTE *)malloc(requiredsize);
	if (!bufp) {
	  (void)ReadServer(lbuf, RECVBUFSIZE, requiredsize, 0);
	  return NO;
	}
	bcopy(lbuf, bufp, readcnt);
      }
      if (!ReadServer(bufp + readcnt, requiredsize - readcnt,
		      requiredsize - readcnt, &readlen)) {
	retval = NO;
        goto endGenRep;
      }
    }
    /* ここまでは、どちらかと言うと純粋な read */

    if (storefunc) {
      res = (*storefunc)(res, bufp + offset * SIZEOFLONG, datalen,
			 addr, maxn, unit);
    }

    *rep = res;
    retval = YES;
  endGenRep:
    if (bufp != lbuf) free((char *)bufp);
    return retval;
  }
  return NO;
}

#define RecvTypeE2Reply(rep, storefunc, addr, maxsize) \
  GeneralReply(rep, storefunc, addr, maxsize, sizeof(char), 2)

#define RecvTypeE3Reply(rep, storefunc, addr, maxn, unit) \
  GeneralReply(rep, storefunc, addr, maxn, unit, 2)

static
int RecvTypeE4Reply(int* rep, StoreFunc storefunc, BYTE* addr, int maxn, int unit) /* IR_GET_LEX */
{
  return GeneralReply(rep, storefunc, addr, maxn, unit, 1);
}


/*
 * サーバから返された第一候補列を、第一候補列バッファに格納する。
 *  したがって、bun->kanjiのポインタの指す位置は変化しない。
 */

static int
firstKouhoStore( int n, RkcContext* cx, BYTE* data, int datalen)
{
    cannawc *return_kouho, *wp, *ewp;
    register int i, save_len ;
    cannawc* first_kouho = cx->Fkouho ;
    int length, euc_len, res = n;
    BYTE *p;

    if (n < 0)
        return n;

    /* コピーすべきバッファの大きさを調べる */
    for (i = 0 ; i < (int)cx->curbun ; i++) {
      first_kouho += ushortstrlen(first_kouho) + 1;
    }
    save_len = first_kouho - cx->Fkouho;

    euc_len = 0; p = data;
    for (i = (int)cx->curbun ; i < n ; i++) {
      int ulen = L4TOL(p); p += SIZEOFLONG;
      euc_len += eucchars(p, ulen);
      p += ulen;
    }

    if (!(wp = (cannawc*) malloc((save_len + euc_len + 2) * sizeof(cannawc)))) {
        /* +2 は euc2ushort が最後まで積めるか不安なため。 */
        return -1;
    }

    return_kouho = wp; ewp = wp + save_len + euc_len + 2;
      for( first_kouho = cx->Fkouho, i = 0; i < (int)cx->curbun; i++ ) {
	(void)ushortstrcpy(wp, first_kouho);
	length = ushortstrlen(first_kouho) + 1;
	wp += length;
	first_kouho += length;
      }
      p = data;
      for (i = (int)cx->curbun ; i < n ; i++) {
	int ulen = L4TOL(p); p += SIZEOFLONG;
	wp += euc2ushort( p, ulen, wp, ewp - wp);
	p += ulen;
      }

/*    *(++wp) = (Ushort)0 ; 下のほうが正しいと思う。 */
      *wp = (cannawc) 0 ;
      free((char *)cx->Fkouho);
      cx->Fkouho = return_kouho ;

    return res;
}

static long
rkc_initialize( char* username )
{
  long reply;
  long len = strlen( (char *)username ) + 1 ;

  if (SendType0Request((long) IR_INIT, len, (BYTE *)username) &&
      RecvType0Reply(&reply)) {
    if (reply < 0) {
      close(ServerFD);
    }
    return reply;
  }
    return -1;
}

static
int Fin_Create( int request )
{
    long reply;

  if (SendTypeE1Request(request) &&
      RecvTypeE1Reply(&reply)) {
    return reply;
  }
  return -1;
}

static
int rkc_finalize()
{
  int retval = Fin_Create(IR_FIN);
  (void)close(ServerFD);
  return retval;
}

static
int rkc_create_context()
{
    return( Fin_Create( IR_CRE_CON ) ) ;
}

static
int Dup_Close_CX( int cx_num, int request )
{
    long reply;

  if (SendTypeE2Request(request, cx_num)&&
      RecvTypeE1Reply(&reply)) {
    return reply;
  }
  return -1;
}

static
int rkc_duplicate_context( RkcContext* cx )
{
    return( Dup_Close_CX( (int)cx->server, IR_DUP_CON ) ) ;
}

static
int rkc_close_context( RkcContext* cx )
{
    return( Dup_Close_CX( (int)cx->server, IR_CLO_CON ) ) ;
}

// Callback function.
static int
dicStore(int n, BYTE* src, int slen, BYTE* dest, int dmax, int unit)
{
  BYTE *p = dest, *endp = dest + dmax - 2; /* 2 for EOS */
  BYTE *wp = src, *wendp = src + slen;
  int len, i;

  for (i = 0 ; i < n && p < endp && wp < wendp ; i++) {
    len = L4TOL(wp); wp += SIZEOFLONG;
    if (endp < p + len) break;
    strcpy((char *)p, (char *)wp);
    p += len;
    wp += len;
  }
  *p = '\0';
  return i;
}

static
int Dic_Dir_List( int context, char* data, int max, int request )
{
  int reply;

  if (SendTypeE3Request(request, context, max) &&
      RecvTypeE2Reply(&reply, dicStore, (BYTE *)data, max)) {
    return reply;
  }
  return -1;
}

static
int rkc_dictionary_list( RkcContext* cx, char* dicnames, int max)
{
  return Dic_Dir_List((int)cx->server, dicnames, max, IR_DIC_LIST);
}


static
int Define_Delete_dic( RkcContext* cx, const char* dicname,
                       unsigned char* wordrec,
                       int request )
{
    long reply;

  if (SendTypeE10Request(request, (int)cx->server,
			(BYTE *)dicname, strlen((char *)dicname) + 1,
			(BYTE *)wordrec, strlen((char *)wordrec) + 1) &&
      RecvTypeE1Reply(&reply)) {
    return reply;
  }
  return -1;
}


static
int rkc_define_dic( RkcContext* cx, const char* dicname, cannawc* wordrec)
{
    unsigned char cbuf[RK_LINE_BMAX];

    ushort2euc(wordrec, ushortstrlen(wordrec), cbuf, RK_LINE_BMAX);
    return Define_Delete_dic(cx, dicname, cbuf, IR_DEF_DIC);
}

static
int rkc_delete_dic( RkcContext* cx, char* dicname, cannawc* wordrec)
{
    unsigned char cbuf[RK_LINE_BMAX];

    ushort2euc(wordrec, ushortstrlen(wordrec), cbuf, RK_LINE_BMAX);
  return Define_Delete_dic(cx, dicname, cbuf, IR_UNDEF_DIC);
}


static
int mount_dic( int req, int con, const char* dat, int mod)
{
    long reply;
    int datlen = strlen((char *)dat) + 1;

  if (SendTypeE9Request(req, con, (BYTE *)dat, datlen, mod) &&
      RecvTypeE1Reply(&reply)) {
    return reply;
  }
  return -1;
}


static
int rkc_mount_dictionary( RkcContext* cx, const char* dicname, int mode )
{
    return mount_dic(IR_MNT_DIC, (int)cx->server, dicname, mode);
}


static
int rkc_umount_dictionary( RkcContext* cx, char* dicname )
{
  return mount_dic(IR_UMNT_DIC, cx->server, dicname, 0);
}


static
int rkc_remount_dictionary( RkcContext* cx, char* dicname, int where )
{
    long reply;
    int datalen = strlen(dicname) + 1;

  if (SendTypeE6Request(IR_RMNT_DIC, cx->server, where,
			(BYTE *)dicname, datalen) &&
      RecvTypeE1Reply(&reply)) {
    return reply;
  }
  return -1;
}


static
int rkc_mount_list( RkcContext* cx, char* data, int max)
{
  return Dic_Dir_List(cx->server, data, max, IR_MNT_LIST);
}


int rkc_get_dir_list( RkcContext* cx, char* ddname, int maxddname )
{
    return( Dic_Dir_List( cx->server, ddname, maxddname, IR_DIR_LIST ) ) ;
}

#define GAKUSHU 1

static
int rkc_convert_end( RkcContext* cx, int mode )
{
    long reply;
    int gakushu = (mode & GAKUSHU) ? 1 : 0;

  if (SendTypeE7Request(IR_CONV_END, cx, gakushu) &&
      RecvTypeE1Reply(&reply)) {
    return reply;
  }
  return -1;
}


static
int convStore( int n, BYTE* data, int datalen, BYTE* contex, int v, int u)
{
  RkcContext *cx = (RkcContext *)contex;
  int ret;

  if ((ret = firstKouhoStore(n, cx, data, datalen)) < 0 ){
    (void)rkc_convert_end( cx, 0 );
  }
  return ret;
}


static
int rkc_convert( RkcContext* cx, cannawc* yomi, int length, int mode )
{
    int reply, datalen = ushort2eucsize(yomi, length) + 1, res = -1;
    unsigned char cbuf[BUFSIZE], *bufp = cbuf;

    if ( datalen >= BUFSIZE && !(bufp = (unsigned char*) malloc(datalen + 2)))
        return -1;

    ushort2euc(yomi, length, bufp, datalen + 2);
    /* +2 は ushort2euc がちゃんと詰めてくれるか不安なため。 */

    if (SendTypeE9Request(IR_CONVERT, cx->server, (BYTE *)bufp, datalen, mode)
	&& RecvTypeE2Reply(&reply, convStore, (BYTE *)cx, 0)) {
      res = reply;
    }
    else {
      res = -1;
    }
    if (bufp != cbuf) free(bufp);
    return res;
}

// Callback function.
static
int yomiStore( int n, BYTE* data, int datalen, BYTE* dest, int destlen, int unit)
{
  int len = L4TOL(data); data += SIZEOFLONG;
  if (n < len) len = n;
    return euc2ushort(data, len, (cannawc*) dest, destlen);
}


static
int rkc_get_yomi( RkcContext* cx, cannawc* yomip )
{
  int reply;

  if (SendTypeE4Request(IR_GET_YOMI, cx->server, (int)cx->curbun, BUFSIZE) &&
      RecvTypeE2Reply(&reply, yomiStore, (BYTE *)yomip, CBUFSIZE)) {
    return reply;
  }
  return -1;
}


static
int kanlisStore( int n, BYTE* data, int datalen, BYTE* cox, int v, int u)
{
  RkcContext *cx = (RkcContext *)cox;
  RkcBun *bun = cx->bun + (int)cx->curbun;
  int len, i;
  cannawc *kouho_list, *wp, *ewp;
  BYTE *p;

  if (n < 0) return n;

  len = 0; p = data;
  for (i = 0 ; i < n ; i++) {
    int ulen = L4TOL(p); p += SIZEOFLONG;
    len += eucchars(p, ulen);
    p += ulen;
  }

  if (len > 0 && (kouho_list = (cannawc*) malloc((len + 2) * sizeof(cannawc)))) {
    /* +2 は euc2ushort が最後まで詰められるか不安なため */
    p = data; wp = kouho_list; ewp = wp + len + 2;
    for (i = 0 ; i < n ; i++) {
      int ulen = L4TOL(p); p += SIZEOFLONG;
      wp += euc2ushort( p, ulen, wp, ewp - wp);
      p += ulen;
    }
    bun->kanji = kouho_list;
  }
  return n;
}


int rkc_get_kanji_list( RkcContext* cx )
{
  int reply;

  if (SendTypeE4Request(IR_KAN_LST, (int)cx->server, (int)cx->curbun,
			BUFSIZE) &&
      RecvTypeE2Reply(&reply, kanlisStore, (BYTE *)cx, 0)) {
    return reply;
  }
  return -1;
}


static
int resizeStore( int n, BYTE* data, int datalen, BYTE* contex, int v, int u)
{
  return firstKouhoStore(n, (RkcContext *)contex, data, datalen);
}


static
int rkc_resize( RkcContext* cx, int yomi_length )
{
    cannawc cbuf[CBUFSIZE];
    register int ret, euclen = 0;
    short curbun;
    int reply;

    if( yomi_length > 0 ){
	curbun = cx->curbun;
	for( ; (cx->curbun < cx->maxbun) ; (cx->curbun)++ ) {
	    /* めっちゃ効率悪いけどしゃあないかぁ */
	    /* だいたいこいつら ushortの事なんか考えてへんやんか */
	    if( (ret = _RkwGetYomi( cx, cbuf, CBUFSIZE )) < 0 ) {
		cx->curbun = curbun;
		return( -1 ) ;
	    }
	    if( yomi_length > ret ) {
		euclen += ushort2eucsize(cbuf, ret);
		yomi_length -= ret;
	    } else {
		euclen += ushort2eucsize(cbuf, yomi_length);
		break;
	    }
	}
	cx->curbun = curbun;
    } else
	euclen = yomi_length;

    if (SendTypeE4Request(IR_RESIZE, cx->server, cx->curbun, euclen) &&
	RecvTypeE2Reply(&reply, resizeStore, (BYTE *)cx, 0)) {
      return reply;
    }
    return -1;
}


static
int rkc_store_yomi( RkcContext* cx, cannawc* yomi, int max_ )
{
    int reply, len;
    unsigned char cbuf[BUFSIZE], *bufp = cbuf;

    len = ushort2eucsize(yomi, max_);
    if (len + 2 <= BUFSIZE || (bufp = (unsigned char*) malloc(len + 2))) {
        (void)ushort2euc(yomi, max_, bufp, len + 2);
    if (!SendTypeE6Request(IR_STO_YOMI, (int)cx->server, (int)cx->curbun,
			   (BYTE *)bufp, len)
	|| !RecvTypeE2Reply(&reply, resizeStore, (BYTE *)cx, 0)) {
      reply = -1;
    }
    if (bufp != cbuf) free(bufp);
  }
  else {
    reply = -1;
  }
  return reply;
}

static long RemoteDicUtilBaseProtoNumber = 0;

static
int Query_Extension()
{
    if( !RemoteDicUtilBaseProtoNumber ){
	int datalen = strlen( REMOTE_DIC_UTIL ) + 1 ;

        if (SendTypeE12Request(IR_QUERY_EXT, (BYTE *)REMOTE_DIC_UTIL, datalen,
			       MAXEXTREQUESTNO + 1) &&
	    RecvTypeE1Reply(&RemoteDicUtilBaseProtoNumber)) {
	  return RemoteDicUtilBaseProtoNumber;
        }
        return -1;
    } else {
	return( RemoteDicUtilBaseProtoNumber ) ;
    }
}

#ifdef EXTENSION
static int
rkc_list_dictionary( RkcContext* cx, const char* dirname, char* dicnames_return,
                         int size )
{
  int reply, datalen = strlen(dirname) + 1;
  int extension_base = Query_Extension() ;

  if (extension_base < 0) return -1;

  if (size < datalen) datalen = size;

  if (SendTypeE6Request(IR_LIST_DIC + extension_base,
			cx->server, size, (BYTE *)dirname, datalen) &&
      RecvTypeE2Reply(&reply, dicStore, (BYTE*) dicnames_return, size)) {
    return reply;
  }
  return -1;
}


static
int rkc_create_dictionary( RkcContext* cx, char* dicname, int mode )
{
    int extension_base = Query_Extension() ;

    if( extension_base < 0 )
	return( -1 ) ;

    return mount_dic(IR_CREAT_DIC + extension_base, cx->server,
		     dicname, mode);
}


static
int rkc_destroy_dictionary( RkcContext* cx, char* dicname, int xx)
{
    int extension_base = Query_Extension() ;

    if( extension_base < 0 )
	return( -1 ) ;

    return mount_dic(IR_DEST_DIC + extension_base, cx->server,
		     dicname, 0);
}


static
int rkc_rename_dictionary( RkcContext* cx, char* dicname, char* newdicname, int mode )
{
    long reply;
    int extension_base = Query_Extension() ;

    if( extension_base < 0 )
	return( -1 ) ;

    if (SendTypeE11Request(IR_RENAME_DIC + extension_base,
			   (int)cx->server,
			   (BYTE *)dicname, strlen(dicname) + 1,
			   (BYTE *)newdicname, strlen(newdicname) + 1,
			   mode) &&
	RecvTypeE1Reply(&reply)) {
      return reply;
    }
    return -1;
}


/* ARGSUSED */
static
int rkc_get_text_dictionary( RkcContext* cx, const char* dirname,
                             const char* dicname, cannawc* info, int infolen )
{
    int extension_base = Query_Extension() ;
    int ret ;

    if( extension_base < 0 )
	return( -1 ) ;

    if (SendTypeE11Request(IR_GET_WORD_DIC + extension_base,
			   (int)cx->server,
			   (BYTE *)dirname, strlen(dirname) + 1,
			   (BYTE *)dicname, strlen(dicname) + 1,
			   infolen) &&
	RecvTypeE2Reply(&ret, yomiStore,
			(BYTE *)info, infolen * SIZEOFSHORT)) {
      return ret;
    }
    return -1;
}


#endif /* EXTENSION */

/* ARGSUSED */
static
int statStore( int n, BYTE* src, int slen, RkStat* dest, int maxn, int unit)
{
  if (!(n < 0)) {
    dest->bunnum = (int)L4TOL(src);	/* bunsetsu bangou */
    src += SIZEOFLONG;
    dest->candnum = (int)L4TOL(src);	/* kouho bangou */
    src += SIZEOFLONG;
    dest->maxcand = (int)L4TOL(src);	/* sou kouho suu */
    src += SIZEOFLONG;
    dest->diccand = (int)L4TOL(src);	/* jisho ni aru kouho suu */
    src += SIZEOFLONG;
    dest->ylen = (int)L4TOL(src);	/* yomigana no nagasa (in byte) */
    src += SIZEOFLONG;
    dest->klen = (int)L4TOL(src);	/* kanji no nagasa (in byte) */
    src += SIZEOFLONG;
    dest->tlen = (int)L4TOL(src);	/* tango no kosuu */
  }
  return n;
}


/* ARGSUSED */
static
int lexStore( int n, BYTE* src, int slen, RkLex* dest, int maxn, int unit)
{
  int i;

  if (n > 0 && n > maxn) n = maxn;

  for (i = 0; i < n; i++, dest++) {
    dest->ylen = (int)L4TOL(src);	/* yomigana no nagasa (in byte) */
    src += SIZEOFLONG;
    dest->klen = (int)L4TOL(src);	/* kanji no nagasa (in byte) */
    src += SIZEOFLONG;
    dest->rownum = (int)L4TOL(src);	/* row number */
    src += SIZEOFLONG;
    dest->colnum = (int)L4TOL(src);	/* column number */
    src += SIZEOFLONG;
    dest->dicnum = (int)L4TOL(src);	/* dic number */
    src += SIZEOFLONG;
  }
  return n;
}


static
int rkc_get_stat( RkcContext* cx, RkStat* stat )
{
    int reply, n, i;
    cannawc cbuf[CBUFSIZE], *src_kanji;

    if (SendTypeE4Request(IR_GET_STA, cx->server, cx->curbun,
			cx->bun[cx->curbun].curcand) &&
        RecvTypeE3Reply(&reply, (StoreFunc) statStore,
		      (BYTE *)stat, 1, sizeof(RkStat) / sizeof(int))) {
    if (reply == 0) {
	stat->ylen = _RkwGetYomi(cx, cbuf, CBUFSIZE);
	switch( cx->bun[cx->curbun].flags ){
	case NOTHING_KOUHO:
	    stat->klen = stat->ylen;
	    break;
	case FIRST_KOUHO:
	    stat->klen = ushortstrlen(cx->bun[cx->curbun].kanji);
	    break;
	case NUMBER_KOUHO:
	    src_kanji = cx->bun[cx->curbun].kanji;
	    n = cx->bun[cx->curbun].curcand;
	    for (i = 0 ; i < n ; i++)
		src_kanji += ushortstrlen( src_kanji ) + 1;
	    stat->klen = ushortstrlen(src_kanji);
	    break;
	}
    }
  }
  return reply;
}


static
int rkc_get_lex( RkcContext* cx, int max, RkLex* info_return )
{
    int ret, len, i, ylen, klen;
    unsigned char kbuf[BUFSIZE], ybuf[BUFSIZE];
    cannawc cbuf[CBUFSIZE], *src_kanji;
    RkcBun *bun = cx->bun + cx->curbun;
    RkLex *tango;

    if (!SendTypeE5Request(IR_GET_LEX, cx->server, cx->curbun,
			   cx->bun[cx->curbun].curcand, max) ||
        !RecvTypeE4Reply(&ret, (StoreFunc) lexStore, (BYTE *)info_return,
			 max, sizeof(RkLex) / sizeof(int))) {
      ret = -1;
    }

    if (ret >= 0) {
	len = _RkwGetYomi(cx, cbuf, CBUFSIZE);
	(void)ushort2euc(cbuf, len, ybuf, BUFSIZE);
	switch( bun->flags ){
	case NOTHING_KOUHO:
	    src_kanji = cbuf;
	    break;
	case FIRST_KOUHO:
	    src_kanji = bun->kanji;
	    break;
	case NUMBER_KOUHO:
	    src_kanji = bun->kanji;
	    for( i = 0; i < bun->curcand; i++ )
		src_kanji += ushortstrlen( src_kanji ) + 1;
	    break;
	}
	(void)ushort2euc(src_kanji, ushortstrlen(src_kanji), kbuf, BUFSIZE);
	ylen = klen = 0;
	tango = info_return;
	for( i = 0; i < ret; i++ ){
	    len = tango->ylen;
	    tango->ylen = euc2ushort(ybuf + ylen, len, cbuf, CBUFSIZE);
	    ylen += len;
	    len = tango->klen;
	    tango->klen = euc2ushort(kbuf + klen, len, cbuf, CBUFSIZE);
	    klen += len;
	    tango++;
	}
    }
    return( ret );
}

#endif /* USE_EUC_PROTOCOL */

static int
rkc_error()
{
    return -1;
}

struct rkcproto eucproto = {
#ifdef USE_EUC_PROTOCOL
  rkc_initialize,
  rkc_finalize,
  rkc_close_context,
  rkc_create_context,
  rkc_duplicate_context,
  rkc_dictionary_list,
  rkc_define_dic,
  rkc_delete_dic,
  rkc_mount_dictionary,
  rkc_remount_dictionary,
  rkc_umount_dictionary,
  rkc_mount_list,
  rkc_convert,
  rkc_convert_end,
  rkc_get_kanji_list,
  rkc_get_stat,
  rkc_resize,
  rkc_store_yomi,
  rkc_get_yomi,
  rkc_get_lex,
  (autoconv_t)rkc_error,
  (subst_yomi_t)rkc_error,
  (flush_yomi_t)rkc_error,
  (get_last_yomi_t)rkc_error,
  (remove_bun_t)rkc_error,
  (get_simple_kanji_t)rkc_error,
  (query_dic_t)rkc_error,
  (get_hinshi_t)rkc_error,
  (store_range_t)rkc_error,
  (set_locale_t)rkc_error,
  (set_app_name_t)rkc_error,
  (notice_group_name_t)rkc_error,
  (through_t)rkc_error,
  (killserver_t)rkc_error,
#ifdef EXTENSION
  rkc_list_dictionary,
  rkc_create_dictionary,
  rkc_destroy_dictionary,
  rkc_rename_dictionary,
  rkc_get_text_dictionary,
  (sync_t)rkc_error,
  (chmod_dic_t)rkc_error,
  (copy_dictionary_t)rkc_error,
#endif /* EXTENSION */
#else /* !USE_EUC_PROTOCOL */
  (initialize_t)rkc_error,
  (finalize_t)rkc_error,
  (close_context_t)rkc_error,
  (create_context_t)rkc_error,
  (duplicate_context_t)rkc_error,
  (dictionary_list_t)rkc_error,
  (define_dic_t)rkc_error,
  (delete_dic_t)rkc_error,
  (mount_dictionary_t)rkc_error,
  (remount_dictionary_t)rkc_error,
  (umount_dictionary_t)rkc_error,
  (mount_list_t)rkc_error,
  (convert_t)rkc_error,
  (convert_end_t)rkc_error,
  (get_kanji_list_t)rkc_error,
  (get_stat_t)rkc_error,
  (resize_t)rkc_error,
  (store_yomi_t)rkc_error,
  (get_yomi_t)rkc_error,
  (get_lex_t)rkc_error,
  (autoconv_t)rkc_error,
  (subst_yomi_t)rkc_error,
  (flush_yomi_t)rkc_error,
  (get_last_yomi_t)rkc_error,
  (remove_bun_t)rkc_error,
  (get_simple_kanji_t)rkc_error,
  (query_dic_t)rkc_error,
  (get_hinshi_t)rkc_error,
  (store_range_t)rkc_error,
  (set_locale_t)rkc_error,
  (set_app_name_t)rkc_error,
  (notice_group_name_t)rkc_error,
  (through_t)rkc_error,
  (killserver_t)rkc_error,
#ifdef EXTENSION
  (list_dictionary_t)rkc_error,
  (create_dictionary_t)rkc_error,
  (remove_dictionary_t)rkc_error,
  (rename_dictionary_t)rkc_error,
  (get_text_dictionary_t)rkc_error,
  (sync_t)rkc_error,
  (chmod_dic_t)rkc_error,
  (copy_dictionary_t)rkc_error,
#endif /* EXTENSION */
#endif /* !USE_EUC_PROTOCOL */
};
