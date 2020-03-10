/**
 * \file    com_client.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning ��Ȩ����, ��Ȩ�ؾ�
 * \todo    TCP�ͻ���
 */
#include "com_func.h"

#define  NET_MSG_LIST                   &cpclient_cfg_st->net_msg_list
#define  SOCK_POOL_LIST                 &cpclient_cfg_st->sock_pool_list

#define		CLIENT_RCV_TIME					10    /* TCP�ͻ���SOCK����ʱ��, ��λ: S */
#define		CLIENT_RESEND_INTERVAL			3000  /* TCP�ͻ����ط�ʱ����, ��λ: ms */
#define		MAX_LINE_HALF					2048
#define		TMP_LEN							256
#define		AM_INFO_CACHE_MAX				1000
#define		ROLL_THE_AM_CACHE_TIMER			300 * 1000	/* �ط�AmCache.db��ʱ���� ��λ: ms */

#define		AM_CACHE_DB						"/mnt/mmcblk0p4/resource/extra/50/AmCache.db"

#define		CREAT_AM_CACHE_TABLE		"create table am_cache (pack_id int, json_data char)"
#define		INSERT_AM_DATA_TO_TABLE		"insert into am_cache (pack_id,json_data) values"
#define		DELETE_AM_DATA_FROM_TABLE	"delete from am_cache where pack_id = "
#define		SUM_AM_DATA_COUNT			"select count(*) from am_cache"
#define		READ_AM_CACHE				"select * from am_cache"
#define		READ_THE_MAX_PACK_ID		"select max(pack_id) from am_cache"
#define		DELETE_THE_FIRST_RECORD		"delete from am_cache where rowid in(select rowid from am_cache limit 1)"

/*
	select pack_id from am_cache limit 1;
	delete from am_cache where pack_id='1';
*/

/************************************************************************
* struct  st_net_msg
*        ������Ϣ
************************************************************************/
typedef struct st_amcache
{
	int pack_id;
	char *json_date;
}stamcache,*pstamcache;

/************************************************************************
* struct  st_net_msg
*        ������Ϣ
************************************************************************/
typedef struct struct_net_msg{
    s32             sock;           /* SOCK���       */
    u32             pack_id;        /* ����           */
    u32             len_u32;        /* ���ݻ�����     */
    u16             port;           /* �������˿�     */
    s8              ip[16];         /* ������IP       */
    s8             *pbuf_s8;        /* ���ݻ�����ָ�� */
    TIMER_HANDLE    timer_handle;   /* ��ʱ�����     */
}st_net_msg, *pst_net_msg;

/************************************************************************
* struct  st_sock_pool
*        ���ӳ�
************************************************************************/
typedef struct struct_sock_pool{
    s32 sock;   /* SOCK���   */
    u16 port;   /* �������˿� */
    s8  ip[16]; /* ������IP   */
}st_sock_pool, *pst_sock_pool;

/************************************************************************
* struct  st_client_cfg
*        ����TCP�ͻ���
************************************************************************/
typedef struct struct_client_cfg{
    st_list net_msg_list;       /* ������Ϣ����      */
    st_list sock_pool_list;     /* �̳߳�Ϣ����      */
	pthread_mutex_t am_cache_mutex; /*����AM���ݿ⻥����*/
	sqlite3 *db;				/*����am���ݿ��ļ�*/
	pthread_t id;
    u32     sock_pool_use_num;  /* �̳߳�ʹ������    */
    s8      client_run;         /* TCP�ͻ������б�־ */
}st_client_cfg, *pst_client_cfg;

static st_client_cfg gs_client_cfg_st = {
    .sock_pool_use_num = 0, /* �̳߳�ʹ������ */
    .client_run = 0,        /* TCP�ͻ������б�־ */
};
/************************************************************************
* @FunctionName( ������ ): pst_client_cfg com_client_cfg_get(void)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����TCP�ͻ�������
*
* @Param(����):
*        None(��)
*
* @ReturnCode(����ֵ):
*        pst_client_cfg
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
pst_client_cfg com_client_cfg_get(void){
    return &gs_client_cfg_st;
}
/**@END! pst_client_cfg com_client_cfg_get(void) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static int _get_db(void *arg, int argc, char **value, char **name)
* @Description(����):      ���ݿ�ص�����
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
static int _get_db(void *arg, int argc, char **value, char **name)
{
	if (argc > 1)
		return 1;
	struct db_arg *parm = arg;
	/*assumer "arg" lenth less than TMP_LEN*/
	/*DBG_PRINTF_S(*value);
	DBG_PRINTF_S(*name);*/
	strncpy(parm->str, value[0], parm->len - 1);
	return 0;
}

