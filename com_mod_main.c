#if 0
/** 多进程模式 */
#define SD_OSAL_MULITI_PROCESS_MODE

/** UI模块是活动模块 */
#define MODULE_COM_ACTIVE

#include <ut_all.h>
#include <ut_type.h>
#include "com_mod_attr.inc"

#ifndef _MAIN_
#define _MAIN_

static UTModAttr *g_stModAttr[] = {
   &g_stCOMModAttr
};

extern SdInt com_debug(void);

int main(int argc , char *argv[])
{
   ut_init_proc_title(argc, argv);
   ut_module_init(g_stModAttr, 1);

#ifdef CMSDEBUG
      com_debug();
#endif

   return 0;
}
#endif
#endif
