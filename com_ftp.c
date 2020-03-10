/**
 * \file    com_ftp.c
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning 版权所�? 侵权必究
 * \todo    COM模块FTP
 */
#include "com_func.h"
#include <time.h>
#include <stdio.h>  
#include <stdlib.h>  
#include <sys/wait.h>  
#include <sys/types.h>  
#include "json/json.h"
#include "com_ctrl.h"

typedef void (*sighandler_t)(int);
int pox_system(const char *cmd_line)
{
	int ret = 0;
	sighandler_t old_handler;
	old_handler = signal(SIGCHLD, SIG_DFL);
	ret = system(cmd_line);
	signal(SIGCHLD, old_handler); 
	return ret;
}

/************************************************************************
* struct  st_ftp_cfg
*        配置FTP客户�?
************************************************************************/
typedef struct struct_ftp_cfg{
    pthread_t thread;   /* 检测下载列�?*/
    s8        ftp_run;  /* FTP客户端运行标�?*/
	int		  iCountFtpFailDownload;	/* Ftp最多失败下载计�?*/
	pthread_mutex_t mtx_upl;  /* Mutex of writing or reading upload-list-file */
	void (* onUplEnd)(int result, int msgid, void * context, int ctxLen);  /* Function that would be called after upload a file */
}st_ftp_cfg, *pst_ftp_cfg;

static st_ftp_cfg gs_ftp_cfg_st;

/************************************************************************
* @FunctionName( 函数�?): pst_ftp_cfg com_ftp_cfg_get(void)
* @CreateDate  (创建日期): 2019/5/16
* @Author      ( �? �?): CJH
*
* @Description(描述): 返回FTP客户端配�?
*
* @Param(参数):
*        None(�?
*
* @ReturnCode(返回�?:
*        pst_ftp_cfg
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(�?
************************************************************************/
pst_ftp_cfg com_ftp_cfg_get(void){
    return &gs_ftp_cfg_st;
}
/**@END! pst_ftp_cfg com_ftp_cfg_get(void) !\(^o^)/~ 结束�?*/

static s32 com_ftp_struct_to_string(IN void * data, IN int len, OUT char * str, IN OUT int * strLen)
{
	s32 ret = 0;
    char * pstr = NULL;
    unsigned char * pdata = NULL;
    int i = 0, offset = 0;
    if(data && str && strLen && *strLen > 0)
    {
        pstr = str;
        pdata = (unsigned char *)data;
        memset(pstr, 0, *strLen);
        if(len > 0 && *strLen >= (len * 2))
        {
            for(i = 0, offset = 0; i < len; i++, offset += 2)
            {
            	snprintf(pstr + offset, (*strLen - offset), "%02x", pdata[i]);
            }
            *strLen = offset;
        }
    }else
    {
        ret = -1;
    }
    return ret;
}

static s32 com_ftp_string_to_struct(IN char * str, OUT void * data, IN OUT int * len)
{
	s32 ret = 0;
    int strLen = 0;
    int i = 0;
    //short tmp1 = 0, tmp2 = 0;
    char * pdata = NULL;
	unsigned int tmp = 0;
    if(str && data && len)
    {
        strLen = strlen(str);
        pdata = (unsigned char *)data;
        if(0 == strLen % 2)
        {
            if(*len >= (strLen / 2))
            {
                *len = strLen / 2;
                for(i = 0; i < *len; i++)
                {
                    sscanf(str + i * 2, "%02x", &tmp);
					pdata[i] = tmp;
                }
            }
        }else
        {
            ret = -1;
        }
    }else
    {
        ret = -1;
    }
    return ret;
}

static s32 com_ftp_data_to_json(IN int code, IN void * data, IN int len, OUT char * jsonstr, IN OUT int * jsonstrLen)
{
	s32 ret = 0;
    char datastr[2048] = {0};
    int datastrlen = sizeof(datastr);
    json_object * obj = NULL;
    const char * cjsonstr = NULL;
    if(code >= -1 && jsonstr && jsonstrLen)
    {
        obj = json_object_new_object();
        if(obj)
        {
            json_object_object_add(obj, "code", json_object_new_int(code));
            if(code >= 0)
            {
            	if(data)
            	{
	                ret = com_ftp_struct_to_string(data, len, datastr, &datastrlen);
	                if(0 == ret)
	                {
	                    json_object_object_add(obj, "context", json_object_new_string(datastr));
	                }
            	}
            }
            if(0 == ret)
            {
                cjsonstr = json_object_get_string(obj);
                if(*jsonstrLen > 0)
                {
                    memset(jsonstr, 0, *jsonstrLen);
                    snprintf(jsonstr, *jsonstrLen, "%s", cjsonstr);
                    if(*jsonstrLen > strlen(cjsonstr))
                    {
                        *jsonstrLen = strlen(cjsonstr);
                    }else
                    {
                        *jsonstrLen = *jsonstrLen - 1;
                    }
                }else
                {
                    ret = -1;
                }
            }
            json_object_put(obj);
        }else
        {
            ret = -1;
        }
    }
    return ret;
}

