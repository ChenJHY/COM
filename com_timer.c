/**
 * \file    com_timer.c
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning 版权所有, 侵权必究
 * \todo    COM软件定时器
 */
#include "com_func.h"


#define  TIMER_LIST                     &cptimer_cfg_st->timer_list

/************************************************************************
* struct  st_timer
*        定时器
************************************************************************/
typedef struct struct_timer{
    com_timer_callback  callback;       /* 回调函数         */
    com_timer_callback  destructor;     /* 析构函数         */
    void               *parg;           /* 回调函数参数     */
    TIMEVAL             val;            /* 第一次定时时间   */
    TIMEVAL             interval;       /* 周期定时时间     */
    u32                 times;          /* 回调函数执行次数 */
    TIMER_HANDLE        handle;         /* 定时器句柄       */
    em_timer_type       timer_type_em;  /* 定时器类型       */
}st_timer, *pst_timer;

/************************************************************************
* struct  st_timer_cfg
*        配置定时器
************************************************************************/
typedef struct struct_timer_cfg{
    st_list     timer_list;     /* TCP服务器链表  */
    UtTimer     utimer;         /* UT定时器句柄   */
    s8          timer_run;      /* 定时器运行标志 */
}st_timer_cfg, *pst_timer_cfg;

static st_timer_cfg gs_timer_cfg_st = {
    .utimer = NULL, /* UT定时器句柄   */
    .timer_run = 0, /* 定时器运行标志 */
};

/************************************************************************
* @FunctionName( 函数名 ): pst_client_cfg com_timer_cfg_get(void)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 返回定时器配置
*
* @Param(参数):
*        None(无)
*
* @ReturnCode(返回值):
*        pst_client_cfg
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
pst_timer_cfg com_timer_cfg_get(void){
    return &gs_timer_cfg_st;
}
/**@END! pst_client_cfg com_timer_cfg_get(void) !\(^o^)/~ 结束咯 */

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
    TIMEVAL val, TIMEVAL interval, u32 times, em_timer_type timer_type_em)
{
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    static TIMER_HANDLE s_handle = 0;
    pst_timer ptimer_st = NULL;
    s32 ret_s32 = 0;

    if (!cptimer_cfg_st->timer_run)
        return TIMER_NONE;

    if (callback == NULL)
        return TIMER_NONE;

    if (val == 0){
        callback(parg);
        if (interval == 0)
            return TIMER_NONE;
        val = interval;
    }

    ptimer_st = (pst_timer)malloc(sizeof(st_timer));
    if (ptimer_st == NULL){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_timer Error(LINE=%d): malloc()\n", __LINE__);
        return TIMER_NONE;
    }

    if (++s_handle > 20000000)
        s_handle = 0;

    ptimer_st->callback = callback;
    ptimer_st->destructor = destructor;
    ptimer_st->parg = parg;
    ptimer_st->val = val;
    ptimer_st->interval = interval;
    ptimer_st->times = times;
    ptimer_st->handle = s_handle;
    ptimer_st->timer_type_em = timer_type_em;

    ret_s32 = com_func_list_elem_insert(TIMER_LIST, ptimer_st);
    /* 分配定时器成功 */
    if (ret_s32 == 0)
        return s_handle;
    /* 分配定时器失败 */
    free(ptimer_st);
    return TIMER_NONE;
}
/**@END! TIMER_HANDLE com_timer_set(com_timer_callback callback, com_timer_callback destructor, void *parg,
                        TIMEVAL val, TIMEVAL interval, u32 times, em_timer_type timer_type_em) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static em_list_operate com_timer_list_callback_handle(const void *elem, void *parg, s32 *parg_s32)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 链表迭代回调函数: 定时器超时处理
*
* @Param(参数):
*       *elem - 链表元素
*       *parg - 定时器
*        args - 变参列表
*
* @ReturnCode(返回值):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
static em_list_operate com_timer_list_callback_handle(void *elem, void *parg, va_list args){
    const pst_timer cplist_st = (pst_timer)elem;

    if (cplist_st == NULL)
        return operate_remove_em;
    /* 衰减时间 */
    if (cplist_st->val > COM_TIMER_UNIT){
        cplist_st->val -= COM_TIMER_UNIT;
        return operate_next_em;
    }
    /* 定时器超时, 执行函数回调 */
    if (cplist_st->callback != NULL)
        cplist_st->callback(cplist_st->parg);
    --cplist_st->times;
    /* 重装定时器 */
    if (cplist_st->times > 0){
        cplist_st->val = cplist_st->interval;
        return operate_next_em;
    }
    /* 达到回调次数, 执行析构函数 */
    if (cplist_st->destructor != NULL)
        cplist_st->destructor(cplist_st->parg);

    free(cplist_st); elem = NULL;
    return operate_remove_em;
}
/**@END! static em_list_operate com_timer_list_callback_handle(const void *elem, void *parg, va_list args) !\(^o^)/~ 结束咯 */

