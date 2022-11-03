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
static char rcs_id[]="@(#) $Id: misc.c,v 1.16.2.4 2004/04/26 21:48:37 aida_s Exp $";
#endif

/* LINTLIBRARY */

#define _DEFAULT_SOURCE // vsyslog(), since glibc 2.19
#define _POSIX_C_SOURCE 200809L
#include "server.h"

#include <syslog.h>
#include <grp.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <ctype.h>
#ifdef HAVE_SYS_IOCTL_H
  #include <sys/ioctl.h>
#endif
#ifndef _WIN32
  #include <unistd.h>
  #ifdef USE_UNIX_SOCKET
    #include <sys/un.h>
  #endif
#endif

#ifndef DICHOME
  #define DICHOME  LOCALSTATE_DIR "/lib/canna/dic"
#endif

#ifndef ERRDIR
  #define ERRDIR   LOCALSTATE_DIR "/spool/canna"
#endif

#define ERRFILE     "CANNA"
#define ERRFILE2    "msgs"
#define ERRSIZE     64

#ifndef ACCESS_FILE
// The list of hostname allowed to connect to the Canna server.
//   ホスト名:ユーザ名1, ユーザ名2, ...
//   ホスト名
//   unix
// ホスト名として "unix" がないと, UNIX ドメインソケットで接続できない.
// 例:
//   localhost
//   unix
#define ACCESS_FILE "/etc/hosts.canna"
#endif

static void FatalError pro((const char *f));
static int CreateAccessControlList pro((void));
static void FreeAccessControlList pro((void));


#ifdef DEBUG
#define LOGFILE "/tmp/canna.log"
static FILE *ServerLogFp = NULL;
static FILE *Fp;
static int DebugMode = 0;
static int LogLevel = 0;
#endif
static int Syslog = 0; /* syslog を通すかどうかのフラグ */

int PortNumberPlus = 0;
int MMountFlag = 0; /* メモリに辞書をロードするかしないかのフラグ */
static char Name[64];

static char *userID=NULL; /* canna server's user id */

#ifdef USE_INET_SOCKET
/* flag for using INET Domain Socket */
#ifdef USE_UNIX_SOCKET
/* By default, not use IPv4/IPv6 socket, if UNIX Domain Socket is available. */
int UseInet = 0;
#else
/* if can use Unix Domain Socket, Use INET domain socket */
int UseInet = 1;
#endif
#endif

#define MAX_PREMOUNTS 20

char *PreMountTabl[MAX_PREMOUNTS];
int npremounts = 0;
static char *MyName ;

// アクセス許可するクライアントIPアドレス一覧.
ACLPtr ACLHead = NULL;

static int caught_signal = 0;
static int openlog_done = 0;
static int rkw_initialize_done = 0;

static void Reset(int );

#ifdef USE_INET_SOCKET
#define USAGE "Usage: cannaserver [-p port_num] [-l num] [-u userid] [-syslog] [-inet] [-d] [dichome]\n" \
    "    -p port_num  If UNIX domain socket, use another filename"
#else
#define USAGE "Usage: cannaserver [-p port_num] [-l num] [-u userid] [-syslog] [-d] [dichome]" \
    "    -p port_num  If UNIX domain socket, use another filename"
#endif // USE_INET_SOCKET


static void Usage()
{
  FatalError(USAGE);
}

extern void getserver_version pro((void));

