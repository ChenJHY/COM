/**
 * \file    com_func.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning 版权所有, 侵权必究
 * \todo    COM功能函数接口
 */
#ifndef __COM_FUNC_H_
#define __COM_FUNC_H_

#include <sd_macro.h>
#include <sd_message.h>
#include <sd_struct.h>
#include <sqlite3.h>
#include <ut/ut_list.h>
#include <ut/ut_module_timer.h>

#include "com_type.h"
#include "com_json.h"
#include "com_timer.h"
#include "com_ctrl.h"
#include "com_client.h"
#include "com_server.h"
#include "com_ftp.h"
#include "com_broadcast.h"

/************************************************************************
* struct  db_arg
*         门禁数据库结构体
************************************************************************/
struct db_arg 
{
	char *str;
	int len;
};

/************************************************************************
* struct  st_net_msg_header
*        网络消息头
************************************************************************/
typedef struct struct_net_msg_header{
    u32 maigc; /* 0xAABBCCDD */
    u32 len;   /* 数据长度 */
}st_net_msg_header, *pst_net_msg_header;

/************************************************************************
* struct  st_node
*        链表节点
************************************************************************/
typedef struct struct_node{
    void  *data; /* 数据元素 */
    struct struct_node *pnext; /* 后继节点 */
}st_node, *pst_node;

/************************************************************************
* struct  st_list
*        链表
************************************************************************/
typedef struct struct_list{
    st_node         head; /* 链表头 */
    pthread_mutex_t lock; /* 链表锁 */
}st_list, *pst_list;

/************************************************************************
* enum  em_list_operate
*        链表操作
************************************************************************/
typedef enum enum_list_operate{
    operate_remove_em = 0x01, /* 删除元素   */
    operate_next_em = 0x02,   /* 下一个元素 */
    operate_break_em = 0x04,  /* 中断遍历   */
}em_list_operate;

typedef em_list_operate(*com_func_list_callback)(void *, void *, va_list); /* 链表迭代回调函数类型 */

/**
 * \fn      static SdInt com_func_msg_send(...)
 * \brief   发送消息到其他模块
 * \param   ...
 * \return  消息发送状态
 * \note
 * \todo
 * \version V1.0
 * \warning 必须保证参数的合法性
 */