/************************************************************************
* @FunctionName( ������ ): static int operate_am_db(const char *file, char *sql, void *arg, int (*handle)
														(void *, int, char **, char **))
* @Description(����):      �������ݿ�
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
static int operate_am_db(const char *file, char *sql, void *arg, int (*handle)
		(void *, int, char **, char **)) 
{
	const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();
	int ret;
#if 0
	const char *file = DBF;
	file = "SRDB.db";
#endif
	
	/*int (*p)(void *, int, char **, char **);*/
	/*log_info(file);*/

	char *err;
	pthread_mutex_lock (&(cpclient_cfg_st->am_cache_mutex));
	ret = sqlite3_exec(cpclient_cfg_st->db, sql, handle, arg, &err);
	pthread_mutex_unlock(&(cpclient_cfg_st->am_cache_mutex));
	if (ret == SQLITE_OK || ret == SQLITE_ABORT)
		ret = 0;
	
	
	return ret;
}

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_client_list_callback_sock_has_net_msg(void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����(NET_MSG_LIST)�����ص�����: SOCK(IP:PORT)��������Ϣ
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
static em_list_operate com_client_list_callback_sock_has_net_msg(void *elem, void *parg, va_list args){
    const pst_net_msg cplist_st = (pst_net_msg)elem;
    const s8 *cpip = (const s8 *)parg;
    u32 port = va_arg(args, u32);
    s32 *pret_s32 = va_arg(args, s32 *);

    if (elem == NULL)
        return operate_remove_em;
    if (parg == NULL)
        return operate_break_em;

    if (port != cplist_st->port)
        return operate_next_em;
    if (strcmp((char *)cpip, (char *)cplist_st->ip))
        return operate_next_em;

    (*pret_s32)++; /* ��SOCK(IP:PORT)������Ϣ */
    return operate_break_em;
}
/**@END! static em_list_operate com_client_list_callback_sock_has_msg(void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static s32 com_client_sock_has_net_msg(s8 *pip, u16 port)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): SOCK(IP:PORT)��������Ϣ
*
* @Param(����):
*       *pip, port - IP, PORT
*
* @ReturnCode(����ֵ):
*        0 - SOCK(IP:PORT)��������Ϣ
*       -1 - SOCK(IP:PORT)��������Ϣ
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static s32 com_client_sock_has_net_msg(s8 *pip, u16 port){
    const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();
    s32 ret_s32 = 0; /* ��Ҫ�޸Ĵ�ֵ */
    com_func_list_iterate(NET_MSG_LIST, &com_client_list_callback_sock_has_net_msg, pip, port, &ret_s32);
    if (ret_s32 != 0)
        return 0;
    return -1;
}
/**@END! static s32 com_client_sock_has_net_msg(s8 *pip, u16 port) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_client_list_callback_sock_get(void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����(SOCK_POOL_LIST)�����ص�����: ����SOCK(IP:PORT)
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
static em_list_operate com_client_list_callback_sock_get(void *elem, void *parg, va_list args){
    const pst_sock_pool plist_st = (pst_sock_pool)elem;
    const s8 *cpip = (s8 *)parg;
    u32  port = va_arg(args, u32);
    s32 *psock = va_arg(args, s32 *);

    if (elem == NULL)
        return operate_remove_em;
    if (parg == NULL)
        return operate_break_em;

    if (port != plist_st->port)
        return operate_next_em;
    if (strcmp((char *)cpip, (char *)plist_st->ip))
        return operate_next_em;
    /* ����SOCK */
    *psock = plist_st->sock;
    return operate_break_em;
}
/**@END! static em_list_operate com_client_list_callback_sock_get(void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static s32 com_client_sock_get(s8 *pip, u16 port, s32 *psock)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����SOCK(IP:PORT)
*
* @Param(����):
*       *pip, port - IP, PORT
*       *psock - ����SOCK
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�
*       -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static s32 com_client_sock_get(s8 *pip, u16 port, s32 *psock){
    const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();
    s32 ret_s32 = 0;
    /* ���ӳ�ȡSOCK(IP:PORT) */
    *psock = -1;
    com_func_list_iterate(SOCK_POOL_LIST, &com_client_list_callback_sock_get, pip, port, psock);
    if ((*psock) != -1)
        return 0;
    /* ���ӳ���, ������Ϣ����SOCK(IP:PORT) */
    *psock = 0;
    ret_s32 = com_client_sock_has_net_msg(pip, port);
    if (ret_s32 != 0)
        return -1;
    return 0;
}
/**@END! static s32 com_client_sock_get(s8 *pip, u16 port, s32 *psock) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_client_list_callback_sock_set(void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����(NET_MSG_LIST)�����ص�����: ����IP:PORTʹ��SOCK
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
static em_list_operate com_client_list_callback_sock_set(void *elem, void *parg, va_list args){
    const pst_net_msg cplist_st = (pst_net_msg)elem;
    const pst_sock_pool cpsock_pool_st = (pst_sock_pool)parg;

    if (elem == NULL)
        return operate_remove_em;
    if (parg == NULL)
        return operate_break_em;

    if (cpsock_pool_st->port != cplist_st->port)
        return operate_next_em;
    if (strcmp((char *)cpsock_pool_st->ip, (char *)cplist_st->ip))
        return operate_next_em;
    /* ����SOCK */
    cplist_st->sock = cpsock_pool_st->sock;
    return operate_next_em;
}
/**@END! static em_list_operate com_client_list_callback_sock_set(void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_client_list_callback_sock_remove(void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����(SOCK_POOL_LIST)�����ص�����: ���ӳ�ɾ��SOCK(IP:PORT)
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
static em_list_operate com_client_list_callback_sock_remove(void *elem, void *parg, va_list args){
    const pst_sock_pool cplist_st = (pst_sock_pool)elem;
    const pst_sock_pool cpsock_pool_st = (pst_sock_pool)parg;
    s32 *pret_s32 = va_arg(args, s32 *);

    if (elem == NULL)
        return operate_remove_em;
    if (parg == NULL)
        return operate_break_em;

    if (cpsock_pool_st->port != cplist_st->port)
        return operate_next_em;
    if (strcmp((char *)cpsock_pool_st->ip, (char *)cplist_st->ip))
        return  operate_next_em;
    /* ɾ��SOCK�ɹ� */
    (*pret_s32)++;
    return (operate_remove_em | operate_break_em);
}
/**@END! static em_list_operate com_client_list_callback_sock_remove(void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_client_list_callback_net_msg_del_by_pack_id(void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����(NET_MSG_LIST)�����ص�����: pack_id��������Ϣ��ɾ��
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
static em_list_operate com_client_list_callback_net_msg_del_by_pack_id(void *elem, void *parg, va_list args){
    const pst_net_msg cplist_st = (pst_net_msg)elem;
    const pst_net_msg cpnet_msg_st = (pst_net_msg)parg;
    TIMER_HANDLE *ptimer_handle = va_arg(args, TIMER_HANDLE *);

    if (elem == NULL)
        return operate_remove_em;
    if (parg == NULL)
        return operate_break_em;

    if (cpnet_msg_st->pack_id != cplist_st->pack_id)
        return operate_next_em;
    /* �����ط���ʱ����� */
    (*ptimer_handle) = cplist_st->timer_handle;
    free(cplist_st); elem = NULL;
    return (operate_remove_em | operate_break_em);
}
/**@END! static em_list_operate com_client_list_callback_net_msg_del_by_pack_id(void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

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
s32 com_client_net_msg_del_by_pack_id(const u32 cpack_id, TIMER_HANDLE *ptimer_handle){
    const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();
    st_net_msg net_msg_st;

    net_msg_st.pack_id = cpack_id;
    *ptimer_handle = TIMER_NONE;

    com_func_list_iterate(NET_MSG_LIST, &com_client_list_callback_net_msg_del_by_pack_id, &net_msg_st, ptimer_handle);

    if ((*ptimer_handle) == TIMER_NONE)
        return -1;
    return 0;
}
/**@END! s32 com_client_net_msg_del_by_pack_id(const u32 cpack_id, TIMER_HANDLE *ptimer_handle) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_client_list_callback_sock_pool_destory(void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��������ص�����: �����̳߳�
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
static em_list_operate com_client_list_callback_sock_pool_destory(void *elem, void *parg, va_list args){
    const pst_sock_pool cplist_st = (pst_sock_pool)elem;

    if (elem != NULL){
        com_func_sock_close(cplist_st->sock);
        cplist_st->sock = 0;
    }

    return operate_remove_em;
}
/**@END! static em_list_operate com_client_list_callback_sock_pool_destory(void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static void *com_client_main_thread(void *parg)
* @Description(����):      ���߳�: TCP�ͻ������߳�
* @Param(����):            pst_sock_pool
************************************************************************/
static void *com_client_main_thread(void *parg){
    const pst_sock_pool cpsock_pool_st = (pst_sock_pool)parg;
    const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();
    u32 rx_len = 0;
    s8 *prx_buf = NULL;
    s32 exist_s32 = 0, ret_s32 = 0;

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): com_client_main_thread() running..\n", __LINE__);

    if (parg == NULL)
        return 0;

    ret_s32 = com_func_list_elem_insert(SOCK_POOL_LIST, cpsock_pool_st);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): com_func_list_elem_insert(SOCK_POOL_LIST)\n", __LINE__);
        return 0;
    }

    cpclient_cfg_st->sock_pool_use_num++;
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): sock pool use(+++) num = (%d)\n", __LINE__, cpclient_cfg_st->sock_pool_use_num);

    com_func_list_iterate(NET_MSG_LIST, &com_client_list_callback_sock_set, cpsock_pool_st); /* ����IP:PORT��SOCK */

    while (cpclient_cfg_st->client_run){
        ret_s32 = com_func_tcp_msg_recv(cpsock_pool_st->sock, &prx_buf, &rx_len, CLIENT_RCV_TIME);
        if (!cpclient_cfg_st->client_run)
            goto exit_goto;
        if (ret_s32 != 0){
            exist_s32 = com_client_sock_has_net_msg(cpsock_pool_st->ip, cpsock_pool_st->port);
            if (exist_s32 != 0) /* ��������Ϣ�˳� */
                goto exit_goto;
            if (ret_s32 == 1) /* ��ʱ�������� */
                continue;
            sleep(1);
            com_func_tcp_server_reconnect(cpsock_pool_st->ip, cpsock_pool_st->port, &cpsock_pool_st->sock, CLIENT_RCV_TIME);
            com_func_list_iterate(NET_MSG_LIST, &com_client_list_callback_sock_set, cpsock_pool_st); /* ����IP:PORT��SOCK */
            continue;
        }
        if ((prx_buf != NULL) && (rx_len > 0))
            com_json_unpack(cpsock_pool_st->sock, prx_buf, rx_len);
    }
