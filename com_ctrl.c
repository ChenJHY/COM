/**
 * \file    com_ctrl.c
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning ��Ȩ����, ��Ȩ�ؾ�
 * \todo    COMģ�����
 */
#include "com_func.h"

static COMManage gs_stCOMManage = {
    .emCOMState = emComStateUnInit, /* COMģ������״̬ */
    .utTimerDogFeed = SD_NULL,      /* ι����ʱ����� */
    .utTimerLogOperCheck = SD_NULL, /* �����־������ʱ�� */
    .iMsgQueue = 0,                 /* ��־������Ϣ���� */
    .stAMDataBuff = {
                        0,
                        {0}
                    }
};

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
LPCOMManage com_ctrl_module_manage_get(void)
{
    return &gs_stCOMManage;
}

LPAMDataBuffDef com_ctrl_am_buff_get(void)
{
    LPCOMManage pComObject = com_ctrl_module_manage_get();
    if(pComObject != SD_NULL)
    {
        return &(pComObject->stAMDataBuff);
    }
    return SD_NULL;
}

/**
 * \fn      static void com_ctrl_log_oper_check(UtTimer utTimer, IN void *ptr)
 * \brief   UT��ʱ���ص�����, ��־�������
 * \param   UtTimer utTimer - ��ʱ�����
 * \param   IN void *ptr - ��������
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning ���뱣֤�����ĺϷ���
 */
static void com_ctrl_log_oper_check(UtTimer utTimer, IN void *ptr)
{
    const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();

    SdInt iRet = 0;
    MsgDef stMsg;
    SdChar *pch = SD_NULL;
    SdInt iLogLevl = 0; 
    SdBool bLogDisplay = SD_FALSE;
    SdBool bLibLogDisplay = SD_FALSE;
    SdBool bLogWrite = SD_FALSE;
    SdInt iCount = 0;

    iRet = ut_message_recv(cpstModuleManage->iMsgQueue, &stMsg, emModCOM, MSGDATA_NOWAIT | MSGDATA_NOERROR);
    if(iRet == SD_SUCCESS)
    {
        ut_timer_stop(utTimer);
        stMsg.MsgData[DATA_LEN - 1] = '\0';
        pch = strtok(stMsg.MsgData, "|");
        while(NULL != pch && iCount < 4)
        {
            iCount++;
            if(1 == iCount)
                iLogLevl = atoi(pch);
            if(2 == iCount)
                bLogDisplay = atoi(pch);
            if(3 == iCount)
                bLibLogDisplay = atoi(pch);
            if(4 == iCount)
                bLogWrite = atoi(pch);
            pch = strtok(NULL, "|");
        }
        UT_LOG_SET(iLogLevl, bLogDisplay, bLibLogDisplay, bLogWrite);
    }
    ut_timer_reset(utTimer);
}

/**
 * \fn      static void com_ctrl_log_oper_check_init(void)
 * \brief   ��־��������ʼ��
 * \param
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning
 */
static void com_ctrl_log_oper_check_init(void)
{
    const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();

    cpstModuleManage->iMsgQueue = ut_message_queue_create("/tmp", 'a');
    if(cpstModuleManage->iMsgQueue < 0)
        UT_LOG_LOGOUT_ERROR(emModCOM, 0, "log message queue create fail");

    cpstModuleManage->utTimerLogOperCheck = ut_timer_create(emModCOM, 0, "LogOperCheck");

    ut_timer_start(cpstModuleManage->utTimerLogOperCheck, /* ��ʱ����� */
                   SD_TRUE,  /* �����Զ�ʱ�� */
                   0,        /* ��ʱ��������(����) */
                   2000,     /* ��ʱ������ʱ��(��λ: ms) */
                   com_ctrl_log_oper_check, /* ��ʱ���ص����� */
                   SD_NULL); /* ��ʱ���ص��������� */
}

/**
 * \fn      static void com_ctrl_module_state_set(enCOMState emNewState)
 * \brief   ����ģ��״̬
 * \param   enCOMState emNewState - ��״̬
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning ���뱣֤�����ĺϷ���
*/
static void com_ctrl_module_state_set(enCOMState emNewState)
{
    const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();

    switch(emNewState)
    {
        case emComStateUnInit: /* δ��ʼ�� */
            cpstModuleManage->emCOMState = emComStateUnInit;
        break;
        case emComStateStop: /* ֹͣ */
            cpstModuleManage->emCOMState = emComStateStop;
        break;
        case emComStateWorking: /* ������*/
            cpstModuleManage->emCOMState = emComStateWorking;
        break;
    }
}

/**
 * \fn     static SdInt com_ctrl_module_cfg_update(IN const void *cpCfg)
 * \brief  ����ģ������
 * \param  IN const void *cpCfg - ����
 * \return ������, ����(0)
 * \return ����, ����(-1)
 * \note
 * \todo
 * \version V1.0
 * \warning ���뱣֤�����ĺϷ���
 */
static SdInt com_ctrl_module_cfg_update(IN const void *cpCfg)
{
    const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstModuleCfg = &cpstModuleManage->stCOMCfg;
    const LPCOMCfgDef cpstNewCfg = (LPCOMCfgDef)cpCfg;

    if(memcmp(cpstModuleCfg, cpCfg, sizeof(COMCfgDef)) == 0) /* ���ò���Ҫ���� */
        return 0;

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM config need update!!");

    if(cpstModuleCfg->usListenCfgTcpPort != cpstNewCfg->usListenCfgTcpPort)
        return -1;
    if(cpstModuleCfg->usListenMsgTcpPort != cpstNewCfg->usListenMsgTcpPort)
        return -1;
    if(cpstModuleCfg->usListenCfgUdpPort != cpstNewCfg->usListenCfgUdpPort)
        return -1;

    if(cpstModuleCfg->usNetAdminTcpPort != cpstNewCfg->usNetAdminTcpPort)
        return -1;
    if(cpstModuleCfg->usNetAdminUdpPort != cpstNewCfg->usNetAdminUdpPort)
        return -1;
    if(strncmp(cpstModuleCfg->szNetAdminIP, cpstNewCfg->szNetAdminIP,  MAX_IPADDR_LENGTH))
        return -1;

    if(cpstModuleCfg->usCfgServerTcpPort != cpstNewCfg->usCfgServerTcpPort)
        return -1;
    if(cpstModuleCfg->usCfgServerUdpPort != cpstNewCfg->usCfgServerUdpPort)
        return -1;
    if(strncmp(cpstModuleCfg->szCfgServerIP, cpstNewCfg->szCfgServerIP, MAX_IPADDR_LENGTH))
        return -1;

    if(cpstModuleCfg->usServerTcpPort != cpstNewCfg->usServerTcpPort)
        return -1;
    if(cpstModuleCfg->usServerUdpPort != cpstNewCfg->usServerUdpPort)
        return -1;
    if(strncmp(cpstModuleCfg->szServerIP, cpstNewCfg->szServerIP, MAX_IPADDR_LENGTH))
        return -1;

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM config update!!No reboot.");

    memcpy(cpstModuleCfg, cpstNewCfg, sizeof(COMCfgDef)); /* ������, ֱ�Ӹ��� */

    return 0;
}

/**
 * \fn     static SdInt com_ctrl_module_cfg(IN const void *cpCfg)
 * \brief  ģ������
 * \param  IN const void *cpCfg - ����
 * \return ����(0)
 * \note
 * \todo
 * \version V1.0
 * \warning ���뱣֤�����ĺϷ���
 */
