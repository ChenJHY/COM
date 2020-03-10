/**
 * \file    com_client.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning ��Ȩ����, ��Ȩ�ؾ�
 * \todo    TCP�ͻ���
 */
#ifndef __COM_CLIENT_H_
#define __COM_CLIENT_H_

#include "com_type.h"

/************************************************************************
* @FunctionName( ������ ): s32 com_client_net_msg_del_by_pack_id(const u32 cpack_id, TIMER_HANDLE *ptimer_handle)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ͨ������ɾ��������Ϣ
*
* @Param(����):
*        pack_id - ����
*       *timer_handle - ���ض�ʱ�����
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�
*       -1 - ʧ��(pack_id������)
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_client_net_msg_del_by_pack_id(const u32 cpack_id, TIMER_HANDLE *ptimer_handle);
/**@END! s32 com_client_net_msg_del_by_pack_id(const u32 cpack_id, TIMER_HANDLE *ptimer_handle) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_client_tcp_server_sendto(s8 *pip, u16 port, const u32 pack_id, s8 *pbuf, u32 len)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����������Ϣ����Ϣ������
*
* @Param(����):
*       *pip - TCP������IP
*        port - TCP�������˿�
*        pack_id - ����(�ǳ���Ҫ)
*        pbuf - ���ݻ�����(������malloc����)
*        len - ���ݳ���
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�
*       -1 - ʧ��(�ͷ����ݻ�������Դ)
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_client_tcp_server_sendto(s8 *pip, u16 port, const u32 pack_id, s8 *pbuf, u32 len);
/**@END! s32 com_client_tcp_server_sendto(s8 *pip, u16 port, const u32 pack_id, s8 *pbuf, u32 len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_client_msg_server_sendto(const u32 pack_id, s8 *pbuf, u32 len)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����������Ϣ����Ϣ������
*
* @Param(����):
*        pack_id - ����(�ǳ���Ҫ)
*        pbuf - ���ݻ�����(������malloc����)
*        len - ���ݳ���
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�
*       -1 - ʧ��(�ͷ����ݻ�������Դ)
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_client_msg_server_sendto(const u32 pack_id, s8 *pbuf, u32 len);
/**@END! s32 com_client_msg_server_sendto(const u32 pack_id, s8 *pbuf, u32 len) !\(^o^)/~ ������ */

/************************************************************************
*����ʶ����������������
************************************************************************/

s32 com_client_msg_face_server_sendto(const u32 pack_id, s8 *pbuf, u32 len);


/************************************************************************
*ָ��ʶ����������������
************************************************************************/
s32 com_client_msg_finger_server_sendto(const u32 pack_id, s8 *pbuf, u32 len);



/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( ������ ): s32 com_client_init(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ��ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_client_init(void *parg);
/**@END! s32 com_client_init(void *arg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_client_start(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_client_start(void *parg);
/**@END! int com_client_start(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_client_stop(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ֹͣ
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_client_stop(void *parg);
/**@END! s32 com_client_stop(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_client_uninit(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_client_uninit(void *parg);
/**@END! static s32 com_client_uninit(void *arg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): int com_client_cache_am_data(const u32 pack_id, s8 *pbuf, u32 len)
* @Description(����):      ��δ���͵��Ž����ݻ�������
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
int com_client_cache_am_data(const u32 pack_id, s8 *pbuf, u32 len);

/************************************************************************
* @FunctionName( ������ ): int com_client_cache_am_data(const u32 pack_id, s8 *pbuf, u32 len)
* @Description(����):      ��δ���͵��Ž����ݻ�������
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
s32 com_remove_am_cache_data_to_db(u32 pack_id);

/************************************************************************
* @FunctionName( ������ ): int com_send_am_data_from_am_db_cache_timer()
* @Description(����):      ��δ���͵��Ž����ݻ�������
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
void* com_send_am_data_from_am_db_cache_timer();



#endif /* __COM_CLIENT_H_ */
