/**
 * \file    com_timer.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning 版权所有, 侵权必究
 * \todo    COM软件定时器
 */
#ifndef __COM_TIMER_H_
#define __COM_TIMER_H_

#include "com_type.h"

#define  COM_TIMER_UNIT          1000 /* com_timer定时器最小时间, 单位: milli */
#define  TIMER_NONE              -1   /* 无定时器 */

typedef s32 TIMER_HANDLE; /* 定时器句柄类型 */
typedef s32 TIMEVAL;      /* 定时器时间类型 */

typedef void(*com_timer_callback)(void *ptr); /* 定时器回调函数类型 */

/************************************************************************
* enum  em_timer_type
*        定时器类型
************************************************************************/
typedef enum enum_timer_type
{
    timer_client_resend_em = 1, /* TCP客户端重发 */
    timer_unlock_ack_em,        /* 开锁应答 */
    emTimerLiftCallAck,
    emDufault,
}em_timer_type;

/************************************************************************
* struct  st_unlock_para
*        开锁参数
************************************************************************/
typedef struct struct_unlock_para{
    s32             sock;           /* Socket句柄 */
    s32             uc;             /* 会话对方UC */
    s32             pack_id;        /* 包号 */
    enUnlockTypeDef unlock_type_em; /* 开锁类型 */
    em_json_ack     json_ack_em;    /* 开锁结果 */
}st_unlock_para, *pst_unlock_para;

typedef struct TagLiftParamDef
{
    SdInt iSock;
    SdInt iUc;
    SdUInt uiPackID;
    em_json_ack emResult;
}LiftParamDef,*LPLiftParamDef;

/************************************************************************
* @FunctionName( 函数名 ): TIMER_HANDLE com_timer_set(com_timer_callback callback, com_timer_callback destructor, void *parg,
                            TIMEVAL val, TIMEVAL interval, u32 times, em_timer_type timer_type_em)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 申请定时器
*
* @Param(参数):
*       callback, destructor, *parg - 回调函数, 析构函数, 参数
*       val, interval - 第一次定时时间, 周期定时时间
*       times - 定时器执行次数
*       timer_type_em - 定时器类型
*
* @ReturnCode(返回值):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
TIMER_HANDLE com_timer_set(com_timer_callback callback, com_timer_callback destructor, void *parg,
    TIMEVAL val, TIMEVAL interval, u32 times, em_timer_type timer_type_em);
/**@END! TIMER_HANDLE com_timer_set(com_timer_callback callback, com_timer_callback destructor, void *parg,
            TIMEVAL val, TIMEVAL interval, u32 times, em_timer_type timer_type_em) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_timer_client_resend_remove(u32 pack_id)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 删除TCP客户端重发定时器
*
* @Param(参数):
*       pack_id - 包号
*
* @ReturnCode(返回值):
*        0 - 成功
*       -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_timer_client_resend_remove(u32 pack_id);
/**@END! s32 com_timer_client_resend_remove(u32 pack_id) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_timer_unlock_ack_exist(void *ptr)
* @CreateDate  (创建日期): 2015/11/28
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 开锁ACK是否存在
*
* @Param(参数):
*       pst_unlock_para
*
* @ReturnCode(返回值):
*        0 - 存在
*       -1 - 不存在
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_timer_unlock_ack_exist(void *ptr);
/**@END! s32 com_timer_unlock_ack_exist(void *ptr) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_timer_unlock_change(enDoorStateDef door_state_em)
* @CreateDate  (创建日期): 2015/11/28
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 开锁ACK改为成功
*
* @Param(参数):
*       door_state_em -门状态
*
* @ReturnCode(返回值):
*        0 - 成功
*       -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_timer_unlock_change(enDoorStateDef door_state_em);
/**@END! s32 com_timer_unlock_change(enDoorStateDef door_state_em) !\(^o^)/~ 结束咯 */

/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_timer_init(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_timer_init(void *parg);
/**@END! s32 com_timer_init(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_timer_start(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 启动
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_timer_start(void *parg);
/**@END! s32 com_timer_start(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_timer_stop(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 停止
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_timer_stop(void *parg);
/**@END! s32 com_timer_stop(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_timer_uninit(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 反初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_timer_uninit(void *parg);
/**@END! s32 com_timer_uninit(void *parg) !\(^o^)/~ 结束咯 */

#endif /* __COM_TIMER_H_ */