exit_goto:
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): exiting tcp client...\n", __LINE__);
    com_func_list_iterate(SOCK_POOL_LIST, &com_client_list_callback_sock_remove, cpsock_pool_st, &ret_s32);
    cpclient_cfg_st->sock_pool_use_num--;
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): sock pool use(---) num = (%d)\n", __LINE__, cpclient_cfg_st->sock_pool_use_num);
    if (prx_buf != NULL)
        free(prx_buf);
    return 0;
}
/**@END! static void *com_client_main_thread(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static void *com_client_connect_thread(void *parg)
* @Description(����):      ���߳�: ����TCP������
* @Param(����):            pst_sock_pool
************************************************************************/
static void *com_client_connect_thread(void *parg){
    const pst_sock_pool cpsock_pool_st = (pst_sock_pool)parg;
    pthread_t thread;
    s32 ret_s32 = 0;
    /* �����߳� */
    pthread_detach(pthread_self());
    if (cpsock_pool_st == NULL)
        return 0;
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): connecting tcp server (%s:%d)..\n", __LINE__, cpsock_pool_st->ip, cpsock_pool_st->port);
    /* ����TCP������ */
    cpsock_pool_st->sock = com_func_tcp_server_connect(cpsock_pool_st->ip, cpsock_pool_st->port, 0);
    if (cpsock_pool_st->sock == -1){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): com_func_tcp_server_connect(%s:%d)\n", __LINE__, cpsock_pool_st->ip, cpsock_pool_st->port);
        free(cpsock_pool_st);
        return 0;
    }
    /* ���ӳɹ�, ����TCP�ͻ������߳� */
    ret_s32 = pthread_create(&thread, NULL, &com_client_main_thread, cpsock_pool_st);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): pthread_create()\n", __LINE__);
        goto exit_goto;
    }
    /* ���շ����߳� */
    ret_s32 = pthread_join(thread, NULL);
    if (ret_s32 != 0)
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): pthread_join()\n", __LINE__);
exit_goto:
    com_func_sock_close(cpsock_pool_st->sock);
    free(cpsock_pool_st);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): exit tcp client finish..(o _ o)..Bye~\n", __LINE__);
    return 0;
}
/**@END! static void *com_client_connect_thread(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static void com_client_destructor(void *parg)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): �ط���������
*
* @Param(����):
*        parg - pst_net_msg
*
* @ReturnCode(����ֵ):
*        None(��)
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static void com_client_destructor(void *parg){
    const pst_net_msg cpnet_msg_st = (pst_net_msg)parg;
    TIMER_HANDLE timer_handle = 0;

    if (cpnet_msg_st == NULL)
        return;

    if (cpnet_msg_st->pbuf_s8 != NULL)
        free(cpnet_msg_st->pbuf_s8);

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): destructor client net msg by pack_id(%d)..\n", __LINE__, cpnet_msg_st->pack_id);
    com_client_net_msg_del_by_pack_id(cpnet_msg_st->pack_id, &timer_handle);
}
/**@END! static void com_client_destructor(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static void com_client_callback_resend(void *parg)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): �ط��ص�����
*
* @Param(����):
*        parg - pst_net_msg
*
* @ReturnCode(����ֵ):
*        None(��)
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static void com_client_callback_resend(void *parg){
    const pst_net_msg cpnet_msg_st = (pst_net_msg)parg;

    if (cpnet_msg_st != NULL)
        com_func_tcp_data_send(cpnet_msg_st->sock, cpnet_msg_st->pbuf_s8, cpnet_msg_st->len_u32);
}
/**@END! static void com_client_callback_resend(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static s32 com_client_net_msg_add(pst_net_msg pnet_msg_st)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ���������Ϣ
*
* @Param(����):
*        net_msg_st - ������Ϣ
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�
*       -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static s32 com_client_net_msg_add(const pst_net_msg cpnet_msg_st){

    const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    TIMEVAL timer_val = 0;
    s32 ret_s32 = 0;

    if (!cpclient_cfg_st->client_run)
    {
        return -1;
    }
    /* ���Ӱ��� */
    cpstCOMManage->pack_id++;
    /* SOCK(IP:PORT)�����򷵻�0 */
    ret_s32 = com_client_sock_get(cpnet_msg_st->ip, cpnet_msg_st->port, &cpnet_msg_st->sock);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): get sock (%d)@(%s:%d)\n", __LINE__, cpnet_msg_st->sock, cpnet_msg_st->ip, cpnet_msg_st->port);
    if (ret_s32 != 0){
        pthread_t connect_thread;
        pst_sock_pool psock_pool_st = (pst_sock_pool)malloc(sizeof(st_sock_pool));
        if (psock_pool_st == NULL){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): malloc()\n", __LINE__);
            return -1;
        }
        psock_pool_st->port = cpnet_msg_st->port;
        strcpy((char *)psock_pool_st->ip, (char *)cpnet_msg_st->ip);
        /* �����߳�����TCP������ */
        ret_s32 = pthread_create(&connect_thread, NULL, &com_client_connect_thread, (void*)psock_pool_st);
		
		
        if (ret_s32 == -1){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): pthread_create()\n", __LINE__);
            free(psock_pool_st);
            return -1;
        }
        timer_val = COM_TIMER_UNIT;
    }
    /* SOCK������.. */
    if (cpnet_msg_st->sock == 0)
        timer_val = COM_TIMER_UNIT;
    {
        pst_net_msg pnet_msg_st = (pst_net_msg)malloc(sizeof(st_net_msg));
        if (pnet_msg_st == NULL){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): malloc()\n", __LINE__);
            return -1;
        }
        memcpy(pnet_msg_st, cpnet_msg_st, sizeof(st_net_msg));
        /* ������Ϣ�������� */
        ret_s32 = com_func_list_elem_insert(NET_MSG_LIST, pnet_msg_st);
        if (ret_s32 != 0){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): com_func_list_elem_insert(NET_MSG_LIST)\n", __LINE__);
            free(pnet_msg_st);
            return -1;
        }
        /* �����ط���ʱ�� */
        pnet_msg_st->timer_handle = com_timer_set(&com_client_callback_resend, &com_client_destructor, pnet_msg_st, timer_val, CLIENT_RESEND_INTERVAL, 3, timer_client_resend_em);
        /* ���붨ʱ��ʧ�� */
        if (pnet_msg_st->timer_handle == TIMER_NONE){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): com_timer_set()\n", __LINE__);
            /* ִ������ */
            com_client_destructor(pnet_msg_st);
            return -1;
        }
    }
    return 0;
}
/**@END! static s32 com_net_msg_add(st_net_msg net_msg_st) !\(^o^)/~ ������ */

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
s32 com_client_tcp_server_sendto(s8 *pip, u16 port, const u32 pack_id, s8 *pbuf, u32 len){
    st_net_msg net_msg_st;
    s32 ret_s32 = 0;

    /* �ͻ�����Ϣ���� */
    net_msg_st.len_u32 = len;
    net_msg_st.pack_id = pack_id;
    net_msg_st.pbuf_s8 = pbuf;
    strcpy((char *)net_msg_st.ip, (char *)pip);
    net_msg_st.port = port;
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): sending msg to tcp server(%s:%d)..\n", __LINE__, net_msg_st.ip, net_msg_st.port);
    ret_s32 = com_client_net_msg_add(&net_msg_st);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): com_client_net_msg_add()\n", __LINE__);
        free(pbuf); pbuf = NULL;
        return -1;
    }
    return 0;
}
/**@END! s32 com_client_tcp_server_sendto(s8 *pip, u16 port, const u32 pack_id, s8 *pbuf, u32 len) !\(^o^)/~ ������ */