static s32 com_ftp_json_to_data(IN char * str, OUT void * data, IN OUT int * len)
{
	s32 ret = -1, result = 0;
    json_object * obj = NULL, * tmp = NULL;
    const char * ctxstr = NULL;
    if(str && data && len)
    {
        obj = json_tokener_parse(str);
        if(obj)
        {
            tmp = json_object_object_get(obj, "code");
            //if(TRUE == json_object_object_get_ex(obj, "code", &tmp))
            if(tmp)
            {
                ret = json_object_get_int(tmp);
                tmp = json_object_object_get(obj, "context");
                //if(TRUE == json_object_object_get_ex(obj, "context", &tmp))
                if(tmp)
                {
                    ctxstr = json_object_get_string(tmp);
                    result = com_ftp_string_to_struct((char *)ctxstr, data, len);
                    if(0 != result)
                    {
                        ret = -1;
                    }
                }else
                {
                	if(*len > 0)
                	{
                		memset(data, 0, *len);
                	}else
                	{
                		ret = -1;
                	}
                }
            }
            json_object_put(obj);
        }
    }
    return ret;
}

static s32 com_ftp_download_check(const s8 *cplist_file, const s8 *cpover_file, s8 *pfrom, s32 fbuf_len, s8 *pover, s32 obuf_len){
	const pst_ftp_cfg cpftp_cfg_st = com_ftp_cfg_get();
	char cmd[256] = {0};
    FILE *fp = NULL;
    s8 *pret_s8 = SD_NULL;
    s32 list_num = 0, over_num = 0, ii = 0, ret_s32 = 0;

    list_num = com_func_file_line(cplist_file);
    if(list_num < 0) {
		return -1;
    }
    if((list_num == 0) || (list_num % 2 != 0))
        return 0;

    over_num = com_func_file_line(cpover_file);
    if(over_num <= 0)
        over_num = 0;
    else{
        if(over_num == list_num)
            return 0;
        if(over_num > list_num){
            snprintf(cmd, sizeof(cmd), "rm -f %s", cpover_file);
            system(cmd);
            return -1;
        }
        if(over_num % 2 != 0){
            snprintf(cmd, sizeof(cmd), "rm -f %s", cpover_file);
            system(cmd);
            over_num = 0;
        }
    }

    list_num /= 2;
    over_num /= 2;

    fp = fopen((const char *)cplist_file, "r");
    if(fp == NULL){ /* 打开失败 */
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): fopen(%s)\n", __LINE__, cplist_file);
        return -1;
    }
    for(ii = 0; ii < over_num + 1; ii++){
        pret_s8 = (s8 *)fgets((char *)pfrom, fbuf_len, fp);
        if(pret_s8 == NULL){ /* 读出错退�?*/
            over_num = list_num;
            break;
        }
        pret_s8 = (s8 *)fgets((char *)pover, obuf_len, fp);
        if(pret_s8 == NULL){ /* 读出错退�?*/
            over_num = list_num;
            break;
        }
    }
    fclose(fp);
    if(over_num == list_num)
        return 0;

    ret_s32 = strlen((char *)pfrom);
    if(ret_s32 <= 0)
        return -1;
    pfrom[ret_s32 - 1] = '\0';
    ret_s32 = strlen((char *)pover);
    if(ret_s32 <= 0)
        return -1;
    pover[ret_s32 - 1] = '\0';

	if((cpftp_cfg_st->iCountFtpFailDownload) >= THE_MAX_FTP_FAIL_DOWNLOAD)
	{
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp (LINE=%d): The ftp download count outoff %d.\n", __LINE__, (cpftp_cfg_st->iCountFtpFailDownload));
		return 0;
	}
	
    return(list_num - over_num); /* 剩余个数 */
}

