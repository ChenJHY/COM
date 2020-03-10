/************************************************************************
 * @FileName   ( 文件名 ) : com_type.h
 * @CreateDate (创建日期) : 2015/11/15
 * @Author     ( 作  者 ) : CJH
 * @Description( 描  述 ) : COM类型定义
 * @Connect    (联系方式) : Tel(电话):(+86)18745950097, 联系人:王德宇
 * @Instruction( 说  明 ) :
************************************************************************/
#ifndef __COM_TYPE_H_
#define __COM_TYPE_H_

#include <stdlib.h> /* malloc, free */
#include <stdio.h> /* printf */
#include <string.h> /* str, mem */
#include <stdarg.h> /* va_list */

#include <pthread.h> /* pthread_mutex_t */
#include <sys/socket.h>

//#define     _IO               volatile          /* 防止编译器优化类型, 可读写 */
#define     _WR               volatile          /* 防止编译器优化类型, 可读写 */
#define     _R                volatile const    /* 放置编译器优化类型, 只读   */

typedef     unsigned char     u8;               /* 无符号, 8BIT , 变量类型    */
typedef     unsigned short    u16;              /* 无符号, 16BIT, 变量类型    */
typedef     unsigned int      u32;              /* 无符号, 32BIT, 变量类型    */
typedef     signed   char     s8;               /* 有符号, 8BIT , 变量类型    */
typedef     signed   short    s16;              /* 有符号, 16BIT, 变量类型    */
typedef     signed   int      s32;              /* 有符号, 32BIT, 变量类型    */

#ifndef NULL
#define  NULL             ((void *)0)           /* 空指针 */
#endif

//#define  bool                 u8
//#define  true                 1
//#define  false                (!true)

#endif
