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

/* $Id: file.h,v 1.5 2003/09/21 12:56:28 aida_s Exp $ */

#ifndef	RKINDEP_FILE_H
#define RKINDEP_FILE_H

#ifdef NEED_RKINDEP_SUBST
# include "RKindep/file.sub"
#endif

/* NFD and FD_SET macros are based on canuum/wnn_os.h */
#ifndef _WIN32
  #include <unistd.h>
  #include <sys/select.h>
  #include <sys/socket.h>
  typedef int SOCKET;
  #define INVALID_SOCKET -1
#else
  #include <winsock2.h>
  #define fd_set FD_SET
#endif // !_WIN32
#include <sys/types.h>
#include <limits.h>
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#include <sys/time.h> // `timeval` structure
#include <time.h>

#include <stdio.h>

#if defined (OPEN_MAX)
# define RKI_NFD OPEN_MAX
#elif defined (NOFILE)
# define RKI_NFD NOFILE
#endif

typedef fd_set rki_fd_set;
# define RKI_FD_SET(fd, set) FD_SET(fd, set)
# define RKI_FD_CLR(fd, set) FD_CLR(fd, set)
# define RKI_FD_ISSET(fd, set) FD_ISSET(fd, set)
# define RKI_FD_ZERO(set) FD_ZERO(set)
# ifdef FD_SETSIZE
#  define RKI_FD_SETSIZE FD_SETSIZE
# else
#  define RKI_FD_SETSIZE (sizeof(fd_set) * 8)
# endif

#ifdef __cplusplus
extern "C" {
#endif

extern int RkiConnect pro((SOCKET fd, struct sockaddr *addrp, size_t len,
      const struct timeval *timeout));
extern char *RkiGetLine pro((FILE *src));
extern void *RkiReadWholeFile pro((FILE *src, size_t *retsize));

extern const char* RkiBasename pro((const char *src));

extern int non_blocking( SOCKET sock, int mode );

#ifdef __cplusplus
}
#endif

#endif	/* RKINDEP_FILE_H */
