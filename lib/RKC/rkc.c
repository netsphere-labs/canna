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
static char rcs_id[] = "$Id: rkc.c,v 1.12 2003/09/24 15:01:07 aida_s Exp $";
#endif

/*
 * MODIFICATION HISTORY
 *	S000	funahasi@oa2	Fri Oct  2 20:14:13 JST 1992
 *		- debug用関数 RkThrough()のために protocolを追加
 *	S001	funahasi@oa2	Tue Oct 13 15:40:08 JST 1992
 *		- version2.x以前のサーバに接続している時 RkQueryDic()が -1
 *		  を返すよう仕様を変更
 *	S002	funahasi@oa2	Thu Nov  5 13:22:49 JST 1992
 *		- RkQueryDic()の引き数に user名を指定出来るよう仕様を変更
 *		- fixed bug RkBgnBun()で旧サーバに接続している場合，modeから
 *		  コード変換部分だけを取り出す処理を修正
 *	S003	funahasi@oa2	Fri Nov 13 01:19:52 JST 1992
 *		- fixed bug version1.1のサーバに接続出来ない
 *		  rkc_get_yomi()を行っているのが原因
 *		- RkwSetAppName()が無かったので追加
 *		- fixed bug version2.1のサーバに接続出来ない
 *		  rkc_set_app_name()を行っているのが原因
 *	S004	funahasi@oa2	Fri Dec  4 02:04:38 JST 1992
 *		- cannastat, cshosのために rkcの内部関数を呼び出す
 *		  インタフェースを追加
 *	S005	funahasi@oa2	Thu Feb 18 12:18:41 JST 1993
 *		- fixed bug Rk[w]GetKanjiList()が仕様と異なっている．
 */

/* LINTLIBRARY */

#include "canna/sglobal.h"
#include    "rkcw.h"
#include    "canna/RK.h"
#include    "rkc.h"
#include "conf.h"
#include "RKindep/ecfuncs.h"

#include    <sys/types.h>
#include <stdio.h>
#ifndef _WIN32
  #include    <pwd.h>
  #include    <grp.h>
  #include    <unistd.h>
  #define closesocket close
  typedef int SOCKET;
  #define INVALID_SOCKET -1
#else
  #define WIN32_LEAN_AND_MEAN
  #define STRICT
  #define NOMINMAX
  #include <windows.h>
  #include <lmcons.h> // UNLEN
#endif
#include    <signal.h>
#include <assert.h>
#include <string.h> // memmove()

static int min(int a, int b) { return a <= b ? a : b; }

extern int _RkwGetYomi( RkcContext* cx, cannawc* yomi, int maxyomi);
extern size_t CNvW2E(const cannawc* src, int srclen, unsigned char* dest, size_t destlen);
extern int rkc_get_dir_list( RkcContext* cx, char* ddname, int maxddname );


/* CX:	コンテクストテーブル
 *	必要なレコードだけをmallocで作成する。
 *	^^^^^^^^^^^^^^^^^^
 */

static RkcContext *RkcCX[MAX_CX] ;

#define RkcFree free
#define BUSY	1

#define AUTO_YOMI_SIZE	512

static short
rkc_call_flag = 0x00   ; /* RkInitializeが呼ばれてRkFinalizeが呼ばれるまで */
			 /* BUSYビットが立っている			   */

static short ProtocolMinor = 0 ;
static short ProtocolMajor = 0;

static SOCKET ServerFD = INVALID_SOCKET;					/* S004 */

extern struct rkcproto wideproto;
#ifdef USE_EUC_PROTOCOL
extern struct rkcproto eucproto;
static struct rkcproto *RKCP = &eucproto;
#else /* !USE_EUC_PROTOCOL */
static struct rkcproto *RKCP = &wideproto;
#endif /* !USE_EUC_PROTOCOL */

static short PROTOCOL = 0 ;

static char ConnectIrohaServerName[ MAX_HOSTNAME + 1 ];
static char *ServerNameSpecified;
static RkcConfigErrorProc config_error_handler;

/*
 * サポートするプロトコルのリスト
 * この順にプロトコルを使用し，サーバに接続しようとする
 */
static const char* ProtoVerTbl[] = {
    W_VERSION,/* サーバと同じメジャー番号の時は、サーバの
                                    バージョンのプロトコルを使っている。*/
    "2.1",    /* サーバよりメジャー番号が小さい時は、
              小さい方のプロトコルを使う。(ここはマクロにした方がよいね)*/
#ifdef USE_EUC_PROTOCOL
    E_VERSION,	/* ver1.2 */
    "1.0",
#endif
    "",
};

static struct {
    cannawc cbuf[CBUFSIZE];
    char buffer[BUFSIZE];
    cannawc wbuf[CBUFSIZE];
} rkc; /* general buffer。ただし、RkwXXX のなかだけで使うことにしよう。*/

/*
 * クライアント・コンテクストユーティリティ関数
 */


/*
 * 新しいコンテクストを作成する。
 */
static RkcContext* newCC()
{
    RkcContext *cx ;
    int i ;

    for( i = 0; i < MAX_CX; i++) {
	if( !RkcCX[ i ] ) {
	    cx = (RkcContext *)malloc( sizeof( RkcContext ) ) ;
	    if( cx ) {
		cx->client = i ;
		cx->bun =    NULL ;
		cx->Fkouho = NULL ;
		cx->lastyomi = NULL ;
		cx->curbun = cx->maxbun = cx->bgnflag = cx->maxyomi = 0 ;
		RkcCX[ i ] = cx ;
	    }
	    return( cx ) ;
	}
    }
    return NULL;
}

/*
 * 指定された文節から最終文節までの先頭候補または、候補列の領域を解放する
 */
static void freeBUN( RkcContext* cx, int from)
{
    RkcBun *bun ;

    for( ; from < cx->maxbun; from++ ) {
	bun = &cx->bun[ from ] ;
	if( bun->flags == NUMBER_KOUHO ) {
	    /* 第一候補しか入っていない文節は、実際にはmallocしたわけではなく
	     * サーバから通知された各文節の第一候補列の中へのポインタを
	     * 設定しているだけだからフリーしない。
	     */
	    RkcFree( (char *)bun->kanji ) ;
	    bun->kanji = NULL ;
	    bun->curcand = bun->maxcand = 0 ;
	    bun->flags = NOTHING_KOUHO ;
	}
    }
}


/*
 * 指定されたコンテクストを解放する。
 */
static void freeCC( int clientcx )
{
    RkcContext     *cx ;

    if( (0 <= clientcx) && (clientcx < MAX_CX) ) {
	cx = RkcCX[ clientcx ] ;
	if( cx->bun ) {
	    freeBUN( cx, 0 ) ;
	    RkcFree( (char *)cx->bun ) ;
	    cx->bun = (RkcBun *)NULL ;
	}
	RkcFree( (char *)cx->Fkouho ) ;
	cx->Fkouho = NULL ;
	RkcFree( (char *)cx->lastyomi );
	cx->lastyomi = NULL;
	cx->curbun = cx->maxbun = 0 ;
	cx->bgnflag = 0 ;
	RkcFree( (char *)cx ) ;
	cx = (RkcContext *)NULL ;
	RkcCX[ clientcx ] = (RkcContext *)NULL ;
    }
}

/*
 * コンテクスト番号に対応したコンテクストを取得する。
 * (クライアント側)
 */
#define NOCHECK    0
#define CHECK	   1

static RkcContext *
getCC( int clientcx, int type )
{
    RkcContext     *cx = (RkcContext *)NULL ;

    if( (0 <= clientcx) && (clientcx < MAX_CX) ) {
        cx = RkcCX[clientcx];
	if (cx)
	    if( (type == CHECK) && (cx->bgnflag != BUSY) )
		/* 変換中の時,maxbunは最低１である */
		cx = (RkcContext *)NULL ;
    }
    return( cx ) ;
}


typedef struct {
    const char* uname;        /* user name */
    const char* gname;        /* group name */
    //const char* topdir;       /* install dir */
} RkUserInfo;

// lib/canna/commondata.c で定義される `uinfo` とは別の変数!
static RkUserInfo* uinfo = NULL;

// lib/canna/henkan.c, lib/canna/kctrl.c から呼び出される. -> クライアント側
// @return 成功 1, 失敗 0
int RkwSetUserInfo( const char* user, const char* group, const char* topdir )
{
    if ( !user || !group )
        return 0;
    if (!uinfo) {
        uinfo = (RkUserInfo*) malloc(sizeof(RkUserInfo));
        if (!uinfo)
            return 0;
    }
    // 参照するだけ.
    uinfo->uname = user;
    uinfo->gname = group;
    //uinfo->topdir = topdir;

    return 1;
}


// @return ユーザ名. 静的領域を指す. 解放してはならない.
static const char* FindUserName()
{
    if (uinfo)
        return uinfo->uname;

#ifndef _WIN32
    struct passwd *pass = getpwuid(getuid()); // 実ユーザ getuid() でよい.
    if( pass )
        return pass->pw_name;

    // logname コマンドは LOGNAME 環境変数を無視する. 使うべきではない.
    //char* username = NULL;
    return getlogin();
#else
    static char buf[UNLEN + 1];
    DWORD siz = sizeof(buf);
    GetUserName(buf, &siz);

    return buf;
#endif
}


static const char* FindGroupName()
{
    if (uinfo)
        return uinfo->gname;

#ifndef _WIN32
    struct group *gr = getgrgid(getgid()) ;
    if (gr && gr->gr_name)
        return gr->gr_name;

    return NULL;
#else
    // 環境変数 USERDOMAIN が簡単だが...
    static char buf[300];
    DWORD siz = sizeof(buf);
    DWORD sidLen = 0;
    SID_NAME_USE snu;
    LookupAccountName(NULL,           // lpSystemName
                      FindUserName(), // lpAccountName
                      NULL,
                      &sidLen,
                      buf,
                      &siz,
                      &snu);
    fprintf(stderr, "DEBUG: domain name = %s\n", buf);
    return buf;
#endif
}