// コマンドラインオプションの解析.
void EarlyInit( int argc, char* argv[] )
{
    char *ddname = NULL;  // 辞書の場所
    char buf[ MAXDATA ];
    int     i;
    int     context;
    struct  passwd *pwent;

    strcpy( Name, argv[ 0 ] );

    for( i = 1; i < argc; i++ ) {
        if( argv[i][0] == '/' ) {
            ddname = (char*) malloc(strlen(argv[i]) + 1);
	    if( ddname )
                strcpy( ddname, argv[ i ] );
	}

        if( !strcmp( argv[i], "-p") ) {
            if (++i < argc) {
                PortNumberPlus = atoi( argv[i] ) ;
                if ( PortNumberPlus < 1 || PortNumberPlus > 65535 ) {
                    fprintf(stderr, "valid port number range is 1 <= num < 65535\n");
                    exit(2);
                }
            }
	  else {
	    fprintf(stderr, "%s\n", USAGE);
	    exit(2);
	    /* NOTREACHED */
	  }
	}
	else if( !strcmp( argv[i], "-u")) {
	  if (++i < argc) {
	    userID = argv[i];
	  }
	  else {
	    fprintf(stderr, "%s\n", USAGE);
	    exit(2);
	    /* NOTREACHED */
	  }
	}
#ifdef USE_INET_SOCKET
	else if( !strcmp( argv[i], "-inet")) {
	  UseInet = 1;
	}
#endif
#ifdef RK_MMOUNT
	else if( !strcmp( argv[i], "-m") ) {
	  MMountFlag = RK_MMOUNT;
	}
#endif
        else if (!strcmp( argv[i], "-syslog")) {
            Syslog = 1;
        }
    }

    if (Syslog) {
      openlog("cannaserver", LOG_PID, LOG_DAEMON);
      openlog_done = 1;
    } /* -syslog だったら、ログファイルを初期化する */

    if( !ddname ) {
        ddname = (char*) malloc(strlen(DICHOME) + 1);
	if( !ddname )
            FatalError("cannaserver:Initialize failed: malloc()\n");
        strcpy( ddname, DICHOME );
    }

    if (userID != NULL) {
        pwent = getpwnam(userID);
	if (pwent) {
	    if(setgid(pwent->pw_gid)) {
	        FatalError("cannaserver:couldn't set groupid to canna user's group\n");
	    }
#ifdef HAVE_INITGROUPS
	    if (initgroups(userID, pwent->pw_gid)) {
	        FatalError("cannserver: couldn't init supplementary groups\n");
	    }
#endif
	    if (setuid(pwent->pw_uid)) {
	        FatalError("cannaserver: couldn't set userid\n");
	    }
	} else if (userID != NULL) {
	    FatalError("cannaserver: -u flag specified, but canna not run as root\n");
	}
    }

#ifdef DEBUG
    DebugMode = 0 ;
    ServerLogFp = stderr ;

    for( i = 1; i < argc; i++ ) {
	if( !strcmp( argv[ i ], "-d" )) {
	    DebugMode = 1 ;
	    LogLevel = 5 ;
	}

	if( !strcmp( argv[ i ], "-l" ) ) {
	  if (++i < argc) {
	    LogLevel = atoi(argv[ i ]);
	    if( LogLevel <= 0 )
		LogLevel = 1 ;
	  }
	  else {
	    Usage();
	    /* NOTREACHED */
	  }
	}
    }

    if (LogLevel && !DebugMode) {
	/* ログファイル作成 */
	if( (Fp = fopen( LOGFILE, "w" ) ) != NULL ){
	    ServerLogFp = Fp ;
	} else {
	    LogLevel = 0;
	    perror("Can't Create Log File!!\n");
	}
    }

#endif /* DEBUG */

    getserver_version() ;

   ir_debug( Dmsg(5, "辞書ホームディレクトリィ = %s\n", ddname ); )

    if ((context = RkwInitialize( (char *)ddname )) < 0)
        FatalError("cannaserver:Initialize failed: RkwInitialize()\n") ;
    rkw_initialize_done = 1;
    free( (char *)ddname ) ;
    RkwCloseContext( context ) ;

    if (gethostname( buf, MAXDATA ) == 0) {
      MyName = malloc(strlen(buf) + 1);
      if (MyName) {
	strcpy(MyName, buf);
      }
    }

   ir_debug( Dmsg(5, "My name is %s\n", MyName ); )

    bzero(PreMountTabl, MAX_PREMOUNTS * sizeof(unsigned char *));

    CreateAccessControlList() ;
}

static void
mysignal(int sig, void (*func)(int) )
{
    struct sigaction new_action;

    sigemptyset(&new_action.sa_mask);
    new_action.sa_handler = func;
    new_action.sa_flags = 0
# ifdef	SA_INTERRUPT
	| SA_INTERRUPT /* don't restart */
# endif
	;
    sigaction(sig, &new_action, NULL);
}

int
BecomeDaemon ()
{
    int     parent, parentid;

    if (DebugMode) {
	mysignal(SIGPIPE,  SIG_IGN) ;
	return 0; /* デーモンにならない */
    }

    parentid = getpid() ;

#ifndef __EMX__
    if ((parent = fork()) == -1) {
	PrintMsg( "Fork failed\n" );
	exit( 1 ) ;
    }
    if ( parent ) {
	_exit( 0 ) ;
    }
    return parentid;
#else
    return 0;
#endif
}

