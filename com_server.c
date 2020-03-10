/**
 * \file    com_server.c
 * \author  CMS - WDY
 * \date    2015-09-10
 * \version
 * \brief
 * \warning 版权所有, 侵权必究
 * \todo    COM模块服务器端程序
 */
#include "com_func.h"

#define  SERVER_LIST                    &cpserver_cfg_st->client_list

#define  SERVER_RCV_TIME                20 /* TCP服务器SOCK接收时间, 单位: S */

/************************************************************************
* enum  em_client_type
*        TCP客户端类型
************************************************************************/
typedef enum enum_client_type{
    client_term_em = 0x01, /* 终端设备   */
    client_cfg_em,         /* 配置服务器 */
    client_msg_em,         /* 信息服务器 */
}em_client_type;

/************************************************************************
* struct  st_tcp_client
*        TCP客户端
************************************************************************/
typedef struct struct_tcp_client{
    pthread_t       thread;
    s32             sock;
    em_client_type  client_type_em;
}st_tcp_client, *pst_tcp_client;

/************************************************************************
* struct  st_server_cfg
*        配置TCP服务器
************************************************************************/
typedef struct struct_server_cfg{
    pthread_t   msg_thread;     /* 接收信息服务器连接 */
    st_list     client_list;    /* TCP服务器链表      */
    s8          server_run;     /* TCP服务器运行标志  */
    u16         tcp_client_num; /* TCP客户端数量      */
    s32         fd_listen;      /* TCP服务器监听句柄 */ 
}st_server_cfg, *pst_server_cfg;

static st_server_cfg gs_server_cfg_st = {
    .tcp_client_num = 0, /* TCP客户端数量     */
    .server_run = 0,     /* TCP服务器运行标志 */
    .fd_listen = 0,      /* TCP服务器监听句柄 */ 
};

/************************************************************************
* @FunctionName( 函数名 ): pst_server_cfg com_server_cfg_get(void)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 返回TCP服务器配置
*
* @Param(参数):
*        None(无)
*
* @ReturnCode(返回值):
*        pst_server_cfg
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
pst_server_cfg com_server_cfg_get(void){
    return &gs_server_cfg_st;
}
/**@END! pst_server_cfg com_server_cfg_get(void) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static em_list_operate com_server_list_callback_client_del(void *elem, void *parg, va_list args)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 链表迭代回调函数: 删除TCP客户端
*
* @Param(参数):
*       *elem - 链表元素
*       *parg -
*        args - 变参列表
*
* @ReturnCode(返回值):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
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
/**@END! static em_list_operate com_server_list_callback_client_del(void *elem, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static em_list_operate com_server_list_callback_list_destory(void *elem, void *parg, va_list args)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 链表迭代回调函数: 销毁TCP服务器的客户端
*
* @Param(参数):
*       *elem - 链表元素
*       *parg -
*        args - 变参列表
*
* @ReturnCode(返回值):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
static em_list_operate com_server_list_callback_client_destory(void *elem, void *parg, va_list args){
    const pst_tcp_client cplist_st = (pst_tcp_client)elem;

    if (cplist_st != NULL){
        com_func_sock_close(cplist_st->sock);
        cplist_st->sock = 0;
    }

    return operate_remove_em;
}
/**@END! static em_list_operate com_server_list_callback_list_destory(void *elem, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static void *com_server_thread(void *parg)
* @Description(描述):      主线程: 服务线程
* @Param(参数):            pst_tcp_client
* @ReturnCode(返回值):
************************************************************************/
static void *com_server_thread(void *parg){
    const pst_tcp_client cptcp_client_st = (pst_tcp_client)parg;
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();
    s8 *prx_buf = NULL;
    s32 sock_s32 = 0, ret_s32 = 0;
    u32 rx_len = 0;

    pthread_detach(pthread_self()); /* 分离线程 */
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
        com_func_sock_close(sock_s32); /* 关闭SOCK */
    /* 删除TCP客户端 */
    com_func_list_iterate(SERVER_LIST, &com_server_list_callback_client_del, cptcp_client_st, &ret_s32);
    --cpserver_cfg_st->tcp_client_num;
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): tcp client(---) num=(%d)\n", __LINE__, cpserver_cfg_st->tcp_client_num);
    return 0;
}
/**@END! static void *com_server_thread(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_server_client_add(const pst_tcp_client cptcp_client_st)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 增加TCP客户端
*
* @Param(参数):
*        cptcp_client_st - TCP客户端
*
* @ReturnCode(返回值):
*        0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
static s32 com_server_client_add(pst_tcp_client ptcp_client_st){
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();
    s32 ret_s32 = 0;

    if (ptcp_client_st == NULL)
        return -1;

    /* 设置SOCK接收非阻塞 */
    ret_s32 = com_func_sock_nonblock(ptcp_client_st->sock);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): com_func_sock_nonblock()\n", __LINE__);
        goto exit_goto;
    }
    /* 创建服务线程 */
    ret_s32 = pthread_create(&ptcp_client_st->thread, NULL, &com_server_thread, ptcp_client_st);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): pthread_create()\n", __LINE__);
        goto exit_goto;
    }
    /* 加入TCP客户端到链表 */
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
/**@END! static s32 com_server_client_add(const pst_tcp_client cptcp_client_st) !\(^o^)/~ 结束咯 */

