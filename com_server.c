/**
 * \file    com_server.c
 * \author  CMS - WDY
 * \date    2015-09-10
 * \version
 * \brief
 * \warning ��Ȩ����, ��Ȩ�ؾ�
 * \todo    COMģ��������˳���
 */
#include "com_func.h"

#define  SERVER_LIST                    &cpserver_cfg_st->client_list

#define  SERVER_RCV_TIME                20 /* TCP������SOCK����ʱ��, ��λ: S */

/************************************************************************
* enum  em_client_type
*        TCP�ͻ�������
************************************************************************/
typedef enum enum_client_type{
    client_term_em = 0x01, /* �ն��豸   */
    client_cfg_em,         /* ���÷����� */
    client_msg_em,         /* ��Ϣ������ */
}em_client_type;

/************************************************************************
* struct  st_tcp_client
*        TCP�ͻ���
************************************************************************/
typedef struct struct_tcp_client{
    pthread_t       thread;
    s32             sock;
    em_client_type  client_type_em;
}st_tcp_client, *pst_tcp_client;

/************************************************************************
* struct  st_server_cfg
*        ����TCP������
************************************************************************/
typedef struct struct_server_cfg{
    pthread_t   msg_thread;     /* ������Ϣ���������� */
    st_list     client_list;    /* TCP����������      */
    s8          server_run;     /* TCP���������б�־  */
    u16         tcp_client_num; /* TCP�ͻ�������      */
    s32         fd_listen;      /* TCP������������� */ 
}st_server_cfg, *pst_server_cfg;

static st_server_cfg gs_server_cfg_st = {
    .tcp_client_num = 0, /* TCP�ͻ�������     */
    .server_run = 0,     /* TCP���������б�־ */
    .fd_listen = 0,      /* TCP������������� */ 
};

/************************************************************************
* @FunctionName( ������ ): pst_server_cfg com_server_cfg_get(void)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����TCP����������
*
* @Param(����):
*        None(��)
*
* @ReturnCode(����ֵ):
*        pst_server_cfg
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
pst_server_cfg com_server_cfg_get(void){
    return &gs_server_cfg_st;
}
/**@END! pst_server_cfg com_server_cfg_get(void) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_server_list_callback_client_del(void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��������ص�����: ɾ��TCP�ͻ���
*
* @Param(����):
*       *elem - ����Ԫ��
*       *parg -
*        args - ����б�
*
* @ReturnCode(����ֵ):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static em_list_operate com_server_list_callback_client_del(void *elem, void *parg, va_list args){
    const pst_tcp_client cplist_st = (pst_tcp_client)elem;
    const pst_tcp_client ptcp_client_st = (pst_tcp_client)parg;
    s32 *pnum_s32 = va_arg(args, s32 *);

    if (elem == NULL)
        return operate_remove_em;
    if (parg == NULL)
        return operate_break_em;

    if (ptcp_client_st->sock != cplist_st->sock)
        return operate_next_em;

    (*pnum_s32)++;
    free(cplist_st); elem = NULL;
    return (operate_remove_em | operate_break_em);
}
/**@END! static em_list_operate com_server_list_callback_client_del(void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_server_list_callback_list_destory(void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��������ص�����: ����TCP�������Ŀͻ���
*
* @Param(����):
*       *elem - ����Ԫ��
*       *parg -
*        args - ����б�
*
* @ReturnCode(����ֵ):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static em_list_operate com_server_list_callback_client_destory(void *elem, void *parg, va_list args){
    const pst_tcp_client cplist_st = (pst_tcp_client)elem;

    if (cplist_st != NULL){
        com_func_sock_close(cplist_st->sock);
        cplist_st->sock = 0;
    }

    return operate_remove_em;
}
/**@END! static em_list_operate com_server_list_callback_list_destory(void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static void *com_server_thread(void *parg)
* @Description(����):      ���߳�: �����߳�
* @Param(����):            pst_tcp_client
* @ReturnCode(����ֵ):
************************************************************************/
static void *com_server_thread(void *parg){
    const pst_tcp_client cptcp_client_st = (pst_tcp_client)parg;
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();
    s8 *prx_buf = NULL;
    s32 sock_s32 = 0, ret_s32 = 0;
    u32 rx_len = 0;

    pthread_detach(pthread_self()); /* �����߳� */
    if (parg == NULL)
        return 0;

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): com tcp server thread running..\n", __LINE__);
    sock_s32 = cptcp_client_st->sock;

    while (1){
        ret_s32 = com_func_tcp_msg_recv(cptcp_client_st->sock, &prx_buf, &rx_len, SERVER_RCV_TIME);
        if (ret_s32 || (!cpserver_cfg_st->server_run))
            goto exit_goto;

        if ((prx_buf != NULL) && rx_len)
            com_json_unpack(cptcp_client_st->sock, prx_buf, rx_len);
    }
