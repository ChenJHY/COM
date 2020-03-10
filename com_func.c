/**
 * \file    com_func.c
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning 版权所有, 侵权必究
 * \todo    COM功能函数接口
 */
#include "com_func.h"

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
    SdULong ulParam1, SdULong ulParam2, SdULong ulContentLength, IN void* pContent)
{
   SdInt iRet = 0;
   iRet = ut_msg_send(emModCOM,         /**< 消息发送模块*/
                      usDstModule,      /**< 消息接收模块*/
                      usMsgID,          /**< 消息ID*/
                      0,                /**< 子消息ID*/
                      ulParam1,         /**< 参数1*/
                      ulParam2,         /**< 参数2*/
                      ulContentLength,  /**< 后续变长内容的长度*/
                      pContent);        /**< 要传送的消息内容*/

   return iRet;
}


static const s32 gsc_net_msg_magic = 0xAABBCCDD;

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
s32 com_func_sock_close(s32 sock){
    socklen_t addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in addr_sock;
    s32 ret_s32 = 0;

    if (sock <= 0)
        return 0;

    ret_s32 = getpeername(sock, (struct sockaddr *)&addr_sock, &addr_len);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): getpeername(%d)\n", __LINE__, sock);
        //        return 0;
    }

//    if (shutdown(sock, SHUT_RDWR)){
//        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): shutdown()\n", __LINE__);
//        return -1;
//    }
    if (close(sock)){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): close()\n", __LINE__);
        return -1;
    }

    return 0;
}
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
s32 com_func_sock_nonblock(s32 sock){
    s32 flag = fcntl(sock, F_GETFL, 0);

    if (-1 == fcntl(sock, F_SETFL, flag | O_NONBLOCK)){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): fcntl()\n", __LINE__);
        return -1;
    }

    return 0;
}
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
s32 com_func_block_timeout(s32 sock, s32 opt, s32 second_u32){
    s32 flag = fcntl(sock, F_GETFL, 0);
    socklen_t len = sizeof(struct timeval);
    struct timeval timeout;
    /* 设置阻塞 */
    flag &= ~O_NONBLOCK;
    if (-1 == fcntl(sock, F_SETFL, flag)){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): fcntl()\n", __LINE__);
        return -1;
    }
    timeout.tv_sec = second_u32;
    timeout.tv_usec = 0;
    /* 设置选项 */
    if (setsockopt(sock, SOL_SOCKET, opt, &timeout, len) == -1){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): setsockopt()\n", __LINE__);
        return -1;
    }
    return 0;
}
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
s32 com_func_tcp_server_create(u16 listen_port){
    struct sockaddr_in addr;
    s32 sock = 0, reuse = 1, ret_s32 = 0;
    /* 创建SOCK */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): socket()\n", __LINE__);
        return-1;
    }
    /* 设置socket非阻塞 */
