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

#include "config.h"
#include "canna/ccompat.h"
#include "RKindep/strops.h"
#include <assert.h>

RCSID("$Id: strops.c,v 1.2 2003/09/06 13:59:33 aida_s Exp $");

void
RkiStrbuf_init(RkiStrbuf* sb)
{
    sb->sb_buf = sb->sb_curr = sb->sb_end = NULL;
}

void
RkiStrbuf_destroy(RkiStrbuf* sb)
{
    free(sb->sb_buf);
    sb->sb_buf = sb->sb_curr = sb->sb_end = NULL;
}

void
RkiStrbuf_clear(RkiStrbuf* sb)
{
    free(sb->sb_buf);
    sb->sb_buf = sb->sb_curr = sb->sb_end = NULL;
}


// @param size "追加する" バイト数.
// @return If failed, -1
int
RkiStrbuf_reserve(RkiStrbuf* sb, size_t size)
{
    size_t oldsize = sb->sb_end - sb->sb_buf;
    size_t newsize;
    size_t used = sb->sb_curr - sb->sb_buf;
    char *tmp;
    if (used + size < oldsize)
        return 0;
    newsize = oldsize ? (oldsize * 2 + size) : (size < 20) ? 20 : size;
    tmp = (char*) realloc(sb->sb_buf, newsize + 1); // +1: for _term()
    if (!tmp)
        return -1;
    sb->sb_buf = tmp;
    sb->sb_curr = tmp + used;
    sb->sb_end = tmp + newsize;
    return 0;
}

int
RkiStrbuf_term(RkiStrbuf* sb)
{
    assert(sb);
    if (sb->sb_curr && !*sb->sb_curr) // これ、領域の一つ外を指すことがある.
        return 0; /* already terminated */
    //if (RKI_STRBUF_RESERVE(sb, 1))
    //return -1;
    *sb->sb_curr = '\0';
    return 0;
}

void
RkiStrbuf_pack(RkiStrbuf* sb)
{
    size_t used = sb->sb_curr - sb->sb_buf;
    char *tmp;
    tmp = (char*) realloc(sb->sb_buf, used + 1); // +1: for _term()
    if (!tmp)
        return;
    sb->sb_buf = tmp;
    sb->sb_curr = sb->sb_end = tmp + used;
}

int
RkiStrbuf_add(RkiStrbuf* sb, const char* src)
{
  return RkiStrbuf_addmem(sb, src, strlen(src));
}

// src のうち size バイトを追加する.
int
RkiStrbuf_addmem(RkiStrbuf* sb, const void* src, size_t size)
{
    if (RKI_STRBUF_RESERVE(sb, size))
        return -1;
    memcpy(sb->sb_curr, src, size);
    sb->sb_curr += size;
    return 0;
}


/* vim: set sw=2: */