static s32 com_ftp_download_poll(void){
	const pst_ftp_cfg cpftp_cfg_st = com_ftp_cfg_get();
    s8 from[768] = {0}, to[768] = {0}, cmd[768] = {0};
    s32 flag = 0, status = 0, ret_s32 = 0;
    FILE *fp = NULL;

    ret_s32 = com_ftp_download_check((const s8 *)DEF_DOWNLOAD_LIST_FILE, (const s8 *)DEF_DOWNLOAD_OVER_FILE, from, sizeof(from), to, sizeof(to));
    if(ret_s32 == 0){ /* 下载完成 */
        snprintf((char *)cmd, sizeof(cmd), "rm -f %s %s", DEF_DOWNLOAD_LIST_FILE, DEF_DOWNLOAD_OVER_FILE);
        system((char *)cmd);
		cpftp_cfg_st->iCountFtpFailDownload = 0;
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Info(LINE=%d): download finsh.\n", __LINE__);
        return 0;
    }else if(ret_s32 > 0){ /* 执行下载 */
        snprintf((char *)cmd, sizeof(cmd), "wget -c --restrict-file-names=nocontrol %s --user=ftpadmin --password=ftpadmin -P %s", from, to);
        ret_s32 = pox_system((char *)cmd);
        flag = WIFEXITED(ret_s32);
        status = WEXITSTATUS(ret_s32);

		//Let the Count to add every download.
		(cpftp_cfg_st->iCountFtpFailDownload)++;

        if(ret_s32 == -1){ /* system执行错误 */
            UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): system(wget)\n", __LINE__);
            return -1;
        }else{
            if(flag){ /* 命令执行正常退�?*/
//                if(0 == status){ /* 执行Ok */
                    fp = fopen(DEF_DOWNLOAD_OVER_FILE, "a+"); /*  */
                    if(fp == NULL){ /* 打开失败 */
                        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): fopen(%s)\n", __LINE__, DEF_DOWNLOAD_OVER_FILE);
                        return -1;
                    }

                    fprintf(fp, "%s\n%s\n", from, to);
                    fclose(fp);
                    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Info(LINE=%d): system(wget) success.\n", __LINE__);
                    return 0;
//                }else{ /* 执行失败*/
//                    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): system(wget) fail.\n", __LINE__);
//                    return -1;
//                }
            }else{ /* 错误退�?*/
                UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): system(wget) exit.\n", __LINE__);
                return -1;
            }
        }
    }else{ /* 出错 */

    }
    return 0;
}

static s32 com_ftp_upload_check(const s8 *cplist_file, const s8 *cpover_file, s8 *pfrom, s32 fbuf_len, s8 *pover, s32 obuf_len, 
										s8 * pdata, s32 data_len){
	const pst_ftp_cfg cpftp_cfg_st = com_ftp_cfg_get();
	char cmd[256] = {0};
    FILE *fp = NULL;
    s8 *pret_s8 = SD_NULL;
    s32 list_num = 0, over_num = 0, ii = 0, ret_s32 = 0;

    list_num = com_func_file_line(cplist_file);
    if(list_num < 0) {
		return -1;
    }
    if((list_num == 0) || (list_num % 3 != 0))
        return 0;

    over_num = com_func_file_line(cpover_file);
    if(over_num <= 0)
        over_num = 0;
    else{
        if(over_num == list_num)
            return 0;
        if(over_num > list_num){
            snprintf(cmd, sizeof(cmd), "rm -f %s", cpover_file);
            system(cmd);
            return -1;
        }
        if(over_num % 3 != 0){
            snprintf(cmd, sizeof(cmd), "rm -f %s", cpover_file);
            system(cmd);
            over_num = 0;
        }
    }

    list_num /= 3;
    over_num /= 3;

    fp = fopen((const char *)cplist_file, "r");
    if(fp == NULL){ /* 打开失败 */
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): fopen(%s)\n", __LINE__, cplist_file);
        return -1;
    }
    for(ii = 0; ii < over_num + 1; ii++){
        pret_s8 = (s8 *)fgets((char *)pfrom, fbuf_len, fp);
        if(pret_s8 == NULL){ /* 读出错退�?*/
            over_num = list_num;
            break;
        }
        pret_s8 = (s8 *)fgets((char *)pover, obuf_len, fp);
        if(pret_s8 == NULL){ /* 读出错退�?*/
            over_num = list_num;
            break;
        }
		pret_s8 = (s8 *)fgets((char *)pdata, data_len, fp);
        if(pret_s8 == NULL){ /* 读出错退�?*/
            over_num = list_num;
            break;
        }
    }
    fclose(fp);
    if(over_num == list_num)
        return 0;

    ret_s32 = strlen((char *)pfrom);
    if(ret_s32 <= 0)
        return -1;
    pfrom[ret_s32 - 1] = '\0';
    ret_s32 = strlen((char *)pover);
    if(ret_s32 <= 0)
        return -1;
    pover[ret_s32 - 1] = '\0';
    ret_s32 = strlen((char *)pdata);
    if(ret_s32 <= 0)
        return -1;
    pdata[ret_s32 - 1] = '\0';

	if((cpftp_cfg_st->iCountFtpFailDownload) >= THE_MAX_FTP_FAIL_DOWNLOAD)
	{
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp (LINE=%d): The ftp download count outoff %d.\n", __LINE__, (cpftp_cfg_st->iCountFtpFailDownload));
		return 0;
	}
	
    return(list_num - over_num); /* 剩余个数 */
}