//    com_func_sock_nonblock(sock); /* 使用IO模型 */
    ret_s32 = com_func_block_timeout(sock, SO_RCVTIMEO, 2); /* 阻塞接收 */
    if (ret_s32 != 0){
        com_func_sock_close(sock);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): com_func_block_timeout()\n", __LINE__);
        return-1;
    }
    /* 设置地址重用 */
    ret_s32 = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (ret_s32 != 0){
        com_func_sock_close(sock);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): setsockopt()\n", __LINE__);
        return -1;
    }
    /* 设置端口重用 */
    ret_s32 = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)); /* 重用端口 */
    if(ret_s32 != 0){
       com_func_sock_close(sock);
       UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): setsockopt()\n", __LINE__);
       return -1;
    }
    /* SOCK参数 */
    addr.sin_family = AF_INET; /* IPv4 */
    addr.sin_port = htons(listen_port); /* 监听端口 */
    addr.sin_addr.s_addr = htonl(INADDR_ANY); /* 监听IP地址(0.0.0.0) */
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    /* 参数与SOCK绑定 */
    if (bind(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): socket()\n", __LINE__);
        com_func_sock_close(sock);
        return -1;
    }
    /* 监听SOCK */
    if (listen(sock, 10) != 0){ /* 参数2: 等待accept的队列长度 */
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): listen()\n", __LINE__);
        com_func_sock_close(sock);
        return -1;
    }
    return sock;
}
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
s32 com_func_tcp_server_connect(const s8 *ip, const u16 port, s32 recv_time){
    struct sockaddr_in addr;
    s32 reuse_addr = 1;
    s32 sock = 0;
    s32 ret_s32 = 0;

    if (ip == NULL)
        return -1;
    /* 创建SOCK */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): socket()\n", __LINE__);
        return -1;
    }
    /* 设置connect阻塞超时 */
    if (com_func_block_timeout(sock, SO_SNDTIMEO, 6) == -1){
        com_func_sock_close(sock);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): com_func_block_timeout()\n", __LINE__);
        return -1;
    }
    /* 设置地址重用 */
    ret_s32 = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    if (ret_s32 != 0){
        com_func_sock_close(sock);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): setsockopt()\n", __LINE__);
        return -1;
    }
    /* TCP服务器参数 */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr((char *)ip);
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    /* 连接TCP服务器 */
    if (connect(sock, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != 0){
        com_func_sock_close(sock);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): connect tcp server (%s:%d) fail!\n", __LINE__, ip, port);
        return -1;
    }
    /* 接收时间 */
    if (recv_time <= 0) /* 非阻塞 */
        ret_s32 = com_func_sock_nonblock(sock);
    else{ /* 阻塞 */
        if (recv_time > 120)
            recv_time = 120;
        ret_s32 = com_func_block_timeout(sock, SO_RCVTIMEO, recv_time);
    }
    if (ret_s32 != 0){
        com_func_sock_close(sock);
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): set sock(%d) opt.\n", __LINE__, sock);
        return -1;
    }
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): connect tcp server (%s:%d)@(%d) success!recv_time = %d\n", __LINE__, ip, port, sock, recv_time);
    return sock;  /* 返回sock句柄 */
}
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
s32 com_func_tcp_server_reconnect(const s8 *ip, const u16 port, s32 *psock, s32 recv_time){
    if (psock == NULL)
        return -1;
    /* 关闭SOCK */
    com_func_sock_close(*psock);
    /* 连接TCP服务器 */
    *psock = com_func_tcp_server_connect(ip, port, recv_time);
    if ((*psock) == -1){
        *psock = 0;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): reconnect tcp server (%s:%d) fail!\n", __LINE__, ip, port);
        return -1;
    }
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): reconnect tcp server (%s:%d)@(%d) success!\n", __LINE__, ip, port, *psock);
    return 0;
}
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
void com_func_header_conver(pst_net_msg_header pheader_st, const s8 *pbuf){
    if ((pbuf == NULL) || (pheader_st == NULL))
        return;
    memcpy(pheader_st, pbuf, sizeof(st_net_msg_header));
    pheader_st->maigc = ntohl(pheader_st->maigc);
//    pheader_st->len = ntohl(pheader_st->len);
}
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
void com_func_net_msg_header_pack(s8 *pbuf, const u32 len){
    st_net_msg_header net_msg_header_st;

    if ((pbuf == NULL) || (len == 0))
        return;
    net_msg_header_st.maigc = gsc_net_msg_magic;
    net_msg_header_st.len = len;

    com_func_header_conver(&net_msg_header_st, (const s8 *)&net_msg_header_st);

    memcpy(pbuf, &net_msg_header_st, sizeof(st_net_msg_header));
}
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
s32 com_func_tcp_data_send(const s32 sock, const s8 *cpbuf_s8, const u32 len){
    socklen_t addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    s32 ret_s32 = 0;

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): sending tcp data to SOCK(%d)..\n", __LINE__, sock);

    if ((cpbuf_s8 == NULL) || (len == 0) || (sock <= 0))
        return -1;

    if (getpeername(sock, (struct sockaddr *)&addr, &addr_len) != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): getpeername()\n", __LINE__);
        return -1;
    }

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): send tcp data (%s) len(%d)\n", __LINE__, cpbuf_s8 + 8, len);

    ret_s32 = send(sock, cpbuf_s8, len, MSG_DONTWAIT);
    if (ret_s32 == -1){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): send to sock(%d) lost connect(-.-)!!\n", __LINE__, sock);
        return -1;
    }

    if (ret_s32 != len)
        return -1;
    return 0;
}
/**@END! s32 com_func_tcp_data_send(const s32 sock, const s8 *cpbuf_s8, const u32 len) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): static s32 com_func_tcp_data_recv(const s32 sock, s8 *precv_s8, const u32 len_u32, s32 timeout)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 接收TCP数据
*
* @Param(参数):
*        sock - SOCK
*       *precv_s8 - 接收缓冲区
*        len - 接收长度
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
static s32 com_func_tcp_data_recv(const s32 sock, s8 *precv_s8, const u32 len_u32, s32 timeout){
    socklen_t addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    s8 ip[20] = { 0 };
    s8 *pip = NULL;
    u16 port = 0;
    u32 rx_len = 0;
    s32 err_s32 = 0, retrys = 0, ret_s32 = 0;

    if ((sock <= 0) || (precv_s8 == NULL) || (len_u32 <= 0))
        return -1;

    if (timeout > 120)
        timeout = 120;
    timeout *= 1000;

    if (getpeername(sock, (struct sockaddr *)&addr, &addr_len) != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): getpeername()\n", __LINE__);
        return -1;
    }

    pip = (s8 *)inet_ntoa(addr.sin_addr);
    strcpy((char *)ip, (char *)pip); pip = ip;
    port = ntohs(addr.sin_port);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): recving tcp data from (%s:%d)@(%d)...\n", __LINE__, pip, port, sock);
    do{
        ret_s32 = recv(sock, precv_s8 + rx_len, len_u32 - rx_len, MSG_DONTWAIT);
        err_s32 = errno;
        if (ret_s32 == -1){
            if ((err_s32 == EAGAIN) || (err_s32 == EWOULDBLOCK) || (err_s32 == EINTR)){
                usleep(100000);
                if ((retrys += 100) <= timeout){
                    continue;
                }else{
                    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): recv tcp data from (%s:%d)@(%d) timeout.\n", __LINE__, pip, port, sock);
                    return 1;
                }
            }
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): recv tcp data from (%s:%d)@(%d) failure.\n", __LINE__, pip, port, sock);
            return -2;
        }else if (ret_s32 == 0){ /* 链接已经断开 */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): recv from (%s:%d)@(%d) lost connect(-.-)!!\n", __LINE__, pip, port, sock);
            return -1;
        }else{
            rx_len += ret_s32;
            precv_s8[rx_len] = '\0';
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): recv tcp data from (%s:%d)@(%d) len (%d)\n", __LINE__, pip, port, sock, ret_s32);
        }
    } while (rx_len < len_u32);
    return 0;
}
/**@END! static s32 com_func_tcp_data_recv(const s32 sock, s8 *precv_s8, const u32 len_u32, s32 timeout) !\(^o^)/~ 结束咯 */

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
s32 com_func_tcp_msg_recv(const s32 sock, s8 **pprecv_s8, u32 *plen, s32 timeout){
    st_net_msg_header net_msg_header_st;
    s8 header_buf[10] = { 0 };
    s8 *pbuf_s8 = *pprecv_s8;
    u32 rx_len = 0;
    s32 ret_s32;

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): com_func_tcp_msg_recv() running...\n", __LINE__);

    *plen = 0;
    if (pbuf_s8 != NULL){
        free(pbuf_s8);
        *pprecv_s8 = NULL;
    }

    rx_len = sizeof(st_net_msg_header);
    pbuf_s8 = header_buf;
    ret_s32 = com_func_tcp_data_recv(sock, pbuf_s8, rx_len, timeout);
    if (ret_s32 != 0)
        return ret_s32;

    {
        s8 read[2] = { 0 };
        do{
            com_func_header_conver(&net_msg_header_st, pbuf_s8);
            if (net_msg_header_st.maigc == gsc_net_msg_magic){
                rx_len = net_msg_header_st.len;
                UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): recv tcp msg maigc = (0x%X), len = (0x%X)\n", __LINE__, gsc_net_msg_magic, rx_len);
                break;
            }
            ret_s32 = com_func_tcp_data_recv(sock, read, 1, timeout);
            if (ret_s32 != 0)
                return ret_s32;
            memcpy(pbuf_s8, pbuf_s8 + 1, rx_len);
            pbuf_s8[rx_len - 1] = read[0];
        } while (1);
    }

    if (rx_len == 0)
        return -1;

    pbuf_s8 = malloc(rx_len + 1);
    if (pbuf_s8 == NULL){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): malloc()\n", __LINE__);
        return -1;
    }
    *pprecv_s8 = pbuf_s8;
    /* 接收TCP数据 */
    ret_s32 = com_func_tcp_data_recv(sock, pbuf_s8, rx_len, timeout);
    if (ret_s32 != 0)
        return ret_s32;
    /* 接收成功, 返回接收数据长度 */
    *plen = rx_len;
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): recv tcp msg (%s)\n", __LINE__, pbuf_s8);

    return 0;
}
/**@END! s32 com_func_tcp_msg_recv(const s32 sock, s8 **pprecv_s8, u32 *plen, s32 timeout) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_func_udp_msg_recv(const s32 sock, s8 *precv_s8, const u32 buf_len, u32 *plen, struct sockaddr_in *addr_src)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( 作  者 ): CJH
*
* @Description(描述): 接收UDPP网络消息
*
* @Param(参数):
*        sock - SOCK句柄
*        precv_s8 - 接收缓冲区
*        buf_len - 缓冲区长度
*       *plen - 返回网络消息长度
*       *addr_src - 网络消息源地址
*
* @ReturnCode(返回值):
*       -2 - 未知错误
*        0 - 成功
*        1 - 超时
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_func_udp_msg_recv(const s32 sock, s8 *precv_s8, const u32 buf_len, u32 *plen, struct sockaddr_in *addr_src){
    const u32 header_len = sizeof(st_net_msg_header);
    socklen_t addr_len = sizeof(struct sockaddr_in);
    st_net_msg_header net_msg_header_st;
    s8 *pbuf = precv_s8;
    u32 rx_len = 0;
    s32 ret_s32 = 0;

//    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): com_func_udp_msg_recv() running..\n", __LINE__);

    *plen = 0;
    /* 接收UDP数据 */
    ret_s32 = recvfrom(sock, pbuf, buf_len - 1, MSG_WAITALL, (struct sockaddr *)addr_src, &addr_len);
    if (ret_s32 <= 0)
        return -2;
    pbuf[ret_s32] = '\0';
    {
        u32 read_num = 0;
        do{
            com_func_header_conver(&net_msg_header_st, pbuf);
            /* 定位网络消息头 */
            if (net_msg_header_st.maigc == gsc_net_msg_magic){
                rx_len = net_msg_header_st.len;
                if (rx_len > (buf_len - (read_num + header_len)))
                    return 1;
                UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): recv udp msg maigc = (0x%X), len = (0x%X)\n", __LINE__, gsc_net_msg_magic, rx_len);
                break;
            }
            read_num++;
            pbuf++;
            if ((ret_s32 - read_num) <= header_len)
                return 1;
        } while (1);
    }

    memcpy(precv_s8, pbuf + header_len, rx_len + 1);
    *plen = rx_len;
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Info(LINE=%d): recv udp msg (%s)\n", __LINE__, precv_s8);

    return 0;
}
/**@END! s32 com_func_udp_msg_recv(const s32 sock, s8 *precv_s8, const u32 buf_len, u32 *plen, struct sockaddr_in *addr_src) !\(^o^)/~ 结束咯 */

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
s32 com_func_list_init(pst_list plist_st){
    s32 ret_s32 = 0;

    if (plist_st == NULL)
        return -1;

    ret_s32 = pthread_mutex_init(&plist_st->lock, NULL);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_plugin Error(LINE=%d): pthread_mutex_init()\n", __LINE__);
        return -1;
    }
    plist_st->head.pnext = NULL;
    pthread_mutex_unlock(&plist_st->lock);

    return 0;
}
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
s32 com_func_list_destory(pst_list plist_st){
    if (plist_st == NULL)
        return -1;

    pthread_mutex_lock(&plist_st->lock);
    pthread_mutex_destroy(&plist_st->lock);

    return 0;
}
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
s32 com_func_list_elem_insert(pst_list plist_st, void *elem){
    const pst_node cphead_st = &plist_st->head;
    pst_node pnode_st = NULL;

    if (plist_st == NULL) /* 链表不存在 */
        return -1;

    pnode_st = (pst_node)malloc(sizeof(st_node));
    if (pnode_st == NULL) /* 新节点molloc失败 */
        return -1;

    pthread_mutex_lock(&plist_st->lock);
    pnode_st->data = elem;
    pnode_st->pnext = cphead_st->pnext;
    cphead_st->pnext = pnode_st;
    pthread_mutex_unlock(&plist_st->lock);

    return 0;
}
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
s32 com_func_list_iterate(pst_list plist_st, com_func_list_callback callback, void *parg, ...){
    const pst_node cphead_st = &plist_st->head;
    pst_node p_st = NULL, q_st = cphead_st;
    em_list_operate list_operate_em;
    va_list args;
    s32 num_s32 = 0;

    if (plist_st == NULL)
        return -1;

    pthread_mutex_lock(&plist_st->lock);
    va_start(args, parg);
    p_st = q_st->pnext;
    while (p_st != NULL){ /* 遍历链表 */
        if (callback)
            list_operate_em = callback(p_st->data, parg, args);
        else
            list_operate_em = operate_next_em;

        if (list_operate_em == operate_next_em){ /* 下一元素 */
            q_st = p_st;
            num_s32++;
        }
        else{
            if (list_operate_em & operate_remove_em){ /* 删除元素 */
                q_st->pnext = p_st->pnext;
                free(p_st);
            }
            if (list_operate_em & operate_break_em) /* 退出迭代 */
                break;
        }
        p_st = q_st->pnext;
    }
    va_end(args);
    pthread_mutex_unlock(&plist_st->lock);

    return num_s32;
}
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
s32 com_func_pathfile_path(const s8 *cpathfile, s8 *path, s32 buf_len){
    char *pbuf = NULL;

    if(cpathfile == NULL)
        return -1;
    if(path == NULL)
        return -1;

    strncpy((char *)path, (char *)cpathfile, buf_len);
    path[buf_len - 1] = '\0';

    pbuf = strrchr((char *)path, '/');

    if(pbuf == NULL){
        path[0] = '\0';
        return -1;
    }
    pbuf[1] = '\0';
    return 0;
}
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
s32 com_func_pathfile_file(const s8 *cpathfile, s8 *pfile, s32 buf_len){
    char *pbuf = NULL;

    if(cpathfile == NULL)
        return -1;
    if(pfile == NULL)
        return -1;

    pbuf = strrchr((char *)cpathfile, '/');
    if(pbuf == NULL){
        pfile[0] = '\0';
        return -1;
    }

    strncpy((char *)pfile, pbuf + 1, buf_len);
    pfile[buf_len - 1] = '\0';
    return 0;
}
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
s32 com_func_file_type(const s8 *cpathfile, s8 *ptype, s32 buf_len){
    char *pbuf = NULL;

    if(cpathfile == NULL)
        return -1;
    if(ptype == NULL)
        return -1;

    pbuf = strchr((char *)cpathfile, '.');
    if(pbuf == NULL){
        ptype[0] = '\0';
        return -1;
    }

    strncpy((char *)ptype, pbuf + 1, buf_len);
    ptype[buf_len - 1] = '\0';
    return 0;
}
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
s32 com_func_cmd_read(const char *cpcmd, s8 *pbuf, s32 buf_len){
    FILE *fp = popen(cpcmd, "r");
    s32 ret_s32 = 0;

    if(fp == SD_NULL)
        return -1;

    ret_s32 = fread(pbuf, 1, buf_len, fp);
    pclose(fp);

    if(ret_s32 <= 0){
        pbuf[0] = '\0';
        return -1;
    }

    pbuf[ret_s32] = '\0';
    return 0;
}
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
s32 com_func_file_size(const s8 *cpfile){
    FILE *fp = NULL;
    s32 size_s32 = 0;

    if ((fp = fopen((char *)cpfile, "rb")) == NULL){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): fopen(%s)\n", __LINE__, cpfile);
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    size_s32 = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    fclose(fp);
    return size_s32;
}
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
s32 com_func_file_line(const s8 *cpfile){
    char cmd[256] = {0};
    s8 buf[12] = {0};
    s32 line_s32 = 0, ret_s32 = 0;

    if(access((const char *)cpfile, F_OK) != 0) /* 文件不存在 */
        return -1;

    snprintf(cmd, sizeof(cmd), "wc -l %s | awk -F \" \" '{printf $1}'", cpfile);
    ret_s32 = com_func_cmd_read(cmd, buf, sizeof(buf));
    if(ret_s32 != 0)
        return -1;

    line_s32 = atoi((char *)buf);
    return line_s32;
}
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
s32 com_func_attachment_savepath(enDataTypeDef emDataTypeDef, const s8 *cpathfile, char *psavepath){
    char type[16] = {0};

    if(cpathfile == NULL)
        return -1;
    com_func_file_type(cpathfile, (s8 *)type, sizeof(type));
    if(type[0] == '\0')
        return -1;

    if(emDataTypeDef == emAdMsg){ /* 广告信息 */
        if(!strcasecmp(type, "txt"))
            strcpy(psavepath, PATH_DATA_AD_TXT); /* 文本文件 */
        else if((!strcasecmp(type, "pcm")) || (!strcasecmp(type, "mp3"))  || (!strcasecmp(type, "wav")))
            strcpy(psavepath, PATH_DATA_AD_SOUND); /* 音频文件 */
        else if((!strcasecmp(type, "mp4")) || (!strcasecmp(type, "rmvb")) || (!strcasecmp(type, "avi")))
            strcpy(psavepath, PATH_DATA_AD_VIEDEO); /* 视频文件 */
        else if((!strcasecmp(type, "jpg")) || (!strcasecmp(type, "jpeg")) || (!strcasecmp(type, "bmp")) || (!strcasecmp(type, "png")))
            strcpy(psavepath, PATH_DATA_AD_PICTURE); /* 图片文件 */
        else
            strcpy(psavepath, PATH_DATA_AD_OTHER); /* 其他文件 */
    }else{ /* 其他信息 */
        if(!strcasecmp(type, "txt"))
            strcpy(psavepath, PATH_DATA_AREAINFO_TXT); /* 文本文件 */
        else if((!strcasecmp(type, "pcm")) || (!strcasecmp(type, "mp3"))  || (!strcasecmp(type, "wav")))
            strcpy(psavepath, PATH_DATA_AREAINFO_SOUND); /* 音频文件 */
        else if((!strcasecmp(type, "mp4")) || (!strcasecmp(type, "rmvb")) || (!strcasecmp(type, "avi")))
            strcpy(psavepath, PATH_DATA_AREAINFO_VIDEO); /* 视频文件 */
        else if((!strcasecmp(type, "jpg")) || (!strcasecmp(type, "jpeg")) || (!strcasecmp(type, "bmp")) || (!strcasecmp(type, "png")))
            strcpy(psavepath, PATH_DATA_AREAINFO_PICTURE); /* 图片文件 */
        else
            strcpy(psavepath, PATH_DATA_AREAINFO_OTHER); /* 其他文件 */
    }

    return 0;
}
/**@END! s32 com_func_attachment_savepath(enDataTypeDef emDataTypeDef, const s8 *cpathfile, char *psavepath) !\(^o^)/~ 结束咯 */

