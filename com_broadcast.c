#include "com_broadcast.h"
#include "com_json.h"
#include "com_ctrl.h"
#include <ut_all.h>



static s32 _com_broadcast_send_lift_info(const void * arg, SdInt iLen);

typedef enum enum_bcst_sta
{
	BCST_STA_BEGIN,
	BCST_STA_DIED,
	BCST_STA_STOP,
	BCST_STA_START,
	BCST_STA_END
}en_bcst_sta;

typedef struct struct_bcst_func
{
	SdInt iIdx;
	en_bcst_cmd enCmd;
	s32 (*handle)(const void *arg, SdInt iLen);
}st_bcst_func;
static st_bcst_func gs_cmd_func[] = {
		{ 0,  enCmdBegin,    NULL },
		{ 1,  enCmdLiftInfo, _com_broadcast_send_lift_info },
		{ -1, enCmdEnd,      NULL }
	};

typedef struct struct_bcst_cfg
{
	SdInt iSockfd;
	SdInt iSta;
	struct sockaddr_in st_addr_to;
	st_bcst_func * st_cmd_func;
}st_bcst_cfg, * pst_bcst_cfg;
static st_bcst_cfg gs_bcst_cfg_st = {
								.iSockfd = -1,
								.iSta = BCST_STA_DIED,
								.st_cmd_func = gs_cmd_func
								};

#define _com_bcst_set_sta(_s) do{\
	gs_bcst_cfg_st.iSta = (_s > BCST_STA_BEGIN && _s < BCST_STA_END) ? _s : BCST_STA_DIED;\
	}while(0)
#define _com_bcst_set_sta_died()  _com_bcst_set_sta(BCST_STA_DIED)
#define _com_bcst_set_sta_stop()  _com_bcst_set_sta(BCST_STA_STOP)
#define _com_bcst_set_sta_start() _com_bcst_set_sta(BCST_STA_START)

#define _com_bcst_is_sta(_s) (gs_bcst_cfg_st.iSta == _s ? SD_TRUE : SD_FALSE)
#define _com_bcst_is_died()  _com_bcst_is_sta(BCST_STA_DIED)
#define _com_bcst_is_stop()  _com_bcst_is_sta(BCST_STA_STOP)
#define _com_bcst_is_start() _com_bcst_is_sta(BCST_STA_START)

#define _com_bcst_cmd_is_valid(_cmd) ((_cmd > enCmdBegin && _cmd < enCmdEnd) ? SD_TRUE : SD_FALSE)

#define _closefd(_fd) do{\
						if(_fd > 0)\
						{\
							close(_fd);\
							_fd = -1;\
						}\
					}while(0)
#define _free(_p) do{\
					if(_p)\
					{\
						free(_p);\
						_p = NULL;\
					}\
				}while(0)

static pst_bcst_cfg _com_broadcast_cfg_get()
{
	return &gs_bcst_cfg_st;
}

static char * _com_bcst_get_dst_addr(int iSockfd)
{
	char * sDst = (char *)calloc(16, 1);
	char szIp[16] = {0};
	char szMsk[16] = {0};
	char * name = "eth0";
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(struct ifreq));
	memcpy(ifr.ifr_name, name, strlen(name));
	int iRet = 0;
	struct sockaddr_in * pSin = NULL;
	struct in_addr stIp, stMsk, stDst;
	unsigned long ulIp, ulMsk, ulDst;
	if(sDst && iSockfd > 0)
	{
		iRet = ioctl(iSockfd, SIOCGIFADDR, &ifr);
		if(iRet < 0)
		{
			_free(sDst);
		}else
		{
			pSin = (struct sockaddr_in *)&(ifr.ifr_addr);
			snprintf(szIp, 16, "%s", (SdChar*)inet_ntoa(pSin->sin_addr));
			
			memset(&ifr, 0, sizeof(struct ifreq));
			memcpy(ifr.ifr_name, name, strlen(name));
			iRet = ioctl(iSockfd, SIOCGIFNETMASK, &ifr, sizeof(struct ifreq));
			if(iRet < 0)
			{
				snprintf(sDst, 16, "255.255.255.255");
			}else
			{
				pSin = (struct sockaddr_in *)&(ifr.ifr_addr);
				snprintf(szMsk, 16, "%s", (SdChar*)inet_ntoa(pSin->sin_addr));
				UT_LOG_LOGOUT_DEBUG(emModCOM, 0, "ip: %s msk: %s", szIp, szMsk);

				iRet = inet_aton(szIp, &stIp);
				if(-1 != iRet)
				{
					iRet = inet_aton(szMsk, &stMsk);
					if(-1 != iRet)
					{
						ulIp  = *(unsigned long *)&stIp;
						UT_LOG_LOGOUT_DEBUG(emModCOM, 0, "ip(inet_aton): %ld msk(inet_aton): %ld", ulIp, ulMsk);
						ulMsk = *(unsigned long *)&stMsk;
						ulMsk = ulMsk ^ 0xFFFFFFFF;
						ulDst = ulIp | ulMsk;
						snprintf(sDst, 16, "%s", (SdChar*)inet_ntoa(*(struct in_addr *)&ulDst));
					}else
					{
						_free(sDst);
					}
				}else
				{
					_free(sDst);
				}
			}
		}
	}
	return sDst;
}