static SdInt com_ctrl_module_cfg(IN const void *cpCfg)
{
    const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstCOMCfgDef = &cpstModuleManage->stCOMCfg;

    if(cpCfg != SD_NULL)
    {
        memcpy(cpstCOMCfgDef, cpCfg, sizeof(COMCfgDef));
    }
    else
    {
        cpstCOMCfgDef->usListenCfgTcpPort = DEFAULT_LISTEN_CFG_TCP_PORT; /* ��������TCP�˿� */
        cpstCOMCfgDef->usListenMsgTcpPort = DEFAULT_LISTEN_MSG_TCP_PORT; /* ������ϢTCP�˿� */
        cpstCOMCfgDef->usListenCfgUdpPort = DEFAULT_LISTEN_CFG_UDP_PORT; /* ��������UDP�˿� */

        strcpy(cpstCOMCfgDef->szCfgServerIP, DEFAULT_CFGSERVER_IP); /* ���÷�����, FTP���� */
        cpstCOMCfgDef->usCfgServerTcpPort = DEFAULT_CFGSERVER_TCP_PORT;
        cpstCOMCfgDef->usCfgServerUdpPort = DEFAULT_CFGSERVER_UDP_PORT;

        strcpy(cpstCOMCfgDef->szNetAdminIP, DEFAULT_NETADMIN_IP); /* ���ܷ�����(��Ϣ������) */
        cpstCOMCfgDef->usNetAdminTcpPort = DEFAULT_NETADMIN_TCP_PORT;
        cpstCOMCfgDef->usNetAdminUdpPort = DEFAULT_NETADMIN_UDP_PORT;

        strcpy(cpstCOMCfgDef->szLiftIP, DEFAULT_LIFT_IP); /* ���ݿ����� */
        cpstCOMCfgDef->usPort= DEFAULT_LIFT_TCP_PORT;

		strcpy(cpstCOMCfgDef->szFaceIP, DEFAULT_FACE_SERVER_IP);
		cpstCOMCfgDef->usFacePort = DEFAULT_FACE_SERVER_TCP_PORT;

		strcpy(cpstCOMCfgDef->szFingerIP, DEFAULT_FINGER_SERVER_IP);
		cpstCOMCfgDef->usFingerPort = DEFAULT_FINGER_SERVER_TCP_PORT;

        strcpy(cpstCOMCfgDef->szServerIP, DEFAULT_SERVER_IP); /* Ԥ�� */
        cpstCOMCfgDef->usServerTcpPort = DEFAULT_SERVER_TCP_PORT;
        cpstCOMCfgDef->usServerUdpPort = DEFAULT_SERVER_UDP_PORT;

        cpstCOMCfgDef->uiHeartBeatPeriod = DEFAULT_HEARTBEAT_PERIOD;

        cpstCOMCfgDef->uiUcCode = DEFAULT_UcCode;
        cpstCOMCfgDef->uiFcCode = DEFAULT_FcCode;
        cpstCOMCfgDef->uiDoorNum = DEFAULT_DoorNum;

        strcpy(cpstCOMCfgDef->szdd,  DEFAULT_dd);
        strcpy(cpstCOMCfgDef->szbbb, DEFAULT_bbb);
        strcpy(cpstCOMCfgDef->szrr,  DEFAULT_rr);
        strcpy(cpstCOMCfgDef->szff,  DEFAULT_ff);
        strcpy(cpstCOMCfgDef->szii,  DEFAULT_ii);
        strcpy(cpstCOMCfgDef->sznn,  DEFAULT_nn);
        strcpy(cpstCOMCfgDef->szaa,  DEFAULT_aa);

        strcpy(cpstCOMCfgDef->szAreaCode, DEFAULT_AreaCode);
        strcpy(cpstCOMCfgDef->szCommunityCode, DEFAULT_CommunityCode);
    }

    cpstCOMCfgDef->szNetAdminIP[MAX_IPADDR_LENGTH - 1] = '\0';
    cpstCOMCfgDef->szServerIP[MAX_IPADDR_LENGTH - 1] = '\0';
    cpstCOMCfgDef->szCfgServerIP[MAX_IPADDR_LENGTH - 1] = '\0';
    cpstCOMCfgDef->szLiftIP[MAX_IPADDR_LENGTH - 1] = '\0';

    cpstCOMCfgDef->szdd[MAX_STR_LEN - 1] = '\0';
    cpstCOMCfgDef->szbbb[MAX_STR_LEN - 1] = '\0';
    cpstCOMCfgDef->szrr[MAX_STR_LEN - 1] = '\0';
    cpstCOMCfgDef->szff[MAX_STR_LEN - 1] = '\0';
    cpstCOMCfgDef->szii[MAX_STR_LEN - 1] = '\0';
    cpstCOMCfgDef->sznn[MAX_STR_LEN - 1] = '\0';
    cpstCOMCfgDef->szaa[MAX_STR_LEN - 1] = '\0';

    cpstCOMCfgDef->szAreaCode[MAX_STR_LEN - 1] = '\0';
    cpstCOMCfgDef->szCommunityCode[MAX_STR_LEN - 1] = '\0';

    return 0;
}

/**
 * \fn      static void com_ctrl_watchdog_feed(UtTimer utTimer, IN void *ptr)
 * \brief   UT��ʱ���ص�����, ι��
 * \param   UtTimer utTimer - ��ʱ�����
 * \param   IN void *ptr - ��������
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning ���뱣֤�����ĺϷ���
 */
static void com_ctrl_watchdog_feed(UtTimer utTimer, IN void *ptr)
{
    com_func_msg_send(emModCOM, COM_COM_WATCHDOG_FEED, 0, 0, 0, SD_NULL);
}

/**
 * \fn      static SdInt com_ctrl_feeddog_timerstart(void)
 * \brief   ����ι����ʱ��
 * \param
 * \return  �ɹ�, ����(0)
 * \return  ʧ��, ����(-1)
 * \note
 * \todo
 * \version V1.0
 * \warning
 */ 
static SdInt com_ctrl_feeddog_timerstart(void)
{
    const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();

    ut_timer_start(cpstModuleManage->utTimerDogFeed, /* ��ʱ�����*/
                   SD_TRUE,                /* �����Զ�ʱ��*/
                   0,                      /* ��ʱ��������(����)*/
                   3000,                   /* ��ʱ������ʱ��(��λ: ms)*/
                   com_ctrl_watchdog_feed, /* ��ʱ���ص�����*/
                   SD_NULL);               /* ��ʱ���ص���������*/
   return 0;
}

s32 com_json_VDP_CallInfo_2(void *ptr, int len);
static void com_ctrl_onUploadEnd(int result, int msgid, void * context, int ctxLen)
{
	switch(msgid)
	{
	case UI_COM_CALL_EVENT_REPORT:
		com_json_VDP_CallInfo_2(context, ctxLen);
        com_func_msg_send(emModUI, COM_UI_CALL_EVENT_REPORT_ACK, 0, 0, 0, SD_NULL);
		break;
	}
}

static void com_ctrl_thread_intaval_timer_callback(IN UtTimer utTimer, IN void * Param);
/**
 * \fn      static SdInt com_ctrl_init(IN const UTMsgDef *cpstMsgDef, IN const void *cpContent)
 * \brief   ģ���ʼ��
 * \param   IN const UTMsgDef* pstMsgDef  ��ȡ����, ����������Ч��
 * \param   IN const void* pContent  ��������
 * \return  �ɹ�, ����(0)
 * \return  ʧ��, ����(-1)
 * \note
 * \todo
 * \version V1.0
 * \warning ���뱣֤�����ĺϷ���
 */
static SdInt com_ctrl_init(IN const UTMsgDef *cpstMsgDef, IN const void *cpContent)
{
    const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();
    enCOMState emModuleState = cpstModuleManage->emCOMState;
    SdInt iRet = 0;

    if(emModuleState != emComStateUnInit)
    {
        if(emModuleState & emComStateWorking) /* ����״̬ */
        {
            if(cpstMsgDef->ulContentLength != sizeof(COMCfgDef)) /* �������Ϸ� */
                return 0;
            iRet = com_ctrl_module_cfg_update(cpContent); /* iRet == 0, ����Ҫ����; -1, ��Ҫ���� */
            if(iRet != 0)
                com_func_msg_send(emModUI, COM_UI_CONFIG_UPDATE_ACK, 0, SD_TRUE, 0, SD_NULL);
            return 0;
        }
        if(emModuleState & emComStateStop) /* �Ѿ���ʼ�� */
            return 0;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s:%d]: COM state error, fail init.", __FILE__, __LINE__);
        return -1; /* δ֪״̬ */
    }

    /* ����COMģ�� */
    if(cpstMsgDef->ulContentLength == sizeof(COMCfgDef)) /* �����Ϸ� */
    {
        com_ctrl_module_cfg(cpContent);
    }
    else /* �������Ϸ�, ʹ��Ĭ������ */
    {
        UT_LOG_LOGOUT_WARNING(emModCOM, 0, "Config COM use default!");
        com_ctrl_module_cfg(SD_NULL);
    }

    /* ��־������Ϣ���г�ʼ�� */
    com_ctrl_log_oper_check_init();

	/* UDP�㲥ģ���ʼ�� */
	com_broadcast_init();
    /* ����ι����ʱ�� */
    cpstModuleManage->utTimerDogFeed = ut_timer_create(emModCOM, 0, "COM DogFeed"); 
    /* �������Ź� */
    com_func_msg_send(emModHW, COM_HW_WATCHDOG_STARTUP, 0, 0, 0, SD_NULL);

    com_timer_init(NULL);
    com_server_init(NULL);
    com_client_init(NULL);
    com_ftp_init(NULL, com_ctrl_onUploadEnd);

    /* ģ�����ֹ̬ͣ */
    com_ctrl_module_state_set(emComStateStop);

    /* ��ӡ������Ϣ */
    {
        const LPCOMCfgDef cpstCOMCfgDef = &cpstModuleManage->stCOMCfg;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Listen, TCP Cfg Port: (%d), TCP Msg Port: (%d), UDP Port: (%d)", 
        cpstCOMCfgDef->usListenCfgTcpPort, cpstCOMCfgDef->usListenMsgTcpPort, cpstCOMCfgDef->usListenCfgUdpPort);

        UT_LOG_LOGOUT_INFO(emModCOM, 0, "NetAdmin  IP: (%s)", cpstCOMCfgDef->szNetAdminIP);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "NetAdmin, TCP Port: (%d), UDP Port: (%d)", cpstCOMCfgDef->usNetAdminTcpPort, cpstCOMCfgDef->usNetAdminUdpPort);

        UT_LOG_LOGOUT_INFO(emModCOM, 0, "Cfg Server IP: (%s)", cpstCOMCfgDef->szCfgServerIP);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "Cfg Server, TCP Port: (%d), UDP Port: (%d)", cpstCOMCfgDef->usCfgServerTcpPort, cpstCOMCfgDef->usCfgServerUdpPort);

        UT_LOG_LOGOUT_INFO(emModCOM, 0, "Lift Server IP: (%s)", cpstCOMCfgDef->szLiftIP);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "Lift Server, TCP Port: (%d)", cpstCOMCfgDef->usPort);
        
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "Reserve Server IP: (%s)", cpstCOMCfgDef->szServerIP);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "Reserve Server, TCP Port: (%d), UDP Port: (%d)", cpstCOMCfgDef->usServerTcpPort, cpstCOMCfgDef->usServerUdpPort);

        UT_LOG_LOGOUT_INFO(emModCOM, 0, "HeartBeat: (%d) millisecond", cpstCOMCfgDef->uiHeartBeatPeriod);

        UT_LOG_LOGOUT_INFO(emModCOM, 0, "Uc    Code: (%d)", cpstCOMCfgDef->uiUcCode);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "Fc    Code: (%d)", cpstCOMCfgDef->uiFcCode);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "Door  Num: (%d)", cpstCOMCfgDef->uiDoorNum);

        UT_LOG_LOGOUT_INFO(emModCOM, 0, "dd:  (%s)", cpstCOMCfgDef->szdd);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "bbb: (%s)", cpstCOMCfgDef->szbbb);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "rr:  (%s)", cpstCOMCfgDef->szrr);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "ff:  (%s)", cpstCOMCfgDef->szff);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "ii:  (%s)", cpstCOMCfgDef->szii);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "nn:  (%s)", cpstCOMCfgDef->sznn);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "aa:  (%s)", cpstCOMCfgDef->szaa);

        UT_LOG_LOGOUT_INFO(emModCOM, 0, "Area Code:  (%s)", cpstCOMCfgDef->szAreaCode);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "Community Code:  (%s)", cpstCOMCfgDef->szCommunityCode);
    }
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM init finish.o(^_^)o ~");
    return 0;
}