static void com_timer_handle(UtTimer utimer, void *ptr){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    ut_timer_stop(utimer);
    com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_handle, NULL);
    ut_timer_reset(utimer);
}


/************************************************************************
* @FunctionName( 函数名 ): static em_list_operate com_timer_list_callback_destory(void *elem, void *parg, va_list args)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 链表迭代回调函数: 销毁定时器链表
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
static em_list_operate com_timer_list_callback_destory(void *elem, void *parg, va_list args){
    const pst_timer cplist_st = (pst_timer)elem;

    if (elem != NULL){
        if (cplist_st->destructor != NULL)
            cplist_st->destructor(cplist_st->parg);
        free(cplist_st); elem = NULL;
    }

    return operate_remove_em;
}
/**@END! static em_list_operate com_timer_list_callback_destory(void *elem, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static em_list_operate com_timer_list_callback_del_by_handle(const void *elem, void *parg, va_list args)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 链表迭代回调函数: 句柄查定时器并删除
*
* @Param(参数):
*       *elem - 链表元素
*       *parg - 定时器
*        args - 变参列表
*
* @ReturnCode(返回值):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
static em_list_operate com_timer_list_callback_del_by_handle(void *elem, void *parg, va_list args){
    const pst_timer cplist_st = (pst_timer)elem;
    const pst_timer cptimer_st = (pst_timer)parg;
    s32 *pret_s32 = va_arg(args, s32 *);

    if (elem == NULL)
        return operate_next_em;
    if (parg == NULL)
        return operate_break_em;

    if (cptimer_st->handle != cplist_st->handle)
        return operate_next_em;

    (*pret_s32)++;
    free(cplist_st); elem = NULL;
    return (operate_remove_em | operate_break_em);
}
/**@END! static em_list_operate com_timer_list_callback_del_by_handle(const void *elem, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_timer_remove(TIMER_HANDLE handle)
* @CreateDate  (创建日期): 2017/01/12
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 删除定时器
*
* @Param(参数):
*       handle - 句柄
*
* @ReturnCode(返回值):
*        0 - 成功
*       -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_timer_remove(TIMER_HANDLE handle){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    st_timer timer_st;
    s32 ret_s32 = 0;

    if (!cptimer_cfg_st->timer_run)
        return 0;

    ret_s32 = -1;
	timer_st.handle = handle;
    com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_del_by_handle, &timer_st, &ret_s32);
    if (ret_s32 == -1)
        return -1; /* 删除重发定时器失败 */
    return 0;
}
/**@END! s32 com_timer_remove(TIMER_HANDLE handle) !\(^o^)/~ 结束咯 */

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
s32 com_timer_client_resend_remove(u32 pack_id){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    st_timer timer_st;
    s32 ret_s32 = 0;

    if (!cptimer_cfg_st->timer_run)
        return 0;

    ret_s32 = com_client_net_msg_del_by_pack_id(pack_id, &timer_st.handle);
    if (ret_s32 == 0){ /* 删除网络消息成功 */
        ret_s32 = -1;
        com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_del_by_handle, &timer_st, &ret_s32);
    }
    if (ret_s32 == -1)
        return -1; /* 删除重发定时器失败 */
    return 0;
}
/**@END! s32 com_timer_client_resend_remove(u32 pack_id) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static em_list_operate com_timer_list_callback_unlock_ack_exist(void *elem, void *parg, va_list args)
* @CreateDate  (创建日期): 2015/12/28
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 链表迭代回调函数: 开锁ACK是否存在
*
* @Param(参数):
*       *elem - 链表元素
*       *parg - 定时器
*        args - 变参列表
*
* @ReturnCode(返回值):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
static em_list_operate com_timer_list_callback_unlock_ack_exist(void *elem, void *parg, va_list args){
    const pst_timer cplist_st = (pst_timer)elem;
    const pst_unlock_para cpunlock_st = (pst_unlock_para)parg;
    pst_unlock_para pelem_st = NULL;
    s32 *pret_s32 = va_arg(args, s32 *);

    if (elem == NULL)
        return operate_next_em;
    pelem_st = (pst_unlock_para)cplist_st->parg;
    if (pelem_st == NULL)
        return operate_next_em;
    if (parg == NULL)
        return operate_break_em;

    if(cplist_st->timer_type_em != timer_unlock_ack_em)
        return operate_next_em;

    if(pelem_st->uc != cpunlock_st->uc)
        return operate_next_em;

    pelem_st->sock = cpunlock_st->sock;
    (*pret_s32)++;
    return operate_break_em;
}
/**@END! static em_list_operate com_timer_list_callback_unlock_ack_exist(void *elem, void *parg, va_list args) !\(^o^)/~ 结束咯 */

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
s32 com_timer_unlock_ack_exist(void *ptr){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    s32 ret_s32 = -1;
    com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_unlock_ack_exist, ptr, &ret_s32);
    if(ret_s32 != -1)
        return 0;
    return -1;
}
/**@END! s32 com_timer_unlock_ack_exist(void *ptr) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static em_list_operate com_timer_list_callback_unlock_ack_change(void *elem, void *parg, va_list args)
* @CreateDate  (创建日期): 2015/12/28
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 链表迭代回调函数: 开锁ACK改为成功
*
* @Param(参数):
*       *elem - 链表元素
*       *parg - 定时器
*        args - 变参列表
*
* @ReturnCode(返回值):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
static em_list_operate com_timer_list_callback_unlock_ack_change(void *elem, void *parg, va_list args){
    const pst_timer cplist_st = (pst_timer)elem;
    pst_unlock_para pelem_st = NULL;
    enDoorStateDef door_state_em = va_arg(args, enDoorStateDef);
    s32 *pret_s32 = va_arg(args, s32 *);

    if (elem == NULL)
        return operate_next_em;
    pelem_st = (pst_unlock_para)cplist_st->parg;
    if (pelem_st == NULL)
        return operate_next_em;

    if(cplist_st->timer_type_em != timer_unlock_ack_em)
        return operate_next_em;

    cplist_st->val = 0; /* 立刻发送 */
    if(door_state_em == emDoorOpen)
        pelem_st->json_ack_em = ack_true_em;

    (*pret_s32)++;
    return operate_break_em;
}
/**@END! static em_list_operate com_timer_list_callback_unlock_ack_change(void *elem, void *parg, va_list args){ !\(^o^)/~ 结束咯 */

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
s32 com_timer_unlock_change(enDoorStateDef door_state_em){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    s32 ret_s32 = -1;
    com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_unlock_ack_change, NULL, door_state_em, &ret_s32);
    if(ret_s32 != -1)
        return 0;
    return -1;
}