exit_goto:
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): com tcp server thread exiting(o _ o)..Bye~\n", __LINE__);
    if (prx_buf != NULL)
        free(prx_buf);
    if (cpserver_cfg_st->server_run)
        com_func_sock_close(sock_s32); /* �ر�SOCK */
    /* ɾ��TCP�ͻ��� */
    com_func_list_iterate(SERVER_LIST, &com_server_list_callback_client_del, cptcp_client_st, &ret_s32);
    --cpserver_cfg_st->tcp_client_num;
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): tcp client(---) num=(%d)\n", __LINE__, cpserver_cfg_st->tcp_client_num);
    return 0;
}
/**@END! static void *com_server_thread(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static s32 com_server_client_add(const pst_tcp_client cptcp_client_st)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����TCP�ͻ���
*
* @Param(����):
*        cptcp_client_st - TCP�ͻ���
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static s32 com_server_client_add(pst_tcp_client ptcp_client_st){
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();
    s32 ret_s32 = 0;

    if (ptcp_client_st == NULL)
        return -1;

    /* ����SOCK���շ����� */
    ret_s32 = com_func_sock_nonblock(ptcp_client_st->sock);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): com_func_sock_nonblock()\n", __LINE__);
        goto exit_goto;
    }
    /* ���������߳� */
    ret_s32 = pthread_create(&ptcp_client_st->thread, NULL, &com_server_thread, ptcp_client_st);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): pthread_create()\n", __LINE__);
        goto exit_goto;
    }
    /* ����TCP�ͻ��˵����� */
    ret_s32 = com_func_list_elem_insert(SERVER_LIST, ptcp_client_st);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): com_func_list_elem_insert()\n", __LINE__);
        goto exit_goto;
    }
    ++cpserver_cfg_st->tcp_client_num;
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): tcp client(+++) num=(%d)\n", __LINE__, cpserver_cfg_st->tcp_client_num);
    return 0;
exit_goto:
    com_func_sock_close(ptcp_client_st->sock);
    free(ptcp_client_st);
    ptcp_client_st = NULL;
    return -1;
}
/**@END! static s32 com_server_client_add(const pst_tcp_client cptcp_client_st) !\(^o^)/~ ������ */

void com_server_thread_intaval_timer_callback(IN UtTimer utTimer, IN void * Param)
{
	const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();
	cpstModuleManage->t_start = time(NULL);
}
/************************************************************************
* @FunctionName( ������ ): static void *com_server_msg_main_thread(void *parg)
* @Description(����):      ���߳�: ������Ϣ���������ն�����
* @Param(����):            pst_com_cfg
* @ReturnCode(����ֵ):
************************************************************************/
static void *com_server_msg_main_thread(void *parg){
    const LPCOMCfgDef cpcom_cfg_st = (LPCOMCfgDef)parg;
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();
    socklen_t addr_len = sizeof(struct sockaddr_in);
    s32 /*fd_listen = 0, */fd_client = 0;
    struct sockaddr_in addr_client;
    s8 *pip_client = NULL;
    em_client_type client_type_em;
    pst_tcp_client ptcp_client_st = NULL;

    /* ����TCP������ */
    cpserver_cfg_st->fd_listen = com_func_tcp_server_create(cpcom_cfg_st->usListenMsgTcpPort);
    if (cpserver_cfg_st->fd_listen == -1){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): com_func_tcp_server_create()\n", __LINE__);
        goto exit_goto;
    }

	UtTimer thread_innaval_timer = 0;
	thread_innaval_timer = ut_timer_create(emModCOM, 0, "therad_innaval_timer");
	if(thread_innaval_timer == 0)
	{
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server_msg_main_thread Error(LINE=%d): ut_timer_create()\n", __LINE__);
		return ;
	}
	else
	{
  		ut_timer_start(thread_innaval_timer, SD_TRUE, 0, THREAD_FEED_INTERVAL_TIME, com_server_thread_intaval_timer_callback, SD_NULL);
	}
    while (cpserver_cfg_st->server_run){
        fd_client = accept(cpserver_cfg_st->fd_listen, (struct sockaddr *)&addr_client, &addr_len);
        if (fd_client == -1)
            continue;
        if (!cpserver_cfg_st->server_run){
            com_func_sock_close(fd_client);
            continue;
        }
        /* �յ�����, �����ͻ���IP */
        pip_client = (s8 *)inet_ntoa(addr_client.sin_addr); /* inet_ntoa ���ؾ�̬������ */
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): msg main thread recv connect from (%s)\n", __LINE__, pip_client);
        /* ���IP�Ϸ��� */
        if (strcmp((char *)pip_client, (char *)cpcom_cfg_st->szNetAdminIP) == 0){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): (%s) is msg server.\n", __LINE__, pip_client);
            client_type_em = client_msg_em;
        }else{
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): msg main thread unknow (%s).\n", __LINE__, pip_client);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): may be a terminal.\n", __LINE__);
            client_type_em = client_term_em;
        }
        /* ����TCP�ͻ��� */
        ptcp_client_st = (pst_tcp_client)malloc(sizeof(st_tcp_client));
        if (ptcp_client_st == NULL){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): malloc()\n", __LINE__);
            com_func_sock_close(fd_client);
            continue;
        }
        ptcp_client_st->sock = fd_client;
        ptcp_client_st->client_type_em = client_type_em;
        /* ����TCP�ͻ��˵����� */
        com_server_client_add(ptcp_client_st);
    }