static s32 com_ftp_upload_poll(void){

	const pst_ftp_cfg cpftp_cfg_st = com_ftp_cfg_get();
	s8 from[768] = {0}, to[768] = {0}, cmd[768] = {0}, rmcmd[512] = {0};
	s32 flag = 0,  ret_s32 = 0;
	s32 data_len = 2048;
	s8 * data = (s8 *)calloc(data_len, 1);
	int ctxLen = 2048, msgid = -1;
	unsigned char * context = (unsigned char *)calloc(ctxLen, 1);
	FILE *fp = NULL;
	pid_t status;
	s32 ret = -1;

	pthread_mutex_lock(&(cpftp_cfg_st->mtx_upl));
	ret_s32 = com_ftp_upload_check((const s8 *)DEF_UPLOAD_LIST_FILE, (const s8 *)DEF_UPLOAD_OVER_FILE, 
								from, sizeof(from), to, sizeof(to), data, data_len);
	pthread_mutex_unlock(&(cpftp_cfg_st->mtx_upl));

	if(ret_s32 == 0)
	{ /* 上传完成 */
		remove(DEF_UPLOAD_LIST_FILE);
		remove(DEF_UPLOAD_OVER_FILE);
		UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Info(LINE=%d): upload finsh.\n", __LINE__);
		ret = 0;
	}
	else if(ret_s32 > 0)
	{ /* 执行上传 */
	
		msgid = com_ftp_json_to_data(data, (void *)context, &ctxLen);
		
		snprintf((char *)cmd, sizeof(cmd), "wput %s ftp://ftpadmin:ftpadmin@%s -t 5", from, to);


		status = pox_system((char *)cmd);

		if(status == -1)
		{ /* system执行错误 */
			UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): system(wput)\n", __LINE__);
			cpftp_cfg_st->onUplEnd(SD_FAIL, msgid, context, ctxLen);
		}
		else
		{
			if(WIFEXITED(status))
			{ /* 命令执行正常退�?*/
			//         if(0 == WEXITSTATUS(status)){ /* 执行Ok */
				fp = fopen(DEF_UPLOAD_OVER_FILE, "a+"); /*  */
				if(fp == NULL)
				{ /* 打开失败 */
					UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): fopen(%s)\n", __LINE__, DEF_UPLOAD_OVER_FILE);
					cpftp_cfg_st->onUplEnd(SD_FAIL, msgid, context, ctxLen);
				}
				else
				{
					fprintf(fp, "%s\n%s\n%s\n", from, to, data);
					fclose(fp);
					UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Info(LINE=%d): system(wput) success.\n", __LINE__);
					cpftp_cfg_st->onUplEnd(SD_SUCCESS, msgid, context, ctxLen);
					ret = 0;
				}
			}
			else
			{ /* 错误退�?*/
				fp = fopen(DEF_UPLOAD_OVER_FILE, "a+"); /*  */
				if(fp == NULL)
				{ /* 打开失败 */
					UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): fopen(%s)\n", __LINE__, DEF_UPLOAD_OVER_FILE);
					cpftp_cfg_st->onUplEnd(SD_FAIL, msgid, context, ctxLen);
				}
				else
				{
					fprintf(fp, "%s\n%s\n", from, to);
					fclose(fp);
					UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): system(wput) exit.\n", __LINE__);
					cpftp_cfg_st->onUplEnd(SD_FAIL, msgid, context, ctxLen);
				}
			}		
		}
	}
	else
	{
		cpftp_cfg_st->onUplEnd(SD_FAIL, -1, NULL, 0);
	}

	if(data)
	{
		free(data);
	}
	if(context)
	{
		free(context);
	}
	return ret;
}
/************************************************************************
* @FunctionName( 函数�?): static void *com_ftp_thread(void *parg)
* @Description(描述):      主线�? FTP执行下载线程
* @Param(参数):
* @ReturnCode(返回�?:
************************************************************************/
static void *com_ftp_thread(void *parg){
    const pst_ftp_cfg cpftp_cfg_st = com_ftp_cfg_get();
    s8 up_down = 0;
	cpftp_cfg_st->iCountFtpFailDownload = 0;
    s32 ret_s32 = 1;

    while(cpftp_cfg_st->ftp_run){
        if(up_down) /* 上传/下载 */
            com_ftp_download_poll();
        else
          ret_s32 = com_ftp_upload_poll();

		usleep(500000); 
        up_down = !up_down;
    }
    return 0;
}
/**@END! static void *com_ftp_thread(void *parg) !\(^o^)/~ 结束�?*/

