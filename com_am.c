#include "com_am.h"
#include <stdio.h>  
#include <stdlib.h> 
#include <errno.h>
#include <string.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <net/if.h>
#if 1
#define AM_TCP_PORT 1234
#define AM_UDP_PORT 65535
static const SdChar szBigen[2] = {0xaa,0xaa};
static const SdChar szEnd[2] = {0x55,0x55};

typedef struct TagAmObjDef
{
    SdInt    iServerTcp;
    SdInt    iClientTcp;
    SdInt    iUdp;
    SdBool   bRun;
    SdBool   bClient;
    UtMutex* pMutex;
    UtThread utThread_tcp;
    UtThread utThread_client;
    UtThread utThread_udp;
}AmObjDef,*LPAmObjDef;

static AmObjDef g_stAmObj = {
                               .iServerTcp = -1,
                               .iClientTcp = -1,
                               .iUdp = -1,
                               .bRun = SD_FALSE,
                               .bClient = SD_FALSE,
                               .pMutex = SD_NULL,
                               .utThread_tcp = SD_NULL,
                               .utThread_client = SD_NULL,
                               .utThread_udp = SD_NULL
                            };

LPAmObjDef com_am_object()
{
    return &g_stAmObj;
}

SdUShort com_crc16_byte(SdUShort crc, const SdUShort data)
{
    return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
}


SdUShort com_crc16i(SdUShort crc, SdUChar const *buffer, size_t len)
{
    while (len--)
    {
            crc = com_crc16_byte(crc, *buffer++);
    }
    return crc;
}

SdUShort com_am_get_crc16(const SdUChar *buffer, size_t len)
{
    return com_crc16i(0,buffer,len);
}

SdInt com_am_tcp_create()
{
    SdInt sockfd;
    struct timeval timeout={1,0};
    sockfd = socket(AF_INET,SOCK_STREAM,0); //建立一个序列化的，可靠的，双向连接的的字节流  
    if(sockfd<0)  
    {  
        UT_LOG_LOGOUT_ERROR(emModCOM, 5,"tcp socket create");  
        return -1;  
    }
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    return sockfd;
}

SdInt com_am_udp_create()
{ 
    SdInt sockfd;
    SdInt opt = 1;
    struct timeval timeout={1,0};
    sockfd=socket(AF_INET,SOCK_DGRAM,0);//IPV4  SOCK_DGRAM 数据报套接字（UDP协议）
    if(sockfd < 0)
    {
        UT_LOG_LOGOUT_ERROR(emModCOM, 5,"udp socket create"); 
        return -1;
    }
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
    return sockfd;
}

