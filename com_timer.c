/**
 * \file    com_timer.c
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning ��Ȩ����, ��Ȩ�ؾ�
 * \todo    COM�����ʱ��
 */
#include "com_func.h"


#define  TIMER_LIST                     &cptimer_cfg_st->timer_list

/************************************************************************
* struct  st_timer
*        ��ʱ��
************************************************************************/
typedef struct struct_timer{
    com_timer_callback  callback;       /* �ص�����         */
    com_timer_callback  destructor;     /* ��������         */
    void               *parg;           /* �ص���������     */
    TIMEVAL             val;            /* ��һ�ζ�ʱʱ��   */
    TIMEVAL             interval;       /* ���ڶ�ʱʱ��     */
    u32                 times;          /* �ص�����ִ�д��� */
    TIMER_HANDLE        handle;         /* ��ʱ�����       */
    em_timer_type       timer_type_em;  /* ��ʱ������       */
}st_timer, *pst_timer;

/************************************************************************
* struct  st_timer_cfg
*        ���ö�ʱ��
************************************************************************/
typedef struct struct_timer_cfg{
    st_list     timer_list;     /* TCP����������  */
    UtTimer     utimer;         /* UT��ʱ�����   */
    s8          timer_run;      /* ��ʱ�����б�־ */
}st_timer_cfg, *pst_timer_cfg;

static st_timer_cfg gs_timer_cfg_st = {
    .utimer = NULL, /* UT��ʱ�����   */
    .timer_run = 0, /* ��ʱ�����б�־ */
};