const SdChar *base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+//";

SdChar *com_func_base64_encode(const SdUChar *bindata, SdChar *base64, SdInt binlength)
{
    SdInt i, j;
    SdUChar current;

    for(i = 0, j = 0 ; i < binlength; i += 3)
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ((unsigned char)(bindata[i] << 4 )) & ((unsigned char)0x30);
        if((i + 1) >= binlength)
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ((unsigned char)(bindata[i+1] >> 4)) & ((unsigned char)0x0F);
        base64[j++] = base64char[(int)current];

        current = ((unsigned char)(bindata[i+1] << 2)) & ((unsigned char)0x3C) ;
        if((i + 2) >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ((unsigned char)(bindata[i+2] >> 6)) & ((unsigned char)0x03);
        base64[j++] = base64char[(int)current];

        current = ((unsigned char)bindata[i+2] ) & ((unsigned char)0x3F) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}

SdInt com_func_base64_decode(const SdChar *base64, SdChar *bindata)
{
    SdInt i, j, k;
    SdChar temp[4];
    for(i = 0, j = 0; base64[i] != '\0' ; i += 4)
    {
        memset(temp, 0xFF, sizeof(temp));
        for(k = 0; k < 64; k ++ )
            if( base64char[k] == base64[i])
                temp[0]= k;
        for(k = 0; k < 64; k++)
            if(base64char[k] == base64[i+1])
                temp[1]= k;
        for(k = 0; k < 64; k++)
            if(base64char[k] == base64[i+2])
                temp[2]= k;
        for(k = 0; k < 64; k++)
            if(base64char[k] == base64[i+3])
                temp[3]= k;

        bindata[j++] = ((SdChar)(((SdChar)(temp[0] << 2)) & 0xFC)) | ((SdChar)((SdChar)(temp[1] >> 4) & 0x03));
        if (base64[i+2] == '=')
            break;

        bindata[j++] = ((SdChar)(((SdChar)(temp[1] << 4)) & 0xF0)) | ((SdChar)((SdChar)(temp[2] >> 2) & 0x0F));
        if (base64[i+3] == '=')
            break;

        bindata[j++] = ((SdChar)(((SdChar)(temp[2] << 6)) & 0xF0)) | ((SdChar)(temp[3] & 0x3F));
    }
    return j;
}


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
s32 com_func_init(void *parg){
    return 0;
}
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
s32 com_func_start(void *parg){
    return 0;
}
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
s32 com_func_stop(void *parg){
    return 0;
}
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
s32 com_func_uninit(void *parg){
    return 0;
}
/**@END! s32 com_func_uninit(void *parg) !\(^o^)/~ 结束咯 */


#if (0)
#include "com_ctrl.h"

const SdInt iHeaderMagic = 0xAABBCCDD; /* 0x61626364, 测试用 */

#define  TCP_CONNECT_TIMER_OUT                  5     /* 单位: second */


/**
 * \fn     void com_func_protocol_header_conver(IN const SdChar *pszBuf, LPProtocolHeader pstHeader);
 * \brief  协议头, 字节序列转换
 * \param  IN const SdChar *pszBuf - 缓冲区数据
 * \param  OUT LPProtocolHeader pstHeader - 协议头
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning 必须保证参数的合法性
*/
void com_func_protocol_header_conver(IN const SdChar *pszBuf, OUT LPProtocolHeader pstHeader)
{
   memcpy(pstHeader, pszBuf, sizeof(ProtocolHeader));
   pstHeader->iMagic = ntohl(pstHeader->iMagic);
//   pstHeader->iLen = ntohl(pstHeader->iLen); /* 网络字节序列到主机字节序列准换 */
}

/**
* \fn     void com_func_sock_nonblock_set(SdInt iSock)
* \brief  设置SOCK非阻塞
* \param  SdInt iSock - Socket句柄
* \return 成功, 返回(0)
* \return 失败, 返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
void com_func_sock_nonblock_set(SdInt iSock)
{
   SdInt iFlag = 0;
   iFlag = fcntl(iSock, F_GETFL, 0); /* 读Sock选项 */
   if(-1 == fcntl(iSock, F_SETFL, iFlag | O_NONBLOCK)) /* 非阻塞 */
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: set sock (%d) nonblock fail.", __FILE__, __LINE__, iSock);
   }
}