/**
 * \fn      static SdInt com_ctrl_start(void)
 * \brief   ģ������
 * \param
 * \return  �ɹ�, ����(0)
 * \return  ʧ��, ����(-1)
 * \note
 * \todo
 * \version V1.0
 * \warning
 */
static SdInt com_ctrl_start(void)
{
    const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();
    enCOMState emModuleState = cpstModuleManage->emCOMState;

    if(emModuleState != emComStateStop)
    {
        if(emModuleState & emComStateWorking) /* ����״̬ */
            return 0; 
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s:%d]: COM state unkonw, error start.", __FILE__, __LINE__);
        return -1;
    }
	cpstModuleManage->t_start = time(NULL);

    com_timer_start(NULL);
    com_server_start(&cpstModuleManage->stCOMCfg);
    com_client_start(NULL);
    com_ftp_start(NULL);
    com_am_start();
	com_broadcast_start();
    /* ģ���������̬ */
    com_ctrl_module_state_set(emComStateWorking);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM start finish.o(^_^)o ~");
	cpstModuleManage->utTimerThreadFeed = ut_timer_create(emModCOM, 0, "Com_Thread_Feed_timer");
	if(cpstModuleManage->utTimerThreadFeed == 0)
	{
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_send_am_data_from_am_db_cache_timer Error(LINE=%d): ut_timer_create()\n", __LINE__);
		return ;
	}
	else
	{
  		ut_timer_start(cpstModuleManage->utTimerThreadFeed, SD_TRUE, 0, THREAD_FEED_INTERVAL_TIME, com_ctrl_thread_intaval_timer_callback, SD_NULL);
	}

    return 0;
}

/**
 * \fn      static SdInt com_ctrl_stop(void)
 * \brief   ģ��ֹͣ
 * \param
 * \return  �ɹ�, ����(0)
 * \return  ʧ��, ����(-1)
 * \note
 * \todo
 * \version V1.0
 * \warning
 */
static SdInt com_ctrl_stop(void)
{
    const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();
    enCOMState emModuleState = cpstModuleManage->emCOMState;

    if(!(emModuleState & emComStateWorking))
    {
        if(emModuleState == emComStateStop)
            return 0;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s:%d]: COM state unkonw, error stop.", __FILE__, __LINE__);
        return -1;
    }

    /* �ر�ι����ʱ�� */
    ut_timer_stop(cpstModuleManage->utTimerDogFeed);

    com_timer_stop(NULL); /* ֹͣ��ʱ��, ɾ��TCP�ͻ����ط���Ϣ */
    com_client_stop(NULL);
    com_server_stop(NULL);
    com_ftp_stop(NULL);
    com_am_stop();
	com_broadcast_stop();
    /* ģ�����ֹ̬ͣ */
    com_ctrl_module_state_set(emComStateStop); /* ģ��ֹͣ */

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM stop finish.o(^_^)o ~");
    return 0;
}

/**
 * \fn      static SdInt com_ctrl_uninit(void)
 * \brief   ģ�鷴��ʼ��
 * \param
 * \return  �ɹ�, ����(0)
 * \return  ʧ��, ����(-1)
 * \note
 * \todo
 * \version V1.0
 * \warning
 */
static SdInt com_ctrl_uninit(void)
{
    const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();
    enCOMState emModuleState = cpstModuleManage->emCOMState;

    if(emModuleState != emComStateStop)
    {
        if((emModuleState & emComStateWorking) || (emModuleState == emComStateUnInit)) /* ֹͣ״̬������״̬ */
            return 0;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s:%d]: COM state unkonw, error uninit.", __FILE__, __LINE__);
        return -1;
    }

    /* ɾ�����Ź���ʱ�� */
    ut_timer_delete(cpstModuleManage->utTimerDogFeed);
    cpstModuleManage->utTimerDogFeed = SD_NULL;

    /* ɾ����־������ⶨʱ�� */
    ut_timer_delete(cpstModuleManage->utTimerLogOperCheck);
    cpstModuleManage->utTimerLogOperCheck = SD_NULL;

    com_timer_uninit(NULL);
    com_server_uninit(NULL);
    com_client_uninit(NULL);
    com_ftp_uninit(NULL);
	com_broadcast_uninit();

    /* ģ�����δ��ʼ��̬ */
    com_ctrl_module_state_set(emComStateUnInit); /* ģ��δ��ʼ��״̬ */
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM uninit finish.o(^_^)o ~");
    return 0;
}

static void com_ctrl_thread_intaval_timer_callback(IN UtTimer utTimer, IN void * Param)
{
	const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();
	time_t end_t = time(NULL);
	UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ctrl_thread_intaval_timer_callback intaval_timer = %d\n", end_t - (cpstModuleManage->t_start));
	if((end_t - (cpstModuleManage->t_start)) > 9)
	{
		com_ctrl_stop();
		com_ctrl_uninit();
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ctrl_thread_intaval_timer_callback stop and uninit the com moudle!\n", end_t - (cpstModuleManage->t_start));
	}
}
/**
 * \fn      static void com_ctrl_singel_func(SdInt iSig)
 * \brief
 * \param   SdInt iSig - �ź�
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning ���뱣֤�����ĺϷ���
 */ 
static void com_ctrl_singel_func(SdInt iSig)
{
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM module recv sig(%d), exit[o(^_^)o..]", iSig);
    com_ctrl_stop();
    com_ctrl_uninit();
    usleep(2000);
    exit(0);
}

/**
 * \fn      static void com_ctrl_ignored_sigpipe(void)
 * \brief
 * \param
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning
 */
static void com_ctrl_ignored_sigpipe(void)
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);
}

/**
 * \fn      void com_env_init(void)
 * \brief
 * \param
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning
 */
void com_env_init(void)
{
    com_ctrl_ignored_sigpipe();
    signal(SIGTERM, com_ctrl_singel_func); /* ����kill all*/
    UT_LOG_SET(emLogError | emLogWarning | emLogInfo | emLogDebug, SD_TRUE, SD_FALSE, SD_TRUE);
}

/**
 * \fn      static void com_ctrl_module_exit(void)
 * \brief
 * \param
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning
 */
static void com_ctrl_module_exit(void)
{
   com_func_msg_send(emModHW, COM_HW_WATCHDOG_CLOSING, 0, 0, 0, SD_NULL);

   com_ctrl_stop();
   usleep(2000);
   com_ctrl_uninit();
   usleep(2000);
   exit(0);
}