void com_server_thread_intaval_timer_callback(IN UtTimer utTimer, IN void * Param)
{
	const LPCOMManage cpstModuleManage = com_ctrl_module_manage_get();
	cpstModuleManage->t_start = time(NULL);
}
/************************************************************************
* @FunctionName( 函数名 ): static void *com_server_msg_main_thread(void *parg)
* @Description(描述):      主线程: 接收信息服务器及终端连接
* @Param(参数):            pst_com_cfg
* @ReturnCode(返回值):
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

    /* 创建TCP服务器 */
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
        /* 收到连接, 解析客户端IP */
        pip_client = (s8 *)inet_ntoa(addr_client.sin_addr); /* inet_ntoa 返回静态缓冲区 */
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): msg main thread recv connect from (%s)\n", __LINE__, pip_client);
        /* 检测IP合法性 */
        if (strcmp((char *)pip_client, (char *)cpcom_cfg_st->szNetAdminIP) == 0){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): (%s) is msg server.\n", __LINE__, pip_client);
            client_type_em = client_msg_em;
        }else{
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): msg main thread unknow (%s).\n", __LINE__, pip_client);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): may be a terminal.\n", __LINE__);
            client_type_em = client_term_em;
        }
        /* 创建TCP客户端 */
        ptcp_client_st = (pst_tcp_client)malloc(sizeof(st_tcp_client));
        if (ptcp_client_st == NULL){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): malloc()\n", __LINE__);
            com_func_sock_close(fd_client);
            continue;
        }
        ptcp_client_st->sock = fd_client;
        ptcp_client_st->client_type_em = client_type_em;
        /* 加入TCP客户端到链表 */
        com_server_client_add(ptcp_client_st);
    }
exit_goto:
    com_func_sock_close(cpserver_cfg_st->fd_listen);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Info(LINE=%d): com_server_msg_main_thread() exiting..(o _ o)..Bye~\n", __LINE__);
    return 0;
}
/**@END! static void *com_server_msg_main_thread(void *parg) !\(^o^)/~ 结束咯 */

/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_server_init(void *arg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_server_init(void *arg){
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();
    s32 ret_s32 = 0;

    /* 初始化链表 */
    ret_s32 = com_func_list_init(SERVER_LIST);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): com_func_list_init(SERVER_LIST)\n", __LINE__);
        return -1;
    }

    return 0;
}
/**@END! s32 com_server_init(void *arg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_server_start(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 启动
* @Param(参数): pst_com_cfg
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_server_start(void *parg){
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();
    s32 ret_s32 = 0;

    /* 运行TCP服务器 */
    cpserver_cfg_st->server_run = 1;

    /* 初始化主线程: 接收信息服务器及终端连接(9902端口) */
    ret_s32 = pthread_create(&cpserver_cfg_st->msg_thread, NULL, &com_server_msg_main_thread, parg);
    if (ret_s32 != 0)
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): pthread_create()\n", __LINE__);

    return 0;
}
/**@END! s32 com_ftp_start(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_server_stop(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 停止
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_server_stop(void *parg){
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();
    s32 ret_s32 = 0;

    /* 停止TCP服务器运行 */
    cpserver_cfg_st->server_run = 0;
    /* 销毁TCP服务器的客户端 */
    com_func_list_iterate(SERVER_LIST, &com_server_list_callback_client_destory, NULL);
    while (cpserver_cfg_st->tcp_client_num);

    ret_s32 = pthread_join(cpserver_cfg_st->msg_thread, NULL);
    if (ret_s32 != 0)
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server Error(LINE=%d): pthread_join(msg_thread)\n", __LINE__);

    return 0;
}
/**@END! s32 com_server_stop(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_server_uninit(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 反初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_server_uninit(void *parg){
    const pst_server_cfg cpserver_cfg_st = com_server_cfg_get();

    com_func_list_destory(SERVER_LIST);

    return 0;
}
/**@END! s32 com_server_uninit(void *parg) !\(^o^)/~ 结束咯 */