/**
* \fn     static SdInt com_func_blocksocket_timeout_set(SdInt iSock, SdInt iSecond)
* \brief  设置阻塞Sock超时时间
* \param  SdInt iSock - Socket 句柄
* \param  SdInt iSecond - 超时间, 单位: second
* \return 成功, 返回0
* \return 失败, 返回-1
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
static SdInt com_func_blocksocket_timeout_set(SdInt iSock, SdInt iSecond)
{
   struct timeval timeout;
   socklen_t len = sizeof(timeout);

   timeout.tv_sec = iSecond;
   timeout.tv_usec = 0;

   if(setsockopt(iSock, SOL_SOCKET, SO_SNDTIMEO, &timeout, len) == -1)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: setsockopt fail.", __FILE__,  __LINE__);
      return -1;
   }

   return 0;
}

/**
* \fn      SdInt com_func_sock_close(IN SdInt sock)
* \brief   关闭SOCK
* \param   SdInt iSock - Socket句柄
* \return  成功, 返回(0)
* \return  失败, 返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_func_sock_close(SdInt iSock)
{
   SdInt iRet = 0;
   struct sockaddr_in sock_addr;
   socklen_t addr_len = sizeof(struct sockaddr_in);

   if(iSock <= 0)
      return 0;

   iRet = getpeername(iSock, (struct sockaddr *)&sock_addr, &addr_len);
   if(iRet != 0)
      return 0;

   if(shutdown(iSock, SHUT_RDWR))
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: shutdown fail.", __FILE__, __LINE__);
      return -1;
   }

   if(close(iSock))
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: close fail.", __FILE__, __LINE__);
      return -1;
   }
   return 0;
}

/**
* \fn     SdInt com_func_TCPserver_create(IN const SdChar *szLocalIP, IN SdUInt16 usLocalPort, OUT SdInt *iSock)
* \brief  创建TCP服务器
* \param  IN const SdChar *szLocalIP - 本地IP地址
* \param  IN SdUInt16 usLocalPort - 本地TCP端口
* \param  OUT SdInt *iSock - 返回Socket句柄
* \return 成功, 返回(0)
* \return 失败, 返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_func_TCPserver_create(IN const SdChar *szLocalIP, IN SdUInt16 usLocalPort, OUT SdInt *iSock)
{
   SdInt iRet = 0;
   SdInt iServerSock = 0;
   SdInt iReUse = 1; /* 1, 重用地址和端口 */
   struct sockaddr_in ServerSockAddr;

   if(iSock == NULL)
      return -1;

   if((iServerSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: socket fail.", __FILE__, __LINE__);
      return -1;
   }
   *iSock = iServerSock; /* 返回Sock句柄 */

   com_func_sock_nonblock_set(iServerSock); /* 设置TCP server sock非阻塞 */
   iRet = setsockopt(iServerSock, SOL_SOCKET, SO_REUSEADDR, &iReUse, sizeof(iReUse)); /* 重用地址 */
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: setsockopt fail.", __FILE__, __LINE__);
      return -1;
   }
   iRet = setsockopt(iServerSock, SOL_SOCKET, SO_REUSEPORT, &iReUse, sizeof(iReUse)); /* 重用端口 */
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: setsockopt fail.", __FILE__, __LINE__);
      return -1;
   }

   memset(&ServerSockAddr,0 , sizeof(ServerSockAddr));
   ServerSockAddr.sin_family = AF_INET;  
   ServerSockAddr.sin_port = htons(usLocalPort);

   if(szLocalIP != NULL)
      ServerSockAddr.sin_addr.s_addr = inet_addr(szLocalIP);
   else
      ServerSockAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 使用0 地址, 监听所有网卡 */

   if(bind(iServerSock,(struct sockaddr *)&ServerSockAddr, sizeof(struct sockaddr)) != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: bind fail.", __FILE__, __LINE__);
      return -1;
   }

   if(listen(iServerSock, 3) != 0) /* 握手完成, accept队列数量3 */
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: listen fail.", __FILE__, __LINE__);
      return -1;
   }

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Create TCP server: (%s:%d)", inet_ntoa(ServerSockAddr.sin_addr), usLocalPort);
   return 0;
}