void hw_com_am_data_upload(const UTMsgDef *pMsg, const void *pContent)
{
  //  UT_LOG_LOGOUT_DEBUG(emModCOM,0,"PackCount:%d PackNum:%d", pMsg->ulParam1, pMsg->ulParam2);
    if(pMsg->ulContentLength > 0 && pContent != SD_NULL
        && pMsg->ulParam1 != 0 && pMsg->ulParam2 != 0)
    {
        LPAMDataBuffDef pstAMDataBuff = com_ctrl_am_buff_get();
        if(pMsg->ulParam2 == 1)
        {
            pstAMDataBuff->uiDataLen = 0;
            memset(pstAMDataBuff->szBuffer,'\0',AM_BUFF_MAX);
            memcpy(pstAMDataBuff->szBuffer,pContent,pMsg->ulContentLength);
            pstAMDataBuff->uiDataLen = pMsg->ulContentLength;
        }
        else
        {
            if(pstAMDataBuff->uiDataLen + pMsg->ulContentLength > AM_BUFF_MAX)
            {
                UT_LOG_LOGOUT_DEBUG(emModCOM, 0,"AM Data too match length");
                memset(pstAMDataBuff->szBuffer,'\0',AM_BUFF_MAX);
                pstAMDataBuff->uiDataLen = 0;
            }
            else
            {
                SdUChar* pwrite =  pstAMDataBuff->szBuffer + pstAMDataBuff->uiDataLen;
                memcpy(pwrite,pContent,pMsg->ulContentLength);
                pstAMDataBuff->uiDataLen = pstAMDataBuff->uiDataLen + pMsg->ulContentLength;
            }
        }
        UT_LOG_LOGOUT_DEBUG(emModCOM,0,"DataLen:%d ulContentLength:%d", pstAMDataBuff->uiDataLen, pMsg->ulContentLength);
        if(pMsg->ulParam1 == pMsg->ulParam2 && pstAMDataBuff->uiDataLen > 0)
        {
            com_am_tcp_send(pstAMDataBuff->szBuffer, pstAMDataBuff->uiDataLen);
            pstAMDataBuff->uiDataLen = 0;
            memset(pstAMDataBuff->szBuffer,'\0',AM_BUFF_MAX);
        }
    }
}

SdInt com_msg_handle(const UTMsgDef *pMsg, const void *pContent)
{
    SdInt iRet = 0;
//	UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_msg_handle...(%d)", pMsg != NULL ? pMsg->usMsgID : -1);

    switch(pMsg->usMsgID)
    {
        case UI_COM_DOOR_OPEN_TIMEOUT:
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_DOOR_OPEN_TIMEOUT...");
            com_json_door_Alarm_Upload((char *)pContent, pMsg->ulContentLength);
        break;
        case HW_COM_DOOR_OPEN_TIMEOUT:
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: HW_COM_DOOR_OPEN_TIMEOUT...");
            com_json_door_Alarm_Upload((char *)pContent, pMsg->ulContentLength);
        break;
        case UI_COM_DOOR_ALARM_THREATED_OPEN:
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_DOOR_ALARM_THREATED_OPEN...");
            com_json_door_Alarm_Upload((char *)pContent, pMsg->ulContentLength);
        break;
        case UI_COM_DOOR_TAMPERALARM_UPLOAD:
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_DOOR_TAMPERALARM_UPLOAD...");
            com_json_door_Alarm_Upload((char *)pContent, pMsg->ulContentLength);
        break;
        case UI_COM_DOOR_REMOTE_PASSWD_OPEN:
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_DOOR_REMOTE_PASSWD_OPEN...");
            com_json_Unlock_CheckPassword((char *)pContent, pMsg->ulContentLength);
        break;
        case UI_COM_LIFT_REQUEST:
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_LIFT_REQUEST...");
            com_json_Lift_Schedule((char *)pContent, pMsg->ulContentLength);
            com_func_msg_send(emModUI, COM_UI_LIFT_REQUEST_ACK, 0, 0, 0, SD_NULL);
        break;
        case HW_COM_AM_DATA_UPLOAD: /* �ϴ�AM���� */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: HW_COM_AM_DATA_UPLOAD...");
            //com_json_Access_Data((char *)pContent, pMsg->ulContentLength);
            hw_com_am_data_upload(pMsg,pContent);
            com_func_msg_send(emModHW, COM_HW_AM_DATA_UPLOAD_ACK, 0, 0, 0, SD_NULL);
        break;
        case HW_COM_DOOR_OPEN_REPORT: /* �ϴ����ż�¼ */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: HW_COM_DOOR_OPEN_REPORT...");
            com_json_VDP_UnlockEvent((void *)pContent, pMsg->ulContentLength);
            com_func_msg_send(emModHW, COM_HW_DOOR_OPEN_REPORT_ACK, 0, 0, 0, SD_NULL);
        break;
		case UI_COM_AM_RECORD:
			UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_AM_RECORD...");
            com_json_AC_record_upload((char *)pContent, pMsg->ulContentLength);
        break;
		case UI_COM_CALL_ALERT_APP:
			UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_CALL_ALERT_APP...");
            com_json_call_alert_app((char *)pContent, pMsg->ulContentLength);
        break;
        case UI_COM_CALL_EVENT_REPORT: /* �ϴ�ͨ����¼ */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_CALL_EVENT_REPORT...");
            com_json_VDP_CallInfo((void *)pContent, pMsg->ulContentLength);
            //com_func_msg_send(emModUI, COM_UI_CALL_EVENT_REPORT_ACK, 0, 0, 0, SD_NULL);
        break;
		case UI_COM_CALL_IMG_REPORT: /* �ϴ�ͼƬ��¼ */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_CALL_IMG_REPORT...");
			com_json_ftp_ImgInfo((void *)pContent, pMsg->ulContentLength);
        break;		
        case UI_COM_DOOR_SESSION_OPEN_ACK: /* �Ự����Ӧ�� */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_DOOR_SESSION_OPEN_ACK...");
            com_timer_unlock_change((pMsg->ulParam1==SD_SUCCESS)?emDoorOpen:emDoorClose);
        break;
//        case UI_COM_DOOR_REMOTE_OPEN_ACK: /* Զ�̿�������Ӧ�� */
//            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_DOOR_REMOTE_OPEN_ACK...");
//            com_timer_unlock_change((pMsg->ulParam1==SD_SUCCESS)?emDoorOpen:emDoorClose);
//        break;
        case UI_COM_INDOOR_LIFT_REQUEST_ACK:
            {
                UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_INDOOR_LIFT_REQUEST_ACK...");
                com_timer_lift_call_ack_change((pMsg->ulParam1 == SD_SUCCESS)? ack_true_em : ack_false_em);
            }
            break;
        case DATA_COM_AREAINFO_READED: /* DATA��С����Ϣ */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: DATA_COM_AREAINFO_READED...");
            com_json_Message_NoteBrowse((void *)pContent, pMsg->ulContentLength);
            com_func_msg_send(emModUI, COM_DATA_AREAINFO_READED_ACK, 0, 0, 0, SD_NULL);
        break;
        case DATA_COM_AREAINFO_RECV_ACK: /* DATA����С����ϢӦ�� */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: DATA_COM_AREAINFO_RECV_ACK...");
        break;
        case DATA_COM_AREAINFO_DELETEED_ACK: /* DATAɾ��С����ϢӦ�� */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: DATA_COM_AREAINFO_DELETEED_ACK...");
        break;


		case FACE_COM_FACE_SYNC_REQUEST: /* FACE������������ͬ�� */
			com_json_Face_RequestFaceSync((void *)pContent, pMsg->ulContentLength);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: FACE_COM_FACE_SYNC_REQUEST...");
        break;
		case FACE_COM_FACE_SYNC_CHECK: /* FACE��������ͬ����� */
			com_json_Face_FaceSyncCheck((void *)pContent, pMsg->ulContentLength);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: FACE_COM_FACE_SYNC_CHECK...");
        break;
		case FACE_COM_FACE_SYNC_ACK: /* FACE��������ͬ�� */
			com_json_Face_FaceSyncAck(pMsg->ulParam1, pMsg->ulParam2);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: FACE_COM_FACE_SYNC_ACK...");
        break;

		
		case FINGER_COM_FINGER_CHAR: /* ָ������ƥ�� */
			com_json_Finger_Char((void *)pContent, pMsg->ulContentLength);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: FINGER_COM_FINGER_CHAR...");
        break;
		case UI_COM_FINGER_MATCH_ACK: /* ָ������ƥ�� */
			com_json_Finger_Match_Ack(pMsg->ulParam1, pMsg->ulParam2);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_FINGER_MATCH_ACK...");
        break;
		
		case FINGER_COM_FINGER_SYNC_REQUEST: /* ָ������ͬ������ */
			com_json_Finger_Sync_Request((void *)pContent, pMsg->ulContentLength);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: FINGER_COM_FINGER_SYNC_REQUEST...");
        break;
		case FINGER_COM_FINGER_SYNC_CHECK: /* ָ������ͬ����� */
			com_json_Finger_Sync_Check((void *)pContent, pMsg->ulContentLength);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: FINGER_COM_FINGER_SYNC_CHECK...");
        break;
		case FINGER_COM_FINGER_SYNC_ACK: /* ָ��ͬ������ */
			com_json_Finger_Finger_Sync_Ack(pMsg->ulParam1,pMsg->ulParam2);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: FINGER_COM_FINGER_SYNC_ACK...");
        break;

        /*add by eddy at 2017-04-14 for Face SDK
        case UI_COM_FACESDK_SYNC_REQUEST: // IU_FACE������������ͬ�� 
            com_json_Face_RequestFaceSync((void *)pContent, pMsg->ulContentLength);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_FACESDK_SYNC_REQUEST...");
        break;
        case UI_COM_FACESDK_SYNC_CHECK: // IU_FACE��������ͬ����� 
            com_json_Face_FaceSyncCheck((void *)pContent, pMsg->ulContentLength);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_FACESDK_SYNC_CHECK...");
        break;
        case UI_COM_FACESDK_SYNC_ACK: // IU_FACE��������ͬ�� 
            com_json_Face_FaceSyncAck(pMsg->ulParam1, pMsg->ulParam2);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_FACESDK_SYNC_ACK...");
        break;
       add end at 2017-04-14 for Face SDK*/


        case HW_COM_WATCHDOG_STARTUP_ACK: /* �������Ź���Ӧ */
            com_ctrl_feeddog_timerstart(); /* ������ʱ��ι�� */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: HW_COM_WATCHDOG_STARTUP_ACK...");
        break;

        case HW_COM_WATCHDOG_FEED_ACK: /* ι����Ӧ */
//            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: HW_COM_WATCHDOG_FEED_ACK...");
        break;

        case HW_COM_WATCHDOG_CLOSING_ACK: /* �رտ��Ź���Ӧ */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: HW_COM_WATCHDOG_CLOSING_ACK...");
        break;

        case UI_COM_INIT: /* ģ���ʼ�� */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_INIT...");
            iRet = com_ctrl_init(pMsg, pContent);
            com_func_msg_send(emModUI, COM_UI_INIT_ACK, (iRet==0)?SD_SUCCESS:SD_FAIL, 0, 0, SD_NULL);
        break;

        case UI_COM_START: /* ����ģ�� */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_START...");
            iRet = com_ctrl_start();
            com_func_msg_send(emModUI, COM_UI_START_ACK, (iRet==0)?SD_SUCCESS:SD_FAIL, 0, 0, SD_NULL);
            #ifndef CMSDEBUG /* ������, �ǵ���ģʽ, �ر���־ */
                UT_LOG_SET(emLogError | emLogWarning | emLogInfo, SD_FALSE, SD_FALSE, SD_FALSE);
            #endif
        break;

        case UI_COM_CONFIG_UPDATE:
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_CONFIG_UPDATE...");
            if(pMsg->ulContentLength == sizeof(COMCfgDef)) /* �����Ϸ�, �������� */
            {
                iRet = com_ctrl_module_cfg_update(pContent); /* iRet=0, ����Ҫ����, -1, ��Ҫ���� */
                com_func_msg_send(emModUI, COM_UI_CONFIG_UPDATE_ACK, 0, (iRet==-1) ? SD_TRUE : SD_FALSE, 0, SD_NULL);
            }
        break;

        case UI_COM_STOP: /* ֹͣģ�� */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_STOP...");
            iRet = com_ctrl_stop();
            com_func_msg_send(emModUI, COM_UI_STOP_ACK, (iRet==0)?SD_SUCCESS:SD_FAIL, 0, 0, SD_NULL);
        break;

        case UI_COM_UNINIT: /* ����ʼ��ģ�� */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_UNINIT...");
            iRet = com_ctrl_uninit();
            com_func_msg_send(emModUI, COM_UI_UNINIT_ACK, (iRet==0)?SD_SUCCESS:SD_FAIL, 0, 0, SD_NULL);
        break;

        case DEV_COM_EXIT: /* �˳�ģ�� */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: DEV_COM_EXIT...");
            com_ctrl_module_exit();
        break;

        case COM_COM_WATCHDOG_FEED:
            com_func_msg_send(emModHW, COM_HW_WATCHDOG_FEED, 0, 0, 0, SD_NULL); /* ι�� */
        break;

        case COM_COM_CLIENT_CREATE:

        break;
        case COM_COM_CLIENT_SEND:

        break;
		case UI_COM_LIFTSTATUS_INFO:
			UT_LOG_LOGOUT_INFO(emModCOM, 0, "COM Recv msg: UI_COM_LIFTSTATUS_INFO...");
			com_func_msg_send(emModUI, COM_UI_LIFTSTATUS_INFO_ACK, SD_SUCCESS, 0, 0, SD_NULL);
			com_broadcast_lift_info(pContent, pMsg->ulContentLength);
		break;
    }
    return 0;
}


