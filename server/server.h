// -*- coding:utf-8-with-signature -*-
/* Copyright (c) 2003 Canna Project. All rights reserved.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the
 * author and contributors not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.  The author and contributors no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * THE AUTHOR AND CONTRIBUTORS DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE AUTHOR AND CONTRIBUTORS BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTUOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: server.h,v 1.4 2003/09/23 07:11:31 aida_s Exp $ */

#ifndef	SERVER_H
#define SERVER_H

#include "canna/ccompat.h"
#include <stdio.h>
#include <sys/types.h>

#include <sys/time.h>
#include <time.h>
#include <sys/times.h> // file access and modification times structure
#include <sys/select.h>
#include <sys/socket.h>

#include "canna/net.h"
#ifndef _WIN32
  #include <unistd.h>
#endif
#include <sys/stat.h>
#include <assert.h>
#include "RKindep/ecfuncs.h"

/* 自動判別支援コメント: これはEUC-JPだぞ。幅という字があれば大丈夫。 */

typedef struct tagEventMgr EventMgr;
typedef struct tagClientBuf ClientBuf;
//typedef struct tagSockHolder SockHolder;
typedef struct tagUserTable UserTable;
//typedef struct _Address Address;
typedef struct _Client *ClientPtr;
typedef struct _ClientStat *ClientStatPtr;

#define DEBUG

#include "IR.h"
#include "comm.h"

//#if !CANNA_LIGHT
// 定義必須.
#define USE_EUC_PROTOCOL
//#endif /* !CANNA_LIGHT */

#define DDPATH              "canna"
#define DDUSER              "user"
#define DDGROUP             "group"
#define DDPATHLEN           (sizeof(DDPATH) - 1)
#define DDUSERLEN           (sizeof(DDUSER) - 1)
#define DDGROUPLEN          (sizeof(DDGROUP) - 1)

#define DATE_LENGH	    29
#define GETDATE 	    1
#define CONNECT 	    2
#define SETTIME 	    3
#define GETTIME 	    4

#define LOCAL_BUFSIZE		2048

#ifndef NDEBUG
#define ir_debug( cannadebug )	 cannadebug
#else
#define ir_debug( cannadebug )
#endif

/* subset of struct addrinfo */
/*
struct _Address {
    int family;
    size_t len;
//#ifdef INET6
    struct sockaddr_storage saddr; // XXX huge padding
#else
    struct sockaddr_in saddr;
#endif
};
*/

// server/misc.c で使用.
//# define IR_ADDR_IN6SA(x) ((struct sockaddr_in6 *)&(x)->saddr)
# define IR_ADDR_IN6(x) (&IR_ADDR_IN6SA(x)->sin6_addr)
# define IR_ADDR_IN6SCOPE(x) (IR_ADDR_IN6SA(x)->sin6_scope_id)
//# ifdef IPV6_V6ONLY
//#  define IR_V6ONLY_BIND
//# endif /* IPV6_V6ONLY */
//# if defined(IR_V6ONLY_BIND) || defined(sun)
//#  define IR_V4MAPPED_AVOIDABLE
//# endif
//# ifndef IR_V4MAPPED_AVOIDABLE
//#  error "You need newer IPv6 stack."
//# endif


/* クライアント毎に作られる、レイヤ5の情報を持つ構造体 */
typedef struct _Client {
    struct tagClientBuf *client_buf ;        /* バッファ */
    int 	usr_no ;		     /* ユーザ管理番号 */
    short 	version_hi ;		     /* protocol major version */
    short 	version_lo ;		     /* protocol miner version */
    time_t	used_time ;		     /* ユーザ消費時間 */
    time_t	idle_date ;		     /* アイドル時間 */
    time_t	connect_date ;		     /* コネクトした時間 */
    char	*username ;		     /* ユーザ名  */
    char	*groupname;		     /* グループ名  */
    char	*hostname ;		     /* ホスト名  */
    struct sockaddr_storage hostaddr;		     /* ホストアドレス */
    int 	pcount[ W_MAXREQUESTNO ] ;   /* プロトコルカウント */
    int		*context_flag;               /* コンテクスト管理フラグ */
    int		cfsize, ncon;		     /* 上のテーブルの大きさ管理 */
    char	*clientname ;		     /* クライアント名  */
} ClientRec ;