/************************************************************************

************************************************************************/
static s32 com_client_net_msg_ack_add(const pst_net_msg cpnet_msg_st){
    const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    TIMEVAL timer_val = 0;
    s32 ret_s32 = 0;

    if (!cpclient_cfg_st->client_run)
        return -1;

    /* SOCK(IP:PORT)�����򷵻�0 */
    ret_s32 = com_client_sock_get(cpnet_msg_st->ip, cpnet_msg_st->port, &cpnet_msg_st->sock);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): get sock (%d)@(%s:%d)\n", __LINE__, cpnet_msg_st->sock, cpnet_msg_st->ip, cpnet_msg_st->port);
    if (ret_s32 != 0){
        pthread_t connect_thread;
        pst_sock_pool psock_pool_st = (pst_sock_pool)malloc(sizeof(st_sock_pool));
        if (psock_pool_st == NULL){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): malloc()\n", __LINE__);
            return -1;
        }
        psock_pool_st->port = cpnet_msg_st->port;
        strcpy((char *)psock_pool_st->ip, (char *)cpnet_msg_st->ip);
        /* �����߳�����TCP������ */
        ret_s32 = pthread_create(&connect_thread, NULL, &com_client_connect_thread, psock_pool_st);
        if (ret_s32 == -1){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): pthread_create()\n", __LINE__);
            free(psock_pool_st);
            return -1;
        }
        timer_val = COM_TIMER_UNIT;
    }
    /* SOCK������.. */
    if (cpnet_msg_st->sock == 0)
        timer_val = COM_TIMER_UNIT;
    {
        pst_net_msg pnet_msg_st = (pst_net_msg)malloc(sizeof(st_net_msg));
        if (pnet_msg_st == NULL){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): malloc()\n", __LINE__);
            return -1;
        }
        memcpy(pnet_msg_st, cpnet_msg_st, sizeof(st_net_msg));
        /* ������Ϣ�������� */
        ret_s32 = com_func_list_elem_insert(NET_MSG_LIST, pnet_msg_st);
        if (ret_s32 != 0){
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): com_func_list_elem_insert(NET_MSG_LIST)\n", __LINE__);
            free(pnet_msg_st);
            return -1;
        }
    }
    return 0;
}
/**@END! static s32 com_net_msg_add(st_net_msg net_msg_st) !\(^o^)/~ ������ */

/************************************************************************

************************************************************************/
s32 com_client_tcp_server_ack_sendto(s8 *pip, u16 port, const u32 pack_id, s8 *pbuf, u32 len){
    st_net_msg net_msg_st;
    s32 ret_s32 = 0;

    /* �ͻ�����Ϣ���� */
    net_msg_st.len_u32 = len;
    net_msg_st.pack_id = pack_id;
    net_msg_st.pbuf_s8 = pbuf;
    strcpy((char *)net_msg_st.ip, (char *)pip);
    net_msg_st.port = port;
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): sending msg to tcp server(%s:%d)..\n", __LINE__, net_msg_st.ip, net_msg_st.port);
    ret_s32 = com_client_net_msg_ack_add(&net_msg_st);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): com_client_net_msg_add()\n", __LINE__);
        free(pbuf); pbuf = NULL;
        return -1;
    }
    return 0;
}


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
s32 com_client_msg_server_sendto(const u32 pack_id, s8 *pbuf, u32 len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpcom_cfg_st = &cpstCOMManage->stCOMCfg;
    s32 ret_s32 = 0;
    ret_s32 = com_client_tcp_server_sendto((s8 *)cpcom_cfg_st->szNetAdminIP, 
            cpcom_cfg_st->usNetAdminTcpPort, pack_id, pbuf, len);
    return ret_s32;
}
/**@END! s32 com_client_msg_server_sendto(const u32 pack_id, s8 *pbuf, u32 len) !\(^o^)/~ ������ */

/************************************************************************
*����ʶ����������������
************************************************************************/

s32 com_client_msg_face_server_sendto(const u32 pack_id, s8 *pbuf, u32 len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpcom_cfg_st = &cpstCOMManage->stCOMCfg;
    s32 ret_s32 = 0;
    ret_s32 = com_client_tcp_server_sendto((s8 *)cpcom_cfg_st->szFaceIP, 
            cpcom_cfg_st->usFacePort, pack_id, pbuf, len);
    return ret_s32;
}

/************************************************************************
*ָ��ʶ����������������
************************************************************************/
s32 com_client_msg_finger_server_sendto(const u32 pack_id, s8 *pbuf, u32 len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpcom_cfg_st = &cpstCOMManage->stCOMCfg;
    s32 ret_s32 = 0;
    ret_s32 = com_client_tcp_server_sendto((s8 *)cpcom_cfg_st->szFingerIP, 
            cpcom_cfg_st->usFingerPort, pack_id, pbuf, len);
    return ret_s32;
}



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
s32 com_client_init(void *parg){
    const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();
	const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    s32 ret_s32 = 0;
	pthread_t id;

	struct db_arg parm;
	char *cmd = (char *)calloc(MAX_LINE_HALF, sizeof(char));
	char *cmd1 = (char *)calloc(MAX_LINE_HALF, sizeof(char));
	parm.str = cmd1;
	parm.len = MAX_LINE_HALF - 1;

    /* ��ʼ������: �̳߳� */
    ret_s32 = com_func_list_init(SOCK_POOL_LIST);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client_init Error(LINE=%d): com_func_list_init()\n", __LINE__);
        return -1;
    }
    /* ��ʼ������: ������Ϣ */
    ret_s32 = com_func_list_init(NET_MSG_LIST);
    if (ret_s32 != 0)
	{
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client_init Error(LINE=%d): com_func_list_init()\n", __LINE__);
        return -1;
    }

	/*��ʼ������AM���ݻ�����*/
	ret_s32 = pthread_mutex_init (&(cpclient_cfg_st->am_cache_mutex),NULL);
	if (ret_s32 != 0)
	{
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client_init Error(LINE=%d): pthread_mutex_init()\n", __LINE__);
        return -1;
	}

	ret_s32 = sqlite3_open(AM_CACHE_DB, &(cpclient_cfg_st->db));	
	//if (ret_s32 != SQLITE_OK) 
	{
		snprintf(cmd, MAX_LINE_HALF - 1, "%s", CREAT_AM_CACHE_TABLE);
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "CREAT_AM_CACHE_TABLE cmd = (%s)", cmd);
		operate_am_db(AM_CACHE_DB, CREAT_AM_CACHE_TABLE, &parm, _get_db);
		//return -1;
	}

	ret_s32 = pthread_create(&(cpclient_cfg_st->id), NULL, com_send_am_data_from_am_db_cache_timer,NULL);