#ifdef CMSDEBUG

static SdInt test_com_msg_UI_COM_INIT(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      UI_COM_INIT, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}

static SdInt test_com_msg_UI_COM_START(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      UI_COM_START, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}
static SdInt test_com_msg_UI_COM_STOP(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      UI_COM_STOP, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}

static SdInt test_com_msg_UI_COM_UNINIT(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      UI_COM_UNINIT, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}

static SdInt test_com_msg_HW_COM_WATCHDOG_STARTUP_ACK(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      HW_COM_WATCHDOG_STARTUP_ACK, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}

static SdInt test_com_msg_HW_COM_WATCHDOG_FEED_ACK(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      HW_COM_WATCHDOG_FEED_ACK, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}

static SdInt test_com_msg_HW_COM_WATCHDOG_CLOSING_ACK(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      HW_COM_WATCHDOG_CLOSING_ACK, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}

static SdInt test_com_msg_DEV_COM_EXIT(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      DEV_COM_EXIT, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}

static SdInt test_com_msg_DATA_COM_AREAINFO_READED(void){
   SdInt iRet = 0;
   ReadAreaInfoDef stReadAreaInfoDef;
   stReadAreaInfoDef.ulIndex = 66;
   strcpy(stReadAreaInfoDef.szRevTime, "2015-04-10 12:15:48");
   strcpy(stReadAreaInfoDef.szBrowseTime, "2015-04-10 12:15:48");
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      DATA_COM_AREAINFO_READED, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(ReadAreaInfoDef), /**< �����䳤���ݵĳ���*/
                      &stReadAreaInfoDef); /**< Ҫ���͵���Ϣ����*/
      return iRet;
}

static SdInt test_com_msg_UI_COM_DOOR_SESSION_OPEN_ACK(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      UI_COM_DOOR_SESSION_OPEN_ACK, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      SD_SUCCESS, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
      return iRet;
}

static SdInt test_com_msg_UI_COM_LIFT_REQUEST(void){
   SdInt iRet = 0;
   RoomAddrDef stRoomAddrDef;
   strcpy(stRoomAddrDef.szdd,  "01");
   strcpy(stRoomAddrDef.szbbb, "02");
   strcpy(stRoomAddrDef.szrr,  "03");
   strcpy(stRoomAddrDef.szff,  "04");
   strcpy(stRoomAddrDef.szii,  "05");

   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      UI_COM_LIFT_REQUEST, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(RoomAddrDef), /**< �����䳤���ݵĳ���*/
                      &stRoomAddrDef); /**< Ҫ���͵���Ϣ����*/
      return iRet;
}

static SdInt test_com_msg_UI_COM_DOOR_REMOTE_PASSWD_OPEN(void){
   SdInt iRet = 0;
   RemotePasswdUnlockDef stRemotePasswdUnlockDef;
   stRemotePasswdUnlockDef.uiUcCode = 22;
   stRemotePasswdUnlockDef.uiFcCode = 156;
   strcpy(stRemotePasswdUnlockDef.szdd,     "01");
   strcpy(stRemotePasswdUnlockDef.szbbb,    "02");
   strcpy(stRemotePasswdUnlockDef.szrr,     "03");
   strcpy(stRemotePasswdUnlockDef.szff,     "04");
   strcpy(stRemotePasswdUnlockDef.szii,     "05");
   strcpy(stRemotePasswdUnlockDef.szPasswd, "szPasswd");

   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      UI_COM_DOOR_REMOTE_PASSWD_OPEN, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(RemotePasswdUnlockDef), /**< �����䳤���ݵĳ���*/
                      &stRemotePasswdUnlockDef); /**< Ҫ���͵���Ϣ����*/
      return iRet;
}

static SdInt test_com_msg_UI_COM_DOOR_ALARM_THREATED_OPEN(void){
   SdInt iRet = 0;
   EPAlarmInfoDef stAlarmInfoDef;
   stAlarmInfoDef.iSrcSn = 22;
   strcpy(stAlarmInfoDef.szDescr, "THREATED");

   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      UI_COM_DOOR_ALARM_THREATED_OPEN, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(EPAlarmInfoDef), /**< �����䳤���ݵĳ���*/
                      &stAlarmInfoDef); /**< Ҫ���͵���Ϣ����*/
      return iRet;
}


