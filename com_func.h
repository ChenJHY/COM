/**
 * \file    com_func.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning ��Ȩ����, ��Ȩ�ؾ�
 * \todo    COM���ܺ����ӿ�
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
*         �Ž����ݿ�ṹ��
************************************************************************/
struct db_arg 
{
	char *str;
	int len;
};

/************************************************************************
* struct  st_net_msg_header
*        ������Ϣͷ
************************************************************************/
typedef struct struct_net_msg_header{
    u32 maigc; /* 0xAABBCCDD */
    u32 len;   /* ���ݳ��� */
}st_net_msg_header, *pst_net_msg_header;

/************************************************************************
* struct  st_node
*        ����ڵ�
************************************************************************/
typedef struct struct_node{
    void  *data; /* ����Ԫ�� */
    struct struct_node *pnext; /* ��̽ڵ� */
}st_node, *pst_node;

/************************************************************************
* struct  st_list
*        ����
************************************************************************/
typedef struct struct_list{
    st_node         head; /* ����ͷ */
    pthread_mutex_t lock; /* ������ */
}st_list, *pst_list;

/************************************************************************
* enum  em_list_operate
*        �������
************************************************************************/
typedef enum enum_list_operate{
    operate_remove_em = 0x01, /* ɾ��Ԫ��   */
    operate_next_em = 0x02,   /* ��һ��Ԫ�� */
    operate_break_em = 0x04,  /* �жϱ���   */
}em_list_operate;

typedef em_list_operate(*com_func_list_callback)(void *, void *, va_list); /* ��������ص��������� */

/**
 * \fn      static SdInt com_func_msg_send(...)
 * \brief   ������Ϣ������ģ��
 * \param   ...
 * \return  ��Ϣ����״̬
 * \note
 * \todo
 * \version V1.0
 * \warning ���뱣֤�����ĺϷ���
 */
SdInt com_func_msg_send(SdUShort usDstModule, SdUShort usMsgID,
    SdULong ulParam1, SdULong ulParam2, SdULong ulContentLength, IN void* pContent);