//	ret_s32 = com_send_am_data_from_am_db_cache_timer();
	if(0 != ret_s32)
	{
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "pthread_create com_client_init Error(LINE=%d): com_send_am_data_from_am_db_cache_timer()\n", __LINE__);
	}

	ret_s32 = operate_am_db(AM_CACHE_DB, SUM_AM_DATA_COUNT, &parm, _get_db);
	UT_LOG_LOGOUT_INFO(emModCOM, 0, "CREAT_AM_CACHE_TABLE ret_s32 = (%d)", ret_s32);
	UT_LOG_LOGOUT_INFO(emModCOM, 0, "CREAT_AM_CACHE_TABLE len = (%s)", parm.str);
	if(strcmp("0", parm.str))
	{
		snprintf(cmd, MAX_LINE_HALF - 1, "%s", READ_THE_MAX_PACK_ID);
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "CREAT_AM_CACHE_TABLE cmd = (%s)", cmd);
		operate_am_db(AM_CACHE_DB, READ_THE_MAX_PACK_ID, &parm, _get_db);
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "READ_THE_MAX_PACK_ID pack_id ");
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "READ_THE_MAX_PACK_ID pack_id = (%s)", parm.str);
		//��AmCache.db�����id��ֵ����
		cpstCOMManage->pack_id = atoi(parm.str);
		/* ���Ӱ��� */
	    cpstCOMManage->pack_id++;
	}

	free(cmd);
	free(cmd1);

    return 0;
}
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
s32 com_client_start(void *parg){
    const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();
    cpclient_cfg_st->client_run = 1;
    return 0;
}
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
s32 com_client_stop(void *parg){
    const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();

    /* ֹͣTCP�ͻ������� */
    cpclient_cfg_st->client_run = 0;
    /* �����̳߳� */
    com_func_list_iterate(SOCK_POOL_LIST, &com_client_list_callback_sock_pool_destory, NULL);
    while (cpclient_cfg_st->sock_pool_use_num);

    return 0;
}
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
s32 com_client_uninit(void *parg){
    const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();

	pthread_join(cpclient_cfg_st->id, NULL);
	sqlite3_close(cpclient_cfg_st->db);
    com_func_list_destory(NET_MSG_LIST);
    com_func_list_destory(SOCK_POOL_LIST);

    return 0;
}
/**@END! static s32 com_client_uninit(void *arg) !\(^o^)/~ ������ */

#if 1


/************************************************************************
* @FunctionName( ������ ): static int com_client_net_msg_add(const pst_net_msg cpnet_msg_st)
* @Description(����):      �����Ž�����ʱ���ӷ��͵�����ͻ�����Ϣ
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
static int com_client_cache_am_data_net_msg_add(const pst_net_msg cpnet_msg_st)
{
	const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();
	const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
	TIMEVAL timer_val = 0;
	int ret = 0;
	struct db_arg parm;
	char *cmd = (char *)calloc(MAX_LINE_HALF, sizeof(char));
	char *cmd1 = (char *)calloc(MAX_LINE_HALF, sizeof(char));

	if (!cpclient_cfg_st->client_run)
		return -1;
	//�����͵İ����Զ�����һ
	//cpstCOMManage->pack_id++;

	parm.str = cmd1;
	parm.len = MAX_LINE_HALF - 1;

	operate_am_db(AM_CACHE_DB, SUM_AM_DATA_COUNT, &parm, _get_db);
	if(AM_INFO_CACHE_MAX >= atoi(parm.str))
	{
		snprintf(cmd, MAX_LINE_HALF - 1, "%s ('%d','%s')", INSERT_AM_DATA_TO_TABLE, cpnet_msg_st->pack_id, cpnet_msg_st->pbuf_s8);
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "INSERT_AM_DATA_TO_TABLE cmd = (%s)", cmd);
		operate_am_db(AM_CACHE_DB, cmd, &parm, _get_db);
	}
	else
	{
		operate_am_db(AM_CACHE_DB, DELETE_THE_FIRST_RECORD, &parm, _get_db);
		snprintf(cmd, MAX_LINE_HALF - 1, "%s ('%d','%s')", INSERT_AM_DATA_TO_TABLE, cpnet_msg_st->pack_id, cpnet_msg_st->pbuf_s8);
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "DELETE_INSERT_AM_DATA_TO_TABLE cmd = (%s)", cmd);
		operate_am_db(AM_CACHE_DB, cmd, &parm, _get_db);
	}
/*
	//�����ݷ��뵽DB����
	snprintf(cmd, MAX_LINE_HALF - 1, "%s ('%d','%s')", INSERT_AM_DATA_TO_TABLE, cpnet_msg_st->pack_id, cpnet_msg_st->pbuf_s8);
	operate_am_db(AM_CACHE_DB, cmd, &parm, _get_db);

	snprintf(cmd, MAX_LINE_HALF - 1, "%s '%d'", DELETE_AM_DATA_FROM_TABLE, cpnet_msg_st->pack_id);
	operate_am_db(AM_CACHE_DB, cmd, &parm, _get_db);

	operate_am_db(AM_CACHE_DB, SUM_AM_DATA_COUNT, &parm, _get_db);
	printf("count = %s\n", parm.str);
*/
	free(cmd);
	free(cmd1);
	return 0;
}

/************************************************************************
* @FunctionName( ������ ): int com_client_tcp_server_cache_am_data(const u32 pack_id, s8 *pbuf, u32 len)
* @Description(����):      tcp�����Ž���������
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
int com_client_tcp_server_cache_am_data(s8 *pip, unsigned short port, const u32 pack_id, char *pbuf, u32 len)
{
    st_net_msg net_msg_st;
    int ret = 0;

    /* �ͻ�����Ϣ���� */
    net_msg_st.len_u32 = len;
    net_msg_st.pack_id = pack_id;
    net_msg_st.pbuf_s8 = pbuf;
    strcpy((char *)net_msg_st.ip, (char *)pip);
    net_msg_st.port = port;
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Info(LINE=%d): sending am data msg to tcp server(%s:%d)..\n", __LINE__, net_msg_st.ip, net_msg_st.port);
    ret = com_client_cache_am_data_net_msg_add(&net_msg_st);
    if (ret != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_client Error(LINE=%d): com_client_cache_am_data_net_msg_add()\n", __LINE__);
        //free(pbuf); 
		pbuf = NULL;
        return -1;
    }
    return 0;
}