/************************************************************************
* @FunctionName( 函数�?): s32 com_ftp_download(const s8 *cpfrom, const s8 *cpto)
* @CreateDate  (创建日期): 2015/12/01
* @Author      ( �? �?): CJH
*
* @Description(描述): FTP下载
*
* @Param(参数):
*       *cpfrom - 下载
*       *cpto - 保存�?
*
* @ReturnCode(返回�?:
*       -1 - 下载失败
*        0 - 成功开始下�?
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(�?
************************************************************************/
s32 com_ftp_download(const s8 *cpfrom, const s8 *cpto){
    char cmd[256] = {0};
    s8 path[256] = {0};
    FILE *fp = NULL;

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Info(LINE=%d): Download from(%s)", __LINE__, cpfrom);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Info(LINE=%d): Save to(%s)", __LINE__, cpto);

    com_func_pathfile_path((const s8 *)DEF_DOWNLOAD_LIST_FILE, path, sizeof(path));

    if(access((char *)path, F_OK) != 0){ /* 检测不到路�?/
        snprintf(cmd, sizeof(cmd), "mkdir -p %s", path); /* 生成命令 */
        system(cmd); /* 创建目录 */
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Info(LINE=%d): Create path(%s)", __LINE__, path);
    }

    fp = fopen(DEF_DOWNLOAD_LIST_FILE, "a+"); /* 文件存在, 文件不存�? 创建, 追加写入且可�?*/
    if(fp == NULL){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): fopen(%s)", __LINE__, DEF_DOWNLOAD_LIST_FILE);
        return -1;
    }

    fprintf(fp, "%s\n%s\n", cpfrom, cpto);
    fclose(fp);

    return 0;
}
/**@END! s32 com_ftp_download(const s8 *cpfrom, const s8 *cpto) !\(^o^)/~ 结束�?*/

/* ######################################################################################## */
/************************************************************************
* @FunctionName( 函数�?): s32 com_ftp_upload(const s8 *cpfrom, const s8 *cpto)
* @CreateDate  (创建日期): 2018/06/07
* @Author      ( �? �?): barrnett
*
* @Description(描述): FTP上传
*
* @Param(参数):
*       *cpfrom - 上传
*       *cpto - 保存�?
*
* @ReturnCode(返回�?:
*       -1 - 上传失败
*        0 - 成功开始上�?
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(�?
************************************************************************/
s32 com_ftp_upload(const s8 *cpfrom, const s8 *cpto, s32 msgid, s8 * context, s32 ctxLen){
    char cmd[256] = {0};
    s8 path[256] = {0};
    FILE *fp = NULL, *fq =NULL;
	int jsonstrLen = 2048;
	char * jsonstr = (char *)calloc(jsonstrLen, 1);
	const pst_ftp_cfg cpftp_cfg_st = com_ftp_cfg_get();

    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Info(LINE=%d): upload from(%s)", __LINE__, cpfrom);
    UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Info(LINE=%d): Save to(%s)", __LINE__, cpto);

    com_func_pathfile_path((const s8 *)DEF_UPLOAD_LIST_FILE, path, sizeof(path));

    if(access((char *)path, F_OK) != 0){ /* 检测不到路�?/
        snprintf(cmd, sizeof(cmd), "mkdir -p %s", path); /* 生成命令 */
        system(cmd); /* 创建目录 */
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Info(LINE=%d): Create path(%s)", __LINE__, path);
    }

	if(0 != com_ftp_data_to_json(msgid, context, ctxLen, jsonstr, &jsonstrLen))
	{
		UT_LOG_LOGOUT_ERROR(emModCOM, 0, "com_ftp Error(LINE=%d): cast context to json failed", __LINE__);
		if(jsonstr)
		{
			free(jsonstr);
		}
		return -1;
	}

	pthread_mutex_lock(&(cpftp_cfg_st->mtx_upl));

    fp = fopen(DEF_UPLOAD_LIST_FILE, "a+"); /* 文件存在, 文件不存�? 创建, 追加写入且可�?*/
    if(fp == NULL){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): fopen(%s)", __LINE__, DEF_UPLOAD_LIST_FILE);
		if(jsonstr)
		{
			free(jsonstr);
		}
		pthread_mutex_unlock(&(cpftp_cfg_st->mtx_upl));
        return -1;
    }
	
    fprintf(fp, "%s\n%s\n%s\n", cpfrom, cpto, jsonstr);
    fclose(fp);
	pthread_mutex_unlock(&(cpftp_cfg_st->mtx_upl));
	#if 0
	fq = fopen(DEF_UPLOAD_OVER_FILE, "a+"); /* 文件存在, 文件不存�? 创建, 追加写入且可�?*/
	 if(fq == NULL){
		 UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): fopen(%s)", __LINE__, DEF_UPLOAD_OVER_FILE);
		 return -1;
	 }
	 
	 fclose(fq);
    #endif 

	if(jsonstr)
	{
		free(jsonstr);
	}

    return 0;
}
/**@END! s32 com_ftp_download(const s8 *cpfrom, const s8 *cpto) !\(^o^)/~ 结束�?*/
/* ######################################################################################## */

/************************************************************************
* @FunctionName( 函数�?): s32 com_ftp_init(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( �? �?): CJH
* @Description(描述): 初始�?
* @Param(参数): NULL
* @ReturnCode(返回�?: 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(�?
************************************************************************/
s32 com_ftp_init(void *parg, void (* onUplEnd)(int, int, void *, int)){
	const pst_ftp_cfg cpftp_cfg_st = com_ftp_cfg_get();
	pthread_mutex_init(&(cpftp_cfg_st->mtx_upl), NULL);
	cpftp_cfg_st->onUplEnd = onUplEnd;
    return 0;
}
/**@END! s32 com_ftp_init(void *parg) !\(^o^)/~ 结束�?*/

