/**
 * \file    com_json.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning 版权所有, 侵权必究
 * \todo    COM模块JSON拆包, 打包
 */
#ifndef __COM_JSON_H_
#define __COM_JSON_H_

#include "json/json.h"
#include "com_type.h"



/************************************************************************
* enum  em_json_ack
*        JSON包ACK
************************************************************************/
typedef enum enum_json_ack{
    ack_false_em = 0x00,/* 失败 */
    ack_true_em,        /* 成功 */
    ack_no_em,          /* 无ACK */
}em_json_ack;

typedef s32(*com_json_callback)(json_object *, void *, va_list); /* JSON打包回调函数类型 */

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
s32 com_json_unpack(const s32 sock, const s8 *pbuf_s8, const u32 len);
/**@END! s32 com_json_unpack(const s32 sock, const s8 *pbuf_s8, const u32 len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Message_NoteBrowse(void *ptr, int len)
* @Description(描述):      JSON打包及发送网络消息: 信息浏览
* @Param(参数): LPReadAreaInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_Message_NoteBrowse(void *ptr, int len);
/**@END! s32 com_json_Message_NoteBrowse(void *ptr, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_VDP_UnlockEvent(void *ptr, int len)
* @Description(描述):      JSON打包及发送网络消息: 开锁事件
* @Param(参数): LPDoorRecordDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_VDP_UnlockEvent(void *ptr, int len);
/**@END! s32 com_json_VDP_UnlockEvent(void *ptr, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_VDP_CallInfo(void *ptr, int len)
* @Description(描述):      JSON打包及发送网络消息: 呼叫事件
* @Param(参数): LPCallEventReportDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_VDP_CallInfo(void *ptr, int len);
/**@END! s32 com_json_VDP_CallInfo(void *ptr, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_ftp_ImgInfo(void *ptr, int len)
* @Description(描述):      JSON打包及发送ftp消息: 图片消息
* @Param(参数): LPCallEventReportDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_ftp_ImgInfo(void *ptr, int len);
/**@END! s32 com_json_VDP_CallInfo(void *ptr, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Access_Data(char *pdata, int len)
* @Description(描述):      JSON打包及发送网络消息: 门禁数据
* @Param(参数): 数据(转Base64编码), 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_Access_Data(char *pdata, int len);
/**@END! s32 com_json_Access_Data(char *pdata, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Lift_Schedule(char *pdata, int len)
* @Description(描述):      JSON打包及发送网络消息: 调度电梯
* @Param(参数): LPRoomAddrDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_Lift_Schedule(char *pdata, int len);
/**@END! s32 com_json_Lift_Schedule(char *pdata, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_Unlock_CheckPassword(char *pdata, int len)
* @Description(描述):      JSON打包及发送网络消息: 远端开锁, 校验密码
* @Param(参数): LPRemotePasswdUnlockDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_Unlock_CheckPassword(char *pdata, int len);
/**@END! s32 com_json_Unlock_CheckPassword(char *pdata, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_door_Alarm_Upload(char *pdata, int len)
* @Description(描述):      JSON打包及发送网络消息: 远端开锁, 校验密码
* @Param(参数): LPEPAlarmInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_door_Alarm_Upload(char *pdata, int len);
/**@END! s32 com_json_door_Alarm_Upload(char *pdata, int len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_json_door_Alarm_Upload(char *pdata, int len)
* @Description(描述):      JSON打包及发送网络消息: 远端开锁, 校验密码
* @Param(参数): LPEPAlarmInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_json_door_Alarm_Upload(char *pdata, int len);

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Face_RequestFaceSync(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:请求人脸数据同步
* @Param(参数): LPRequestInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_RequestFaceSync(SdChar *pdata, SdInt len);

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Face_FaceSyncCheck(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:人脸数据同步检测
* @Param(参数): LPEPAlarmInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_FaceSyncCheck(SdChar *pdata, SdInt len);

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Face_FaceSyncAck(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:人脸数据同步Ack
* @Param(参数): LPEPAlarmInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Face_FaceSyncAck(SdBool Result, const SdInt iSock);


/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Finger_Char(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:指纹特征值发送
* @Param(参数): LPEPAlarmInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Char(SdChar *pdata, SdInt len);

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Finger_Sync_Request(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:指纹数据同步请求
* @Param(参数): LPEPAlarmInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Sync_Request(SdChar *pdata, SdInt len);

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Finger_Sync_Check(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:指纹数据同步检测
* @Param(参数): LPEPAlarmInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Sync_Check(SdChar *pdata, SdInt len);

/************************************************************************
* @FunctionName( 函数名 ): SdInt com_json_Finger_Finger_Sync(SdChar *pdata, SdInt len)
* @Description(描述):      JSON打包及发送网络消息:指纹同步命令
* @Param(参数): LPEPAlarmInfoDef, 数据长度
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
SdInt com_json_Finger_Finger_Sync(SdChar *pdata, SdInt len);





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
s32 com_json_init(void *parg);
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
s32 com_json_start(void *parg);
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
s32 com_json_stop(void *parg);
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
s32 com_json_uninit(void *parg);
/**@END! s32 com_json_uninit(void *parg) !\(^o^)/~ 结束咯 */

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
s32 com_json_to_net_msg(json_object *json, s8 **ppbuf);

s32 com_json_liftstatus_info(LPLiftInfoDef ps_info, s8 ** jsonstr);

/************************************************************************
* @FunctionName( 函数名 ): int com_char_to_net_msg(char *pbuf, char **ppbuf)
* @Description(描述):      将未发送的门禁数据缓存起来
* @Param(参数): 
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
int com_char_to_net_msg(char *pbuf, char **ppbuf);


#endif /* __COM_JSON_H_ */
