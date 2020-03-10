/**
 * \file    com_json.c
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning 版权所有, 侵权必究
 * \todo    COM模块JSON拆包, 打包
 */
#include "com_func.h"
#include "ut/ut_module_timer.h"
#include <sys/time.h>


#define COM_UNLOCK_TIME      8000 /* 开锁超时时间 */

/************************************************************************
* @FunctionName( 函数名 ): int com_char_to_net_msg(char *pbuf, char **ppbuf)
* @Description(描述):      将未发送的门禁数据缓存起来
* @Param(参数): 
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
int com_char_to_net_msg(char *pbuf, char **ppbuf)
{
    const u32 header_len = sizeof(st_net_msg_header);
    s32 ret_s32 = 0;

    s8 *ptmp_buf = pbuf;
    if (ptmp_buf == NULL)
        return -1;

    ret_s32 = strlen((char *)ptmp_buf);
    *ppbuf = malloc(ret_s32 + header_len);
    if ((*ppbuf) == NULL)
        return -1;

    memcpy((*ppbuf) + header_len, ptmp_buf, ret_s32);
    com_func_net_msg_header_pack(*ppbuf, ret_s32);

    ret_s32 += header_len;
    return ret_s32;

}
/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_to_net_msg(json_object *json, s8 **ppbuf)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): JSON -> 网络消息(添加包头)
*
* @Param(参数):
*        *json - JSON
*       **ppbuf - 返回网络消息(malloc分配)
*
* @ReturnCode(返回值):
*        -1 - 失败
*        >0 - 网络消息长度
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_json_to_net_msg(json_object *json, s8 **ppbuf){
    const u32 header_len = sizeof(st_net_msg_header);
    s32 ret_s32 = 0;
	//UT_LOG_LOGOUT_INFO(emModCOM, 0, "INSERT_AM_DATA_TO_TABLE json_date1 = (%s)", json);

    s8 *ptmp_buf = (s8 *)json_object_to_json_string(json); /* 转换成字符串 */
	//UT_LOG_LOGOUT_INFO(emModCOM, 0, "INSERT_AM_DATA_TO_TABLE json_date2 = (%s)", json);
    if (ptmp_buf == NULL)
        return -1;

    ret_s32 = strlen((char *)ptmp_buf) + 1;
    *ppbuf = malloc(ret_s32 + header_len);
    if ((*ppbuf) == NULL)
        return -1;

    memcpy((*ppbuf) + header_len, ptmp_buf, ret_s32);
    com_func_net_msg_header_pack(*ppbuf, ret_s32);

    ret_s32 += header_len;
    return ret_s32;
}
/**@END! static s32 com_json_to_net_msg(json_object *json, s8 **ppbuf) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_pack(const char *pBizType, const char *pSubMsgType, const u32 pack_id, const em_json_ack json_ack_em, s8 **ppbuf,
                                com_json_callback callback, void *parg, ...)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): JSON打包并转换为网络消息
*
* @Param(参数):
*        *pBizType, *pSubMsgType - "BizType", "SubMsgType"
*         pack_id, json_ack_em - 包号, "Result"
*       **ppbuf - 返回网络消息(malloc分配)
*         callback - 打包回调函数
*        *parg, ... - 变参列表
*
* @ReturnCode(返回值):
*        -1 - 失败
*        >0 - 网络消息长度
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
static s32 com_json_pack(const char *pBizType, const char *pSubMsgType, const u32 pack_id, const em_json_ack json_ack_em, s8 **ppbuf,
    com_json_callback callback, void *parg, ...){
    json_object *json = NULL;
    s32 ret_s32 = 0;

    *ppbuf = NULL;

    json = json_object_new_object();
    if (json == NULL)
        return -1;

    json_object_object_add(json, "BizType", json_object_new_string((char *)pBizType));
    json_object_object_add(json, "SubMsgType", json_object_new_string((char *)pSubMsgType));
    json_object_object_add(json, "PacketNo", json_object_new_int(pack_id));
    if ((json_ack_em == ack_false_em) || (json_ack_em == ack_true_em))
        json_object_object_add(json, "Result", json_object_new_int(json_ack_em));

    if (callback != NULL){
        va_list args;
        va_start(args, parg);
        ret_s32 = callback(json, parg, args); /* 回调函数, 添加其他部分 */
        va_end(args);
        if (ret_s32 == -1)
            goto exit_goto;
    }

    ret_s32 = com_json_to_net_msg(json, ppbuf);
exit_goto:
    json_object_put(json); /* 释放资源 */
    return ret_s32;
}
/**@END! static s32 com_json_pack(const char *pBizType, const char *pSubMsgType, const u32 pack_id, const em_json_ack json_ack_em, s8 **ppbuf,
                com_json_callback callback, void *parg, ...) !\(^o^)/~ 结束咯 */

static void com_json_unlock_callback(void *ptr){
    pst_unlock_para punlock_para_st = (pst_unlock_para)ptr;
    char *pBizType = NULL;
    char *pSubMsgType = NULL;
    s32 ret_s32 = 0;

    if(punlock_para_st == NULL)
        return ;

    switch(punlock_para_st->unlock_type_em){
        case emTalkingUnlock:
            pBizType = "Unlock";
            pSubMsgType = "TalkingUnlockACK";
        break;
        case emWatchingUnlock:
            pBizType = "Unlock";
            pSubMsgType = "WatchingUnlockACK";
        break;
        case emPhyManageUnlock: /* 物理管理中心开锁 */
        case emNetAdminUnlock: /* 网管开锁 */
        case emThirdUnlock:/* 第三方开锁 */
            pBizType = "Unlock";
            pSubMsgType = "RemoteACK";
        break;
        default:
        return ;
    }
    {
        s8 *pbuf = NULL;
        ret_s32 = com_json_pack(pBizType, pSubMsgType, punlock_para_st->pack_id, punlock_para_st->json_ack_em, &pbuf, NULL, NULL);
        if (ret_s32 != -1){
            ret_s32 = com_func_tcp_data_send(punlock_para_st->sock, pbuf, ret_s32);
            free(pbuf);
        }
    }
}

static void com_json_unlock_destructor(void *ptr){
    pst_unlock_para punlock_para_st = (pst_unlock_para)ptr;
    if(punlock_para_st == NULL)
        return ;
    free(punlock_para_st); /* 释放缓冲区资源 */
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): Unlock Destructor.\n", __LINE__);
}

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_Message_NoteBrowse_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数: 信息浏览
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static s32 com_json_Message_NoteBrowse_callback(json_object *json, void *parg, va_list args){
    const LPCOMCfgDef cpcom_cfg_st = (LPCOMCfgDef)parg;
    LPReadAreaInfoDef parea_info_st = va_arg(args, LPReadAreaInfoDef);
    json_object *BrowseInfo = NULL;

    if(parea_info_st == NULL)
        return -1;
    BrowseInfo = json_object_new_object();
    if(BrowseInfo == NULL)
        return -1;

    json_object_object_add(json, "BrowseInfo", BrowseInfo); 
    json_object_object_add(BrowseInfo, "uc", json_object_new_int(cpcom_cfg_st->uiUcCode)); 
    json_object_object_add(BrowseInfo, "NoteSn", json_object_new_int(parea_info_st->ulIndex)); 
    json_object_object_add(BrowseInfo, "AttachmentRev", json_object_new_string(parea_info_st->szRevTime)); 
    json_object_object_add(BrowseInfo, "Browse", json_object_new_string(parea_info_st->szBrowseTime)); 

    return 0;
}
/**@END! static s32 com_json_Message_NoteBrowse_callback(json_object *json, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Message_NoteBrowse(void *ptr, int len)
* @Description(描述):      JSON打包及发送网络消息: 信息浏览
* @Param(参数): LPReadAreaInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_Message_NoteBrowse(void *ptr, int len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const u32 pack_id = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    s32 ret_s32 = 0;
    if(len != sizeof(ReadAreaInfoDef))
        return -1;

    ret_s32 = com_json_pack("Message", "NoteBrowse", pack_id, ack_no_em, &pbuf, &com_json_Message_NoteBrowse_callback, &cpstCOMManage->stCOMCfg, ptr);
    if(ret_s32 != -1)
        ret_s32 = com_client_msg_server_sendto(pack_id, pbuf, ret_s32);

    return ret_s32;
}
/**@END! s32 com_json_Message_NoteBrowse(void *ptr, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_VDP_UnlockEvent_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数: 开锁事件
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static s32 com_json_VDP_UnlockEvent_callback(json_object *json, void *parg, va_list args){
    const LPCOMCfgDef cpcom_cfg_st = (LPCOMCfgDef)parg;
    LPDoorRecordDef pdoor_record_st = va_arg(args, LPDoorRecordDef);
    json_object *EventInfo = NULL;
    s32 unlock_type = 0, operate_uc = 0;
    SdChar szAppId[MAX_NAME_LENGTH];

    if(pdoor_record_st == NULL)
        return -1;

    EventInfo = json_object_new_object();
    if(EventInfo == NULL)
        return -1;
    json_object_object_add(json, "EventInfo", EventInfo);

    switch(pdoor_record_st->emUnlockType){
        case emTalkingUnlock: /* 通话过程中开锁 */
            unlock_type = emTalkingUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
        case emWatchingUnlock: /* 监视过程中开锁 */
            unlock_type = emWatchingUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
        case emLocalPasswdUnlock: /* 本地密码开锁 */
            unlock_type = emLocalPasswdUnlock;
            operate_uc = cpcom_cfg_st->uiUcCode;
        break;
        case emEntranceCardUnlock: /* 刷卡开锁 */
            unlock_type = emEntranceCardUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
		case emButtonUnlock: /* 按钮开锁 */
            unlock_type = emButtonUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
        case emRemotePasswdUnlock: /* 远端密码开锁 */
            unlock_type = emRemotePasswdUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
        case emThreatUnlock: /* 胁迫密码开锁 */
            unlock_type = emThreatUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
        case emNetAdminUnlock: /* 远端开锁 */
            unlock_type = emNetAdminUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
        case emPhyManageUnlock: /* 物理管理中心开锁 */
            unlock_type = emPhyManageUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
        case emThirdUnlock: /* 第三方开锁 */
            unlock_type = emThirdUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
        case emPhoneUnlock: /* 手机开锁 */
            unlock_type = emPhoneUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
        case emFaceUnlock: /* 人脸开锁 */
            unlock_type = emFaceUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
        case emFingerUnlock: /* 指纹开锁 */
            unlock_type = emFingerUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
        case emTwoDUnlock: /* 二维码开锁 */
            unlock_type = emTwoDUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
            strncpy(szAppId, pdoor_record_st->szUserName, sizeof(pdoor_record_st->szUserName) - 1);
        break;
        case emBluetoothUnlock: /* 蓝牙开锁 */
            unlock_type = emBluetoothUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
            strncpy(szAppId, pdoor_record_st->szUserName, sizeof(pdoor_record_st->szUserName) - 1);
        break;
        default : /* 不认识的开锁类型 */
            unlock_type = emUnknownUnlock;
            operate_uc = pdoor_record_st->iOperatoruc;
        break;
    }

    json_object_object_add(EventInfo, "EventType", json_object_new_int(unlock_type)); /* 事件类型 */
    json_object_object_add(EventInfo, "Operatoruc", json_object_new_int(operate_uc)); /* 开门者UC */
    json_object_object_add(EventInfo, "Dooruc", json_object_new_int(cpcom_cfg_st->uiUcCode)); /* 开门主机UC */
    json_object_object_add(EventInfo, "DoorNum", json_object_new_int(cpcom_cfg_st->uiDoorNum)); /* 开门主机门号 */
    if(emBluetoothUnlock == (pdoor_record_st->emUnlockType)||emTwoDUnlock == (pdoor_record_st->emUnlockType))
    {
        json_object_object_add(EventInfo, "Appid", json_object_new_string(szAppId));
    }
    
    {
        s8 time_str[64] = {0};
        time_t now_time;
        struct tm *tmt;
        time(&now_time);
        tmt = localtime(&now_time);
        sprintf((char *)time_str, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + tmt->tm_year, tmt->tm_mon + 1,
                tmt->tm_mday, tmt->tm_hour, tmt->tm_min, tmt->tm_sec);
        json_object_object_add(EventInfo, "Date", json_object_new_string((char *)time_str)); /* 开门时间 */
    }
    return 0;
}
/**@END! static s32 com_json_VDP_UnlockEvent_callback(json_object *json, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_VDP_UnlockEvent(void *ptr, int len)
* @Description(描述):      JSON打包及发送网络消息: 开锁事件
* @Param(参数): LPDoorRecordDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_VDP_UnlockEvent(void *ptr, int len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const u32 pack_id = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    s32 ret_s32 = 0;
    if(len != sizeof(DoorRecordDef))
        return -1;
    
    ret_s32 = com_json_pack("VDP", "UnlockEvent", pack_id, ack_no_em, &pbuf, &com_json_VDP_UnlockEvent_callback, &cpstCOMManage->stCOMCfg, ptr);
    if(ret_s32 != -1)
   	{
        ret_s32 = com_client_msg_server_sendto(pack_id, pbuf, ret_s32);
		//ret_s32 = com_client_cache_am_data(pack_id, pbuf + 8, ret_s32);
    }

    return ret_s32;
}
/**@END! s32 com_json_VDP_UnlockEvent(void *ptr, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_VDP_CallInfo_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数: 呼叫事件
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static s32 com_json_VDP_CallInfo_callback(json_object *json, void *parg, va_list args){
   LPCallEventReportDef pcall_record_st = (LPCallEventReportDef)parg;
   s32 unlock = 0; /* 0=未开锁, 1=开锁 */
   s32 call_watch = 0; /* 0=呼叫, 1=监视 */
   json_object *EventInfo = NULL;

   if(pcall_record_st == NULL)
      return -1;

   EventInfo = json_object_new_object();
   if(EventInfo == NULL)
      return -1;
   json_object_object_add(json, "EventInfo", EventInfo);

   if(pcall_record_st->iIfUnLock) /* 0=未开锁, 1=开锁 */
      unlock = TRUE;
   if(pcall_record_st->iType) /* 0=呼叫, 1=监视 */
      call_watch = 1;

   json_object_object_add(EventInfo, "EventType", json_object_new_int(call_watch));
   json_object_object_add(EventInfo, "Start", json_object_new_string(pcall_record_st->pszStartTime)); /* 开始时间 */
   json_object_object_add(EventInfo, "Answer", json_object_new_string(pcall_record_st->pszAnsTime)); /* 开始开始时间 */
   json_object_object_add(EventInfo, "End", json_object_new_string(pcall_record_st->pszHangUpTime)); /* 结束时间 */
   json_object_object_add(EventInfo, "Image", json_object_new_string(pcall_record_st->szImage)); /* 图片 */
   json_object_object_add(EventInfo, "Unlock", json_object_new_boolean(unlock)); /* 有没有开锁 */
   json_object_object_add(EventInfo, "Srcuc", json_object_new_int(pcall_record_st->iSrcuc)); /* 发起方UC */
   json_object_object_add(EventInfo, "Desuc", json_object_new_int(pcall_record_st->iDesuc)); /* 接受方UC */
   json_object_object_add(EventInfo, "State", json_object_new_int(pcall_record_st->iState));
   json_object_object_add(EventInfo, "dd", json_object_new_string(pcall_record_st->stRoomAddrDef.szdd));
   json_object_object_add(EventInfo, "bbb", json_object_new_string(pcall_record_st->stRoomAddrDef.szbbb));
   json_object_object_add(EventInfo, "rr", json_object_new_string(pcall_record_st->stRoomAddrDef.szrr));
   json_object_object_add(EventInfo, "ff", json_object_new_string(pcall_record_st->stRoomAddrDef.szff));
   json_object_object_add(EventInfo, "ii", json_object_new_string(pcall_record_st->stRoomAddrDef.szii));
   return 0;
}
/**@END! static s32 com_json_VDP_CallInfo_callback(json_object *json, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_VDP_CallInfo(void *ptr, int len)
* @Description(描述):      JSON打包及发送网络消息: 呼叫事件
* @Param(参数): LPCallEventReportDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_VDP_CallInfo(void *ptr, int len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const u32 pack_id = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    s32 ret_s32 = 0;
	LPCallEventReportDef pCallEventReport = (LPCallEventReportDef)ptr;
    if(len != sizeof(CallEventReportDef))
        return -1;

	if(pCallEventReport)
	{
		printf("----------COM: %s\n", pCallEventReport->szImage);
		printf("----------COM: %d\n", pCallEventReport->iDesuc);
		printf("----------COM: %s\n", pCallEventReport->stRoomAddrDef.szbbb);
		printf("----------COM: %s\n", pCallEventReport->stRoomAddrDef.szdd);
		printf("----------COM: %s\n", pCallEventReport->stRoomAddrDef.szrr);
		printf("----------COM: %s\n", pCallEventReport->stRoomAddrDef.szff);
		printf("----------COM: %s\n", pCallEventReport->stRoomAddrDef.szii);
		printf("----------COM: %d\n", pCallEventReport->iIfUnLock);
		printf("----------COM: %d\n", pCallEventReport->iSrcuc);
		printf("----------COM: %d\n", pCallEventReport->iState);
		printf("----------COM: %d\n", pCallEventReport->iType);
		printf("----------COM: %s\n", pCallEventReport->pszAnsTime);
		printf("----------COM: %s\n", pCallEventReport->pszHangUpTime);
		printf("----------COM: %s\n", pCallEventReport->pszStartTime);
		if(strlen(pCallEventReport->szImage) > 0) // Upload the image first if the image needed to be uploaded
		{
			ret_s32 = com_json_ftp_ImgInfo(ptr, len);
		}else
		{
			ret_s32 = com_json_pack("VDP", "CallInfo", pack_id, ack_no_em, &pbuf, &com_json_VDP_CallInfo_callback, ptr);
		    if(ret_s32 != -1)
		        ret_s32 = com_client_msg_server_sendto(pack_id, pbuf, ret_s32);
			com_func_msg_send(emModUI, COM_UI_CALL_EVENT_REPORT_ACK, 0, 0, 0, SD_NULL);
		}
	}else
	{
		return -1;
	}

    return ret_s32;
}
/**@END! s32 com_json_VDP_CallInfo(void *ptr, int len) !\(^o^)/~ 结束咯 */

s32 com_json_VDP_CallInfo_2(void *ptr, int len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const u32 pack_id = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    s32 ret_s32 = 0;
	LPCallEventReportDef pCallEventReport = (LPCallEventReportDef)ptr;
    if(len != sizeof(CallEventReportDef))
        return -1;

	if(pCallEventReport)
	{
		printf("----------COM: %s\n", pCallEventReport->szImage);
		printf("----------COM: %d\n", pCallEventReport->iDesuc);
		printf("----------COM: %s\n", pCallEventReport->stRoomAddrDef.szbbb);
		printf("----------COM: %s\n", pCallEventReport->stRoomAddrDef.szdd);
		printf("----------COM: %s\n", pCallEventReport->stRoomAddrDef.szrr);
		printf("----------COM: %s\n", pCallEventReport->stRoomAddrDef.szff);
		printf("----------COM: %s\n", pCallEventReport->stRoomAddrDef.szii);
		printf("----------COM: %d\n", pCallEventReport->iIfUnLock);
		printf("----------COM: %d\n", pCallEventReport->iSrcuc);
		printf("----------COM: %d\n", pCallEventReport->iState);
		printf("----------COM: %d\n", pCallEventReport->iType);
		printf("----------COM: %s\n", pCallEventReport->pszAnsTime);
		printf("----------COM: %s\n", pCallEventReport->pszHangUpTime);
		printf("----------COM: %s\n", pCallEventReport->pszStartTime);
		ret_s32 = com_json_pack("VDP", "CallInfo", pack_id, ack_no_em, &pbuf, &com_json_VDP_CallInfo_callback, ptr);
	    if(ret_s32 != -1)
	        ret_s32 = com_client_msg_server_sendto(pack_id, pbuf, ret_s32);
		com_func_msg_send(emModUI, COM_UI_CALL_EVENT_REPORT_ACK, 0, 0, 0, SD_NULL);
	}else
	{
		return -1;
	}

    return ret_s32;
}


/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_ftp_ImgInfo(void *ptr, int len)
* @Description(描述):      JSON打包及发送ftp消息: 图片事件
* @Param(参数): LPCallEventReportDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/

s32 com_json_ftp_ImgInfo(void *ptr, int len){
	
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
	LPCallEventReportDef pcall_record_st = (LPCallEventReportDef)ptr;
	s32 ii = 0;
	s8	upload[512] = {0}, save[512] = {0};	
	char dbPatch2[512] = {0}; 
    FILE *fp = NULL;
		 
	if(access((char *)DEF_UPLOAD_IMGINFO, F_OK) != 0){ /* 检测不到路径*/
        snprintf(dbPatch2, sizeof(dbPatch2), "mkdir -p %s", DEF_UPLOAD_IMGINFO); /* 生成命令 */
        system(dbPatch2); /* 创建目录 */
 
    }	 
		 
	snprintf(upload, sizeof(upload), "%s%s", DEF_UPLOAD_IMGINFO, pcall_record_st->szImage);
	snprintf((char *)save, sizeof(save), "%s", cpstCOMManage->stCOMCfg.szNetAdminIP);
	
	UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): upload(%s)\n", __LINE__, upload);
	UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): save(%s)\n", __LINE__, save);
	com_ftp_upload(upload, save, UI_COM_CALL_EVENT_REPORT, ptr, len);
	return 0;
}

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_Access_Data_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数: 门禁数据
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static s32 com_json_Access_Data_callback(json_object *json, void *parg, va_list args){
    s32 len_s32 = va_arg(args, s32);
    s8 *pbase64 = NULL;
    if(parg == NULL)
        return -1;

    pbase64 = malloc(len_s32 / 6 * 8 + 16);
    if(pbase64 == NULL)
        return -1;

    com_func_base64_encode(parg, (char *)pbase64, len_s32);
    json_object_object_add(json, "AccessInfo", json_object_new_string((char *)pbase64));
    free(pbase64);

    return 0;
}
/**@END! static s32 com_json_Access_Data_callback(json_object *json, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Access_Data(char *pdata, int len)
* @Description(描述):      JSON打包及发送网络消息: 门禁数据
* @Param(参数): 数据(转Base64编码), 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_Access_Data(char *pdata, int len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const u32 pack_id = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    s32 ret_s32 = 0;

    ret_s32 = com_json_pack("Access", "Data", pack_id, ack_no_em, &pbuf, &com_json_Access_Data_callback, pdata, len);
    if(ret_s32 != -1)
    {
        ret_s32 = com_client_msg_server_sendto(pack_id, pbuf, ret_s32);
    }

    return ret_s32;
}

static void com_json_lift_call_ack_callback(void *ptr)
{
    LPLiftParamDef pstLiftParam = (LPLiftParamDef)ptr;
    s8 * szbuf = NULL;
    SdInt iRet = com_json_pack("Lift", "CallAck", pstLiftParam->uiPackID, pstLiftParam->emResult, &szbuf, NULL, NULL);
    if (iRet != -1)
    {
        iRet = com_func_tcp_data_send(pstLiftParam->iSock, szbuf, iRet);
        ut_mem_free(szbuf);
    }
}

static void com_json_lift_call_ack_destructor(void *ptr)
{
    LPLiftParamDef pstLiftParam = (LPLiftParamDef)ptr;
    if(pstLiftParam == NULL)
    {
        return ;
    }
    ut_mem_free(pstLiftParam); /* 释放缓冲区资源 */
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): lift call ack Destructor.\n", __LINE__); 
}