static em_list_operate com_timer_list_callback_lift_call_ack_exist(void *elem, void *parg, va_list args)
{
    const pst_timer cplist_st = (pst_timer)elem;
    const LPLiftParamDef cpunlock_st = (LPLiftParamDef)parg;
    LPLiftParamDef pelem_st = NULL;
    s32 *pret_s32 = va_arg(args, s32 *);

    if (elem == NULL)
    {
        return operate_next_em;
    }
    pelem_st = (LPLiftParamDef)cplist_st->parg;
    if (pelem_st == NULL)
    {
        return operate_next_em;
    }
    if (parg == NULL)
    {
        return operate_break_em;
    }
    if(cplist_st->timer_type_em != emTimerLiftCallAck)
    {
        return operate_next_em;
    }
    if(pelem_st->iUc != cpunlock_st->iUc)
    {
        return operate_next_em;
    }
    pelem_st->iSock = cpunlock_st->iSock;
    (*pret_s32)++;
    return operate_break_em;
}

s32 com_timer_lift_call_ack_exist(void *ptr)
{
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    s32 ret_s32 = -1;
    com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_lift_call_ack_exist, ptr, &ret_s32);
    if(ret_s32 != -1)
        return 0;
    return -1;
}

static em_list_operate com_timer_list_callback_lift_call_ack_change(void *elem, void *parg, va_list args)
{
    const pst_timer cplist_st = (pst_timer)elem;
    LPLiftParamDef pelem_st = NULL;
    em_json_ack emAckReslut = va_arg(args, em_json_ack);
    s32 *pret_s32 = va_arg(args, s32 *);

    if (elem == NULL)
    {
        return operate_next_em;
    }
    pelem_st = (LPLiftParamDef)cplist_st->parg;
    if (pelem_st == NULL)
    {
        return operate_next_em;
    }
    if(cplist_st->timer_type_em != emTimerLiftCallAck)
    {
        return operate_next_em;
    }
    cplist_st->val = 0; /* 立刻发送 */
    if(emAckReslut == ack_true_em)
    {
        pelem_st->emResult = emAckReslut;
    }
    (*pret_s32)++;
    return operate_break_em;
}

