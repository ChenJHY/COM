/************************************************************************
 * @FileName   ( �ļ��� ) : com_type.h
 * @CreateDate (��������) : 2015/11/15
 * @Author     ( ��  �� ) : CJH
 * @Description( ��  �� ) : COM���Ͷ���
 * @Connect    (��ϵ��ʽ) : Tel(�绰):(+86)18745950097, ��ϵ��:������
 * @Instruction( ˵  �� ) :
************************************************************************/
#ifndef __COM_TYPE_H_
#define __COM_TYPE_H_

#include <stdlib.h> /* malloc, free */
#include <stdio.h> /* printf */
#include <string.h> /* str, mem */
#include <stdarg.h> /* va_list */

#include <pthread.h> /* pthread_mutex_t */
#include <sys/socket.h>

//#define     _IO               volatile          /* ��ֹ�������Ż�����, �ɶ�д */
#define     _WR               volatile          /* ��ֹ�������Ż�����, �ɶ�д */
#define     _R                volatile const    /* ���ñ������Ż�����, ֻ��   */

typedef     unsigned char     u8;               /* �޷���, 8BIT , ��������    */
typedef     unsigned short    u16;              /* �޷���, 16BIT, ��������    */
typedef     unsigned int      u32;              /* �޷���, 32BIT, ��������    */
typedef     signed   char     s8;               /* �з���, 8BIT , ��������    */
typedef     signed   short    s16;              /* �з���, 16BIT, ��������    */
typedef     signed   int      s32;              /* �з���, 32BIT, ��������    */

#ifndef NULL
#define  NULL             ((void *)0)           /* ��ָ�� */
#endif

//#define  bool                 u8
//#define  true                 1
//#define  false                (!true)

#endif