/************************************************************************
* @FunctionName( ������ ): pst_client_cfg com_timer_cfg_get(void)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ���ض�ʱ������
*
* @Param(����):
*        None(��)
*
* @ReturnCode(����ֵ):
*        pst_client_cfg
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
pst_timer_cfg com_timer_cfg_get(void){
    return &gs_timer_cfg_st;
}
/**@END! pst_client_cfg com_timer_cfg_get(void) !\(^o^)/~ ������ */

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
    /* ���䶨ʱ���ɹ� */
    if (ret_s32 == 0)
        return s_handle;
    /* ���䶨ʱ��ʧ�� */
    free(ptimer_st);
    return TIMER_NONE;
}
/**@END! TIMER_HANDLE com_timer_set(com_timer_callback callback, com_timer_callback destructor, void *parg,
                        TIMEVAL val, TIMEVAL interval, u32 times, em_timer_type timer_type_em) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_timer_list_callback_handle(const void *elem, void *parg, s32 *parg_s32)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��������ص�����: ��ʱ����ʱ����
*
* @Param(����):
*       *elem - ����Ԫ��
*       *parg - ��ʱ��
*        args - ����б�
*
* @ReturnCode(����ֵ):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
static em_list_operate com_timer_list_callback_handle(void *elem, void *parg, va_list args){
    const pst_timer cplist_st = (pst_timer)elem;

    if (cplist_st == NULL)
        return operate_remove_em;
    /* ˥��ʱ�� */
    if (cplist_st->val > COM_TIMER_UNIT){
        cplist_st->val -= COM_TIMER_UNIT;
        return operate_next_em;
    }
    /* ��ʱ����ʱ, ִ�к����ص� */
    if (cplist_st->callback != NULL)
        cplist_st->callback(cplist_st->parg);
    --cplist_st->times;
    /* ��װ��ʱ�� */
    if (cplist_st->times > 0){
        cplist_st->val = cplist_st->interval;
        return operate_next_em;
    }
    /* �ﵽ�ص�����, ִ���������� */
    if (cplist_st->destructor != NULL)
        cplist_st->destructor(cplist_st->parg);

    free(cplist_st); elem = NULL;
    return operate_remove_em;
}
/**@END! static em_list_operate com_timer_list_callback_handle(const void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

static void com_timer_handle(UtTimer utimer, void *ptr){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    ut_timer_stop(utimer);
    com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_handle, NULL);
    ut_timer_reset(utimer);
}


/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_timer_list_callback_destory(void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��������ص�����: ���ٶ�ʱ������
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
static em_list_operate com_timer_list_callback_destory(void *elem, void *parg, va_list args){
    const pst_timer cplist_st = (pst_timer)elem;

    if (elem != NULL){
        if (cplist_st->destructor != NULL)
            cplist_st->destructor(cplist_st->parg);
        free(cplist_st); elem = NULL;
    }

    return operate_remove_em;
}
/**@END! static em_list_operate com_timer_list_callback_destory(void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_timer_list_callback_del_by_handle(const void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��������ص�����: ����鶨ʱ����ɾ��
*
* @Param(����):
*       *elem - ����Ԫ��
*       *parg - ��ʱ��
*        args - ����б�
*
* @ReturnCode(����ֵ):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
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
/**@END! static em_list_operate com_timer_list_callback_del_by_handle(const void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_timer_remove(TIMER_HANDLE handle)
* @CreateDate  (��������): 2017/01/12
* @Author      ( ��  �� ): CJH
*
* @Description(����): ɾ����ʱ��
*
* @Param(����):
*       handle - ���
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�
*       -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
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
        return -1; /* ɾ���ط���ʱ��ʧ�� */
    return 0;
}
/**@END! s32 com_timer_remove(TIMER_HANDLE handle) !\(^o^)/~ ������ */

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
s32 com_timer_client_resend_remove(u32 pack_id){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    st_timer timer_st;
    s32 ret_s32 = 0;

    if (!cptimer_cfg_st->timer_run)
        return 0;

    ret_s32 = com_client_net_msg_del_by_pack_id(pack_id, &timer_st.handle);
    if (ret_s32 == 0){ /* ɾ��������Ϣ�ɹ� */
        ret_s32 = -1;
        com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_del_by_handle, &timer_st, &ret_s32);
    }
    if (ret_s32 == -1)
        return -1; /* ɾ���ط���ʱ��ʧ�� */
    return 0;
}
/**@END! s32 com_timer_client_resend_remove(u32 pack_id) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_timer_list_callback_unlock_ack_exist(void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2015/12/28
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��������ص�����: ����ACK�Ƿ����
*
* @Param(����):
*       *elem - ����Ԫ��
*       *parg - ��ʱ��
*        args - ����б�
*
* @ReturnCode(����ֵ):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
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
/**@END! static em_list_operate com_timer_list_callback_unlock_ack_exist(void *elem, void *parg, va_list args) !\(^o^)/~ ������ */

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
s32 com_timer_unlock_ack_exist(void *ptr){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    s32 ret_s32 = -1;
    com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_unlock_ack_exist, ptr, &ret_s32);
    if(ret_s32 != -1)
        return 0;
    return -1;
}
/**@END! s32 com_timer_unlock_ack_exist(void *ptr) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): static em_list_operate com_timer_list_callback_unlock_ack_change(void *elem, void *parg, va_list args)
* @CreateDate  (��������): 2015/12/28
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��������ص�����: ����ACK��Ϊ�ɹ�
*
* @Param(����):
*       *elem - ����Ԫ��
*       *parg - ��ʱ��
*        args - ����б�
*
* @ReturnCode(����ֵ):
*        em_list_operate
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
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

    cplist_st->val = 0; /* ���̷��� */
    if(door_state_em == emDoorOpen)
        pelem_st->json_ack_em = ack_true_em;

    (*pret_s32)++;
    return operate_break_em;
}
/**@END! static em_list_operate com_timer_list_callback_unlock_ack_change(void *elem, void *parg, va_list args){ !\(^o^)/~ ������ */

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
    cplist_st->val = 0; /* ���̷��� */
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
s32 com_timer_init(void *parg){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();
    s32 ret_s32 = 0;

    /* ��ʼ������ */
    ret_s32 = com_func_list_init(TIMER_LIST);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_timer Error(LINE=%d): com_func_list_init()\n", __LINE__);
        return -1;
    }

    return 0;
}
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
s32 com_timer_start(void *parg){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();

    cptimer_cfg_st->utimer = ut_timer_create(emModCOM, 0, "com_timer");

    ut_timer_start(cptimer_cfg_st->utimer, /* ��ʱ����� */
                   SD_TRUE,         /* �����Զ�ʱ�� */
                   0,               /* ��ʱ��������(����) */
                   COM_TIMER_UNIT,  /* ��ʱ������ʱ��(��λ: ms) */
                   com_timer_handle,/* ��ʱ���ص����� */
                   NULL);           /* ��ʱ���ص��������� */

    cptimer_cfg_st->timer_run = 1;

    return 0;
}
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
s32 com_timer_stop(void *parg){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();

    /* ֹͣ��ʱ������ */
    cptimer_cfg_st->timer_run = 0;
    /* ���ٶ�ʱ������ */
    com_func_list_iterate(TIMER_LIST, &com_timer_list_callback_destory, NULL);

    return 0;
}
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
s32 com_timer_uninit(void *parg){
    const pst_timer_cfg cptimer_cfg_st = com_timer_cfg_get();

    com_func_list_destory(TIMER_LIST);

    return 0;
}
/**@END! s32 com_timer_uninit(void *parg) !\(^o^)/~ ������ */
