exit_goto:
    com_func_sock_close(cpserver_cfg_st->fd_listen);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): com_server_msg_main_thread() exiting..(o _ o)..Bye~\n", __LINE__);
    return 0;
}
/**@END! static void *com_server_msg_main_thread(void *parg) !\(^o^)/~ ������ */

/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( ������ ): s32 com_server_init(void *arg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ��ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_server_init(void *arg){
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();
    s32 ret_s32 = 0;

    /* ��ʼ������ */
    ret_s32 = com_func_list_init(SERVER_LIST);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): com_func_list_init(SERVER_LIST)\n", __LINE__);
        return -1;
    }

    return 0;
}
/**@END! s32 com_server_init(void *arg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_server_start(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����
* @Param(����): pst_com_cfg
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_server_start(void *parg){
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();
    s32 ret_s32 = 0;

    /* ����TCP������ */
    cpserver_cfg_st->server_run = 1;

    /* ��ʼ�����߳�: ������Ϣ���������ն�����(9902�˿�) */
    ret_s32 = pthread_create(&cpserver_cfg_st->msg_thread, NULL, &com_server_msg_main_thread, parg);
    if (ret_s32 != 0)
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): pthread_create()\n", __LINE__);

    return 0;
}
/**@END! s32 com_ftp_start(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_server_stop(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ֹͣ
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_server_stop(void *parg){
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();
    s32 ret_s32 = 0;

    /* ֹͣTCP���������� */
    cpserver_cfg_st->server_run = 0;
    /* ����TCP�������Ŀͻ��� */
    com_func_list_iterate(SERVER_LIST, &com_server_list_callback_client_destory, NULL);
    while (cpserver_cfg_st->tcp_client_num);

    ret_s32 = pthread_join(cpserver_cfg_st->msg_thread, NULL);
    if (ret_s32 != 0)
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): pthread_join(msg_thread)\n", __LINE__);

    return 0;
}
/**@END! s32 com_server_stop(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_server_uninit(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_server_uninit(void *parg){
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();

    com_func_list_destory(SERVER_LIST);

    return 0;
}
/**@END! s32 com_server_uninit(void *parg) !\(^o^)/~ ������ */


#if (0)
#include "com_ctrl.h"

#include "com_func.h"

#include <sys/epoll.h>

#define  EPOLL_MAX_EVENTS                       32    /* EPOLL����¼��� */

#define  LOCK_COM_SERVER()        ut_mutex_lock(pstCOMManage->UtMutexTCPServer) /* ���������� */

#define  UNLOCK_COM_SERVER()      ut_mutex_unlock(pstCOMManage->UtMutexTCPServer) /* �ͷ������� */

/**
 * \struct TCPClientNode
 * \brief  TCP�ͻ��˽ڵ�
 */
typedef struct tagTCPClientNode
{
   UtThread thread;
   SdInt iSock;
}TCPClientNode, *LPTCPClientNode;