/************************************************************************
* @FunctionName( 函数�?): s32 com_ftp_start(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( �? �?): CJH
* @Description(描述): 启动
* @Param(参数): NULL
* @ReturnCode(返回�?: 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(�?
************************************************************************/
s32 com_ftp_start(void *parg){
    const pst_ftp_cfg cpftp_cfg_st = com_ftp_cfg_get();
    s32 ret_s32 = 0;

    cpftp_cfg_st->ftp_run = 1;
    ret_s32 = pthread_create(&cpftp_cfg_st->thread, NULL, &com_ftp_thread, NULL);
    if (ret_s32 != 0){
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): pthread_create()\n", __LINE__);
        return -1;
    }

    return 0;
}
/**@END! s32 com_ftp_start(void *parg) !\(^o^)/~ 结束�?*/

/************************************************************************
* @FunctionName( 函数�?): s32 com_ftp_stop(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( �? �?): CJH
* @Description(描述): 停止
* @Param(参数): NULL
* @ReturnCode(返回�?: 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(�?
************************************************************************/
s32 com_ftp_stop(void *parg){
    const pst_ftp_cfg cpftp_cfg_st = com_ftp_cfg_get();
    cpftp_cfg_st->ftp_run = 0;
	pthread_join(cpftp_cfg_st->thread, NULL);
    return 0;
}
/**@END! s32 com_ftp_stop(void *parg) !\(^o^)/~ 结束�?*/

/************************************************************************
* @FunctionName( 函数�?): s32 com_ftp_uninit(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( �? �?): CJH
* @Description(描述): 反初始化
* @Param(参数): NULL
* @ReturnCode(返回�?: 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(�?
************************************************************************/
s32 com_ftp_uninit(void *parg){
    const pst_ftp_cfg cpftp_cfg_st = com_ftp_cfg_get();
    s32 ret_s32 = 0;
    ret_s32 = pthread_join(cpftp_cfg_st->thread, NULL);
    if (ret_s32 != 0)
        UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp Error(LINE=%d): pthread_join(thread)\n", __LINE__);
	pthread_mutex_destroy(&(cpftp_cfg_st->mtx_upl));
    return 0;
}
/**@END! s32 com_ftp_uninit(void *parg) !\(^o^)/~ 结束�?*/






#if (0)
/**
 * \fn      SdInt com_ftp_download_upload_check(IN SdChar *pszListFile, OUT SdChar *pszFTPFrom, SdInt iFromBufLen
                  , IN SdChar *pszOverFile, OUT SdChar *pszFTPTo, SdInt iToBufLen);
 * \brief   上传或下载检�?
 * \param   IN SdChar *pszListFile - (上传/下载)列表文件
 * \param   OUT SdChar *pszFTPFrom - 上传�? 返回本地路径, 下载�? 返回FTP服务器地址
 * \param   SdInt iFromBufLen - From缓冲区长�?
 * \param   IN SdChar *pszOverFile - (上传/下载)完成列表文件
 * \param   OUT SdChar *pszFTPTo - 上传�? 返回FTP服务器地址, 下载�? 返回本地路径
 * \param   SdInt iToBufLen - To缓冲区长�?
 * \return  -1 - 出错
 * \return   0 - 完成
 * \return  >0 - �?上传/下载)条数
 * \note
 * \todo
 * \version V1.0
 * \warning 必须保证参数的合法�?
*/
SdInt com_ftp_download_upload_check(IN SdChar *pszListFile, OUT SdChar *pszFTPFrom, SdInt iFromBufLen
, IN SdChar *pszOverFile, OUT SdChar *pszFTPTo, SdInt iToBufLen)
{
   SdChar szCmdBuf[128] = {0};
   FILE *fp = SD_NULL;
   SdInt iListNum = 0;
   SdInt iOverNum = 0;
   SdChar *pszRet = SD_NULL;

   if(access(pszListFile, F_OK) != 0) /* 列表文件不存�?*/
      return -1;

   iListNum = (SdInt)com_func_file_line(pszListFile);
   if(iListNum == -1)
      return -1;

   fp = fopen(pszListFile, "r"); /* 列表文件存在, 只读打开 */
   if(fp == SD_NULL) /* 打开失败 */
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: fopen (%s0 fail.", __FILE__, __LINE__, pszListFile);
      return -1;
   }

   if(fclose(fp) != 0) /* 关闭列表文件 */
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: fclose (%s) fail.", __FILE__, __LINE__, pszListFile);
   fp = SD_NULL;
   if(iListNum % 2 != 0) /* 列表文件可靠性校�?*/
   {
      sprintf(szCmdBuf, "rm -f %s", pszListFile); /* 生成命令, 删除列表文件 */
      system(szCmdBuf);
      if(access(pszOverFile, F_OK) == 0) /* 完成列表文件存在 */
      {
         sprintf(szCmdBuf, "rm -f %s", pszOverFile); /* 生成命令, 删除完成列表文件 */
         system(szCmdBuf);
      }
      return -1;
   }

   if(access(pszOverFile, F_OK) == 0) /* 完成列表文件存在 */
   {
      fp = fopen(pszOverFile, "r"); /* 只读打开 */
      if(fp == SD_NULL) /* 打开失败 */
      {
         UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: fopen (%s) fail.", __FILE__, __LINE__, pszOverFile);
         return -1;
      }
      iOverNum = com_func_file_line_get(fp);
      if(iOverNum % 2 != 0)
      {
         sprintf(szCmdBuf, "rm -f %s", pszOverFile); /* 生成命令, 删除完成列表 */
         system(szCmdBuf);
         iOverNum = 0;
      }
      if(fclose(fp) != 0) /* 关闭完成列表文件 */
         UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: fclose (%s) fail.", __FILE__, __LINE__, pszOverFile);

   }
   else /*完成列表不存�?*/
      iOverNum = 0;

   iListNum /= 2;
   iOverNum /= 2;

   if(iListNum > iOverNum){ /* 还需要执�?*/
      SdInt ii = 0;
      fp = fopen(pszListFile, "r"); /* 文件存在, 只读打开 */
      if(fp == SD_NULL) /* 打开失败 */
      {
         UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: fopen (%s) fail.", __FILE__, __LINE__, pszListFile);
         return -1;
      }
      for(ii =0; ii < iOverNum + 1; ii++)
      {
         pszRet = fgets(pszFTPFrom, iFromBufLen, fp);
         if(pszRet == SD_NULL) /* 读出错退�?*/
         {
            iOverNum = iListNum;
            break;
         }
         pszRet = fgets(pszFTPTo, iToBufLen, fp);
         if(pszRet == SD_NULL) /* 读出错退�?*/
         {
            iOverNum= iListNum;
            break;
         }
      }

      pszFTPFrom[strlen(pszFTPFrom) - 1] = '\0'; /* 清除'\n' */
      pszFTPTo[strlen(pszFTPTo) - 1] = '\0'; /* 清除'\n' */
      if(fclose(fp) != 0) /* 关闭完成列表文件 */
         UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: fclose (%s) fail.", __FILE__, __LINE__, pszListFile);

   }else{
      return 0; /* 无操�?*/
   }
   return(iListNum - iOverNum); /* 剩余个数 */
}