#if (0)
#define  LOCK_COM_TIMER()        ut_mutex_lock(pstCOMManage->utMutexComTimer); /* ���������� */
#define  UNLOCK_COM_TIMER()      ut_mutex_unlock(pstCOMManage->utMutexComTimer); /* �ͷ������� */

typedef void  (*FCallBack)(void *ptr); /* ����ص��������� */

typedef struct tagCOMTimer{
   FCallBack CallBack;      /* �ص�����                      */
   FCallBack Destructor;    /* ��������                      */
   void *ptr;               /* �ص���������                  */
   TIMEVAL Val;             /* ��һ�ζ�ʱʱ��                */
   TIMEVAL Interval;        /* ���ڶ�ʱʱ��, 0, һ���Զ�ʱ�� */
   SdUChar uszTimes;        /* �ص�����ִ�д���              */
   TIMER_HANDLE handle;     /* ��ʱ�����                    */
   enTimerType emTimerType; /* ��ʱ������ */
}COMTimer, *LPCOMTimer;

TIMER_HANDLE com_timer_set(FCallBack CallBack, FCallBack Destructor, void *ptr, 
   TIMEVAL Val, TIMEVAL Interval, SdUChar uszTimes, enTimerType emTimerType)
{
   static TIMER_HANDLE s_handle = 0;
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPCOMTimer pstCOMTimer = SD_NULL;
   if(pstCOMManage == SD_NULL)
      return TIMER_NONE;
   if(CallBack == SD_NULL) /* �ص�����Ϊ�� */
      return TIMER_NONE;
   if(uszTimes == 0)
      return TIMER_NONE;

   pstCOMTimer = ut_mem_new(COMTimer, 1); /* ����һ����ʱ�� */
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

   if(Val == 0) /* ����ִ�лص����� */
   {
      (*pstCOMTimer->CallBack)(pstCOMTimer->ptr);
      pstCOMTimer->Val = Interval;
   }

   LOCK_COM_TIMER(); /* ���������� */
   pstCOMManage->utListComTimer = ut_list_append(pstCOMManage->utListComTimer, pstCOMTimer); /* ��ӵ����� */
   UNLOCK_COM_TIMER(); /* �ͷ������� */
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

   LOCK_COM_TIMER(); /* ���������� */
   Elem = pstCOMManage->utListComTimer;
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* ȡ������һԪ�� */
      TmpElem = Elem;
      Elem = NextElem; /* ָ����һ����ʱ�� */
      if(pstCOMTimer->handle != handle) /* ���Ǹö�ʱ�� */
      {
         continue;
      }
      /* �ҵ���ʱ��, ������Դ */
      pstCOMManage->utListComTimer = ut_list_remove_by_elem(pstCOMManage->utListComTimer, TmpElem);
      if(pstCOMTimer->Destructor != SD_NULL) /* ִ���������� */
      {
         (*pstCOMTimer->Destructor)(pstCOMTimer->ptr);
      }
      ut_mem_free(pstCOMTimer); /* �ͷ���Դ */
      break; /* �˳� */
   }
   UNLOCK_COM_TIMER(); /* �ͷ������� */

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
   LOCK_COM_TIMER(); /* ���������� */
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* ȡ������һԪ�� */
      TmpElem = Elem;
      Elem = NextElem; /* ָ����һ����ʱ�� */
      UNLOCK_COM_TIMER(); /* �ͷ������� */

      if(pstCOMTimer->Val > COM_TIMER_UNIT) /* ˥��ʱ�� */
      {
         pstCOMTimer->Val -= COM_TIMER_UNIT;
         continue;
      }
      /* ��ʱ����ʱ */
      if(pstCOMTimer->CallBack != SD_NULL) /* ִ�к����ص� */
      {
         (*pstCOMTimer->CallBack)(pstCOMTimer->ptr);
      }
      --pstCOMTimer->uszTimes;
      if(pstCOMTimer->uszTimes > 0) /* ��װ��ʱ�� */
      {
         pstCOMTimer->Val = pstCOMTimer->Interval;
         continue;
      }
      LOCK_COM_TIMER(); /* ���������� */
      /* �ﵽ�ص�����, ������Դ */
      pstCOMManage->utListComTimer = ut_list_remove_by_elem(pstCOMManage->utListComTimer, TmpElem);
      if(pstCOMTimer->Destructor != SD_NULL) /* ִ���������� */
      {
         (*pstCOMTimer->Destructor)(pstCOMTimer->ptr);
      }
      ut_mem_free(pstCOMTimer); /* �ͷ���Դ */
   }
   UNLOCK_COM_TIMER(); /* �ͷ������� */
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

   LOCK_COM_TIMER(); /* ���������� */
   Elem = pstCOMManage->utListComTimer;
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* ȡ������һԪ�� */
      if(pstCOMTimer->emTimerType == emTimerType) /* ��ͬ��ʱ������ */
         ++iNum;
      Elem = NextElem; /* ָ����һ����ʱ�� */
   }
   UNLOCK_COM_TIMER(); /* �ͷ������� */

   return iNum;
}