static SdInt test_com_msg_HW_COM_AM_DATA_UPLOAD(void){
   SdInt iRet = 0;
   SdChar szBuf[] = {0xAA, 0xAA, 0, 0, 0, 0, 0, 0, 0x82, 0x06, 0, 0, 0, 0, 0, 0, 0x09, 0x8A, 0x38, 0x55, 0x55};

   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      HW_COM_AM_DATA_UPLOAD, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(szBuf), /**< �����䳤���ݵĳ���*/
                      szBuf); /**< Ҫ���͵���Ϣ����*/
      return iRet;
}

static SdInt test_com_msg_UI_COM_DOOR_REMOTE_OPEN_ACK(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      UI_COM_DOOR_REMOTE_OPEN_ACK, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      SD_SUCCESS, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/

      return iRet;
}
#if (0)
static SdInt test_com_msg_COM_COM_CLIENT_CREATE(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      COM_COM_CLIENT_CREATE, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
      return iRet;
}

static SdInt test_com_msg_COM_COM_CLIENT_SEND(void){
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      COM_COM_CLIENT_SEND, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
      return iRet;
}
#endif
static SdInt test_com_msg_HW_COM_DOOR_OPEN_REPORT(void){
   SdInt iRet = 0;
//#if (0)
   DoorRecordDef stDoorRecordDef;
   stDoorRecordDef.emUnlockType = emFingerUnlock;
   stDoorRecordDef.iOperatoruc = 222;
   stDoorRecordDef.stDoorState.emDoorState = emDoorClose;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      HW_COM_DOOR_OPEN_REPORT, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(DoorRecordDef), /**< �����䳤���ݵĳ���*/
                      &stDoorRecordDef); /**< Ҫ���͵���Ϣ����*/
//#endif
      return iRet;
}

static SdInt test_com_msg_UI_COM_CALL_EVENT_REPORT(void){
   SdInt iRet = 0;
   CallEventReportDef stCallEventReportDef;
   stCallEventReportDef.iDesuc = 666;
   stCallEventReportDef.iSrcuc = 888;
   stCallEventReportDef.iIfUnLock = 1;
   stCallEventReportDef.iType = 1;
   strcpy(stCallEventReportDef.pszAnsTime, "abcdefghijk");
   strcpy(stCallEventReportDef.pszHangUpTime, "ABCDEFGHIJKL");
   strcpy(stCallEventReportDef.pszStartTime, "ABCDEFGHIJKL");
   strcpy(stCallEventReportDef.szImage, "No image");

   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      UI_COM_CALL_EVENT_REPORT, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(CallEventReportDef), /**< �����䳤���ݵĳ���*/
                      &stCallEventReportDef); /**< Ҫ���͵���Ϣ����*/
      return iRet;
}


static SdInt test_com_msg_FACE_COM_FACE_SYNC_REQUEST(void)
{
	SdInt iRet = 0;
	FaceVerInfoDef stFaceVerInfo;
	strcpy(stFaceVerInfo.szVerInfo, "2016-01-01 10:10:10");

	iRet = ut_msg_send(emModCOM,
					   emModCOM,
					   FACE_COM_FACE_SYNC_REQUEST,
					   0,
					   0,
					   0,
					   sizeof(FaceVerInfoDef),
					   &stFaceVerInfo);
	return iRet;
}

static SdInt test_com_msg_FACE_COM_FACE_SYNC_CHECK(void)
{
	SdInt iRet = 0;
	FaceVerInfoDef stFaceVerInfo;
	strcpy(stFaceVerInfo.szVerInfo, "v0.1.1.2");


	iRet = ut_msg_send(emModCOM,
					   emModCOM,
					   FACE_COM_FACE_SYNC_CHECK,
					   0,
					   0,
					   0,
					   sizeof(FaceVerInfoDef),
					   &stFaceVerInfo);
	return iRet;
}

static SdInt test_com_msg_COM_FACE_FACE_SYNC_ACK(void)
{
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      FACE_COM_FACE_SYNC_ACK, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      0, /**< �����䳤���ݵĳ���*/
                      SD_NULL); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}



static SdInt test_com_msg_FINGER_COM_FINGER_CHAR(void)
{
   SdInt iRet = 0;
   SdChar szBuf[] = {0xAA, 0xAA, 0, 0, 0, 0, 0, 0, 0x82, 0x06, 0, 0, 0, 0, 0, 0, 0x09, 0x8A, 0x38, 0x55, 0x55};
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      FINGER_COM_FINGER_CHAR, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(szBuf), /**< �����䳤���ݵĳ���*/
                      szBuf); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}

static SdInt test_com_msg_FINGER_COM_FINGER_SYNC_REQUEST(void)
{
   	SdInt iRet = 0;
	FaceVerInfoDef stFaceVerInfo;
	strcpy(stFaceVerInfo.szVerInfo, "v0.1.1.4");

	iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      FINGER_COM_FINGER_SYNC_REQUEST, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(FaceVerInfoDef), /**< �����䳤���ݵĳ���*/
                      &stFaceVerInfo); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}

static SdInt test_com_msg_FINGER_COM_FINGER_SYNC_CHECK(void)
{
	SdInt iRet = 0;
	FaceVerInfoDef stFaceVerInfo;
	strcpy(stFaceVerInfo.szVerInfo, "v0.1.1.5");
	
	iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      FINGER_COM_FINGER_SYNC_CHECK, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(FaceVerInfoDef), /**< �����䳤���ݵĳ���*/
                      &stFaceVerInfo); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}

static SdInt test_com_msg_COM_FINGER_FINGER_SYNC_ACK(void)
{
	SdInt iRet = 0;
	FaceVerInfoDef stFaceVerInfo;
	strcpy(stFaceVerInfo.szVerInfo, "v0.1.1.6");
	iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      FINGER_COM_FINGER_SYNC_ACK, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(FaceVerInfoDef), /**< �����䳤���ݵĳ���*/
                      &stFaceVerInfo); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}

static SdInt test_com_msg_COM_UI_COM_AM_RECORD(void)
{
	SdInt iRet = 0;
	CardInfoDef stpalarm_info_st;
	stpalarm_info_st.iCardId = 100;
	stpalarm_info_st.iEventType = 1;
	stpalarm_info_st.emIOType = emIOTypeComing;
	stpalarm_info_st.iDoorId = 2;
	iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      UI_COM_AM_RECORD, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(stpalarm_info_st), /**< �����䳤���ݵĳ���*/
                      &stpalarm_info_st); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}



#if(0)
static SdInt test_com_msg_SERVER_COM_REQUEST_FACE_SYNC_ACK(void)
{
   SdInt iRet = 0;
   SdChar szBuff[] = {"BizType":"FACE","SubMsgType":"RequestFaceSyncAck","PacketNo":0,"Info":{"FtpUser":"user","FtpPasswd":"passwd","FtpIpAddr":"192.168.1.0","FtpPath":"load/facedata/1/facedata.db","Update":1}};
   
   iRet = ut_msg_send(emModCOM, /**< ��Ϣ����ģ��*/
                      emModCOM, /**< ��Ϣ����ģ��*/
                      FACE_COM_FACE_SYNC_ACK, /**< ��ϢID*/
                      0, /**< ����ϢID*/
                      0, /**< ����1*/
                      0, /**< ����2*/
                      sizeof(szBuff), /**< �����䳤���ݵĳ���*/
                      szBuff); /**< Ҫ���͵���Ϣ����*/
   return iRet;
}
#endif


