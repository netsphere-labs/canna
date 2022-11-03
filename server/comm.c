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

#define _POSIX_C_SOURCE 200809L // これがないと struct addrinfo が有効にならない
#include "server.h"
#include "RKindep/file.h"
#include "RKindep/strops.h"
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#ifndef _WIN32
  #ifdef USE_UNIX_SOCKET  /* UNIX ドメインの作成 */
    #include <sys/un.h>
  #endif
#endif

RCSID("$Id: comm.c,v 1.4.2.3 2004/04/26 21:48:37 aida_s Exp $");

/* TODO: better error reporting */

#define COMM_DEBUG
#define MAX_LISTENERS 3
#define FIRST_WANT 4

// Listen socket. 一つの listen socket に複数の client buf がぶら下がる.
typedef struct {
    SOCKET l_fd;
    GetConnectionInfoProc l_info_proc;
    void *l_info_obj;
} ListenerRec;


// クライアントとやり取りするためのバッファ
struct tagClientBuf {
    SOCKET fd; // Accepted socket.
    const ListenerRec *parent; // Listen socket.
  ClientPtr client;
  size_t nwant;
  char *sendptr;
  unsigned int nfail;
  RkiStrbuf recvbuf;
  RkiStrbuf sendbuf;
};

// client buf のリスト (の1要素)
typedef struct tagClibufList {
  struct tagClibufList *cbl_next;
  int cbl_finalized;
  ClientBuf cbl_body;
} ClibufList;
#define MEMBER_TO_OBJ(t, x, m) ((t *)((char *)(x) - offsetof(t, m)))
#define CBL_BODY_TO_ENTRY(clibuf) MEMBER_TO_OBJ(ClibufList, clibuf, cbl_body)

// 唯一のイベントマネジャーインスタンス.
EventMgr *global_event_mgr = NULL;

static void ClientBuf_destroy pro((ClientBuf *obj));
static int ClientBuf_recv pro((ClientBuf *obj));
static int ClientBuf_send pro((ClientBuf *obj));
#define ClientBuf_getfd_fast(obj) ((obj)->fd)
#define CLIENT_BUF_IS_SENDING(obj) \
    ((obj)->sendbuf.sb_curr != (obj)->sendbuf.sb_buf)


static int
process_request(ClientPtr *clientp, ClientBuf *client_buf, BYTE *data,
                size_t len)
{
  int request;
  int nwant, r;
  ClientPtr client = *clientp;
  const char *username = client ? client->username : NULL;
  const char *hostname = client ? client->hostname : NULL;

#ifdef DEBUG
  CallFuncName = NULL;
#endif
  if (client && client->version_hi > 1)
    nwant = parse_wide_request(&request, data, len, username, hostname);
  else
    nwant = parse_euc_request(&request, data, len, username, hostname);

  if (nwant)
    return nwant; /* 失敗、またはもっとデータが必要 */

  /* 実際のプロトコルに応じた処理（関数を呼ぶ） */

  if (client) /* initialize等の場合は呼ばない */
      (void)ClientStat(client, SETTIME, request, 0);
  /* プロトコルの種類毎に統計を取る */
  if (client && client->version_hi > 1) {
#ifdef EXTENSION
    if( request < W_MAXREQUESTNO )
#endif
      ++client->pcount[request];
  } else if (client) {
#ifdef EXTENSION
    if( request < MAXREQUESTNO )
#endif
      ++client->pcount[request];
  }

#ifdef DEBUG
  if (CallFuncName)
    Dmsg( 3,"Now Call %s\n", CallFuncName );
#endif
  if (!client)
    r = ir_nosession(clientp, client_buf);
  else
    r = (*CallFunc) (clientp);
  ir_debug(Dmsg(8,"%s returned %d\n", CallFuncName, r));

  /* クライアントの累積サーバ使用時間を設定する */
  if (client && client == *clientp) /* initialize,finalize等のときは呼ばない */
    ClientStat(client, GETTIME, request, 0);

  if (r)
    r = -1; /* どういう失敗でもとりあえず-1を返す */
  return r;
}


