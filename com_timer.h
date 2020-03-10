/**
 * \file    com_timer.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning ��Ȩ����, ��Ȩ�ؾ�
 * \todo    COM�����ʱ��
 */
#ifndef __COM_TIMER_H_
#define __COM_TIMER_H_

#include "com_type.h"

#define  COM_TIMER_UNIT          1000 /* com_timer��ʱ����Сʱ��, ��λ: milli */
#define  TIMER_NONE              -1   /* �޶�ʱ�� */

typedef s32 TIMER_HANDLE; /* ��ʱ��������� */
typedef s32 TIMEVAL;      /* ��ʱ��ʱ������ */

typedef void(*com_timer_callback)(void *ptr); /* ��ʱ���ص��������� */

/************************************************************************
* enum  em_timer_type
*        ��ʱ������
************************************************************************/
typedef enum enum_timer_type
{
    timer_client_resend_em = 1, /* TCP�ͻ����ط� */
    timer_unlock_ack_em,        /* ����Ӧ�� */
    emTimerLiftCallAck,
    emDufault,
}em_timer_type;

/************************************************************************
* struct  st_unlock_para
*        ��������
************************************************************************/
typedef struct struct_unlock_para{
    s32             sock;           /* Socket��� */
    s32             uc;             /* �Ự�Է�UC */
    s32             pack_id;        /* ���� */
    enUnlockTypeDef unlock_type_em; /* �������� */
    em_json_ack     json_ack_em;    /* ������� */
}st_unlock_para, *pst_unlock_para;

typedef struct TagLiftParamDef
{
    SdInt iSock;
    SdInt iUc;
    SdUInt uiPackID;
    em_json_ack emResult;
}LiftParamDef,*LPLiftParamDef;

/************************************************************************
* @FunctionName( ������ ): TIMER_HANDLE com_timer_set(com_timer_callback callback, com_timer_callback destructor, void *parg,
                            TIMEVAL val, TIMEVAL interval, u32 times, em_timer_type timer_type_em)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ���붨ʱ��
*
* @Param(����):
*       callback, destructor, *parg - �ص�����, ��������, ����
*       val, interval - ��һ�ζ�ʱʱ��, ���ڶ�ʱʱ��
*       times - ��ʱ��ִ�д���
*       timer_type_em - ��ʱ������
*
* @ReturnCode(����ֵ):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
TIMER_HANDLE com_timer_set(com_timer_callback callback, com_timer_callback destructor, void *parg,
    TIMEVAL val, TIMEVAL interval, u32 times, em_timer_type timer_type_em);
/**@END! TIMER_HANDLE com_timer_set(com_timer_callback callback, com_timer_callback destructor, void *parg,
            TIMEVAL val, TIMEVAL interval, u32 times, em_timer_type timer_type_em) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_timer_client_resend_remove(u32 pack_id)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ɾ��TCP�ͻ����ط���ʱ��
*
* @Param(����):
*       pack_id - ����
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�
*       -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_timer_client_resend_remove(u32 pack_id);
/**@END! s32 com_timer_client_resend_remove(u32 pack_id) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_timer_unlock_ack_exist(void *ptr)
* @CreateDate  (��������): 2015/11/28
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����ACK�Ƿ����
*
* @Param(����):
*       pst_unlock_para
*
* @ReturnCode(����ֵ):
*        0 - ����
*       -1 - ������
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_timer_unlock_ack_exist(void *ptr);
/**@END! s32 com_timer_unlock_ack_exist(void *ptr) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_timer_unlock_change(enDoorStateDef door_state_em)
* @CreateDate  (��������): 2015/11/28
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����ACK��Ϊ�ɹ�
*
* @Param(����):
*       door_state_em -��״̬
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�
*       -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_timer_unlock_change(enDoorStateDef door_state_em);
/**@END! s32 com_timer_unlock_change(enDoorStateDef door_state_em) !\(^o^)/~ ������ */

/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( ������ ): s32 com_timer_init(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ��ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_timer_init(void *parg);
/**@END! s32 com_timer_init(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_timer_start(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_timer_start(void *parg);
/**@END! s32 com_timer_start(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_timer_stop(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ֹͣ
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_timer_stop(void *parg);
/**@END! s32 com_timer_stop(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_timer_uninit(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_timer_uninit(void *parg);
/**@END! s32 com_timer_uninit(void *parg) !\(^o^)/~ ������ */

#endif /* __COM_TIMER_H_ */