/************************************************************************
* @FunctionName( ������ ): int com_client_cache_am_data(const u32 pack_id, s8 *pbuf, u32 len)
* @Description(����):      ��δ���͵��Ž����ݻ�������
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
int com_client_cache_am_data(const u32 pack_id, s8 *pbuf, u32 len)
{
	const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpcom_cfg_st = &cpstCOMManage->stCOMCfg;
    int ret = 0;
    ret = 	com_client_tcp_server_cache_am_data((char *)cpcom_cfg_st->szNetAdminIP, 
            cpcom_cfg_st->usNetAdminTcpPort, pack_id, pbuf, len);
    return ret;
}

/************************************************************************
* @FunctionName( ������ ): int com_client_cache_am_data(const u32 pack_id, s8 *pbuf, u32 len)
* @Description(����):      ���Ž����ͳ������ݴ��Ž��������ݿ�ɾ��
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
s32 com_remove_am_cache_data_to_db(u32 pack_id)
{
	struct db_arg parm;
	char *cmd = (char *)calloc(MAX_LINE_HALF, sizeof(char));
	char *cmd1 = (char *)calloc(MAX_LINE_HALF, sizeof(char));

	parm.str = cmd1;
	parm.len = MAX_LINE_HALF - 1;

	snprintf(cmd, MAX_LINE_HALF - 1, "%s %d", DELETE_AM_DATA_FROM_TABLE, pack_id);
	UT_LOG_LOGOUT_INFO(emModCOM, 0, "INSERT_AM_DATA_TO_TABLE cmd = (%s)", cmd);
	operate_am_db(AM_CACHE_DB, cmd, &parm, _get_db);

	free(cmd);
	free(cmd1);
	return 0;

}

/************************************************************************
* @FunctionName( ������ ): void com_com_send_am_data_from_am_db_cache_timer_callback(IN UtTimer utTimer, IN void * Param)
* @Description(����):      ��δ���͵��Ž����ݻ�������
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
void com_com_send_am_data_from_am_db_cache_timer_callback(IN UtTimer utTimer, IN void * Param)
{
	const u32 header_len = sizeof(st_net_msg_header);
	const pst_client_cfg cpclient_cfg_st = com_client_cfg_get();
	UtList* st_listamcache;
	char** pResult;
	char *cmd = (char *)calloc(MAX_LINE_HALF, sizeof(char));
	char *json_data_buf;
	int nRow;
	int nCol;
	int index;
	int ret;
	pstamcache pst_am_cache;

	//st_listamcache = ut_list_prepend(NULL, (void *)pst_am_cache);
	pthread_mutex_lock (&(cpclient_cfg_st->am_cache_mutex));
	ret = sqlite3_get_table(cpclient_cfg_st->db, READ_AM_CACHE, &pResult, &nRow, &nCol, NULL);
	UT_LOG_LOGOUT_INFO(emModCOM, 0, "INSERT_AM_DATA_TO_TABLE cmd = (%s)", *pResult);
	pthread_mutex_unlock (&(cpclient_cfg_st->am_cache_mutex));

	if(0 == nRow && 0 == nCol)
	{
		goto exit;
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "The AmCache.db is NULL!");
	}
	index = nCol;
	int i,j,count = 1;
	if(SQLITE_OK == ret)
	{
		for(i = 0; i < nRow; i++)
		{
			for(j = 0; j < (nCol - 1); j++)
			{
				UT_LOG_LOGOUT_INFO(emModCOM, 0, "nRow = (%d) nCol = (%d) i+j = (%d) i = (%d) j = (%d)", nRow, nCol, i + j, i, j);
				pst_am_cache = (pstamcache)malloc(sizeof(stamcache));
				pst_am_cache->pack_id = atoi(pResult[index]);
				pst_am_cache->json_date = pResult[++index];
				UT_LOG_LOGOUT_INFO(emModCOM, 0, "INSERT_AM_DATA_TO_TABLE pack_id = (%d)", pst_am_cache->pack_id);
				UT_LOG_LOGOUT_INFO(emModCOM, 0, "INSERT_AM_DATA_TO_TABLE json_date = (%s)", pst_am_cache->json_date);

				snprintf(cmd, MAX_LINE_HALF - 1, "%s", pst_am_cache->json_date);
				ret = com_char_to_net_msg(cmd, &json_data_buf);
				if(-1 == ret)
				{
					UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_char_to_net_msg error ");
				}
				UT_LOG_LOGOUT_INFO(emModCOM, 0, "INSERT_AM_DATA_TO_TABLE pack_id = (%d)", pst_am_cache->pack_id);
				UT_LOG_LOGOUT_INFO(emModCOM, 0, "INSERT_AM_DATA_TO_TABLE json_date = (%s)", json_data_buf);
				com_client_msg_server_sendto(pst_am_cache->pack_id, json_data_buf, strlen(cmd) + 8);
				//st_listamcache = ut_list_append(st_listamcache, pst_am_cache);
				//free(pst_am_cache);
				index++;
			}
		}
	}
#if 0
    UtList* elem = st_listamcache;
	while(SD_NULL != elem)
	{
		pstamcache st_am_cache = (pstamcache)elem->data;
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "INSERT_AM_DATA_TO_TABLE cmd = (%d)", st_am_cache->pack_id);
		if(count >= (index/2))
		{
			break;
		}
		//com_client_msg_server_sendto(st_am_cache->pack_id, st_am_cache->json_date, sizeof(st_am_cache->json_date));
		elem = ut_list_next(elem);
		count++;
	}
#endif
exit:
	//ut_list_free(st_listamcache);
	sqlite3_free_table(pResult);
	free(cmd);
	
	
}

/************************************************************************
* @FunctionName( ������ ): int com_send_am_data_from_am_db_cache_timer()
* @Description(����):      ��δ���͵��Ž����ݻ�������
* @Param(����): 
* @ReturnCode(����ֵ):     -1 - ʧ��, >0 - ������Ϣ����
************************************************************************/
void* com_send_am_data_from_am_db_cache_timer()
{
	UtTimer am_timer = 0;
	am_timer = ut_timer_create(emModCOM, 0, "am_db_timer");
	if(am_timer == 0)
	{
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_send_am_data_from_am_db_cache_timer Error(LINE=%d): ut_timer_create()\n", __LINE__);
		return ;
	}
	else
	{
  		ut_timer_start(am_timer, SD_TRUE, 0, ROLL_THE_AM_CACHE_TIMER, com_com_send_am_data_from_am_db_cache_timer_callback, SD_NULL);
	}
	return ;
}


#endif

























































#if (0)
#include "com_ctrl.h"

#include "com_func.h"

#include "com_timer.h"

#define  RECEIVE_TIMEOUT                        10 /* ���ճ�ʱʱ��, ��λ: �� */