static void
ClientBuf_init( ClientBuf *obj, const ListenerRec *parent, SOCKET fd )
{
    assert(obj);

  obj->fd = fd;
  obj->parent = parent;
  obj->client = NULL;
  obj->nwant = FIRST_WANT;
#ifdef COMM_DEBUG
  obj->sendptr = (char *)0xdeadbeef;
#endif
  obj->nfail = 0;
  RkiStrbuf_init(&obj->recvbuf);
  RkiStrbuf_init(&obj->sendbuf);
}


static void
ClientBuf_destroy( ClientBuf *obj )
{
  close(obj->fd);
  close_session(&obj->client, 0);
  RkiStrbuf_destroy(&obj->recvbuf);
  RkiStrbuf_destroy(&obj->sendbuf);
}


static int
ClientBuf_recv(ClientBuf *obj)
{
  ssize_t size;
  int newwant;
  RkiStrbuf *buf = &obj->recvbuf;
  int savederr;

  assert(obj->nwant && !CLIENT_BUF_IS_SENDING(obj));
  if (RKI_STRBUF_RESERVE(buf, buf->sb_curr - buf->sb_buf + obj->nwant)) {
    nomem_msg("ClientBuf_recv()");
    return -1;
  }
  ir_debug(Dmsg(7, "ClientBuf_recv(): receiving %d bytes, nwant=%d\n",
	buf->sb_end - buf->sb_curr, obj->nwant));
  size = recv(obj->fd, buf->sb_curr, buf->sb_end - buf->sb_curr, 0);
  savederr = errno;
  ir_debug(Dmsg(7, "ClientBuf_recv(): recv() returned %d\n", size));
  if (size < 0) {
    if (savederr == EINTR || savederr == EWOULDBLOCK || savederr == EAGAIN) {
      if (++obj->nfail < 5)
	return 0;
      ir_debug(Dmsg(7,
	    "ClientBuf_recv(): too many temporary errors. errno=%d\n",
	    savederr));
    }
    goto recvfail;
  } else if (size == 0)
    goto recvfail;

  obj->nfail = 0;
  buf->sb_curr += size;
  if (size < obj->nwant) {
    obj->nwant -= size;
    return 0;
  }
  obj->nwant = 0;

  newwant = process_request(&obj->client, obj,
      (BYTE *)buf->sb_buf, buf->sb_curr - buf->sb_buf);
  ir_debug(Dmsg(7, "ClientBuf_recv(): newwant=%d\n", size));
  if (newwant < 0)
    return -1;
  obj->nwant = newwant;
  return 0;

recvfail:
  PrintMsg("[%s] Receive request failed\n",
      obj->client ? obj->client->username : "unknown");
  ir_debug(Dmsg(5, "ClientBuf_recv(): Receive request failed\n"));
  return -1;
}


static int
ClientBuf_send( ClientBuf *obj )
{
  ssize_t size;
  RkiStrbuf *buf = &obj->sendbuf;
  int savederr = 0;

  assert(!obj->nwant && CLIENT_BUF_IS_SENDING(obj));
  ir_debug(Dmsg(7, "ClientBuf_send(): sending %d bytes\n",
	buf->sb_curr - obj->sendptr));
  size = send(obj->fd, obj->sendptr, buf->sb_curr - obj->sendptr, 0);
  savederr = errno;
  ir_debug(Dmsg(7, "ClientBuf_send(): send() returned %d\n", size));
  if (size < 0) {
    if (savederr == EINTR || savederr == EWOULDBLOCK || savederr == EAGAIN) {
      if (++obj->nfail < 5)
	return 0;
      ir_debug(Dmsg(7,
	    "ClientBuf_send(): too many temporary errors. errno=%d\n",
	    savederr));
    }
    goto fail;
  }
  assert(size > 0);
  obj->nfail = 0;
  obj->sendptr += size;
  if (obj->sendptr == buf->sb_curr) {
    ir_debug(Dmsg(5, "クライアントへの返信が完了, fd=%d\n", obj->fd));
#ifdef COMM_DEBUG
    obj->sendptr = (char *)0xdeadbeef;
#endif
    obj->nwant = FIRST_WANT;
#if 0 && defined(COMM_DEBUG)
    RkiStrbuf_clear(&obj->recvbuf);
    RkiStrbuf_clear(buf);
#else
    obj->recvbuf.sb_curr = obj->recvbuf.sb_buf;
    buf->sb_curr = buf->sb_buf;
#endif
  }
  return 0;

fail:
  PrintMsg("[%s] Send request failed\n",
      obj->client ? obj->client->username : "unknown");
  Dmsg(1, "Send Error[ %d ]\n", savederr);
  return -1;
}