/**@END! s32 com_json_Access_Data(char *pdata, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_Lift_Schedule_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数: 调度电梯
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static s32 com_json_Lift_Schedule_callback(json_object *json, void *parg, va_list args){
    LPLiftCtrlDef pstLiftCtrl = (LPLiftCtrlDef)parg;
    json_object *Info = NULL;
    if(parg == NULL)
    {
        return -1;
    }
    Info = json_object_new_object();
    if(Info == NULL)
    {
       return -1;
    }
    json_object_object_add(json, "Info", Info);
    json_object_object_add(Info, "Operatoruc", json_object_new_int(pstLiftCtrl->iOptianUC)); /* 操作UC */
    json_object_object_add(Info, "Eventuc", json_object_new_int(pstLiftCtrl->iEventUC));
    json_object_object_add(Info, "StartFloor", json_object_new_int(pstLiftCtrl->iStartFloor)); /* 请求楼层*/
    json_object_object_add(Info, "StartRoom", json_object_new_int(pstLiftCtrl->iStartRoom));
    json_object_object_add(Info, "TargetFloor", json_object_new_int(pstLiftCtrl->iTargetFloor)); /* 目标楼层*/
    json_object_object_add(Info, "TargetRoom", json_object_new_int(pstLiftCtrl->iTargetRoom));
    return 0;
}
/**@END! static s32 com_json_Lift_Schedule_callback(json_object *json, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Lift_Schedule(char *pdata, int len)
* @Description(描述):      JSON打包及发送网络消息: 调度电梯
* @Param(参数): LPRoomAddrDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_Lift_Schedule(char *pdata, int len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const u32 pack_id = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    s32 ret_s32 = 0;
    if(len != sizeof(LiftCtrlDef))
    {
        return -1;
    }
    ret_s32 = com_json_pack("Lift", "Call", pack_id, ack_no_em, &pbuf, &com_json_Lift_Schedule_callback, pdata);
    if(ret_s32 != -1)
    {
        ret_s32 = com_client_tcp_server_sendto((s8 *)cpstCOMManage->stCOMCfg.szLiftIP, (u16)cpstCOMManage->stCOMCfg.usPort, pack_id, pbuf, ret_s32);
    }
    return ret_s32;
}

/**@END! s32 com_json_Lift_Schedule(char *pdata, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_Unlock_CheckPassword_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数: 远端开锁, 校验密码
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static s32 com_json_Unlock_CheckPassword_callback(json_object *json, void *parg, va_list args){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstCOMCfgDef = &cpstCOMManage->stCOMCfg;
    LPRemotePasswdUnlockDef premote_passwd_unlock_st = (LPRemotePasswdUnlockDef)parg;
    json_object *DeviceInfo = NULL;
    if(parg == NULL)
        return -1;

    DeviceInfo = json_object_new_object();
    if(DeviceInfo == NULL)
       return -1;
    json_object_object_add(json, "DeviceInfo", DeviceInfo);
    json_object_object_add(json, "Password", json_object_new_string(premote_passwd_unlock_st->szPasswd));

    json_object_object_add(DeviceInfo, "uc", json_object_new_int(cpstCOMCfgDef->uiUcCode)); /* 主机UC号 */
    json_object_object_add(DeviceInfo, "dd", json_object_new_string(premote_passwd_unlock_st->szdd)); /* 区 */
    json_object_object_add(DeviceInfo, "bbb", json_object_new_string(premote_passwd_unlock_st->szbbb)); /* 栋 */
    json_object_object_add(DeviceInfo, "rr", json_object_new_string(premote_passwd_unlock_st->szrr)); /* 单元 */
    json_object_object_add(DeviceInfo, "ff", json_object_new_string(premote_passwd_unlock_st->szff)); /* 层 */
    json_object_object_add(DeviceInfo, "ii", json_object_new_string(premote_passwd_unlock_st->szii)); /* 房间 */
    
    return 0;
}
/**@END! static s32 com_json_Unlock_CheckPassword_callback(json_object *json, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Unlock_CheckPassword(char *pdata, int len)
* @Description(描述):      JSON打包及发送网络消息: 远端开锁, 校验密码
* @Param(参数): LPRemotePasswdUnlockDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_Unlock_CheckPassword(char *pdata, int len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const u32 pack_id = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    s32 ret_s32 = 0;
    if(len != sizeof(RemotePasswdUnlockDef))
        return -1;

    ret_s32 = com_json_pack("Unlock", "CheckPassword", pack_id, ack_no_em, &pbuf, &com_json_Unlock_CheckPassword_callback, pdata);
    if(ret_s32 != -1)
        ret_s32 = com_client_msg_server_sendto(pack_id, pbuf, ret_s32);

    return ret_s32;
}
/**@END! s32 com_json_Unlock_CheckPassword(char *pdata, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_door_Alarm_Upload_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数: 门口机上传报警信息
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static s32 com_json_door_Alarm_Upload_callback(json_object *json, void *parg, va_list args){
    static int SrcSn = 0;
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstCOMCfg = &cpstCOMManage->stCOMCfg;
    LPEPAlarmInfoDef palarm_info_st = (LPEPAlarmInfoDef)parg;
    json_object *DeviceInfo = NULL;
    json_object *DefenceArea = NULL;
    json_object *arry0 = NULL;
    char buf[16] = {0};
    if(parg == NULL)
        return -1;

    DeviceInfo= json_object_new_object();
    if(DeviceInfo == NULL)
       return -1;
    json_object_object_add(json, "DeviceInfo", DeviceInfo);

    DefenceArea= json_object_new_array();
    if(DefenceArea == NULL)
       return -1;
    json_object_object_add(json, "DefenceArea", DefenceArea);

    arry0= json_object_new_object();
    if(arry0 == NULL)
       return -1;
    json_object_array_add(DefenceArea, arry0);

    {
        struct timeval tv;
        gettimeofday(&tv, NULL); /* 时间 */
        sprintf(buf, "%lu", (unsigned long)(tv.tv_sec * 1000 + tv.tv_usec / 1000));
        json_object_object_add(json, "Now", json_object_new_string(buf));
    }

    json_object_object_add(DeviceInfo, "community", json_object_new_string(cpstCOMCfg->szCommunityCode));
    json_object_object_add(DeviceInfo, "uc", json_object_new_int(cpstCOMCfg->uiUcCode)); 
    json_object_object_add(DeviceInfo, "fc", json_object_new_int(cpstCOMCfg->uiFcCode));
    json_object_object_add(DeviceInfo, "dd", json_object_new_string(cpstCOMCfg->szdd));
    json_object_object_add(DeviceInfo, "bbb", json_object_new_string(cpstCOMCfg->szbbb));
    json_object_object_add(DeviceInfo, "rr", json_object_new_string(cpstCOMCfg->szrr));
    json_object_object_add(DeviceInfo, "ff", json_object_new_string(cpstCOMCfg->szff));
    json_object_object_add(DeviceInfo, "ii", json_object_new_string(cpstCOMCfg->szii));

    json_object_object_add(arry0, "SrcSn", json_object_new_int(++SrcSn)); /* 报警流水号 */
    json_object_object_add(arry0, "AreaNum", json_object_new_int(0)); /* 防区号 */
    palarm_info_st->szDescr[MAX_ALARM_INFO_LEN - 1] = '\0';
    json_object_object_add(arry0, "Descr", json_object_new_string(palarm_info_st->szDescr)); /* 报警描述 */
    json_object_object_add(arry0, "State", json_object_new_int(2)); /* 2, 已设防, 报警未处理 */
    json_object_object_add(arry0, "Type", json_object_new_int(1)); /* 1, 24小时防区 */

    return 0;
}
/**@END! static s32 com_json_door_Alarm_Upload_callback(json_object *json, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_door_AC_Upload_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数: 门口机上传报警信息
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static s32 com_json_door_AC_Upload_callback(json_object *json, void *parg, va_list args){
    LPCardInfoDef palarm_info_st = (LPCardInfoDef)parg;
    json_object *ACEventInfo = NULL;
    json_object *arry0 = NULL;
    char buf[16] = {0};
	char strEventType[6] = {0};
    if(parg == NULL)
        return -1;

    ACEventInfo = json_object_new_object();
    if(ACEventInfo == NULL)
       return -1;
    json_object_object_add(json, "ACEventInfo", ACEventInfo);


    s8 time_str[64] = {0};
    time_t now_time;
    struct tm *tmt;
    time(&now_time);
    tmt = localtime(&now_time);
    sprintf((char *)time_str, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + tmt->tm_year, tmt->tm_mon + 1,
            tmt->tm_mday, tmt->tm_hour, tmt->tm_min, tmt->tm_sec);
    

    json_object_object_add(ACEventInfo, "CardId", json_object_new_int(palarm_info_st->iCardId));
	sprintf(strEventType, "%d", palarm_info_st->iEventType);
    json_object_object_add(ACEventInfo, "EventType", json_object_new_string(strEventType)); 
    json_object_object_add(ACEventInfo, "Timestamp", json_object_new_string((char *)time_str));
    json_object_object_add(ACEventInfo, "DoorId", json_object_new_int(palarm_info_st->iDoorId));
    json_object_object_add(ACEventInfo, "IOType", json_object_new_int(palarm_info_st->emIOType));

    return 0;
}
/**@END! static s32 com_json_door_AC_Upload_callback(json_object *json, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_call_alert_app_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数: 门口机上传报警信息
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static s32 com_json_call_alert_app_callback(json_object *json, void *parg, va_list args){
    LPCallAlertAppInfoDef palarm_info_st = (LPCallAlertAppInfoDef)parg;
    json_object *DeviceInfo = NULL, *Transfer = NULL;
    json_object *arry0 = NULL;
    char buf[16] = {0};
	char strEventType[6] = {0};
    if(parg == NULL)
        return -1;

	const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const u32 pack_id = cpstCOMManage->pack_id;
	Transfer   = json_object_new_object();
	if(Transfer == NULL)
		return -1;
	json_object_object_add(json, "Transfer", Transfer);
	json_object_object_add(Transfer, "BizType", json_object_new_string("CallingApp"));
	json_object_object_add(Transfer, "PacketNo", json_object_new_int(pack_id));

    DeviceInfo = json_object_new_object();
    if(DeviceInfo == NULL)
       return -1;
    json_object_object_add(Transfer, "DeviceInfo", DeviceInfo);

    json_object_object_add(DeviceInfo, "uc", json_object_new_int(palarm_info_st->iUc));
    json_object_object_add(DeviceInfo, "dd", json_object_new_string(palarm_info_st->stRoomAddrDef.szdd)); 
    json_object_object_add(DeviceInfo, "bbb", json_object_new_string(palarm_info_st->stRoomAddrDef.szbbb));
    json_object_object_add(DeviceInfo, "rr", json_object_new_string(palarm_info_st->stRoomAddrDef.szrr));
    json_object_object_add(DeviceInfo, "ff", json_object_new_string(palarm_info_st->stRoomAddrDef.szff));
	json_object_object_add(DeviceInfo, "ii", json_object_new_string(palarm_info_st->stRoomAddrDef.szii));
	json_object_object_add(DeviceInfo, "communityId", json_object_new_string(palarm_info_st->szCommunity));

    return 0;
}
/**@END! static s32 com_json_call_alert_app_callback(json_object *json, void *parg, va_list args) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_door_Alarm_Upload(char *pdata, int len)
* @Description(描述):      JSON打包及发送网络消息: 远端开锁, 校验密码
* @Param(参数): LPEPAlarmInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_AC_record_upload(char *pdata, int len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const u32 pack_id = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    s32 ret_s32 = 0;
    if(len != sizeof(CardInfoDef))
        return -1;

	
    ret_s32 = com_json_pack("AC", "UploadEvent", pack_id, ack_no_em, &pbuf, &com_json_door_AC_Upload_callback, pdata);
    if(ret_s32 != -1)
    {
    	
        ret_s32 = com_client_msg_server_sendto(pack_id, pbuf, ret_s32);
		ret_s32 = com_client_cache_am_data(pack_id, pbuf + 8, ret_s32);
    }

    return ret_s32;
}
/**@END! s32 com_json_door_Alarm_Upload(char *pdata, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_call_alert_app(char *pdata, int len)
* @Description(描述):      JSON打包及发送网络消息: 远端开锁, 校验密码
* @Param(参数): LPEPAlarmInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_call_alert_app(char *pdata, int len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const u32 pack_id = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    s32 ret_s32 = 0;
    if(len != sizeof(CallAlertAppInfoDef))
        return -1;

	
    ret_s32 = com_json_pack("Terminal", "Transfer", pack_id, ack_no_em, &pbuf, &com_json_call_alert_app_callback, pdata);
    if(ret_s32 != -1)
        ret_s32 = com_client_msg_server_sendto(pack_id, pbuf, ret_s32);

    return ret_s32;
}
/**@END! s32 com_json_call_alert_app(char *pdata, int len) !\(^o^)/~ 结束咯 */


/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_door_Alarm_Upload(char *pdata, int len)
* @Description(描述):      JSON打包及发送网络消息: 远端开锁, 校验密码
* @Param(参数): LPEPAlarmInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_door_Alarm_Upload(char *pdata, int len){
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const u32 pack_id = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    s32 ret_s32 = 0;
    if(len != sizeof(EPAlarmInfoDef))
        return -1;

    ret_s32 = com_json_pack("Alarm", "Upload", pack_id, ack_no_em, &pbuf, &com_json_door_Alarm_Upload_callback, pdata);
    if(ret_s32 != -1)
        ret_s32 = com_client_msg_server_sendto(pack_id, pbuf, ret_s32);

    return ret_s32;
}



/************************************************************************
* @FunctionName( 函数名 ): static SdInt com_json_Face_RequestFaceSync_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数:请求人脸数据同步
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static SdInt com_json_Face_RequestFaceSync_callback(json_object *json, void *parg, va_list args)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstCOMCfg = &cpstCOMManage->stCOMCfg;
    LPFaceVerInfoDef pstFaceVerInfo = (LPFaceVerInfoDef)parg;
    
    json_object *Info = NULL;
    
    if(parg == NULL)
        return -1;

    Info= json_object_new_object();
    if(Info == NULL)
       return -1;
    json_object_object_add(json, "Info", Info);

    json_object_object_add(Info, "Operatoruc", json_object_new_int(cpstCOMCfg->uiUcCode));
    json_object_object_add(Info, "VersionInfo", json_object_new_string(pstFaceVerInfo->szVerInfo));

    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Face_RequestFaceSync_callback");

    return 0;
}

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Face_RequestFaceSync(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:请求人脸数据同步
* @Param(参数):
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_RequestFaceSync(SdChar *pdata, SdInt len)
{
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Face_RequestFaceSync11");
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const SdInt ciPackId = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    SdInt ret_s32 = 0;
    if(len != sizeof(FaceVerInfoDef))
        return -1;

    ret_s32 = com_json_pack("FACE", "RequestFaceSync", ciPackId, ack_no_em, &pbuf, &com_json_Face_RequestFaceSync_callback, pdata);
    if(ret_s32 != -1)
    {
        ret_s32 = com_client_msg_face_server_sendto(ciPackId, pbuf, ret_s32);
    }

//    printf("pbuf:%s\n",*pbuf);
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Face_RequestFaceSync");
    return ret_s32;
}
#if(0)
/************************************************************************
* @FunctionName( 函数名 ): static SdInt com_json_Face_RequestFaceSyncAck_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数:请求人脸数据同步应答
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static SdInt com_json_Face_RequestFaceSyncAck_callback(json_object *json, void *parg, va_list args)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstCOMCfg = &cpstCOMManage->stCOMCfg;
    LPFtpInfoDef pstFtpInfo = (LPFtpInfoDef)parg;
    
    json_object *Info = NULL;
    
    if(parg == NULL)
        return -1;

    Info= json_object_new_object();
    if(Info == NULL)
       return -1;
    json_object_object_add(json, "Info", Info);

    json_object_object_add(Info, "FtpUser", json_object_new_string(pstFtpInfo->szFtpUser));
    json_object_object_add(Info, "FtpPasswd", json_object_new_string(pstFtpInfo->szFtpPasswd));
    json_object_object_add(Info, "FtpIpAddr", json_object_new_string(pstFtpInfo->szFtpIpAddr));
    json_object_object_add(Info, "FtpPath", json_object_new_string(pstFtpInfo->szFtpPath));
    json_object_object_add(Info, "Update", json_object_new_int(pstFtpInfo->bUpdate));

}

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Face_RequestFaceSyncAck(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:请求人脸数据同步应答
* @Param(参数): 
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_RequestFaceSyncAck(SdChar *pdata, SdInt len)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const SdInt ciPackId = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    SdInt ret_s32 = 0;
    if(len != sizeof(FtpInfoDef))
        return -1;

    ret_s32 = com_json_pack("Face", "RequestFaceSyncAck", ciPackId, ack_no_em, &pbuf, &com_json_Face_RequestFaceSyncAck_callback, pdata);
    //if(ret_s32 != -1)
        //ret_s32 = com_func_msg_send(emModFACE, COM_FACE_FACE_SYNC_REQUEST_ACK, 0, 0, sizeof(FtpInfoDef), &pbuf);
    
    return ret_s32;
}
#endif


/************************************************************************
* @FunctionName( 函数名 ): static SdInt com_json_Face_FaceSyncCheck_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数:人脸数据同步检测
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static SdInt com_json_Face_FaceSyncCheck_callback(json_object *json, void *parg, va_list args)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstCOMCfg = &cpstCOMManage->stCOMCfg;
    LPFaceVerInfoDef pstFaceVerInfo = (LPFaceVerInfoDef)parg;
    
    json_object *Info = NULL;
    
    if(parg == NULL)
        return -1;

    Info= json_object_new_object();
    if(Info == NULL)
       return -1;
    json_object_object_add(json, "Info", Info);

    json_object_object_add(Info, "Operatoruc", json_object_new_int(cpstCOMCfg->uiUcCode));
    json_object_object_add(Info, "VersionInfo", json_object_new_string(pstFaceVerInfo->szVerInfo));

    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Face_FaceSyncCheck_callback");

    return 0;
}

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Face_FaceSyncCheck(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:人脸数据同步检测
* @Param(参数): 
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_FaceSyncCheck(SdChar *pdata, SdInt len)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const SdInt ciPackId = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    SdInt ret_s32 = 0;
    if(len != sizeof(FaceVerInfoDef))
        return -1;

    ret_s32 = com_json_pack("FACE", "FaceSyncCheck", ciPackId, ack_no_em, &pbuf, &com_json_Face_FaceSyncCheck_callback, pdata);
    if(ret_s32 != -1)
    {
        ret_s32 = com_client_msg_face_server_sendto(ciPackId, pbuf, ret_s32);
    }

    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Face_FaceSyncCheck");
    
    return ret_s32;

}

#if(0)
/************************************************************************
* @FunctionName( 函数名 ): static SdInt com_json_Face_FaceSync_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数:人脸数据同步
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static SdInt com_json_Face_FaceSync_callback(json_object *json, void *parg, va_list args)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstCOMCfg = &cpstCOMManage->stCOMCfg;
    LPDoorRecordDef pdoor_record_st = va_arg(args, LPDoorRecordDef);
    LPFaceVerInfoDef pstFaceVerInfo = (LPFaceVerInfoDef)parg;
    
    json_object *Info = NULL;
    
    if(parg == NULL)
        return -1;

    Info= json_object_new_object();
    if(Info == NULL)
       return -1;
    json_object_object_add(json, "Info", Info);

    json_object_object_add(Info, "Operatoruc", json_object_new_int(pdoor_record_st->iOperatoruc));
    json_object_object_add(Info, "VersionInfo", json_object_new_string(pstFaceVerInfo->szVersionInfo));

}

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Face_FaceSync(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:人脸数据同步
* @Param(参数): 
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_FaceSync(SdChar *pdata, SdInt len)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const SdInt ciPackId = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    SdInt ret_s32 = 0;
    if(len != sizeof(FaceVerInfoDef))
        return -1;

    ret_s32 = com_json_pack("Face", "FaceSync", ciPackId, ack_no_em, &pbuf, &com_json_Face_FaceSync_callback, pdata);
    if(ret_s32 != -1)
        ret_s32 = com_client_msg_server_sendto(ciPackId, pbuf, ret_s32);

    return ret_s32;

}
#endif


/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Face_FaceSyncAck(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:人脸数据同步Ack
* @Param(参数):
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_FaceSyncAck(SdBool Result, const SdInt iSock)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const SdInt ciPackId = cpstCOMManage->pack_id;
    const SdBool bResult = (SdBool)Result;
    const SdInt  sock    = iSock;
    s8* pbuf = NULL;
    SdInt ret_s32 = 0;
    ret_s32 = com_json_pack("FACE", "FaceSyncAck", ciPackId, bResult, &pbuf, NULL, NULL);
    if(ret_s32 != -1)
    {
        ret_s32 = com_func_tcp_data_send(sock ,pbuf,ret_s32);
    }
    if(NULL != pbuf)
    {
        free(pbuf);
    }
    return ret_s32;

}


/************************************************************************
* @FunctionName( 函数名 ): static SdInt com_json_Finger_Char_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数:指纹网络匹配
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static SdInt com_json_Finger_Char_callback(json_object *json, void *parg, va_list args)
{
    SdInt len_s32 = va_arg(args, SdInt);
    SdChar *pcbase64 = NULL;
    json_object *Info = NULL;
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstCOMCfg = &cpstCOMManage->stCOMCfg;

    if(parg == NULL)
        return -1;

    pcbase64 = malloc(len_s32 / 6 * 8 + 16);
    if(NULL == pcbase64)
        return -1;
    
    com_func_base64_encode(parg ,(SdChar *)pcbase64 ,len_s32);
    
    Info= json_object_new_object();
    if(Info == NULL)
       return -1;
    json_object_object_add(json, "Info", Info);

    json_object_object_add(Info, "Operatoruc", json_object_new_int(cpstCOMCfg->uiUcCode));
    json_object_object_add(Info, "FingerMatch", json_object_new_string((SdChar *)pcbase64));

    free(pcbase64);

    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Finger_Char_callback");

    return 0;
}

void com_Finger_char_timer_callback(void *parg)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    com_func_msg_send(emModUI, COM_UI_FINGER_MATCH, 2, 0, 0, NULL);
    printf("%s %d\r\n", __FUNCTION__, __LINE__);
    cpstCOMManage->iFingerTimer = -1;
}

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Finger_Char(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:指纹网络匹配
* @Param(参数): 
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Char(SdChar *pdata, SdInt len)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const SdInt ciPackId = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    SdInt iret = 0;

    iret = com_json_pack("FINGER", "FingerChar", ciPackId, ack_no_em, &pbuf, &com_json_Finger_Char_callback, pdata, len);
    if(iret != -1)
    {
        iret = com_client_msg_finger_server_sendto(ciPackId, pbuf, iret);
    }

    if(cpstCOMManage->iFingerTimer != -1) //定时器已经存在
    {
        com_timer_remove(cpstCOMManage->iFingerTimer);
        cpstCOMManage->iFingerTimer = -1;
    }

    cpstCOMManage->iFingerTimer = com_timer_set(&com_Finger_char_timer_callback,NULL,NULL,10000,1,1,emDufault);
    if(cpstCOMManage->iFingerTimer == -1) //创建定时器失败
    {
        UT_LOG_LOGOUT_DEBUG(emModCOM,5,"creat the Finger_Char timer failed");
    }
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Finger_Char");
    
    return iret;
}

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Finger_Match_Ack(SdBool Result, const SdInt iSock)
* @Description(描述):      JSON打包及发送网络消息:指纹匹配Ack
* @Param(参数):
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Match_Ack(SdBool Result, const SdInt iSock)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const SdInt ciPackId = cpstCOMManage->pack_id;
    const SdBool bResult = (SdBool)Result;
    const SdInt  sock    = iSock;
    s8* pbuf = NULL;
    SdInt ret_s32 = 0;
    ret_s32 = com_json_pack("FINGER", "FingerMatchAck", ciPackId, bResult, &pbuf, NULL, NULL);
    if(ret_s32 != -1)
    {
        ret_s32 = com_func_tcp_data_send(sock ,pbuf,ret_s32);
    }
    if(NULL != pbuf)
    {
        free(pbuf);
    }
    return ret_s32;

}

/************************************************************************
* @FunctionName( 函数名 ): static SdInt com_json_Finger_Sync_Request_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数:指纹数据同步请求
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static SdInt com_json_Finger_Sync_Request_callback(json_object *json, void *parg, va_list args)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstCOMCfg = &cpstCOMManage->stCOMCfg;
    LPFingerVerInfoDef pstFingerVerInfo = (LPFingerVerInfoDef)parg;
    
    json_object *Info = NULL;
    
    if(parg == NULL)
        return -1;

    Info= json_object_new_object();
    if(Info == NULL)
       return -1;
    json_object_object_add(json, "Info", Info);

    json_object_object_add(Info, "Operatoruc", json_object_new_int(cpstCOMCfg->uiUcCode));
    json_object_object_add(Info, "VersionInfo", json_object_new_string(pstFingerVerInfo->szVerInfo));

    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Finger_Sync_Request_callback");

    return 0;
}

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Finger_Sync_Request(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:指纹数据同步请求
* @Param(参数): 
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Sync_Request(SdChar *pdata, SdInt len)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const SdInt ciPackId = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    SdInt  iret = 0;

    iret = com_json_pack("FINGER" ,"RequestFPSync" ,ciPackId ,ack_no_em ,&pbuf ,com_json_Finger_Sync_Request_callback ,pdata);
    if(-1 != iret)
       {
        iret = com_client_msg_finger_server_sendto(ciPackId ,pbuf ,iret);
       }
    
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Finger_Sync_Request");
    
    return iret;
}

/************************************************************************
* @FunctionName( 函数名 ): static SdInt com_json_Finger_Sync_Check_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数:指纹数据同步检测
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static SdInt com_json_Finger_Sync_Check_callback(json_object *json, void *parg, va_list args)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstCOMCfg = &cpstCOMManage->stCOMCfg;
    LPFingerVerInfoDef pstFingerVerInfo = (LPFingerVerInfoDef)parg;
    
    json_object *Info = NULL;
    
    if(parg == NULL)
        return -1;

    Info= json_object_new_object();
    if(Info == NULL)
       return -1;
    json_object_object_add(json, "Info", Info);

    json_object_object_add(Info, "Operatoruc", json_object_new_int(cpstCOMCfg->uiUcCode));
    json_object_object_add(Info, "VersionInfo", json_object_new_string(pstFingerVerInfo->szVerInfo));

    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Finger_Sync_Check_callback");

    return 0;
}

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Finger_Sync_Check(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:指纹数据同步检测
* @Param(参数):
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Sync_Check(SdChar *pdata, SdInt len)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const SdInt ciPackId = cpstCOMManage->pack_id;
    s8 *pbuf = NULL;
    SdInt  iret = 0;

    iret = com_json_pack("FINGER" ,"FPSyncCheck" ,ciPackId ,ack_no_em ,&pbuf ,com_json_Finger_Sync_Check_callback ,pdata);
    if(-1 != iret)
    {
        iret = com_client_msg_finger_server_sendto(ciPackId ,pbuf ,iret);
    }
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Finger_Sync_Check");
    
    return iret;
}

#if 0
/************************************************************************
* @FunctionName( 函数名 ): static SdInt com_json_Finger_Finger_Sync_callback(json_object *json, void *parg, va_list args)
* @Description(描述):      回调函数:指纹同步命令
* @Param(参数):            *json, *parg, args - JSON, 回调函数参数, 变参列表
* @ReturnCode(返回值):     -1 - 错误, 0 - 成功
************************************************************************/
static SdInt com_json_Finger_Finger_Sync_Ack_callback(json_object *json, void *parg, va_list args)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const LPCOMCfgDef cpstCOMCfg = &cpstCOMManage->stCOMCfg;
    SdInt iResult = (SdInt)parg;
    
    json_object *Info = NULL;
    
    if(parg == NULL)
        return -1;

    Info= json_object_new_object();
    if(Info == NULL)
       return -1;
    json_object_object_add(json, "Result", json_object_new_int(iResult));

    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Finger_Finger_Sync_callback");

    return 0;
}
#endif