SdInt com_test_msg_send(SdInt iNum){
   SdInt iRet = 0;
   switch(iNum){
      case 1:
         iRet = test_com_msg_UI_COM_INIT();
      break;
      case 2:
         iRet = test_com_msg_UI_COM_START();
      break;
      case 3:
         iRet = test_com_msg_UI_COM_STOP();
      break;
      case 4:
         iRet = test_com_msg_UI_COM_UNINIT();
      break;
      case 5:
         iRet = test_com_msg_HW_COM_WATCHDOG_STARTUP_ACK();
      break;
      case 6:
         iRet = test_com_msg_HW_COM_WATCHDOG_FEED_ACK();
      break;
      case 7:
         iRet = test_com_msg_HW_COM_WATCHDOG_CLOSING_ACK();
      break;
      case 8:
         iRet = test_com_msg_DEV_COM_EXIT();
      break;
      case 9:
         iRet = test_com_msg_DATA_COM_AREAINFO_READED();
      break;
      case 10:
         iRet = test_com_msg_UI_COM_DOOR_SESSION_OPEN_ACK();
      break;
      case 11:
         iRet = test_com_msg_UI_COM_DOOR_REMOTE_OPEN_ACK();
      break;
      case 12:
         iRet = test_com_msg_HW_COM_AM_DATA_UPLOAD();
      break;
//      case 13:
//         iRet = test_com_msg_COM_COM_CLIENT_CREATE();
//      break;
//      case 14:
//         iRet = test_com_msg_COM_COM_CLIENT_SEND();
//      break;
      case 13:
         iRet = test_com_msg_HW_COM_DOOR_OPEN_REPORT();
      break;
      case 14:
         iRet = test_com_msg_UI_COM_CALL_EVENT_REPORT();
      break;
      case 15:
         iRet = test_com_msg_UI_COM_LIFT_REQUEST();
      break;
      case 16:
         iRet = test_com_msg_UI_COM_DOOR_REMOTE_PASSWD_OPEN();
      break;
      case 17:
         iRet = test_com_msg_UI_COM_DOOR_ALARM_THREATED_OPEN();
      break;
	  case 18:
         iRet = test_com_msg_FACE_COM_FACE_SYNC_REQUEST();
      break;
	  case 19:
         iRet = test_com_msg_FACE_COM_FACE_SYNC_CHECK();
      break;
	  case 20:
         iRet = test_com_msg_COM_FACE_FACE_SYNC_ACK();
      break;
	  case 21:
         iRet = test_com_msg_FINGER_COM_FINGER_CHAR();
      break;
	  case 22:
         iRet = test_com_msg_FINGER_COM_FINGER_SYNC_REQUEST();
      break;
	  case 23:
         iRet = test_com_msg_FINGER_COM_FINGER_SYNC_CHECK();
      break;
	  case 24:
         iRet = test_com_msg_COM_FINGER_FINGER_SYNC_ACK();
      break;
	  case 25:
         iRet = test_com_msg_COM_UI_COM_AM_RECORD();
      break;
      default:
         iRet = -2;
      break;
   }

   return iRet;
}
/**@END! SdInt test_send_msg(SdInt iNum) !\(^o^)/~ ������ */
/*
typedef  SdUInt32    u32;
typedef  SdUShort    u16;
typedef  SdUInt8     u8;

typedef  SdInt8      s8;
typedef  SdInt       s32;
*/
/************************************************************************
* @FunctionName( ������ ): static s8 com_test_string_get(char **ppstart, u16 *plen_u16)
* @CreateDate  (��������): 2015/10/06
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��ȡ�ַ�����ʼ������
*
* @Param(����):
*        **ppstart_s8 - ����, ǰ�ַ�����ʼ, ���, ��ǰ�ַ�����ʼ
*         *plen_u16 - ����, ǰ�ַ�������, ���, ��ǰ�ַ�������
*
* @ReturnCode(����ֵ):
*        -2 - �������ַ�
*        -1 - ��������
*         0 - �������ַ�
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static s8 com_test_string_get(char **ppstart, u16 *plen_u16){
   char *pbuf_s8 = (*ppstart) + (*plen_u16);

   (*ppstart) += (*plen_u16); /* ǰһ���ַ����� */

   if(*pbuf_s8 == '\0')
      *pbuf_s8 = ' '; /* ����ַ������� */

   while(*pbuf_s8 == ' '){
      (*ppstart)++; /* ��λ�ַ�����ʼ */
      pbuf_s8++;
   }

   *plen_u16 = 0; /* ����ַ������� */

   while(((*pbuf_s8 != ' ') && (*pbuf_s8 != '\0'))){
      (*plen_u16)++;
      pbuf_s8++;
   }

   if(*plen_u16 == 0) /* �ַ�������, 0 */
      return -1;

   if(*pbuf_s8 == '\0') /* �������ַ� */
      return -2;
   else{
      *pbuf_s8 = '\0'; /* �����ַ������� */
      return 0;
   }
}
/**@END! static s8 com_test_string_get(char **ppstart, u16 *plen_u16) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static s8 com_test_number_get(char **ppstart, u16 *plen_u16, u32 *pnumber_u32)
* @CreateDate  (��������): 2015/10/06
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��ȡ�ַ�����ʼ������, ��ת������
*
* @Param(����):
*        **ppstart - ����, ǰ�ַ�����ʼ, ���, ��ǰ�ַ�����ʼ
*         *plen_u16 - ����, ǰ�ַ�������, ���, ��ǰ�ַ�������
*         *pnumber_u32 - ���, ����
*
* @ReturnCode(����ֵ):
*        -2 - �������ַ�
*        -1 - ��������
*         0 - �������ַ�
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static s8 com_test_number_get(char **ppstart, u16 *plen_u16, u32 *pnumber_u32){
   s8 ret = com_test_string_get(ppstart, plen_u16);

   char *pbuf_s8 = *ppstart;
   u16 len_u16 = *plen_u16; /* �ַ������� */

   if(ret == -1) /* ���ݴ��� */
      return -1;

   while(len_u16--){
      if(!(pbuf_s8[len_u16] >= '0' && pbuf_s8[len_u16] <= '9')){
         if((pbuf_s8[len_u16] == '-') || (pbuf_s8[len_u16] == '+')){
            if(len_u16 != 0)
               return -1;  /* ���ֳ���, ����ʧ�� */
         }else{
            return -1;  /* ���ֳ���, ����ʧ�� */
         }
      }
   }

   *pnumber_u32 = atoi(pbuf_s8);
   return ret;
}
/**@END! static s8 com_test_number_get(char **ppstart, u16 *plen_u16, u32 *pnumber_u32) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static s8 com_test_cmd_tail_checks(char **ppstart, u16 *plen_u16)
* @CreateDate  (��������): 2015/10/06
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����Ƿ���"�ǿո��ַ�"
*
* @Param(����):
*        **ppstart - ����, ǰ�ַ�����ʼ, ���, ǰ�ַ�����ʼ, 
*         *plen_u16 - ����, ǰ�ַ�������, ���, ǰ�ַ�������
*
* @ReturnCode(����ֵ):
*       -1 - ��"�ǿո��ַ�"
*        0 - ��"�ǿո��ַ�"
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static s8 com_test_cmd_tail_check(char **ppstart, u16 *plen_u16){
   u16 len_u16 = *plen_u16;
   char *pbuf_s8 = *ppstart;
   s8 ret_s8 = 0;

   ret_s8 = com_test_string_get(ppstart, &len_u16);
   *ppstart = pbuf_s8;

   if(ret_s8 == (s8)-1)
      return 0;
   return -1;
}
/**@END! static s8 com_test_cmd_tail_checks(char **ppstart, u16 *plen_u16) !\(^o^)/~ ������ */

#include <stdio.h>   /* printf   */
#include <stdlib.h>  /* atoi     */
#include <string.h>  /* mem, str */

#define  ECHO         printf

u8 table_u8[] = {"0123456789ABCDEF"};

#define  NEW_LINE    0x80 /* ���� */
#define  PUT             0x40 /* ��� */
#define  FILL_ZERO(N)   (0x20|N) /* ���0 */

u8 *num2table(u32 num_u32, u8 base_u8, const u8 *cptable_u8, u8 flag_u8){
   static u8  snum_u8[11];
   u8 *pnum_u8 = snum_u8;
   u8  bit_u8 = 1, sel_u8 = 0;
   u32 base_u32 = 1;

   memset(snum_u8, '0', sizeof(snum_u8));

   if(base_u8 == 16){ /* ʮ������ */
      snum_u8[0] = '0';
      snum_u8[1] = 'x';
      pnum_u8 = snum_u8 + 2;
   }

   if(num_u32 == 0){
      snum_u8[0] = '0';
      base_u32 = base_u8;
   }else{
      for(bit_u8 = 0; num_u32 >= base_u32; ){
         base_u32 *= base_u8;
         bit_u8++;
      }
   }
   
   if(flag_u8 & FILL_ZERO(0)){ /* ���0 */
      if((flag_u8 & 0x0F) > bit_u8){
         if((flag_u8 & 0x0F) < sizeof(snum_u8)){
            pnum_u8 = pnum_u8 + ((flag_u8 & 0x0F) - bit_u8);
         }
      }
   }
   
   for(sel_u8 = 0; sel_u8 < bit_u8; sel_u8++){
      base_u32 /= base_u8;
      pnum_u8[sel_u8] = cptable_u8[num_u32 / base_u32];
      num_u32 %= base_u32;
   }

   if(flag_u8 & NEW_LINE){ /* ���� */
      pnum_u8[bit_u8++] = '\n';
   }
   pnum_u8[bit_u8] = '\0';

   if(flag_u8 & PUT){

   }
   return snum_u8;
}

u8 *float2table(float float_float, const u8 *cptable_u8, u8 flag_u8){
   u8 *pnum_u8;
   u8  bit_u8 = 1;
   u32 int_u32 = 1;

   for(bit_u8 = 0; (u32)float_float >= int_u32; ){
      int_u32 *= 10;
      bit_u8++;
   }
   printf("int_u32 = %d\n", int_u32);
   printf("float_float = %f\n", float_float);
   flag_u8 = 0;
   pnum_u8 = num2table((u32)(float_float * int_u32), 10, cptable_u8, flag_u8);
   return pnum_u8;
}