/**
* \fn      static void *com_client_SERVERnetadmin_recv_thread(void *ptr)
* \brief   �ͻ��˽����߳�: ������Ϊ������
* \param   void *ptr - pstCOMManage
* \return
* \note
* \todo
* \version V1.0
* \warning ���뱣֤�����ĺϷ���
*/
static void *com_client_SERVERnetadmin_recv_thread(void *ptr){
   LPCOMManage pstCOMManage =(LPCOMManage)ptr;
   SdChar *pszRxBuf = SD_NULL; /* ������յ����� */
   SdInt iRxLen = 0; /* ������յ����ݳ��� */
   SdInt iRet = 0;
   SdInt iSock = 0;
   SdInt iSendNum = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Connect Server NetAdmin success!(^o^) /");

   if(pstCOMManage == NULL)
      goto thread_exit;

   iSock = pstCOMManage->NetAdminConnectTCPSock;

   while(1)
   {
      iRet = com_func_NETmsg_recv(iSock, &pszRxBuf, &iRxLen, RECEIVE_TIMEOUT * 1000);
      if(iRet == 0) /* �����ݽ��� */
      {
         com_json_SERVERnetadmin_ack_unpack(iSock, pszRxBuf, &iRxLen, pstCOMManage);
         iRxLen = 0;
      }
      else
      {
         iSendNum = com_timer_TimerType_num_get(emTimerTypeServerNetAdmin); /* ���͵����ܷ��������� */
//         if(iRet == 1) /* ��ʱ */
//         {
            if(iSendNum == 0) /* ���ط�����, �˳� */
               goto thread_exit; /* ȷ�������߳��������ڳ��ڷ��� */
//         }else
         if((iRet == -1) || (iRet == -2)) /* ���ӹر�, �쳣����, ���� */
         {
            iRet = com_func_TCPserver_reconnect(pstCOMManage->stCOMCfg.szNetAdminIP, pstCOMManage->stCOMCfg.usNetAdminTcpPort, &pstCOMManage->NetAdminConnectTCPSock);
            if(iRet == 0) /* �����ӳɹ� */
            {
               iSock = pstCOMManage->NetAdminConnectTCPSock;
               UT_LOG_LOGOUT_INFO(emModCOM, 0, "ReConnect Server NetAdmin Success.^_^!!!");
            }
            else /* ����ʧ�� */
            {
               sleep(1);
            }
         }
         else /* �쳣�ж� */
         {
            sleep(1);
         }
      }
   }
thread_exit:
   com_func_sock_close(iSock); /* �ر�Socket */

   pstCOMManage->NetAdminConnectTCPSock = 0;

   if(pszRxBuf != NULL)
   {
      ut_mem_free(pszRxBuf);
   }
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Quit thread: Connected Server NetAdmin!(o _ o)..Bye~");
   pthread_exit(0);
}

/**
* \fn      static void *com_client_SERVERnetadmin_thread(void *ptr)
* \brief   �ͻ����߳�: ������Ϊ������
* \param   void *ptr - &stCOMManage
* \return
* \note
* \todo
* \version V1.0
* \warning ���뱣֤�����ĺϷ���
*/
static void *com_client_SERVERnetadmin_thread(void *ptr)
{
   SdInt iRet = 0;
   UtThread ServerRecvThread; /* TCP client �߳� */
   void *res = SD_NULL; /* TCP client recv�̷߳��� */
   LPCOMManage pstCOMManage =(LPCOMManage)ptr;

   pthread_detach(pthread_self());

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Connecting Server NetAdmin...");

   if(pstCOMManage == NULL)
      goto thread_exit;

   iRet = com_func_TCPserver_connect(pstCOMManage->stCOMCfg.szNetAdminIP, pstCOMManage->stCOMCfg.usNetAdminTcpPort, &pstCOMManage->NetAdminConnectTCPSock);
   if(iRet == -1)
      goto thread_exit;

   /* ���������߳� */
   iRet = ut_thread_create(&ServerRecvThread, UT_THREAD_STACK_SIZE_DEFAULT, UT_THREAD_PRIORITY_DEFAULT,
                           com_client_SERVERnetadmin_recv_thread, ptr);
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_thread_create fail.", __FILE__, __LINE__);
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Connect Server NetAdmin fail!(X_X)~");
      goto thread_exit;
   }

   iRet= pthread_join(ServerRecvThread, NULL);
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: pthread_join fail.", __FILE__, __LINE__);
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Connect Server NetAdmin fail!(X_X)~");
      goto thread_exit;
   }

thread_exit:
   pstCOMManage->emCOMState &= ~emComStateWorkNetAdminConnect; /* ������ܷ��������ӱ�־ */
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Quit thread: Connecting Server NetAdmin!(o _ o)..Bye~");
   pthread_exit(0);
}

void com_client_SERVERnetadmin_CallBack(void *ptr)
{
   LPServerNetAdminPara pstServerNetAdminPara = (LPServerNetAdminPara)ptr;
   if(pstServerNetAdminPara == SD_NULL)
      return ;

   com_func_TCPdata_send(*(pstServerNetAdminPara->piSock), pstServerNetAdminPara->pszBuf, pstServerNetAdminPara->iBufLen, 10); /* ��ʱ10milli ���� */ 
}

void com_client_SERVERnetadmin_Destructor(void *ptr)
{
   LPServerNetAdminPara pstServerNetAdminPara = (LPServerNetAdminPara)ptr;
   if(pstServerNetAdminPara == SD_NULL)
      return ;

   if(pstServerNetAdminPara->pszBuf != SD_NULL)
      ut_mem_free(pstServerNetAdminPara->pszBuf); /* �ͷŻ�������Դ */
   if(pstServerNetAdminPara != SD_NULL)
      ut_mem_free(pstServerNetAdminPara); /* �ͷŲ�����Դ */
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Destructor Server NetAdmin.");
}

/**
* \fn      SdInt com_client_SERVERnetadmin_sendto(IN const SdChar *pszBuf, SdUInt iLen, IN LPCOMManage pstCOMManage)
* \brief   �������ݵ����ܷ�����
* \param   IN const SdChar *pszBuf - ����
* \param   SdUInt iLen - ���ݳ���
* \return  �ɹ�, ����(0)
* \return  ʧ��, ����(-1)
* \note
* \todo
* \version V1.0
* \warning ���뱣֤�����ĺϷ���
*/
SdInt com_client_SERVERnetadmin_sendto(IN SdChar *pszBuf, SdUInt iLen){
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPServerNetAdminPara pstServerNetAdminPara = SD_NULL;
   UtThread Thread; /* �������ӷ������߳� */
   SdInt iRet = 0;
   TIMEVAL ValSet = 0; /* �����ӷ�����, ���̷���, ����, COM_TIMER_UNITʱ����� */
   TIMER_HANDLE handle = 0;

   if(pstCOMManage == NULL)
      return -1;

   if(!(pstCOMManage->emCOMState & emComStateWorking)) /* ��δ��working״̬, ��ֹ���� */
      return -1;

   if(!(pstCOMManage->emCOMState & emComStateWorkNetAdminConnect)) /* ������δ����, ���ӷ����� */
   {
      iRet = ut_thread_create(&Thread, UT_THREAD_STACK_SIZE_DEFAULT, 
                  UT_THREAD_PRIORITY_DEFAULT, com_client_SERVERnetadmin_thread, pstCOMManage);
      if(iRet != 0)
      {
         UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_thread_create fail\n", __FILE__, __LINE__);
         return -1;
      }
      pstCOMManage->emCOMState |= emComStateWorkNetAdminConnect; /* ���ӷ����� */
      ValSet = COM_TIMER_UNIT;
   }

   pstServerNetAdminPara = ut_mem_new(ServerNetAdminPara, 1);
   if(pstServerNetAdminPara == SD_NULL)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_mem_new error\n", __FILE__,  __LINE__);
      return -1;
   }

   pstServerNetAdminPara->piSock = &pstCOMManage->NetAdminConnectTCPSock;
   pstServerNetAdminPara->uiPackId = pstCOMManage->uiPackId++; /* ��д���� */
   pstServerNetAdminPara->iBufLen = iLen;
   pstServerNetAdminPara->pszBuf = pszBuf;

   handle = com_timer_set(&com_client_SERVERnetadmin_CallBack, &com_client_SERVERnetadmin_Destructor, pstServerNetAdminPara,
            ValSet, 3000, 3, emTimerTypeServerNetAdmin);

   return 0;
}