int
ClientBuf_store_reply( ClientBuf *obj, const BYTE *data, size_t len )
{
  ir_debug(Dmsg(7, "ClientBuf_store_reply() start\n"));
  assert(!obj->nwant && !CLIENT_BUF_IS_SENDING(obj));
  if (RkiStrbuf_addmem(&obj->sendbuf, data, len)) {
    nomem_msg("ClientBuf_store_reply()");
    return -1;
  }
  obj->sendptr = obj->sendbuf.sb_buf;
  return 0;
}


// server/session.c からのみ呼び出される.
int
ClientBuf_get_connection_info( ClientBuf *obj, struct sockaddr* addr,
                               char **hostname )
{
    assert(addr);
    const ListenerRec *parent = obj->parent;
    return (*parent->l_info_proc)(parent->l_info_obj, obj->fd, addr, hostname);
}


SOCKET ClientBuf_getfd( ClientBuf *obj )
{
  return obj->fd;
}

ClientPtr
ClientBuf_getclient( ClientBuf *obj )
{
  return obj->client;
}

struct tagEventMgr {
    // 一つのイベントマネジャーは複数のソケットを listen.
    ListenerRec listeners[MAX_LISTENERS];
    size_t nlisteners; // 登録ずみの数.
    ClibufList *cbl;  // 所有する.
  size_t nclibufs;
  int quitflag;
  int exit_status;
};


// 新しいイベントマネジャーを生成して返す.
// @return If failed, NULL.
EventMgr* EventMgr_new()
{
    EventMgr* obj = (EventMgr*) malloc(sizeof(EventMgr));
    if (!obj)
        return NULL;
    obj->nlisteners = 0;
    obj->cbl = NULL;
    obj->nclibufs = 0;
    obj->quitflag = 0;
    obj->exit_status = 220; /* これは絶対に返らない */

    return obj;
}


void EventMgr_delete( EventMgr *obj )
{
  ClibufList *curr;
#ifdef COMM_DEBUG
  size_t nclibufs = 0;
#endif
  if (!obj)
    return;
  curr = obj->cbl;
  while (curr) {
    ClibufList *next = curr->cbl_next;
#ifdef COMM_DEBUG
    ++nclibufs;
#endif
    ClientBuf_destroy(&curr->cbl_body);
    free(curr);
    curr = next;
  }
#ifdef COMM_DEBUG
  assert(nclibufs == obj->nclibufs);
#endif
  free(obj);
}

int
EventMgr_add_listener_sock( EventMgr *obj, SOCKET listenerfd,
                            GetConnectionInfoProc info_proc, void *info_obj )
{
    ListenerRec *entry = obj->listeners + obj->nlisteners;

    assert(obj->nlisteners < MAX_LISTENERS);
    assert(listenerfd != INVALID_SOCKET );
  if (listenerfd >= RKI_FD_SETSIZE) {
    PrintMsg("EventMgr_add_listener_sock(): out of rki_fd_set: fd=%d\n",
	listenerfd);
    return -1;
  }
  entry->l_fd = listenerfd;
  entry->l_info_proc = info_proc;
  entry->l_info_obj = info_obj;
  ++obj->nlisteners;
  return 0;
}


void EventMgr_quit_later( EventMgr *obj, int status )
{
  obj->quitflag = 1;
  obj->exit_status = status;
}


void
EventMgr_finalize_notify( EventMgr *obj, const ClientBuf *clibuf )
{
  ClibufList *entry = CBL_BODY_TO_ENTRY(clibuf);
  assert(clibuf);
  assert(!entry->cbl_finalized);
  entry->cbl_finalized = 1;
}