void
CloseServer()
{
    if (Syslog && openlog_done) {
        closelog();
    }

    if (rkw_initialize_done)
	RkwFinalize() ;
}


/* 初期化に失敗した場合に呼ぶ。EventMgr_run()まで来たら呼ばないこと。 */
static void FatalError(const char* f)
{
    fprintf(stderr,"%s\n", f);
    CloseServer();
    exit(2);
    /*NOTREACHED*/
}

#define MAXARGS 10

#ifndef NDEBUG

void Dmsg(int Pri, const char *f, ...)
{
  va_list ap;

  va_start(ap, f);

  if (!ServerLogFp) {
    ServerLogFp = stderr;
  }
  if (LogLevel >= Pri) {
    vfprintf(ServerLogFp, f, ap);
    fflush(ServerLogFp);
  }
  va_end(ap);
}
#endif


void PrintMsg(const char *f, ...)
{
    va_list ap;
#if !defined(HAVE_VSYSLOG)
    const char *args[MAXARGS];
    int argno = 0;
#endif
    time_t Time;
  char    *date;

    va_start(ap, f);

#if !defined(HAVE_VSYSLOG)
    while (++argno < MAXARGS && (args[argno] = va_arg(ap, const char *)))
        ;
    args[MAXARGS - 1] = (const char *)0;
#endif

    if (Syslog) {
#ifdef HAVE_VSYSLOG
        vsyslog(LOG_WARNING, f, ap);
#else
        syslog(LOG_WARNING, f, args[0], args[1], args[2], args[3], args[4],
               args[5], args[6], args[7], args[8]);
#endif
    } else {
    Time = time(NULL);
    date = (char *)ctime(&Time);
    date[24] = '\0';
    fprintf(stderr, "%s :", date);
    vfprintf(stderr, f, ap);
    fflush( stderr ) ;
  }
    va_end(ap);
}


void nomem_msg( const char *where )
{
  if (where)
    PrintMsg("%s: out of memory\n", where);
  else
    PrintMsg("out of memory\n");
}


// callback
static void Reset(int sig)
{
    caught_signal = sig;
#ifdef SIGNALRETURNSINT
    return 0;
#endif
}

int
CheckSignal()
{
    if( caught_signal == SIGTERM ) {
	PrintMsg( "Cannaserver Terminated\n" ) ;
	return 1;
    } else if(caught_signal) {
	PrintMsg( "Caught a signal(%d)\n", caught_signal ) ;
	return 1;
    }
    return 0;
}


#define IR_ADDR_IN(x) ( ((struct sockaddr_in*)(x))->sin_addr )

static int
AddrAreEqual( const struct sockaddr_storage* x,
              const struct sockaddr_storage* y )
{
    // int IN6_ARE_ADDR_EQUAL(const struct in6_addr *, const struct in6_addr *)
    // は RFC 2292, RFC 3542 で定義される. POSIX ではない.

    int res = 0;
    if (x->ss_family != y->ss_family)
        return 0;

    switch (x->ss_family) {
    case AF_UNIX:
        //res = !strcmp( ((struct sockaddr_un*) x)->sun_path,
        //((struct sockaddr_un*) y)->sun_path );
        return 1;  // ACL リストにはパス名は入っていない.

    case AF_INET:
        // sin_port は調べない.
        res = IR_ADDR_IN(x).s_addr == IR_ADDR_IN(y).s_addr;
	break;

    case AF_INET6:
        {
            const struct sockaddr_in6* x6 = (struct sockaddr_in6*) x;
            const struct sockaddr_in6* y6 = (struct sockaddr_in6*) y;

            // リンクローカルアドレス fe80::/10 ではスコープid必須.
            // -> IPv6アドレスとスコープidの比較が必要.
            // 余談: サイトローカルアドレスは RFC 3879 (Sep 2004) で廃止.
            // sin6_port は調べない.
            res = x6->sin6_scope_id == y6->sin6_scope_id &&
                  IN6_ARE_ADDR_EQUAL(&x6->sin6_addr, &y6->sin6_addr);
        }
        break;

    default:
        abort(); /* NOTREACHED */
    }
    return res;
}


/**
 * CreateAccessControlList() から呼び出される. ホスト名からアドレスを正引きし
 * て, AddrList エントリを生成する.
 * @param hostname ホスト名. "unix" の場合, UNIX ドメインソケットとみなす.
 * @return 新しい AddrList エントリ. 呼出し側が FreeAddrList() すること.
 *         失敗したら NULL.
 */