/**
 * かな漢字変換の初期化: サーバに接続する.
 *
 * @return   0 or -1
 */
int
RkwInitialize( const char* hostname ) /* とりあえずrkcの場合は、引き数を無視する */
{
    int    i;
    long  server ;
    RkcContext *cx ;
    const char* username;
    char *data ;

    if( rkc_call_flag == BUSY )
        return 0;

    rkc_configure();
    if (config_error_handler)
	(*config_error_handler)(RkcErrorBuf_get(&rkc_errors));
    if (ServerNameSpecified) {
        free(ServerNameSpecified);
        ServerNameSpecified = NULL;
    }
    ConnectIrohaServerName[0] = '\0';
    if( hostname && strlen(hostname) > 0 &&
#ifdef __EMX__
	    !_fnisabs( hostname ) &&
#else
	    hostname[0] != '/' &&
#endif
        (ServerNameSpecified = (char*) malloc(strlen(hostname) + 1)) ) {
        strcpy(ServerNameSpecified, hostname);
    }

    if( (ServerFD = rkc_Connect_Iroha_Server(ConnectIrohaServerName)) ==
                INVALID_SOCKET ) { /* S004 */
	errno = EPIPE ;
        goto init_err;
    }

    /* ユーザ名を取得する */
    username = FindUserName() ;
    if( !username ||
        !(data = (char*) malloc( strlen(username) + strlen(W_VERSION)+2 ))) {
        goto init_err;
    }

    /* コンテクストを初期化する */
    for( i=0; i < MAX_CX; i++)
	RkcCX[ i ] = (RkcContext *)NULL ;

    /* コンテクストを作成する */
    if( (cx = newCC()) == (RkcContext *)NULL ) {
        RkcFree(data);
        goto init_err;
    }

    /* 最初はワイドキャラベースのプロトコルを使用する */
    for( i = 0; *ProtoVerTbl[i]; i++ ){
	strcpy( data, ProtoVerTbl[i] );
	strcat( data, ":" );
	strcat( data, username );
	ProtocolMajor = *ProtoVerTbl[i] - '0'; /* Majorが1桁たけ対応 */
        PROTOCOL = (ProtocolMajor > 1);

#ifdef USE_EUC_PROTOCOL
	RKCP = PROTOCOL > 0 ? &wideproto : &eucproto;
#endif

        /* サーバに初期化を要求し、サーバのコンテクストを取得する */
	if ((server = (*RKCP->initialize)( data )) < 0) {
	    /* 既にコンテクストを確保しているので、それを解放する */
	    if( (ServerFD = rkc_Connect_Iroha_Server( ConnectIrohaServerName )) < 0 ) { /* S004 */
		freeCC( cx->client ) ;
		RkcFree(data);
		errno = EPIPE ;
		goto init_err;
	    }
	    continue;
	}
	break;
    }
    RkcFree(data);

    if (!*ProtoVerTbl[i]) {
        freeCC(cx->client);
        errno = EPIPE;
        closesocket(ServerFD);
        goto init_err;
    }

    /* サーバのマイナーバージョンを得る */
    ProtocolMinor =
      (short)((unsigned long)(server & 0xffff0000) >> (unsigned)0x10);

    /* サーバから取得したコンテクスト番号を入れる */
    cx->server = server & 0x0000ffff ;
    rkc_call_flag = BUSY ;

    /* プロトコルバージョンが 3.2 以上だったらグループ名を通知する */
    if (canna_version(ProtocolMajor, ProtocolMinor) > canna_version(3, 1)) {
        const char *gname = FindGroupName();
        if (gname) {
	(*RKCP->notice_group_name)(cx, gname);
      }
    }
    return( cx->client ) ;
 init_err:
    rkc_config_fin();
    return -1;
}


/**
 *  かな漢字変換の終了
 */
void RkwFinalize()
{
    int i ;

    if( rkc_call_flag != BUSY )
		return;

    /* 全コンテクストを解放する
     *	    変換中のコンテクストはどうするのか ?
     */
    for( i = 0; i < MAX_CX; i++ ){
	if( RkcCX[ i ] ) {
	    freeCC( i ) ;
	}
    }

    (*RKCP->finalize)();

    ProtocolMinor = 0 ;
    rkc_call_flag = 0 ;
    ProtocolMajor = 0;
    if (ServerNameSpecified) {
	free(ServerNameSpecified);
	ServerNameSpecified = (char *)0;
    }
    ConnectIrohaServerName[0] = '\0';

    if (uinfo) {
        free( uinfo);
        uinfo = NULL;
    }
    rkc_config_fin();
}

/*
 *  RkwCloseContext ()
 *
 *  Description:
 *  -----------
 *  コンテクストの開放
 *
 *  Input:
 *  -----
 *  cxnum
 *
 *  Returns:
 *  -------
 *  0 or -1
 */
int
RkwCloseContext( int cxnum )
{
    register RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if( !cx || (rkc_call_flag != BUSY) )
	return( -1 ) ;

    if ((*RKCP->close_context)(cx) == -1)
	return( -1 ) ;

    freeCC( cxnum ) ;
    return( 0 );
}


/*
 * KillServer
 *
 * Version 3.3 以前のサーバには送らない 戻り値 OLDSRV = -110
 * Version 3.3 のサーバには送る
 *
 */
int
RkwKillServer()
{
/*
  Protocol Version 3.3 からサポート。それ以前のサーバへは送ってはいけない。
*/

  if (canna_version(ProtocolMajor, ProtocolMinor) > canna_version(3, 2)) {
    return (*RKCP->killserver)();
  }
  return OLDSRV;

/* Protocol Version 3.3 */

}

/*
 *  RkwCreateContext ()
 *
 *  Description:
 *  -----------
 *  新しいコンテクストの作成
 *
 *  Returns:
 *  -------
 *  コンテクスト番号 or -1
 */
int
RkwCreateContext()
{
    int    server ;
    RkcContext *cx ;

    if( rkc_call_flag != BUSY )
	return( -1 ) ;

    /* コンテクストを作成する */
    if( (cx = newCC()) == (RkcContext *)NULL )
	return( -1 ) ;

    if ((server = (*RKCP->create_context)()) == -1) {
	/* 既にコンテクストを確保しているので、それを解放する */
	freeCC( cx->client ) ;
	return( -1 ) ;
    }

    cx->server = server ;
    return( cx->client ) ;
}

/*
 *  RkwCreateContext ()
 *
 *  Description:
 *  -----------
 *  新しいコンテクストの作成
 *
 *  Input:
 *  -----
 *  src_cx
 *
 *  Returns:
 *  -------
 *  コンテクスト番号 or -1
 */
int
RkwDuplicateContext( int src_cx )
{
    register RkcContext *cx_dest, *cx_src = getCC( src_cx, NOCHECK ) ;
    register int	dest_cx ;

    if( !cx_src || (rkc_call_flag != BUSY) )
	return( -1 ) ;

    /* コンテクストを作成する */
    if( (cx_dest = newCC()) == (RkcContext *)NULL )
	return( -1 ) ;

    if ((dest_cx = (*RKCP->duplicate_context)(cx_src)) == -1) {
	/* 既にコンテクストを確保しているので、それを解放する */
	freeCC( cx_dest->client ) ;
	return( -1 ) ;
    }

    cx_dest->server = dest_cx ;
    return( cx_dest->client ) ;
}


/*
 *  RkwGetDicList ()
 *
 *  Description:
 *  -----------
 *  辞書リストに追加できる辞書名の取得
 *
 *  Input:
 *  -----
 *  cxnum
 *
 *  Returns:
 *  -------
 *  辞書名の個数 or -1
 */
int
RkwGetDicList( int cxnum, char* dicnames, int max)
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if( !cx )
	return( -1 ) ;

    if( max <= 0 )
	return( 0 ) ;
    else if( !dicnames ) {
      return (*RKCP->dictionary_list)(cx, rkc.buffer, BUFSIZE);
    }
    return (*RKCP->dictionary_list)(cx, dicnames, max);
}


/* 単語登録 */
static int
_RkwDefineDic( int cxnum, const char* dicname, cannawc* wordrec )
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if( !cx )
	return( -1 ) ;

    return (*RKCP->define_dic)(cx, dicname, wordrec);
}


/**
 *  単語登録
 *
 *  Input:
 *  -----
 *  cxnum
 *
 *  Returns:
 *  -------
 *  コンテクスト番号 or -1
 */
int
RkwDefineDic(int cxnum, const char* dicname, const cannawc* wordrec)
{
    if ( !dicname || !wordrec )
        return -1;

    WStrncpy(rkc.cbuf, wordrec, CBUFSIZE);
    return _RkwDefineDic(cxnum, dicname, rkc.cbuf);
}


/* 単語削除 */
static int
_RkwDeleteDic( int cxnum, char* dicname, cannawc* wordrec )
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if( !cx )
	return( -1 ) ;

    return (*RKCP->delete_dic)(cx, dicname, wordrec);
}

int
RkwDeleteDic(int cxnum, char* dicname, cannawc* wordrec)
{
    if( !dicname || !wordrec )
        return -1;

    WStrncpy(rkc.cbuf, wordrec, CBUFSIZE);
    return _RkwDeleteDic(cxnum, dicname, rkc.cbuf);
}