#if (0)
#include "com_ctrl.h"

#include "com_func.h"

#include <sys/epoll.h>

#define  EPOLL_MAX_EVENTS                       32    /* EPOLL最大事件数 */

#define  LOCK_COM_SERVER()        ut_mutex_lock(pstCOMManage->UtMutexTCPServer) /* 申请链表锁 */

#define  UNLOCK_COM_SERVER()      ut_mutex_unlock(pstCOMManage->UtMutexTCPServer) /* 释放链表锁 */

/**
 * \struct TCPClientNode
 * \brief  TCP客户端节点
 */
typedef struct tagTCPClientNode
{
   UtThread thread;
   SdInt iSock;
}TCPClientNode, *LPTCPClientNode;

/**
* \fn      SdInt com_server_client_add_to_list(SdInt iSock, pthread_t thread, const struct list_head *list_head)
* \brief   添加客户端Socket到链表
* \param   SdInt iSock - Scket句柄
* \param   pthread_t thread - 线程句柄
* \param   INOUT UtList **List - 链表表头
* \return  成功, 返回(0)
* \return  失败, 返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
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
* \brief   从链表删除客户端Socket
* \param   SdInt iSock - Scket句柄
* \param   INOUT UtList **List - 链表表头
* \return
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
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
 * \brief   销毁客户端链表
 * \param   INOUT UtList **List - 链表头
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning 必须保证参数的合法性
*/
void com_server_TCPclient_list_destory(INOUT UtList **List)
{
   LPTCPClientNode pstClientNode = SD_NULL;
   UtList *Elem = *List;
   UtList *NextElem = SD_NULL;

   while(Elem != SD_NULL){
      pstClientNode =(LPTCPClientNode)Elem->data;
      NextElem = ut_list_next(Elem); /* 下一个元素 */
      *List = ut_list_remove_by_elem(*List, Elem);
      com_func_sock_close(pstClientNode->iSock); /* 关闭sock */
      ut_mem_free(pstClientNode); /* 释放客户端节点 */
      Elem = NextElem;
   }
}

/**
* \fn      static void *com_server_CLIENTnetadmin_thread(void *ptr)
* \brief   服务器线程: 网管作为客户端
* \param   void *ptr - pstTCPClientManage
* \return
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
static void *com_server_CLIENTnetadmin_thread(void *ptr)
{
   LPTCPClientManage pstTCPClientManage =(LPTCPClientManage)ptr;
   LPCOMManage pstCOMManage = SD_NULL;

   SdInt iSock = 0;
   SdChar *pszRxBuf = SD_NULL; /* 网络接收的数据 */
   SdInt iRxLen = 0; /* 网络接收的数据长度 */
   SdInt iRet = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Client NetAdmin connect tcp server success.(^o^) /");

   pthread_detach(pthread_self()); /* 分离 */

   if(pstTCPClientManage == SD_NULL)
      goto thread_exit; /* 参数不合法 */

   iSock = pstTCPClientManage->iSock;
   pstCOMManage = pstTCPClientManage->pstCOMManage;

   while(1) /* TCP 客户端关闭这个连接时, 线程退出 */
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

      LOCK_COM_SERVER(); /* 申请链表锁 */
      
      com_server_client_del_from_list(iSock, &pstCOMManage->UtListTCPClient); /* 从链表删除一个客户端 */
      --pstCOMManage->TCPClientCnt;

      UNLOCK_COM_SERVER(); /* 释放链表锁 */

      UT_LOG_LOGOUT_INFO(emModCOM, 0, "TCP Client(---) num = (%d)", pstCOMManage->TCPClientCnt);
   }
   if(pszRxBuf != SD_NULL)
      ut_mem_free(pszRxBuf);

   pthread_exit(0);
}