/************************************************************************
* @FunctionName( ������ ): s32 com_func_sock_close(s32 sock)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): �ر�SOCK
*
* @Param(����): sock - SOCK���
*
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_sock_close(s32 sock);
/**@END! s32 com_func_sock_close(s32 sock) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_sock_nonblock(s32 sock)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����SOCK������
*
* @Param(����): sock - SOCK���
*
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_sock_nonblock(s32 sock);
/**@END! s32 com_func_sock_nonblock(s32 sock) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_block_timeout(s32 sock, s32 opt, s32 second_u32)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����SOCK������ʱ
*
* @Param(����):
*        sock - SOCK���
*        opt - ѡ��(SO_RCVTIMEO, SO_SNDTIMEO, ..)
*        second_u32 - ʱ��(��λ: second)
*
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_block_timeout(s32 sock, s32 opt, s32 second_u32);
/**@END! s32 com_func_block_timeout(s32 sock, s32 opt, s32 second_u32) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_tcp_server_create(u16 listen_port)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����SOCK������ʱ
*
* @Param(����): listen_port - �����˿�
*
* @ReturnCode(����ֵ): >0 - SOCK, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_tcp_server_create(u16 listen_port);
/**@END! s32 com_func_tcp_server_create(u16 listen_port) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_tcp_server_connect(const s8 *ip, const u16 port, s32 recv_time)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����TCP������
*
* @Param(����):
*       *ip:port - IP:�˿�
*        recv_time - ���ӳɹ�, SOCK�������ճ�ʱʱ��(��λ: second, ��� 120, <=0 ������)
*
* @ReturnCode(����ֵ): >0 - SOCK, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_tcp_server_connect(const s8 *ip, const u16 port, s32 recv_time);
/**@END! s32 com_func_tcp_server_connect(const s8 *ip, const u16 port, s32 recv_time) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_tcp_server_reconnect(const s8 *ip, const u16 port, s32 *psock, s32 recv_time)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��������TCP������
*
* @Param(����):
*       *ip:port - IP:PORT
*       *psock - ��������SOCK, �����SOCK
*        recv_time - ���ӳɹ�, SOCK�������ճ�ʱʱ��(��λ: second, ���: 120, <=0 ������)
*
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_tcp_server_reconnect(const s8 *ip, const u16 port, s32 *psock, s32 recv_time);
/**@END! s32 com_func_tcp_server_reconnect(const s8 *ip, const u16 port, s32 *psock, s32 recv_time) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): void com_func_header_conver(pst_net_msg_header pheader_st, const s8 *pbuf)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ת��������Ϣͷ
*
* @Param(����): pheader_st - ������Ϣͷ, *pbuf -
*
* @ReturnCode(����ֵ): None(��)
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
void com_func_header_conver(pst_net_msg_header pheader_st, const s8 *pbuf);
/**@END! void com_func_header_conver(pst_net_msg_header pheader_st, const s8 *pbuf) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): void com_func_net_msg_header_pack(s8 *pbuf, const u32 len)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ���������Ϣͷ
*
* @Param(����): *pbuf - ������, len - ����������
*
* @ReturnCode(����ֵ): None(��)
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
void com_func_net_msg_header_pack(s8 *pbuf, const u32 len);
/**@END! void com_func_net_msg_header_pack(s8 *pbuf, const u32 len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_tcp_data_send(const s32 sock, const s8 *cpbuf_s8, const u32 len)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����TCP����
*
* @Param(����):
*        sock - SOCK
*       *cpbuf_s8 - ���ݻ�����
*        len - ���ͳ���
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�
*       -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_tcp_data_send(const s32 sock, const s8 *cpbuf_s8, const u32 len);
/**@END! s32 com_func_tcp_data_send(const s32 sock, const s8 *cpbuf_s8, const u32 len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_tcp_msg_recv(const s32 sock, s8 **pprecv_s8, u32 *plen, s32 timeout)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����TCP������Ϣ
*
* @Param(����):
*        sock - SOCK
*      **pprecv_s8 - ���ؽ�������(malloc����)
*       *plen - ����������Ϣ����
*        timeout - ���ճ�ʱʱ��(��λ: second, ���: 120��)
*
* @ReturnCode(����ֵ):
*       -2 - δ֪����
*       -1 - SOCK�ر�
*        0 - �ɹ�
*        1 - ��ʱ
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_tcp_msg_recv(const s32 sock, s8 **pprecv_s8, u32 *plen, s32 timeout);
/**@END! s32 com_func_tcp_msg_recv(const s32 sock, s8 **pprecv_s8, u32 *plen, s32 timeout) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_list_init(pst_list plist_st)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): �����ʼ��
*
* @Param(����):
*        plist_st - ����
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_list_init(pst_list plist_st);
/**@END! s32 com_func_list_init(pst_list *pplist_st) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_list_destory(pst_list plist_st)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ��������
*
* @Param(����):
*        plist_st - ����
*
* @ReturnCode(����ֵ):
*        0
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_list_destory(pst_list plist_st);
/**@END! s32 com_func_list_destory(pst_list *pplist_st) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_list_elem_insert(pst_list plist_st, void *elem)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): �������Ԫ��
*
* @Param(����):
*        plist_st - ����
*       *elem - Ԫ��
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�
*       -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_list_elem_insert(pst_list plist_st, void *elem);
/**@END! s32 com_func_list_elem_insert(pst_list plist_st, void *elem) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_list_iterate(pst_list plist_st, com_func_list_callback callback, void *parg, ...)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): �������
*
* @Param(����):
*        plist_st - ����
*        callback - �����ص�����
*       *parg,... - ����б�
*
* @ReturnCode(����ֵ):
*        0 - �ɹ�
*       -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_list_iterate(pst_list plist_st, com_func_list_callback callback, void *parg, ...);
/**@END! s32 com_func_list_iterate(pst_list plist_st, com_func_list_callback callback, void *parg, ...) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_pathfile_path(const s8 *cpathfile, s8 *path, s32 buf_len)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): /path/file.type����/path
*
* @Param(����):
*       *cpathfile - /·��/�ļ�.����
*       *cpath - /·��
*        buf_len - *path����
*
* @ReturnCode(����ֵ): -1 - ʧ��, 0 - �ɹ�
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_pathfile_path(const s8 *cpathfile, s8 *path, s32 buf_len);
/**@END! s32 com_func_pathfile_path(const s8 *cpathfile, s8 *path, s32 buf_len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_pathfile_file(const s8 *cpathfile, s8 *pfile, s32 buf_len)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): /path/file.type����file.type
*
* @Param(����):
*       *cpathfile - /·��/�ļ�.����
*       *pfile - �ļ�.����
*        buf_len - *pfile����
*
* @ReturnCode(����ֵ): -1 - ʧ��, 0 - �ɹ�
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_pathfile_file(const s8 *cpathfile, s8 *pfile, s32 buf_len);
/**@END! s32 com_func_pathfile_file(const s8 *cpathfile, s8 *pfile, s32 buf_len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_file_type(const s8 *cpathfile, s8 *ptype, s32 buf_len)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): file.type����type
*
* @Param(����):
*       *cpathfile - /·��/�ļ�.����
*       *ptype - ����
*        buf_len - *ptype����
*
* @ReturnCode(����ֵ): -1 - ʧ��, 0 - �ɹ�
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_file_type(const s8 *cpathfile, s8 *ptype, s32 buf_len);
/**@END! s32 com_func_file_type(const s8 *cpathfile, s8 *ptype, s32 buf_len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_cmd_read(const char *cpcmd, s8 *pbuf, s32 buf_len)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): ����������ض���
*
* @Param(����):
*       *cpcmd - ����
*       *pbuf - ������
*        buf_len - *pbuf����
*
* @ReturnCode(����ֵ): -1 - ʧ��, 0 - �ɹ�
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_cmd_read(const char *cpcmd, s8 *pbuf, s32 buf_len);
/**@END! s32 com_func_cmd_read(const char *cpcmd, s8 *pbuf, s32 buf_len) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_file_size(const s8 *cpfile)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): �����ļ��ߴ�
*
* @Param(����): *cpfile - Դ�ļ�
*
* @ReturnCode(����ֵ): -1 - ʧ��, ���� - �ļ��ֽ���
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_file_size(const s8 *cpfile);
/**@END! s32 com_func_file_size(const s8 *cpfile) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_file_line(const s8 *cpfile)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): �����ļ�����
*
* @Param(����): *cpfile - Դ�ļ�
*
* @ReturnCode(����ֵ): -1 - ʧ��, ���� - ����
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_file_line(const s8 *cpfile);
/**@END! s32 com_func_file_line(const s8 *cpfile) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_attachment_savepath(enDataTypeDef emDataTypeDef, const s8 *cpathfile, char *psavepath)
* @CreateDate  (��������): 2019/5/16
* @Author      ( ��  �� ): CJH
*
* @Description(����): С����Ϣ��������·��
*
* @Param(����):
*        emDataTypeDef - С����Ϣ����
*       *cpathfile - /·��/�ļ�.����
*        psavepath - ����·��
*
* @ReturnCode(����ֵ): -1 - ʧ��, ���� - ����
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_attachment_savepath(enDataTypeDef emDataTypeDef, const s8 *cpathfile, char *psavepath);
/**@END! s32 com_func_attachment_savepath(enDataTypeDef emDataTypeDef, const s8 *cpathfile, char *psavepath) !\(^o^)/~ ������ */


SdChar *com_func_base64_encode(const SdUChar *bindata, SdChar *base64, SdInt binlength);
SdInt com_func_base64_decode(const SdChar *base64, SdChar *bindata);

/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_init(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ��ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_init(void *parg);
/**@END! s32 com_func_init(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_start(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_start(void *parg);
/**@END! s32 com_func_start(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_stop(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ֹͣ
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_stop(void *parg);
/**@END! s32 com_func_stop(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_func_uninit(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_func_uninit(void *parg);
/**@END! s32 com_func_uninit(void *parg) !\(^o^)/~ ������ */


#endif /* __COM_FUNC_H_ */