/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Finger_Finger_Sync_Ack(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:指纹同步命令
* @Param(参数):
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Finger_Sync_Ack(SdBool Result, const SdInt iSock)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    const SdInt ciPackId = cpstCOMManage->pack_id;
    const SdBool bResult = (SdBool)Result;
    const SdInt  sock    = iSock;
    s8 *pbuf = NULL;
    SdInt ret_s32 = 0;

    ret_s32 = com_json_pack("FINGER", "FingerSyncAck", ciPackId, bResult, &pbuf, NULL, NULL);
    if(ret_s32 != -1)
    {
        ret_s32 = com_func_tcp_data_send(sock ,pbuf,ret_s32);
    }
    if(NULL != pbuf)
    {
        free(pbuf);
    }
    return ret_s32;
}







/**@END! s32 com_json_door_Alarm_Upload(char *pdata, int len) !\(^o^)/~ 结束咯 */

/*************************************************************************************************************************/
/*********************************************************** U N P A C K *************************************************/
/********************************************************* U N P A C K ***************************************************/
/******************************************************* U N P A C K ****************************************************/
/***************************************************** U N P A C K *******************************************************/
/*************************************************** U N P A C K *********************************************************/
/************************************************* U N P A C K ***********************************************************/
/*************************************************************************************************************************/

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_Message_NoteRelease_handle(json_object *json)
* @Description(描述): 处理: 信息发布
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     0 - 成功, 其他 - 错误
************************************************************************/
static s32 com_json_Message_NoteRelease_handle(json_object *json, const s32 sock){
    json_object *json_tmp = NULL;
    json_object *json_obj = NULL;
    s8 *pstr = NULL;
    AreaInfoDef stAreaInfoDef;
    s32 pack_id = 0, ret_s32 = 0;

    memset(&stAreaInfoDef, 0, sizeof(stAreaInfoDef));

    /* 包号 */
    json_tmp = json_object_object_get(json, "PacketNo");
    if(json_tmp == NULL)
        return -1;
    pack_id =json_object_get_int(json_tmp);

    /* 事件类型 */
    json_tmp = json_object_object_get(json, "NoteType");
    if(json_tmp == NULL)
        return -1;
 
    ret_s32 = json_object_get_int(json_tmp);
    if(ret_s32 == 0){ /* 物业信息 */
        stAreaInfoDef.emDataType = emAreaMsg;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): DataType(%d) Property information\n", __LINE__, stAreaInfoDef.emDataType);
    }else if(ret_s32 == 1){ /* 广告信息 */
        stAreaInfoDef.emDataType = emAdMsg;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): DataType(%d) Advertising messages\n", __LINE__, stAreaInfoDef.emDataType);
    }else if(ret_s32 == 2){ /* 紧急通知 */
        stAreaInfoDef.emDataType = emUrgentMsg;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): DataType(%d) Urgent message\n", __LINE__, stAreaInfoDef.emDataType);
    }else if(ret_s32 == 3){ /* 定位信息 */
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): DataType(%d) Location information\n", __LINE__, stAreaInfoDef.emDataType);
    }else{ /* TBD, 不知道是啥 */
        stAreaInfoDef.emDataType = emAreaMsg;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): DataType(%d) Others information\n", __LINE__, stAreaInfoDef.emDataType);
    }

    /* 创建时间 */
    json_tmp = json_object_object_get(json, "DataTime");
    if(json_tmp == NULL)
        return -1;
    pstr = (s8 *)json_object_get_string(json_tmp);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): CreateTime(%s)\n", __LINE__, pstr);
    snprintf(stAreaInfoDef.szCreateTime, sizeof(stAreaInfoDef.szCreateTime), "%s", pstr);
    /* 消息信息 */
    json_obj = json_object_object_get(json, "NoteInfo");
    if(json_obj == NULL)
        return -1;
    /* 消息流水号 */
    json_tmp = json_object_object_get(json_obj, "NoteSn");
    if(json_tmp == NULL)
        return -1;
    stAreaInfoDef.ulIndex = json_object_get_int(json_tmp);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): Index(%d)\n", __LINE__, stAreaInfoDef.ulIndex);
    /* 消息生效时间 */
    json_tmp = json_object_object_get(json_obj, "Start");
    if(json_tmp == NULL)
        return -1;
    pstr = (s8 *)json_object_get_string(json_tmp);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): Start(%s)\n", __LINE__, pstr);
    snprintf(stAreaInfoDef.szStart, sizeof(stAreaInfoDef.szStart), "%s", pstr);
    /* 消息失效时间 */
    json_tmp = json_object_object_get(json_obj, "End");
    if(json_tmp == NULL)
        return -1;
    pstr = (s8 *)json_object_get_string(json_tmp);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): End(%s)\n", __LINE__, pstr);
    snprintf(stAreaInfoDef.szEnd, sizeof(stAreaInfoDef.szEnd), "%s", pstr);
    /* 消息标题 */
    json_tmp = json_object_object_get(json_obj, "Title");
    if(json_tmp == NULL)
        return -1;
    pstr = (s8 *)json_object_get_string(json_tmp);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): Title(%s)\n", __LINE__, pstr);
    snprintf(stAreaInfoDef.szTitle, sizeof(stAreaInfoDef.szTitle), "%s", pstr);
    /* 消息内容 */
    json_tmp = json_object_object_get(json_obj, "Msg");
    if(json_tmp == NULL)
        return -1;
    pstr = (s8 *)json_object_get_string(json_tmp);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): Content(%s)\n", __LINE__, pstr);
    snprintf(stAreaInfoDef.szContent, sizeof(stAreaInfoDef.szContent), "%s", pstr);
   /* 消息循环时间 */
    json_tmp = json_object_object_get(json_obj, "loop");
    if(json_tmp == NULL){ /* 一次性消息 */
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: Oncely Msg", __FILE__, __LINE__);
        stAreaInfoDef.emLoopType = emOnce;
    }else{ /* 每周消息 */
        s32 ii = 0;
        ret_s32 = json_object_array_length(json_tmp);
		if(ret_s32 == 0)
		{
			stAreaInfoDef.emLoopType = emOnce;
		}
		else if(ret_s32 > 0)
		{
        	stAreaInfoDef.emLoopType = emWeekly;
		}
        if(ret_s32 >= MAX_LOOP_TIME_NUM)
            ret_s32 = MAX_LOOP_TIME_NUM - 1;
        for(ii = 0; ii < ret_s32; ii++){
            json_object *elm = json_object_array_get_idx(json_tmp, ii);
            json_object *obj = NULL;

            obj = json_object_object_get(elm , "Loopstart");
            if(obj != NULL){
                pstr = (s8 *)json_object_get_string(obj);
                UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): LoopStart(%s)\n", __LINE__, pstr);
                snprintf(stAreaInfoDef.szLoopStart[ii], sizeof(stAreaInfoDef.szLoopStart[ii]), "%s", pstr);
            }

            obj = json_object_object_get(elm , "Loopend");
            if(obj != NULL){
                pstr = (s8 *)json_object_get_string(obj);
                UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): LoopEnd(%s)\n", __LINE__, pstr);
                snprintf(stAreaInfoDef.szLoopEnd[ii], sizeof(stAreaInfoDef.szLoopEnd[ii]), "%s", pstr);
            }

            obj = json_object_object_get(elm , "Loopweek");
            if(obj != NULL){
                pstr = (s8 *)json_object_get_string(obj);
                UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): Loopweek(%s)\n", __LINE__, pstr);
                snprintf(stAreaInfoDef.szLoopWeek[ii], sizeof(stAreaInfoDef.szLoopWeek[ii]), "%s", pstr);
            }
        }
    }
    /* 附件, 广告有效 */
    json_tmp = json_object_object_get(json_obj, "Attachment");
    if(json_tmp != NULL){ /* 有附件 */
        const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
        s32 ii = 0, pathfile_len = 0, name_len = 0;
        s8 name[64] = {0}, download[512] = {0}, save[512] = {0};

        ret_s32 = json_object_array_length(json_tmp);
        for(ii = 0; ii < ret_s32; ii++){
            json_object *elm = json_object_array_get_idx(json_tmp, ii);
            pstr = (s8 *)json_object_get_string(elm);
            if(com_func_pathfile_file(pstr, name, sizeof(name)))
                continue;
            name_len = strlen((char *)name);
            if((name_len + pathfile_len) >= MAX_FILE_PATH_LENGTH)
                break;
            pathfile_len += snprintf(stAreaInfoDef.szFilePath + pathfile_len, sizeof(stAreaInfoDef.szFilePath), "%s", name);
            stAreaInfoDef.szFilePath[pathfile_len++] = '/'; /* 插入分隔符 */
            com_func_attachment_savepath(stAreaInfoDef.emDataType, name, (char *)save);
            snprintf((char *)download, sizeof(download), "ftp://%s/%s", cpstCOMManage->stCOMCfg.szNetAdminIP, (char *)pstr);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): dowanload(%s)\n", __LINE__, download);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): save(%s)\n", __LINE__, save);
            com_ftp_download(download, save);
        }
        if(pathfile_len > 0)
            --pathfile_len;
        stAreaInfoDef.szFilePath[pathfile_len] = '\0';
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): FilePath(%s)\n", __LINE__, stAreaInfoDef.szFilePath);
    }
    {
        s8 *pbuf = NULL;
        ret_s32 = com_json_pack("Message", "NoteReleaseACK", pack_id, ack_true_em, &pbuf, NULL, NULL);
        if (ret_s32 != -1){
            ret_s32 = com_func_tcp_data_send(sock, pbuf, ret_s32);
            free(pbuf);
        }
    }
    if(ret_s32 != -1)
        com_func_msg_send(emModDATA, COM_DATA_AREAINFO_RECV, 0, 0, sizeof(AreaInfoDef), &stAreaInfoDef);
    return ret_s32;
}
/**@END! static s32 com_json_Message_NoteRelease_handle(json_object *json, const s32 sock) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_door_control_handle(json_object *json)
* @Description(描述): 处理: 信息发布
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     0 - 成功, 其他 - 错误
************************************************************************/
static s32 com_json_door_control_handle(json_object *json, const s32 sock){
    json_object *json_tmp = NULL;
    json_object *json_obj = NULL;
    s8 *pstr = NULL;
    s32 pack_id = 0, ret_s32 = 0;
	enDoorCtrlDef emDoorCtrlType;  
    

    /* 包号 */
    json_tmp = json_object_object_get(json, "PacketNo");
    if(json_tmp == NULL)
        return -1;
    pack_id =json_object_get_int(json_tmp);

    /* 事件类型 */
    json_tmp = json_object_object_get(json, "Oprationtype");
    if(json_tmp == NULL)
        return -1;

 	
	
    ret_s32 = json_object_get_int(json_tmp);
    if(ret_s32 == 1){ /* 开启通道管制 */
        emDoorCtrlType = emOpenAccessCtrl;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): emDoorCtrlType(%d)\n", __LINE__, emDoorCtrlType);
    }else if(ret_s32 == 2){ /* 关闭通道管制 */
        emDoorCtrlType = emCloseAccessCtrl;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): emDoorCtrlType(%d)\n", __LINE__, emDoorCtrlType);
    }else if(ret_s32 == 3){ /* 开启消防联动 */
        emDoorCtrlType = emOpenFireLinkage;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): emDoorCtrlType(%d)\n", __LINE__, emDoorCtrlType);
    }else if(ret_s32 == 4){ /* 关闭消防联动 */
    	emDoorCtrlType = emCloseFireLinkage;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): emDoorCtrlType(%d)\n", __LINE__, emDoorCtrlType);
    }else{ /* TBD, 不知道是啥 */
        emDoorCtrlType = emNULL;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): emDoorCtrlType(%d)\n", __LINE__, emDoorCtrlType);
    }

  
    {
        s8 *pbuf = NULL;
        ret_s32 = com_json_pack("Door", "ControlACK", pack_id, ack_true_em, &pbuf, NULL, NULL);
        if (ret_s32 != -1){
            ret_s32 = com_func_tcp_data_send(sock, pbuf, ret_s32);
            free(pbuf);
        }
    }
	if(ret_s32 != -1)
    	com_func_msg_send(emModHW, COM_HW_DOOR_CTRL, emDoorCtrlType, 0, 0, NULL);
	
    return ret_s32;
}
/**@END! static s32 com_json_door_control_handle(json_object *json, const s32 sock) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_blacklist_upgrade_handle(json_object *json)
* @Description(描述): 处理: 信息发布
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     0 - 成功, 其他 - 错误
************************************************************************/
static s32 com_json_blacklist_upgrade_handle(json_object *json, const s32 sock){
    json_object *json_tmp = NULL;
    json_object *json_obj = NULL;
    s8 *pstr = NULL;
    s32 pack_id = 0, ret_s32 = 0;
	char* pDir = NULL;
	char* pVer = NULL;
	BlackListDef stBlackList;
	json_object *VerInfo = NULL;
	
	memset(&stBlackList, 0, sizeof(BlackListDef));
    /* 包号 */
    json_tmp = json_object_object_get(json, "PacketNo");
    if(json_tmp == NULL)
        return -1;
    pack_id =json_object_get_int(json_tmp);

    /* 服务器存放路径 */
    json_tmp = json_object_object_get(json, "Directory");
    if(json_tmp == NULL)
        return -1;
	pDir = (char *)json_object_get_string(json_tmp);
	if (pDir == NULL)
		return -1;
 	strncpy(stBlackList.szDir, pDir, sizeof(stBlackList.szDir) - 1);
	
  	VerInfo = json_object_object_get(json, "VerInfo");
    if (VerInfo == NULL)
        return -1;
	json_tmp = json_object_object_get(VerInfo, "BlacklistVer");
	if (json_tmp == NULL)
			return -1;
	pVer = (char *)json_object_get_string(json_tmp);
	if(pVer == NULL)
			return -1;
	strncpy(stBlackList.szVer, pVer, sizeof(stBlackList.szVer) - 1);
	
    {
        s8 *pbuf = NULL;
        ret_s32 = com_json_pack("Blackist", "UpgradeACK", pack_id, ack_true_em, &pbuf, NULL, NULL);
        if (ret_s32 != -1){
            ret_s32 = com_func_tcp_data_send(sock, pbuf, ret_s32);
            free(pbuf);
        }
    }
	if(ret_s32 != -1)
	{	
    	com_func_msg_send(emModUI, COM_UI_BLACKLIST_UPGRADE, 0, 0, sizeof(BlackListDef), &stBlackList);
		UT_LOG_LOGOUT_DEBUG(emModCOM,5,"send COM_UI_BLACKLIST_UPGRADE");
	}
	
    return ret_s32;
}
/**@END! static s32 com_json_blacklist_upgrade_handle(json_object *json, const s32 sock) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_Message_NoteDel_handle(json_object *json)
* @Description(描述): 处理: 信息删除
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     0 - 成功, 其他 - 错误
************************************************************************/
static s32 com_json_Message_NoteDel_handle(json_object *json, const s32 sock){
    json_object *json_tmp = NULL;
    s32 pack_id = 0, ret_s32 = 0;
    u32 note_sn = 0;

    /* 包号 */
    json_tmp = json_object_object_get(json, "PacketNo");
    if(json_tmp == NULL)
        return -1;
    pack_id = json_object_get_int(json_tmp);

    json_tmp = json_object_object_get(json, "NoteSn");
    if(json_tmp == NULL)
        return -1;
    note_sn = json_object_get_int(json_tmp);

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): NoteSn(%d)\n", __LINE__, note_sn);
    {
        s8 *pbuf = NULL;
        ret_s32 = com_json_pack("Message", "NoteDelACK", pack_id, ack_true_em, &pbuf, NULL, NULL);
        if (ret_s32 != -1){
            ret_s32 = com_func_tcp_data_send(sock, pbuf, ret_s32);
            free(pbuf);
        }
    }
    if (ret_s32 != -1)
        com_func_msg_send(emModDATA, COM_DATA_AREAINFO_DELETEED, note_sn, 0, 0, NULL);
    return ret_s32;
}
/**@END! static s32 com_json_Message_NoteDel_handle(json_object *json, const s32 sock) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_Message_Weather_handle(json_object *json, const s32 sock)
* @Description(描述): 处理: 天气信息
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     0 - 成功, 其他 - 错误
************************************************************************/
static s32 com_json_Message_Weather_handle(json_object *json, const s32 sock){
    json_object *json_tmp = NULL;
    char *pstr = NULL;
    char buf[512] = {0};
    AreaInfoDef stAreaInfoDef;
    s32 pack_id = 0, ii = 0, str_len = 0, ret_s32 = 0;

    /* 包号 */
    json_tmp = json_object_object_get(json, "PacketNo");
    if(json_tmp == NULL)
        return -1;
    pack_id = json_object_get_int(json_tmp);

    json_tmp = json_object_object_get(json, "WeatherInfo");
    if(json_tmp == NULL)
        return -1;
 
    ret_s32 = json_object_array_length(json_tmp);
    for(ii = 0; ii < ret_s32; ii++){
        json_object *elm = json_object_array_get_idx(json_tmp, ii); /* 取得元素 */
        json_object *obj = NULL;
        
        memset(&stAreaInfoDef, 0, sizeof(stAreaInfoDef));

        str_len = 0;
        stAreaInfoDef.emLoopType = emEveryday;
        stAreaInfoDef.emDataType = emWeatherMsg;

        obj = json_object_object_get(elm, "day"); /* 星期 */
        if(obj == NULL)
            buf[str_len++] = '|';
        else{
            pstr = (char *)json_object_get_string(obj);
            str_len += snprintf(buf + str_len, sizeof(buf), "%s|", pstr);
        }

        obj = json_object_object_get(elm, "date"); /* 日期 */
        if(obj == NULL)
            buf[str_len++] = '|';
        else{
            pstr = (char *)json_object_get_string(obj);
            strncpy(stAreaInfoDef.szTitle, pstr, MAX_NAME_LENGTH - 1);
            str_len += snprintf(buf + str_len, sizeof(buf), "%s|", pstr);
        }

        obj = json_object_object_get(elm, "range"); /* 每天的气温范围 */
        if(obj == NULL)
            buf[str_len++] = '|';
        else{
            pstr = (char *)json_object_get_string(obj);
            str_len += snprintf(buf + str_len, sizeof(buf), "%s|", pstr);
        }

        obj = json_object_object_get(json, "temperature"); /* 当前时刻时实温度 */
        if(obj == NULL)
            buf[str_len++] = '|';
        else{
            pstr = (char *)json_object_get_string(obj);
            str_len += snprintf(buf + str_len, sizeof(buf), "%s|", pstr);
        }

        obj = json_object_object_get(elm, "weather"); /* 天气描述 */
        if(obj == NULL)
            buf[str_len++] = '|';
        else{
            pstr = (char *)json_object_get_string(obj);
            str_len += snprintf(buf + str_len, sizeof(buf), "%s|", pstr);
        }

        obj = json_object_object_get(elm, "wind"); /* 风力大小 */
        if(obj == NULL)
            buf[str_len++] = '|';
        else{
            pstr = (char *)json_object_get_string(obj);
            str_len += snprintf(buf + str_len, sizeof(buf), "%s|", pstr);
        }

        obj = json_object_object_get(elm, "pm25"); /* PM2.5情况 */
        if(obj == NULL)
            buf[str_len++] = '|';
        else{
            pstr = (char *)json_object_get_string(obj);
            str_len += snprintf(buf + str_len, sizeof(buf), "%s|", pstr);
        }

        obj= json_object_object_get(elm, "light"); /* 紫外线强度 */
        if(obj == NULL)
            buf[str_len++] = '|';
        else{
            pstr = (char *)json_object_get_string(obj);
            str_len += snprintf(buf + str_len, sizeof(buf), "%s", pstr);
        }

        if(str_len > 0){
            buf[str_len] = '\0'; /* 填充字符串结尾 */
            strncpy(stAreaInfoDef.szContent, buf, MAX_LOG_DATA_LENGTH - 1);
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): Title(%s)-Content(%s)\n", __LINE__, stAreaInfoDef.szTitle, stAreaInfoDef.szContent);
            com_func_msg_send(emModDATA, COM_DATA_AREAINFO_RECV, 0, 0, sizeof(AreaInfoDef), &stAreaInfoDef);
        }
    }
    {
        s8 *pbuf = NULL;
        ret_s32 = com_json_pack("Message", "WeatherACK", pack_id, ack_true_em, &pbuf, NULL, NULL);
        if (ret_s32 != -1){
            ret_s32 = com_func_tcp_data_send(sock, pbuf, ret_s32);
            free(pbuf);
        }
    }
    return ret_s32;
}
/**@END! static s32 com_json_Message_Weather_handle(json_object *json, const s32 sock) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_Unlock_Remote_handle(json_object *json, const s32 sock, enUnlockTypeDef unlock_type_em)
* @Description(描述): 处理: 远端开锁
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     0 - 成功, 其他 - 错误
************************************************************************/

