/***************************************************************
文件功能：无编码器的转子速度检测程序
文件版本：VERSION 1.0
最新更新：2009.09.27
************************************************************/
#ifndef MOTOR_SPEED_CHECK_INCLUDE_H
#define MOTOR_SPEED_CHECK_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorInclude.h"
#include "MotorDefine.h"

/************************************************************/
/***********************结构体定义***************************/
struct FEISU_SIGNAL1 {     	// bits  description
    Uint16  UdcHigh:1;      // 表示母线电压过高
	Uint16  CHECK_GET:1; 	// 表示已经搜索到速度
    Uint16  REM2:1;   	
    Uint16  REM3:1;   // 
    Uint16  REM4:1;   // 
    Uint16  REM5:1;   // 
    Uint16  REM6:1;   //
    Uint16  REM7:1;   //
    Uint16  REM8:1;   //
    Uint16  REM9:1;   //
	Uint16  REM10:1;  //
	Uint16  REM11:1;  //
	Uint16  REM12:1;  //
	Uint16  SpdSig:1;  		// 速度的符号， 0: 表示反向， 1: 正向
	Uint16  CHECK_BIT14:1;  // 从停机频率开始追踪
	Uint16  CHECK_BIT15:1;  // 从最大频率开始追踪
};//转速跟踪标志1的bit位定义

union FEISU_SIG1_DEF {
   Uint16                all;
   struct FEISU_SIGNAL1  bit;
};

struct FEISU_SIGNAL2 {     // bits  description
    Uint16  FEISUS2REM0:1;  //
    Uint16  FEISUS2REM1:1;      // 
	Uint16  FEISUS2REM2:1; //get the speed already
    Uint16  FEISUS2REM3:1;   // 
    Uint16  FEISUS2REM4:1;   // 
    Uint16  FEISUS2REM5:1;   // 
    Uint16  AlmostOver:1;   // 
    Uint16  DelayOver:1;   //
    Uint16  FEISUS2REM8:1;   //
    Uint16  FEISUS2REM9:1;   //
    Uint16  FEISUS2REM10:1;   //
	Uint16  FEISUS2REM11:1;  //
	Uint16  FEISUS2REM12:1;  //
	Uint16  FEISUS2REM13:1;  //
	Uint16  FEISUS2REM14:1;  //
	Uint16  TwoCyclesOver:1;  //
};//转速跟踪标志2的bit位定义

union FEISU_SIG2_DEF {
   Uint16                all;
   struct FEISU_SIGNAL2  bit;
};

typedef struct FEISU_STRUCT_DEF{
	union 	FEISU_SIG1_DEF Case4Sig1;
	union 	FEISU_SIG2_DEF Case4Sig2;
	Uint	CycleCnt;                   // 搜索次数计数器
	Uint	UdcRiseCnt;
	Uint	UdcOvCnt;
	Uint	GuoduCnt;
	Uint	VoltCNT;
	Uint	AlmostCNT;
	Uint	LowFreqCNT;
	Uint	UdcBak;
	Uint	CheckMode;
	s16		SpeedLast;      // 停机频率
	Uint	Speed5hz;
	s16		SpeedCheck;
	s16		SpeedMaxPu;
	Uint	VoltCheck;
	Uint	VoltCheckAim;
    Uint    VoltTemp;
	s16		CurDelta;
	s16		Ger4A;                      // 频率变化步长
	s16     CsrWtOver;                  // 等待电流闭环结束
	s16		UdcDelta;
	Uint	UdcOld;
}FEISU_STRUCT;	//转速跟踪使用变量的结构定义

typedef struct FEISU_STRUCT_NEW_DEF{
	Uint  t_DetaTime;   // 追踪拖动开始计数器
	Uint  stop_time;    // 短接开始标志
	Uint  inh_mag ;     // 整个阶段计数器
	s16   the_endspeed;
	s16   open_cl;
	s16   wsre;
	s16   gDebugFlag;
	Uint  xisspeed;
	s16   jicicg;       // 短接后电流小标志
	s16   jicics;
	s16   dbctime;
	Uint  xwjdbcbz;     // 循环奇偶判断计数器(4ms计数器)
	s16	  gTheta;
	s16   gThteta1;
	s16   gWs;
	s16   gWs_out;      // 短接后开始计数器
	long  ang_amu;
	s16   Ialph;
	s16   Ibeta;
    
    // 以下是功能码传递的参数
    //Uint  FreqAdd;      //F5-30
    //Uint  CurrentAmp;
    //Uint  PauseTime;
    //Uint  CurrP;
    //Uint  CurrUpperLim;
    Uint  TimeTotal;
    Uint  VoltageDec;
}FEISU_STRUCT_NEW;//新转速跟踪使用变量的结构定义
/************************************************************/
/*******************供外部引用变量声明***********************/

/************************************************************/
/*******************供外部引用函数声明***********************/
void RunCaseSpeedCheck2(void);

#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================