AddrList* GetAddrListFromName( const char* hostname )
{
    AddrList *res = NULL;
    struct addrinfo hints, *info;
    struct addrinfo* infolists = NULL;
    int i;

#ifdef USE_UNIX_SOCKET
    if (!strcmp(hostname, "unix")) {
        res = calloc(1, sizeof(AddrList));
        if (!res)
            return NULL;
        struct sockaddr_un* u = &res->addr;
        u->sun_family = AF_UNIX;
        u->sun_path[0] = '\0';
        res->next = NULL;
        return res;
    }
#endif // USE_UNIX_SOCKET

    bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC; // IPv4/IPv6両対応
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG; // 実行ホストが IPv6 only の場合, IPv4-Mapped IPv6 Address を返す.
    getaddrinfo(hostname, NULL, &hints, &infolists);

    for (info = infolists; info; info = info->ai_next) {
        AddrList *newnode;
        newnode = calloc(1, sizeof(AddrList));
        if (!newnode) {
            freeaddrinfo( infolists );
            goto failed;
        }
        memcpy(&newnode->addr, info->ai_addr, info->ai_addrlen);
        newnode->next = res;
        res = newnode;
    }
    freeaddrinfo( infolists );

    return res;

failed:
    FreeAddrList(res);
    return NULL;
}


const AddrList *
SearchAddrList( const AddrList *list, const struct sockaddr_storage* addrp )
{
    for (; list; list = list->next) {
        if (AddrAreEqual(&list->addr, addrp))
            break;
    }
    return list;
}

void
FreeAddrList( AddrList *list )
{
    while (list) {
        AddrList *next = list->next;
        free(list);
        list = next;
    }
}


static char* skip_space(char* p) {
    while (*p && isspace(*p) ) p++;
    return p;
}

// /etc/hosts.canna ファイル (許可ホストリスト) を読み込む.
// IPv6アドレスの場合は [] で囲むこと.
//     [fe80::xxxx:xxxx%hoge]:user1, user2, ...
// @return ファイルが見つからない場合 -1.
//         そのほかの失敗も -1.
static int
CreateAccessControlList()
{
    char   buf[BUFSIZE];
    char* wp;  // 語の先頭
    char* p ;
    ACLPtr  current;
    ACLPtr  prev = (ACLPtr)NULL ;
    FILE    *fp ;
    //int namelen;

    /* /etc/hosts.canna からホスト名を求める */
    if( (fp = fopen(ACCESS_FILE, "r")) == NULL )
        return -1 ;

    if (ACLHead)
      FreeAccessControlList();

    while( fgets(buf, BUFSIZE, fp) != NULL ) {
        wp = buf ;
        if ( (p = strchr(wp, '#')) != NULL ) // '#' 1行コメント
            *p = '\0';
        wp = skip_space(wp);
        if ( !*wp )
            continue;

        if( !(current = (ACLPtr)malloc( sizeof( ACLRec ) )) ) {
	    PrintMsg("Cound not create access control entry!" ) ;
	    fclose( fp ) ;
	    FreeAccessControlList() ;
            return -1 ;
        }
	bzero( current, sizeof( ACLRec ) ) ;

        if( *wp == '[' ) {  // IPv6アドレス
	    size_t bodylen;
	    wp++;
	    p = strchr( wp, ']' );
            if( !p ) {
                fprintf(stderr, "Invalid IPv6 address.\n");
                free(current);
                continue;
            }
            *p = '\0';
            /* ここでの形式チェックは厳密でなくてよい */
	    bodylen = strspn( wp, "0123456789ABCDEFabcdef:." );
            if ( !bodylen || !(wp[bodylen] == '\0' || wp[bodylen] == '%') ||
                    strchr(wp, ':') == NULL ) {
                fprintf(stderr, "Invalid IPv6 address.\n");
                free(current);
                continue;
            }

            current->hostname = strndup(wp, p - wp);
            p++;  // ']' をskip
        }
        else { // Hostname or IPv4 address
            p = wp;
            while (isalnum(*p) || *p == '.' || *p == '-')
                p++;
            // この後ろに来るのは ':' かコメントのみ.
            if ( p - wp == 0 || (*p && !(isspace(*p) || *p == ':')) ) {
                fprintf(stderr, "Invalid host name.\n");
                free(current);
                continue;
            }
            current->hostname = strndup(wp, p - wp);
        }

	/* AccessControlListをインターネットアドレスで管理する.
           ホスト名からインターネットアドレスを求めて ACLRecに登録する.
           hostname = "unix" の場合, UNIX ドメインソケットとみなす. */
        current->hostaddrs = GetAddrListFromName(current->hostname);
        if (!current->hostaddrs) {
	  /* アドレスが見つからない場合 */
	  /* インターネットアドレス表記が間違っているので無視する */
            fprintf(stderr, "Warning: Not found IP address: %s\n", current->hostname);
            free( current->hostname );
            free( current );
            continue;
        }
	/* 今のところアドレスが重複していてもそのまま覚えておく */

        // ユーザ名リストがある場合
        wp = skip_space(p);
        if( *wp ) {
            current->usernames = malloc(strlen(wp) + 1);
            if ( !current->usernames ) {
                free( current->hostname ); FreeAddrList(current->hostaddrs);
                free( current );
                fclose(fp);
                return -1;
            }

            char* q = current->usernames;
            while (1) {
                p = wp = skip_space(wp);

                // ユーザ名は Portable Filename Character Set
                while ( *p && (isalnum(*p) || strchr("._-", *p)) )
                    p++;
                if ( p - wp == 0 || (*p && !(isspace(*p) || *p == ',')) ) {
                    fprintf(stderr, "Invalid user name.\n");
                    if ( (p = strchr(p, ',')) != NULL ) {
                        p++; // ',' の次
                        continue;
                    }
                    break;
                }

                memcpy(q, wp, p - wp); q[p - wp] = '\0'; q += (p - wp) + 1;
                current->usercnt ++ ;

                p = skip_space(p);
                if ( !*p ) break;
                else if ( *p == ',' ) {
                    wp = p + 1; continue;
                }
                else {
                    // カンマなしに次の語.
                    fprintf(stderr, "Invalid access control entry.\n");
                    break;
                }
            }
        }

	if( ACLHead ) {
	    current->prev = prev ;
	    prev->next = current ;
	} else {
	    ACLHead = current ;
	    current->prev = NULL ;
	}
	current->next = NULL ;
	prev = current ;
    }

    fclose( fp ) ;
    return 0;
}

