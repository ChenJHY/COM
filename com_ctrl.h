/**
 * \file    com_ctrl.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning ��Ȩ����, ��Ȩ�ؾ�
 * \todo    COMģ�����
 */
#ifndef __COM_CTRL_H_
#define __COM_CTRL_H_

#include "com_type.h"

#define  COM_COM_CLIENT_CREATE                  2222
#define  COM_COM_CLIENT_SEND                    2223

#define  DEFAULT_LISTEN_CFG_TCP_PORT            9900            /* Ĭ�ϼ�������TCP�˿� */
#define  DEFAULT_LISTEN_MSG_TCP_PORT            9902            /* Ĭ�ϼ�����ϢTCP�˿� */
#define  DEFAULT_LISTEN_CFG_UDP_PORT            9901            /* Ĭ�ϼ�������UDP�˿� */

#define  DEFAULT_CFGSERVER_IP                   "192.168.0.118" /* ���÷�����Ĭ��IP��ַ */
#define  DEFAULT_CFGSERVER_TCP_PORT             9900            /* ���÷�����Ĭ��TCP�����˿� */
#define  DEFAULT_CFGSERVER_UDP_PORT             9901            /* ���÷�����Ĭ��UDP�����˿� */

#define  DEFAULT_NETADMIN_IP                    "192.168.0.118" /* ���ܷ�����Ĭ��IP��ַ */
#define  DEFAULT_NETADMIN_TCP_PORT              9902            /* ���ܷ�����Ĭ��TCP�����˿� */
#define  DEFAULT_NETADMIN_UDP_PORT              9901            /* ���ܷ�����Ĭ��UDP�����˿� */

#define  DEFAULT_FACE_SERVER_IP                 "192.168.0.118" /* ����ʶ������ַ */
#define  DEFAULT_FACE_SERVER_TCP_PORT           9910			/* ����ʶ��TCP�˿� */

#define  DEFAULT_FINGER_SERVER_IP               "192.168.0.118" /* ָ��ʶ������ַ */
#define  DEFAULT_FINGER_SERVER_TCP_PORT         9910			/* ָ��ʶ��TCP�˿� */

#define  DEFAULT_LIFT_IP                        "192.168.0.118" /* ���ݿ�����Ĭ��IP��ַ */
#define  DEFAULT_LIFT_TCP_PORT                  9902            /* ���ݿ�����Ĭ��TCP�����˿� */

#define  DEFAULT_SERVER_IP                      "\0"            /* Ԥ��������Ĭ��IP��ַ */
#define  DEFAULT_SERVER_TCP_PORT                0               /* Ԥ��������Ĭ��TCP�����˿� */
#define  DEFAULT_SERVER_UDP_PORT                0               /* Ԥ��������Ĭ��UDP�����˿� */

#define  DEFAULT_HEARTBEAT_PERIOD               10000           /* Ĭ��COM�������� */
#define  DEFAULT_UcCode                         0               /* Ĭ��UC�� */
#define  DEFAULT_FcCode                         0               /* Ĭ��FC�� */
#define  DEFAULT_DoorNum                        0               /* Ĭ���ź� */

#define  DEFAULT_dd                             ""
#define  DEFAULT_bbb                            ""
#define  DEFAULT_rr                             ""
#define  DEFAULT_ff                             ""
#define  DEFAULT_ii                             ""
#define  DEFAULT_nn                             ""
#define  DEFAULT_aa                             ""

#define  DEFAULT_AreaCode                       ""
#define  DEFAULT_CommunityCode                  ""

#define AM_BUFF_MAX                             2048
#define	THREAD_FEED_INTERVAL_TIME				3000


/**
 * \enum  enCOMState
 * \brief COMģ��״̬
 */
typedef enum tagCOMState
{
    emComStateUnInit  = 0x01, /* ģ��δ��ʼ�� */
    emComStateStop    = 0x02, /* ģ��ֹͣ */
    emComStateWorking = 0x04, /* ģ�������� */
}enCOMState;


typedef struct TagAMDataBuffDef
{
    SdUInt      uiDataLen;
    SdUChar     szBuffer[AM_BUFF_MAX];
}AMDataBuffDef,*LPAMDataBuffDef;

/**
 * \struct COMManage
 * \brief  COMģ�����
 */
typedef struct tagCOMManage
{
    enCOMState  emCOMState;          /* ģ������״̬ */
    COMCfgDef   stCOMCfg;            /* COMģ���ʼ������ */
    UtTimer     utTimerDogFeed;      /* UT��ʱ�����, ι�� */
    UtTimer     utTimerLogOperCheck; /* UT��ʱ�����, ��־������� */
	UtTimer		utTimerThreadFeed;	/* ���̸߳����߳�ι����ʱ�� */
    SdInt       iMsgQueue;           /* ��־������Ϣ���� */
    SdUInt      pack_id;             /* ���� */
    AMDataBuffDef stAMDataBuff;
	SdInt		iFingerTimer;		/*ָ������ƥ����*/
	time_t		t_start;			/*���̸߳����߳�ι����ʱ����*/
}COMManage, *LPCOMManage;

/**
 * \fn      LPCOMManage com_ctrl_module_manage_get(void)
 * \brief   ��ȡģ�����ָ��
 * \param
 * \return  LPCOMManage
 * \note
 * \todo
 * \version V1.0
 * \warning
*/
LPCOMManage com_ctrl_module_manage_get(void);

#endif /* __COM_CTRL_H_ */
