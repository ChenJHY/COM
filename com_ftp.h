/**
 * \file    com_ftp.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning 版权所有, 侵权必究
 * \todo    COM模块FTP
 */
#ifndef __COM_FTP_H_
#define __COM_FTP_H_

#include "com_type.h"

#define  DEF_DOWNLOAD_LIST_FILE                 "/opt/cms/download_list.txt"
#define  DEF_DOWNLOAD_OVER_FILE                 "/opt/cms/download_over.txt"

#define  DEF_UPLOAD_LIST_FILE                   "/opt/cms/upload_list.txt"
#define  DEF_UPLOAD_OVER_FILE                   "/opt/cms/upload_over.txt"

#define  DEF_UPLOAD_IMGINFO                    "/CallInfo/"
#define	 THE_MAX_FTP_FAIL_DOWNLOAD				10		/* Ftp最多失败下载数值 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_ftp_download(const s8 *cpfrom, const s8 *cpto)
* @CreateDate  (创建日期): 2015/12/01
* @Author      ( 作  者 ): CJH
*
* @Description(描述): FTP下载
*
* @Param(参数):
*       *cpfrom - 下载
*       *cpto - 保存到
*
* @ReturnCode(返回值):
*       -1 - 下载失败
*        0 - 成功开始下载
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_ftp_download(const s8 *cpfrom, const s8 *cpto);
/**@END! s32 com_ftp_download(const s8 *cpfrom, const s8 *cpto) !\(^o^)/~ 结束咯 */

s32 com_ftp_upload(const s8 *cpfrom, const s8 *cpto, s32 msgid, s8 * context, s32 ctxLen);

/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_ftp_init(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_ftp_init(void *parg, void (* onUplEnd)(int, int, void *, int));
/**@END! s32 com_ftp_init(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_ftp_start(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 启动
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_ftp_start(void *parg);
/**@END! s32 com_ftp_start(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_ftp_stop(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 停止
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_ftp_stop(void *parg);
/**@END! s32 com_ftp_stop(void *parg) !\(^o^)/~ 结束咯 */

/************************************************************************
* @FunctionName( 函数名 ): s32 com_ftp_uninit(void *parg)
* @CreateDate  (创建日期): 2015/11/15
* @Author      ( 作  者 ): CJH
* @Description(描述): 反初始化
* @Param(参数): NULL
* @ReturnCode(返回值): 0 - 成功, -1 - 失败
*------------------------------------------------------------------------*@RevisionHistory(变更)
*  None(无)
************************************************************************/
s32 com_ftp_uninit(void *parg);
/**@END! s32 com_ftp_uninit(void *parg) !\(^o^)/~ 结束咯 */

#endif /* __COM_FTP_H_ */
