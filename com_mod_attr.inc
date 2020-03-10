#if defined(SD_OSAL_MULITI_PROCESS_MODE) && !defined(MODULE_COM_ACTIVE)
#else
/**
 * \enum   enSubmodCOMDef
 * \brief  COM内部的子用户模块.
 */
typedef enum tagSubmodCOMDef
{
    emSubmodCOMMain,
    emSubmodCOMCount
}enSubmodCOMDef;

#if 0
SdInt com_msg_handle(const UTMsgDef *pMsg, const void *pContent)
{
    if (pMsg != SD_NULL)
    {
        UT_LOG_TRACE_INFO(emModCOM, 0, "pMsg->usMsgID = %hu, pContent=0x%p", 
                pMsg->usMsgID, pContent);
    }
    else
    {
        UT_LOG_TRACE_INFO(emModCOM, 0, "pMsg = 0x%p, pContent=0x%p", 
                pMsg, pContent);
    }

    return SD_SUCCESS;
}
#endif

extern void com_env_init(void);

static void com_init(void *pPollerFuncPara)
{
    UT_LOG_TRACE_INFO(emModCOM, 0, " ");

   com_env_init();
}

static void com_preprocess(void *pPollerFuncPara)
{
    UT_LOG_TRACE_INFO(emModCOM, 0, " ");
    //com_ctrl_start();
}


static void com_postprocess(void *pPollerFuncPara)
{
    UT_LOG_TRACE_INFO(emModCOM, 0, " ");
    //com_ctrl_stop();
}

static void com_uninit(void *pPollerFuncPara)
{
    UT_LOG_TRACE_INFO(emModCOM, 0, " ");
    //com_ctrl_uninit();
}
#endif

#if defined(SD_OSAL_MULITI_PROCESS_MODE) && !defined(MODULE_COM_ACTIVE)
#else

extern SdInt com_msg_handle(const UTMsgDef *pMsg, const void *pContent);

/**
 * \var     g_stCOMSubmodAttr
 * \brief   COM子用户模块的属性
 */
static UTSubmodAttr g_stCOMMainSubmodAttr = 
        {
            .uiSubmodID     = emSubmodCOMMain,
            .szSubmodName   = "COMMAIN",
            .utThreadAttr   = 
                            {
                                .iStackSize = UT_THREAD_STACK_SIZE_DEFAULT,
                                .iPriority  = UT_THREAD_PRIORITY_DEFAULT
                            },
            .utMQAttr       =
                            {
                                .mq_maxmsg  = UT_MQ_MSG_NUM_MAX,
                                .mq_msgsize = UT_MQ_MSG_LEN_MAX
                            },
            .pPollerFuncPara = SD_NULL,/**< 子模块回调函数参数 */
            .funcInit        = com_init,        /**< 初始化，创建后调用一次 */
            .funcPreprocess  = com_preprocess,  /**< 前置处理，每循环处理调用一次 */
            .funcPostprocess = com_postprocess, /**< 后置处理，每循环处理调用一次 */
            .funcUninit      = com_uninit,      /**< 去初始化，销毁前调用一次 */
            .funcMsgHandle   = com_msg_handle
        };

/**
 * \var     g_arrpCOMSubmodAttr
 * \brief   COM用户模块所属所有子用户模块的属性
 */
static UTSubmodAttr* g_arrpCOMSubmodAttr[] = 
        {
            &g_stCOMMainSubmodAttr
        };
#endif

/**
 * \var COM模块的属性
 */
static UTModAttr g_stCOMModAttr = 
        {
            .uiModID        = (SdUInt)emModCOM,
            .szModName      = "COM",
#if defined(SD_OSAL_MULITI_PROCESS_MODE) && !defined(MODULE_COM_ACTIVE)
            .bActiveMod     = SD_FALSE,
            .uiSubmodCount  = 1,
            .pArrSubmodAttr = g_arrpDefSubmodAttr
#else
            .bActiveMod     = SD_TRUE,
            .uiSubmodCount  = (SdUInt)emSubmodCOMCount,
            .pArrSubmodAttr = g_arrpCOMSubmodAttr
#endif
        };