int
RkwMountDic( int cxnum, const char* dicname, int mode)
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if( !dicname || !cx )
	return( -1 ) ;

    return (*RKCP->mount_dictionary)(cx, dicname, mode);
}

int
RkwRemountDic(int cxnum, char* dicname, int where)
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if( !dicname || !cx )
	return( -1 ) ;

    return (*RKCP->remount_dictionary)(cx, dicname, where);
}

int
RkwUnmountDic(int cxnum, char* dicname)
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if( !dicname || !cx )
	return( -1 ) ;

    return (*RKCP->umount_dictionary)(cx, dicname);
}

int
RkwGetMountList(int cxnum, char* dicnames_return, int max)
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if( !cx )
	return( -1 ) ;

    if( !dicnames_return ) {
        return (*RKCP->mount_list)(cx, rkc.buffer, BUFSIZE);
    } else if( max <= 0 )
	return( 0 ) ;

    return (*RKCP->mount_list)(cx, dicnames_return, max);
}


/* サーチパスを設定 */
int
RkwSetDicPath( int cxnum, const char* path )
{
    return( 0 ) ;
}


/**
 *	カレント文節から最終文節までの先頭候補を求め、格納する
 */
static void
StoreFirstKouho( RkcContext* cx, int bun_max )
{
    /* ここは、よく考えないと構造まで変えた意味が無くなるし
     *	 後でえらい目に遭うかも知れない
     */
    int		i ;
    RkcBun		*bun ;
    cannawc* kouhobuf ;

    /* カレント文節から最終文節までの候補を解放する */
    freeBUN( cx, cx->curbun ) ;

    /* ここには、rkc_*(rkcConvert.c)で第一候補列が格納されている */
    kouhobuf = cx->Fkouho ;

    /* ゼロ文節から最終文節までの第一候補のポインタを設定する */
    for( i = 0; i < bun_max; i++ ) {
	bun = &cx->bun[ i ] ;
	/* カレント文節までの文節で候補一覧を既に取得している文節は、
	 * ポインタの再設定はしない。
	 */
	if( bun->flags != NUMBER_KOUHO ) {
	    bun->kanji = kouhobuf ;
	    bun->curcand = 0 ;		/*  文節0文節1文節2文節3文節4@@ */
	    bun->maxcand = 1 ;		/*  ↑	 ↑   ↑   ↑	↑	*/
	    bun->flags = FIRST_KOUHO ;	/*	bun->kaji		*/
	}
        kouhobuf += ushortstrlen(kouhobuf) + 1 ;
    }
    cx->maxbun = bun_max ;
}


/**
 *    連文節変換開始
 * @param yomi NULL ありうる.
 */
static int
_RkwBgnBun(int cxnum, cannawc* yomi, int maxyomi, int mode)
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;
    int nbun, mask;					/* S002 */

    if( (maxyomi <= 0) || !cx )
        return -1;

    /* RkBgnBunだけは、BUSYフラグが立っていると入ってはいけない */
    if( cx->bgnflag == BUSY )
        return -1;

    /* 旧サーバに接続している場合，modeからコード変換部分だけを取り出す */
    if( ProtocolMajor < 3 ){
        int code;

	for( code = mode, mask = 0L; code ; code >>= RK_XFERBITS ){
	    if( (code & RK_XFERMASK) == RK_CTRLHENKAN ){
		break;
	    }
	    mask = (mask << RK_XFERBITS) | RK_XFERMASK;
	}
	mode &= mask;					/* S002 */
    }

    /* maxyomiの文節数だけアロケートしておく */
    /* これ以上の文節数は存在しないはず */

    cx->curbun = cx->maxbun = 0 ;
    if ( yomi != NULL ) {
	if( !(cx->bun = (RkcBun *)calloc( maxyomi, sizeof(RkcBun) )) )
	    return( -1 ) ;
	cx->lastyomi = NULL;
	nbun = (*RKCP->convert)(cx, yomi, maxyomi, mode);
	if( nbun <= 0 ) {
	    RkcFree( cx->bun ) ;
	    cx->bun = NULL ;
	    return( -1 ) ;
	}
	StoreFirstKouho( cx, nbun ) ;
    } else {
	if( !(cx->bun = (RkcBun *)calloc( AUTO_YOMI_SIZE, sizeof(RkcBun) )) )
	    return( -1 ) ;
	if( !(cx->lastyomi = (cannawc*) malloc( sizeof(cannawc) * CBUFSIZE )) ) {
	    RkcFree( cx->bun );
	    cx->bun = NULL ;
	    return( -1 ) ;
	}

	nbun = (*RKCP->autoconv)(cx, maxyomi, mode);
	if( nbun < 0 ) {
	    RkcFree( cx->bun ) ;
	    cx->bun = NULL ;
	    RkcFree( cx->lastyomi );
	    cx->lastyomi = NULL;
	    return( -1 ) ;
	}
	*(cx->lastyomi) = (cannawc) 0;
    }
    cx->bgnflag = BUSY ;
    return nbun;
}


// @param maxyomi   yomi の長さ. ナルを含まない.
int
RkwBgnBun(int cxnum, const cannawc* yomi, int maxyomi, int mode)
{
    int len;

    if (yomi) {
        len = ushortstrncpy(rkc.cbuf, yomi, min(maxyomi + 1, CBUFSIZE));
        return _RkwBgnBun(cxnum, rkc.cbuf, len, mode);
    }
    else {  /* 自動変換開始 */
        return _RkwBgnBun(cxnum, NULL, maxyomi, mode);
    }
}

int
RkwEndBun( int cxnum, int mode )
{
    RkcContext  *cx = getCC( cxnum, CHECK ) ;
    int ret ;

    if( cx ) {
	/*
	 * rkc_convert_endで学習すべき候補をサーバに知らせる
	 */
	if ((ret = (*RKCP->convert_end)(cx, mode )) >= 0) {
	    freeBUN( cx, 0 ) ;
	    RkcFree( cx->bun ) ;
	    RkcFree( cx->Fkouho ) ;
	    cx->bun = NULL ;
	    cx->Fkouho = NULL ;
	    cx->curbun = cx->maxbun = 0 ;
	    cx->bgnflag = 0 ;

	    RkcFree( cx->lastyomi );
	    cx->lastyomi = NULL;
	    cx->maxyomi = 0;
	}
	return( ret ) ;
    }

    return( 0 ) ;
}


/**
 *	必要に応じて全候補を読み出す
 */
static int
LoadKouho( RkcContext* cx )
{
    RkcBun	*bun = &cx->bun[ cx->curbun ] ;
    int 		ret ;

    if( bun->flags == FIRST_KOUHO ) {
	/*	候補を全て読み出す。
	 *	読み出しに失敗したら、先頭候補しかないふりをする
	 */
	if ((ret = (*RKCP->get_kanji_list)(cx)) >= 0) {
	    /* 読みだし成功 */
	    bun->curcand = 0 ;
	    bun->maxcand = ret ;
	} else if( errno == EPIPE )
	    return( -1 ) ;

	bun->flags = NUMBER_KOUHO ;
    }
    return( 0 ) ;
}

int
RkwXfer( int cxnum, int knum)
{
    RkcContext  *cx = getCC( cxnum, CHECK ) ;
    RkcBun	 *bun ;

    if( cx ) {
	bun = &cx->bun[cx->curbun];
	if( LoadKouho( cx ) < 0 )
	    return( -1 ) ;
	if ( 0 <= knum && knum < bun->maxcand )
	    bun->curcand = knum;
	return( bun->curcand );
    }
    return( 0 );
}

int
RkwNfer( int cxnum)
{
    RkcContext  *cx = getCC( cxnum, CHECK ) ;
    RkcBun	 *bun ;

    if( cx ) {
	bun = &cx->bun[ cx->curbun ];
	if( LoadKouho( cx ) < 0 )
	    return( -1 ) ;
	bun->curcand = bun->maxcand - 1; /* 読みは、最後にある(0オリジン) */
	return( bun->curcand ) ;
    }
    return( 0 );
}

int
RkwNext(int cxnum)
{
    RkcContext  *cx = getCC( cxnum, CHECK ) ;
    RkcBun	 *bun ;

    if( cx ) {
	bun = &cx->bun[ cx->curbun ];
	if( LoadKouho( cx ) < 0 )
	    return( -1 ) ;
	if ( ++bun->curcand > bun->maxcand-1 )
	    bun->curcand = 0;
	return( bun->curcand ) ;
    }
    return( 0 ) ;
}

int
RkwPrev(int cxnum)
{
    register RkcContext  *cx = getCC( cxnum, CHECK ) ;
    register RkcBun	 *bun ;

    if( cx ) {
	bun = &cx->bun[ cx->curbun ];
	if( LoadKouho( cx ) < 0 )
	    return( -1 ) ;
	if ( --bun->curcand < 0 )
	    bun->curcand = bun->maxcand-1 ; /* ０オリジン */
	return( bun->curcand ) ;
    }
    return( 0 );
}


static cannawc*
SeekKouho( RkcBun* bun, int to )
{
    int    i ;
    cannawc* src_yomi ;

    src_yomi = bun->kanji ;
    for( i = 0; i < to; i++ )
        src_yomi += ushortstrlen(src_yomi) + 1 ;

    return src_yomi;
}


static int
_RkwGetKanji( int cxnum, cannawc* kanji, int maxkanji )
{
    RkcContext		    *cx = getCC( cxnum, CHECK ) ;
    RkcBun		    *bun ;
    cannawc* src_kouho ;

    if( !cx )
        return -1;

    bun = &cx->bun[ cx->curbun ] ;
			     /* 読みしかない場合は読みを返す */
	src_kouho = SeekKouho( bun, (bun->maxcand ? bun->curcand : 0) ) ;
	if( ushortstrlen(src_kouho ) > maxkanji )
	    return( 0 ) ;
	ushortstrcpy( kanji, src_kouho );

    return ushortstrlen(src_kouho);
}


