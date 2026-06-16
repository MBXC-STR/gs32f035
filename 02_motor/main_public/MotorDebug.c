#include "MotorDefine.h"
#include "SubPrgInclude.h"
#include "MotorInclude.h"

#pragma DATA_SECTION(gDebugBuffer, "debug_buffer");

//#define MOTOR_SYSTEM_DEBUG      //调试代码开关
#define CPU_TIME_DEBUG          // CPU时间片调试

#ifdef MOTOR_SYSTEM_DEBUG
#ifdef TMS320F2808
#define  C_DEBUG_BUFF_SIZE	    1000		//调试缓冲区大小
#else   // 28035
#define C_DEBUG_BUFF_SIZE       1
#endif
#else   

#define C_DEBUG_BUFF_SIZE   1

#endif

Uint gDebugBuffer[C_DEBUG_BUFF_SIZE];
Uint gDebugPoint = 0;
int  gDebugFlag;