SdInt com_func_msg_send(SdUShort usDstModule, SdUShort usMsgID,
    SdULong ulParam1, SdULong ulParam2, SdULong ulContentLength, IN void* pContent);

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_sock_close(s32 sock)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 关闭SOCK
*
* @Param(参数): sock - SOCK句柄
*
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_sock_close(s32 sock);
/**@END! s32 com_func_sock_close(s32 sock) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_sock_nonblock(s32 sock)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 设置SOCK非阻塞
*
* @Param(参数): sock - SOCK句柄
*
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_sock_nonblock(s32 sock);
/**@END! s32 com_func_sock_nonblock(s32 sock) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_block_timeout(s32 sock, s32 opt, s32 second_u32)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 设置SOCK阻塞超时
*
* @Param(参数):
*        sock - SOCK句柄
*        opt - 选项(SO_RCVTIMEO, SO_SNDTIMEO, ..)
*        second_u32 - 时间(单位: second)
*
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_block_timeout(s32 sock, s32 opt, s32 second_u32);
/**@END! s32 com_func_block_timeout(s32 sock, s32 opt, s32 second_u32) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_tcp_server_create(u16 listen_port)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 设置SOCK阻塞超时
*
* @Param(参数): listen_port - 监听端口
*
* @ReturnCode(返回值): >0 - SOCK, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_tcp_server_create(u16 listen_port);
/**@END! s32 com_func_tcp_server_create(u16 listen_port) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_tcp_server_connect(const s8 *ip, const u16 port, s32 recv_time)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 连接TCP服务器
*
* @Param(参数):
*       *ip:port - IP:端口
*        recv_time - 连接成功, SOCK阻塞接收超时时间(单位: second, 最大 120, <=0 非阻塞)
*
* @ReturnCode(返回值): >0 - SOCK, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_tcp_server_connect(const s8 *ip, const u16 port, s32 recv_time);
/**@END! s32 com_func_tcp_server_connect(const s8 *ip, const u16 port, s32 recv_time) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_tcp_server_reconnect(const s8 *ip, const u16 port, s32 *psock, s32 recv_time)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 重新连接TCP服务器
*
* @Param(参数):
*       *ip:port - IP:PORT
*       *psock - 输入已损坏SOCK, 输出新SOCK
*        recv_time - 连接成功, SOCK阻塞接收超时时间(单位: second, 最大: 120, <=0 非阻塞)
*
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_tcp_server_reconnect(const s8 *ip, const u16 port, s32 *psock, s32 recv_time);
/**@END! s32 com_func_tcp_server_reconnect(const s8 *ip, const u16 port, s32 *psock, s32 recv_time) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): void com_func_header_conver(pst_net_msg_header pheader_st, const s8 *pbuf)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 转换网络消息头
*
* @Param(参数): pheader_st - 网络消息头, *pbuf -
*
* @ReturnCode(返回值): None(无)
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
void com_func_header_conver(pst_net_msg_header pheader_st, const s8 *pbuf);
/**@END! void com_func_header_conver(pst_net_msg_header pheader_st, const s8 *pbuf) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): void com_func_net_msg_header_pack(s8 *pbuf, const u32 len)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 添加网络消息头
*
* @Param(参数): *pbuf - 缓冲区, len - 缓冲区长度
*
* @ReturnCode(返回值): None(无)
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
void com_func_net_msg_header_pack(s8 *pbuf, const u32 len);
/**@END! void com_func_net_msg_header_pack(s8 *pbuf, const u32 len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_tcp_data_send(const s32 sock, const s8 *cpbuf_s8, const u32 len)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 发送TCP数据
*
* @Param(参数):
*        sock - SOCK
*       *cpbuf_s8 - 数据缓冲区
*        len - 发送长度
*
* @ReturnCode(返回值):
*        0 - 成功
*       -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_tcp_data_send(const s32 sock, const s8 *cpbuf_s8, const u32 len);
/**@END! s32 com_func_tcp_data_send(const s32 sock, const s8 *cpbuf_s8, const u32 len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_tcp_msg_recv(const s32 sock, s8 **pprecv_s8, u32 *plen, s32 timeout)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 接收TCP网络消息
*
* @Param(参数):
*        sock - SOCK
*      **pprecv_s8 - 返回接收数据(malloc分配)
*       *plen - 返回网络消息长度
*        timeout - 接收超时时间(单位: second, 最大: 120秒)
*
* @ReturnCode(返回值):
*       -2 - 未知错误
*       -1 - SOCK关闭
*        0 - 成功
*        1 - 超时
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_tcp_msg_recv(const s32 sock, s8 **pprecv_s8, u32 *plen, s32 timeout);
/**@END! s32 com_func_tcp_msg_recv(const s32 sock, s8 **pprecv_s8, u32 *plen, s32 timeout) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_list_init(pst_list plist_st)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 链表初始化
*
* @Param(参数):
*        plist_st - 链表
*
* @ReturnCode(返回值):
*        0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_list_init(pst_list plist_st);
/**@END! s32 com_func_list_init(pst_list *pplist_st) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_list_destory(pst_list plist_st)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 链表销毁
*
* @Param(参数):
*        plist_st - 链表
*
* @ReturnCode(返回值):
*        0
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_list_destory(pst_list plist_st);
/**@END! s32 com_func_list_destory(pst_list *pplist_st) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_list_elem_insert(pst_list plist_st, void *elem)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 链表插入元素
*
* @Param(参数):
*        plist_st - 链表
*       *elem - 元素
*
* @ReturnCode(返回值):
*        0 - 成功
*       -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_list_elem_insert(pst_list plist_st, void *elem);
/**@END! s32 com_func_list_elem_insert(pst_list plist_st, void *elem) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_list_iterate(pst_list plist_st, com_func_list_callback callback, void *parg, ...)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 链表迭代
*
* @Param(参数):
*        plist_st - 链表
*        callback - 迭代回调函数
*       *parg,... - 变参列表
*
* @ReturnCode(返回值):
*        0 - 成功
*       -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_list_iterate(pst_list plist_st, com_func_list_callback callback, void *parg, ...);
/**@END! s32 com_func_list_iterate(pst_list plist_st, com_func_list_callback callback, void *parg, ...) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_pathfile_path(const s8 *cpathfile, s8 *path, s32 buf_len)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): /path/file.type返回/path
*
* @Param(参数):
*       *cpathfile - /路径/文件.类型
*       *cpath - /路径
*        buf_len - *path长度
*
* @ReturnCode(返回值): -1 - 失败, 0 - 成功
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_pathfile_path(const s8 *cpathfile, s8 *path, s32 buf_len);
/**@END! s32 com_func_pathfile_path(const s8 *cpathfile, s8 *path, s32 buf_len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_pathfile_file(const s8 *cpathfile, s8 *pfile, s32 buf_len)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): /path/file.type返回file.type
*
* @Param(参数):
*       *cpathfile - /路径/文件.类型
*       *pfile - 文件.类型
*        buf_len - *pfile长度
*
* @ReturnCode(返回值): -1 - 失败, 0 - 成功
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_pathfile_file(const s8 *cpathfile, s8 *pfile, s32 buf_len);
/**@END! s32 com_func_pathfile_file(const s8 *cpathfile, s8 *pfile, s32 buf_len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_file_type(const s8 *cpathfile, s8 *ptype, s32 buf_len)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): file.type返回type
*
* @Param(参数):
*       *cpathfile - /路径/文件.类型
*       *ptype - 类型
*        buf_len - *ptype长度
*
* @ReturnCode(返回值): -1 - 失败, 0 - 成功
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_file_type(const s8 *cpathfile, s8 *ptype, s32 buf_len);
/**@END! s32 com_func_file_type(const s8 *cpathfile, s8 *ptype, s32 buf_len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_cmd_read(const char *cpcmd, s8 *pbuf, s32 buf_len)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 命令行输出重定向
*
* @Param(参数):
*       *cpcmd - 命令
*       *pbuf - 缓冲区
*        buf_len - *pbuf长度
*
* @ReturnCode(返回值): -1 - 失败, 0 - 成功
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_cmd_read(const char *cpcmd, s8 *pbuf, s32 buf_len);
/**@END! s32 com_func_cmd_read(const char *cpcmd, s8 *pbuf, s32 buf_len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_file_size(const s8 *cpfile)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 返回文件尺寸
*
* @Param(参数): *cpfile - 源文件
*
* @ReturnCode(返回值): -1 - 失败, 其他 - 文件字节数
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_file_size(const s8 *cpfile);
/**@END! s32 com_func_file_size(const s8 *cpfile) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_file_line(const s8 *cpfile)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 返回文件行数
*
* @Param(参数): *cpfile - 源文件
*
* @ReturnCode(返回值): -1 - 失败, 其他 - 行数
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_file_line(const s8 *cpfile);
/**@END! s32 com_func_file_line(const s8 *cpfile) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_attachment_savepath(enDataTypeDef emDataTypeDef, const s8 *cpathfile, char *psavepath)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 小区信息附件保存路径
*
* @Param(参数):
*        emDataTypeDef - 小区信息类型
*       *cpathfile - /路径/文件.类型
*        psavepath - 附件路径
*
* @ReturnCode(返回值): -1 - 失败, 其他 - 行数
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_attachment_savepath(enDataTypeDef emDataTypeDef, const s8 *cpathfile, char *psavepath);
/**@END! s32 com_func_attachment_savepath(enDataTypeDef emDataTypeDef, const s8 *cpathfile, char *psavepath) !\(^o^)/~ 结束咯 */


SdChar *com_func_base64_encode(const SdUChar *bindata, SdChar *base64, SdInt binlength);
SdInt com_func_base64_decode(const SdChar *base64, SdChar *bindata);

/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_init(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_init(void *parg);
/**@END! s32 com_func_init(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_start(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 启动
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_start(void *parg);
/**@END! s32 com_func_start(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_stop(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 停止
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_stop(void *parg);
/**@END! s32 com_func_stop(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_uninit(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 反初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_uninit(void *parg);
/**@END! s32 com_func_uninit(void *parg) !\(^o^)/~ 结束咯 */


#endif /* __COM_FUNC_H_ */