// @return 成功 = 0
static int
EventMgr_accept( EventMgr *obj, ListenerRec *listener_entry )
{
    ClibufList *cbl_ent = NULL;
    SOCKET connfd;

    ir_debug(Dmsg(7, "EventMgr_accept() start\n"));
    connfd = accept(listener_entry->l_fd, NULL, 0);
    if (connfd == INVALID_SOCKET) {
        /* rarely happens; probably ECONNABORTED or EINTR */
        PrintMsg("EventMgr_accept(): accept: errno=%d\n", errno);
        goto fail;
    } else if (connfd >= RKI_FD_SETSIZE) {
        PrintMsg("EventMgr_accept(): out of rki_fd_set: fd=%d\n", connfd);
        goto fail;
    }
    // select() で待つので不要.
    //if (non_blocking(connfd, 1)) {
    //PrintMsg("EventMgr_accept(): set_nonblock(): errno=%d\n", errno);
    //goto fail;
    //}

    if (!(cbl_ent = malloc(sizeof(ClibufList))))
        goto nomem;
    ClientBuf_init(&cbl_ent->cbl_body, listener_entry, connfd);
  cbl_ent->cbl_finalized = 0;
  cbl_ent->cbl_next = obj->cbl;
  obj->cbl = cbl_ent;
  ++obj->nclibufs;
  ir_debug(Dmsg(5, "クライアントとの接続に成功, fd=%d\n", connfd));

    return 0;

nomem:
  nomem_msg("EventMgr_accept()");
fail:
  if (cbl_ent)
    ClientBuf_destroy(&cbl_ent->cbl_body);
  free(cbl_ent);
  if (connfd != INVALID_SOCKET)
    close(connfd);
  return -1;
}

static void
EventMgr_check_fds(EventMgr *obj, rki_fd_set *rfds, rki_fd_set *wfds)
{
  int listenerno;
  ClibufList **cbl_link;

  ir_debug(Dmsg(7, "EventMgr_check_fds() start\n"));
  for (listenerno = 0; listenerno < obj->nlisteners; ++listenerno) {
    if (RKI_FD_ISSET(obj->listeners[listenerno].l_fd, rfds))
      EventMgr_accept(obj, obj->listeners + listenerno);
  }

    for (cbl_link = &obj->cbl; *cbl_link; cbl_link = &(*cbl_link)->cbl_next) {
        ClibufList *cbl_ent = *cbl_link;
        ClientBuf *client_buf = &cbl_ent->cbl_body;
        int fd = ClientBuf_getfd_fast(client_buf);
        int error = 0;

        if (RKI_FD_ISSET(fd, rfds))
            error = ClientBuf_recv(client_buf);
    else if (RKI_FD_ISSET(fd, wfds))
      error = ClientBuf_send(client_buf);

    if (error ||
	(cbl_ent->cbl_finalized && !CLIENT_BUF_IS_SENDING(client_buf))) {
      ir_debug(Dmsg(5, "クライアントとの接続を切る, fd=%d\n",
	    ClientBuf_getfd_fast(client_buf)));
      *cbl_link = cbl_ent->cbl_next;
      --obj->nclibufs;
      ClientBuf_destroy(client_buf);
      free(cbl_ent);
      if (!*cbl_link)
	break;
    }
  }
}


