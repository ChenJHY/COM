#ifndef _COM_LIFT_H_
#define _COM_LIFT_H_

#include <sd_message.h>
#include <sd_struct.h>
#include <sd_macro.h>
#include "com_type.h"

typedef enum enum_bcst_cmd
{
	enCmdBegin = 0,
	enCmdLiftInfo = 1,
	enCmdEnd
}en_bcst_cmd;

#define com_broadcast_lift_info(_arg, _len) com_broadcast_send(enCmdLiftInfo, _arg, _len)

#define COM_BCST_UDP_PORT 9903

s32 com_broadcast_init();

s32 com_broadcast_start();

s32 com_broadcast_stop();

s32 com_broadcast_uninit();

s32 com_broadcast_send(en_bcst_cmd cmd, const void * arg, SdInt len);

#endif

