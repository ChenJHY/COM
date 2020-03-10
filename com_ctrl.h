/**
 * \file    com_ctrl.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning 版权所有, 侵权必究
 * \todo    COM模块管理
 */
#ifndef __COM_CTRL_H_
#define __COM_CTRL_H_

#include "com_type.h"

#define  COM_COM_CLIENT_CREATE                  2222
#define  COM_COM_CLIENT_SEND                    2223

#define  DEFAULT_LISTEN_CFG_TCP_PORT            9900            /* 默认监听配置TCP端口 */
#define  DEFAULT_LISTEN_MSG_TCP_PORT            9902            /* 默认监听信息TCP端口 */
#define  DEFAULT_LISTEN_CFG_UDP_PORT            9901            /* 默认监听配置UDP端口 */

#define  DEFAULT_CFGSERVER_IP                   "192.168.0.118" /* 配置服务器默认IP地址 */
#define  DEFAULT_CFGSERVER_TCP_PORT             9900            /* 配置服务器默认TCP监听端口 */
#define  DEFAULT_CFGSERVER_UDP_PORT             9901            /* 配置服务器默认UDP监听端口 */

#define  DEFAULT_NETADMIN_IP                    "192.168.0.118" /* 网管服务器默认IP地址 */
#define  DEFAULT_NETADMIN_TCP_PORT              9902            /* 网管服务器默认TCP监听端口 */
#define  DEFAULT_NETADMIN_UDP_PORT              9901            /* 网管服务器默认UDP监听端口 */

#define  DEFAULT_FACE_SERVER_IP                 "192.168.0.118" /* 人脸识别服务地址 */
#define  DEFAULT_FACE_SERVER_TCP_PORT           9910			/* 人脸识别TCP端口 */

#define  DEFAULT_FINGER_SERVER_IP               "192.168.0.118" /* 指纹识别服务地址 */
#define  DEFAULT_FINGER_SERVER_TCP_PORT         9910			/* 指纹识别TCP端口 */

#define  DEFAULT_LIFT_IP                        "192.168.0.118" /* 电梯控制器默认IP地址 */
#define  DEFAULT_LIFT_TCP_PORT                  9902            /* 电梯控制器默认TCP监听端口 */

#define  DEFAULT_SERVER_IP                      "\0"            /* 预留服务器默认IP地址 */
#define  DEFAULT_SERVER_TCP_PORT                0               /* 预留服务器默认TCP监听端口 */
#define  DEFAULT_SERVER_UDP_PORT                0               /* 预留服务器默认UDP监听端口 */

#define  DEFAULT_HEARTBEAT_PERIOD               10000           /* 默认COM心跳周期 */
#define  DEFAULT_UcCode                         0               /* 默认UC号 */
#define  DEFAULT_FcCode                         0               /* 默认FC号 */
#define  DEFAULT_DoorNum                        0               /* 默认门号 */

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
 * \brief COM模块状态
 */
typedef enum tagCOMState
{
    emComStateUnInit  = 0x01, /* 模块未初始化 */
    emComStateStop    = 0x02, /* 模块停止 */
    emComStateWorking = 0x04, /* 模块运行中 */
}enCOMState;


typedef struct TagAMDataBuffDef
{
    SdUInt      uiDataLen;
    SdUChar     szBuffer[AM_BUFF_MAX];
}AMDataBuffDef,*LPAMDataBuffDef;

/**
 * \struct COMManage
 * \brief  COM模块管理
 */
typedef struct tagCOMManage
{
    enCOMState  emCOMState;          /* 模块运行状态 */
    COMCfgDef   stCOMCfg;            /* COM模块初始化配置 */
    UtTimer     utTimerDogFeed;      /* UT定时器句柄, 喂狗 */
    UtTimer     utTimerLogOperCheck; /* UT定时器句柄, 日志操作检测 */
	UtTimer		utTimerThreadFeed;	/* 子线程给主线程喂狗定时器 */
    SdInt       iMsgQueue;           /* 日志操作消息队列 */
    SdUInt      pack_id;             /* 包号 */
    AMDataBuffDef stAMDataBuff;
	SdInt		iFingerTimer;		/*指纹网络匹配句柄*/
	time_t		t_start;			/*子线程给主线程喂狗的时间间隔*/
}COMManage, *LPCOMManage;

/**
 * \fn      LPCOMManage com_ctrl_module_manage_get(void)
 * \brief   获取模块管理指针
 * \param
 * \return  LPCOMManage
 * \note
 * \todo
 * \version V1.0
 * \warning
*/
LPCOMManage com_ctrl_module_manage_get(void);

#endif /* __COM_CTRL_H_ */