// ただのコピー.
// @param dlen dst の大きさ.
// @return ナル終端を含まない要素数.
int
ushort2wchar(const cannawc* src, int slen, cannawc* dst, int dlen)
{
    assert( src );
    assert( dst );
    assert( dlen > 0 );

    int len = min(slen, dlen - 1);
    memmove(dst, src, len * sizeof(cannawc));
    dst[len] = 0;

    return len;
}

#define wchar2ushort ushort2wchar


int
RkwGetKanji(int cxnum, cannawc* kanji, int maxkanji)
{
    int len;

    len = _RkwGetKanji(cxnum, rkc.cbuf, CBUFSIZE);
    if (len < 0) {
	return( len );
    }
    else {
	if( !kanji ) {
	  return ushort2wchar(rkc.cbuf, len, rkc.wbuf, CBUFSIZE);
	}
	else if (maxkanji <= 0) {
	    return( 0 );
	}
	else {
	  return ushort2wchar(rkc.cbuf, len, kanji, maxkanji);
	}
    }
}


// @param kouho  maybe NULL.
static int
_RkwGetKanjiList( int cxnum, cannawc* kouho, int max)
{
    RkcContext	*cx = getCC( cxnum, CHECK ) ;
    RkcBun	*bun ;
    cannawc *dest_kouho, *src_kouho ;
    int i, len ;
    int total ;

    if( cx ) {
	bun = &cx->bun[ cx->curbun ];
	if( LoadKouho( cx ) < 0 )
	    return( -1 ) ;
	if( !bun->kanji )
	    return( 0 ) ;
	if( !kouho )
	    return( bun->maxcand ? bun->maxcand : 1 ) ;
	/* 候補をコピーする */
	src_kouho = bun->kanji ;
	dest_kouho = kouho ;
	for( total = ushortstrlen( src_kouho ) + 1, i = 0;
	    (i < bun->maxcand) && (total < max) ; i++, total += len ) {
	    len = ushortstrcpy( dest_kouho, src_kouho ) + 1 ;
	    src_kouho += len ;
	    dest_kouho += len ;
	}
        *(dest_kouho++) = (cannawc) 0 ;
        *(dest_kouho) = (cannawc) 0 ;
        return i ;
    }
    return -1;
}


int
RkwGetKanjiList( int cxnum, cannawc* kanjis, int maxkanjis )
{
    int nkanji, len, i, j = 0, k = 0;
    int retval;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
    cannawc cbuf[CBIGBUFSIZE];
#else
    cannawc* cbuf = GlobalAlloc(GMEM_FIXED, sizeof(cannawc) * CBIGBUFSIZE);
    if (!cbuf)
        return 0;
#endif

  if( !kanjis ) {
    retval = _RkwGetKanjiList(cxnum, NULL, 0);
  }
  else if (maxkanjis <= 0) {
    retval = 0;
  }
  else {
  nkanji = _RkwGetKanjiList(cxnum, cbuf, CBIGBUFSIZE);

  for (i = 0 ; i < nkanji ; i++) {
    len = ushortstrlen(cbuf + j);
    if (k + len > maxkanjis - 2)				/* S005 */
      break;							/* S005 */
    k += ushort2wchar(cbuf + j, len, kanjis + k, maxkanjis);	/* S005 */
    kanjis[k++] = (cannawc)0;
    j += len + 1;
  }
  kanjis[k] = (cannawc)0;
  retval = i;
  }
#ifdef USE_MALLOC_FOR_BIG_ARRAY
    GlobalFree( cbuf);
#endif
    return retval;
}

int
RkwGoTo( int cxnum, int bnum)
{
    RkcContext  *cx = getCC( cxnum, CHECK ) ;

    if( cx ){
	if ( 0 <= bnum && bnum < cx->maxbun )
	     cx->curbun = bnum;
	return(cx->curbun);
    }
    return( 0 );
}

int
RkwLeft( int cxnum)
{
    RkcContext  *cx = getCC( cxnum, CHECK ) ;

    if( cx ){
	if ( --cx->curbun < 0 )
	     cx->curbun = cx->maxbun-1 ;
	return( cx->curbun );
    }
    return( 0 );
}

int
RkwRight( int cxnum)
{
    RkcContext  *cx = getCC( cxnum, CHECK ) ;

    if( cx ){
	if ( ++cx->curbun > cx->maxbun-1 )
	     cx->curbun = 0;
	return( cx->curbun );
    }
    return( 0 );
}

#define ENLARGE     -1
#define SHORTEN     -2
#define MIN_YOMI     1


static int
RKReSize( int cxnum, int len )
{
    RkcContext  *cx = getCC( cxnum, CHECK ) ;
    int ret;		/* 総文節数 */

    if( cx ) {
	/* 文節長が変更できるかどうかチェックする */
	register RkcBun *bun = &cx->bun[ cx->curbun ] ;

	/* カレント文節が候補列を既に読み込んでいる場合の処理 */
	if( bun->flags == NUMBER_KOUHO ) {
	     /* 文節縮めでカレント読みの長さがMIN_YOMIの場合は何もせずに */
	     /* カレント文節数を返す */
            int yomilen = ushortstrlen(SeekKouho( bun,
				       (bun->maxcand ? (bun->maxcand-1) : 0)));

	    if( (len == SHORTEN) || (len == MIN_YOMI) ) {
		if( yomilen == MIN_YOMI )
		    return( cx->maxbun ) ;
	    } else {
		int curbun_save = cx->curbun ;
		int yomi_zan ;

		for( yomi_zan = 0; cx->curbun < cx->maxbun; cx->curbun++ ) {
		    int ylen, retval = 0;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
                    cannawc tmp_yomi[CBUFSIZE];
#else
                    cannawc* tmp_yomi = GlobalAlloc(GMEM_FIXED,
                                                    sizeof(cannawc) * CBUFSIZE);
		    if (!tmp_yomi) {
		      return -1;
		    }
#endif
		    ylen = _RkwGetYomi(cx, tmp_yomi, CBUFSIZE);
		    if (ylen < 0) {
		      retval = -1;
		    }
		    else {
		      yomi_zan += ylen ;
		    }
#ifdef USE_MALLOC_FOR_BIG_ARRAY
		    GlobalFree((HGLOBAL)tmp_yomi);
#endif
		    if (retval < 0) {
		      return retval;
		    }
		}
		cx->curbun = curbun_save ;
		yomi_zan += cx->maxyomi;
		if( ((len == ENLARGE) && (yomilen + 1 > yomi_zan))
						      || (yomi_zan < len) )
		    return( cx->maxbun ) ;
	    }
	}
	/* 〇文節から最終文節まで格納される */
	if ((ret = (*RKCP->resize)(cx, len)) <= 0) {
	    return( -1 ) ;
	}
	StoreFirstKouho( cx, ret ) ;
        if ( cx->lastyomi != NULL ){
	    if ((len = (*RKCP->get_last_yomi)(cx, cx->lastyomi, CBUFSIZE))
		< 0) return -1;
	    cx->maxyomi = len;
	}
	return( ret ) ;
    }
    return( 0 ) ;
}

int
RkwResize( int cxnum, int len )
{
    if( len <= 0 ) {
	register RkcContext  *cx = getCC( cxnum, CHECK ) ;

	if( cx )
	    return( cx->maxbun ) ;
	else
	    return( 0 ) ;
    }

    return( RKReSize( cxnum, len ) ) ;
}


/* 文節伸ばし */
int
RkwEnlarge( int cxnum )
{
    return( RKReSize( cxnum, ENLARGE  ) ) ;
}


/* 文節縮め */
int
RkwShorten( int cxnum)
{
    return( RKReSize( cxnum, SHORTEN ) ) ;
}


static int
_RkwStoreYomi( int cxnum, cannawc* yomi, int max)
{
    RkcContext  *cx = getCC( cxnum, CHECK ) ;
    int ret, len ;

    ret = 0 ;
    if ( !cx )
        return 0;

    if ((ret =  (*RKCP->store_yomi)(cx, yomi, max)) < 0) {
	    return( -1 ) ;
	}
	StoreFirstKouho( cx, ret ) ;
	if (!max && cx->curbun && cx->curbun == cx->maxbun)
	  cx->curbun--;
        if ( cx->lastyomi != NULL ){
	    if ((len = (*RKCP->get_last_yomi)(cx, cx->lastyomi, CBUFSIZE))
		< 0 ) return -1;
	    cx->maxyomi = len;
	}
}


int
RkwStoreYomi(int cxnum, const cannawc* yomi, int maxyomi)
{
    int len;

    if (yomi && maxyomi >= 0) {
        WStrncpy(rkc.cbuf, yomi, min(maxyomi + 1, CBUFSIZE));
        len = WStrlen(rkc.cbuf);
    }
    else {
        rkc.cbuf[0] = 0;
        len = 0;
    }
    return _RkwStoreYomi(cxnum, rkc.cbuf, len);
}