/**
* \fn      SdInt com_server_client_add_to_list(SdInt iSock, pthread_t thread, const struct list_head *list_head)
* \brief   ��ӿͻ���Socket������
* \param   SdInt iSock - Scket���
* \param   pthread_t thread - �߳̾��
* \param   INOUT UtList **List - �����ͷ
* \return  �ɹ�, ����(0)
* \return  ʧ��, ����(-1)
* \note
* \todo
* \version V1.0
* \warning ���뱣֤�����ĺϷ���
*/
SdInt com_server_client_add_to_list(SdInt iSock, pthread_t thread, INOUT UtList **List)
{
   LPTCPClientNode pstNewClientNode;

   pstNewClientNode = ut_mem_new(TCPClientNode, 1);
   if(pstNewClientNode == SD_NULL)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_mem_new error.", __FILE__,  __LINE__);
      return -1;
   }
   memset(pstNewClientNode, 0, sizeof(TCPClientNode));

   pstNewClientNode->iSock = iSock;
   pstNewClientNode->thread = thread;

   *List = ut_list_append(*List, pstNewClientNode);

   return 0;
}

/**
* \fn      void com_server_client_del_from_list(SdInt iSock, INOUT UtList **List)
* \brief   ������ɾ���ͻ���Socket
* \param   SdInt iSock - Scket���
* \param   INOUT UtList **List - �����ͷ
* \return
* \note
* \todo
* \version V1.0
* \warning ���뱣֤�����ĺϷ���
*/
void com_server_client_del_from_list(SdInt iSock, INOUT UtList **List)
{
   LPTCPClientNode pstClientNode = SD_NULL;
   UtList *Elem = *List;

   while(Elem != SD_NULL){
      pstClientNode =(LPTCPClientNode)Elem->data;
      if(pstClientNode ->iSock == iSock)
      {
         *List = ut_list_remove_by_elem(*List, Elem);
         ut_mem_free(pstClientNode);
         break;
      }
      Elem = ut_list_next(Elem);
   }
}

/**
 * \fn      void com_server_TCPclient_list_destory(INOUT UtList **List);
 * \brief   ���ٿͻ�������
 * \param   INOUT UtList **List - ����ͷ
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning ���뱣֤�����ĺϷ���
*/
void com_server_TCPclient_list_destory(INOUT UtList **List)
{
   LPTCPClientNode pstClientNode = SD_NULL;
   UtList *Elem = *List;
   UtList *NextElem = SD_NULL;

   while(Elem != SD_NULL){
      pstClientNode =(LPTCPClientNode)Elem->data;
      NextElem = ut_list_next(Elem); /* ��һ��Ԫ�� */
      *List = ut_list_remove_by_elem(*List, Elem);
      com_func_sock_close(pstClientNode->iSock); /* �ر�sock */
      ut_mem_free(pstClientNode); /* �ͷſͻ��˽ڵ� */
      Elem = NextElem;
   }
}

/**
* \fn      static void *com_server_CLIENTnetadmin_thread(void *ptr)
* \brief   �������߳�: ������Ϊ�ͻ���
* \param   void *ptr - pstTCPClientManage
* \return
* \note
* \todo
* \version V1.0
* \warning ���뱣֤�����ĺϷ���
*/
static void *com_server_CLIENTnetadmin_thread(void *ptr)
{
   LPTCPClientManage pstTCPClientManage =(LPTCPClientManage)ptr;
   LPCOMManage pstCOMManage = SD_NULL;

   SdInt iSock = 0;
   SdChar *pszRxBuf = SD_NULL; /* ������յ����� */
   SdInt iRxLen = 0; /* ������յ����ݳ��� */
   SdInt iRet = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Client NetAdmin connect tcp server success.(^o^) /");

   pthread_detach(pthread_self()); /* ���� */

   if(pstTCPClientManage == SD_NULL)
      goto thread_exit; /* �������Ϸ� */

   iSock = pstTCPClientManage->iSock;
   pstCOMManage = pstTCPClientManage->pstCOMManage;

   while(1) /* TCP �ͻ��˹ر��������ʱ, �߳��˳� */
   {
      if((iRet = com_func_NETmsg_recv(iSock, &pszRxBuf, &iRxLen, 20000)) != 0)
      {
         goto thread_exit;
      }
      com_json_unpack(iSock, pszRxBuf, iRxLen, SD_NULL);
      iRxLen = 0;
    }

thread_exit:
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Quit thread: Connected Client NetAdmin!(o _ o)..Bye~");
   {
      struct sockaddr_in sock_addr;
      socklen_t addr_len = sizeof(struct sockaddr_in);

      com_func_sock_close(iSock);

      LOCK_COM_SERVER(); /* ���������� */
      
      com_server_client_del_from_list(iSock, &pstCOMManage->UtListTCPClient); /* ������ɾ��һ���ͻ��� */
      --pstCOMManage->TCPClientCnt;

      UNLOCK_COM_SERVER(); /* �ͷ������� */

      UT_LOG_LOGOUT_INFO(emModCOM, 0, "TCP Client(---) num = (%d)", pstCOMManage->TCPClientCnt);
   }
   if(pszRxBuf != SD_NULL)
      ut_mem_free(pszRxBuf);

   pthread_exit(0);
}