SdInt com_json_Unlock_Remote_handle(json_object *json, SdInt iSock){
   st_unlock_para unlock_para_st;
   pst_unlock_para punlock_para_st;
   json_object *json_tmp = SD_NULL;
   json_object *EventInfo = SD_NULL;
   SdInt iPacketNo = SD_NULL;
   TIMER_HANDLE handle;
   SdInt iRemoteUnlockUserType = 0;
   RemoteUnlockDef stRemoteUnlockDef;
   SdInt iNum = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d (%s)]:", __FILE__, __LINE__, __FUNCTION__);

   /* 包号 */
   json_tmp = json_object_object_get(json, "PacketNo");
   iPacketNo = json_object_get_int(json_tmp);

   unlock_para_st.pack_id = iPacketNo;

   EventInfo = json_object_object_get(json, "EventInfo");
   /* 远程开锁对方UC */
   json_tmp = json_object_object_get(EventInfo, "Operatoruc");
   if(json_tmp == SD_NULL)
      return -1;
   unlock_para_st.uc = json_object_get_int(json_tmp);
   stRemoteUnlockDef.iOperatoruc = unlock_para_st.uc;

   /* 远程开锁对方类型 */
   json_tmp = json_object_object_get(EventInfo, "Operatortype");
   if(json_tmp == SD_NULL)
      return -1;
   iRemoteUnlockUserType = json_object_get_int(json_tmp);

   switch(iRemoteUnlockUserType)
   {   
      case 1: /* 物理管理中心开锁 */
         unlock_para_st.unlock_type_em = emPhyManageUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockPhyManage;
      break;
      case 2: /* 网管开锁 */
         unlock_para_st.unlock_type_em = emNetAdminUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockNetAdmin;
      break;
      case 3: /* 第三方开锁 */
         unlock_para_st.unlock_type_em = emThirdUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockThird;
      break;
      default:
         unlock_para_st.unlock_type_em = emUnknownUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockUnknow;
      break;
   }

   unlock_para_st.sock = iSock;
   unlock_para_st.json_ack_em = ack_false_em;

	punlock_para_st = (pst_unlock_para)malloc(sizeof(st_unlock_para));
	if(punlock_para_st == NULL)
	    return -1;
	memcpy(punlock_para_st, &unlock_para_st, sizeof(st_unlock_para));

	handle = com_timer_set(&com_json_unlock_callback, &com_json_unlock_destructor, punlock_para_st,
	        COM_UNLOCK_TIME, COM_UNLOCK_TIME, 1, timer_unlock_ack_em);
		if(handle == TIMER_NONE){ /* 分配定时器失败 */
		free(punlock_para_st);
		return -1;
	}

   com_func_msg_send(emModUI, COM_UI_DOOR_REMOTE_OPEN, 0, 0, sizeof(RemoteUnlockDef), &stRemoteUnlockDef);

   return 0;
}


/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_App_HangUp_handle(json_object *json, const s32 sock, enUnlockTypeDef unlock_type_em)
* @Description(描述): 处理: 远端开锁
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     0 - 成功, 其他 - 错误
************************************************************************/

SdInt com_json_App_HangUp_handle(json_object *json, SdInt iSock){
   st_unlock_para unlock_para_st;
   pst_unlock_para punlock_para_st;
   json_object *json_tmp = SD_NULL;
   json_object *EventInfo = SD_NULL;
   SdInt iPacketNo = SD_NULL;
   TIMER_HANDLE handle;
   SdInt iRemoteUnlockUserType = 0;
   RemoteUnlockDef stRemoteUnlockDef;
   SdInt iNum = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d (%s)]:", __FILE__, __LINE__, __FUNCTION__);

   /* 包号 */
   json_tmp = json_object_object_get(json, "PacketNo");
   iPacketNo = json_object_get_int(json_tmp);

   unlock_para_st.pack_id = iPacketNo;

   EventInfo = json_object_object_get(json, "EventInfo");
   /* 远程开锁对方UC */
   json_tmp = json_object_object_get(EventInfo, "Operatoruc");
   if(json_tmp == SD_NULL)
      return -1;
   unlock_para_st.uc = json_object_get_int(json_tmp);
   stRemoteUnlockDef.iOperatoruc = unlock_para_st.uc;

   /* 远程开锁对方类型 */
   json_tmp = json_object_object_get(EventInfo, "Operatortype");
   if(json_tmp == SD_NULL)
      return -1;
   iRemoteUnlockUserType = json_object_get_int(json_tmp);

   switch(iRemoteUnlockUserType)
   {   
      case 1: /* 物理管理中心开锁 */
         unlock_para_st.unlock_type_em = emPhyManageUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockPhyManage;
      break;
      case 2: /* 网管开锁 */
         unlock_para_st.unlock_type_em = emNetAdminUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockNetAdmin;
      break;
      case 3: /* 第三方开锁 */
         unlock_para_st.unlock_type_em = emThirdUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockThird;
      break;
      default:
         unlock_para_st.unlock_type_em = emUnknownUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockUnknow;
      break;
   }

   unlock_para_st.sock = iSock;
   unlock_para_st.json_ack_em = ack_false_em;

	punlock_para_st = (pst_unlock_para)malloc(sizeof(st_unlock_para));
	if(punlock_para_st == NULL)
	    return -1;
	memcpy(punlock_para_st, &unlock_para_st, sizeof(st_unlock_para));

	handle = com_timer_set(&com_json_unlock_callback, &com_json_unlock_destructor, punlock_para_st,
	        COM_UNLOCK_TIME, COM_UNLOCK_TIME, 1, timer_unlock_ack_em);
		if(handle == TIMER_NONE){ /* 分配定时器失败 */
		free(punlock_para_st);
		return -1;
	}

   com_func_msg_send(emModUI, COM_UI_APP_HANG_UP, 0, 0, sizeof(RemoteUnlockDef), &stRemoteUnlockDef);

   return 0;
}



/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_Unlock_SessionUnlock_handle(json_object *json, const s32 sock, enUnlockTypeDef unlock_type_em)
* @Description(描述): 处理: 回话过程中开锁
* @Param(参数):            *json, 参数, sock, socket句柄, unlock_type_em, 开锁类型
* @ReturnCode(返回值):     0 - 成功, 其他 - 错误
************************************************************************/
static s32 com_json_Unlock_SessionUnlock_handle(json_object *json, const s32 sock, enUnlockTypeDef unlock_type_em){
    st_unlock_para unlock_para_st;
    pst_unlock_para punlock_para_st;
    json_object *json_tmp = NULL;
    TIMER_HANDLE handle;
    s32 pack_id = 0, ret_s32 = 0;

    /* 包号 */
    json_tmp = json_object_object_get(json, "PacketNo");
    if(json_tmp == NULL)
        return -1;
    pack_id = json_object_get_int(json_tmp);

    /* 主机会话对方UC */
    json_tmp = json_object_object_get(json, "Operatoruc");
    if(json_tmp == NULL)
        return -1;

    unlock_para_st.sock = sock;
    unlock_para_st.uc = json_object_get_int(json_tmp); /* 提取操作UC */
    unlock_para_st.pack_id = pack_id;

    ret_s32 = com_timer_unlock_ack_exist(&unlock_para_st); /* 会重设socket句柄 */
    if(ret_s32 != -1)
    {
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): Waiting unlock ACK..\n", __LINE__);
        return 0;
    }

    unlock_para_st.json_ack_em = ack_false_em;
    unlock_para_st.unlock_type_em = unlock_type_em;

    punlock_para_st = (pst_unlock_para)malloc(sizeof(st_unlock_para));
    if(punlock_para_st == NULL)
        return -1;
    memcpy(punlock_para_st, &unlock_para_st, sizeof(st_unlock_para));

    handle = com_timer_set(&com_json_unlock_callback, &com_json_unlock_destructor, punlock_para_st,
                COM_UNLOCK_TIME, COM_UNLOCK_TIME, 1, timer_unlock_ack_em);
    if(handle == TIMER_NONE){ /* 分配定时器失败 */
        free(punlock_para_st);
        return -1;
    }

    com_func_msg_send(emModUI, COM_UI_DOOR_SESSION_OPEN, punlock_para_st->uc, 0, 0, NULL);
    return 0;
}
/**@END! static s32 com_json_Unlock_SessionUnlock_handle(json_object *json, const s32 sock, enUnlockTypeDef unlock_type_em) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_general_ACK_handle(json_object *json)
* @Description(描述):      ACK处理:
* @Param(参数):            *json - JSON
* @ReturnCode(返回值):     -1 - 失败, 0 - 成功
************************************************************************/
static s32 com_json_general_ACK_handle(json_object *json){
    json_object *json_tmp = NULL;
    u32 pack_id = 0;

    json_tmp = json_object_object_get(json, "PacketNo");
    if (json_tmp == NULL)
        return -1;
    pack_id = json_object_get_int(json_tmp);

    /* 删除重发定时器 */
    com_timer_client_resend_remove(pack_id);
	com_remove_am_cache_data_to_db(pack_id);

    return 0;
}
/**@END! static s32 com_json_general_ACK_handle(json_object *json) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_Access_Data_handle(json_object *json, const s32 sock)
* @Description(描述): 处理: 门禁数据
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     0 - 成功, 其他 - 错误
************************************************************************/
static s32 com_json_Access_Data_handle(json_object *json, const s32 sock){
    json_object *json_tmp = NULL;
    s8 *pbase64 = NULL;
    s8 *pbin = NULL;
    u32 pack_id = 0;
    s32 bin_len = 0, ret_s32 = 0;

    json_tmp = json_object_object_get(json, "PacketNo");
    if (json_tmp == NULL)
        return -1;
    pack_id = json_object_get_int(json_tmp);

    json_tmp = json_object_object_get(json, "AccessInfo");
    if(json_tmp == NULL)
        return -1;

    pbase64 = (s8 *)json_object_get_string(json_tmp);
    ret_s32 = strlen((char *)pbase64);
    if(ret_s32 <= 0)
        return -1;

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): AccessInfo len(%d)\n", __LINE__, bin_len);

    pbin = malloc(ret_s32 + 16);
    bin_len = com_func_base64_decode((char *)pbase64, (char *)pbin);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): AccessInfo len(%d)\n", __LINE__, bin_len);

    //    {
    //        SdInt i = 0;
    //        for(i = 0; i < bin_len; i++)
    //        {
    //            printf("bin[%02d] = [0x%02X]\n", i, pbin[i]);
    //        }
    //    }

    {
        s8 *pbuf = NULL;
        ret_s32 = com_json_pack("Access", "DataACK", pack_id, ack_true_em, &pbuf, NULL, NULL);
        if (ret_s32 != -1){
            ret_s32 = com_func_tcp_data_send(sock, pbuf, ret_s32);
            free(pbuf);
        }
    }
    if (ret_s32 != -1)
        com_func_msg_send(emModHW, COM_HW_AM_DATA_DOWN, 0, 0, bin_len, pbin);

    free(pbin);
   
    return ret_s32;
}
/**@END! static s32 com_json_Access_Data_handle(json_object *json, const s32 sock) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_json_Unlock_CheckPasswordACK_handle(json_object *json, const s32 sock)
* @Description(描述): 处理: 门禁数据
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     0 - 成功, 其他 - 错误
************************************************************************/
static s32 com_json_Unlock_CheckPasswordACK_handle(json_object *json, const s32 sock){
    json_object *json_tmp = NULL;
    json_object *DeviceInfo = NULL;
    RemotePasswdUnlockAckDef stRemotePasswdUnlockAckDef;
    const char *cpbuf = NULL;
    u32 pack_id = 0;

    memset(&stRemotePasswdUnlockAckDef, 0, sizeof(RemotePasswdUnlockAckDef));
    json_tmp = json_object_object_get(json, "PacketNo");
    if (json_tmp == NULL)
        return -1;
    pack_id = json_object_get_int(json_tmp);

    json_tmp = json_object_object_get(json, "Result");
    if (json_tmp == NULL)
        return -1;
    stRemotePasswdUnlockAckDef.emUnlockPasswdType = json_object_get_int(json_tmp);

    json_tmp = json_object_object_get(json, "Delay");
    if (json_tmp != NULL)
        stRemotePasswdUnlockAckDef.iDelay = json_object_get_int(json_tmp);

    DeviceInfo = json_object_object_get(json, "DeviceInfo");
    if (DeviceInfo == NULL)
        return -1;

    json_tmp = json_object_object_get(DeviceInfo, "dd");
    if (json_tmp == NULL)
        return -1;
    cpbuf = json_object_get_string(json_tmp);
    strncpy(stRemotePasswdUnlockAckDef.szdd, cpbuf, sizeof(stRemotePasswdUnlockAckDef.szdd) - 1);

    json_tmp = json_object_object_get(DeviceInfo, "bbb");
    if (json_tmp == NULL)
        return -1;
    cpbuf = json_object_get_string(json_tmp);
    strncpy(stRemotePasswdUnlockAckDef.szbbb, cpbuf, sizeof(stRemotePasswdUnlockAckDef.szbbb) - 1);

    json_tmp = json_object_object_get(DeviceInfo, "rr");
    if (json_tmp == NULL)
        return -1;
    cpbuf = json_object_get_string(json_tmp);
    strncpy(stRemotePasswdUnlockAckDef.szrr, cpbuf, sizeof(stRemotePasswdUnlockAckDef.szrr) - 1);

    json_tmp = json_object_object_get(DeviceInfo, "ff");
    if (json_tmp == NULL)
        return -1;
    cpbuf = json_object_get_string(json_tmp);
    strncpy(stRemotePasswdUnlockAckDef.szff, cpbuf, sizeof(stRemotePasswdUnlockAckDef.szff) - 1);

    json_tmp = json_object_object_get(DeviceInfo, "ii");
    if (json_tmp == NULL)
        return -1;
    cpbuf = json_object_get_string(json_tmp);
    strncpy(stRemotePasswdUnlockAckDef.szii, cpbuf, sizeof(stRemotePasswdUnlockAckDef.szii) - 1);

    /* 删除重发定时器 */
    com_timer_client_resend_remove(pack_id);
    com_func_msg_send(emModUI, COM_UI_DOOR_REMOTE_PASSWD_OPEN_ACK, 0, 0, sizeof(RemotePasswdUnlockAckDef), &stRemotePasswdUnlockAckDef);

    return 0;
}
/**@END! static s32 com_json_Unlock_CheckPasswordACK_handle(json_object *json, const s32 sock) !\(^o^)/~ 结束咯 */


int com_json_dev_lift_call_handle(json_object *json, const s32 iSock)
{
    json_object *json_tmp = NULL;
    json_object * json_Info = NULL;
    SdUInt uiPackID = 0;
    TIMER_HANDLE handle;
    LPLiftParamDef pstLiftParam = SD_NULL;
    json_tmp = json_object_object_get(json, "PacketNo");
    if (json_tmp == NULL)
    {
        return -1;
    }
    uiPackID = json_object_get_int(json_tmp);
    json_Info = json_object_object_get(json,"Info");
    if(NULL != json_Info)
    {
        LiftCtrlDef stLiftCtrl;
        json_tmp = json_object_object_get(json_Info,"Operatoruc");
        stLiftCtrl.iOptianUC = json_object_get_int(json_tmp);
        json_tmp = json_object_object_get(json_Info,"Eventuc");
        stLiftCtrl.iEventUC = json_object_get_int(json_tmp);
        json_tmp = json_object_object_get(json_Info,"StartFloor");
        stLiftCtrl.iStartFloor= json_object_get_int(json_tmp);
        json_tmp = json_object_object_get(json_Info,"StartRoom");
        stLiftCtrl.iStartRoom = json_object_get_int(json_tmp);
        json_tmp = json_object_object_get(json_Info,"TargetFloor");
        stLiftCtrl.iTargetFloor = json_object_get_int(json_tmp);
        json_tmp = json_object_object_get(json_Info,"TargetRoom");
        stLiftCtrl.iTargetRoom = json_object_get_int(json_tmp);
        pstLiftParam = ut_mem_new(LiftParamDef,1);
        pstLiftParam->iSock = iSock;
        pstLiftParam->iUc = stLiftCtrl.iOptianUC;
        pstLiftParam->uiPackID = uiPackID;
        pstLiftParam->emResult = ack_false_em;
        handle = com_timer_set(&com_json_lift_call_ack_callback, &com_json_lift_call_ack_destructor, (void*)pstLiftParam,
                                COM_UNLOCK_TIME, COM_UNLOCK_TIME, 1, emTimerLiftCallAck);
        if(handle == TIMER_NONE)
        { /* 分配定时器失败 */
            ut_mem_free(pstLiftParam);
            return -1;
        }
        com_func_msg_send(emModUI, COM_UI_INDOOR_LIFT_REQUEST, 0, 0, sizeof(LiftCtrlDef), &stLiftCtrl);
        UT_LOG_LOGOUT_DEBUG(emModCOM,5,"send COM_UI_INDOOR_LIFT_REQUEST");
    }
    return 0;
}

