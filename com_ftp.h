/**
 * \file    com_ftp.h
 * \author  CMS - WDY
 * \date    2015-08-12
 * \version
 * \brief
 * \warning ��Ȩ����, ��Ȩ�ؾ�
 * \todo    COMģ��FTP
 */
#ifndef __COM_FTP_H_
#define __COM_FTP_H_

#include "com_type.h"

#define  DEF_DOWNLOAD_LIST_FILE                 "/opt/cms/download_list.txt"
#define  DEF_DOWNLOAD_OVER_FILE                 "/opt/cms/download_over.txt"

#define  DEF_UPLOAD_LIST_FILE                   "/opt/cms/upload_list.txt"
#define  DEF_UPLOAD_OVER_FILE                   "/opt/cms/upload_over.txt"

#define  DEF_UPLOAD_IMGINFO                    "/CallInfo/"
#define	 THE_MAX_FTP_FAIL_DOWNLOAD				10		/* Ftp���ʧ��������ֵ */

/************************************************************************
* @FunctionName( ������ ): s32 com_ftp_download(const s8 *cpfrom, const s8 *cpto)
* @CreateDate  (��������): 2015/12/01
* @Author      ( ��  �� ): CJH
*
* @Description(����): FTP����
*
* @Param(����):
*       *cpfrom - ����
*       *cpto - ���浽
*
* @ReturnCode(����ֵ):
*       -1 - ����ʧ��
*        0 - �ɹ���ʼ����
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_ftp_download(const s8 *cpfrom, const s8 *cpto);
/**@END! s32 com_ftp_download(const s8 *cpfrom, const s8 *cpto) !\(^o^)/~ ������ */

s32 com_ftp_upload(const s8 *cpfrom, const s8 *cpto, s32 msgid, s8 * context, s32 ctxLen);

/* ######################################################################################## */
/* ######################################################################################## */

/************************************************************************
* @FunctionName( ������ ): s32 com_ftp_init(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ��ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_ftp_init(void *parg, void (* onUplEnd)(int, int, void *, int));
/**@END! s32 com_ftp_init(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_ftp_start(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_ftp_start(void *parg);
/**@END! s32 com_ftp_start(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_ftp_stop(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ֹͣ
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_ftp_stop(void *parg);
/**@END! s32 com_ftp_stop(void *parg) !\(^o^)/~ ������ */

/************************************************************************
* @FunctionName( ������ ): s32 com_ftp_uninit(void *parg)
* @CreateDate  (��������): 2015/11/15
* @Author      ( ��  �� ): CJH
* @Description(����): ����ʼ��
* @Param(����): NULL
* @ReturnCode(����ֵ): 0 - �ɹ�, -1 - ʧ��
*------------------------------------------------------------------------*@RevisionHistory(���)
*  None(��)
************************************************************************/
s32 com_ftp_uninit(void *parg);
/**@END! s32 com_ftp_uninit(void *parg) !\(^o^)/~ ������ */

#endif /* __COM_FTP_H_ */