static void
FreeAccessControlList()
{
    ACLPtr  wp, tailp = NULL;

    if( !(wp = ACLHead) )
        return ;

    for( ; wp != NULL; wp = wp->next ) {
	    if( wp->hostname )
		free( wp->hostname ) ;
	    if( wp->usernames )
		free( wp->usernames ) ;
	    FreeAddrList( wp->hostaddrs ) ;
	    tailp = wp ;
    }

    for ( wp = tailp; wp != NULL; wp = wp->prev ) {
	if( wp->next )
	    free( wp->next ) ;
    }
    ACLHead = NULL ;
}


// server/session.c から呼び出される.
// @return 許可する = 0, 許可しない = -1
int
CheckAccessControlList( const struct sockaddr_storage* hostaddrp,
                        const char *username )
{
    int i;
    const char *userp;
    ACLPtr wp;

    if (!ACLHead) return 0;

    ir_debug(Dmsg(5, "My name is %s\n", MyName));

    for (wp = ACLHead ; wp ; wp = wp->next) {
    /* AccessControlListで持っているインタネットアドレスと一致する
       ものをサーチする */
        if (SearchAddrList(wp->hostaddrs, hostaddrp)) {
            if ( !wp->usernames )
                return 0;

            for (i = 0, userp = wp->usernames ; i < wp->usercnt ; i++) {
                if (!strcmp(userp, username))
                    return 0;
                userp += strlen(userp) + 1;
            }
            return -1;
        }
    }
    return -1;
}

int
NumberAccessControlList()
{
  ACLPtr wp;
  int n;

  for (wp = ACLHead, n = 0; wp ; wp = wp->next) {
    n++;
  }
  return n;
}