#if 0
/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Face_RequestFaceSync_handle(char *pdata, int len);
* @Description(描述):      JSON打包及发送网络消息: 请求人脸数据同步
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_RequestFaceSync_handle(json_object *json, const s32 iSock)
{
    json_object *json_tmp = NULL;
    json_object * json_Info = NULL;
    SdInt uiPackID = 0;
    const SdChar *pstr = NULL;
    SdInt ret_s32 = 0;

    json_tmp = json_object_object_get(json, "PacketNo");
    if(NULL == json_tmp)
    {
        return -1;    
    }

    uiPackID = json_object_get_int(json_tmp);
    json_Info = json_object_object_get(json,"Info");

    if(NULL != json_Info)
    {
        FaceRequestSynDef stFaceRequestSyn;
        json_tmp = json_object_object_get(json_Info,"Operatoruc");
        stFaceRequestSyn.iOperatoruc = json_object_get_int(json_tmp);
        json_tmp = json_object_object_get(json_Info,"VersionInfo");
        pstr = json_object_get_string(json_tmp);
        strncpy(stFaceRequestSyn.szVersionInfo, pstr, sizeof(stFaceRequestSyn.szVersionInfo) - 1);
//        snprintf(stFaceRequestSyn.iVersionInfo, sizeof(stFaceRequestSyn.iVersionInfo), "%s", pstr);

        {
            SdChar *pbuf = NULL;
            //ret_s32 = com_json_pack("Face", "RequestFaceSync", uiPackID, ack_true_em, &pbuf, NULL, NULL);
            if (ret_s32 != -1)
            {
                ret_s32 = com_func_tcp_data_send(iSock, pbuf, ret_s32);
                free(pbuf);
            }
        }
        UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Face_RequestFaceSync_handle");
//        com_func_msg_send(emModFACE, COM_FACE_FACE_SYNC_REQUEST, 0, 0, sizeof(FaceRequestSynDef), &stFaceRequestSyn);
    }
    return 0;
}
#endif

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Face_RequestFaceSyncAck_handle(char *pdata, int len);
* @Description(描述):      JSON打包及发送网络消息: 请求人脸数据同步应答
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_RequestFaceSyncAck_handle(json_object *json, const s32 iSock)
{
    
    SdInt iRe = SD_FAIL;
    json_object *json_tmp = NULL;
    json_object * json_Info = NULL;
    SdInt uiPackID = 0;
    const SdChar *pstr = NULL;
    FaceDataInfoDef stFaceDataInfo = { 0 };

    json_tmp = json_object_object_get(json, "PacketNo");
    //printf("json=%p----- json_tmp=%p\n", json, json_tmp);
    if(NULL == json_tmp)
    {
        return -1;    
    }

    uiPackID = json_object_get_int(json_tmp);
    json_Info = json_object_object_get(json,"Info");

    if(NULL != json_Info)
    {
        json_tmp = json_object_object_get(json_Info,"FtpUser");
        pstr = json_object_get_string(json_tmp);
        if (pstr)
        {
            strncpy(stFaceDataInfo.szUser, pstr, sizeof(stFaceDataInfo.szUser) - 1);
        }
        
        json_tmp = json_object_object_get(json_Info,"FtpPasswd");
           pstr = json_object_get_string(json_tmp);
        if (pstr)
        {
            strncpy(stFaceDataInfo.szPasswd, pstr, sizeof(stFaceDataInfo.szPasswd) - 1);
        }
        
        json_tmp = json_object_object_get(json_Info,"FtpIpAddr");
           pstr = json_object_get_string(json_tmp);
        if (pstr)
        {
            strncpy(stFaceDataInfo.szIpAddr, pstr, sizeof(stFaceDataInfo.szIpAddr) - 1);
        }
        
        json_tmp = json_object_object_get(json_Info,"FtpPath");
        pstr = json_object_get_string(json_tmp);
        if (pstr)
        {
            strncpy(stFaceDataInfo.szFile, pstr, sizeof(stFaceDataInfo.szFile) - 1);
            //printf("pstr: %s    stFaceDataInfo.szFile:%s\n",pstr, stFaceDataInfo.szFile);
        }

        iRe = 0;
        json_tmp = json_object_object_get(json_Info,"Update");
        if(json_tmp)
        {
            iRe = json_object_get_int(json_tmp);
        }
        
        json_tmp = json_object_object_get(json_Info,"Chk");
        pstr = json_object_get_string(json_tmp);
        if(pstr)
        {
            strncpy(stFaceDataInfo.szchk, pstr, sizeof(stFaceDataInfo.szchk) - 1);
        }
    }  
#if 0
    printf("FtpUser : %s\n",stFaceDataInfo.szUser);
    printf("FtpPasswd : %s\n",stFaceDataInfo.szPasswd);
    printf("FtpIpAddr : %s\n",stFaceDataInfo.szIpAddr);
    printf("FtpPath : %s\n",stFaceDataInfo.szFile);
    printf("Update : %d\n",iRe);
    printf("Chk : %s\n",stFaceDataInfo.szchk);
#endif
    com_timer_client_resend_remove(uiPackID);
    com_func_msg_send(emModFACE, COM_FACE_FACE_SYNC_REQUEST_ACK, iRe, 0, sizeof(FaceDataInfoDef), &stFaceDataInfo);
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Face_RequestFaceSyncAck_handle");

    return 0;
}

#if(0)
/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Face_FaceSyncCheck_handle(char *pdata, int len);
* @Description(描述):      JSON打包及发送网络消息: 人脸数据同步检测
* @Param(参数):               *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_FaceSyncCheck_handle(json_object *json, const s32 iSock)
{
    json_object *json_tmp = NULL;
    json_object * json_Info = NULL;
    SdInt uiPackID = 0;
    const SdChar *pstr = NULL;
    SdInt ret_s32 = 0;

    json_tmp = json_object_object_get(json, "PacketNo");
    if(NULL == json_tmp)
    {
        return -1;    
    }

    uiPackID = json_object_get_int(json_tmp);
    json_Info = json_object_object_get(json,"Info");

    if(NULL != json_Info)
    {
        FaceRequestSynDef stFaceRequestSyn;
        json_tmp = json_object_object_get(json_Info,"Operatoruc");
        stFaceRequestSyn.iOperatoruc = json_object_get_int(json_tmp);
        json_tmp = json_object_object_get(json_Info,"VersionInfo");
        pstr = json_object_get_string(json_tmp);
        strncpy(stFaceRequestSyn.szVersionInfo, pstr, sizeof(stFaceRequestSyn.szVersionInfo) - 1);
        
        {
            s8 *pbuf = NULL;
            //ret_s32 = com_json_pack("Face", "FaceSyncCheck", uiPackID, ack_true_em, &pbuf, NULL, NULL);
            if (ret_s32 != -1)
            {
                ret_s32 = com_func_tcp_data_send(iSock, pbuf, ret_s32);
                free(pbuf);
            }
        }
//        com_func_msg_send(emModDATA, COM_DATA_AREAINFO_RECV, 0, 0, sizeof(AreaInfoDef), &stAreaInfoDef);
    }
    return 0;
}
#endif

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Face_FaceSyncCheckAck_handle(char *pdata, int len);
* @Description(描述):      JSON打包及发送网络消息: 人脸数据同步检测应答
* @Param(参数):               *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_FaceSyncCheckAck_handle(json_object *json)
{
    json_object *json_tmp = NULL;
    SdInt uiPackId = 0;
    SdInt iResult = 0;

    json_tmp = json_object_object_get(json, "PacketNo");
    if(NULL == json_tmp)
    {
        return -1;    
    }
    uiPackId = json_object_get_int(json_tmp);

    json_tmp = json_object_object_get(json, "Result");
    if(NULL == json_tmp)
    {
        return -1;    
    }
    iResult = json_object_get_int(json_tmp);

    /* 删除重发定时器 */
    com_timer_client_resend_remove(uiPackId);
    com_func_msg_send(emModFACE, COM_FACE_FACE_SYNC_CHECK_ACK, iResult, 0, 0, NULL);
    
    return 0;
}

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Face_FaceSync_handle(char *pdata, int len);
* @Description(描述):      JSON打包及发送网络消息: 人脸数据同步
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_FaceSync_handle(json_object *json, const s32 iSock)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    SdInt iOperatoruc = cpstCOMManage->stCOMCfg.uiUcCode;
    SdInt uiPackID = 0;
    const SdInt sock = iSock;
    json_object *json_tmp = NULL;
    json_object * json_Info = NULL;
    const SdChar *pstr = NULL;
    SdInt ret_s32 = 0;
    FaceVerInfoDef stFaceVerInfo;

    json_tmp = json_object_object_get(json, "PacketNo");
    if(NULL == json_tmp)
    {
        return -1;    
    }
    uiPackID = json_object_get_int(json_tmp);
    json_Info = json_object_object_get(json,"Info");
    if(NULL != json_Info)
    {
        json_tmp = json_object_object_get(json_Info,"Operatoruc");
        if(json_tmp)
        {
            iOperatoruc = json_object_get_int(json_tmp);
        }
        
        json_tmp = json_object_object_get(json_Info,"VersionInfo");
        pstr = json_object_get_string(json_tmp);
        if(pstr)
        {
            strncpy(stFaceVerInfo.szVerInfo, pstr, sizeof(stFaceVerInfo.szVerInfo) - 1);
        }
        {
            s8 *pbuf = NULL;
            if (ret_s32 != -1)
            {
                ret_s32 = com_func_tcp_data_send(iSock, pbuf, ret_s32);
                free(pbuf);
            }
        }
        //com_timer_client_resend_remove(uiPackID);
        com_func_msg_send(emModFACE, COM_FACE_FACE_SYNC, iOperatoruc, sock, sizeof(FaceVerInfoDef), &stFaceVerInfo);
        UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Face_FaceSync_handle");
    }
    return 0;
}

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Finger_Char_Ack_handle(char *pdata, int len);
* @Description(描述):      JSON打包及发送网络消息: 指纹特征值发送
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Char_Ack_handle(json_object *json, const s32 iSock)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
//    TIMER_HANDLE FingerNet_timer = cpstCOMManage->iFingerTimer;
    json_object *json_tmp = NULL;
    SdInt uiPackID = 0;
    SdInt iResult = 0;

    json_tmp = json_object_object_get(json, "PacketNo");
    if(NULL == json_tmp)
    {
        return -1;    
    }
    uiPackID = json_object_get_int(json_tmp);
    
    com_timer_client_resend_remove(uiPackID);

    com_timer_remove(cpstCOMManage->iFingerTimer);
    cpstCOMManage->iFingerTimer = -1;

    com_func_msg_send(emModFINGER, COM_FINGER_FINGER_CHAR_ACK, 0, 0, 0, NULL);
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Finger_Char_Ack_handle");

    return 0;
}

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Finger_Char_Ack_handle(char *pdata, int len);
* @Description(描述):      JSON打包及发送网络消息: 指纹特征值发送
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Match_handle(json_object *json, const s32 iSock)
{
    json_object *json_tmp = NULL;
    NetFingMatchInfoDef stNetFingMatchInfo;
    const SdInt sock = iSock;
    SdInt uiPackID = 0;
    SdInt iResult = 0;

    json_tmp = json_object_object_get(json, "PacketNo");
    if(NULL == json_tmp)
    {
        return -1;    
    }
    uiPackID = json_object_get_int(json_tmp);

    json_tmp = json_object_object_get(json, "FingerInfoID");
    if(NULL == json_tmp)
        return -1;
    stNetFingMatchInfo.uiFingerID = json_object_get_int(json_tmp);

    json_tmp = json_object_object_get(json, "Operatoruc");
    if(NULL == json_tmp)
        return -1;
    stNetFingMatchInfo.uiUC = json_object_get_int(json_tmp);
    
    json_tmp = json_object_object_get(json, "Result");
    if(NULL == json_tmp)
        return -1;
    iResult = json_object_get_int(json_tmp);

    com_timer_client_resend_remove(uiPackID);
    com_func_msg_send(emModUI, COM_UI_FINGER_MATCH, iResult, sock, sizeof(NetFingMatchInfoDef), &stNetFingMatchInfo);
    {
        s8 *pbuf = NULL;
        com_func_tcp_data_send(iSock, pbuf, 0);
        free(pbuf);
    }
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Finger_Match_handle");

    return 0;
}


/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Finger_Sync_Request_Ack_handle(char *pdata, int len);
* @Description(描述):      JSON打包及发送网络消息: 指纹数据同步请求
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Sync_Request_Ack_handle(json_object *json, const s32 iSock)
{
    SdInt iRe = SD_FAIL;
    json_object *json_tmp = NULL;
    json_object * json_Info = NULL;
    SdInt uiPackID = 0;
    const SdChar *pstr = NULL;
    FingerDataInfoDef stFingerDataInfo;
    json_tmp = json_object_object_get(json, "PacketNo");
    if(NULL == json_tmp)
    {
        return -1;    
    }
    uiPackID = json_object_get_int(json_tmp);
    json_Info = json_object_object_get(json,"Info");

    if(NULL != json_Info)
    {
        
        json_tmp = json_object_object_get(json_Info,"FtpUser");
        pstr = json_object_get_string(json_tmp);
        if(pstr)
        {
            strncpy(stFingerDataInfo.szUser, pstr, sizeof(stFingerDataInfo.szUser) - 1);
        }
        json_tmp = json_object_object_get(json_Info,"FtpPasswd");
        pstr = json_object_get_string(json_tmp);
        if(pstr)
        {
              strncpy(stFingerDataInfo.szPasswd, pstr, sizeof(stFingerDataInfo.szPasswd) - 1);
        }

        json_tmp = json_object_object_get(json_Info,"FtpIpAddr");
        pstr = json_object_get_string(json_tmp);
        if(pstr)
        {
               strncpy(stFingerDataInfo.szIpAddr, pstr, sizeof(stFingerDataInfo.szIpAddr) - 1);
        }
        
        json_tmp = json_object_object_get(json_Info,"FtpPath");
        pstr = json_object_get_string(json_tmp);
        if(pstr)
        {
              strncpy(stFingerDataInfo.szFile, pstr, sizeof(stFingerDataInfo.szFile) - 1);
        }
        
        json_tmp = json_object_object_get(json_Info,"Update");
        if(json_tmp)
        {
            iRe = json_object_get_int(json_tmp);   
        }
        
        json_tmp = json_object_object_get(json_Info,"Chk");
        pstr = json_object_get_string(json_tmp);
        if(pstr)
        {
               strncpy(stFingerDataInfo.szchk, pstr, sizeof(stFingerDataInfo.szchk) - 1);
        }
    }
    com_timer_client_resend_remove(uiPackID);
    com_func_msg_send(emModFINGER, COM_FINGER_FINGER_SYNC_REQUEST_ACK,iRe, 0,sizeof(FingerDataInfoDef), &stFingerDataInfo);
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Finger_Sync_Request_Ack_handle");

    return 0;
}


/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Finger_Char_Ack_handle(char *pdata, int len);
* @Description(描述):      JSON打包及发送网络消息: 指纹数据同步检测
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Sync_Check_Ack_handle(json_object *json, const s32 iSock)
{
    json_object *json_tmp = NULL;
    SdInt uiPackID = 0;
    SdInt iResult = 0;

    json_tmp = json_object_object_get(json, "PacketNo");
    if(NULL == json_tmp)
    {
        return -1;    
    }
    uiPackID = json_object_get_int(json_tmp);
    json_tmp = json_object_object_get(json, "Result");
    if(NULL == json_tmp)
    {
        return -1;    
    }
    iResult = json_object_get_int(json_tmp);

    com_timer_client_resend_remove(uiPackID);
    com_func_msg_send(emModFINGER, COM_FINGER_FINGER_SYNC_CHECK_ACK, iResult, 0, 0, NULL);
    UT_LOG_LOGOUT_DEBUG(emModCOM,0,"com_json_Finger_Sync_Check_Ack_handle");

    return 0;
}


/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Finger_Finger_Sync_handle(char *pdata, int len);
* @Description(描述):      JSON打包及发送网络消息: 指纹同步命令
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Finger_Sync_handle(json_object *json, const s32 iSock)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    SdInt iOperatoruc = cpstCOMManage->stCOMCfg.uiUcCode;
    json_object *json_tmp = NULL;
    json_object * json_Info = NULL;
    SdInt uiPackID = 0;
    FingerVerInfoDef stFingerVerInfo;
    const SdChar *pstr = NULL;
    SdInt ret_s32 = 0;
    json_tmp = json_object_object_get(json, "PacketNo");
    if(NULL == json_tmp)
    {
        return -1;    
    }
    uiPackID = json_object_get_int(json_tmp);
    json_Info = json_object_object_get(json,"Info");
    if(NULL != json_Info)
    {     
        json_tmp = json_object_object_get(json_Info,"Operatoruc");
        if(json_tmp)
        {
            iOperatoruc = json_object_get_int(json_tmp);
        }
        
        json_tmp = json_object_object_get(json_Info,"VersionInfo");
        pstr = json_object_get_string(json_tmp);
        if(pstr)
        {
            strncpy(stFingerVerInfo.szVerInfo, pstr, sizeof(stFingerVerInfo.szVerInfo) - 1);
        }
        
        {
            s8 *pbuf = NULL;
            //ret_s32 = com_json_pack("FINGER", "FingerSync", uiPackID, ack_true_em, &pbuf, NULL, NULL);
            if (ret_s32 != -1)
            {
                ret_s32 = com_func_tcp_data_send(iSock, pbuf, ret_s32);
                free(pbuf);
            }
        }
        UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_Face_FaceSync_handle");
        com_func_msg_send(emModFINGER, COM_FINGER_FINGER_SYNC, iOperatoruc, iSock, sizeof(FingerVerInfoDef), &stFingerVerInfo);
    }

    return 0;
}

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_OPEN_DOOR_handle(json_object *json, const s32 iSock);
* @Description(描述):      JSON打包及发送网络消息: APP开门消息
* @Param(参数):            *json, 参数, sock, socket句柄
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_OPEN_DOOR_handle(json_object *json, const s32 iSock)
{
    const LPCOMManage cpstCOMManage = com_ctrl_module_manage_get();
    SdInt iOperatoruc = cpstCOMManage->stCOMCfg.uiUcCode;
    DoorRecordDef stDoorRecord;
    json_object * json_tmp = NULL;
    json_object * json_Info = NULL;
    SdInt uiPackID = 0;
    
    const SdChar *pstr = NULL;
    SdInt ret_s32 = 0;
    json_tmp = json_object_object_get(json, "PacketNo");
    if(NULL == json_tmp)
    {
        return -1;    
    }
    uiPackID = json_object_get_int(json_tmp);
    json_Info = json_object_object_get(json,"DeviceInfo");
    if(NULL != json_Info)
    {     
        json_tmp = json_object_object_get(json_Info,"UC");
        if(json_tmp)
        {
            stDoorRecord.iOperatoruc = json_object_get_int(json_tmp);
        }
        
        json_tmp = json_object_object_get(json_Info,"AppId");
        pstr = json_object_get_string(json_tmp);
        if(pstr)
        {
            strncpy(stDoorRecord.szUserName, pstr, sizeof(stDoorRecord.szUserName) - 1);
        }
    
        UT_LOG_LOGOUT_DEBUG(emModCOM,5,"com_json_OPEN_DOOR_handle");
        com_func_msg_send(emModUI,COM_UI_APP_OPEN_DOOR_RECORD , 0, 0, sizeof(DoorRecordDef), &stDoorRecord);
    }
// COM_UI_APP_OPEN_DOOR_RECORD
    return 0;
}

static s32 com_json_liftstatus_info_callback(json_object *json, void *parg, va_list args){
	s32 len = va_arg(args, s32);
	int i = 0;
	json_object * array = NULL;
	json_object * val = NULL;
	s32 iRet = -1;
	LPLiftInfoDef data = NULL;
	int iStatus = 0, iLift = 0, iNum = 0;
	int iCrt = 0;
	if(parg && sizeof(LiftInfoDef) == len)
	{
		data = (LPLiftInfoDef)parg;
		json_object * device = json_object_new_object();
		json_object_object_add(device, "dd", json_object_new_int(data->iQNum));
		json_object_object_add(device, "bbb", json_object_new_int(data->iDNum));
		json_object_object_add(device, "rr", json_object_new_int(data->iRNum));
		json_object_object_add(json, "device", device);
		
		array = json_object_new_array();
		if(data->iLiftNum <= sizeof(data->szLiftNum))
		{
			for(i = 1; i <= data->iLiftNum; i++)
			{
				val = json_object_new_object();
				iNum = data->szLiftNum[i - 1];
				switch(i)
				{
				case 1:
					iStatus = data->iLiftStatusA;
					iLift = data->iLiftFNA;
					break;
				case 2:
					iStatus = data->iLiftStatusB;
					iLift = data->iLiftFNB;
					break;
				case 3:
					iStatus = data->iLiftStatusC;
					iLift = data->iLiftFNC;
					break;
				case 4:
					iStatus = data->iLiftStatusD;
					iLift = data->iLiftFND;
					break;
				case 5:
					iStatus = data->iLiftStatusE;
					iLift = data->iLiftFNE;
					break;
				}
				json_object_object_add(val, "Num", json_object_new_int(iNum - '0'));
				json_object_object_add(val, "Status", json_object_new_int(iStatus));
				json_object_object_add(val, "Floor", json_object_new_int(iLift));
				json_object_array_add(array, val);
			}
			json_object_object_add(json, "LiftInfo", array);
			iRet = 0;
		}
	}
	return iRet;
}
s32 com_json_liftstatus_info(LPLiftInfoDef ps_info, s8 ** jsonstr)
{
	s32 iRet = -1;
	static u32 pack_id = 1;
	if(ps_info && jsonstr)
	{
		iRet = com_json_pack("Lift", "emMsgBack", pack_id++, ack_no_em, jsonstr, &com_json_liftstatus_info_callback, 
							(void *)ps_info, sizeof(*ps_info));
		pack_id = pack_id % 100000 + 1;
	}
	return iRet;
}