/**
* \fn       SdInt com_func_TCPsever_connect(IN const SdChar *pszRemoteIP, SdUInt16 usRemotePort, OUT SdInt *piSock)
* \brief   连接TCP server, 返回sock句柄
* \param    IN const SdChar *pszRemoteIP - 目标IP地址
* \param    SdUInt16 usRemotePort - 目标端口
* \param    OUT SdInt *piSock - 返回Sock句柄
* \return   成功，返回(0)
* \return   失败，返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_func_TCPserver_connect(IN const SdChar *pszRemoteIP, SdUInt16 usRemotePort, OUT SdInt *piSock)
{
   SdInt iRet = 0;
   SdInt iSock;
   SdInt iReUseAddr = 1;
   struct sockaddr_in ClientSockAddr;

   if(pszRemoteIP == NULL || piSock == NULL)
      return -1;

   if((iSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: socket fail.", __FILE__, __LINE__);
      return -1;
   }

   if(com_func_blocksocket_timeout_set(iSock, TCP_CONNECT_TIMER_OUT) == -1)  /* Connect阻塞(目标IP无回应)超时 */
      return -1;
   
   iRet = setsockopt(iSock, SOL_SOCKET, SO_REUSEADDR, &iReUseAddr, sizeof(iReUseAddr));
   if(iRet != 0)  
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: setsockopt fail.", __FILE__, __LINE__);
      return -1;
   }

   memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
   ClientSockAddr.sin_family = AF_INET;  
   ClientSockAddr.sin_port = htons(usRemotePort);
   ClientSockAddr.sin_addr.s_addr = inet_addr(pszRemoteIP);

   if(connect(iSock, (const struct sockaddr *)&ClientSockAddr, sizeof(ClientSockAddr)) != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: connect (%s:%d) fail.", __FILE__, __LINE__, pszRemoteIP, usRemotePort);
      return -1;
   }
   com_func_sock_nonblock_set(iSock); /* 设置非阻塞 */
   *piSock = iSock; /* 返回sock句柄 */

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Connect TCP Server (%s:%d) success!", inet_ntoa(ClientSockAddr.sin_addr), usRemotePort);

   return 0;
}