/**
 * \fn      SdInt com_ftp_download_poll(void);
 * \brief   FTP下载轮询
 * \param
 * \return   -1 - 出错
 * \return    0 - 执行Ok
 * \note
 * \todo
 * \version V1.0
 * \warning 必须保证参数的合法�?
*/
SdInt com_ftp_download_poll(void)
{
   SdInt iRet = 0;
   SdInt iPathLen = 0;
   SdChar szCmdBuf[1024] = {0};
   SdChar szDownloadFrom[512] = {0}, szSaveTo[512] = {0};
   SdInt iFlag = 0, iStatus = 0;
   FILE *fp = SD_NULL;

   iRet = com_ftp_download_upload_check(DEF_DOWNLOAD_LIST_FILE, szDownloadFrom, sizeof(szDownloadFrom), 
               DEF_DOWNLOAD_OVER_FILE, szSaveTo, sizeof(szSaveTo));
   if(iRet == 0) /* 都下载完�?*/
   {
      sprintf(szCmdBuf, "rm -f %s  %s", DEF_DOWNLOAD_LIST_FILE, DEF_DOWNLOAD_OVER_FILE);
      system(szCmdBuf);

      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Down finish, delete list file..");

      return 0;
   }
   else if(iRet > 0) /* 执行下载 */
   {
      sprintf(szCmdBuf, "wget -c --restrict-file-names=nocontrol %s --user=ftpadmin --password=ftpadmin -P %s", szDownloadFrom, szSaveTo);

      iRet = system(szCmdBuf);
      iFlag = WIFEXITED(iRet);
      iStatus = WEXITSTATUS(iRet);
      if(-1 == iRet) /* system执行错误 */
      {
         UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: func (system) error.", __FILE__, __LINE__);
         return -1;
      }
      else
      {
         if(iFlag) /* 命令执行正常退�?*/
         {
//            if(0 == iStatus) /* 执行Ok */
//            {
               fp = fopen(DEF_DOWNLOAD_OVER_FILE, "a+"); /*  */
               if(fp == SD_NULL) /* 打开失败 */
               {
                  UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: fopen (%s) fail.", __FILE__, __LINE__, DEF_DOWNLOAD_OVER_FILE);
                  return -1;
               }

               fprintf(fp, "%s\n%s\n", szDownloadFrom, szSaveTo);

               if(fclose(fp) != 0) /* 关闭文件 */
                  UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: fclose (%s) fail.", __FILE__, __LINE__, DEF_DOWNLOAD_OVER_FILE);
               UT_LOG_LOGOUT_INFO(emModCOM, 0, "func (system(CMD)) success.");
               return 0;
//            }
//            else /* 执行失败*/
//            {
//               UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: func (system(%s)) fail.", __FILE__, __LINE__, szCmdBuf);
//               return -1;
//            }
         }  
         else /* 错误退�?*/
         {
            UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: func (system(%s)) exit.", __FILE__, __LINE__, szCmdBuf);
            return -1;
         }
      }
   }
   else /* 出错 */
   {

   }
   return iRet;
}