#if (0)
static s32 com_json_Unlock_Remote_handle(json_object *json, const s32 sock){
   LPUnlockPara pstUnlockPara = SD_NULL;
   json_object *json_tmp = SD_NULL;
   json_object *EventInfo = SD_NULL;
   SdInt iPacketNo = SD_NULL;
   SdInt iRemoteUnlockUserType = 0;
   RemoteUnlockDef stRemoteUnlockDef;
   SdInt iNum = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d (%s)]:", __FILE__, __LINE__, __FUNCTION__);

   /* 包号 */
   json_tmp = json_object_object_get(json, "PacketNo");
   iPacketNo = json_object_get_int(json_tmp);

   iNum = com_timer_UnlockAck_num_get(emTimerTypeRemoteUnlock, iPacketNo);
   if(iNum != 0)
   {
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: Waiting ack.", __FILE__, __LINE__);
      return 0;
   }
   pstUnlockPara = ut_mem_new(UnlockPara, 1);
   if(pstUnlockPara == NULL)
      return -1;

   pstUnlockPara->iPacketNo = iPacketNo;

   EventInfo = json_object_object_get(json, "EventInfo");
   /* 远程开锁对方UC */
   json_tmp = json_object_object_get(EventInfo, "Operatoruc");
   if(json_tmp == SD_NULL)
      return -1;
   pstUnlockPara->iOperatoruc = json_object_get_int(json_tmp);
   stRemoteUnlockDef.iOperatoruc = pstUnlockPara->iOperatoruc;

   /* 远程开锁对方类型 */
   json_tmp = json_object_object_get(EventInfo, "Operatortype");
   if(json_tmp == SD_NULL)
      return -1;
   iRemoteUnlockUserType = json_object_get_int(json_tmp);

   switch(iRemoteUnlockUserType)
   {   
      case 1: /* 物理管理中心开锁 */
         pstUnlockPara->emUnlockType = emPhyManageUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockPhyManage;
      break;
      case 2: /* 网管开锁 */
         pstUnlockPara->emUnlockType = emNetAdminUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockNetAdmin;
      break;
      case 3: /* 第三方开锁 */
         pstUnlockPara->emUnlockType = emThirdUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockThird;
      break;
      default:
         pstUnlockPara->emUnlockType = emUnknownUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockUnknow;
      break;
   }

   pstUnlockPara->iSock = iSock;
   pstUnlockPara->emAck = emAckFail;

   pstUnlockPara->handle = com_timer_set(&com_unlock_Unlock_CallBack, &com_unlock_Unlock_Destructor, pstUnlockPara,
      COM_UNLOCK_TIME, COM_UNLOCK_TIME, 1, emTimerTypeRemoteUnlock);
   if(pstUnlockPara->handle == TIMER_NONE) /* 分配定时器失败 */
   {
      com_unlock_Unlock_Destructor(pstUnlockPara); /* 执行析构函数 */
      ut_mem_free(pstUnlockPara); /* 释放资源 */
      return -1;
   }

   com_func_msg_send(emModUI, COM_UI_DOOR_REMOTE_OPEN, 0, 0, sizeof(RemoteUnlockDef), &stRemoteUnlockDef);

   return 0;
}
#endif

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_unpack(const s32 sock, const s8 *pbuf_s8, const u32 len)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 网络消息拆包
*
* @Param(参数):
*        sock - SOCK句柄
*       *pbuf_s8 - 数据(已去掉包头)
*        len - 数据长度
*
* @ReturnCode(返回值):
*       -1 - 失败
*       其他 - ACK处理函数返回值
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_json_unpack(const s32 sock, const s8 *pbuf_s8, const u32 len){
    json_object *json = NULL;
    json_object *json_tmp = NULL;
    char *pBizType = NULL;
    char *pSubMsgType = NULL;
    s32 ret_s32 = 0;

    if (len == 0)
        return -1;
    if (pbuf_s8 == NULL)
        return -1;

    json = json_tokener_parse((char *)pbuf_s8);
    if (json == NULL){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Error(LINE=%d): json_tokener_parse()\n", __LINE__);
        return -1;
    }

    json_tmp = json_object_object_get(json, "BizType");
    pBizType = (char *)json_object_get_string(json_tmp);
    if (pBizType == NULL){
        ret_s32 = -1;
        goto exit_goto;
    }

    if(strcmp("OPEN_DOOR", pBizType) != 0)
    {
        json_tmp = json_object_object_get(json, "SubMsgType");
        pSubMsgType = (char *)json_object_get_string(json_tmp);
        if (pSubMsgType == NULL)
        {
            ret_s32 = -1;
            goto exit_goto;
        }
    }
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): BizType -> (%s)\n", __LINE__, pBizType);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_json Info(LINE=%d): SubMsgType -> (%s)\n", __LINE__, pSubMsgType);

    switch (pBizType[0]){
        case 'A':
            if (!strcmp(pSubMsgType, "Data")) /* if (!strcmp(pBizType, "Access") */
                ret_s32 = com_json_Access_Data_handle(json, sock);
            else if (!strcmp(pSubMsgType, "DataACK")) /* if (!strcmp(pBizType, "Access") */
                ret_s32 = com_json_general_ACK_handle(json);
            else if (!strcmp(pSubMsgType, "UploadACK")) /* if (!strcmp(pBizType, "Alarm") */
                ret_s32 = com_json_general_ACK_handle(json);
				else if (!strcmp(pSubMsgType, "UploadEventACK")) /* if (!strcmp(pBizType, "Alarm") */
            {
                ret_s32 = com_json_general_ACK_handle(json);
            }
        break;
        case 'F':                   
            if (!strcmp(pSubMsgType, "RequestFaceSyncAck"))
                {
                    ret_s32 = com_json_Face_RequestFaceSyncAck_handle(json, sock);
                }
            else if (!strcmp(pSubMsgType, "FaceSyncCheckAck"))
                {
                    ret_s32 = com_json_Face_FaceSyncCheckAck_handle(json);
                }
            else if (!strcmp(pSubMsgType, "FaceSync"))
                {
                    ret_s32 = com_json_Face_FaceSync_handle(json, sock);
                }
            else if (!strcmp(pSubMsgType, "FingerCharAck"))
                {
                    ret_s32 = com_json_Finger_Char_Ack_handle(json, sock);
                }
            else if (!strcmp(pSubMsgType, "RequestFPSyncAck"))
                {
                    ret_s32 = com_json_Finger_Sync_Request_Ack_handle(json, sock);
                }
            else if (!strcmp(pSubMsgType, "FPSyncCheckAck"))
                {
                    ret_s32 = com_json_Finger_Sync_Check_Ack_handle(json, sock);
                }
            else if (!strcmp(pSubMsgType, "FingerSync"))
                {
                    ret_s32 = com_json_Finger_Finger_Sync_handle(json, sock);
                }
            else if (!strcmp(pSubMsgType, "FingerMatch"))
                {
                    ret_s32 = com_json_Finger_Match_handle(json, sock);
                }
        break;  
		case 'H':
			if (!strcmp(pSubMsgType, "Remote"))
			{
				ret_s32 = com_json_App_HangUp_handle(json, sock);
			}
        break;
        case 'L':
            if (!strcmp(pSubMsgType, "CallAck")) /* if (!strcmp(pBizType, "Lift") */
            {
                ret_s32 = com_json_general_ACK_handle(json);
            } 
            else if (!strcmp(pSubMsgType,"Call"))
            {
                ret_s32 = com_json_dev_lift_call_handle(json, sock);
            }
        break;
        case 'M':
            if (!strcmp(pSubMsgType, "NoteRelease")) /* if (!strcmp(pBizType, "Message") */
                ret_s32 = com_json_Message_NoteRelease_handle(json, sock);
            else if(!strcmp(pSubMsgType, "NoteDel")) /* if (!strcmp(pBizType, "Message") */
                ret_s32 = com_json_Message_NoteDel_handle(json, sock);
            else if(!strcmp(pSubMsgType, "Weather")) /* if (!strcmp(pBizType, "Message") */
                ret_s32 = com_json_Message_Weather_handle(json, sock);
        break;
        case 'O':
            ret_s32 = com_json_OPEN_DOOR_handle(json, sock);
        break;    
        case 'V':
            if (!strcmp(pSubMsgType, "UnlockEventACK")) /* if (!strcmp(pBizType, "VDP") */
                ret_s32 = com_json_general_ACK_handle(json);
            else if (!strcmp(pSubMsgType, "CallInfo")) /* if (!strcmp(pBizType, "VDP") */
                ret_s32 = com_json_general_ACK_handle(json);
        break;
        case 'U':
            if (!strcmp(pSubMsgType, "TalkingUnlock")) /* if (!strcmp(pBizType, "Unlock") */
                ret_s32 = com_json_Unlock_SessionUnlock_handle(json, sock, emTalkingUnlock);
            else if (!strcmp(pSubMsgType, "WatchingUnlock")) /* if (!strcmp(pBizType, "Unlock") */
                ret_s32 = com_json_Unlock_SessionUnlock_handle(json, sock, emWatchingUnlock);
            else if (!strcmp(pSubMsgType, "CheckPasswordACK")) /* if (!strcmp(pBizType, "Unlock") */
                ret_s32 = com_json_Unlock_CheckPasswordACK_handle(json, sock);
			else if (!strcmp(pSubMsgType, "Remote"))
			{
				ret_s32 = com_json_Unlock_Remote_handle(json, sock);
			}
        break;
		case 'D':
			if(!strcmp(pSubMsgType, "Control"))
				ret_s32 = com_json_door_control_handle(json, sock);
			else if(!strcmp(pSubMsgType, "ControlACK"))
				ret_s32 = com_json_general_ACK_handle(json);
		break;
		case 'B':
		    if(!strcmp(pSubMsgType, "Upgrade"))
				ret_s32 = com_json_blacklist_upgrade_handle(json, sock);
		break;
		case 'T':
			if (!strcmp(pSubMsgType, "TransferACK"))
				ret_s32 = com_json_general_ACK_handle(json);
		break;
        default:
            ret_s32 = -1;
        break;
    }
exit_goto:
    json_object_put(json);
    return ret_s32;
}
/**@END! s32 com_json_unpack(const s32 sock, const s8 *pbuf_s8, const u32 len) !\(^o^)/~ 结束咯 */

/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_init(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_json_init(void *parg){
    return 0;
}
/**@END! s32 com_json_init(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_start(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 启动
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_json_start(void *parg){
    return 0;
}
/**@END! s32 com_json_start(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_stop(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 停止
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_json_stop(void *parg){
    return 0;
}
/**@END! s32 com_json_stop(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_uninit(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 反初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_json_uninit(void *parg){
    return 0;
}
/**@END! s32 com_json_uninit(void *parg) !\(^o^)/~ 结束咯 */















#if (0)
#include "com_func.h"

#include "com_ctrl.h"

#include "com_timer.h"

#include "com_unlock.h"

#ifdef SSSSSSSSSStart_Start_Unpack_ACK /* UnPack ACK */
#endif

/**
* \fn      SdInt com_json_ack_header_pack(SdInt iSock, IN SdChar *pszBuf, SdInt iLen)
* \brief   添加应答包包头, 并发送数据
* \param   SdInt iSock - Socket句柄
* \param   IN SdChar *pszBuf - 数据
* \param   SdInt iLen - 数据长度
* \return  成功, 返回(0)
* \return  失败, 返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_json_ack_header_pack(SdInt iSock, IN SdChar *pszBuf, SdInt iLen)
{
   ProtocolHeader stProtocolHeader;
   SdInt iHeadLen = sizeof(ProtocolHeader);
   SdInt iRet = 0;

   stProtocolHeader.iMagic = iHeaderMagic;
   stProtocolHeader.iLen = iLen - iHeadLen;
   com_func_protocol_header_conver(&stProtocolHeader, &stProtocolHeader); /* 协议头字节序列转换 */

   memcpy(pszBuf, &stProtocolHeader, iHeadLen); /* 添加协议头 */

   iRet = com_func_TCPdata_send(iSock, pszBuf, iLen, 500);

   return iRet;
}

/**
* \fn      SdInt com_json_ack_pack(SdInt iSock, IN SdChar *pszBizType, IN SdChar *pszSubMsgType, IN SdChar *pszPacketNo, enACK emAck,
               void (*f_JsonPack)(IN json_object *, OUT SdChar **, OUT SdInt *, void *ptr), void *ptr)
* \brief   制作应答包
* \param   SdInt iSock - Socket句柄
* \param   IN SdChar *pszBizType - JSON包"BizType"值
* \param   IN SdChar *pszSubMsgType - JSON包"SubMsgType"值
* \param   IN SdChar *pszPacketNo - JSON包"PacketNo"值
* \param   enACK emAck - 是否应答, 若应答-> 是否成功, 并添加JSON包"result"的值
* \param   void (*f_JsonPack)(IN json_object *, OUT SdChar **, OUT SdInt *, void *ptr) - 回调函数
               回调函数为JSON包添加其他Key和值, 并将JSON包转换成字符串输出
* \param   void *ptr - 回调函数参数
* \return  成功, 返回(0)
* \return  失败, 返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_json_ack_pack(SdInt iSock, IN SdChar *pszBizType, IN SdChar *pszSubMsgType, SdInt iPacketNo, enACK emAck,
   void (*f_JsonPack)(IN json_object *, OUT SdChar **, OUT SdInt *, void *ptr), void *ptr){
   json_object *json = SD_NULL;
   SdChar *pszBuf = SD_NULL;
   SdInt iJsonLen = 0;
   SdInt iRet = 0;

   json = json_object_new_object();
   if(json == SD_NULL)
      return -1;

   json_object_object_add(json, "BizType", json_object_new_string(pszBizType));
   json_object_object_add(json, "SubMsgType", json_object_new_string(pszSubMsgType));
   json_object_object_add(json, "PacketNo", json_object_new_int(iPacketNo)); 
   if(emAck != emAckNo)
      json_object_object_add(json, "result", json_object_new_int(emAck));

   if(f_JsonPack != SD_NULL)
      (f_JsonPack)(json, &pszBuf, &iJsonLen, ptr); /* 回调函数, 添加其他部分 */
   else /* 无回调函数, 应在此添加转换数据 */
   {
      /* Json对象转换成 String, 此部分不要修改 */
      SdChar *pszTmpBuf = SD_NULL;
      SdInt iHeaderLen = sizeof(ProtocolHeader); /* 协议头 */
      pszTmpBuf = (SdChar *)json_object_to_json_string(json); /* 转换成字符串 */
      iJsonLen = strlen(pszTmpBuf) + 1 + iHeaderLen;
      pszBuf = ut_mem_new(SdChar, iJsonLen); /* 分配资源, 存储数据 */
      memcpy(pszBuf + iHeaderLen,  pszTmpBuf, (iJsonLen - iHeaderLen));
   }

   iRet = com_json_ack_header_pack(iSock, pszBuf, iJsonLen); /* 添加包头, 数据发送 */
   ut_mem_free(pszBuf); /* 发送完成, 释放资源 */

   json_object_put(json); /* 释放资源 */
   return iRet;
}

SdInt com_json_SERVERnetadmin_ack_unpack(SdInt iSock, IN SdChar *pszBuf, SdInt iLen, IN LPCOMManage pstCOMManage){
   json_object *json = SD_NULL;
   json_object *json_tmp = SD_NULL;
   SdInt iRet = 0;
   SdUInt uiPackId = 0;
   SdChar *pszBizType = SD_NULL;
   SdChar *pszSubMsgType = SD_NULL;

   if(iLen == 0)
      return -1;

   if(pstCOMManage == SD_NULL)
      return -1;

   json = json_tokener_parse(pszBuf);
   if(json == SD_NULL)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s %d]: json_tokener_parse FAIL", __FILE__, __LINE__);
      return -1;
   }

   json_tmp = json_object_object_get(json, "PacketNo");
   if(json_tmp == SD_NULL)
      return -1;

   json_tmp = json_object_object_get(json, "BizType");
   pszBizType = (SdChar *)json_object_get_string(json_tmp);
   if(pszBizType == SD_NULL)
      return -1;

   json_tmp = json_object_object_get(json, "SubMsgType");
   pszSubMsgType = (SdChar *)json_object_get_string(json_tmp);
   if(pszSubMsgType == SD_NULL)
      return -1;

   uiPackId = json_object_get_int(json_tmp);
   json_object_put(json_tmp);

   iRet = com_timer_client_resend_delete(emTimerTypeServerNetAdmin, uiPackId);

   return iRet;
}

SdInt com_json_SERVERterm_ack_unpack(SdInt iSock, IN SdChar *pszBuf, SdInt iLen){
   json_object *json = SD_NULL;
   json_object *json_tmp = SD_NULL;
   SdInt iRet = 0;
   SdUInt uiPackId = 0;

   if(iLen == 0)
      return -1;

   json = json_tokener_parse(pszBuf);
   if(json == SD_NULL)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s %d]: json_tokener_parse FAIL", __FILE__, __LINE__);
      return -1;
   }

   json_tmp = json_object_object_get(json, "PacketNo");
   if(json_tmp == SD_NULL)
      return -1;

   uiPackId = json_object_get_int(json_tmp);

   iRet = com_timer_client_resend_delete(emTimerTyperServerTerm, uiPackId);


   json_object_put(json_tmp);
   json_object_put(json); /* 释放资源, 不知道对不对? */

   return iRet;
}

#ifdef EEEEEEEEEEnd_End_Unpack_ACK /* UnPack ACK */
#endif

#ifdef SSSSSSSSSStart_Start_SERVERnetadmin /* Pack 网管服务器 */
#endif

/*
   添加8byte包头, 数据包发送
   -1, 发送失败; 0, 发送成功
*/
SdInt com_json_SERVERnetadmin_header_pack(IN SdChar *pszBuf, SdInt iLen)
{
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   ProtocolHeader stProtocolHeader;
   SdInt iHeadLen = sizeof(ProtocolHeader);
   SdInt iRet = 0;

   stProtocolHeader.iMagic = iHeaderMagic;
   stProtocolHeader.iLen = iLen - iHeadLen;
   com_func_protocol_header_conver(&stProtocolHeader, &stProtocolHeader); /* 协议头字节序列转换 */

   memcpy(pszBuf, &stProtocolHeader, sizeof(ProtocolHeader)); /* 添加协议头 */

   iRet = com_client_SERVERnetadmin_sendto(pszBuf, iLen) ;

   return iRet;
}

SdInt com_json_SERVERnetadmin_pack(IN SdChar *pszBizType, IN SdChar *pszSubMsgType, 
   void (*f_JsonPack)(IN json_object *, OUT SdChar **, OUT SdInt *, void *ptr), void *ptr)
{
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   json_object *json = SD_NULL;
   SdChar *pszBuf = SD_NULL;
   SdInt iJsonLen = 0;
   SdInt iRet = 0;

   json = json_object_new_object();
   if(json == SD_NULL)
      return -1;

   json_object_object_add(json, "BizType", json_object_new_string(pszBizType));
   json_object_object_add(json, "SubMsgType", json_object_new_string(pszSubMsgType));
   json_object_object_add(json, "PacketNo", json_object_new_int(pstCOMManage->uiPackId)); 
   
   if(f_JsonPack != SD_NULL)
      (f_JsonPack)(json, &pszBuf, &iJsonLen, ptr); /* 回调函数, 添加其他部分 */
   else /* 无回调函数, 此情况不应该出现..但还是要发出去.. */
   {
      /* Json对象转换成 String, 此部分不要修改 */
      SdChar *pszTmpBuf = SD_NULL;
      SdInt iHeaderLen = sizeof(ProtocolHeader); /* 协议头 */
      pszTmpBuf = (SdChar *)json_object_to_json_string(json); /* 转换成字符串 */
      iJsonLen = strlen(pszTmpBuf) + 1 + iHeaderLen;
      pszBuf = ut_mem_new(SdChar, iJsonLen); /* 分配资源, 存储数据 */
      memcpy(pszBuf + iHeaderLen,  pszTmpBuf, (iJsonLen - iHeaderLen));
   }

   iRet = com_json_SERVERnetadmin_header_pack(pszBuf, iJsonLen); /* 添加包头, 数据发送 */
   if(iRet == -1) /* 打包并发送到网管服务器失败 */
      ut_mem_free(pszBuf); /* 释放资源 */

   json_object_put(json); /* 释放资源 */
   return iRet;
}

void com_json_Message_NoteBrowse_callback(IN json_object *json, OUT SdChar **ppszBuf, OUT SdInt *piLen,void *ptr){
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPReadAreaInfoDef pstReadAreaInfoDef = (LPReadAreaInfoDef)ptr;
   LPCOMCfgDef pstCOMCfgDef = SD_NULL;
   json_object *BrowseInfo = json_object_new_object();

   if(pstCOMManage == SD_NULL)
      return ;
   pstCOMCfgDef = &pstCOMManage->stCOMCfg;

   json_object_object_add(json, "BrowseInfo", BrowseInfo); 
   json_object_object_add(BrowseInfo, "uc", json_object_new_int(pstCOMCfgDef->uiUcCode)); 
   json_object_object_add(BrowseInfo, "NoteSn", json_object_new_int(pstReadAreaInfoDef->ulIndex)); 
   json_object_object_add(BrowseInfo, "AttachmentRev", json_object_new_string(pstReadAreaInfoDef->szRevTime)); 
   json_object_object_add(BrowseInfo, "Browse", json_object_new_string(pstReadAreaInfoDef->szBrowseTime)); 

   /* Json对象转换成, String */
   {
      SdChar *pszTmpBuf = SD_NULL;
      SdInt iHeaderLen = sizeof(ProtocolHeader); /* 协议头 */
      pszTmpBuf = (SdChar *)json_object_to_json_string(json); /* 转换成字符串 */
      *piLen = strlen(pszTmpBuf) + 1 + iHeaderLen;
      *ppszBuf = ut_mem_new(SdChar, *piLen); /* 分配资源, 存储数据 */
      memcpy(*ppszBuf + iHeaderLen,  pszTmpBuf, (*piLen - iHeaderLen));
   }

   /* 释放资源 */
   json_object_put(BrowseInfo);

   return ;
}