/**
* \fn      static void *com_server_CLIENTterm_thread(void *ptr)
* \brief   �������߳�: �����ն�Ϊ�ͻ���
* \param   void *ptr - pstTCPClientManage
* \return
* \note
* \todo
* \version V1.0
* \warning ���뱣֤�����ĺϷ���
*/
static void *com_server_CLIENTterm_thread(void *ptr)
{
   LPTCPClientManage pstTCPClientManage =(LPTCPClientManage)ptr;
   LPCOMManage pstCOMManage = SD_NULL;

   SdInt iSock = 0;
   SdChar *pszRxBuf = SD_NULL; /* ������յ����� */
   SdInt iRxLen = 0; /* ������յ����ݳ��� */
   SdInt iRet = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Client Terminal connect tcp server success.(^o^) /");

   pthread_detach(pthread_self()); /* ���� */

   if(pstTCPClientManage == SD_NULL)
      goto thread_exit; /* �������Ϸ� */

   iSock = pstTCPClientManage->iSock;
   pstCOMManage = pstTCPClientManage->pstCOMManage;

   while(1) /* TCP �ͻ��˹ر��������ʱ, �߳��˳� */
   {
      if((iRet = com_func_NETmsg_recv(iSock, &pszRxBuf, &iRxLen, 20000)) != 0)
      {
         goto thread_exit;
      }
      com_json_unpack(iSock, pszRxBuf, iRxLen, SD_NULL);
      iRxLen = 0;
    }

thread_exit:
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Quit thread: Connected Terminal!(o _ o)..Bye~");
   {
      struct sockaddr_in sock_addr;
      socklen_t addr_len = sizeof(struct sockaddr_in);

      com_func_sock_close(iSock);

      LOCK_COM_SERVER(); /* ���������� */
      
      com_server_client_del_from_list(iSock, &pstCOMManage->UtListTCPClient); /* ������ɾ��һ���ͻ��� */
      --pstCOMManage->TCPClientCnt;

      UNLOCK_COM_SERVER(); /* �ͷ������� */

      UT_LOG_LOGOUT_INFO(emModCOM, 0, "TCP Client(---) num = (%d)", pstCOMManage->TCPClientCnt);
   }
   if(pszRxBuf != SD_NULL)
      ut_mem_free(pszRxBuf);

   pthread_exit(0);
}