/* S003 */
int
_RkwGetYomi( RkcContext* cx, cannawc* yomi, int maxyomi)
{
    RkcBun	*bun ;
    cannawc* src_yomi;
    int len, retval = -1;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
    cannawc tmp_yomi[CBUFSIZE];
#else
    cannawc* tmp_yomi = GlobalAlloc(GMEM_FIXED, sizeof(cannawc) * CBUFSIZE);
    if (!tmp_yomi)
        return -1;
#endif

    if( cx ){
	bun = &cx->bun[ cx->curbun ] ;
	if( !PROTOCOL && (ProtocolMinor == 0) ) {
	    /* Ver 1.0 では，取りあえず候補一覧を取ってくる */
	    if( LoadKouho( cx ) < 0 )
	      retval = -1;
	}
	if( bun->flags == NUMBER_KOUHO ) {
	    src_yomi = SeekKouho( bun, (bun->maxcand ? (bun->maxcand-1) : 0) );
	} else {
	    if ((*RKCP->get_yomi)(cx, tmp_yomi) < 0)
		retval = -1;
	    src_yomi = tmp_yomi;
	}

	if( (len = ushortstrlen(src_yomi )) > maxyomi )
	    retval = 0;

	memmove( yomi, src_yomi, (len + 1) * sizeof(cannawc) );
	retval = len;
    }
#ifdef USE_MALLOC_FOR_BIG_ARRAY
    GlobalFree((HGLOBAL)tmp_yomi);
#endif
    return retval;
}

int
RkwGetYomi( int cxnum, cannawc* yomi, int maxyomi)
{
    int len;

    if ((len = _RkwGetYomi(getCC( cxnum, CHECK ), rkc.cbuf, CBUFSIZE)) < 0) {
        return( len );
    }
    else {
    if( !yomi ) {
      return ushort2wchar(rkc.cbuf, len, rkc.wbuf, CBUFSIZE);
    }
    else if( maxyomi <= 0 )
      return( 0 );

    return ushort2wchar(rkc.cbuf, len, yomi, maxyomi);
  }
}

int
RkwGetLex(int cxnum, RkLex* lex, int maxlex)
{
    RkcContext  *cx = getCC( cxnum, CHECK ) ;
    int ret = -1 ;

    if( cx ){
      if (lex == (RkLex *)NULL) {
#ifndef USE_MALLOC_FOR_BIG_ARRAY
	RkLex buf[1024];
#else
	RkLex *buf = (RkLex *)GlobalAlloc(GMEM_FIXED, sizeof(RkLex) * 1024);
	if (!buf) {
	  return 0;
	}
#endif
	ret = (*RKCP->get_lex)(cx, 1024, buf);
#ifdef USE_MALLOC_FOR_BIG_ARRAY
	GlobalFree((HGLOBAL)buf);
#endif
      }
      else if( maxlex <= 0 )
	return( 0 );

      ret = (*RKCP->get_lex)(cx, maxlex, lex);
    }
    return( ret ) ;
}

int
RkwGetStat(int cxnum, RkStat* stat)
{
    RkcContext  *cx = getCC( cxnum, CHECK ) ;
    int ret ;

    if( cx && stat ){
	ret = (*RKCP->get_stat)(cx, stat);
	if( !PROTOCOL && (ProtocolMinor == 0) ) {
	    int p[ 7 ] ;
	    register int tmp1, tmp2 ;
	    int i ;

	    memmove( p, stat, sizeof( RkStat ) ) ;
	    tmp1 = p[ 5 ];
	    tmp2 = p[ 6 ] ;
	    for( i = 4; i > 1; i-- )
		p[ i + 2 ] = p[ i ] ;
	    p[ 2 ] = tmp1 ;
	    p[ 3 ] = tmp2 ;
	    memmove( stat, p, sizeof( RkStat ) ) ;
	}
    } else {
	ret = -1 ;
    }

    return( ret ) ;
}

char *
RkwGetServerName()
{
  if (ConnectIrohaServerName[0]) {
    return( ConnectIrohaServerName ) ;
  }
  else {
    return ServerNameSpecified;
  }
}

int
RkwGetProtocolVersion( int* majorp, int* minorp)
{
    *majorp = ProtocolMajor;
    *minorp = ProtocolMinor;
    return 0;
}

exp(int)
RkwGetServerVersion( int* majorp, int* minorp)
{
    if( !PROTOCOL )
	return( RkwGetProtocolVersion(majorp, minorp) );

    return( rkcw_get_server_info(majorp, minorp) );
}

							/* begin:S004 */
SOCKET RkcGetServerFD()
{
    return ServerFD ;
}

SOCKET G070_RkcGetServerFD()
{
    return ServerFD ;
}


SOCKET
RkcConnectIrohaServer( const char* servername )
{
    /* XXX:
     * RkcDisconnectIrohaServerに相当するインターフェースが無いので、
     * これはメモリリークを引き起こす。今のところこのAPIは古いcannastatが
     * ver 1.xのサーバと通信する場合にだけ使うので、この問題は無視する
     * ことにする。
     */
    rkc_configure();
    return rkc_Connect_Iroha_Server(servername) ;
}
							/* end:S004 */

SOCKET
G069_RkcConnectIrohaServer( const char* servername )
{
    return RkcConnectIrohaServer(servername);
}

void
RkcListenConfigErrors( RkcConfigErrorProc handler )
{
    config_error_handler = handler;
}


#ifdef EXTENSION
static
int CheckRemoteToolProtoVersion(int mode)
{
  if (!PROTOCOL && ProtocolMinor < 2) /* protocol version 1.2 */
    return -1;
  else if (canna_version(ProtocolMajor, ProtocolMinor) < canna_version(3, 1) &&
	   (mode & (RK_GRP_DIC | RK_SYS_DIC | RK_GRP_DIR | RK_SYS_DIR))) {
    return -1;
  }
  return 0;
}


// @return If failed, -1.
int
RkwListDic( int cxnum, const char* dirname, char* dicnames_return, int size )
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;
    if (!cx)
        return -1;

    if (CheckRemoteToolProtoVersion(0))
        return ACCES;

    if( !dirname ) {
        if( !(dirname = FindUserName()) )
            return -1 ;
    }

    if( !dicnames_return ) {
#ifndef USE_MALLOC_FOR_BIG_ARRAY
	char buffer[ MAXDATA ] ;

	return (*RKCP->list_dictionary)(cx, dirname, buffer, MAXDATA);
#else
	char *buffer = GlobalAlloc(GMEM_FIXED, MAXDATA);
	if (buffer) {
	  int retval =
	    (*RKCP->list_dictionary)(cx, dirname, buffer, MAXDATA);
	  GlobalFree((HGLOBAL)buffer);
	  return retval;
	}
	else {
	  return 0;
	}
#endif
    }
    else if( size <= 0 )
	return( 0 ) ;

    return (*RKCP->list_dictionary)(cx, dirname, dicnames_return, size);
}

int
RkwCreateDic( int cxnum, char* dicname, int mode )
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if (!cx || !dicname) {
      return -1;
    }
    if (CheckRemoteToolProtoVersion(mode)) {
      return ACCES;
    }

    return (*RKCP->create_dictionary)(cx, (char *)dicname, mode);
}

exp(int)
RkwRemoveDic( int cxnum, char* dicname, int mode )
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if (!cx || !dicname) {
      return -1;
    }
    if (CheckRemoteToolProtoVersion(mode)) {
      return ACCES;
    }

    return (*RKCP->remove_dictionary)(cx, dicname, mode);
}

exp(int)
RkwRenameDic( int cxnum, char* dicname, char* newdicname, int mode )
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if (!cx || !dicname || !newdicname) {
      return -1;
    }
    if (CheckRemoteToolProtoVersion(mode)) {
      return ACCES;
    }

    return (*RKCP->rename_dictionary)(cx, dicname, newdicname, mode);
}


exp(int)
RkwCopyDic( int cxnum, char* dirname, char* dicname, char* newdicname,
            int mode )
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if (!cx || !dirname || !dicname) {
      return -1;
    }
/*
  Protocol Version 3.2 からサポート。それ以前のサーバへは送ってはいけない。
*/

  if (canna_version(ProtocolMajor, ProtocolMinor) > canna_version(3, 1)) {
    return (*RKCP->copy_dictionary)(cx, dirname, dicname, newdicname, mode);
  }
  return -1;

/* Protocol Version 3.2 */

}


static
int _RkwGetWordTextDic( int cxnum, const char* dirname, const char* dicname, cannawc* info,
                    int infolen )
{
    RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if (!cx || !dirname || !dicname) {
      return -1;
    }

    if (CheckRemoteToolProtoVersion(0)) {
      return ACCES;
    }

    return (*RKCP->get_text_dictionary)
                (cx, dirname, dicname, info, infolen);
}


// info [長さ=infolen] に格納する.
exp(int)
RkwGetWordTextDic( int cxnum, const char* dirname, const char* dicname,
                   cannawc* info,
                   int infolen)
{
    int len;
    if ( !info )
        return -1;

    len = _RkwGetWordTextDic(cxnum, dirname, dicname, rkc.cbuf, CBUFSIZE);
    if (len < 0)
        return len;

    //if( !info ) {
    //  return ushort2wchar(rkc.cbuf, len, rkc.buffer, BUFSIZE);
    //}
    if ( infolen <= 0 )
      return 0;

    return ushort2wchar(rkc.cbuf, len, info, infolen);
}

#else
RkwListDic(){}
RkwCreateDic(){}
RkwRemoveDic(){}
RkwRenameDic(){}
RkwGetWordTextDic(){}
#endif /* EXTENSION */

/* 逐次自動変換機能関数				*/
/*						*/
/* 逐次自動変換機能導入によって追加される関数	*/