/**
* \fn      static void *com_server_CLIENTterm_thread(void *ptr)
* \brief   服务器线程: 其他终端为客户端
* \param   void *ptr - pstTCPClientManage
* \return
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
static void *com_server_CLIENTterm_thread(void *ptr)
{
   LPTCPClientManage pstTCPClientManage =(LPTCPClientManage)ptr;
   LPCOMManage pstCOMManage = SD_NULL;

   SdInt iSock = 0;
   SdChar *pszRxBuf = SD_NULL; /* 网络接收的数据 */
   SdInt iRxLen = 0; /* 网络接收的数据长度 */
   SdInt iRet = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Client Terminal connect tcp server success.(^o^) /");

   pthread_detach(pthread_self()); /* 分离 */

   if(pstTCPClientManage == SD_NULL)
      goto thread_exit; /* 参数不合法 */

   iSock = pstTCPClientManage->iSock;
   pstCOMManage = pstTCPClientManage->pstCOMManage;

   while(1) /* TCP 客户端关闭这个连接时, 线程退出 */
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

      LOCK_COM_SERVER(); /* 申请链表锁 */
      
      com_server_client_del_from_list(iSock, &pstCOMManage->UtListTCPClient); /* 从链表删除一个客户端 */
      --pstCOMManage->TCPClientCnt;

      UNLOCK_COM_SERVER(); /* 释放链表锁 */

      UT_LOG_LOGOUT_INFO(emModCOM, 0, "TCP Client(---) num = (%d)", pstCOMManage->TCPClientCnt);
   }
   if(pszRxBuf != SD_NULL)
      ut_mem_free(pszRxBuf);

   pthread_exit(0);
}