/**
* \fn      static int com_server_TCPserver_main(void *ptr)
* \brief   TCP sever ��ѭ��
* \param   void *para - &stCOMManage
* \return  �ɹ�, ����(0)
* \return  ʧ��, ����(-1)
* \note
* \todo
* \version V1.0
* \warning ���뱣֤�����ĺϷ���
*/
static SdInt com_server_TCPserver_main(void *ptr)
{
   LPCOMManage pstCOMManage =(LPCOMManage)ptr;
   SdInt iRet = 0;
   struct epoll_event ev; /* epolע���¼� */
   struct epoll_event events[EPOLL_MAX_EVENTS]; /* �ں˷����¼����� */
   SdInt iListenSock, iNewSock, i;
   SdInt iEpollWaitRet ,epollfd;
   struct sockaddr_in local;
   socklen_t addrlen = sizeof(struct sockaddr);

   SdShort usClientPort;
   SdChar *pszClientIP = SD_NULL;
   UtThread ClientThread; /* �ͻ����߳� */
   TCPClientManage stTCPClientManage;

   if(pstCOMManage == SD_NULL)
      return -1; /* �������Ϸ� */

   epollfd = epoll_create(10); /* ������ ���10 */
   if(epollfd == -1)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: epoll_create error.", __FILE__,  __LINE__);
      return -1;
   }

   iListenSock = pstCOMManage->TCPServerSock;
   ev.events = EPOLLIN; /* fd �� */
   ev.data.fd = iListenSock;

   if(epoll_ctl(epollfd, EPOLL_CTL_ADD, iListenSock, &ev) != 0) /* �¼�ע�� */
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: epoll_ctl error.", __FILE__,  __LINE__);
      return -1;
   }

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "TCP server thread id: (%d)", pstCOMManage->UtThreadTCPServer);

   while(pstCOMManage->TCPServerRunFlag)
   {
      iEpollWaitRet = epoll_wait(epollfd, events, EPOLL_MAX_EVENTS, 500); /* 500milli, -1 == ���� */
      if(iEpollWaitRet == -1)
      {
         if(errno == EINTR) /* �¼�����ǰ, ���źŴ�� */
         {
            continue; /* �����ȴ� */
         }
         break; /* �˳�server */
         UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: epoll_wait EINTR.", __FILE__,  __LINE__, strerror(errno));
      }
      else if(iEpollWaitRet == 0) /* �ȴ���ʱ */
      {
         continue; /* ��������ѭ��, ���µȴ� */
         /*UT_LOG_LOGOUT_INFO(emModCOM, 0, "tcp server wait connect request");*/
      }

      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Recv (%d) client connect server req.", iEpollWaitRet);
      for(i = 0; i < iEpollWaitRet; i++) /* ��ѯ�����¼� */
      {
         if(events[i].data.fd == iListenSock)
         {
            iNewSock = accept(iListenSock,(struct sockaddr *) &local, &addrlen); /* �������� */
            if(iNewSock == -1) /* �������ӳ��� */
            {
               UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: accept error.", __FILE__,  __LINE__);
               continue; /* ��������ѭ�� */
            }
            com_func_sock_nonblock_set(iNewSock); /* ������Sock */

            pszClientIP = inet_ntoa(local.sin_addr); /* pszClientIP ָ��̬������ */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "TCP server accept client (%s:%d).", pszClientIP, ntohs(local.sin_port));

            memset(&stTCPClientManage, 0, sizeof(TCPClientManage));
            stTCPClientManage.pstCOMManage = pstCOMManage;
            stTCPClientManage.iSock= iNewSock;

            if(0 == strcmp(pszClientIP, pstCOMManage->stCOMCfg.szNetAdminIP)) /* �������� */
            {
               iRet = ut_thread_create(&ClientThread, UT_THREAD_STACK_SIZE_DEFAULT, UT_THREAD_PRIORITY_DEFAULT,
                                                      com_server_CLIENTnetadmin_thread, &stTCPClientManage);
               if(iRet != 0)
               {
                  UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_thread_create error.", __FILE__,  __LINE__);
                  com_func_sock_close(iNewSock); /* �����ͻ���ʧ��, �ر�sock */
                  continue; /* �˳�forѭ�� */
               }
               stTCPClientManage.thread = ClientThread;
            }
            else if(0 == strcmp(pszClientIP, pstCOMManage->stCOMCfg.szServerIP)) /* CMS ���������� */
            {

            }
            else /* ��������, �ն� */
            {
               iRet = ut_thread_create(&ClientThread, UT_THREAD_STACK_SIZE_DEFAULT, UT_THREAD_PRIORITY_DEFAULT,
                                                      com_server_CLIENTterm_thread, &stTCPClientManage);
               if(iRet != 0)
               {
                  UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_thread_create error.", __FILE__,  __LINE__);
                  com_func_sock_close(iNewSock); /* �����ͻ���ʧ��, �ر�sock */
                  continue; /* �˳�forѭ�� */
               }
               stTCPClientManage.thread = ClientThread;
            }

            if(0 != LOCK_COM_SERVER()) /* ���������� */
            {
               UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_mutex_lock fail.", __FILE__,  __LINE__);
               com_func_sock_close(iNewSock); /* ��������ʧ��, �ر�sock */
               continue; /* ��������ѭ�� */
            }

            if(com_server_client_add_to_list(iNewSock, ClientThread, &pstCOMManage->UtListTCPClient) != 0) /* ����µ�Socket������ */
            {
               UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: add client to list fail.", __FILE__,  __LINE__);
            }
            else
            {
               ++pstCOMManage->TCPClientCnt; /* ���ӿͻ������� */
               UT_LOG_LOGOUT_INFO(emModCOM, 0, "TCP Client(+++) num = (%d).", pstCOMManage->TCPClientCnt);
            }

            UNLOCK_COM_SERVER(); /* �ͷ������� */
         }
         else
         {

         }
      }
   }
   return 0;
}