static int
_RkwSubstYomi( int cxnum, int ys, int ye, cannawc* yomi, int nyomi )
{
    RkcContext *cx = getCC( cxnum, CHECK );
    int len, curbun, nbun = -1, pbun, retval = -1;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
    cannawc cbuf[CBUFSIZE];
#else
    cannawc* cbuf = GlobalAlloc(GMEM_FIXED, sizeof(cannawc) * CBUFSIZE);
    if (!cbuf)
        return -1;
#endif

    if( cx ){
	len = cx->maxyomi;
	if( ys < 0 || ye < 0 || ye < ys || len < ys || len < ye ) {
	  retval = -1;
	  goto done;
	}

        nyomi = min( ushortstrlen( yomi ), nyomi);
	curbun = cx->curbun;
	cx->curbun = 0;
	if ((nbun = (*RKCP->subst_yomi)(cx, cx->maxbun, ys, ye, yomi, nyomi))
	    < 0) {
	    cx->curbun = curbun;
	    retval = -1;
	    goto done;
	}

	pbun = cx->maxbun;
	cx->maxbun = 0; /* StoreFirstKouho であんまりものを捨てないように */
	StoreFirstKouho(cx, nbun);

	if (nbun != pbun) {
	  len = (*RKCP->get_last_yomi)(cx, cx->lastyomi, CBUFSIZE);
	  if (len < 0) {
	    retval = -1;
	    goto done;
	  }
	} else {
	    len = ys;
	    ushortstrncpy( cbuf, &(cx->lastyomi[ye]), (cx->maxyomi - ye) );
	    len += ushortstrcpy( &(cx->lastyomi[ys]), yomi );
	    len += ushortstrcpy( &(cx->lastyomi[ys + nyomi]), cbuf );
	}
	cx->maxyomi = len;
    }
    retval = nbun;
  done:
#ifdef USE_MALLOC_FOR_BIG_ARRAY
    GlobalFree( cbuf);
#endif
    return retval;
}


int
RkwSubstYomi( int cxnum, int ys, int ye, cannawc* yomi, int nyomi )
{
    RkcContext *cx = getCC( cxnum, CHECK );
    int len;

    if( !cx )
        return -1;

    len = wchar2ushort(yomi, nyomi, rkc.cbuf, CBUFSIZE);
    return _RkwSubstYomi(cxnum, ys, ye, rkc.cbuf, len);
}


int
RkwFlushYomi( int cxnum )
{
    RkcContext *cx = getCC( cxnum, CHECK );
    int curbun, nbun = -1;

    if( cx ){
	curbun = cx->curbun;
	cx->curbun = 0;
	if ((nbun = (*RKCP->flush_yomi)(cx)) < 0) {
	    cx->curbun = curbun;
	    return( -1 ) ;
	}

	if( nbun != cx->maxbun ) {
	    cx->curbun = cx->maxbun;
	    StoreFirstKouho( cx, nbun );
	    cx->curbun = curbun;
	}

	*(cx->lastyomi) = (Ushort)0;
	cx->maxyomi = 0;
    }
    return( nbun ) ;
}


static int
_RkwGetLastYomi( int cxnum, cannawc* yomi, int maxyomi )
{
    RkcContext *cx = getCC( cxnum, CHECK );
    int	len = -1;

    if( !cx )
        return -1;

    if( !maxyomi || cx->maxyomi > maxyomi )
        return( 0 );
    len = ushortstrncpy( yomi, cx->lastyomi, cx->maxyomi );
    return len;
}


int
RkwGetLastYomi( int cxnum, cannawc* yomi, int maxyomi )
{
    int len;

    len = _RkwGetLastYomi(cxnum, rkc.cbuf, CBUFSIZE);
    if (len < 0)
        return -1;

    if( !yomi ) {
      return ushort2wchar(rkc.cbuf, len, rkc.wbuf, CBUFSIZE);
    }
    else if( maxyomi <= 0 )
      return 0;

    return ushort2wchar(rkc.cbuf, len, yomi, maxyomi);
}


/*
 * 先頭文節から指定された文節までの先頭候補または、候補列の領域を解放する
 */
static void
removeBUN( RkcContext* cx, int to )
{
    RkcBun *bun ;
    int i;

    for( i = 0; i < to; i++ ) {
	bun = &cx->bun[ i ] ;
	if( bun->flags == NUMBER_KOUHO ) {
	    /* 第一候補しか入っていない文節は、実際にはmallocしたわけではなく
	     * サーバから通知された各文節の第一候補列の中へのポインタを
	     * 設定しているだけだからフリーしない。
	     */
	    RkcFree( (char *)bun->kanji ) ;
            bun->kanji = NULL ;
	    bun->curcand = bun->maxcand = 0 ;
	    bun->flags = NOTHING_KOUHO ;
	}
    }
}

int
RkwRemoveBun( int cx_num, int mode )
{
    RkcContext  *cx = getCC( cx_num, CHECK );
    int cnt, i;
    int ret;

    if( cx ) {
	/*
	 * rkcw_remove_bun で学習すべき候補をサーバに知らせる
	 */
	if ((ret = (*RKCP->remove_bun)(cx, mode)) < 0)
	    return( -1 );

	removeBUN( cx, cx->curbun + 1 );
	for( cnt = 0, i = cx->curbun + 1; i < cx->maxbun; cnt++, i++ ) {
	    cx->bun[cnt].kanji = cx->bun[i].kanji;
	    cx->bun[cnt].curcand = cx->bun[i].curcand;
	    cx->bun[cnt].maxcand = cx->bun[i].maxcand;
	    cx->bun[cnt].flags = cx->bun[i].flags;
            cx->bun[i].kanji = NULL ;
	    cx->bun[i].curcand = cx->bun[i].maxcand = 0 ;
	    cx->bun[i].flags = NOTHING_KOUHO ;
	}
	cx->curbun = cx->maxbun = 0;
	StoreFirstKouho( cx, ret ) ;
	return( ret ) ;
    }

    return( 0 ) ;
}

static int
_RkwGetSimpleKanji( int cxnum, char* dicname, cannawc* yomi, int maxyomi,
                    cannawc* kanjis, int maxkanjis,
                    cannawc* hinshis, int maxhinshis)
{
    RkcContext *cx = getCC( cxnum, CHECK ) ;

    if( cx ){
	return (*RKCP->get_simple_kanji)
	  (cx, (char *)dicname, yomi, maxyomi, kanjis,
	   maxkanjis, hinshis, maxhinshis);
    }
    return( -1 ) ;
}

int
RkwGetSimpleKanji( int cxnum, char* dicname, cannawc* yomi, int maxyomi,
                   cannawc* kanjis, int maxkanjis,
                   unsigned char* hinshis, int maxhinshis )
{
    cannawc cbuf[CBUFSIZE], cbuf2[CBIGBUFSIZE], cbuf3[CBIGBUFSIZE];
    int nkanji, len, i, j = 0, k = 0, l = 0, m = 0;

    if( !dicname || !yomi || maxyomi <= 0 )
        return( -1 );

  len = wchar2ushort(yomi, maxyomi, cbuf, CBUFSIZE);
  nkanji = _RkwGetSimpleKanji(cxnum, dicname, cbuf, len,
			  cbuf2, CBIGBUFSIZE, cbuf3, CBIGBUFSIZE );

  if( nkanji <= 0 || !kanjis || !hinshis )
    return( nkanji );
  if( maxkanjis <= 0 || maxhinshis <= 0 )
    return( 0 );

  for( i = 0 ; i < nkanji ; i++ ) {
    k += ushort2wchar(cbuf2 + j, ushortstrlen(cbuf2 + j),
		   kanjis + k, maxkanjis - k) + 1;
    j += ushortstrlen(cbuf2 + j) + 1;
    l += ushort2euc(cbuf3 + m, ushortstrlen(cbuf3 + m),
		   hinshis + l, maxhinshis - l) + 1;
    m += ushortstrlen(cbuf3 + m) + 1;
  }
    kanjis[k] = hinshis[l] = '\0';
    return nkanji ;
}

/* S002 */
int
RkwQueryDic( int cxnum, const char* username, char* dicname, struct
             DicInfo* status )
{
    RkcContext *cx = getCC( cxnum, NOCHECK );

    if( !cx || !dicname || ProtocolMajor < 3 )
	return( -1 ) ;

    if( !username )
	if( !(username = FindUserName()) )
	    return( -1 ) ;

    if( !status ) {
      struct DicInfo buffer; /* ダミー */

      return (*RKCP->query_dic)(cx, username, dicname, &buffer);
    }
    else{
	return (*RKCP->query_dic)(cx, username, dicname, status);
    }
}

static int
_RkwGetHinshi( int cxnum, cannawc* dst, int maxdst )
{
    RkcContext *cx = getCC( cxnum, CHECK );

    if( cx ){
	return (*RKCP->get_hinshi)(cx, dst, maxdst);
    }
    return( -1 ) ;
}


int
RkwGetHinshi( int cxnum, cannawc* dst, int maxdst )
{
    int len;

    len = _RkwGetHinshi(cxnum, rkc.cbuf, CBUFSIZE);
    if (len < 0) {
        return -1;
    }
  else {
    if( !dst ) {
      return ushort2wchar(rkc.cbuf, len, rkc.wbuf, CBUFSIZE);
    }
    else if( maxdst <= 0 )
      return 0;

    return ushort2wchar(rkc.cbuf, len, dst, maxdst);
  }
}


static int
_RkwStoreRange( int cxnum, cannawc* yomi, int maxyomi )
{
    RkcContext *cx = getCC( cxnum, CHECK );

    if( cx ){
	return (*RKCP->store_range)(cx, yomi, maxyomi);
    }
    return( -1 ) ;
}


int
RkwStoreRange( int cxnum, cannawc* yomi, int maxyomi )
{
    int len;

    if( !yomi || maxyomi <= 0 )
        return -1;

  len = wchar2ushort(yomi, maxyomi, rkc.cbuf, CBUFSIZE);
  return _RkwStoreRange(cxnum, rkc.cbuf, len);
}


