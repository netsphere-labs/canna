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

/* sccs_id[]="@(#) NEC UNIX( PC-UX/EWS-UX ) rkc.h 2.4 91/11/11 12:01:18"; */
/* #ident	"@(#) NEC/V(386) R3.0B rkc.h 5.9 90/03/26 21:04:36" */
/* $Id: rkc.h,v 1.5 2003/09/17 08:50:53 aida_s Exp $ */

#if defined(ENGINE_SWITCH)
#include "RKrename.h"
#endif

#include "ccompat.h"
#include "rkcapi.h"

/* ʸ�����쥳����
 *
 */

#define MAX_HOSTNAME	256

typedef struct _RkcBun {
    unsigned short  *kanji  ;	/* ������ޤ����������� */
    short	    curcand ;	/* �����ȴ��������ֹ� */
    short	    maxcand ;	/* ����������� */
    short	    flags   ;	/* �ե饰 */
#define NOTHING_KOUHO	 0x00
#define FIRST_KOUHO	0x01	   /* kanji����Ƭ����Τ� */
#define NUMBER_KOUHO	0x02	   /* kanji�ϸ�������Υݥ��� */
} RkcBun ;			   /* ���ξ�硢curcand��0,maxcand��1 */


/*
 *  ���饤����ȥ���ƥ����ȥ쥳�ݥ�
 *
 */
typedef struct _RkcContext {
    short	    server ;  /* ���ݥС�����ƥ������ֹ� */
    short	    client ;  /* ���饤����ȡ�����ƥ������ֹ� */
    RkcBun	    *bun   ;  /* ʸ�����쥳��������ؤΥݥ��� */
    unsigned short *Fkouho ; /* ��������ؤΥݥ��� */
    short	    curbun ;  /* ������ʸ���ֹ� */
    short	    maxbun ;  /* ʸ����� */
    short	    bgnflag ; /* RkBgnBun�Υե饰 */
    unsigned short *lastyomi;
    short	    maxyomi;
} RkcContext ;

extern int ushort2euc(), euc2ushort(), ushort2wchar(), wchar2ushort(),
    wcharstrlen(), ushortstrlen(), ushortstrcpy() ;

typedef long (*initialize_t) pro((char *));
typedef int (*finalize_t) pro((void));
typedef int (*close_context_t) pro((RkcContext *));
typedef int (*create_context_t) pro((void));
typedef int (*duplicate_context_t) pro((RkcContext *));
typedef int (*dictionary_list_t) pro((RkcContext *, char *, int));
typedef int (*define_dic_t) pro((RkcContext *, char *, Ushort *));
typedef int (*delete_dic_t) pro((RkcContext *, char *, Ushort *));
typedef int (*mount_dictionary_t) pro((RkcContext *, char *, int));
typedef int (*remount_dictionary_t) pro((RkcContext *, char *, int));
typedef int (*umount_dictionary_t) pro((RkcContext *, char *));
typedef int (*mount_list_t) pro((RkcContext *, char *, int));
typedef int (*convert_t) pro((RkcContext *, Ushort *, int, int));
typedef int (*convert_end_t) pro((RkcContext *, int));
typedef int (*get_kanji_list_t) pro((RkcContext *));
typedef int (*get_stat_t) pro((RkcContext *, RkStat *));
typedef int (*resize_t) pro((RkcContext *, int));
typedef int (*store_yomi_t) pro((RkcContext *, Ushort *, int));
typedef int (*get_yomi_t) pro((RkcContext *, Ushort *));
typedef int (*get_lex_t) pro((RkcContext *, int, RkLex *));
typedef int (*autoconv_t) pro((RkcContext *, int, int));
typedef int (*subst_yomi_t) pro((RkcContext *, int, int, int, Ushort *, int));
typedef int (*flush_yomi_t) pro((RkcContext *));
typedef int (*get_last_yomi_t) pro((RkcContext *, Ushort *, int));
typedef int (*remove_bun_t) pro((RkcContext *, int));
typedef int (*get_simple_kanji_t)
    pro((RkcContext *, char *, Ushort *, int, Ushort *, int, Ushort *, int));
typedef int (*query_dic_t)
    pro((RkcContext *, char *, char *, struct DicInfo *));