int SetDicHome( ClientPtr client, int cxnum )
{
    char dichome[ 256 ] ;

    if (cxnum < 0)
	return( -1 ) ;

    if (client->username && client->username[0]) {
      if (client->groupname && client->groupname[0]) {
	if (strlen(DDUSER) + strlen(client->username) +
	    strlen(DDGROUP) + strlen(client->groupname) +
	    strlen(DDPATH) + 4 >= 256)
	  return ( -1 );
	sprintf(dichome, "%s/%s:%s/%s:%s",
		DDUSER, client->username,
		DDGROUP, client->groupname,
		DDPATH);
      }
      else {
	if (strlen(DDUSER) + strlen(client->username) +
	    strlen(DDPATH) + 2 >= 256)
	  return ( -1 );
	sprintf(dichome, "%s/%s:%s",
		DDUSER, client->username,
		DDPATH);
      }
    }
    else {
      strcpy(dichome, DDPATH);
    }

   ir_debug( Dmsg(5,"辞書ホームディレクトリィ：%s\n", dichome ); )
    if( RkwSetDicPath( cxnum, dichome ) == -1 ) {
	return( -1 ) ;
    }
    return( 1 ) ;
}


ClientPtr *
get_all_other_clients( ClientPtr self, size_t *count )
{
    EventMgrIterator curr, end;
    ClientPtr *res, *p;

    *count = 0;
    EventMgr_clibuf_end(global_event_mgr, &end);

    for (EventMgr_clibuf_first(global_event_mgr, &curr);
	    curr.it_val != end.it_val;
	    EventMgrIterator_next(&curr)) {
	ClientPtr who = ClientBuf_getclient(curr.it_val);
	if (who && who != self)
	    ++*count;
    }

    res = p = malloc(*count * sizeof(ClientPtr));
    if (!res) {
	*count = 0;
	return res;
    }

    for (EventMgr_clibuf_first(global_event_mgr, &curr);
	    curr.it_val != end.it_val;
	    EventMgrIterator_next(&curr)) {
	ClientPtr who = ClientBuf_getclient(curr.it_val);
	if (who && who != self)
	    *p++ = who;
    }
    return res;
}

void
AllSync()
{
    EventMgrIterator curr, end;

    EventMgr_clibuf_first(global_event_mgr, &curr);
    EventMgr_clibuf_end(global_event_mgr, &end);

    for (EventMgr_clibuf_first(global_event_mgr, &curr);
	    curr.it_val != end.it_val;
	    EventMgrIterator_next(&curr)) {
	ClientPtr client = ClientBuf_getclient(curr.it_val);
	int i;
	if (!client)
	    continue;
	for (i = 0; i < client->ncon; ++i)
	    RkwSync(client->context_flag[i], NULL);
    }
}

void
DetachTTY()
{
  char    errfile[ERRSIZE];
  int     errfd;

#ifdef DEBUG
  if (!DebugMode)
  {
#endif
    /* 標準エラー出力をエラーファイルに切り替えて、標準入出力をクローズする */

    if(!Syslog) {
      sprintf(errfile,"%s/%s%d%s", ERRDIR, ERRFILE, PortNumberPlus, ERRFILE2);

      if((errfd = open(errfile, O_CREAT | O_RDWR | O_TRUNC, 0644)) < 0) {
	(void)fprintf(stderr, "Warning: %s: %s open failed\n", Name, errfile);
	(void)perror("");
      } else {
	if(dup2( errfd, fileno(stderr)) < 0) {
	  (void)fprintf(stderr, "Warning: %s: %s dup2 failed\n", Name, errfile);
	  (void)perror("");
	  close(fileno(stderr));
	}
      }
      close(fileno(stdin));
      close(fileno(stdout));
      close(errfd);
    }
    /*
     * TTY の切り離し
     */
#if defined(HAVE_SETSID)
    (void)setsid();
#elif defined(__EMX__)
    (void)_setsid();
#elif defined(SETPGRP_VOID)
    /* defined(SYSV) || defined(linux) || defined(__OSF__) */
    setpgrp();
#else
    setpgrp(0, getpid());
#endif

#if defined(TIOCNOTTY) && !defined(HAVE_SETSID)
    {
      int fd = open("/dev/tty", O_RDWR, 0);
      if (fd >= 0) {
	(void)ioctl(fd, TIOCNOTTY, (char *)0);
	(void)close(fd);
      }
    }
#endif

#ifdef DEBUG
  }
#endif

    /*
     * シグナル処理
     */
    mysignal(SIGHUP,   SIG_IGN);
    mysignal(SIGINT,   Reset);
    mysignal(SIGALRM,  SIG_IGN);
    mysignal(SIGPIPE,  SIG_IGN) ;
    mysignal(SIGTERM,  Reset); /* for killserver */

    umask( 002 ) ;
}