int
RkwSetLocale( int cxnum, char* locale )
{
    RkcContext *cx = getCC( cxnum, NOCHECK );

    if( cx ){
	return (*RKCP->set_locale)(cx, (char *)locale);
    }
    return( -1 ) ;
}


/**
 *  辞書ファイルへの同期処理
 *
 *  Input:
 *  -----
 *  dicname: 辞書名@辞書名@...@@
 *
 * @return  0 or -1
 */
int
RkwSync( int cxnum, const char* dicname )
{
    RkcContext *cx = getCC( cxnum, NOCHECK );

  if (cx) {
    if (canna_version(ProtocolMajor, ProtocolMinor) > canna_version(3, 1)) {
      return (*RKCP->sync)(cx, dicname ? dicname : "");
    }
  }
  return -1;
}


/**
 *  アプリケーション名の登録
 *
 *  Input:
 *  -----
 *  apname: アプリケーション名
 *
 *  Returns:
 *  -------
 *  0 or -1
 */
int
RkwSetAppName( int cxnum, char* apname )
{
    RkcContext *cx = getCC( cxnum, NOCHECK );

    if( cx && (ProtocolMajor > 2) && apname){
	return (*RKCP->set_app_name)(cx, apname);
    }
    return( -1 ) ;
}


/*
 *  RkwChmodDic ()
 *
 *  Description:
 *  -----------
 *  辞書のアクセス権の変更
 *
 *  Input:
 *  -----
 *  dicname: 辞書の名前
 *
 *  mode: モード
 *
 *  Returns:
 *  -------
 *  0 or -1
 */
exp(int)
RkwChmodDic(int cxnum, char* dicname, int mode)
{
    RkcContext *cx = getCC( cxnum, NOCHECK );

    if (cx &&
      canna_version(ProtocolMajor, ProtocolMinor) > canna_version(3, 1)) {
    return (*RKCP->chmod_dic)(cx, dicname, mode);
  }
  return -1;
}


/* EUC functions */

#ifndef OMIT_EUC_FUNCS

int
RkInitialize( char* hostname )
{
    return( RkwInitialize( hostname ) ) ;
}

void
RkFinalize()
{
    RkwFinalize() ;
}

int RkKillServer()
{
    return RkwKillServer();
}

int
RkCloseContext( int cxnum )
{
    return( RkwCloseContext( cxnum ) ) ;
}

int
RkCreateContext()
{
    return( RkwCreateContext() ) ;
}

int
RkDuplicateContext( int src_cx )
{
    return( RkwDuplicateContext( src_cx ) ) ;
}


int
RkGetDicList( int cxnum, char* dicnames, int max)
{
    return( RkwGetDicList( cxnum, dicnames, max ) ) ;
}


int
RkDefineDic( int cxnum, char* dicname, unsigned char* wordrec)
{
    if( !dicname || !wordrec )
	return( -1 ) ;

    euc2ushort(wordrec, strlen((char*) wordrec), rkc.cbuf, CBUFSIZE);
    return _RkwDefineDic(cxnum, dicname, rkc.cbuf);
}


int
RkDeleteDic(int cxnum, char* dicname, unsigned char* wordrec)
{
    cannawc cbuf[CBUFSIZE];

    if( !dicname || !wordrec )
	return( -1 ) ;

    euc2ushort(wordrec, strlen((char*) wordrec), cbuf, CBUFSIZE);
    return( _RkwDeleteDic(cxnum, dicname, cbuf) );
}


int
RkMountDic( int cxnum, char* dicname, int mode )
{
    return( RkwMountDic( cxnum, dicname, mode ) );
}


int
RkRemountDic( int cxnum, char* dicname, int where )
{
    return( RkwRemountDic( cxnum, dicname, where ) );
}


int
RkUnmountDic( int cxnum, char* dicname )
{
    return( RkwUnmountDic( cxnum, dicname ) );
}


int
RkGetMountList( int cxnum, char* dicnames_return, int max_ )
{
    return( RkwGetMountList( cxnum, dicnames_return, max_ ) );
}


/* サーチパスを設定 */
int
RkSetDicPath( int cxnum, char* path )
{
    /*
     * サーバが立ち上がるときに設定するのでRKCでは何もせずに返す
     */
    return( 0 ) ;
}


/* 辞書リストを取得 */
int
RkGetDirList( int cxnum, char* ddname, int maxddname )
{
#ifdef USE_EUC_PROTOCOL
    register RkcContext *cx = getCC( cxnum, NOCHECK ) ;

    if( !cx || ProtocolMajor > 1 )
	return( -1 ) ;

    if( !ddname ) {
	char buffer[BUFSIZE] ;

	return (rkc_get_dir_list(cx, buffer, BUFSIZE));
    } else if( maxddname <= 0 )
	return( 0 ) ;

    return( rkc_get_dir_list( cx, ddname, maxddname ) ) ;
#else /* !USE_EUC_PROTOCOL */
    return -1;
#endif /* !USE_EUC_PROTOCOL */
}


int
RkBgnBun(int cxnum, unsigned char* yomi, int maxyomi, int mode)
{
    cannawc cbuf[CBIGBUFSIZE];
    int len;

    if (yomi) {
        len = euc2ushort(yomi, maxyomi, cbuf, CBIGBUFSIZE);
        return( _RkwBgnBun(cxnum, cbuf, len, mode) );
    }
    else {  /* 自動変換開始 */
      return _RkwBgnBun(cxnum, NULL, maxyomi, mode);
    }
}


int
RkEndBun( int cxnum, int mode )
{
    return( RkwEndBun( cxnum, mode ) );
}

int
RkXfer( int cxnum, int knum )
{
    return( RkwXfer( cxnum, knum ) );
}


int
RkNfer( int cxnum )
{
    return( RkwNfer( cxnum ) );
}


int
RkNext( int cxnum )
{
    return( RkwNext( cxnum ) );
}


int
RkPrev( int cxnum)
{
    return( RkwPrev( cxnum ) );
}


int
RkGetKanji( int cxnum, unsigned char* kanji, int maxkanji)
{
    cannawc cbuf[CBUFSIZE];
    int len;

    if( (len = _RkwGetKanji(cxnum, cbuf, CBUFSIZE)) < 0 ){
	return( len );
    }
    else {
	if( !kanji ) {
            unsigned char buffer[CBUFSIZE];
            return ushort2euc(cbuf, len, buffer, CBUFSIZE) ;
	}
	else if( maxkanji <= 0 )
	    return( 0 );

        return ushort2euc(cbuf, len, kanji, maxkanji);
    }
}


int
RkGetKanjiList( int cxnum, unsigned char* kanjis, int maxkanjis)
{
    cannawc cbuf[CBIGBUFSIZE];
    int nkanji, len, i, j = 0, k = 0;
    unsigned char eucbuf[CBUFSIZE*2];				/* S005 */
    int euclen;							/* S005 */

  if( !kanjis ) {
    return( _RkwGetKanjiList(cxnum, NULL, 0) );
  }
  else if( maxkanjis <= 0 )
    return( 0 );

  nkanji = _RkwGetKanjiList(cxnum, cbuf, CBIGBUFSIZE);

    for (i = 0 ; i < nkanji ; i++) {
        len = ushortstrlen(cbuf + j);
        euclen = ushort2euc(cbuf + j, len, eucbuf, CBUFSIZE * 2);	/* S005 */
    if (k + euclen > maxkanjis - 2)				/* S005 */
      break;							/* S005 */
    strcpy((char *)kanjis + k, (char *)eucbuf);			/* S005 */
    k += euclen + 1;						/* S005 */
    j += len + 1;
  }
  kanjis[k] = (unsigned char)0;
  return( i );							/* S005 */
}


int
RkGoTo( int cxnum, int bnum)
{
    return( RkwGoTo( cxnum, bnum ) );
}


int
RkLeft( int cxnum )
{
    return( RkwLeft( cxnum ) );
}


int
RkRight( int cxnum )
{
    return( RkwRight( cxnum ) );
}


int
RkResize( int cxnum, int len)
{
    /* この len はバイトやけど，RkwResize には文字数を渡さなあかんのと
       ちゃうやろか? こんなん，どないやって変換したらええんやろ? */
    cannawc cbuf[CBUFSIZE];
    unsigned char tmpbuf[BUFSIZE];
    register int euclen, uslen = 0;
    int curbun, ret;
    register RkcContext  *cx = getCC( cxnum, CHECK ) ;

    if( cx ) {
	if( len <= 0 )
	    return( cx->maxbun );

	/* しゃあないから，読みを取ってきて文字数を調べたろ */
	curbun = cx->curbun;
	for( ; (cx->curbun < cx->maxbun) && len; (cx->curbun)++ ) {
	    /* こんなんしとったらめちゃくちゃ効率悪いがな...とほほ */
	    if( (ret = _RkwGetYomi( cx, cbuf, CBUFSIZE )) < 0 ) {
		cx->curbun = curbun;
		return( -1 ) ;
	    }
	    if( (euclen = ushort2euc(cbuf, ret, tmpbuf, BUFSIZE)) > len ) {
		uslen += euc2ushort(tmpbuf, len, cbuf, CBUFSIZE);
		break;
	    } else {
		uslen += ret;
		len -= euclen;
	    }
	}
	cx->curbun = curbun;
    } else {
	if( len <= 0 )
	    return( 0 );
    }
    return( RKReSize( cxnum, uslen ) );
}


/* 文節伸ばし */
int
RkEnlarge( int cxnum )
{
    return( RKReSize( cxnum, ENLARGE  ) ) ;
}


/* 文節縮め */
int
RkShorten( int cxnum)
{
    return( RKReSize( cxnum, SHORTEN ) ) ;
}