int EventMgr_run( EventMgr *obj )
{
  struct timeval timeout;
  int sync_flag = 0;

  timeout.tv_sec = 108; /* why? */
  timeout.tv_usec = 0;

  for (;;) {
    rki_fd_set rfds, wfds;
    int nfds = 0;
    int listenerno;
    ClibufList *cbl_ent;
    struct timeval timeout_tmp;
    int r;
    int needwrite = 0;
    int savederr;

    RKI_FD_ZERO(&rfds);
    RKI_FD_ZERO(&wfds);
    for (listenerno = 0; listenerno < obj->nlisteners; ++listenerno) {
      int fd = obj->listeners[listenerno].l_fd;
      RKI_FD_SET(fd, &rfds);
      nfds = RKI_MAX(nfds, fd + 1);
    }
    for (cbl_ent = obj->cbl; cbl_ent; cbl_ent = cbl_ent->cbl_next) {
      int fd = ClientBuf_getfd_fast(&cbl_ent->cbl_body);
      if (CLIENT_BUF_IS_SENDING(&cbl_ent->cbl_body)) {
	RKI_FD_SET(fd, &wfds);
	needwrite = 1;
      } else {
	if (!obj->quitflag)
	  RKI_FD_SET(fd, &rfds);
      }
      nfds = RKI_MAX(nfds, fd + 1);
    }

    if (obj->quitflag && !needwrite)
      break;
    ir_debug(Dmsg(5, "\nselect()で待ちを開始\n"));
    timeout_tmp = timeout;
    r = select(nfds, &rfds, &wfds, NULL, &timeout_tmp);
    savederr = errno;
    ir_debug(Dmsg(5, "select() returned %d\n", r));
    if (r < 0) {
      if (savederr != EINTR) {
	/* What happened? */
	PrintMsg("EventMgr_run(): select: errno=%d\n", savederr);
	obj->exit_status = 3;
	break;
      }
    } else if (r == 0) {
      /* select の制限時間を越えたので sync 処理を行う */
      if (sync_flag == 0) {/* sync_flag が 0 の時は Allsync を行う */
	ir_debug(Dmsg(5, "EventMgr_run(): select: all sync start\n"));
	AllSync();
	sync_flag = 1; /* 一回行なったので フラグを立てる */
      }
    } else {
      sync_flag = 0; /* データが来たのでフラグを下げる */
      EventMgr_check_fds(obj, &rfds, &wfds);
    }
    if (CheckSignal()) {
      obj->exit_status = 1;
      break;
    }
  }
  return obj->exit_status;
}


void
EventMgr_clibuf_first( EventMgr *obj, EventMgrIterator *it )
{
  ClibufList *entry = obj->cbl;
  it->entry = entry;
  if (entry)
    it->it_val = &entry->cbl_body;
  else
    it->it_val = NULL;
}


void
EventMgr_clibuf_end( EventMgr *obj, EventMgrIterator *it )
{
  it->it_val = NULL;
}


void
EventMgrIterator_next( EventMgrIterator *obj )
{
  ClibufList *entry = (ClibufList *)obj->entry;
  ClibufList *next = entry->cbl_next;
  obj->entry = next;
  if (next)
    obj->it_val = &next->cbl_body;
  else
    obj->it_val = NULL;
}

enum {
  SOCK_BIND_ERROR = -1,
  SOCK_OTHER_ERROR = -2,
  SOCK_OK = 0
};

#ifdef USE_UNIX_SOCKET  /* UNIX ドメインの作成 */
// listen() までを行う.
// @param sock [out] ソケットのファイルディスクリプタを得る.
static int open_unix_socket( SOCKET* sock )
{
    assert( sock );

    //int oldUmask;
    SOCKET request = INVALID_SOCKET;
    int status = SOCK_OTHER_ERROR;

    struct sockaddr_un unaddr;
    memset(&unaddr, 0, sizeof unaddr);
    unaddr.sun_family = AF_UNIX;
    //oldUmask = umask(0);   umask をわざわざクリアする必要なし.
    const int sockpathmax = sizeof(unaddr.sun_path) - 3;

    if ( mkdir(IR_UNIX_DIR, 0755) == -1 && errno != EEXIST ) {
        ir_debug( Dmsg(5, "Could not mkdir %s errno = %d\n", IR_UNIX_DIR, errno));
        *sock = request;
        return status;
    }
    if (RkiStrlcpy(unaddr.sun_path, IR_UNIX_PATH, sockpathmax) >= sockpathmax) {
        ir_debug( Dmsg(5, "Path to socket is too long\n"));
        *sock = request;
        return status;
    }
    if ( PortNumberPlus >= 1 ) {
        sprintf( unaddr.sun_path + strlen(unaddr.sun_path), ":%d",
                 PortNumberPlus ) ;
    }

    if ( (request = socket(AF_UNIX, SOCK_STREAM, 0)) == INVALID_SOCKET ) {
        ir_debug( Dmsg(5, "Warning: UNIX socket for server failed.\n"));
        //umask(oldUmask);
        *sock = request;
        return status;
    }

    if ( bind(request, (struct sockaddr*) &unaddr, sizeof(struct sockaddr_un))) {
        fprintf(stderr, "ERROR: Server could not bind %s\n", unaddr.sun_path);
        close(request);
        *sock = INVALID_SOCKET;
        return SOCK_BIND_ERROR;
    }
    ir_debug( Dmsg(5, "UNIX Socket path: %s\n", unaddr.sun_path));

    if ( listen(request, 5) < 0 ) { // mark as passive socket.
        ir_debug( Dmsg(5,"Warning: Server could not listen.\n"));
        close(request);
        unlink(unaddr.sun_path);
        *sock = INVALID_SOCKET;
        return SOCK_BIND_ERROR;
    }

    *sock = request;
    return SOCK_OK;
}
#endif /* USE_UNIX_SOCKET */