s32 com_timer_lift_call_ack_change(em_json_ack emAckResult)
{
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    SdInt iRet = -1;
    com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_lift_call_ack_change, NULL, emAckResult, &iRet);
    if(iRet != -1)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

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
s32 com_timer_init(void *parg){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    s32 ret_s32 = 0;

    /* 初始化链表 */
    ret_s32 = com_func_list_init(TIMER_LIST);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_timer Error(LINE=%d): com_func_list_init()\n", __LINE__);
        return -1;
    }

    return 0;
}
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
s32 com_timer_start(void *parg){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();

    cptimer_cfg_st->utimer = ut_timer_create(emModCOM, 0, "com_timer");

    ut_timer_start(cptimer_cfg_st->utimer, /* 定时器句柄 */
                   SD_TRUE,         /* 周期性定时器 */
                   0,               /* 定时器索引号(类型) */
                   COM_TIMER_UNIT,  /* 定时器周期时间(单位: ms) */
                   com_timer_handle,/* 定时器回调函数 */
                   NULL);           /* 定时器回调函数参数 */

    cptimer_cfg_st->timer_run = 1;

    return 0;
}
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
s32 com_timer_stop(void *parg){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();

    /* 停止定时器运行 */
    cptimer_cfg_st->timer_run = 0;
    /* 销毁定时器链表 */
    com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_destory, NULL);

    return 0;
}
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
s32 com_timer_uninit(void *parg){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();

    com_func_list_destory(TIMER_LIST);

    return 0;
}
/**@END! s32 com_timer_uninit(void *parg) !\(^o^)/~ 结束咯 */
































#if (0)
#define  LOCK_COM_TIMER()        ut_mutex_lock(pstCOMManage->utMutexComTimer); /* 申请链表锁 */
#define  UNLOCK_COM_TIMER()      ut_mutex_unlock(pstCOMManage->utMutexComTimer); /* 释放链表锁 */

typedef void  (*FCallBack)(void *ptr); /* 定义回调函数类型 */

typedef struct tagCOMTimer{
   FCallBack CallBack;      /* 回调函数                      */
   FCallBack Destructor;    /* 析构函数                      */
   void *ptr;               /* 回调函数参数                  */
   TIMEVAL Val;             /* 第一次定时时间                */
   TIMEVAL Interval;        /* 周期定时时间, 0, 一次性定时器 */
   SdUChar uszTimes;        /* 回调函数执行次数              */
   TIMER_HANDLE handle;     /* 定时器句柄                    */
   enTimerType emTimerType; /* 定时器类型 */
}COMTimer, *LPCOMTimer;