/**
* \fn      static int com_server_TCPserver_main(void *ptr)
* \brief   TCP sever 主循环
* \param   void *para - &stCOMManage
* \return  成功, 返回(0)
* \return  失败, 返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
static SdInt com_server_TCPserver_main(void *ptr)
{
   LPCOMManage pstCOMManage =(LPCOMManage)ptr;
   SdInt iRet = 0;
   struct epoll_event ev; /* epol注册事件 */
   struct epoll_event events[EPOLL_MAX_EVENTS]; /* 内核返回事件集合 */
   SdInt iListenSock, iNewSock, i;
   SdInt iEpollWaitRet ,epollfd;
   struct sockaddr_in local;
   socklen_t addrlen = sizeof(struct sockaddr);

   SdShort usClientPort;
   SdChar *pszClientIP = SD_NULL;
   UtThread ClientThread; /* 客户端线程 */
   TCPClientManage stTCPClientManage;

   if(pstCOMManage == SD_NULL)
      return -1; /* 参数不合法 */

   epollfd = epoll_create(10); /* 最大监听 句柄10 */
   if(epollfd == -1)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: epoll_create error.", __FILE__,  __LINE__);
      return -1;
   }

   iListenSock = pstCOMManage->TCPServerSock;
   ev.events = EPOLLIN; /* fd 读 */
   ev.data.fd = iListenSock;

   if(epoll_ctl(epollfd, EPOLL_CTL_ADD, iListenSock, &ev) != 0) /* 事件注册 */
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: epoll_ctl error.", __FILE__,  __LINE__);
      return -1;
   }

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "TCP server thread id: (%d)", pstCOMManage->UtThreadTCPServer);

   while(pstCOMManage->TCPServerRunFlag)
   {
      iEpollWaitRet = epoll_wait(epollfd, events, EPOLL_MAX_EVENTS, 500); /* 500milli, -1 == 阻塞 */
      if(iEpollWaitRet == -1)
      {
         if(errno == EINTR) /* 事件请求前, 被信号打断 */
         {
            continue; /* 继续等待 */
         }
         break; /* 退出server */
         UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: epoll_wait EINTR.", __FILE__,  __LINE__, strerror(errno));
      }
      else if(iEpollWaitRet == 0) /* 等待超时 */
      {
         continue; /* 结束本次循环, 重新等待 */
         /*UT_LOG_LOGOUT_INFO(emModCOM, 0, "tcp server wait connect request");*/
      }

      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Recv (%d) client connect server req.", iEpollWaitRet);
      for(i = 0; i < iEpollWaitRet; i++) /* 轮询所有事件 */
      {
         if(events[i].data.fd == iListenSock)
         {
            iNewSock = accept(iListenSock,(struct sockaddr *) &local, &addrlen); /* 接收链接 */
            if(iNewSock == -1) /* 建立链接出错 */
            {
               UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: accept error.", __FILE__,  __LINE__);
               continue; /* 结束本次循环 */
            }
            com_func_sock_nonblock_set(iNewSock); /* 非阻塞Sock */

            pszClientIP = inet_ntoa(local.sin_addr); /* pszClientIP 指向静态缓冲区 */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "TCP server accept client (%s:%d).", pszClientIP, ntohs(local.sin_port));

            memset(&stTCPClientManage, 0, sizeof(TCPClientManage));
            stTCPClientManage.pstCOMManage = pstCOMManage;
            stTCPClientManage.iSock= iNewSock;

            if(0 == strcmp(pszClientIP, pstCOMManage->stCOMCfg.szNetAdminIP)) /* 网管连接 */
            {
               iRet = ut_thread_create(&ClientThread, UT_THREAD_STACK_SIZE_DEFAULT, UT_THREAD_PRIORITY_DEFAULT,
                                                      com_server_CLIENTnetadmin_thread, &stTCPClientManage);
               if(iRet != 0)
               {
                  UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_thread_create error.", __FILE__,  __LINE__);
                  com_func_sock_close(iNewSock); /* 创建客户端失败, 关闭sock */
                  continue; /* 退出for循环 */
               }
               stTCPClientManage.thread = ClientThread;
            }
            else if(0 == strcmp(pszClientIP, pstCOMManage->stCOMCfg.szServerIP)) /* CMS 服务器连接 */
            {

            }
            else /* 其他连接, 终端 */
            {
               iRet = ut_thread_create(&ClientThread, UT_THREAD_STACK_SIZE_DEFAULT, UT_THREAD_PRIORITY_DEFAULT,
                                                      com_server_CLIENTterm_thread, &stTCPClientManage);
               if(iRet != 0)
               {
                  UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_thread_create error.", __FILE__,  __LINE__);
                  com_func_sock_close(iNewSock); /* 创建客户端失败, 关闭sock */
                  continue; /* 退出for循环 */
               }
               stTCPClientManage.thread = ClientThread;
            }

            if(0 != LOCK_COM_SERVER()) /* 申请链表锁 */
            {
               UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_mutex_lock fail.", __FILE__,  __LINE__);
               com_func_sock_close(iNewSock); /* 申请链表失败, 关闭sock */
               continue; /* 结束本次循环 */
            }

            if(com_server_client_add_to_list(iNewSock, ClientThread, &pstCOMManage->UtListTCPClient) != 0) /* 添加新的Socket到链表 */
            {
               UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: add client to list fail.", __FILE__,  __LINE__);
            }
            else
            {
               ++pstCOMManage->TCPClientCnt; /* 增加客户端数量 */
               UT_LOG_LOGOUT_INFO(emModCOM, 0, "TCP Client(+++) num = (%d).", pstCOMManage->TCPClientCnt);
            }

            UNLOCK_COM_SERVER(); /* 释放链表锁 */
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
 * \brief   TCP server线程函数
 * \param
 * \return  成功, 返回(0)
 * \return  失败, 返回(-1)
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
 * \brief  启动服务器线程
 * \param INOUT void *ptr - &stCOMManage
 * \return 成功, 返回(0)
 * \return 失败, 返回(-1)
 * \note
 * \todo
 * \version V1.0
 * \warning 必须保证参数的合法性 
 */
SdInt com_server_TCPserver_thread_start(INOUT void *ptr)
{
   LPCOMManage pstCOMManage =(LPCOMManage)ptr;
   SdInt iRet = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server_TCPserver_thread_start running...");
   if(pstCOMManage == SD_NULL)
      return -1; /* 参数不合法 */

   pstCOMManage->TCPServerRunFlag = SD_TRUE; /* TCP server 运行 */

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
 * \brief  停止服务器线程
 * \param INOUT void *ptr - &stCOMManage
 * \return 成功, 返回(0)
 * \return 失败, 返回(-1)
 * \note
 * \todo
 * \version V1.0
 * \warning 必须保证参数的合法性 
 */
SdInt com_server_TCPserver_thread_stop(INOUT void *ptr)
{
   LPCOMManage pstCOMManage =(LPCOMManage)ptr;
   SdInt iRet = 0;
   void *ptrRet = SD_NULL;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_server_TCPserver_thread_stop running...");
   if(pstCOMManage == SD_NULL)
      return -1; /* 参数不合法 */

   LOCK_COM_SERVER(); /* 申请链表锁 */
   com_server_TCPclient_list_destory(&pstCOMManage->UtListTCPClient); /* 销毁客户端链表 */
   UNLOCK_COM_SERVER(); /* 释放链表锁 */

   while(pstCOMManage->TCPClientCnt); /* 等待所有客户端线程退出 */

   pstCOMManage->TCPServerRunFlag = SD_FALSE; /* TCP server 停止 */

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