static s8  com_test_cmd_test_analyse(char **ppstart, u16 *plen_u16){
   u32 num_u32 = 0;
   u32 base = 0;
   u32 flag = 0;
   u8 *pbuf_u8;
   s8 ret_s8 = 0;

   ret_s8 = com_test_number_get(ppstart, plen_u16, &num_u32); /* ������ֵ */
   if(ret_s8 == -1) /* �������� */
      return -1;

   ret_s8 = com_test_number_get(ppstart, plen_u16, &base); /* ������ֵ */
   if(ret_s8 == -1) /* �������� */
      return -1;

   ret_s8 = com_test_number_get(ppstart, plen_u16, &flag); /* ������ֵ */
   if(ret_s8 == -1) /* �������� */
      return -1;

   ret_s8 = com_test_cmd_tail_check(ppstart, plen_u16);
   if(ret_s8) /* ���зǿո��ַ� */
      return -1;

   flag = FILL_ZERO(flag);
   flag |= NEW_LINE;

   printf("num_u32 = (%d), base = (%d), flag = (0x%X)\n", num_u32, (u8)base, flag);
   pbuf_u8 = float2table((float)num_u32 / 10000, table_u8, flag);
   printf("num2table = (%s)\n", pbuf_u8);
   return 0;
}

/**@Start help�������, ��: help */
void com_test_cmd_help_analyse(void){
   puts("Use CMD: \"s m num\" to send a message.");
   puts("Use CMD: \"system SHELL\" to use shell.");
   puts("+=========================================================================+");
   puts("| num:  Messages:                     |                                   |");
   puts("|                                     |                                   |");
   puts("|  1   UI_COM_INIT                    |                                   |");
   puts("|  2   UI_COM_START                   |                                   |");
   puts("|  3   UI_COM_STOP                    |                                   |");
   puts("|  4   UI_COM_UNINIT                  |                                   |");
   puts("|  5   HW_COM_WATCHDOG_STARTUP_ACK    |                                   |");
   puts("|  6   HW_COM_WATCHDOG_FEED_ACK       |                                   |");
   puts("|  7   HW_COM_WATCHDOG_CLOSEING_ACK   |                                   |");
   puts("|  8   DEV_COM_EXIT                   |                                   |");
   puts("|  9   DATA_COM_AREAINFO_READED       |                                   |");
   puts("|  10  UI_COM_DOOR_SESSION_OPEN_ACK   |                                   |");
   puts("|  11  UI_COM_DOOR_REMOTE_OPEN_ACK    |                                   |");
   puts("|  12  HW_COM_AM_DATA_UPLOAD          |                                   |");
//   puts("|  13     COM_COM_CLIENT_CREATE       |                                   |");
//   puts("|  14     COM_COM_CLIENT_SEND         |                                   |");
   puts("|  13  HW_COM_DOOR_OPEN_REPORT        |                                   |");
   puts("|  14  UI_COM_CALL_EVENT_REPORT       |                                   |");
   puts("|  15  UI_COM_LIFT_REQUEST            |                                   |");
   puts("|  16  UI_COM_DOOR_REMOTE_OPEN_ACK    |                                   |");
   puts("|  17  UI_COM_DOOR_ALARM_THREATED_OPEN|                                   |");
   puts("|  18  FACE_COM_FACE_SYNC_REQUEST     |                                   |");
   puts("|  19  FACE_COM_FACE_SYNC_CHECK       |                                   |");
   puts("|  20  COM_FACE_FACE_SYNC_ACK         |                                   |");
   puts("|  21  FINGER_COM_FINGER_CHAR         |                                   |");
   puts("|  22  FINGER_COM_FINGER_SYNC_REQUEST |                                   |");
   puts("|  23  FINGER_COM_FINGER_SYNC_CHECK   |                                   |");
   puts("|  24  COM_FINGER_FINGER_SYNC_ACK     |                                   |");
   puts("|  25  UI_COM_AM_RECORD               |                                   |");
   puts("|                                     |                                   |");
   puts("+=========================================================================+");
}
/**@End help������� */

/**@Start send�������, ��: send msg 1 */
static s8 com_test_cmd_send_analyse(char **ppstart, u16 *plen_u16){
   s8 ret_s8 = com_test_string_get(ppstart, plen_u16); /* ��ȡ�ַ��� */

   char *pbuf_s8 = *ppstart; /* �ַ�����ʼ */
   u16 cmd_len_u16 = *plen_u16; /* �ַ������� */

   u32 msg_num_u32 = 0;
   s32 ret_s32 = 0;

   ret_s8 = com_test_number_get(ppstart, plen_u16, &msg_num_u32); /* ������ֵ */
   if(ret_s8 == -1) /* �������� */
      return -1;

   ret_s8 = com_test_cmd_tail_check(ppstart, plen_u16);
   if(ret_s8) /* ���зǿո��ַ� */
      return -1;

   if(memcmp("msg", pbuf_s8, cmd_len_u16) == 0){
      ret_s32 = com_test_msg_send(msg_num_u32);
      ECHO("Send msg: (%d)\n", msg_num_u32);
      if(ret_s32 >= (s8)0){
         ECHO("Success, ret = (%d).\n", ret_s32);
      }else if(ret_s32 == (s8)-1){
         ECHO("Fail.\n");
      }else if(ret_s32 == (s8)-2){ /* ����ʶ����Ϣ */
         ECHO("Unknow msg number.\n");
      }else{
         ECHO("Unknow ret = (%d).\n", ret_s32);
      }
   }else
      return -1;

   return 0;
}
/**@End send������� */

/**@Start system�������, ��: system ls */
s8 com_test_cmd_system_analyse(char **ppstart, u16 *plen_u16){
   system(((*ppstart) + ((*plen_u16) + 1)));
   return 0;
}
/**@End system������� */

/************************************************************************
* @FunctionName( ������ ): void com_test_cmd_analyse(char *pbuf_s8, u16 *pmaxLen_u16)
* @CreateDate  (��������): 2015/05/15
* @Author      ( ��  �� ): WDY
*
* @Description(����): ����һ���ַ�, ���ͺ���, ������Ӧ����
*
* @Param(����):
*        *pbuf_s8 - �ַ����ݻ�����
*        *pmaxLen_u16 - ����������Ч���ݳ���
*
* @ReturnCode(����ֵ):
*        None(��)
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
void com_test_cmd_analyse(char *pbuf_s8, u16 *pmaxLen_u16){
   if(*pmaxLen_u16 != 0){
      char *pcmd_buf = pbuf_s8;
      u16 str_len_u16 = 0; /* �ַ������� */
      s8 ret_s8 = 0;

      pbuf_s8[*pmaxLen_u16] = '\0'; /* ����ַ�����β */
      pbuf_s8[*pmaxLen_u16+1] = '\0'; /* ����ַ�����β */

      com_test_string_get(&pcmd_buf, &str_len_u16);

      if(memcmp("test", pcmd_buf, str_len_u16) == 0){
         ret_s8 = com_test_cmd_test_analyse(&pcmd_buf, &str_len_u16);
      }else if(memcmp("send", pcmd_buf, str_len_u16) == 0){
         ret_s8 = com_test_cmd_send_analyse(&pcmd_buf, &str_len_u16);
      }else if(memcmp("system", pcmd_buf, str_len_u16) == 0){
         ret_s8 = com_test_cmd_system_analyse(&pcmd_buf, &str_len_u16);
      }else if(memcmp("help", pcmd_buf, str_len_u16) == 0){
         com_test_cmd_help_analyse();
      }else{
         ret_s8 = -1;
      }

      if(ret_s8 == (s8)-1){ /* ������� */
         pcmd_buf[str_len_u16] = ' '; /* ���������ַ�������('\0') */
         pbuf_s8[*pmaxLen_u16] = '\0'; /* �����ַ�������λ�� */
         ECHO("Sorry, wrong cmd: [%s]\n", pbuf_s8);
      }

      *pmaxLen_u16 = 0;
   }
}
/**@END! void com_test_cmd_analyse(char *pbuf_s8, u16 *pmaxLen_u16) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): u32 com_debug(void)
* @CreateDate  (��������): 2015/09/01
* @Author      ( ��  �� ): WDY
*
* @Description(����): ������- while(1)�ȴ����벢�����ź�
*
* @Param(����):
*        None(��)
*
* @ReturnCode(����ֵ):
*        0
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
u32 com_debug(void)
{
   char recv[80];
   u16 len_u16 = 0;

   usleep(50000);
   com_test_cmd_help_analyse(); /* debug������Ϣ */

   while(1)
   {
      printf("Enter command:\n");
      fflush(stdout);

      if(fgets(recv, sizeof(recv), stdin) != SD_NULL){
         len_u16 = strlen(recv) - 1;
         com_test_cmd_analyse(recv, &len_u16);
      }else{
         printf("===============fgets error===============\n");
      }
   }

   return 0;
}
/**@END! u32 com_debug(void) !\(^o^)/~ ������ */

#endif