SdInt com_udp_ip_info_send(SdInt iSock, struct sockaddr_in * psin, SdChar* pData, SdInt iLen)
{
    socklen_t sin_len = 0;
    struct sockaddr_in server_addr;
    SdInt isize = 0;
    struct sockaddr_in * pstSin;
    struct ifreq    ifr;
    SdChar szHeadPor[11] = {0};
    SdUShort usCrc16 = 0;
    SdChar szLen[2] = {0};
    SdChar szTagEnd[4] = {0x00,0x00,0x55,0x55};
    SdChar szBuff[1024] = {0};
    SdChar szData[1024] = {0};
    SdChar szTmp[1024] = {0};
    SdChar* szPCCmd = "CallSecurityDevice";
    SdChar* szSN = "SN=0000000012";
    SdUChar* pszCh = SD_NULL;
    SdChar  hd[6] = {0};
    char *name = "eth0";
    SdChar* szCmd = "route add -host 255.255.255.255 dev eth0";
    memcpy( ifr.ifr_name, name,strlen(name));
    memcpy(szHeadPor,pData,sizeof(szHeadPor));
    //usCrc16 = com_am_get_crc16(pData+2,iLen-2-4);
    //memcpy(szTagEnd,&usCrc16,2);
    //UT_LOG_LOGOUT_DEBUG(emModCOM,5,"%2X %2X",szTagEnd[0],szTagEnd[1]);
    if(szHeadPor[8] != 0x14)
    {
        UT_LOG_LOGOUT_DEBUG(emModCOM, 5, "CallSecurityDevice cmd is not");
        return SD_FAIL;
    }
    if(0 != memcmp(pData+sizeof(szHeadPor),szPCCmd,strlen(szPCCmd)))
    {
        UT_LOG_LOGOUT_DEBUG(emModCOM, 5, "AM PC send is not %s", szPCCmd);
        return SD_FAIL;
    }
    system(szCmd);
    memset(szHeadPor,0,sizeof(szHeadPor));
    szHeadPor[0] = 0xaa;
    szHeadPor[1] = 0xaa;
    szHeadPor[8] = 0xc8;
    if(ioctl(iSock,SIOCGIFHWADDR,&ifr, sizeof(struct ifreq))<0)
    {
            //UT_LOG_LOGOUT_ERROR(emModCOM,5,"get MAC");
            snprintf(szData,sizeof(szData),"%s,MAC=,",szSN);
            memcpy(szTmp,szData,strlen(szData));
    }
    else
    {
        memcpy( hd, ifr.ifr_hwaddr.sa_data, sizeof(hd));
        //UT_LOG_LOGOUT_DEBUG(emModCOM,5,"MAC=%02X:%02X:%02X:%02X:%02X:%02X",hd[0],hd[1],hd[2],hd[3],hd[4],hd[5]);
        snprintf(szData,sizeof(szData),"%s,MAC=%02X:%02X:%02X:%02X:%02X:%02X,",szSN,hd[0],hd[1],hd[2],hd[3],hd[4],hd[5]);
        memcpy(szTmp,szData,strlen(szData));
    }
    if((ioctl(iSock, SIOCGIFADDR, &ifr))<0)
    {
        UT_LOG_LOGOUT_ERROR(emModCOM,5,"get ip address");
        return SD_FAIL;
    }
    else
    {
       pstSin = (struct sockaddr_in *)&(ifr.ifr_addr); 
       //UT_LOG_LOGOUT_DEBUG(emModCOM,5,"IPAddress=%s",(SdChar*)inet_ntoa(pstSin->sin_addr));
       snprintf(szData,sizeof(szData),"%sIPAddress=%s,GateWay=0.0.0.0,",szTmp,(SdChar*)inet_ntoa(pstSin->sin_addr));
       memcpy(szTmp,szData,strlen(szData)); 
    }
    if((ioctl(iSock, SIOCGIFNETMASK, &ifr, sizeof(struct ifreq)))<0)
    {
        //UT_LOG_LOGOUT_ERROR(emModCOM,5,"get netmask");
        snprintf(szData,sizeof(szData),"%sNetMask=,",szTmp);
    }
    else
    {
        pstSin = (struct sockaddr_in *)&(ifr.ifr_addr);
        //UT_LOG_LOGOUT_DEBUG(emModCOM,5,"NetMask=%s",(SdChar*)inet_ntoa(pstSin->sin_addr));
        snprintf(szData,sizeof(szData),"%sNetMask=%s",szTmp,(SdChar*)inet_ntoa(pstSin->sin_addr));
    }
    UT_STOC2_LITTLE(strlen(szData),szLen);
    szHeadPor[9] = szLen[0];
    szHeadPor[10] = szLen[1];
    memcpy(szBuff,szHeadPor,sizeof(szHeadPor));
    iLen = sizeof(szHeadPor);
    pszCh = szBuff;
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"%s",szData);
    memcpy(pszCh+iLen,szData,strlen(szData));
    iLen = iLen + strlen(szData);
    pszCh = szBuff + 2;
    usCrc16 = com_am_get_crc16(pszCh,iLen-2);
    memcpy(szTagEnd,&usCrc16,2);
    pszCh = szBuff;
    memcpy(pszCh+iLen,szTagEnd,sizeof(szTagEnd));
    iLen = iLen + sizeof(szTagEnd);
    //psin->sin_port=htons(65535);
    psin->sin_addr.s_addr = inet_addr("255.255.255.255");
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"iLen %d iP:%s port:%d",iLen,(SdChar*)inet_ntoa(psin->sin_addr),ntohs(psin->sin_port));
    sin_len = sizeof(struct sockaddr);
    #if 0
    SdInt opt = 1;
    SdInt sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
    struct sockaddr_in addrto;  
    bzero(&addrto, sizeof(struct sockaddr_in));  
    addrto.sin_family=AF_INET;  
    //addrto.sin_addr.s_addr=htonl(INADDR_BROADCAST);  
    addrto.sin_addr.s_addr = inet_addr("255.255.255.255");
    addrto.sin_port=psin->sin_port;  
    int nlen=sizeof(addrto);  
    sleep(1);
    isize = sendto(sock,szBuff,iLen,0,(struct sockaddr *)&addrto,nlen);
    close(sock);
    #else
    isize = sendto(iSock,szBuff,iLen,0,(struct sockaddr *)psin,sin_len);
    #endif
    if(isize <= 0)
    {
        UT_LOG_LOGOUT_ERROR(emModCOM,5,"udp send fail");
    }
}

