/**
 * \file    com_client.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning 版权所有, 侵权必究
 * \todo    TCP客户端
 */
#ifndef __COM_CLIENT_H_
#define __COM_CLIENT_H_

#include "com_type.h"

/************************************************************************
* @FunctionName( 函数名 ): s32 com_client_net_msg_del_by_pack_id(const u32 cpack_id, TIMER_HANDLE *ptimer_handle)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 通过包号删除网络消息
*
* @Param(参数):
*        pack_id - 包号
*       *timer_handle - 返回定时器句柄
*
* @ReturnCode(返回值):
*        0 - 成功
*       -1 - 失败(pack_id不存在)
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_client_net_msg_del_by_pack_id(const u32 cpack_id, TIMER_HANDLE *ptimer_handle);
/**@END! s32 com_client_net_msg_del_by_pack_id(const u32 cpack_id, TIMER_HANDLE *ptimer_handle) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_client_tcp_server_sendto(s8 *pip, u16 port, const u32 pack_id, s8 *pbuf, u32 len)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 发送网络消息到信息服务器
*
* @Param(参数):
*       *pip - TCP服务器IP
*        port - TCP服务器端口
*        pack_id - 包号(非常重要)
*        pbuf - 数据缓冲区(必须由malloc分配)
*        len - 数据长度
*
* @ReturnCode(返回值):
*        0 - 成功
*       -1 - 失败(释放数据缓冲区资源)
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_client_tcp_server_sendto(s8 *pip, u16 port, const u32 pack_id, s8 *pbuf, u32 len);
/**@END! s32 com_client_tcp_server_sendto(s8 *pip, u16 port, const u32 pack_id, s8 *pbuf, u32 len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_client_msg_server_sendto(const u32 pack_id, s8 *pbuf, u32 len)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 发送网络消息到信息服务器
*
* @Param(参数):
*        pack_id - 包号(非常重要)
*        pbuf - 数据缓冲区(必须由malloc分配)
*        len - 数据长度
*
* @ReturnCode(返回值):
*        0 - 成功
*       -1 - 失败(释放数据缓冲区资源)
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_client_msg_server_sendto(const u32 pack_id, s8 *pbuf, u32 len);
/**@END! s32 com_client_msg_server_sendto(const u32 pack_id, s8 *pbuf, u32 len) !\(^o^)/~ 结束咯 */

/************************************************************************
*人脸识别发送往服务器函数
************************************************************************/

s32 com_client_msg_face_server_sendto(const u32 pack_id, s8 *pbuf, u32 len);


/************************************************************************
*指纹识别发送往服务器函数
************************************************************************/
s32 com_client_msg_finger_server_sendto(const u32 pack_id, s8 *pbuf, u32 len);



/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_client_init(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_client_init(void *parg);
/**@END! s32 com_client_init(void *arg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_client_start(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 启动
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_client_start(void *parg);
/**@END! int com_client_start(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_client_stop(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 停止
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_client_stop(void *parg);
/**@END! s32 com_client_stop(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_client_uninit(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 反初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_client_uninit(void *parg);
/**@END! static s32 com_client_uninit(void *arg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): int com_client_cache_am_data(const u32 pack_id, s8 *pbuf, u32 len)
* @Description(描述):      将未发送的门禁数据缓存起来
* @Param(参数): 
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
int com_client_cache_am_data(const u32 pack_id, s8 *pbuf, u32 len);

/************************************************************************
* @FunctionName( 函数名 ): int com_client_cache_am_data(const u32 pack_id, s8 *pbuf, u32 len)
* @Description(描述):      将未发送的门禁数据缓存起来
* @Param(参数): 
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
s32 com_remove_am_cache_data_to_db(u32 pack_id);

/************************************************************************
* @FunctionName( 函数名 ): int com_send_am_data_from_am_db_cache_timer()
* @Description(描述):      将未发送的门禁数据缓存起来
* @Param(参数): 
* @ReturnCode(返回值):     -1 - 失败, >0 - 网络消息长度
************************************************************************/
void* com_send_am_data_from_am_db_cache_timer();



#endif /* __COM_CLIENT_H_ */