int
RkStoreYomi( int cxnum, unsigned char* yomi, int maxyomi)
{
    cannawc cbuf[CBUFSIZE];
    int len;

    if (yomi && maxyomi >= 0) {
        len = min( (int)strlen((char*) yomi), maxyomi);
        len = euc2ushort(yomi, len, cbuf, CBUFSIZE) + 1;
    }
    else {
        cbuf[0] = 0;
        len = 0;
    }
    return _RkwStoreYomi(cxnum, cbuf, len) ;
}


int
RkGetYomi( int cxnum, unsigned char* yomi, int maxyomi)
{
    cannawc cbuf[CBUFSIZE];
    int len;

    if ((len = _RkwGetYomi(getCC( cxnum, CHECK ), cbuf, CBUFSIZE)) < 0)
      return len ;

    if( !yomi ) {
        unsigned char buffer[CBUFSIZE];
      return ushort2euc(cbuf, len, buffer, CBUFSIZE) ;
    }
    else if( maxyomi <= 0 )
      return( 0 );

    return ushort2euc(cbuf, len, yomi, maxyomi);
}


int
RkGetLex( int cxnum, RkLex* lex, int maxlex)
{
    RkLex *tango;
    cannawc ybuf[CBUFSIZE], kbuf[CBUFSIZE];
    unsigned char cbuf[BUFSIZE];
    int i, len, ylen, klen, ret;

    if( (ret = RkwGetLex( cxnum, lex, maxlex )) <= 0 )
	return( ret );
    if (lex != (RkLex *)NULL) {
      len = _RkwGetYomi(getCC( cxnum, CHECK ), ybuf, CBUFSIZE);
      len = _RkwGetKanji(cxnum, kbuf, CBUFSIZE);
      ylen = klen = 0;
      tango = lex;
      for( i = 0; i < ret; i++ ){
	len = tango->ylen;
	tango->ylen = ushort2euc(ybuf + ylen, len, cbuf, BUFSIZE);
	ylen += len;
	len = tango->klen;
	tango->klen = ushort2euc(kbuf + klen, len, cbuf, BUFSIZE);
	klen += len;
	tango++;
      }
    }
    return( ret );
}


int
RkGetStat( int cxnum, RkStat* stat)
{
    unsigned char cbuf[BUFSIZE];
    int ret;

    if( (ret = RkwGetStat( cxnum, stat )) < 0 )
	return( ret );
    stat->ylen = RkGetYomi(cxnum, cbuf, BUFSIZE);
    stat->klen = RkGetKanji(cxnum, cbuf, BUFSIZE);
    return( ret );
}


#ifdef EXTENSION
int
RkListDic( int cxnum, char* dirname, char* dicnames_return, int size )
{
    return RkwListDic(cxnum, (char *)dirname,(char *) dicnames_return, size);
}

exp(int)
RkCreateDic( int cxnum, char* dicname, int mode )
{
    return RkwCreateDic(cxnum, (char *)dicname, mode);
}

int RkRemoveDic( int cxnum, char* dicname, int mode )
{
    return RkwRemoveDic(cxnum, (char *)dicname, mode);
}

int RkRenameDic( int cxnum, char* dicname, char* newdicname, int mode )
{
    return RkwRenameDic(cxnum, (char *)dicname, (char *)newdicname, mode);
}


int RkCopyDic( int cxnum, char* dirname, char* dicname, char* newdicname,
               int mode)
{
    return RkwCopyDic(cxnum, (char *)dirname, (char *)dicname,
		    (char *)newdicname, mode);
}


exp(int)
RkGetWordTextDic( int cxnum, char* dirname, char* dicname,
                  char* info, int infolen)
{
    cannawc cbuf[CBUFSIZE];
    int len;

    if ((len = _RkwGetWordTextDic(cxnum, dirname, dicname, cbuf, CBUFSIZE)) < 0)
        return len;

    if( !info ) {
        unsigned char buffer[CBUFSIZE];
        return ushort2euc(cbuf, len, buffer, CBUFSIZE) ;
    }
    else if( infolen <= 0 )
      return 0;

    return ushort2euc(cbuf, len, (unsigned char*) info, infolen);
}

#else
RkListDic(){}
RkCreateDic(){}
RkRemoveDic(){}
RkRenameDic(){}
RkGetWordTextDic(){}
#endif /* EXTENSION */

int
RkSubstYomi( int cxnum, int ys, int ye, unsigned char* yomi, int nyomi )
{
    RkcContext *cx = getCC( cxnum, CHECK );
    unsigned char cbuf[CBUFSIZE];
    cannawc cbuf2[CBUFSIZE];
    int len;

  if (cx) {
    ushort2euc(cx->lastyomi, cx->maxyomi, cbuf, CBUFSIZE);
    ys = euc2ushort(cbuf, ys, cbuf2, CBUFSIZE);
    ye = euc2ushort(cbuf, ye, cbuf2, CBUFSIZE);
    len = euc2ushort(yomi, nyomi, cbuf2, CBUFSIZE);
    return( _RkwSubstYomi(cxnum, ys, ye, cbuf2, len) );
  }
  return( -1 ) ;
}


int
RkFlushYomi( int cxnum )
{
    return( RkwFlushYomi( cxnum ) );
}

int
RkGetLastYomi( int cxnum, unsigned char* yomi, int maxyomi )
{
    cannawc cbuf[CBUFSIZE];
    int len;

    if( (len = _RkwGetLastYomi(cxnum, cbuf, CBUFSIZE)) < 0 )
        return -1;

    if( !yomi ) {
        unsigned char buffer[CBUFSIZE];
        return ushort2euc(cbuf, len, buffer, CBUFSIZE) ;
    }
    else if( maxyomi <= 0 )
      return 0;

    return ushort2euc(cbuf, len, yomi, maxyomi);
}


int
RkRemoveBun( int cxnum, int mode )
{
    return( RkwRemoveBun( cxnum, mode ) );
}


int
RkGetSimpleKanji( int cxnum, char* dicname, unsigned char* yomi, int maxyomi,
                  unsigned char* kanjis, int maxkanjis,
                  unsigned char* hinshis, int maxhinshis )
{
    cannawc cbuf[CBUFSIZE], cbuf2[CBIGBUFSIZE], cbuf3[CBIGBUFSIZE];
    int nkanji, len, i, j = 0, k = 0, l = 0, m = 0;

  if( !dicname || !yomi || maxyomi <= 0 )
    return( -1 );

  len = euc2ushort(yomi, maxyomi, cbuf, CBUFSIZE);
  nkanji = _RkwGetSimpleKanji(cxnum, dicname, cbuf, len,
			  cbuf2, CBIGBUFSIZE, cbuf3, CBIGBUFSIZE );

  if( nkanji <= 0 || !kanjis || !hinshis )
    return( nkanji );
  if( maxkanjis <= 0 || maxhinshis <= 0 )
    return( 0 );

  for( i = 0 ; i < nkanji ; i++ ) {
    k += ushort2euc(cbuf2 + j, ushortstrlen(cbuf2 + j),
                    kanjis + k, maxkanjis - k) + 1;
    j += ushortstrlen(cbuf2 + j) + 1;
    l += ushort2euc(cbuf3 + m, ushortstrlen(cbuf3 + m),
                    hinshis + l, maxhinshis - l) + 1;
    m += ushortstrlen(cbuf3 + m) + 1;
  }
  kanjis[k] = hinshis[l] = (unsigned char)0;
  return ( nkanji );
}


/* S002 */
int
RkQueryDic( int cxnum, char* username, char* dicname, struct DicInfo* status )
{
    return RkwQueryDic(cxnum, username, dicname, status);
}

int
RkGetHinshi( int cxnum, unsigned char* dst, int maxdst )
{
    cannawc cbuf[CBUFSIZE];
    int len;

    if( (len = _RkwGetHinshi( cxnum, cbuf, CBUFSIZE )) < 0 )
        return -1;

    if( !dst ) {
        unsigned char buffer[CBUFSIZE];
        return ushort2euc(cbuf, len, buffer, CBUFSIZE) ;
    }
    else if( maxdst <= 0 )
      return 0;

    return ushort2euc(cbuf, len, dst, maxdst);
}


int
RkStoreRange( int cxnum, unsigned char* yomi, int maxyomi )
{
    cannawc cbuf[CBUFSIZE];
    int len;

    if( !yomi || maxyomi <= 0 )
        return -1;

    len = euc2ushort(yomi, maxyomi, cbuf, CBUFSIZE);
    return _RkwStoreRange( cxnum, cbuf, len ) ;
}

int
RkSetLocale( int cxnum, char* locale )
{
    return( RkwSetLocale( cxnum, locale ) );
}

int
RkSync( int cxnum, char* dicname )
{
    return( RkwSync( cxnum, dicname ) );
}

int
RkSetAppName( int cxnum, char* apname )
{
    return( RkwSetAppName( cxnum, apname ) );
}

int
RkChmodDic( int cxnum, char* dicname, int mode)
{
    return RkwChmodDic(cxnum, (char *)dicname, mode);
}
#endif /* OMIT_EUC_FUNCS */


/**
 *  commandで示される機能を実行する
 *
 *  Input:
 *  -----
 *  command: コマンド番号
 *  buf: データ/結果の先頭ポインタ
 *  content_size: データのサイズ
 *  buffer_size: 結果領域のサイズ
 *
 *  Returns:
 *  -------
 *  bufに格納された大きさ or -1
 */
int
RkThrough( int cxnum, int command, char* buf, int content_size,
           int buffer_size )
{
    RkcContext *cx = getCC( cxnum, NOCHECK );

    if( cx ){
	return (*RKCP->through)
	  (cx, command, (char *)buf, content_size, buffer_size);
    }
    return( -1 ) ;
}							/* S000:end */