void* com_udp_func(void* ptr)
{
    socklen_t sin_len = 0;
    SdInt iRev = 0;
    SdChar szBuff[1024] = {0};
    SdChar szLen[2] = {0};
    SdChar * pRead = szBuff;
    SdChar * pWrite = szBuff;
    SdChar * szTagEnd = SD_NULL;
    SdInt iLenWrite = sizeof(szBuff);
    SdInt iLenData = 0;
    SdInt iLenPacket = 0;
    struct sockaddr_in sin; 
    SdChar szHeadPor[11] = {0};
    SdChar szEndPor[4] = {0};
    LPAmObjDef pstAmObj = com_am_object();
    if(SD_NULL != pstAmObj)
    {
        bzero(&sin,sizeof(sin));  
        sin.sin_family=AF_INET;  
        sin.sin_addr.s_addr=htonl(INADDR_ANY);
        sin.sin_port=htons(AM_UDP_PORT);  
        sin_len=sizeof(sin);  
        bind(pstAmObj->iUdp,(struct sockaddr *)&sin,sizeof(sin));
        while(pstAmObj->bRun)
        {
            iRev = recvfrom(pstAmObj->iUdp, szBuff, iLenWrite,0,(struct sockaddr *)&sin,&sin_len);
            if(iRev <= 0)
            {
                if(errno ==  EWOULDBLOCK || errno == EAGAIN)
                {
                    UT_LOG_LOGOUT_DEBUG(emModCOM, 5, "udp recv timeout");
                    continue;
                }
                else
                {
                    UT_LOG_LOGOUT_ERROR(emModCOM, 5, "udp recv error %d",iRev);
                    close(pstAmObj->iUdp);
                    pstAmObj->iUdp = com_am_udp_create();
                    bzero(&sin,sizeof(sin));  
                    sin.sin_family=AF_INET;  
                    sin.sin_addr.s_addr=htonl(INADDR_ANY);  
                    sin.sin_port=htons(AM_UDP_PORT);  
                    sin_len=sizeof(sin);  
                    bind(pstAmObj->iUdp,(struct sockaddr *)&sin,sizeof(sin));
                }
            }
            else
            {
                UT_LOG_LOGOUT_DEBUG(emModCOM, 5, "udp recv %d",iRev);
                iLenData += iRev;
                while(iLenData > 0)
                {
                    if(iLenData > sizeof(szHeadPor))
                    {
                        memcpy(szHeadPor,pRead,sizeof(szHeadPor));
                        if(0 == memcmp(szHeadPor,szBigen,sizeof(szBigen)))
                        {
                            memcpy(szLen,szHeadPor+9,sizeof(szLen));
                            SdShort sLen = UT_LITTLE_C2TOS(szLen);
                            iLenPacket = sLen+sizeof(szHeadPor)+sizeof(szEndPor);
                            if( iLenPacket <= iLenData)
                            {
                                szTagEnd = pRead + iLenPacket - sizeof(szEnd);
                                if( 0 == memcmp(szTagEnd,szEnd,sizeof(szEnd)))
                                {
                                    com_udp_ip_info_send(pstAmObj->iUdp,&sin, pRead, iLenPacket);
                                }
                                else
                                {
                                    UT_LOG_LOGOUT_DEBUG(emModCOM, 5, "PC AM protcol end is error");
                                    goto setp1;
                                }
                                iLenData = iLenData - (sLen+sizeof(szHeadPor)+sizeof(szEndPor));
                                if(iLenData <= 0)
                                {
                                    iLenData = 0;
                                    pRead = szBuff;
                                    pWrite = szBuff;
                                    iLenWrite = sizeof(szBuff);
                                }
                                else
                                {
                                   memcpy(szBuff,pRead,iLenData);
                                   pRead = szBuff;
                                   pWrite = pRead + iLenData;
                                   iLenWrite = sizeof(szBuff) - iLenData;
                                }
                                break;
                            }
                            else
                            {
                                iLenWrite = sizeof(szBuff) - iLenData;
                                if(iLenWrite > 0)
                                {
                                    memcpy(szBuff,pRead,iLenData);
                                    pRead = szBuff;
                                    pWrite = szBuff + iLenData;
                                    break;
                                }
                            }
                        }
                        setp1:
                        pRead = pRead + 1;
                        iLenData = iLenData - 1;
                    }
                    else 
                    {
                        memcpy(szBuff,pRead,iLenData);
                        pRead = szBuff;
                        pWrite = szBuff + iLenData;
                        iLenWrite = sizeof(szBuff) - iLenData;
                        break;
                    }
                }  
            }
        }
    }
}