/**
* \fn       SdInt com_func_TCPsever_reconnect(IN const SdChar *pszRemoteIP, SdUInt16 usRemotePort, INOUT SdInt *piSock)
* \brief   重新连接TCP server, 返回sock句柄
* \param    IN const SdChar *pszRemoteIP - 目标IP地址
* \param    SdUInt16 usRemotePort - 目标端口
* \param    INOUT SdInt *piSock - 输入连接失效sock句柄, 返回Sock句柄
* \return   成功，返回(0)
* \return   失败，返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_func_TCPserver_reconnect(IN const SdChar *pszRemoteIP, SdUInt16 usRemotePort, INOUT SdInt *piSock){
   SdInt iRet = 0;
   struct sockaddr_in sock_addr;
   socklen_t addr_len = sizeof(struct sockaddr_in);

   com_func_sock_close(*piSock); /* 关闭sock */

   iRet = com_func_TCPserver_connect(pszRemoteIP, usRemotePort, piSock);
   if(iRet == -1)
      return -1;

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "ReConnect TCP Server (%s:%d) success!", pszRemoteIP, usRemotePort);
   return 0;
}

/**
* \fn	SdInt com_func_TCPdata_send(SdInt iSock, IN const SdChar *pszBuf, SdUInt iLen, SdInt iMilliTimeOut)
* \brief	超时发送TCP数据
* \param    SdInt iSock - sock句柄
* \param   IN const SdChar *pszBuf - 发送缓冲区指针
* \param   SdUInt len - 缓冲区长度
* \param   SdInt iMilliTimeOut - 毫秒发送超时时间
* \return   1 - 发送超时
* \return  -1 - Socket关闭
* \return  -2 - 未定义错误
* \return   0 - 发送成功
* \note     
* \todo    
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_func_TCPdata_send(SdInt iSock, IN const SdChar *pszBuf, SdUInt iLen, SdInt iMilliTimeOut)
{
   struct sockaddr_in sock_addr;
   socklen_t addr_len = sizeof(struct sockaddr_in);

   SdChar *pszIP = NULL;
   SdUShort usPort = 0;

   SdInt iRet;
   SdInt iTxLen = 0; /* 发送长度 */
   SdInt iRetryTimes = 0; /* 重试次数 */

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func_TCPdata_send(..) running...");
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Send TCP data: (%s), len = (%d).", pszBuf + 8, iLen);

   if((pszBuf== NULL) || (iLen <= 0)  || (iSock <= 0))
      return -1;

   if(getpeername(iSock, (struct sockaddr *)&sock_addr, &addr_len) != 0)
   {
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s:%d]: getpeername error.",  __FILE__, __LINE__);
      return -1;
   }
   pszIP = inet_ntoa(sock_addr.sin_addr);
   usPort = ntohs(sock_addr.sin_port);

   do
   {
      iRet = send(iSock, pszBuf + iTxLen, iLen - iTxLen, 0);
      if(iRet == -1) /* 发送出错 */
      {
         if((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR)) /* 发送数据啊.. */
         { /* EAGAIN == EWOULDBLOCK, 数据不足 , EINTR = 由于信号中断, 返回 */
            usleep(1000); /* 1milli后重试接收 */
            if(++iRetryTimes < iMilliTimeOut)
               continue;
            else /* 发送超时 */
            {
               UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: Send TCP data to (%s:%d) timeout (%d) seconds.",  __FILE__, __LINE__, pszIP, usPort, iMilliTimeOut / 1000);
               return 1;
            }
         }
         else if(errno == ECONNRESET) /* 连接关闭 */
         {
            UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: Close TCP sock (%s:%d).", __FILE__, __LINE__, pszIP, usPort);
         }

         UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: Send TCP data to (%s:%d) failure.", __FILE__, __LINE__, pszIP, usPort);
         return -2; /* 未知错误, 直接退出 */
      }
      else /* 发送数据 */
      {
         iTxLen += iRet;
      }
   }while(iTxLen < iLen);
   return 0;
}


/**
* \fn      static SdInt com_func_TCPdata_recv(SdInt iSock, OUT SdChar *pszBuf,  SdInt iLen, SdInt iMilliTimeOut)
* \brief   超时接收TCP数据
* \param    SdInt iSock - sock句柄
* \param   OUT SdChar *pszBuf - 接收缓冲区
* \param   SdUInt len - 接收数据长度
* \param   SdInt iMilliTimeOut - 接收超时时间(单位: milli)
* \return   1 - 接收超时
* \return  -1 - Socket关闭
* \return  -2 - 未定义错误
* \return   0 - 接收成功
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
static SdInt com_func_TCPdata_recv(SdInt iSock, OUT SdChar *pszBuf,  SdInt iLen, SdInt iMilliTimeOut)
{
   struct sockaddr_in sock_addr;
   socklen_t addr_len = sizeof(struct sockaddr_in);

   SdChar *pszIP = NULL;
   SdUShort usPort = 0;
   SdInt iRet;
   SdInt iRxLen = 0; /* 接收长度 */
   SdInt iRetryTimes = 0; /* 重试次数 */

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func_TCPdata_recv running...");

   if((pszBuf == NULL) ||(iLen <= 0))
      return -1;

   if(getpeername(iSock,(struct sockaddr *)&sock_addr, &addr_len) != 0)
   {
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "[%s:%d]: getpeername error.",  __FILE__, __LINE__);
      return -1;
   }
   pszIP = inet_ntoa(sock_addr.sin_addr);
   usPort = ntohs(sock_addr.sin_port);

   memset((void *)pszBuf, 0, iLen); /* 清空接收缓冲区 */

   do
   {
      iRet = recv(iSock, pszBuf + iRxLen, iLen - iRxLen, 0);
      if(iRet == -1) /* 接收出错 */
      {
         if((errno == EAGAIN) ||(errno == EWOULDBLOCK) ||(errno == EINTR)) /* 没有数据啊.. */
         { /* EAGAIN == EWOULDBLOCK, 数据不足 , EINTR = 由于信号中断, 返回 */
            usleep(1000); /* 1milli后重试接收 */
            if(++iRetryTimes < iMilliTimeOut)
               continue;
            else /* 接收超时 */
            {
               UT_LOG_LOGOUT_INFO(emModCOM, 0, "Recv TCP data from (%s:%d) timeout (%d) seconds.", pszIP, usPort, iMilliTimeOut / 1000);
               return 1;
            }
         }
         UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: Recv TCP data from (%s:%d) failure.", __FILE__, __LINE__, pszIP, usPort);
         return -2; /* 未知错误, 直接退出 */
      }
      else if(iRet == 0) /* 链接已经断开 */
      {
         UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: (%s:%d) not connected.", __FILE__, __LINE__, pszIP, usPort);
         return -1;
      }
      else /* 收到数据 */
      {
         iRxLen += iRet;
         pszBuf[iRxLen] = '\0';
         UT_LOG_LOGOUT_INFO(emModCOM, 0, "Recv TCP data: len = (%d) from (%s:%d).", iRxLen, pszIP, usPort);

      }
   }while(iRxLen < iLen);
   return 0;
}