typedef struct _ClientStat {
    int 	id ;			     /* ソケット番号 */
    int 	usr_no ;		     /* ユーザ管理番号 */
    time_t	used_time ;		     /* ユーザ消費時間 */
    time_t	idle_date ;		     /* アイドル時間 */
    time_t	connect_date ;		     /* コネクトした時間 */
    int 	pcount[ OLD_MAXREQUESTNO ] ; /* プロトコルカウント */
    char	username[ NAME_LENGTH+1] ;   /* ユーザ名  */
    char	hostname[ HOST_NAME ] ;      /* ホスト名  */
    char	context_flag[ OLD_MAX_CX ] ;	 /* コンテクスト管理フラグ */
} ClientStatRec ;

typedef struct _AddrList {
    struct sockaddr_storage addr;
    struct _AddrList *next;
} AddrList;

typedef struct _AccessControlList {
    struct _AccessControlList  *prev ;
    struct _AccessControlList  *next ;
    char *hostname ;
    // '\0' 区切りのユーザ名リスト.
    char *usernames ;
    int  usercnt ;
    AddrList *hostaddrs;
} ACLRec ;

typedef struct _AccessControlList *ACLPtr ;

#ifdef USE_INET_SOCKET
/* Flag for using IPv4/IPv6 socket */
extern int UseInet;
//#ifdef INET6
//extern int UseInet6;
//#endif /* INET6 */
#endif

extern int PortNumberPlus;
extern UserTable *global_user_table;
#ifdef DEBUG
extern const char *DebugProc[][2];
extern const char *DebugProcWide[][2];
#endif
extern const char *CallFuncName;
extern int (*CallFunc) pro((ClientPtr *clientp));
extern ACLPtr ACLHead ;
extern int canna_server_hi;
extern int canna_server_lo;

/* misc.c */
//#ifdef __STDC__
//# define USE_VARARGS
//#endif
#define vapro(x) x

#ifndef NDEBUG
void Dmsg(int Pri, const char *f, ...);
#endif /* DEBUG */
void PrintMsg vapro((const char *f, ...));
void nomem_msg pro((const char *));

void EarlyInit pro((int, char **));
int BecomeDaemon pro((void));
void CloseServer pro((void));
int CheckSignal pro((void));
AddrList *GetAddrListFromName pro((const char *hostname));
const AddrList *SearchAddrList pro((const AddrList *list,
                              const struct sockaddr_storage* addrp));
void FreeAddrList pro((AddrList *list));
int NumberAccessControlList pro((void));
int CheckAccessControlList( const struct sockaddr_storage* hostaddrp,
                            const char *username );
int SetDicHome pro((ClientPtr client, int cxnum));
ClientPtr *get_all_other_clients pro((ClientPtr self, size_t *count));
void AllSync pro((void));
void DetachTTY pro((void));

/* convert.c */
int ir_nosession pro((ClientPtr *clientp, ClientBuf *client_buf));
int ir_error pro((ClientPtr *clientp));
#ifdef DEBUG
void DebugDispKanji pro((int cxnum, int num));
void DebugDump pro((int level, const char *buf, int size));
#endif

/* wconvert.c */
int parse_wide_request pro((int *request, BYTE *data, size_t len,
      const char *username, const char *hostname));
int parse_euc_request pro((int *request, BYTE *data, size_t len,
      const char *username, const char *hostname));
char *insertUserSla pro((char *, int));
int checkPermissionToRead pro((ClientPtr client,
      char *dirname, char *dicname));

/* session.c */
UserTable *UserTable_new pro((void));
void UserTable_delete pro((UserTable *obj));
void close_session pro((ClientPtr *clientp, int notify));
int open_session pro((ClientPtr *clientp, char *name, ClientBuf *client_buf));
void ClientStat pro((ClientPtr client, int type,
      int request_Type, char *return_date));

/* util.c */
#define ushort2euc CNvW2E

size_t euc2ushort pro((const char *src, size_t srclen,
      Ushort *dest, size_t destlen));
size_t ushortstrlen pro((const Ushort *ws));
Ushort *ushortmemchr pro((const Ushort *ws, int ch, size_t len));
size_t ushortstrcpy pro((Ushort *wd, const Ushort *ws));
size_t ushortstrncpy pro((Ushort *wd, const Ushort *ws, size_t len));
int WidenClientContext pro((ClientPtr cli, size_t n));
int set_cxt pro((ClientPtr cl, int n));
void off_cxt pro((ClientPtr cl, int cn));
int chk_cxt pro((ClientPtr cl, int cn));

#endif	/* SERVER_H */
/* vim: set sw=2: */