void com_client_SERVERterm_CallBack(void *ptr)
{
   LPServerTermPara pstServerTermPara = (LPServerTermPara)ptr;
   if(pstServerTermPara == SD_NULL)
      return ;

   com_func_TCPdata_send(pstServerTermPara->pstTCPServerPara->iSock, pstServerTermPara->pszBuf, pstServerTermPara->iBufLen, 10); /* ��ʱ10milli ���� */ 
}

void com_client_SERVERterm_Destructor(void *ptr)
{
   LPServerTermPara pstServerTermPara = (LPServerTermPara)ptr;
   if(pstServerTermPara == SD_NULL)
      return ;

   if(pstServerTermPara->pszBuf != SD_NULL)
      ut_mem_free(pstServerTermPara->pszBuf); /* �ͷŻ�������Դ */

   if(pstServerTermPara != SD_NULL)
      ut_mem_free(pstServerTermPara); /* �ͷŲ�����Դ */
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Destructor Server Term.");
}

static void *com_client_SERVERterm_recv_thread(void *ptr){
   LPTCPServerPara pstTCPServerPara = (LPTCPServerPara)ptr;
   SdChar *pszRxBuf = SD_NULL; /* ������յ����� */
   SdInt iRxLen = 0; /* ������յ����ݳ��� */
   SdInt iRet = 0;
   SdInt iSock = 0;
   SdInt iSendNum = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Connect Server Term success!(^o^) /");

   if(pstTCPServerPara == NULL)
      goto thread_exit;

   iSock = pstTCPServerPara->iSock;

   while(1)
   {
      iRet = com_func_NETmsg_recv(iSock, &pszRxBuf, &iRxLen, RECEIVE_TIMEOUT * 1000);
      if(iRet == 0) /* �����ݽ��� */
      {
         com_json_SERVERterm_ack_unpack(iSock, pszRxBuf, &iRxLen);
         iRxLen = 0;
      }
      else
      {
         iSendNum = com_timer_TimerType_num_get(emTimerTyperServerTerm); /* ���͵����ܷ��������� */
         if(iSendNum == 0) /* ���ط�����, �˳� */
            goto thread_exit; /* ȷ�������߳��������ڳ��ڷ��� */
         if((iRet == -1) || (iRet == -2)) /* ���ӹر�, �쳣����, ���� */
         {
            iRet = com_func_TCPserver_reconnect(pstTCPServerPara->szIP, pstTCPServerPara->usPort, &pstTCPServerPara->iSock);
            if(iRet == 0) /* �����ӳɹ� */
            {
               iSock = pstTCPServerPara->iSock;
               UT_LOG_LOGOUT_INFO(emModCOM, 0, "ReConnect Server Term Success.^_^!!!");
            }
            else /* ����ʧ�� */
            {
               sleep(1);
            }
         }
         else /* �쳣�ж� */
         {
            sleep(1);
         }
      }
   }
thread_exit:
   com_func_sock_close(iSock); /* �ر�Socket */

   if(pstTCPServerPara != SD_NULL)
      ut_mem_free(pstTCPServerPara);

   if(pszRxBuf != NULL)
      ut_mem_free(pszRxBuf);

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Quit thread: Connected Server Term!(o _ o)..Bye~");
   pthread_exit(0);
}

static void *com_client_SERVERterm_thread(void *ptr)
{
   LPTCPServerPara pstTCPServerPara = (LPTCPServerPara)ptr;
   SdInt iRet = 0;
   UtThread ServerRecvThread; /* TCP client �߳� */
   void *res = SD_NULL; /* TCP client recv�̷߳��� */

   pthread_detach(pthread_self());

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Connecting Server Term...");

   if(pstTCPServerPara == NULL)
      goto thread_exit;

   iRet = com_func_TCPserver_connect(pstTCPServerPara->szIP, pstTCPServerPara->usPort, &pstTCPServerPara->iSock);
   if(iRet == -1)
      goto thread_exit;

   /* ���������߳� */
   iRet = ut_thread_create(&ServerRecvThread, UT_THREAD_STACK_SIZE_DEFAULT, UT_THREAD_PRIORITY_DEFAULT,
                           com_client_SERVERterm_recv_thread, ptr);
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_thread_create fail.", __FILE__, __LINE__);
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Connect Server Term fail!(X_X)~");
      goto thread_exit;
   }

   iRet= pthread_join(ServerRecvThread, NULL);
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: pthread_join fail.", __FILE__, __LINE__);
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Connect Server Term  fail!(X_X)~");
      goto thread_exit;
   }

thread_exit:
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Quit thread: Connecting Server Term !(o _ o)..Bye~");
   pthread_exit(0);
}

SdInt com_client_SERVERterm_sendto(IN SdChar *pszIP, SdUShort  usPort, IN SdChar *pszBuf, SdUInt iLen){
   LPCOMManage pstCOMManage = com_ctrl_module_manage_get();
   LPServerTermPara pstServerTermPara = SD_NULL;
   LPTCPServerPara pstTCPServerPara = SD_NULL;
   UtThread Thread; /* �������ӷ������߳� */
   SdInt iRet = 0;
   TIMER_HANDLE handle = 0;

   if(pstCOMManage == NULL)
      return -1;

   if(!(pstCOMManage->emCOMState & emComStateWorking)) /* ��δ��working״̬, ��ֹ���� */
      return -1;

   pstTCPServerPara = ut_mem_new(TCPServerPara, 1);
   if(pstTCPServerPara == SD_NULL)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_mem_new error\n", __FILE__,  __LINE__);
      return -1;
   }

   pstTCPServerPara->iSock = 0;
   strcpy(pstTCPServerPara->szIP, pszIP);
   pstTCPServerPara->usPort = usPort;
   iRet = ut_thread_create(&Thread, UT_THREAD_STACK_SIZE_DEFAULT, 
                  UT_THREAD_PRIORITY_DEFAULT, com_client_SERVERterm_thread, pstTCPServerPara);
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_thread_create fail\n", __FILE__, __LINE__);
      if(pstTCPServerPara != SD_NULL)
         ut_mem_free(pstTCPServerPara);
      return -1;
   }

   pstServerTermPara = ut_mem_new(ServerTermPara, 1);
   if(pstServerTermPara == SD_NULL)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_mem_new error\n", __FILE__,  __LINE__);
      if(pstTCPServerPara != SD_NULL)
         ut_mem_free(pstTCPServerPara);
      return -1;
   }

   pstServerTermPara->pstTCPServerPara = pstTCPServerPara;
   pstServerTermPara->uiPackId = pstCOMManage->uiPackId++; /* ��д���� */
   pstServerTermPara->iBufLen = iLen;
   pstServerTermPara->pszBuf = pszBuf;

   handle = com_timer_set(&com_client_SERVERterm_CallBack, &com_client_SERVERterm_Destructor, pstServerTermPara,
            COM_TIMER_UNIT, 3000, 3, emTimerTyperServerTerm);

   return 0;
}
#endif