#ifdef USE_INET_SOCKET  /* IPv4/IPv6 ソケットの作成 */
// listen() までを行う.
// @param sock [out] ソケットfdを書き戻す.
static int open_inet_socket( SOCKET* sock )
{
    struct addrinfo hints, *info;
    struct addrinfo* infolist = NULL;
    char portbuf[11];
    //const struct servent* sp;
    int retry;
    SOCKET request;
    int status = SOCK_OTHER_ERROR;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;  // IPv4/IPv6両対応
    hints.ai_socktype = SOCK_STREAM;
    // AI_PASSIVE をセットして node = NULLのときは, INADDR_ANY, IN6ADDR_ANY_INIT.
    hints.ai_flags = AI_PASSIVE;

    // 起動時にポート番号指定された
    if ( PortNumberPlus >= 1 ) {
        sprintf(portbuf, "%d", PortNumberPlus);
        hints.ai_flags |= AI_NUMERICSERV;
    }
    else {
        // getservbyname() は廃れたが, デフォルトポート番号を使うかの判定
        // /etc/services ファイルからポート番号取得.
        if ( getservbyname(IR_SERVICE_NAME, "tcp") )
            strcpy(portbuf, IR_SERVICE_NAME);
        else {
            ir_debug( Dmsg(5,"Warning: Port number not found on '/etc/services'.\n"));
            sprintf(portbuf, "%d", IR_DEFAULT_PORT);
            hints.ai_flags |= AI_NUMERICSERV;
        }
    }
    ir_debug( Dmsg(5,"Service: [%s]\n", portbuf));

    *sock = INVALID_SOCKET;
    int err = getaddrinfo( NULL, portbuf, &hints, &infolist);
    if (err != 0) {
        ir_debug( Dmsg(5,"Warning: (internal error) getaddrinfo() failed: %s\n",
                       gai_strerror(err)) );
        return SOCK_OTHER_ERROR;
    }

    request = socket( infolist->ai_family, infolist->ai_socktype,
                      infolist->ai_protocol );
    if (request == INVALID_SOCKET ) {
        ir_debug( Dmsg(5,"Warning: INET socket for server failed.\n"));
        freeaddrinfo(infolist);
        return SOCK_OTHER_ERROR;
    }

    int one = 1;
    setsockopt(request, SOL_SOCKET, SO_REUSEADDR, (char*) &one, sizeof(one));

    if ( bind(request, infolist->ai_addr, infolist->ai_addrlen) < 0 ) {
        ir_debug( Dmsg(5,"Warning: Server could not bind: %s.\n", portbuf) );
        close(request);
        freeaddrinfo(infolist);
        return SOCK_BIND_ERROR;
    }
    // もういらん
    freeaddrinfo(infolist); infolist = NULL;

    if ( listen(request, 5) < 0 ) {
        ir_debug( Dmsg(5,"Warning: Server could not listen.\n"));
        close(request);
        return SOCK_BIND_ERROR;
    }
    // if (set_nonblock(request)) { これはおかしい。select() はブロッキングモード

    *sock = request;
    return SOCK_OK;
}
#endif /* USE_INET_SOCKET */


#ifdef USE_UNIX_SOCKET
// @param addr [out] 通信相手の情報を addr に書き込む
// @param hostname [out] ホスト名の文字列を生成する. 呼び出し側が解放すること
static int
get_addr_unix(void *dummy, SOCKET connfd, struct sockaddr* addr, char **hostname)
{
    assert(addr);
    char buf[MAXDATA];

    //if (gethostname(buf, MAXDATA - 7) < 0) {
    //PrintMsg("gethostname failed\n");
    //return -1;
    //}
    //buf[MAXDATA - 7] = '\0';
    //strcat(buf, "(UNIX)") ;
    socklen_t addrlen = sizeof(struct sockaddr_un);
    int retval = getpeername(connfd, addr, &addrlen);
    if (!retval) { // 成功
        if ( (*hostname = strdup("unix")) == NULL )  // hosts.canna と合わせる
            return -1;
    }
    return retval;
}
#endif // USE_UNIX_SOCKET