/**
* \fn      SdInt com_func_NETmsg_recv(SdInt iSock, OUT SdChar **ppszRxBuf, OUT SdInt *piRxLen, OUT LPProtocolHeader pstHeader, SdInt iMilliTimeOut)
* \brief   接收网络消息(会检测包头)
* \param   SdInt iSock - Scket句柄
* \param   OUT SdChar **ppszRxBuf -接收缓冲区, malloc分配
* \param   OUT SdInt *piRxLen - 接收到的数据长度
* \param   SdInt iMilliTimeOut - 毫秒接收超时时间
* \return  成功, 返回(0)
* \return  失败, 返回(-1)
* \return  超时, 返回(1)
* \return  未知, 返回(-2)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_func_NETmsg_recv(SdInt iSock, OUT SdChar **ppszRxBuf, OUT SdInt *piRxLen, SdInt iMilliTimeOut)
{
   SdChar *pszRxBuf = *ppszRxBuf, *pszBuf = NULL;
   SdInt iRxLen = 0;
   SdInt iRet = 0;
   ProtocolHeader stProtocolHeader;

   *piRxLen = 0;
   if(pszRxBuf != SD_NULL)
   {
      ut_mem_free(pszRxBuf);
      *ppszRxBuf = SD_NULL;
   }

   iRxLen = sizeof(ProtocolHeader) ;
   pszRxBuf = ut_mem_malloc(iRxLen + 1);
   if(pszRxBuf == SD_NULL)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_mem_malloc error.", __FILE__, __LINE__);
      return -1;
   }memset(pszRxBuf, 0, iRxLen);

   *ppszRxBuf = pszRxBuf; /* 返回malloc缓冲区指针 */

   iRet = com_func_TCPdata_recv(iSock, pszRxBuf, iRxLen, iMilliTimeOut);
   if(iRet != 0)
      return iRet;

   {
      SdChar szRead[2] = {0};
      do{
         com_func_protocol_header_conver(pszRxBuf, &stProtocolHeader); /* 协议头字节序列转换 */
         if(stProtocolHeader.iMagic == iHeaderMagic) /* 定位协议头 */
         {
            iRxLen = stProtocolHeader.iLen;
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "Recv iMagic = (0x%X), len = (0x%X).", iHeaderMagic, stProtocolHeader.iLen);
            break;
         }
         iRet = com_func_TCPdata_recv(iSock, szRead, 1, iMilliTimeOut);
         if(iRet == 0) /* 数据移位, 定位协议头 */
         {
            memcpy(pszRxBuf, pszRxBuf + 1, iRxLen);
            pszRxBuf[iRxLen - 2] = szRead[0];
         }
         else /* 接收出错 */
         {   
            return iRet;
         }
      }while(1);
   }

   if(iRxLen == 0)
      return -1;

   ut_mem_free(pszRxBuf);
   *ppszRxBuf = SD_NULL;
   pszRxBuf = ut_mem_malloc(iRxLen + 1);
   if(pszRxBuf == NULL)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_mem_malloc error.", __FILE__, __LINE__);   
      return -1;
   }memset(pszRxBuf, 0, iRxLen);

   *ppszRxBuf = pszRxBuf; /* 返回malloc缓冲区指针 */

   iRet = com_func_TCPdata_recv(iSock, pszRxBuf, iRxLen, iMilliTimeOut);
   if(iRet != 0)
      return iRet;

   *piRxLen = iRxLen; /* 返回数据长度 */

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Recv TCP data: (%s), len = (%d).", pszRxBuf, iRxLen);

   return 0;
}


/**
* \fn      SdInt com_func_pathfile_path_get(IN SdChar *pszPathFile, OUT SdChar *pszPathBuf, SdInt iPathBufLen)
* \brief   /path/file.type 提取/path/
* \param   IN SdChar *pszPathFile - /path/file.type 缓冲区
* \param   OUT SdChar *pszPathBuf - /path存放
* \param   SdInt iPathBufLen - 存放/path 的缓冲区长度
* \return  成功, 返回(0)
* \return  失败, 返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_func_pathfile_path_get(IN SdChar *pszPathFile, OUT SdChar *pszPathBuf, SdInt iPathBufLen)
{
   SdInt iPathFileLen = 0;
   SdChar *pszBuf = SD_NULL;

   /* 路径缓冲区合法性检验 */
   if(pszPathFile == SD_NULL)
      return -1;
   iPathFileLen = strlen(pszPathFile);
   /* 路径数据合法性检验 */
   if((iPathFileLen == 0) || (iPathFileLen > iPathBufLen))
      return -1;
   /* 名字缓冲区合法性检验 */
   if(pszPathBuf == SD_NULL)
      return -1;

   strcpy(pszPathBuf, pszPathFile);

   pszBuf = strrchr(pszPathBuf, '/');
   if(pszBuf == SD_NULL){
      *pszPathBuf = '\0';
      return -1;
   }

   pszBuf[1] = '\0';

   return -1;
}