TIMER_HANDLE com_timer_set(FCallBack CallBack, FCallBack Destructor, void *ptr, 
   TIMEVAL Val, TIMEVAL Interval, SdUChar uszTimes, enTimerType emTimerType)
{
   static TIMER_HANDLE s_handle = 0;
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPCOMTimer pstCOMTimer = SD_NULL;
   if(pstCOMManage == SD_NULL)
      return TIMER_NONE;
   if(CallBack == SD_NULL) /* 回调函数为空 */
      return TIMER_NONE;
   if(uszTimes == 0)
      return TIMER_NONE;

   pstCOMTimer = ut_mem_new(COMTimer, 1); /* 创建一个定时器 */
   if(pstCOMTimer == SD_NULL)
   {
      return TIMER_NONE;
   }

   if(++s_handle > 20000)
      s_handle = 0;
   pstCOMTimer->CallBack = CallBack;
   pstCOMTimer->Destructor = Destructor;
   pstCOMTimer->ptr = ptr;
   pstCOMTimer->Val = Val;
   pstCOMTimer->Interval = Interval;
   pstCOMTimer->uszTimes = uszTimes;
   pstCOMTimer->handle = s_handle;
   pstCOMTimer->emTimerType = emTimerType;

   if(Val == 0) /* 立刻执行回调函数 */
   {
      (*pstCOMTimer->CallBack)(pstCOMTimer->ptr);
      pstCOMTimer->Val = Interval;
   }

   LOCK_COM_TIMER(); /* 申请链表锁 */
   pstCOMManage->utListComTimer = ut_list_append(pstCOMManage->utListComTimer, pstCOMTimer); /* 添加到链表 */
   UNLOCK_COM_TIMER(); /* 释放链表锁 */
   return s_handle;
}
#if (0)
TIMER_HANDLE com_timer_del(TIMER_HANDLE handle){
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPCOMTimer pstCOMTimer = SD_NULL;
   UtList *Elem = SD_NULL;
   UtList *TmpElem = SD_NULL;
   UtList *NextElem = SD_NULL;

   if(pstCOMManage == SD_NULL)
      return ;

   if(handle == TIMER_NONE)
   {
      return TIMER_NONE;
   }

   LOCK_COM_TIMER(); /* 申请链表锁 */
   Elem = pstCOMManage->utListComTimer;
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* 取链表下一元素 */
      TmpElem = Elem;
      Elem = NextElem; /* 指向下一个定时器 */
      if(pstCOMTimer->handle != handle) /* 不是该定时器 */
      {
         continue;
      }
      /* 找到定时器, 回收资源 */
      pstCOMManage->utListComTimer = ut_list_remove_by_elem(pstCOMManage->utListComTimer, TmpElem);
      if(pstCOMTimer->Destructor != SD_NULL) /* 执行析构函数 */
      {
         (*pstCOMTimer->Destructor)(pstCOMTimer->ptr);
      }
      ut_mem_free(pstCOMTimer); /* 释放资源 */
      break; /* 退出 */
   }
   UNLOCK_COM_TIMER(); /* 释放链表锁 */

   return TIMER_NONE;
}
#endif
void com_timer_handle(UtTimer utTimer, IN void* ptr)
{
   LPCOMManage pstCOMManage = (LPCOMManage)ptr;
   LPCOMTimer pstCOMTimer = SD_NULL;
   UtList *Elem = SD_NULL;
   UtList *TmpElem = SD_NULL;
   UtList *NextElem = SD_NULL;

   if(pstCOMManage == SD_NULL)
      return ;

   ut_timer_stop(utTimer);
   Elem = pstCOMManage->utListComTimer;
   LOCK_COM_TIMER(); /* 申请链表锁 */
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* 取链表下一元素 */
      TmpElem = Elem;
      Elem = NextElem; /* 指向下一个定时器 */
      UNLOCK_COM_TIMER(); /* 释放链表锁 */

      if(pstCOMTimer->Val > COM_TIMER_UNIT) /* 衰减时间 */
      {
         pstCOMTimer->Val -= COM_TIMER_UNIT;
         continue;
      }
      /* 定时器超时 */
      if(pstCOMTimer->CallBack != SD_NULL) /* 执行函数回调 */
      {
         (*pstCOMTimer->CallBack)(pstCOMTimer->ptr);
      }
      --pstCOMTimer->uszTimes;
      if(pstCOMTimer->uszTimes > 0) /* 重装定时器 */
      {
         pstCOMTimer->Val = pstCOMTimer->Interval;
         continue;
      }
      LOCK_COM_TIMER(); /* 申请链表锁 */
      /* 达到回调次数, 回收资源 */
      pstCOMManage->utListComTimer = ut_list_remove_by_elem(pstCOMManage->utListComTimer, TmpElem);
      if(pstCOMTimer->Destructor != SD_NULL) /* 执行析构函数 */
      {
         (*pstCOMTimer->Destructor)(pstCOMTimer->ptr);
      }
      ut_mem_free(pstCOMTimer); /* 释放资源 */
   }
   UNLOCK_COM_TIMER(); /* 释放链表锁 */
   ut_timer_reset(utTimer);
}

