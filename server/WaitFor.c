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
/*
  This program contains a part of the X server.  The communication part
  of the X server is modified and built into this program.
*/
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#if !defined(lint) && !defined(__CODECENTER__)
static char rcs_id[] = "$Id: WaitFor.c,v 2.9 1996/11/27 05:04:35 kon Exp $";
#endif

/* LINTLIBRARY */

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#ifdef AIXV3
#include <sys/select.h>
#endif

#include <sys/param.h>
#include <signal.h>
#include "net.h"
#include "IR.h"

extern long *AllSockets;
extern long *AllClients;
extern long *LastSelectMask;
extern long *clientsReadable;
extern long WellKnownConnections ;

extern ClientPtr *ConnectionTranslation;

extern void CheckConnections();
extern void EstablishNewConnections();

extern int errno;

static int
_anyset(src)
long *src;
{
    int cri;

    for( cri = 0; cri < mskcnt; cri++ ){
	if( src[cri] )
	    return( TRUE );
    }
    return( FALSE );
}
#define ANYSET(src) _anyset(src)

/*****************
 * WaitForSomething:
 *****************/

extern ClientPtr *clientReady;

void
WaitForSomething(nready, nnew)
int *nready;
int *nnew;
{
    int i, sync_flag; /* sync_flag：すでに AllSync を行なったかの確認 */
    long curclient;
    int selecterr;
    struct timeval overtime;

    *nready = 0;
    *nnew = 0;
    CLEARBITS(clientsReadable);
    sync_flag = 0; /* また AllSyncを行なうようにフラグを寝せる */
    while (1)
    {
        COPYBITS(AllSockets, LastSelectMask);
	overtime.tv_sec = (long)108; /* select 制限時間の設定 目一杯(108秒) */
	overtime.tv_usec = (long)0;
	i = select (connow_socks, (FDSET_T *)LastSelectMask,
		    (FDSET_T *) NULL, (FDSET_T *) NULL, &overtime);
	selecterr = errno;

	if (i <= 0) /* An error or timeout occurred */
	{
	  if (i < 0){ 
	    if (selecterr == EBADF)    /* Some client disconnected */
	    {
	      CheckConnections ();
	      if (! ANYSET (AllClients))
	      return;
	    }
	    else if (selecterr != EINTR){
	      PrintMsg("WaitForSomething(): select: errno=%d\n", selecterr);
	    }	
	  }
	  else{ /* select の制限時間を越えたので sync 処理を行なう。 */
	    if (sync_flag == 0)
	    {/* sync_flag が 0 の時は Allsync を行なう*/
/*	      PrintMsg("WaitForSomething(): select: all sync start\n"); */
	      AllSync();
	      sync_flag = 1; /* 一回行なったので フラグを立てる */
	    }	  
	  }
	}
	else
	{
	    /* selectを抜けたＦＤをセットする */
	    MASKANDSETBITS(clientsReadable, LastSelectMask, AllClients) ;

	    /* 接続要求があったか */
	    if (LastSelectMask[0] & WellKnownConnections)
		EstablishNewConnections(nnew) ;

	    /* 要求処理のためにループをぬける */
	    if (*nnew || (ANYSET (clientsReadable)) )
			break;
	}
    }

    if (ANYSET(clientsReadable))
    {
	for (i=0; i<mskcnt; i++)
	{
	    while (clientsReadable[i])
	    {
		curclient = ffs (clientsReadable[i]) - 1;
		clientReady[(*nready)++] =
			ConnectionTranslation[curclient + (32 * i)];
		clientsReadable[i] &= ~(1 << curclient);
	    }
	}	
    }
}