void* com_tcp_client_func(void* ptr)
{
    SdChar szBuff[800];
    SdInt iLen = 0;
    SdInt iCount = 0;
    SdInt iOutTime = 1800;
    LPAmObjDef pstAmObj = com_am_object();
    if(SD_NULL == pstAmObj)
    {
        return SD_NULL;
    }
    while(pstAmObj->bClient)
    {
        iLen = recv(pstAmObj->iClientTcp,szBuff,sizeof(szBuff),0);
        if(iLen <= 0)
        {
            if(errno ==  EWOULDBLOCK || errno == EAGAIN)
            {
                UT_LOG_LOGOUT_DEBUG(emModCOM,5,"tcp client recv outtime"); 
                iCount++;
                if(iCount >= iOutTime)
                {
                    if(SD_SUCCESS == ut_mutex_trylock(pstAmObj->pMutex))
                    {
                        pstAmObj->bClient = SD_FALSE;
                        close(pstAmObj->iClientTcp);
                        pstAmObj->iClientTcp = -1;
                        pstAmObj->utThread_client = SD_NULL;
                        ut_mutex_unlock(pstAmObj->pMutex);
                        break;
                    }
                }
            }
            else
            {
                UT_LOG_LOGOUT_ERROR(emModCOM,5,"tcp client recv");
                if(SD_SUCCESS == ut_mutex_trylock(pstAmObj->pMutex))
                {
                    pstAmObj->bClient = SD_FALSE;
                    close(pstAmObj->iClientTcp);
                    pstAmObj->iClientTcp = -1;
                    pstAmObj->utThread_client = SD_NULL;
                    ut_mutex_unlock(pstAmObj->pMutex);
                    break;
                }
            }
        }
        else
        {
            UT_LOG_LOGOUT_DEBUG(emModCOM,5,"send COM_HW_AM_DATA_DOWN");
            ut_msg_send(emModCOM, emModHW, COM_HW_AM_DATA_DOWN, 0, 0, 0, iLen, szBuff);
        }
    }
    ut_thread_exit();
}

void* com_tcp_func(void* ptr)
{
    SdInt client = -1;
    struct timeval timeout={1,0};
    struct sockaddr_in server_addr; //存储服务器端socket地址结构  
    struct sockaddr_in client_addr; //存储客户端 socket地址结构  
    int err;    //返回值  
    socklen_t addrlen = sizeof(client_addr);  
    LPAmObjDef pstAmObj = com_am_object();
    if(SD_NULL == pstAmObj)
    {
        return SD_NULL;
    }
    memset(&server_addr,0,sizeof(server_addr));  
    server_addr.sin_family = AF_INET;           //协议族  
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   //本地地址  
    server_addr.sin_port = htons(AM_TCP_PORT);  
    err = bind(pstAmObj->iServerTcp,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));  
    if(err<0)  
    {  
        UT_LOG_LOGOUT_ERROR(emModCOM,5,"tcp bind error");  
        return SD_NULL;  
    }  
    err = listen(pstAmObj->iServerTcp,5);   //设置监听的队列大小  
    if(err < 0)  
    {  
        UT_LOG_LOGOUT_ERROR(emModCOM,5,"tcp listen error");  
        return SD_NULL;  
    }  
    while(pstAmObj->bRun)  
    {  
        client = accept(pstAmObj->iServerTcp,(struct sockaddr *)&client_addr,&addrlen);  //注，此处为了获取返回值使用 指针做参数  
        if(client < 0)
        {
            if(errno ==  EWOULDBLOCK || errno == EAGAIN)
            {
                UT_LOG_LOGOUT_DEBUG(emModCOM,5,"tcp accept outtime");  
                continue;
            }
            else
            {
                UT_LOG_LOGOUT_ERROR(emModCOM,5,"tcp accept");
                continue;
            }
        }
        else
        {
            ut_mutex_lock(pstAmObj->pMutex);
            if(pstAmObj->bClient == SD_TRUE)
            {
                pstAmObj->bClient = SD_FALSE;
                if(SD_NULL != pstAmObj->utThread_client)
                {
                    ut_thread_join(pstAmObj->utThread_client);
                }
                close(pstAmObj->iClientTcp);
                pstAmObj->iClientTcp = -1;
            }
            ut_mutex_unlock(pstAmObj->pMutex);
            pstAmObj->iClientTcp = client;
            setsockopt(pstAmObj->iClientTcp,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
            pstAmObj->bClient = SD_TRUE;
            ut_thread_create(&(pstAmObj->utThread_client),UT_THREAD_STACK_SIZE_DEFAULT,
                            UT_THREAD_PRIORITY_DEFAULT,com_tcp_client_func,SD_NULL);
          
        }
    }  
}