SdInt com_timer_TimerType_num_get(enTimerType emTimerType)
{
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPCOMTimer pstCOMTimer = SD_NULL;
   UtList *Elem = SD_NULL;
   UtList *NextElem = SD_NULL;
   SdInt iNum = 0;

   if(pstCOMManage == SD_NULL)
      return ;

   LOCK_COM_TIMER(); /* 申请链表锁 */
   Elem = pstCOMManage->utListComTimer;
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* 取链表下一元素 */
      if(pstCOMTimer->emTimerType == emTimerType) /* 相同定时器类型 */
         ++iNum;
      Elem = NextElem; /* 指向下一个定时器 */
   }
   UNLOCK_COM_TIMER(); /* 释放链表锁 */

   return iNum;
}

void com_timer_create(LPCOMManage pstCOMManage)
{
   if(pstCOMManage == SD_NULL)
      return ;
   pstCOMManage->utMutexComTimer = ut_mutex_create(); /* 申请COM模块定时器链表锁 */

   /* 创建定时器, 为com_timer提供时钟 */
   pstCOMManage->utTimerComTimer = ut_timer_create(emModCOM, 0, "ComTimer"); 
   ut_timer_start(pstCOMManage->utTimerComTimer, /* 定时器句柄*/
                  SD_TRUE, /* 周期性定时器*/
                  0, /* 定时器索引号(类型)*/
                  COM_TIMER_UNIT, /* 定时器周期时间(单位: ms)*/
                  com_timer_handle, /* 定时器回调函数*/
                  pstCOMManage); /* 定时器回调函数参数*/
}

void com_timer_destroy(LPCOMManage pstCOMManage)
{
   ut_timer_stop(pstCOMManage->utTimerComTimer); /* 关闭com_timer时钟 */

   ut_timer_delete(pstCOMManage->utTimerComTimer);
   pstCOMManage->utTimerComTimer = SD_NULL;

   LOCK_COM_TIMER(); /* 申请链表锁 */
   /* 此处销毁定时器链表 */
   {
      LPCOMTimer pstCOMTimer = SD_NULL;
      UtList *Elem = SD_NULL;
      UtList *TmpElem = SD_NULL;
      UtList *NextElem = SD_NULL;
      Elem = pstCOMManage->utListComTimer;
      while(Elem != SD_NULL)
      {
         pstCOMTimer = (LPCOMTimer)Elem->data;
         NextElem = ut_list_next(Elem); /* 取链表下一元素 */
         TmpElem = Elem;
         Elem = NextElem; /* 指向下一个定时器 */
         pstCOMManage->utListComTimer = ut_list_remove_by_elem(pstCOMManage->utListComTimer, TmpElem);
         if(pstCOMTimer->Destructor != SD_NULL) /* 执行析构函数 */
         {
            (*pstCOMTimer->Destructor)(pstCOMTimer->ptr);
         }
         ut_mem_free(pstCOMTimer); /* 释放资源 */
      }
   }
   UNLOCK_COM_TIMER(); /* 释放链表锁 */

   ut_mutex_destroy(pstCOMManage->utMutexComTimer); /* 销毁COM模块定时器链表锁 */
}

