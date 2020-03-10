/**
 * \file    com_json.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning ��Ȩ����, ��Ȩ�ؾ�
 * \todo    COMģ��JSON���, ���
 */
#ifndef __COM_JSON_H_
#define __COM_JSON_H_

#include "json/json.h"
#include "com_type.h"



/************************************************************************
* enum  em_json_ack
*        JSON��ACK
************************************************************************/
typedef enum enum_json_ack{
    ack_false_em = 0x00,/* ʧ�� */
    ack_true_em,        /* �ɹ� */
    ack_no_em,          /* ��ACK */
}em_json_ack;

typedef s32(*com_json_callback)(json_object *, void *, va_list); /* JSON����ص��������� */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_unpack(const s32 sock, const s8 *pbuf_s8, const u32 len)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ������Ϣ���
*
* @Param(����):
*        sock - SOCK���
*       *pbuf_s8 - ����(��ȥ����ͷ)
*        len - ���ݳ���
*
* @ReturnCode(����ֵ):
*       -1 - ʧ��
*       ���� - ACK����������ֵ
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_json_unpack(const s32 sock, const s8 *pbuf_s8, const u32 len);
/**@END! s32 com_json_unpack(const s32 sock, const s8 *pbuf_s8, const u32 len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_Message_NoteBrowse(void *ptr, int len)
* @Description(����):      JSON���������������Ϣ: ��Ϣ���
* @Param(����): LPReadAreaInfoDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
s32 com_json_Message_NoteBrowse(void *ptr, int len);
/**@END! s32 com_json_Message_NoteBrowse(void *ptr, int len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_VDP_UnlockEvent(void *ptr, int len)
* @Description(����):      JSON���������������Ϣ: �����¼�
* @Param(����): LPDoorRecordDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
s32 com_json_VDP_UnlockEvent(void *ptr, int len);
/**@END! s32 com_json_VDP_UnlockEvent(void *ptr, int len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_VDP_CallInfo(void *ptr, int len)
* @Description(����):      JSON���������������Ϣ: �����¼�
* @Param(����): LPCallEventReportDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
s32 com_json_VDP_CallInfo(void *ptr, int len);
/**@END! s32 com_json_VDP_CallInfo(void *ptr, int len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_ftp_ImgInfo(void *ptr, int len)
* @Description(����):      JSON���������ftp��Ϣ: ͼƬ��Ϣ
* @Param(����): LPCallEventReportDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
s32 com_json_ftp_ImgInfo(void *ptr, int len);
/**@END! s32 com_json_VDP_CallInfo(void *ptr, int len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_Access_Data(char *pdata, int len)
* @Description(����):      JSON���������������Ϣ: �Ž�����
* @Param(����): ����(תBase64����), ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
s32 com_json_Access_Data(char *pdata, int len);
/**@END! s32 com_json_Access_Data(char *pdata, int len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_Lift_Schedule(char *pdata, int len)
* @Description(����):      JSON���������������Ϣ: ���ȵ���
* @Param(����): LPRoomAddrDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
s32 com_json_Lift_Schedule(char *pdata, int len);
/**@END! s32 com_json_Lift_Schedule(char *pdata, int len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_Unlock_CheckPassword(char *pdata, int len)
* @Description(����):      JSON���������������Ϣ: Զ�˿���, У������
* @Param(����): LPRemotePasswdUnlockDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
s32 com_json_Unlock_CheckPassword(char *pdata, int len);
/**@END! s32 com_json_Unlock_CheckPassword(char *pdata, int len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_door_Alarm_Upload(char *pdata, int len)
* @Description(����):      JSON���������������Ϣ: Զ�˿���, У������
* @Param(����): LPEPAlarmInfoDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
s32 com_json_door_Alarm_Upload(char *pdata, int len);
/**@END! s32 com_json_door_Alarm_Upload(char *pdata, int len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_door_Alarm_Upload(char *pdata, int len)
* @Description(����):      JSON���������������Ϣ: Զ�˿���, У������
* @Param(����): LPEPAlarmInfoDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
s32 com_json_door_Alarm_Upload(char *pdata, int len);

/************************************************************************
* @FunctionName( ������ ): SdInt com_json_Face_RequestFaceSync(SdChar *pdata, SdInt len)
* @Description(����):      JSON���������������Ϣ:������������ͬ��
* @Param(����): LPRequestInfoDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
SdInt com_json_Face_RequestFaceSync(SdChar *pdata, SdInt len);

/************************************************************************
* @FunctionName( ������ ): SdInt com_json_Face_FaceSyncCheck(SdChar *pdata, SdInt len)
* @Description(����):      JSON���������������Ϣ:��������ͬ�����
* @Param(����): LPEPAlarmInfoDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
SdInt com_json_Face_FaceSyncCheck(SdChar *pdata, SdInt len);

/************************************************************************
* @FunctionName( ������ ): SdInt com_json_Face_FaceSyncAck(SdChar *pdata, SdInt len)
* @Description(����):      JSON���������������Ϣ:��������ͬ��Ack
* @Param(����): LPEPAlarmInfoDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
SdInt com_json_Face_FaceSyncAck(SdBool Result, const SdInt iSock);


/************************************************************************
* @FunctionName( ������ ): SdInt com_json_Finger_Char(SdChar *pdata, SdInt len)
* @Description(����):      JSON���������������Ϣ:ָ������ֵ����
* @Param(����): LPEPAlarmInfoDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
SdInt com_json_Finger_Char(SdChar *pdata, SdInt len);

/************************************************************************
* @FunctionName( ������ ): SdInt com_json_Finger_Sync_Request(SdChar *pdata, SdInt len)
* @Description(����):      JSON���������������Ϣ:ָ������ͬ������
* @Param(����): LPEPAlarmInfoDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
SdInt com_json_Finger_Sync_Request(SdChar *pdata, SdInt len);

/************************************************************************
* @FunctionName( ������ ): SdInt com_json_Finger_Sync_Check(SdChar *pdata, SdInt len)
* @Description(����):      JSON���������������Ϣ:ָ������ͬ�����
* @Param(����): LPEPAlarmInfoDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
SdInt com_json_Finger_Sync_Check(SdChar *pdata, SdInt len);

/************************************************************************
* @FunctionName( ������ ): SdInt com_json_Finger_Finger_Sync(SdChar *pdata, SdInt len)
* @Description(����):      JSON���������������Ϣ:ָ��ͬ������
* @Param(����): LPEPAlarmInfoDef, ���ݳ���
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
SdInt com_json_Finger_Finger_Sync(SdChar *pdata, SdInt len);





/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_init(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ��ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_json_init(void *parg);
/**@END! s32 com_json_init(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_start(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_json_start(void *parg);
/**@END! s32 com_json_start(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_stop(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ֹͣ
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_json_stop(void *parg);
/**@END! s32 com_json_stop(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_json_uninit(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_json_uninit(void *parg);
/**@END! s32 com_json_uninit(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static s32 com_json_to_net_msg(json_object *json, s8 **ppbuf)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): JSON -> ������Ϣ(��Ӱ�ͷ)
*
* @Param(����):
*        *json - JSON
*       **ppbuf - ����������Ϣ(malloc����)
*
* @ReturnCode(����ֵ):
*        -1 - ʧ��
*        >0 - ������Ϣ����
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_json_to_net_msg(json_object *json, s8 **ppbuf);

s32 com_json_liftstatus_info(LPLiftInfoDef ps_info, s8 ** jsonstr);

/************************************************************************
* @FunctionName( ������ ): int com_char_to_net_msg(char *pbuf, char **ppbuf)
* @Description(����):      ��δ���͵��Ž����ݻ�������
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
int com_char_to_net_msg(char *pbuf, char **ppbuf);


#endif /* __COM_JSON_H_ */