SdInt com_am_start()
{
    LPAmObjDef pstAmObj = com_am_object();
    if(SD_FALSE == pstAmObj->bRun)
    {
        pstAmObj->bRun = SD_TRUE;
        pstAmObj->iServerTcp = com_am_tcp_create();
        pstAmObj->iUdp = com_am_udp_create();
        if(SD_NULL == pstAmObj->pMutex)
        {
            pstAmObj->pMutex = ut_mutex_create();
        }
        if(SD_NULL == pstAmObj->utThread_tcp)
        {
            ut_thread_create(&(pstAmObj->utThread_tcp), UT_THREAD_STACK_SIZE_DEFAULT, 
                                UT_THREAD_PRIORITY_DEFAULT, com_tcp_func, SD_NULL);
        }
        if(SD_NULL == pstAmObj->utThread_udp)
        {
            ut_thread_create(&(pstAmObj->utThread_udp), UT_THREAD_STACK_SIZE_DEFAULT, 
                                UT_THREAD_PRIORITY_DEFAULT, com_udp_func, SD_NULL);
        }
    }
}
SdInt com_am_stop()
{
    LPAmObjDef pstAmObj = com_am_object();
    if(SD_TRUE == pstAmObj->bRun)
    {
        pstAmObj->bRun = SD_FALSE;
        if(SD_NULL != pstAmObj->utThread_tcp)
        {
            ut_thread_join(pstAmObj->utThread_tcp);
            pstAmObj->utThread_tcp = SD_NULL;
        }
        ut_mutex_lock(pstAmObj->pMutex);
        if(pstAmObj->bClient == SD_TRUE)
        {
            pstAmObj->bClient = SD_FALSE;
            if(SD_NULL != pstAmObj->utThread_client)
            {
                    ut_thread_join(pstAmObj->utThread_client);
            }
            close(pstAmObj->iClientTcp);
            pstAmObj->iClientTcp = -1;
        }
        ut_mutex_unlock(pstAmObj->pMutex);
        if(SD_NULL != pstAmObj->utThread_udp)
        {
            ut_thread_join(pstAmObj->utThread_udp);
            pstAmObj->utThread_udp = SD_NULL;
        }
        if(pstAmObj->iServerTcp > 0)
        {
            close(pstAmObj->iServerTcp);
            pstAmObj->iServerTcp = -1;
        }
        if(pstAmObj->iClientTcp > 0)
        {
            close(pstAmObj->iClientTcp);
            pstAmObj->iClientTcp = 0;
        }
        if(pstAmObj->iUdp > 0)
        {
            close(pstAmObj->iUdp);
            pstAmObj->iUdp = -1;
        }
        if(SD_NULL != pstAmObj->pMutex)
        {
            ut_mutex_destroy(pstAmObj->pMutex);
            pstAmObj->pMutex = SD_NULL;
        }
        return SD_SUCCESS;
    }
    return SD_FAIL;
}

SdInt com_am_tcp_send(void* pData, SdInt iLen)
{
    LPAmObjDef pstAmObj = com_am_object();
    if(SD_TRUE == pstAmObj->bRun && -1 != pstAmObj->iClientTcp)
    {
        return send(pstAmObj->iClientTcp, pData, iLen, 0);
    }
    UT_LOG_LOGOUT_DEBUG(emModCOM,5,"tcp send fail");
    return SD_FAIL;
}
#endif