SdInt com_timer_UnlockAck_change(void)
{
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPCOMTimer pstCOMTimer = SD_NULL;
   LPUnlockPara pstUnlockPara = SD_NULL;
   UtList *Elem = SD_NULL;
   UtList *TmpElem = SD_NULL;
   UtList *NextElem = SD_NULL;
   SdInt iNum = 0;

   if(pstCOMManage == SD_NULL)
      return ;

   LOCK_COM_TIMER(); /* 申请链表锁 */
   Elem = pstCOMManage->utListComTimer;
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* 取链表下一元素 */
      TmpElem = Elem;
      Elem = NextElem; /* 指向下一个定时器 */

      if(pstCOMTimer->emTimerType != emTimerTypeSessionUnlock)
      {
         if(pstCOMTimer->emTimerType != emTimerTypeRemoteUnlock)
         {
            continue; /* 定时器类型不对 */
         }
      }
      pstUnlockPara = (LPUnlockPara)pstCOMTimer->ptr;
      if(pstUnlockPara == SD_NULL) /* 参数为空 */
      {
         continue;
      }

      ++iNum;
      pstUnlockPara->emAck = emAckSuccess;
      pstCOMTimer->Val = 0; /* 立刻发送 */

      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Change timer(Unlock ack).");
   }
   UNLOCK_COM_TIMER(); /* 释放链表锁 */
   return iNum;
}

SdInt com_timer_UnlockAck_num_get(enTimerType emTimerType, SdUInt uiPackId)
{
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPCOMTimer pstCOMTimer = SD_NULL;
   LPUnlockPara pstUnlockPara = SD_NULL;
   UtList *Elem = SD_NULL;
   UtList *TmpElem = SD_NULL;
   UtList *NextElem = SD_NULL;
   SdInt iNum = 0;

   if(pstCOMManage == SD_NULL)
      return ;

   LOCK_COM_TIMER(); /* 申请链表锁 */
   Elem = pstCOMManage->utListComTimer;
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* 取链表下一元素 */
      TmpElem = Elem;
      Elem = NextElem; /* 指向下一个定时器 */

      if(pstCOMTimer->emTimerType != emTimerType) /* 定时器类型不对 */
      {
         continue;
      }
      pstUnlockPara = (LPUnlockPara)pstCOMTimer->ptr;
      if(pstUnlockPara == SD_NULL) /* 参数为空 */
      {
         continue;
      }
      if(pstUnlockPara->iPacketNo != uiPackId)
      {
         continue;
      }

      ++iNum;
   }
   UNLOCK_COM_TIMER(); /* 释放链表锁 */
   return iNum;
}

SdInt com_timer_client_resend_delete(enTimerType emTimerType, SdUInt uiPackId)
{
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPCOMTimer pstCOMTimer = SD_NULL;
   LPServerNetAdminPara pstServerNetAdminPara = SD_NULL;
   UtList *Elem = SD_NULL;
   UtList *TmpElem = SD_NULL;
   UtList *NextElem = SD_NULL;
   SdInt iNum = 0;

   if(pstCOMManage == SD_NULL)
      return ;

   LOCK_COM_TIMER(); /* 申请链表锁 */
   Elem = pstCOMManage->utListComTimer;
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* 取链表下一元素 */
      TmpElem = Elem;
      Elem = NextElem; /* 指向下一个定时器 */
      if(pstCOMTimer->emTimerType != emTimerType) /* 定时器类型不对 */
      {
         continue;
      }
      pstServerNetAdminPara = (LPServerNetAdminPara)pstCOMTimer->ptr;
      if(pstServerNetAdminPara == SD_NULL) /* 参数为空 */
      {
         continue;
      }

      if(pstServerNetAdminPara->uiPackId != uiPackId) /* 包号不匹配 */
      {
         continue;
      }
      pstCOMManage->utListComTimer = ut_list_remove_by_elem(pstCOMManage->utListComTimer, TmpElem);
      if(pstCOMTimer->Destructor != SD_NULL) /* 执行析构函数 */
      {
         ++iNum;
         (*pstCOMTimer->Destructor)(pstCOMTimer->ptr);
      }

      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Remove timer(TCP client resend) success.");
      ut_mem_free(pstCOMTimer); /* 释放资源 */
      break; /* 用return 0退出, 找了一天问题... */
   }
   UNLOCK_COM_TIMER(); /* 释放链表锁 */
   return iNum;
}
#endif