void com_timer_create(LPCOMManage pstCOMManage)
{
   if(pstCOMManage == SD_NULL)
      return ;
   pstCOMManage->utMutexComTimer = ut_mutex_create(); /* ����COMģ�鶨ʱ�������� */

   /* ������ʱ��, Ϊcom_timer�ṩʱ�� */
   pstCOMManage->utTimerComTimer = ut_timer_create(emModCOM, 0, "ComTimer"); 
   ut_timer_start(pstCOMManage->utTimerComTimer, /* ��ʱ�����*/
                  SD_TRUE, /* �����Զ�ʱ��*/
                  0, /* ��ʱ��������(����)*/
                  COM_TIMER_UNIT, /* ��ʱ������ʱ��(��λ: ms)*/
                  com_timer_handle, /* ��ʱ���ص�����*/
                  pstCOMManage); /* ��ʱ���ص���������*/
}

void com_timer_destroy(LPCOMManage pstCOMManage)
{
   ut_timer_stop(pstCOMManage->utTimerComTimer); /* �ر�com_timerʱ�� */

   ut_timer_delete(pstCOMManage->utTimerComTimer);
   pstCOMManage->utTimerComTimer = SD_NULL;

   LOCK_COM_TIMER(); /* ���������� */
   /* �˴����ٶ�ʱ������ */
   {
      LPCOMTimer pstCOMTimer = SD_NULL;
      UtList *Elem = SD_NULL;
      UtList *TmpElem = SD_NULL;
      UtList *NextElem = SD_NULL;
      Elem = pstCOMManage->utListComTimer;
      while(Elem != SD_NULL)
      {
         pstCOMTimer = (LPCOMTimer)Elem->data;
         NextElem = ut_list_next(Elem); /* ȡ������һԪ�� */
         TmpElem = Elem;
         Elem = NextElem; /* ָ����һ����ʱ�� */
         pstCOMManage->utListComTimer = ut_list_remove_by_elem(pstCOMManage->utListComTimer, TmpElem);
         if(pstCOMTimer->Destructor != SD_NULL) /* ִ���������� */
         {
            (*pstCOMTimer->Destructor)(pstCOMTimer->ptr);
         }
         ut_mem_free(pstCOMTimer); /* �ͷ���Դ */
      }
   }
   UNLOCK_COM_TIMER(); /* �ͷ������� */

   ut_mutex_destroy(pstCOMManage->utMutexComTimer); /* ����COMģ�鶨ʱ�������� */
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

   LOCK_COM_TIMER(); /* ���������� */
   Elem = pstCOMManage->utListComTimer;
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* ȡ������һԪ�� */
      TmpElem = Elem;
      Elem = NextElem; /* ָ����һ����ʱ�� */

      if(pstCOMTimer->emTimerType != emTimerTypeSessionUnlock)
      {
         if(pstCOMTimer->emTimerType != emTimerTypeRemoteUnlock)
         {
            continue; /* ��ʱ�����Ͳ��� */
         }
      }
      pstUnlockPara = (LPUnlockPara)pstCOMTimer->ptr;
      if(pstUnlockPara == SD_NULL) /* ����Ϊ�� */
      {
         continue;
      }

      ++iNum;
      pstUnlockPara->emAck = emAckSuccess;
      pstCOMTimer->Val = 0; /* ���̷��� */

      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Change timer(Unlock ack).");
   }
   UNLOCK_COM_TIMER(); /* �ͷ������� */
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

   LOCK_COM_TIMER(); /* ���������� */
   Elem = pstCOMManage->utListComTimer;
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* ȡ������һԪ�� */
      TmpElem = Elem;
      Elem = NextElem; /* ָ����һ����ʱ�� */

      if(pstCOMTimer->emTimerType != emTimerType) /* ��ʱ�����Ͳ��� */
      {
         continue;
      }
      pstUnlockPara = (LPUnlockPara)pstCOMTimer->ptr;
      if(pstUnlockPara == SD_NULL) /* ����Ϊ�� */
      {
         continue;
      }
      if(pstUnlockPara->iPacketNo != uiPackId)
      {
         continue;
      }

      ++iNum;
   }
   UNLOCK_COM_TIMER(); /* �ͷ������� */
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

   LOCK_COM_TIMER(); /* ���������� */
   Elem = pstCOMManage->utListComTimer;
   while(Elem != SD_NULL)
   {
      pstCOMTimer = (LPCOMTimer)Elem->data;
      NextElem = ut_list_next(Elem); /* ȡ������һԪ�� */
      TmpElem = Elem;
      Elem = NextElem; /* ָ����һ����ʱ�� */
      if(pstCOMTimer->emTimerType != emTimerType) /* ��ʱ�����Ͳ��� */
      {
         continue;
      }
      pstServerNetAdminPara = (LPServerNetAdminPara)pstCOMTimer->ptr;
      if(pstServerNetAdminPara == SD_NULL) /* ����Ϊ�� */
      {
         continue;
      }

      if(pstServerNetAdminPara->uiPackId != uiPackId) /* ���Ų�ƥ�� */
      {
         continue;
      }
      pstCOMManage->utListComTimer = ut_list_remove_by_elem(pstCOMManage->utListComTimer, TmpElem);
      if(pstCOMTimer->Destructor != SD_NULL) /* ִ���������� */
      {
         ++iNum;
         (*pstCOMTimer->Destructor)(pstCOMTimer->ptr);
      }

      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Remove timer(TCP client resend) success.");
      ut_mem_free(pstCOMTimer); /* �ͷ���Դ */
      break; /* ��return 0�˳�, ����һ������... */
   }
   UNLOCK_COM_TIMER(); /* �ͷ������� */
   return iNum;
}
#endif