/**
* \fn      SdInt com_func_pathfile_file_get(IN SdChar *pszPathFile, OUT SdChar *pszFileBuf, SdInt iFileBufLen)
* \brief   /path/file.type 提取file.type
* \param   IN SdChar *pszPathFile - /path/file.type 缓冲区
* \param   OUT SdChar *pszFileBuf - file.type存放
* \param   SdInt iFileBufLen - 存放file.type的缓冲区长度
* \return  成功, 返回(0)
* \return  失败, 返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_func_pathfile_file_get(IN SdChar *pszPathFile, OUT SdChar *pszFileBuf, SdInt iFileBufLen)
{
   SdInt iPathFileLen =0, iFileLen = 0;
   SdChar *pszFile = SD_NULL;

   /* 路径文件缓冲区合法性检验 */
   if(pszPathFile == SD_NULL)
      return -1;
   iPathFileLen = strlen(pszPathFile);
   /* 路径文件数据合法性检验 */
   if(iPathFileLen == 0)
      return -1;
   /* 文件缓冲区合法性检验 */
   if(pszFileBuf == SD_NULL)
      return -1;

   pszFile = strrchr(pszPathFile, '/');
   if(pszFile == SD_NULL)
      return -1;

   iFileLen = strlen(pszFile);
   if((iFileLen > iFileBufLen) || (iFileLen == 0))
      return -1;

   strcpy(pszFileBuf, pszFile + 1);
   return 0;
}

/**
* \fn       SdInt com_func_file_type_get(IN SdChar *pszFile, OUT SdChar *pszTypeBuf, SdInt iTypeBufLen)
* \brief    file.type 提取type
* \param   IN SdChar *pszFile - file.type 缓冲区
* \param   OUT SdChar *pszTypeBuf - type存放
* \param   SdInt iTypeBufLen - 存放type的缓冲区长度
* \return  成功, 返回(0)
* \return  失败, 返回(-1)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_func_file_type_get(IN SdChar *pszFile, OUT SdChar *pszTypeBuf, SdInt iTypeBufLen)
{
   SdInt iFileLen = 0, iTypeLen = 0;
   SdChar *pszType = SD_NULL;

   /* 文件缓冲区合法性检验 */
   if(pszFile == SD_NULL)
      return -1;
   iFileLen = strlen(pszFile);
   /* 文件数据合法性检验 */
   if(iFileLen == 0)
      return -1;
   /* 类型缓冲区合法性检验 */
   if(pszTypeBuf == SD_NULL)
      return -1;

   pszType = strchr(pszFile, '.');
   if(pszType == SD_NULL)
      return -1;

   iTypeLen = strlen(pszType);
   if((iTypeLen > iTypeBufLen) || (iTypeLen == 0))
      return -1;

   strcpy(pszTypeBuf, pszType + 1);
   return 0;
}


SdInt com_func_filesize_get(SdChar *pszPath)
{
   SdInt iFileSize = 0;
   FILE *fp = SD_NULL;
   fp = fopen(pszPath, "r");
   if(fp == SD_NULL)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: file (%s) not exist.", __FILE__, __LINE__, pszPath);
      return 0;
   }
   fseek(fp, 0, SEEK_END);
   iFileSize = ftell(fp);
   fclose(fp);
   return iFileSize;
}


/**
* \fn       SdInt com_func_file_line_get(FILE * fp)
* \brief    获取文件行数
* \param   FILE * fp - 文件句柄, 此文件必须打开
* \return  文件行数
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_func_file_line_get(FILE *fp)
{
   SdInt iLine = 0;
   SdInt iC;

   rewind(fp); /* 定位到文件头 */

   while((iC = fgetc(fp)) != EOF)
      if(iC == '\n')
         iLine ++;
   return iLine;
}

/**
* \fn       SdInt com_func_areainfo_savepath_get(enDataTypeDef emDataTypeDef, SdChar *pszFile, SdChar *pszPath)
* \brief    物业信息附件保存路径
* \param   enDataTypeDef emDataTypeDef - 物业信息类型(广告附件单独存放)
* \param   IN SdChar *pszFile - 文件名字(用于获取文件类型, 进行文件夹分类)
* \param   OUT SdChar *pszPath - 保存路径
* \return  成功, 返回(0)
* \note
* \todo
* \version V1.0
* \warning 必须保证参数的合法性
*/
SdInt com_func_areainfo_savepath_get(enDataTypeDef emDataTypeDef, IN SdChar *pszFile, OUT SdChar *pszPath)
{
   SdChar szType[16] = {0};

   com_func_file_type_get(pszFile, szType, sizeof(szType));

   if(emDataTypeDef == emAdMsg) /* 广告信息 */
   {
      if(!strcasecmp(szType, "txt"))
          strcpy(pszPath, PATH_DATA_AD_TXT); /* 文本文件 */
      else if((!strcasecmp(szType, "pcm")) ||(!strcasecmp(szType, "mp3")) ||(!strcasecmp(szType, "wav")))
          strcpy(pszPath, PATH_DATA_AD_SOUND); /* 音频文件 */
      else if((!strcasecmp(szType, "mp4")) ||(!strcasecmp(szType, "rmvb")) ||(!strcasecmp(szType, "avi")))
          strcpy(pszPath, PATH_DATA_AD_VIEDEO); /* 视频文件 */
      else if((!strcasecmp(szType, "jpg")) ||(!strcasecmp(szType, "jpeg")) ||(!strcasecmp(szType, "bmp")) ||(!strcasecmp(szType, "png")))
          strcpy(pszPath, PATH_DATA_AD_PICTURE); /* 图片文件 */
      else
          strcpy(pszPath, PATH_DATA_AD_OTHER); /* 其他文件 */
   }
   else /* 小区信息 */
   {
      if(!strcasecmp(szType, "txt"))
          strcpy(pszPath, PATH_DATA_AREAINFO_TXT); /* 文本文件 */
      else if((!strcasecmp(szType, "pcm")) ||(!strcasecmp(szType, "mp3")) ||(!strcasecmp(szType, "wav")))
          strcpy(pszPath, PATH_DATA_AREAINFO_SOUND); /* 音频文件 */
      else if((!strcasecmp(szType, "mp4")) ||(!strcasecmp(szType, "rmvb")) ||(!strcasecmp(szType, "avi")))
          strcpy(pszPath, PATH_DATA_AREAINFO_VIDEO); /* 视频文件 */
      else if((!strcasecmp(szType, "jpg")) ||(!strcasecmp(szType, "jpeg")) ||(!strcasecmp(szType, "bmp")) ||(!strcasecmp(szType, "png")))
          strcpy(pszPath, PATH_DATA_AREAINFO_PICTURE); /* 图片文件 */
      else
          strcpy(pszPath, PATH_DATA_AREAINFO_OTHER); /* 其他文件 */
   }
   return 0;
}


const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+//";

char *com_func_base64_encode( const unsigned char * bindata, char * base64, int binlength )
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}

int com_func_base64_decode( const char * base64, unsigned char * bindata )
{
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+3] )
                temp[3]= k;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) | ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        if ( base64[i+2] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) | ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) | ((unsigned char)(temp[3]&0x3F));
    }
    return j;
}
#endif
