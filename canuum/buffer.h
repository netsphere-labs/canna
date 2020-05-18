/*
 *  buffer.h,v 1.3 2001/06/14 18:16:06 ura Exp
 *  Canna: $Id: buffer.h,v 1.2 2003/01/04 07:31:02 aida_s Exp $
 */

/*
 * FreeWnn is a network-extensible Kana-to-Kanji conversion system.
 * This file is part of FreeWnn.
 * 
 * Copyright Kyoto University Research Institute for Mathematical Sciences
 *                 1987, 1988, 1989, 1990, 1991, 1992
 * Copyright OMRON Corporation. 1987, 1988, 1989, 1990, 1991, 1992, 1999
 * Copyright ASTEC, Inc. 1987, 1988, 1989, 1990, 1991, 1992
 * Copyright FreeWnn Project 1999, 2000
 *
 * Maintainer:  FreeWnn Project   <freewnn@tomo.gr.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* buffer��ɽ���Τ������ϡ�hanten�ˤ�äƷ��ꤵ��롣
ȿž���뤫���������֤ˤ��뤫�����Ω�ˡ�
�Хåե�������ǡ�ʸƬ����mark�ޤǡ�mark����cursor�ޤǡ�
cursor����buffer�κǸ�ޤǤξ��֤���Ω�����ꤹ�뤳�Ȥ��Ǥ��롣
         when mark is set
                hanten bit0(0x01),bit1(0x02):      --- mark
                bit2(0x04),bit3(0x08):        mark --- cursor
                bit4(0x10),bit5(0x20):      cursor --- 
          when mark is not set(-1)
                bit4,bit5:      
                bit6(0x40):     t_b_st --- t_b_end �ܡ����ɽ��
    first one....hanten
    second one...under_line
*/
struct buf
{
  w_char *buffer;               /* ��Ͽ�ѥХåե� */
  char *buf_msg; /** ��̾�����Ϥ�����ΥХåե���������ɽ������ʸ���� */
  int start_col; /** ��̾���ϻ��β��̾�Υ����Υ������Ȱ��� */
  int t_c_p; /** ��̾�����ѥХåե�����ǤΥ���������� */
  int hanten; /** ��̾�����ѥХåե�����ȿžɽ�������Ƥ��뤫�ɤ��� */
  int maxlen; /**��̾�����ѥХåե�������ˤϤ��äƤ���ʸ���� */
  int buflen; /** length of the buffer */
  int t_m_start;                /* -1 : mark�����ꤵ��Ƥ��ʤ��� */
  /* 0 �ʾ� : �ޡ����ΰ��� */
  int t_b_st;   /**�ܡ����ɽ���γ��ϰ��� */
  int t_b_end;  /**�ܡ����ɽ���ν�λ���� */
  int vlen;                     /*�Хåե�����ǥ����꡼���ɽ��������� */
  int duplicate;                /* �����꡼�󤬥������뤹����ˡ���ʣ����ɽ������ʸ���� */

  int vst;                      /* buffer����ǲ��̤�ɽ������Ƥ���ǽ�ΰ��� */
  /*����ϡ�����˾�Υ롼����Ǥ�����ʤ�����˾�ޤ����� */
  int (*key_in_fun) ();         /*�����˴ؿ������ꤵ��Ƥ���ȡ�key_table�˥Х���ɤ�
                                   �ʤ�ʸ�������Ϥ�
                                   ���������˥Хåե����ˤ���ʸ�������줺������ˤ��δؿ����ƤФ�� */
  int (*redraw_fun) ();         /* romkan kara redraw ga kaette kitatoki */
  int (*ctrl_code_fun) ();      /*�����˴ؿ������ꤵ��Ƥ���ȡ�key_table�˥Х���ɤ�
                                   �ʤ�����ȥ���ʸ�������Ϥ���������˥٥���Ĥ餹�����
                                   ���δؿ����ƤФ�� */
  int (**key_table) ();         /*�����Х���ɤΥơ��֥� */
  char *rk_clear_tbl;           /* romakn_clear �� hituyouka */
};


extern int *bunsetsu;
extern int *bunsetsuend;
extern struct wnn_env **bunsetsu_env;   /* ʸ����Ѵ��˻Ȥä� env */
extern int *touroku_bnst;
/* extern int bunsetsucnt; */
extern int touroku_bnst_cnt;

extern struct buf *c_b;         /* �����Ѥ����Ƥ���buf�򼨤������Х��ѿ� */
extern w_char *kill_buffer;     /* ����Хåե����礭���ϡ�maxchg�ȤʤäƤ��� */
extern int kill_buffer_offset;  /* ����Хåե����Ȥ��Ƥ���Ĺ�� */
extern w_char *remember_buf;    /* ����Х��Хåե��礭���ϡ�maxchg�ȤʤäƤ��� */