/**
 * \fn      static void *tcp_server_thread_fun(void *para);
 * \brief   TCP server�̺߳���
 * \param
 * \return  �ɹ�, ����(0)
 * \return  ʧ��, ����(-1)
 * \note
 * \todo
 * \version V1.0
 * \warning
 */ 
static void *com_server_TCPserver_thread(void *ptr)
{
   LPCOMManage pstCOMManage =(LPCOMManage)ptr;
   SdInt iRet = 0;

   if(pstCOMManage == SD_NULL)
      goto thread_exit;

   iRet = com_func_TCPserver_create(SD_NULL, pstCOMManage->stCOMCfg.usListenMsgTcpPort, &pstCOMManage->TCPServerSock);
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: Create TCP server fail.", __FILE__, __LINE__);
      goto thread_exit;
   }

   iRet = com_server_TCPserver_main(ptr);
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "COM error: com_server_TCPserver_main error.");
      goto thread_exit;
   }

thread_exit:
   com_func_sock_close(pstCOMManage->TCPServerSock);
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Quit thread: TCP Server!(o _ o)..Bye~");
   pthread_exit((void *)0 );
}

/**
 * \fn     SdInt com_server_TCPserver_thread_start(INOUT void *ptr);
 * \brief  �����������߳�
 * \param INOUT void *ptr - &stCOMManage
 * \return �ɹ�, ����(0)
 * \return ʧ��, ����(-1)
 * \note
 * \todo
 * \version V1.0
 * \warning ���뱣֤�����ĺϷ��� 
 */
SdInt com_server_TCPserver_thread_start(INOUT void *ptr)
{
   LPCOMManage pstCOMManage =(LPCOMManage)ptr;
   SdInt iRet = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server_TCPserver_thread_start running...");
   if(pstCOMManage == SD_NULL)
      return -1; /* �������Ϸ� */

   pstCOMManage->TCPServerRunFlag = SD_TRUE; /* TCP server ���� */

   iRet = ut_thread_create(&pstCOMManage->UtThreadTCPServer, UT_THREAD_STACK_SIZE_DEFAULT, UT_THREAD_PRIORITY_DEFAULT, 
                                          com_server_TCPserver_thread, ptr);
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_thread_create fail.", __FILE__, __LINE__);
      return -1;
   }

   return 0;
}

/**
 * \fn     SdInt com_server_TCPserver_thread_stop(INOUT void *ptr);
 * \brief  ֹͣ�������߳�
 * \param INOUT void *ptr - &stCOMManage
 * \return �ɹ�, ����(0)
 * \return ʧ��, ����(-1)
 * \note
 * \todo
 * \version V1.0
 * \warning ���뱣֤�����ĺϷ��� 
 */
SdInt com_server_TCPserver_thread_stop(INOUT void *ptr)
{
   LPCOMManage pstCOMManage =(LPCOMManage)ptr;
   SdInt iRet = 0;
   void *ptrRet = SD_NULL;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server_TCPserver_thread_stop running...");
   if(pstCOMManage == SD_NULL)
      return -1; /* �������Ϸ� */

   LOCK_COM_SERVER(); /* ���������� */
   com_server_TCPclient_list_destory(&pstCOMManage->UtListTCPClient); /* ���ٿͻ������� */
   UNLOCK_COM_SERVER(); /* �ͷ������� */

   while(pstCOMManage->TCPClientCnt); /* �ȴ����пͻ����߳��˳� */

   pstCOMManage->TCPServerRunFlag = SD_FALSE; /* TCP server ֹͣ */

   if(pstCOMManage->UtThreadTCPServer == 0)
      return 0;

   iRet= pthread_join(pstCOMManage->UtThreadTCPServer, &ptrRet);
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: pthread_join error.", __FILE__, __LINE__);
      return -1;
   }

   pstCOMManage->UtThreadTCPServer = 0;
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Stop TCP server thread success.");

   return 0;
}
#endif