s32 com_broadcast_init()
{
	SdInt iSockfd;
    SdInt iOpt = 1;
	s32 iRet = -1, reuse = 1;
    struct timeval timeout={1,0};
	pst_bcst_cfg pstCfg = _com_broadcast_cfg_get();
	if(pstCfg)
	{
		if(SD_TRUE == _com_bcst_is_died())
		{
		    iSockfd=socket(AF_INET,SOCK_DGRAM,0);//IPV4  SOCK_DGRAM 数据报套接字（UDP协议）
		    if(iSockfd < 0)
		    {
		        UT_LOG_LOGOUT_ERROR(emModCOM, 5,"[%s:%d]udp socket create failed", __FUNCTION__, __LINE__);
		    }else
		    {
		    	setsockopt(iSockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
		    	setsockopt(iSockfd, SOL_SOCKET, SO_BROADCAST, (char *)&iOpt, sizeof(iOpt));
				iRet = setsockopt(iSockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
			    if (0 != iRet)
				{
			        close(iSockfd);
			        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): setsockopt()\n", __LINE__);
			        iRet = -1;
			    }else
			    {
				    iRet = setsockopt(iSockfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
				    if(iRet != 0)
					{
				       close(iSockfd);
				       UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_func Error(LINE=%d): setsockopt()\n", __LINE__);
				       iRet = -1;
				    }else
				    {
				    	pstCfg->iSockfd = iSockfd;
						iRet = 0;
						_com_bcst_set_sta_stop();
				    }
			    }
		    }
		}else
		{
			iRet = 0;
		}
	}
	return iRet;
}

s32 com_broadcast_start()
{
	s32 iRet = -1;
	char * szDst = NULL;
	pst_bcst_cfg pstCfg = _com_broadcast_cfg_get();
	LPCOMManage	pstCOMManage = com_ctrl_module_manage_get();
	if(SD_TRUE == _com_bcst_is_stop())
	{
		if(pstCOMManage)
		{
			if(pstCfg)
			{
				bzero(&(pstCfg->st_addr_to), sizeof(pstCfg->st_addr_to));
		        pstCfg->st_addr_to.sin_family = AF_INET;
		        pstCfg->st_addr_to.sin_addr.s_addr = htonl(INADDR_ANY);
				pstCfg->st_addr_to.sin_port = htons(COM_BCST_UDP_PORT);
		        bind(pstCfg->iSockfd, (struct sockaddr *)&(pstCfg->st_addr_to), sizeof(pstCfg->st_addr_to));
				#if 1
				szDst = _com_bcst_get_dst_addr(pstCfg->iSockfd);
				if(szDst)
				{
					UT_LOG_LOGOUT_DEBUG(emModCOM, 0, "broadcast dst ip addr: %s", szDst);
					pstCfg->st_addr_to.sin_addr.s_addr = inet_addr(szDst);
					_com_bcst_set_sta_start();
					iRet = 0;
				}
				_free(szDst);
				#endif
				#if 0
				pstCfg->st_addr_to.sin_addr.s_addr = inet_addr("255.255.255.255");
				_com_bcst_set_sta_start();
				iRet = 0;
				#endif
			}
		}
	}
	return iRet;
}

s32 com_broadcast_stop()
{
	s32 iRet = -1;
	if(SD_TRUE == _com_bcst_is_start())
	{
		_com_bcst_set_sta_stop();
		iRet = 0;
	}
	return iRet;
}

s32 com_broadcast_uninit()
{
	s32 iRet = -1;
	pst_bcst_cfg pstCfg = _com_broadcast_cfg_get();
	if(pstCfg)
	{
		if(SD_TRUE == _com_bcst_is_stop())
		{
			_closefd(pstCfg->iSockfd);
			_com_bcst_set_sta_died();
			iRet = 0;
		}
	}
	return iRet;
}

static s32 _com_broadcast_send_lift_info(const void * arg, SdInt iLen)
{
	s32 iRet = -1;
	LPLiftInfoDef ps_info = NULL;
	s8 * jsonstr = NULL;
	pst_bcst_cfg pstCfg = _com_broadcast_cfg_get();
	socklen_t len = 0;
	int iJsonlen = 0;
	if(pstCfg && iLen == sizeof(LiftInfoDef))
	{
		ps_info = (LPLiftInfoDef)arg;
		iJsonlen = com_json_liftstatus_info(ps_info, &jsonstr);
		if(jsonstr)
		{
			len = (socklen_t)sizeof(pstCfg->st_addr_to);
			if(pstCfg->iSockfd > 0)
			{
				iRet = sendto(pstCfg->iSockfd, jsonstr, iJsonlen, 0, (struct sockaddr *)&(pstCfg->st_addr_to), len);
				if(iRet == iJsonlen)
				{
					iRet = 0;
				}
			}else
			{
				UT_LOG_LOGOUT_DEBUG(emModCOM, 0, "broadcast sockfd <= 0!");
			}
		}else
		{
			UT_LOG_LOGOUT_DEBUG(emModCOM, 0, "gen liftstatus info json failed!");
		}
	}else
	{
		UT_LOG_LOGOUT_DEBUG(emModCOM, 0, "Get broadcast cfg failed or %s's arg invalid!", __FUNCTION__);
	}
	return iRet;
}

s32 com_broadcast_send(en_bcst_cmd cmd, const void * arg, SdInt len)
{
	s32 iRet = -1;
	pst_bcst_cfg pstCfg = _com_broadcast_cfg_get();
	if(pstCfg)
	{
		if(SD_TRUE == _com_bcst_cmd_is_valid(cmd) && SD_TRUE == _com_bcst_is_start())
		{
			iRet = pstCfg->st_cmd_func[cmd].handle(arg, len);
		}
	}
	return iRet;
}

#undef _com_bcst_set_sta_died
#undef _com_bcst_set_sta_stop
#undef _com_bcst_set_sta_start
#undef _com_bcst_set_sta

#undef _com_bcst_is_died
#undef _com_bcst_is_stop
#undef _com_bcst_is_start
#undef _com_bcst_is_sta

#undef _com_bcst_cmd_is_valid

#undef _closefd
#undef _free