#ifdef USE_INET_SOCKET  /* IPv4/IPv6 ソケットの作成 */
static int
get_addr_inet(void *dummy, SOCKET connfd, struct sockaddr* addr, char **hostname)
{
    assert(addr);

    char		buf[MAXDATA];
    socklen_t fromlen = sizeof( struct sockaddr_storage );

    memset(addr, 0, fromlen );
    if ( getpeername(connfd, addr, &fromlen) < 0 ) {
        PrintMsg( "getpeername error No.%d\n", errno );
        return -1;
    }

    if (addr->sa_family == AF_INET || addr->sa_family == AF_INET6) {
        int res = getnameinfo(addr, fromlen, buf, MAXDATA, NULL, 0, 0);
        if (res) {
            /* cannot store even a numeric hostname */
            PrintMsg( "getaddrinfo error No.%d\n", res );
            return -1;
        }
    }
    else {
        PrintMsg( "unknown protocol family: %d\n", addr->sa_family );
        return -1;
    }

    if ((*hostname = strdup(buf)) == NULL)
        return -1;
    return 0;
}
#endif // USE_INET_SOCKET


// 関数名と異なり、ここでソケットを準備する.
// @return If failed, -1.
int SockHolder_new()
{
    int status = SOCK_OK;

#ifdef USE_UNIX_SOCKET
    SOCKET unsock = INVALID_SOCKET;
    //bzero(&obj->unaddr, sizeof obj->unaddr);
#endif
#ifdef USE_INET_SOCKET
    SOCKET insock = INVALID_SOCKET;
#endif

    ir_debug( Dmsg(3,"今からソケットを作る\n") );

#ifdef USE_UNIX_SOCKET /* UNIX ドメイン */
    if ( (status = open_unix_socket(&unsock)) != SOCK_OK) {
        ir_debug( Dmsg(5,"Warning: UNIX domain not created.\n"));
        goto fail;
    }
    ir_debug( Dmsg(3,"UNIX ドメインはできた\n") );
#endif /*  USE_UNIX_SOCKET */

#ifdef USE_INET_SOCKET  /* INET ドメイン */
    if (UseInet) {
        if ( (status = open_inet_socket(&insock)) != SOCK_OK) {
            ir_debug( Dmsg(5,"Warning: INET domain not created.\n"));
            goto fail;
        }
        ir_debug( Dmsg(3, "INET ドメインはできた\n") );
    }
#endif /* USE_INET_SOCKET */

    ir_debug( Dmsg(3,"ソケットの準備はできた\n") );

#ifdef USE_UNIX_SOCKET
    assert(unsock != INVALID_SOCKET);
    if ( EventMgr_add_listener_sock(global_event_mgr, unsock,
                                    &get_addr_unix, NULL) ) {
        return -1;
    }
#endif

#ifdef USE_INET_SOCKET
    if (UseInet) {
        if (EventMgr_add_listener_sock(global_event_mgr,
                                       insock, &get_addr_inet, NULL)) {
            return -1;
        }
    }
#endif

    return 0;

fail:
    if (status == SOCK_BIND_ERROR) {
        fprintf(stderr, "\n");
        fprintf(stderr, "ERROR:\n ");
        fprintf(stderr, "   Another `cannaserver` is running.\n");
        fprintf(stderr, "\n");
    } else {
        assert(status == SOCK_OTHER_ERROR);
        fprintf(stderr, "ERROR: Could not open sockets in some errors\n ");
    }

    return -1;
}


/*
void SockHolder_delete( SockHolder *obj )
{
    if (!obj)
        return;
#ifdef USE_UNIX_SOCKET
    if (obj->unsock != INVALID_SOCKET) {
        close(obj->unsock);
        unlink(obj->unaddr.sun_path);
    }
#endif
#ifdef USE_INET_SOCKET
    if (obj->insock != INVALID_SOCKET)
        close(obj->insock);
#endif
    free(obj);
}
*/