SdInt com_json_Message_NoteBrowse_pack(void *ptr)
{
   SdInt iRet = 0;
   iRet =  com_json_SERVERnetadmin_pack("Message", "NoteBrowse", com_json_Message_NoteBrowse_callback, ptr);
   return iRet;
}
111111111111111111111111111111111111111111111111111111111
void com_json_VDP_UnlockEvent_callback(IN json_object *json, OUT SdChar **ppszBuf, OUT SdInt *piLen, void *ptr){
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPCOMCfgDef pstCOMCfgDef = SD_NULL;
   LPDoorRecordDef pstDoorRecordDef = (LPDoorRecordDef)ptr;
   SdInt iUnlockType = 0;
   SdInt iOperateUc = 0;
   SdChar szTime[48] = {0};
   json_object *EventInfo = SD_NULL;
   json_object *tmpJson = SD_NULL;
   time_t nowTime;
   struct tm *tmNowTime;

   if(pstCOMManage == SD_NULL)
      return ;
   pstCOMCfgDef = &pstCOMManage->stCOMCfg;

   if(ptr == SD_NULL)
      return ;

   EventInfo = json_object_new_object();
   if(EventInfo == SD_NULL)
      return ;

   json_object_object_add(json, "EventInfo", EventInfo);

   switch(pstDoorRecordDef->emUnlockType)
   {
      case emLocalPasswdUnlock: /* 本地密码开锁 */
         iUnlockType = 2;
         iOperateUc = pstCOMCfgDef->uiUcCode; /* 开锁UC为主机UC */
      break;
      case emTalkingUnlock: /* 会话过程中开锁 */
         iUnlockType = 0;
         iOperateUc = pstDoorRecordDef->iOperatoruc; /* 开门终端UC号 */
      break;
      case emWatchingUnlock: /* 监视过程中开锁 */
         iUnlockType = 1;
         iOperateUc = pstDoorRecordDef->iOperatoruc; /* 开门终端UC号 */
      break;
      default : /* 不认识的开锁类型 */
         iUnlockType = 3;
         iOperateUc = pstDoorRecordDef->iOperatoruc;
      break;
   }

   json_object_object_add(EventInfo, "EventType", json_object_new_int(iUnlockType)); /* 事件类型 */
   json_object_object_add(EventInfo, "Operatoruc", json_object_new_int(iOperateUc)); /* 开门者UC */
   json_object_object_add(EventInfo, "Dooruc", json_object_new_int(pstCOMCfgDef->uiUcCode)); /* 开门主机UC */
   json_object_object_add(EventInfo, "DoorNum", json_object_new_int(0)); /* 开门主机门号 */

   time(&nowTime);
   tmNowTime = localtime(&nowTime);
   sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", 1900+tmNowTime->tm_year, tmNowTime->tm_mon
      , tmNowTime->tm_mday, tmNowTime->tm_hour, tmNowTime->tm_min, tmNowTime->tm_sec);
   json_object_object_add(EventInfo, "Date", json_object_new_string(szTime)); /* 开门时间 */

   /* Json对象转换成 String, 此部分不要修改 */
   {
      SdChar *pszTmpBuf = SD_NULL;
      SdInt iHeaderLen = sizeof(ProtocolHeader); /* 协议头 */
      pszTmpBuf = (SdChar *)json_object_to_json_string(json); /* 转换成字符串 */
      *piLen = strlen(pszTmpBuf) + 1 + iHeaderLen;
      *ppszBuf = ut_mem_new(SdChar, *piLen); /* 分配资源, 存储数据 */
      memcpy(*ppszBuf + iHeaderLen,  pszTmpBuf, (*piLen - iHeaderLen));
   }

   /* 释放资源 */
   json_object_put(EventInfo);

   return ;
}


SdInt com_json_VDP_UnlockEvent_pack(void *ptr)
{
   SdInt iRet = 0;
   iRet =  com_json_SERVERnetadmin_pack("VDP", "UnlockEvent", com_json_VDP_UnlockEvent_callback, ptr);
   return iRet;
}
222222222222222222222222222222222
void com_json_VDP_CallInfo_callback(IN json_object *json, OUT SdChar **ppszBuf, OUT SdInt *piLen, void *ptr){
   LPCallEventReportDef pstCallEventReportDef = (LPCallEventReportDef)ptr;
   boolean IfUnlock = FALSE; /**< 是否开锁(0未开、1开锁) */
   SdInt iCallOrWatch = 0; /**< 呼叫(0), 监视(1) */
   json_object *EventInfo = SD_NULL;

   if(ptr == SD_NULL)
      return ;

   EventInfo = json_object_new_object();
   if(EventInfo == SD_NULL)
      return ;

   if(pstCallEventReportDef->iIfUnLock) /**< 是否开锁(0未开、1开锁) */
      IfUnlock = TRUE;
   if(pstCallEventReportDef->iType) /**< 呼叫(0), 监视(1) */
      iCallOrWatch = 0;

   json_object_object_add(json, "EventInfo", EventInfo);

   json_object_object_add(EventInfo, "EventType", json_object_new_int(iCallOrWatch));
   json_object_object_add(EventInfo, "Start", 
      json_object_new_string(pstCallEventReportDef->pszStartTime)); /* 开始时间 */
   json_object_object_add(EventInfo, "Answer", 
      json_object_new_string(pstCallEventReportDef->pszAnsTime)); /* 开始开始时间 */
   json_object_object_add(EventInfo, "End", 
      json_object_new_string(pstCallEventReportDef->pszHangUpTime)); /* 结束时间 */
   json_object_object_add(EventInfo, "Image", 
      json_object_new_string(pstCallEventReportDef->szImage)); /* 图片 */
   json_object_object_add(EventInfo, "Unlock", json_object_new_boolean(IfUnlock)); /* 有没有开锁 */
   json_object_object_add(EventInfo, "Srcuc", json_object_new_int(pstCallEventReportDef->iSrcuc)); /* 发起方UC */
   json_object_object_add(EventInfo, "Desuc", json_object_new_int(pstCallEventReportDef->iDesuc)); /* 接受方UC */

   /* Json对象转换成 String, 此部分不要修改 */
   {
      SdChar *pszTmpBuf = SD_NULL;
      SdInt iHeaderLen = sizeof(ProtocolHeader); /* 协议头 */
      pszTmpBuf = (SdChar *)json_object_to_json_string(json); /* 转换成字符串 */
      *piLen = strlen(pszTmpBuf) + 1 + iHeaderLen;
      *ppszBuf = ut_mem_new(SdChar, *piLen); /* 分配资源, 存储数据 */
      memcpy(*ppszBuf + iHeaderLen,  pszTmpBuf, (*piLen - iHeaderLen));
   }

   /* 释放资源 */
   json_object_put(EventInfo);

   return ;
}

SdInt com_json_VDP_CallInfo_pack(void *ptr)
{
   SdInt iRet = 0;
   iRet =  com_json_SERVERnetadmin_pack("VDP", "CallInfo", com_json_VDP_CallInfo_callback, ptr);
   return iRet;
}
3333333333333333333
void com_json_Access_Data_callback(IN json_object *json, OUT SdChar **ppszBuf, OUT SdInt *piLen, void *ptr){
   if(ptr == SD_NULL)
      return ;

   json_object_object_add(json, "BIN", json_object_new_string(ptr)); /* 结束时间 */

   /* Json对象转换成 String, 此部分不要修改 */
   {
      SdChar *pszTmpBuf = SD_NULL;
      SdInt iHeaderLen = sizeof(ProtocolHeader); /* 协议头 */
      pszTmpBuf = (SdChar *)json_object_to_json_string(json); /* 转换成字符串 */
      *piLen = strlen(pszTmpBuf) + 1 + iHeaderLen;
      *ppszBuf = ut_mem_new(SdChar, *piLen); /* 分配资源, 存储数据 */
      memcpy(*ppszBuf + iHeaderLen,  pszTmpBuf, (*piLen - iHeaderLen));
   }

   /* 释放资源 */

   return ;
}

SdInt com_json_Access_Data_pack(void *ptr)
{
   SdInt iRet = 0;
   iRet =  com_json_SERVERnetadmin_pack("Access", "Data", com_json_Access_Data_callback, ptr);
   return iRet;
}

#ifdef EEEEEEEEEEnd_End_SERVERnetadmin /* Pack 网管服务器 */
#endif

#ifdef SSSSSSSSSStart_Start_SERVERterm /* Pack 终端服务器 */
#endif
/*
   添加8byte包头, 数据包发送
   -1, 发送失败; 0, 发送成功
*/
SdInt com_json_SERVERterm_header_pack(IN SdChar *pszIP, SdUShort  usPort, IN SdChar *pszBuf, SdInt iLen)
{
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   ProtocolHeader stProtocolHeader;
   SdInt iHeadLen = sizeof(ProtocolHeader);
   SdInt iRet = 0;

   stProtocolHeader.iMagic = iHeaderMagic;
   stProtocolHeader.iLen = iLen - iHeadLen;
   com_func_protocol_header_conver(&stProtocolHeader, &stProtocolHeader); /* 协议头字节序列转换 */

   memcpy(pszBuf, &stProtocolHeader, sizeof(ProtocolHeader)); /* 添加协议头 */

   iRet = com_client_SERVERterm_sendto(pszIP, usPort, pszBuf, iLen) ;

   return iRet;
}

SdInt com_json_SERVERterm_pack(IN SdChar *pszIP, SdUShort  usPort, IN SdChar *pszBizType, IN SdChar *pszSubMsgType, 
   void (*f_JsonPack)(IN json_object *, OUT SdChar **, OUT SdInt *, void *ptr), void *ptr)
{
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   json_object *json = SD_NULL;
   SdChar *pszBuf = SD_NULL;
   SdInt iJsonLen = 0;
   SdInt iRet = 0;

   json = json_object_new_object();
   if(json == SD_NULL)
      return -1;

   json_object_object_add(json, "BizType", json_object_new_string(pszBizType));
   json_object_object_add(json, "SubMsgType", json_object_new_string(pszSubMsgType));
   json_object_object_add(json, "PacketNo", json_object_new_int(pstCOMManage->uiPackId)); 
   
   if(f_JsonPack != SD_NULL)
      (f_JsonPack)(json, &pszBuf, &iJsonLen, ptr); /* 回调函数, 添加其他部分 */
   else /* 无回调函数, 此情况不应该出现..但还是要发出去.. */
   {
      /* Json对象转换成 String, 此部分不要修改 */
      SdChar *pszTmpBuf = SD_NULL;
      SdInt iHeaderLen = sizeof(ProtocolHeader); /* 协议头 */
      pszTmpBuf = (SdChar *)json_object_to_json_string(json); /* 转换成字符串 */
      iJsonLen = strlen(pszTmpBuf) + 1 + iHeaderLen;
      pszBuf = ut_mem_new(SdChar, iJsonLen); /* 分配资源, 存储数据 */
      memcpy(pszBuf + iHeaderLen,  pszTmpBuf, (iJsonLen - iHeaderLen));
   }

   iRet = com_json_SERVERterm_header_pack(pszIP, usPort, pszBuf, iJsonLen); /* 添加包头, 数据发送 */
   if(iRet == -1) /* 打包并发送到网管服务器失败 */
      ut_mem_free(pszBuf); /* 释放资源 */

   json_object_put(json); /* 释放资源 */
   return iRet;
}

#ifdef EEEEEEEEEEnd_End_SERVERnetadmin /* Pack 终端服务器 */
#endif


#ifdef SSSSSSSSSStart_Start_Unpack /* UnPack */
#endif
444444444444444444444444444444
SdInt com_json_Message_NoteDel_unpack(json_object *json, SdInt iSock){
   json_object *json_tmp = SD_NULL;
   SdInt iRet = 0;
   SdInt iPacketNo = SD_NULL;
   SdULong ulNoteSn = 0;

   /* 包号 */
   json_tmp = json_object_object_get(json, "PacketNo");
   iPacketNo = json_object_get_int(json_tmp);

   json_tmp = json_object_object_get(json, "NoteSn");
   ulNoteSn = json_object_get_int(json_tmp);

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: Messsage_NoteDel Sn = [%d]", __FILE__, __LINE__, ulNoteSn);

   iRet = com_json_ack_pack(iSock, "Messsage", "NoteDelACK", iPacketNo, emAckSuccess, SD_NULL, SD_NULL);
   com_func_msg_send(emModDATA, COM_DATA_AREAINFO_DELETEED, ulNoteSn, 0, 0, SD_NULL);
   return iRet;
}

SdInt com_json_Message_NoteRelease_unpack(json_object *json, SdInt iSock){
   json_object *json_tmp = SD_NULL;
   json_object *obj_tmp = SD_NULL;
   AreaInfoDef stAreaInfoDef;
   SdInt iArryLen = 0;
   SdInt iRet = 0;
   SdInt iPacketNo = SD_NULL;
   SdChar *pszStr = SD_NULL;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d (%s)]:", __FILE__, __LINE__, __FUNCTION__);

   memset(&stAreaInfoDef, '\0', sizeof(stAreaInfoDef));

   /* 包号 */
   json_tmp = json_object_object_get(json, "PacketNo");
   iPacketNo =json_object_get_int(json_tmp);

   /* 事件类型 */
   json_tmp = json_object_object_get(json, "NoteType");
   if(json_tmp == SD_NULL) return -1;
   {
      SdInt iDataType = json_object_get_int(json_tmp);
      if(iDataType == 0) /* 物业信息 */
         stAreaInfoDef.emDataType = emAreaMsg;
      else if(iDataType == 1) /* 广告信息 */
         stAreaInfoDef.emDataType = emAdMsg;
      else if(iDataType == 2) /* 紧急通知 */
         stAreaInfoDef.emDataType = emUrgentMsg;
//      else if(iDataType == 3) /* 定位信息 */
      else /* TBD, 不知道是啥 */
         stAreaInfoDef.emDataType = emAreaMsg;
   }
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: emDataType = %d", __FILE__, __LINE__, stAreaInfoDef.emDataType);

   /* 创建时间 */
   obj_tmp = json_object_object_get(json, "DataTime");
   if(obj_tmp == SD_NULL) return -1;
   pszStr = (char *)json_object_get_string(obj_tmp);
   strcpy(stAreaInfoDef.szCreateTime, pszStr);
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: CreateTime = [%s]", __FILE__, __LINE__, stAreaInfoDef.szCreateTime);

   /* 消息信息 */
   obj_tmp = json_object_object_get(json, "NoteInfo");
   if(obj_tmp == SD_NULL) return -1;
   /* 消息流水号 */
   json_tmp = json_object_object_get(obj_tmp, "NoteSn");
   if(json_tmp == SD_NULL) return -1;
   stAreaInfoDef.ulIndex = json_object_get_int(json_tmp);
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: ulIndex = %d", __FILE__, __LINE__, stAreaInfoDef.ulIndex);
   /* 消息生效时间 */
   json_tmp = json_object_object_get(obj_tmp, "Start");
   if(json_tmp == SD_NULL) return -1;
   pszStr = (char *)json_object_get_string(json_tmp);
   strcpy(stAreaInfoDef.szStart, pszStr);
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: Start = [%s]", __FILE__, __LINE__, stAreaInfoDef.szStart);
   /* 消息失效时间 */
   json_tmp = json_object_object_get(obj_tmp, "End");
   if(json_tmp == SD_NULL) return -1;
   pszStr = (char *)json_object_get_string(json_tmp);
   strcpy(stAreaInfoDef.szEnd, pszStr);
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: End = [%s]", __FILE__, __LINE__, stAreaInfoDef.szEnd);
   /* 消息标题 */
   json_tmp = json_object_object_get(obj_tmp, "Title");
   if(json_tmp == SD_NULL) return -1;
   pszStr = (char *)json_object_get_string(json_tmp);
   if(strlen(pszStr) >= MAX_NAME_LENGTH)
      pszStr[MAX_NAME_LENGTH] = '\0';
   strcpy(stAreaInfoDef.szTitle, pszStr);
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: Title = [%s]", __FILE__, __LINE__, stAreaInfoDef.szTitle);
   /* 消息内容 */
   json_tmp = json_object_object_get(obj_tmp, "Msg");
   if(json_tmp == SD_NULL) return -1;
   pszStr = (char *)json_object_get_string(json_tmp);
   if(strlen(pszStr) >= MAX_LOG_DATA_LENGTH)
      pszStr[MAX_LOG_DATA_LENGTH] = '\0';
   strcpy(stAreaInfoDef.szContent, pszStr);
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: Content = [%s]", __FILE__, __LINE__, stAreaInfoDef.szContent);
   /* 消息循环时间 */
   json_tmp = json_object_object_get(obj_tmp, "loop");
   if(json_tmp == SD_NULL) /* 一次性消息 */
   {
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: Oncely Msg", __FILE__, __LINE__);
      stAreaInfoDef.emLoopType = emOnce;
   }
   else /* 每周消息 */
   {
      SdInt ii = 0;
      iArryLen = json_object_array_length(json_tmp);
      stAreaInfoDef.emLoopType = emWeekly;
      if(iArryLen > MAX_LOOP_TIME_NUM)
         iArryLen = MAX_LOOP_TIME_NUM;
      for(ii = 0; ii < iArryLen; ii++)
      {
         json_object *elm = json_object_array_get_idx(json_tmp, ii);
         json_object *obj = json_object_array_get_idx(json_tmp, ii);
         obj = json_object_object_get(elm , "Loopstart");
         if(obj != SD_NULL)
            pszStr = (char *)json_object_get_string(obj);
         pszStr[5] = '\0'; /* 防止数据太长 */
         strcpy(stAreaInfoDef.szLoopStart[ii], pszStr);
         UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: LoopStart = [%s]", __FILE__, __LINE__, stAreaInfoDef.szLoopStart[ii]);

         obj = json_object_object_get(elm , "Loopend");
         if(obj != SD_NULL)
            pszStr = (char *)json_object_get_string(obj);
         pszStr[5] = '\0'; /* 防止数据太长 */
         strcpy(stAreaInfoDef.szLoopEnd[ii], pszStr);
         UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: LoopEnd = [%s]", __FILE__, __LINE__, stAreaInfoDef.szLoopEnd[ii]);

         obj = json_object_object_get(elm , "Loopweek");
         if(obj != SD_NULL)
            pszStr = (char *)json_object_get_string(obj);
         pszStr[MAX_NAME_LENGTH - 1] = '\0'; /* 防止数据太长 */
         strcpy(stAreaInfoDef.szLoopWeek[ii], pszStr);
         UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: LoopWeek = [%s]", __FILE__, __LINE__, stAreaInfoDef.szLoopWeek[ii]);
      }
   }
   /* 附件, 广告有效 */
   json_tmp = json_object_object_get(obj_tmp, "Attachment");
   if(json_tmp != SD_NULL) /* 有附件 */
   {
      SdInt ii = 0, iFilePathLen = 0, iNameLen =  0;
      SdChar szName[64] = {0}, szType[16] = {0};
      SdChar szDownloadFrom[512] = {0},  szSaveTo[512] = {0};
      LPCOMManage pstCOMManage = com_ctrl_commanage_get();

      iArryLen = json_object_array_length(json_tmp);
      for(ii = 0; ii < iArryLen; ii++)
      {
         json_object *elm = json_object_array_get_idx(json_tmp, ii); /* 取得元素 */
         pszStr = (char *)json_object_get_string(elm);

         com_func_pathfile_file_get(pszStr, szName, sizeof(szName)); /* 得到文件名 */
         iNameLen = strlen(szName);
         if((iFilePathLen + iNameLen) > MAX_FILE_PATH_LENGTH) /* 缓冲区满了, 退出 */
            break;

         strcpy(stAreaInfoDef.szFilePath + iFilePathLen, (char *)szName);
         iFilePathLen += iNameLen;
         stAreaInfoDef.szFilePath[iFilePathLen++] = '/'; /* 插入分隔符 */

         com_func_areainfo_savepath_get(stAreaInfoDef.emDataType,  szName, szSaveTo); /* 取得小区信息存储路径 */

         sprintf(szDownloadFrom, "ftp://%s/\"%s\"", pstCOMManage->stCOMCfg.szCfgServerIP, pszStr);
         com_ftp_download(szDownloadFrom, szSaveTo);
      }
      if(iFilePathLen > 0)
         --iFilePathLen;
      stAreaInfoDef.szFilePath[iFilePathLen] = '\0'; /* 插入结束符 */
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: FilePath = [%s], Len = %d", __FILE__, __LINE__,  stAreaInfoDef.szFilePath, iFilePathLen);
   }

   iRet = com_json_ack_pack(iSock, "Messsage", "NoteReleaseACK", iPacketNo, emAckSuccess, SD_NULL, SD_NULL);
   com_func_msg_send(emModDATA, COM_DATA_AREAINFO_RECV, 0, 0, sizeof(AreaInfoDef), &stAreaInfoDef);
   return iRet;
}
555555555555555555
SdInt com_json_Message_Weather_unpack(json_object *json, SdInt iSock){
   json_object *json_tmp = SD_NULL;
   json_object *json_arry = SD_NULL;
   SdInt iPacketNo = SD_NULL;
   SdInt iRet = 0;
   AreaInfoDef stAreaInfoDef;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d (%s)]:", __FILE__, __LINE__, __FUNCTION__);

   memset(&stAreaInfoDef, '\0', sizeof(stAreaInfoDef));

   /* 包号 */
   json_tmp = json_object_object_get(json, "PacketNo");
   iPacketNo = json_object_get_int(json_tmp);

   json_arry = json_object_object_get(json, "WeatherInfo");
   if(json_arry != SD_NULL) /* 有天气信息 */
   {
      SdInt ii = 0;
      SdChar *pszStr = SD_NULL;
      SdInt iArryLen = 0, iStrLen = 0;
      SdChar pszSendBuf[384] = {0};

      iArryLen = json_object_array_length(json_arry);
      for(ii = 0; ii < iArryLen; ii++)
      {
         json_object *elm = json_object_array_get_idx(json_arry, ii); /* 取得元素 */

         stAreaInfoDef.emLoopType = emEveryday;
         stAreaInfoDef.emDataType =  emWeatherMsg;

         json_tmp = json_object_object_get(elm, "day"); /* 星期 */
         if(json_tmp == SD_NULL)
            pszStr[0] = '\0';
         else
            pszStr = (char *)json_object_get_string(json_tmp);
         iStrLen = sprintf(pszSendBuf, "%s|", pszStr);

         json_tmp = json_object_object_get(elm, "date"); /* 日期 */
         if(json_tmp == SD_NULL)
            pszStr[0] = '\0';
         else
            pszStr = (char *)json_object_get_string(json_tmp);
         iStrLen += sprintf(pszSendBuf+iStrLen, "%s|", pszStr);
         strncpy(stAreaInfoDef.szTitle, pszStr, MAX_NAME_LENGTH);

         json_tmp = json_object_object_get(elm, "range"); /* 每天的气温范围 */
         if(json_tmp == SD_NULL)
            pszStr[0] = '\0';
         else
            pszStr = (char *)json_object_get_string(json_tmp);
         iStrLen += sprintf(pszSendBuf+iStrLen, "%s|", pszStr);

         json_tmp = json_object_object_get(json, "temperature"); /* 当前时刻时实温度 */
         if(json_tmp == SD_NULL)
            pszStr[0] = '\0';
         else
            pszStr = (char *)json_object_get_string(json_tmp);
         iStrLen += sprintf(pszSendBuf+iStrLen, "%s|", pszStr);

         json_tmp = json_object_object_get(elm, "weather"); /* 天气描述 */
         if(json_tmp == SD_NULL)
            pszStr[0] = '\0';
         else
            pszStr = (char *)json_object_get_string(json_tmp);
         iStrLen += sprintf(pszSendBuf+iStrLen, "%s|", pszStr);

         json_tmp = json_object_object_get(elm, "wind"); /* 风力大小 */
         if(json_tmp == SD_NULL)
            pszStr[0] = '\0';
         else
            pszStr = (char *)json_object_get_string(json_tmp);
         iStrLen += sprintf(pszSendBuf+iStrLen, "%s|", pszStr);

         json_tmp = json_object_object_get(elm, "pm25"); /* PM2.5情况 */
         if(json_tmp == SD_NULL)
            pszStr[0] = '\0';
         else
            pszStr = (char *)json_object_get_string(json_tmp);
         iStrLen += sprintf(pszSendBuf+iStrLen, "%s|", pszStr);

         json_tmp = json_object_object_get(elm, "light"); /* 紫外线强度 */
         if(json_tmp == SD_NULL)
            pszStr[0] = '\0';
         else
            pszStr = (char *)json_object_get_string(json_tmp);
         iStrLen += sprintf(pszSendBuf+iStrLen, "%s", pszStr);
         pszSendBuf[iStrLen] = '\0'; /* 填充字符串结尾 */
         strncpy(stAreaInfoDef.szContent, pszSendBuf, MAX_LOG_DATA_LENGTH);
         UT_LOG_LOGOUT_INFO(emModCOM, 0, "((%s) Weather: (%s))", stAreaInfoDef.szTitle, stAreaInfoDef.szContent);
         com_func_msg_send(emModDATA, COM_DATA_AREAINFO_RECV, 0, 0, sizeof(AreaInfoDef), &stAreaInfoDef);
      }
   }

   iRet = com_json_ack_pack(iSock, "Messsage", "WeatherACK", iPacketNo, emAckSuccess, SD_NULL, SD_NULL);

   return iRet;
}
666666666666666666666666666
SdInt com_json_Unlock_SessionUnlock_unpack(json_object *json, SdInt iSock, enUnlockTypeDef emUnlockType){
   LPUnlockPara pstUnlockPara = SD_NULL;
   json_object *json_tmp = SD_NULL;
   SdInt iPacketNo = SD_NULL;
   SdInt iNum = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d (%s)]:", __FILE__, __LINE__, __FUNCTION__);

   /* 包号 */
   json_tmp = json_object_object_get(json, "PacketNo");
   iPacketNo = json_object_get_int(json_tmp);

   iNum = com_timer_UnlockAck_num_get(emTimerTypeSessionUnlock, iPacketNo);
   if(iNum != 0)
   {
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: Waiting ack.", __FILE__, __LINE__);
      return 0;
   }

   pstUnlockPara = ut_mem_new(UnlockPara, 1);

   pstUnlockPara->iPacketNo = iPacketNo;

   /* 主机会话对方UC */
   json_tmp = json_object_object_get(json, "Operatoruc");
   if(json_tmp == SD_NULL)
      return -1;
   pstUnlockPara->iOperatoruc = json_object_get_int(json_tmp); /* 提取操作UC */

   pstUnlockPara->iSock = iSock;
   pstUnlockPara->emAck = emAckFail;
   pstUnlockPara->emUnlockType = emUnlockType;
   pstUnlockPara->handle = com_timer_set(&com_unlock_Unlock_CallBack, &com_unlock_Unlock_Destructor, pstUnlockPara,
         COM_UNLOCK_TIME, COM_UNLOCK_TIME, 1, emTimerTypeSessionUnlock);
   if(pstUnlockPara->handle == TIMER_NONE) /* 分配定时器失败 */
   {
      com_unlock_Unlock_Destructor(pstUnlockPara); /* 执行析构函数 */
      ut_mem_free(pstUnlockPara); /* 释放资源 */
      return -1;
   }

   com_func_msg_send(emModUI, COM_UI_DOOR_SESSION_OPEN, pstUnlockPara->iOperatoruc, 0, 0, SD_NULL);
   return 0;
}
77777777777777777777777777777777777777777777777
SdInt com_json_Unlock_Remote_unpack(json_object *json, SdInt iSock){
   LPUnlockPara pstUnlockPara = SD_NULL;
   json_object *json_tmp = SD_NULL;
   json_object *EventInfo = SD_NULL;
   SdInt iPacketNo = SD_NULL;
   SdInt iRemoteUnlockUserType = 0;
   RemoteUnlockDef stRemoteUnlockDef;
   SdInt iNum = 0;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d (%s)]:", __FILE__, __LINE__, __FUNCTION__);

   /* 包号 */
   json_tmp = json_object_object_get(json, "PacketNo");
   iPacketNo = json_object_get_int(json_tmp);

   iNum = com_timer_UnlockAck_num_get(emTimerTypeRemoteUnlock, iPacketNo);
   if(iNum != 0)
   {
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: Waiting ack.", __FILE__, __LINE__);
      return 0;
   }
   pstUnlockPara = ut_mem_new(UnlockPara, 1);
   if(pstUnlockPara == NULL)
      return -1;

   pstUnlockPara->iPacketNo = iPacketNo;

   EventInfo = json_object_object_get(json, "EventInfo");
   /* 远程开锁对方UC */
   json_tmp = json_object_object_get(EventInfo, "Operatoruc");
   if(json_tmp == SD_NULL)
      return -1;
   pstUnlockPara->iOperatoruc = json_object_get_int(json_tmp);
   stRemoteUnlockDef.iOperatoruc = pstUnlockPara->iOperatoruc;

   /* 远程开锁对方类型 */
   json_tmp = json_object_object_get(EventInfo, "Operatortype");
   if(json_tmp == SD_NULL)
      return -1;
   iRemoteUnlockUserType = json_object_get_int(json_tmp);

   switch(iRemoteUnlockUserType)
   {   
      case 1: /* 物理管理中心开锁 */
         pstUnlockPara->emUnlockType = emPhyManageUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockPhyManage;
      break;
      case 2: /* 网管开锁 */
         pstUnlockPara->emUnlockType = emNetAdminUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockNetAdmin;
      break;
      case 3: /* 第三方开锁 */
         pstUnlockPara->emUnlockType = emThirdUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockThird;
      break;
      default:
         pstUnlockPara->emUnlockType = emUnknownUnlock;
         stRemoteUnlockDef.emRemoteUnlockUserTypeDef = emRemoteUnlockUnknow;
      break;
   }

   pstUnlockPara->iSock = iSock;
   pstUnlockPara->emAck = emAckFail;

   pstUnlockPara->handle = com_timer_set(&com_unlock_Unlock_CallBack, &com_unlock_Unlock_Destructor, pstUnlockPara,
      COM_UNLOCK_TIME, COM_UNLOCK_TIME, 1, emTimerTypeRemoteUnlock);
   if(pstUnlockPara->handle == TIMER_NONE) /* 分配定时器失败 */
   {
      com_unlock_Unlock_Destructor(pstUnlockPara); /* 执行析构函数 */
      ut_mem_free(pstUnlockPara); /* 释放资源 */
      return -1;
   }

   com_func_msg_send(emModUI, COM_UI_DOOR_REMOTE_OPEN, 0, 0, sizeof(RemoteUnlockDef), &stRemoteUnlockDef);

   return 0;
}