/**
 * \fn      SdInt com_ftp_upload_poll(void);
 * \brief   FTP上传轮询
 * \param
 * \return   -1 - 出错
 * \return    0 - 执行Ok
 * \note
 * \todo
 * \version V1.0
 * \warning 必须保证参数的合法�?
*/
SdInt com_ftp_upload_poll(void)
{

}

/**
 * \fn      static void *com_ftp_thread(void *ptr);
 * \brief   FTP执行线程
 * \param  void *ptr - &COMManage
 * \return
 * \note
 * \todo
 * \version V1.0
 * \warning 必须保证参数的合法�?
*/
static void *com_ftp_thread(void *ptr)
{
   SdBool bUpDown = SD_TRUE;
   LPCOMManage pstCOMManage =(LPCOMManage)ptr;
   SdInt iRet = 0;

   while(pstCOMManage->FTPClientRunFlag)
   {
      if(bUpDown) /* 上传/下载 */
         com_ftp_download_poll();
      else
         com_ftp_upload_poll();

      usleep(500000); /* 500milli */
      bUpDown = !bUpDown;
   }
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "com_ftp_thread exit..");
   pthread_exit(0);
}

/**
 * \fn      SdInt com_ftp_thread_create(void *ptr);
 * \brief   创建FTP执行线程
 * \param  void *ptr - &COMManage
 * \return   0 - 创建成功
 * \return -1 - 创建失败
 * \note
 * \todo
 * \version V1.0
 * \warning 必须保证参数的合法�?
*/
SdInt com_ftp_thread_create(void *ptr){
   LPCOMManage pstCOMManage =(LPCOMManage)ptr;
   SdInt iRet = 0;

   pstCOMManage->FTPClientRunFlag = SD_TRUE;

   iRet = ut_thread_create(&pstCOMManage->UtThreadFTP, UT_THREAD_STACK_SIZE_DEFAULT, UT_THREAD_PRIORITY_DEFAULT,
                                          com_ftp_thread, ptr);
   if(iRet != 0)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: ut_thread_create fail.", __FILE__, __LINE__);
      return -1;
   }
   return 0;
}

/**
 * \fn      SdInt com_ftp_download(IN SdChar *pszDownloadFrom, IN SdChar *pszSaveTo);
 * \brief   FTP下载请求
 * \param   IN SdChar *szDownloadFrom - 下载地址
 * \param   IN SdChar *pszSaveTo - 保存路径
 * \return  0 - 成功
 * \return -1 - 失败
 * \note
 * \todo
 * \version V1.0
 * \warning 必须保证参数的合法�?
*/
SdInt com_ftp_download(IN SdChar *pszDownloadFrom, IN SdChar *pszSaveTo)
{
   FILE *fp = SD_NULL;
   SdChar szPath[128] = {0};
   SdChar szCmdBuf[128] = {0};

   UT_LOG_LOGOUT_INFO(emModCOM, 0, "DownLoad From: (%s).", pszDownloadFrom);
   UT_LOG_LOGOUT_INFO(emModCOM, 0, "Save To: (%s).", pszSaveTo);
    
   com_func_pathfile_path_get(DEF_DOWNLOAD_LIST_FILE, szPath, sizeof(szPath));

   if(access(szPath, F_OK) != 0) /* 检测不到路�?/
   {
      sprintf(szCmdBuf, "mkdir -p %s", szPath); /* 生成命令 */
      system(szCmdBuf); /* 创建目录 */
      UT_LOG_LOGOUT_INFO(emModCOM, 0, "Create Path = (%s).", szPath);
   }

   fp = fopen(DEF_DOWNLOAD_LIST_FILE, "a+"); /* 文件存在, 文件不存�? 创建, 追加写入且可�?*/
   if(fp == SD_NULL)
   {
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: fopen (%s) fail.", __FILE__, __LINE__, DEF_DOWNLOAD_LIST_FILE);
      return -1;
   }

   fprintf(fp, "%s\n%s\n", pszDownloadFrom, pszSaveTo); /* 添加一条下�?*/

   if(fclose(fp) != 0) /* 关闭列表文件 */
      UT_LOG_LOGOUT_ERROR(emModCOM, 0, "[%s:%d]: fclose (%s) fail.", __FILE__, __LINE__, DEF_DOWNLOAD_LIST_FILE);
   fp = SD_NULL;
 
   return 0;
}
#endif