typedef int (*get_hinshi_t) pro((RkcContext *, Ushort *, int));
typedef int (*store_range_t) pro((RkcContext *, Ushort *, int));
typedef int (*set_locale_t) pro((RkcContext *, char *));
typedef int (*set_app_name_t) pro((RkcContext *, char *));
typedef int (*notice_group_name_t) pro((RkcContext *, char *));
typedef int (*through_t) pro((RkcContext *, int, char *, int, int));
typedef int (*killserver_t) pro((void));
#ifdef EXTENSION
typedef int (*list_dictionary_t) pro((RkcContext *, char *, char *, int));
typedef int (*create_dictionary_t) pro((RkcContext *, char *, int));
typedef int (*remove_dictionary_t) pro((RkcContext *, char *, int));
typedef int (*rename_dictionary_t) pro((RkcContext *, char *, char *, int));
typedef int (*get_text_dictionary_t)
     pro((RkcContext *, char *, char *, Ushort *, int));
typedef int (*sync_t) pro((RkcContext *, char *));
typedef int (*chmod_dic_t) pro((RkcContext *, char *, int));
typedef int (*copy_dictionary_t)
     pro((RkcContext *, char *, char *, char *, int));
#endif

struct rkcproto {
  initialize_t initialize;
  finalize_t finalize;
  close_context_t close_context;
  create_context_t create_context;
  duplicate_context_t duplicate_context;
  dictionary_list_t dictionary_list;
  define_dic_t define_dic;
  delete_dic_t delete_dic;
  mount_dictionary_t mount_dictionary;
  remount_dictionary_t remount_dictionary;
  umount_dictionary_t umount_dictionary;
  mount_list_t mount_list;
  convert_t convert;
  convert_end_t convert_end;
  get_kanji_list_t get_kanji_list;
  get_stat_t get_stat;
  resize_t resize;
  store_yomi_t store_yomi;
  get_yomi_t get_yomi;
  get_lex_t get_lex;
  autoconv_t autoconv;
  subst_yomi_t subst_yomi;
  flush_yomi_t flush_yomi;
  get_last_yomi_t get_last_yomi;
  remove_bun_t remove_bun;
  get_simple_kanji_t get_simple_kanji;
  query_dic_t query_dic;
  get_hinshi_t get_hinshi;
  store_range_t store_range;
  set_locale_t set_locale;
  set_app_name_t set_app_name;
  notice_group_name_t notice_group_name;
  through_t through;
  killserver_t killserver;
#ifdef EXTENSION
  list_dictionary_t list_dictionary;
  create_dictionary_t create_dictionary;
  remove_dictionary_t remove_dictionary;
  rename_dictionary_t rename_dictionary;
  get_text_dictionary_t get_text_dictionary;
  sync_t sync;
  chmod_dic_t chmod_dic;
  copy_dictionary_t copy_dictionary;
#endif /* EXTENSION */
};

/* BASIC TYPE:
 *	subete no data ha MSB first(Motorolla order) de tenkai sareru
 *		unsigned char	w
 *		unsigned short	wx
 *		unsigned long	wxyz
 */	
#define LOMASK(x)	((x)&255)
#define	LTOL4(l, l4)	{\
	(l4)[0] = LOMASK((l)>>24); (l4)[1] = LOMASK((l)>>16);\
	(l4)[2] = LOMASK((l)>> 8); (l4)[3] = LOMASK((l));\
}
#define	LTOL3(l, l3)	{\
(l3)[0] = LOMASK((l)>>16); (l3)[1] = LOMASK((l)>> 8); (l3)[2] = LOMASK((l));\
}
#define	STOS2(s, s2)	{\
	(s2)[0] = LOMASK((s)>> 8); (s2)[1] = LOMASK((s));\
}

#define RK_LINE_BMAX 1024 /* ����� RKintern.h �Τ�Ʊ���ͤǤʤ���Фʤ�ʤ� */

#if 0
#define I16toI32(x) (((x) & 0x8000) ? ((x) | 0xffff8000) : (x))
#endif
#define I16toI32(x) (x)
#define I8toI32(x) (((x) & 0x80) ? ((x) | 0xffffff80) : (x))

#ifndef YES
#define YES 1
#endif
#ifndef NO
#define NO  0
#endif

#define SIZEOFSHORT 2 /* for protocol */
#define SIZEOFLONG  4 /* for protocol */

#define MAX_CX 100

typedef struct {
  char *uname;        /* user name */
  char *gname;        /* group name */
  char *topdir;       /* install dir */
} RkUserInfo;

/* function prototypes .. */

extern rkc_Connect_Iroha_Server pro((char *));