SdInt com_json_Access_Data_unpack(json_object *json, SdInt iSock){
   SdChar szBin[2048];
   json_object *json_tmp = SD_NULL;
   SdInt iRet = 0;
   SdInt iPacketNo = SD_NULL;
   SdChar *pszBase64 = SD_NULL;
   SdInt iLenBase64 = 0;
   SdInt iLenBin = 0, ii = 0;

   /* 包号 */
   json_tmp = json_object_object_get(json, "PacketNo");
   iPacketNo = json_object_get_int(json_tmp);

   json_tmp = json_object_object_get(json, "BIN");
   pszBase64 = (SdChar *)json_object_get_string(json_tmp);
   if(pszBase64 == SD_NULL)
      return -1;

   iLenBase64 = strlen(pszBase64);
   if((iLenBase64 / 4 * 3) > sizeof(szBin))
      return -1;
   iLenBin = com_func_base64_decode(pszBase64, szBin);

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: Access_Data: iLenBase64 = (%d) is (%s), iLenBin = (%d)", __FILE__, __LINE__, iLenBase64, pszBase64, iLenBin);

   iRet = com_json_ack_pack(iSock, "Access", "DataACK", iPacketNo, emAckSuccess, SD_NULL, SD_NULL);
   com_func_msg_send(emModHW, COM_HW_AM_DATA_DOWN, 0, 0, iLenBin, szBin);
   return iRet;
}

SdInt com_json_unpack(SdInt iSock, IN SdChar *pszBuf, SdInt iLen, void *ptr){
   json_object *json = SD_NULL;
   json_object *json_tmp = SD_NULL;
   SdChar *pszBizType = SD_NULL;
   SdChar *pszSubMsgType = SD_NULL;

   if(iLen == 0)
      return -1;

   json = json_tokener_parse(pszBuf);
   if(json == SD_NULL)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s %d]: json_tokener_parse FAIL", __FILE__, __LINE__);
      return -1;
   }

   json_tmp = json_object_object_get(json, "BizType");
   pszBizType = (SdChar *)json_object_get_string(json_tmp);
   if(pszBizType == SD_NULL)
      return -1;

   json_tmp = json_object_object_get(json, "SubMsgType");
   pszSubMsgType = (SdChar *)json_object_get_string(json_tmp);
   if(pszSubMsgType == SD_NULL)
      return -1;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d (%s)]: BizType -> [%s]", __FILE__, __LINE__, __FUNCTION__, pszBizType);
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d (%s)]: SubMsgType -> [%s]", __FILE__, __LINE__, __FUNCTION__, pszSubMsgType);

   if(!strcmp(pszBizType, "Message"))
   {
      if(!strcmp(pszSubMsgType, "NoteRelease"))
      {
         com_json_Message_NoteRelease_unpack(json, iSock);
      }
      else if(!strcmp(pszSubMsgType, "NoteDel"))
      {
         com_json_Message_NoteDel_unpack(json, iSock);
      }else if(!strcmp(pszSubMsgType, "Weather"))
      {
         com_json_Message_Weather_unpack(json, iSock);
      }
   }else 
   if(!strcmp(pszBizType, "Unlock"))
   {
      if(!strcmp(pszSubMsgType, "TalkingUnlock"))
      {
         com_json_Unlock_SessionUnlock_unpack(json, iSock, emTalkingUnlock);
      }else
      if(!strcmp(pszSubMsgType, "WatchingUnlock"))
      {
         com_json_Unlock_SessionUnlock_unpack(json, iSock, emWatchingUnlock);
      }else
      if(!strcmp(pszSubMsgType, "Remote"))
      {
         com_json_Unlock_Remote_unpack(json, iSock);
      }
   }else 
   if(!strcmp(pszBizType, "Access"))
   {
      if(!strcmp(pszSubMsgType, "Data"))
      {
         com_json_Access_Data_unpack(json, iSock);
      }
   }

   json_object_put(json_tmp);
   json_object_put(json); /* 释放资源, 不知道对不对? */

   return 0;
}

#ifdef EEEEEEEEEEnd_End_Unpack /* UnPack */
#endif


#if (0)
void com_json_Unlock_RemotePasswordUnlock_callback(IN json_object *json, OUT SdChar **ppszBuf, OUT SdInt *piLen,void *ptr)
{
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPRemoteOpenDoorDef pstRemoteOpenDoorDef = (LPRemoteOpenDoorDef)ptr;
   LPCOMCfgDef pstCOMCfgDef = SD_NULL;
   json_object *DeviceInfo = SD_NULL;

   if(pstCOMManage == SD_NULL)
      return ;
   pstCOMCfgDef = &pstCOMManage->stCOMCfg;

   DeviceInfo = json_object_new_object();
   if(DeviceInfo == SD_NULL)
      return ;

   json_object_object_add(DeviceInfo , "Dooruc", json_object_new_int(pstCOMManage->stCOMCfg.uiUcCode));
   pstRemoteOpenDoorDef->stOpenDoorParam.szPassWd[MAX_PASSWORD_LENGTH - 1] = '\0';
   json_object_object_add(DeviceInfo , "Password", json_object_new_string(pstRemoteOpenDoorDef->stOpenDoorParam.szPassWd));
   json_object_object_add(json, "DeviceInfo", DeviceInfo); 

   /* Json对象转换成, String */
   {
      SdChar *pszTmpBuf = SD_NULL;
      SdInt iHeaderLen = sizeof(ProtocolHeader); /* 协议头 */
      pszTmpBuf = (SdChar *)json_object_to_json_string(json); /* 转换成字符串 */
      *piLen = strlen(pszTmpBuf) + 1 + iHeaderLen;
      *ppszBuf = ut_mem_new(SdChar, *piLen); /* 分配资源, 存储数据 */
      memcpy(*ppszBuf + iHeaderLen,  pszTmpBuf, (*piLen - iHeaderLen));
   }

   /* 释放资源 */
   json_object_put(DeviceInfo); /* 释放资源 */
   return ;
}

SdInt com_json_Unlock_RemotePasswordUnlock_pack(void *ptr)
{
   SdInt iRet = 0;
   LPRemoteOpenDoorDef pstRemoteOpenDoorDef = (LPRemoteOpenDoorDef)ptr;
   if(pstRemoteOpenDoorDef == SD_NULL)
      return -1;

   iRet =  com_json_SERVERterm_pack(pstRemoteOpenDoorDef->stRemoteInfo.szRemoteIP, pstRemoteOpenDoorDef->stRemoteInfo.usPort,
      "Unlock", "RemotePasswordUnlock", com_json_Unlock_RemotePasswordUnlock_callback, ptr);

   return iRet;
}
#endif
#if (0)

SdInt com_json_Alarm_Upload_pack(void *ptr)
{
   SdInt iRet = 0;
   iRet =  com_json_SERVERnetadmin_pack("Alarm", "Upload", com_json_Alarm_Upload_callback, ptr);
   return iRet;
}
void com_json_Alarm_Upload_callback(IN json_object *json, OUT SdChar **ppszBuf, OUT SdInt *piLen,void *ptr){
   LPCOMManage pstCOMManage = com_ctrl_commanage_get();
   LPCOMCfgDef pstCOMCfgDef = SD_NULL;
   SdChar szObj[96] = {0};
   SdInt iLen = 0;
   json_object *tech = SD_NULL;
   json_object *App = SD_NULL;
   json_object *DefenceArea = SD_NULL;
   json_object *DeviceInfo = SD_NULL;
   json_object *arry = SD_NULL;
   struct timeval tv;

   if(pstCOMManage == SD_NULL)
      return ;

   tech = json_object_new_array();
   if(tech == SD_NULL)
      return;
   App = json_object_new_array();
   if(App == SD_NULL)
   {
      json_object_put(tech);
      return;
   }
   DefenceArea = json_object_new_array();
   if(DefenceArea == SD_NULL)
   {
      json_object_put(tech);
      json_object_put(App);
       return;
   }
   DeviceInfo = json_object_new_object();
   if(DeviceInfo == SD_NULL)
   {
      json_object_put(tech);
      json_object_put(App);
      json_object_put(DefenceArea);
      return;
   }
   arry = json_object_new_object();
   if(arry == SD_NULL)
   {
      json_object_put(tech);
      json_object_put(App);
      json_object_put(DefenceArea);
      json_object_put(DeviceInfo);
      return;
   }

   pstCOMCfgDef = &pstCOMManage->stCOMCfg;
   struct timeval tv;

   gettimeofday(&tv, NULL); /* 时间 */
   iLen = sprintf(szObj, "%ld", tv.tv_sec * 1000 + tv.tv_usec / 1000);
   json_object_object_add(json, "Now", json_object_new_string(szObj));

   json_object_object_add(json, "DeviceInfo", DeviceInfo);
   json_object_object_add(json, "DefenceArea", DefenceArea);
   json_object_array_add(DefenceArea, arry);
   json_object_object_add(DeviceInfo, "uc", json_object_new_int(pstCOMCfgDef->uiUcCode)); 
   json_object_object_add(DeviceInfo, "fc", json_object_new_int(pstCOMCfgDef->uiFcCode));
   json_object_object_add(DeviceInfo, "dd", json_object_new_string(pstCOMCfgDef->szdd));
   json_object_object_add(DeviceInfo, "bbb", json_object_new_string(pstCOMCfgDef->szbbb));
   json_object_object_add(DeviceInfo, "rr", json_object_new_string(pstCOMCfgDef->szrr));
   json_object_object_add(DeviceInfo, "ff", json_object_new_string(pstCOMCfgDef->szff));
   json_object_object_add(DeviceInfo, "ii", json_object_new_string(pstCOMCfgDef->szii));
   json_object_object_add(DeviceInfo, "App", App);

   /* 如果是分机, 此处应加APP字段 */
/*   if(strlen(pstCOMCfgDef->szApp_0) && strlen(pstCOMCfgDef->szApp_1)){
      json_object_array_add(App, json_object_new_string(pstCOMCfgDef->szApp_0));
      json_object_array_add(App, json_object_new_string(pstCOMCfgDef->szApp_1));
   }*/

   json_object_object_add(arry, "SrcSn", json_object_new_int(1));
   json_object_object_add(arry, "AreaNum", json_object_new_int(10));

   json_object_object_add(arry, "Descr", json_object_new_string("鎶ヨ娴嬭瘯")); /* 我在报警 */
   json_object_object_add(arry, "State", json_object_new_int(2));
   json_object_object_add(arry, "Type", json_object_new_int(2)); /* =1, 24小时防区; =2  瞬时防区, =3  延时防区 */
   json_object_object_add(arry, "Delay", json_object_new_int(40000));

   /* Json对象转换成, String */
   {
      SdChar *pszTmpBuf = SD_NULL;
      SdInt iHeaderLen = sizeof(ProtocolHeader); /* 协议头 */
      pszTmpBuf = (SdChar *)json_object_to_json_string(json); /* 转换成字符串 */
      *piLen = strlen(pszTmpBuf) + 1 + iHeaderLen;
      *ppszBuf = ut_mem_new(SdChar, *piLen); /* 分配资源, 存储数据 */
      memcpy(*ppszBuf + iHeaderLen,  pszTmpBuf, (*piLen - iHeaderLen));
   }

   /* 释放资源 */
   json_object_put(tech);
   json_object_put(DefenceArea);
   json_object_put(App);
   json_object_put(arry);
   json_object_put(DeviceInfo);

   return ;
}
#endif

#if (0)
/* BizType = "Alarm", SubMsgType = "Setting", 报警->防区设置 */
SdInt com_json_Alarm_Setting_pack(IN LPCOMManage pstCOMManage){
   json_object *json = SD_NULL;
   NetMsg stNetMsg;
   SdChar *pSendBuf = SD_NULL;
   SdInt iSendLen = 0;
   SdInt iHeadLen = sizeof(ProtocolHeader);
   SdInt iRet = 0;

   json = json_object_new_object();
   if(json == SD_NULL)
      return -1;

   /* Start加入发送数据 */
   json_object_object_add(json, "BizType", json_object_new_string("Alarm"));
   json_object_object_add(json, "SubMsgType", json_object_new_string("Setting"));
   json_object_object_add(json, "PacketNo", json_object_new_int(pstCOMManage->uiPackId)); 

   {
      SdChar szObj[8] = {0};
      json_object *tech = json_object_new_array();
      json_object *DefenceArea = json_object_new_array();

      SdInt iLen = 0;

      json_object *arry0 = json_object_new_object();
      json_object *arry1 = json_object_new_object();

      json_object *DeviceInfo  = json_object_new_object();

      json_object_object_add(json, "DeviceInfo", DeviceInfo);
      json_object_object_add(json, "DefenceArea", DefenceArea);
      json_object_array_add(DefenceArea, arry0);
      json_object_array_add(DefenceArea, arry1);

      iLen = sprintf(szObj, "%d", 37); szObj[iLen] = '\0';
      json_object_object_add(DeviceInfo, "uc", json_object_new_string(szObj)); 
      iLen = sprintf(szObj, "%d", 155); szObj[iLen] = '\0';
      json_object_object_add(DeviceInfo, "fc",   json_object_new_string(szObj));
      iLen = sprintf(szObj, "%07d", 0); szObj[iLen] = '\0';
      json_object_object_add(DeviceInfo, "dd",  json_object_new_string(szObj));
      iLen = sprintf(szObj, "%07d", 0); szObj[iLen] = '\0';
      json_object_object_add(DeviceInfo, "bbb", json_object_new_string(szObj));
      iLen = sprintf(szObj, "%07d", 1); szObj[iLen] = '\0';
      json_object_object_add(DeviceInfo, "rr",  json_object_new_string(szObj));
      iLen = sprintf(szObj, "%04d", 0); szObj[iLen] = '\0';
      json_object_object_add(DeviceInfo, "ff",  json_object_new_string(szObj));
      iLen = sprintf(szObj, "%04d", 0); szObj[iLen] = '\0';
      json_object_object_add(DeviceInfo, "ii",  json_object_new_string(szObj));

      json_object_object_add(arry0, "AreaNum",   json_object_new_int(1));
      json_object_object_add(arry0, "Descr",   json_object_new_string("红外探测"));
      json_object_object_add(arry0, "State",   json_object_new_int(0));

      json_object_object_add(arry1, "AreaNum",   json_object_new_int(2));
      json_object_object_add(arry1, "Descr",   json_object_new_string("烟感探测"));
      json_object_object_add(arry1, "State",   json_object_new_int(1));

   }
   /* End加入发送数据 */

   stNetMsg.pszBuf = (SdChar *)json_object_to_json_string(json);
   iSendLen = strlen(stNetMsg.pszBuf);
   stNetMsg.stProtocolHeader.iMagic = iHeaderMagic;
   stNetMsg.stProtocolHeader.iLen = iSendLen;
   com_func_protocol_header_conver(&stNetMsg, &stNetMsg); /* 协议头字节序列转换 */
/* start 测试加入 */
//   stNetMsg.stProtocolHeader.iLen = 0x30303030;
/* end 测试加入 */
   pSendBuf = ut_mem_malloc(iHeadLen + iSendLen);
   if(pSendBuf == SD_NULL){
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s %d]: malloc Fail!!", __FILE__, __LINE__);
      return -1;
   }
   memcpy(pSendBuf, &stNetMsg, sizeof(ProtocolHeader));
   memcpy(pSendBuf + iHeadLen, stNetMsg.pszBuf, iSendLen);
   iSendLen += iHeadLen;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s %d]: (LEN = %d) - [%s]", __FILE__, __LINE__, iSendLen - iHeadLen, pSendBuf);
   iRet = com_client_SERVERnetadmin_sendto(pSendBuf, iSendLen, pstCOMManage) ;
   ut_mem_free(pSendBuf);
   json_object_put(json);  

   return iRet;
}
#endif
#if (0)

#endif
#endif
