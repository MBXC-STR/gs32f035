/***************************************************************
文件功能：包含一些独立功能计算:对地短路检测,上电过程判断,逐波限流,硬件过流过压处理,
          直流制动,制动电阻控制
文件版本：VERSION 1.0
最新更新：2009.09.27
************************************************************/
#ifndef MOTOR_PUBLIC_CAL_INCLUDE_H
#define  MOTOR_PUBLIC_CAL_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorInclude.h"
#include "MotorDefine.h"
#include "MotorSpeedCheceInclude.h"

/************************************************************/
/************************宏定义******************************/

/************************************************************/
/***********************结构体定义***************************/
typedef struct SHORT_GND_STRUCT_DEF {
	Uint 	Comper;
	Uint 	BaseUDC;
	Uint	ocFlag;				// 对地短路检测子故障标志, 1为过流, 2为过压
	s16		ShortCur;
}SHORT_GND_STRUCT;	//上电对地短路检测数据结构
typedef struct BRAKE_CONTROL_STRUCT_DEF{
	Uint	Flag;				//当前开通/关断状态: 
	                            // bit0 = 0 : 表示还未进行控制，第一拍后置1
	                            // bit1 = 1 : 处于开通阶段，bit1 = 0: 处于关断阶段
	Uint	Cnt;				//当前开通次数
	Uint	OnCnt;				//能够持续开通次数
	Uint	OffCnt;				//能够持续关断次数
	Uint    VoltageLim;         //制动电阻开始动作的电压值
	Uint    BreResShotDetFlag;    //制动电阻短路检测标志 0不需检测制动电阻短路检测，1为需要
	Uint    WaitTime;           //制动故障判断时用于时间等待
    Uint    ErrCode;           //用于制动电阻短路状态检测时的过流故障记录
    Uint    DelayClose;         //同步机制动电阻延迟关断时间
    Uint    Ontimetotal;
    Uint    Ontimeset;
#if (SOFTSERIES == MD500SOFT)
	Uint    IBreakQ12;
	Uint    IBreak;
	Ulong   IgbtShotTicker;
	Uint    FilterTicker;
    Ulong 	IBreakTotal;
	Uint 	IBreakrsm;
    Uint	OnCounter;			// 开通次数
	Uint	TotalCounter;		// 总开关次数
	Uint	OverLoadDegradeCoff;
	Uint	OverLoadInterval;
	Uint	minPercent;
	Uint	*pBreakOverLoad;
#endif
}BRAKE_CONTROL_STRUCT;//制动电阻控制模块使用的数据结构
typedef struct DC_BRAKE_STRUCT_DEF{
	s16			Time;			//计数用
	PID_STRUCT	PID;			//制动电压
}DC_BRAKE_STRUCT;//直流制动模块使用的数据结构
typedef struct JUDGE_POWER_LOW_DEF {
	Uint 	PowerUpFlag;
	Uint 	WaiteTime;
	Uint 	UDCOld;
}JUDGE_POWER_LOW;	//上电缓冲判断使用数据结构
/************************************************************/
/*******************供外部引用变量声明***********************/
extern DC_BRAKE_STRUCT			gDCBrake;	//直流制动用变量
extern BRAKE_CONTROL_STRUCT	    gBrake;		//制动电阻控制用变量
extern SHORT_GND_STRUCT		    gShortGnd;
extern JUDGE_POWER_LOW			gLowPower;	//上电缓冲判断使用数据结构

/************************************************************/
/*******************供外部引用函数声明***********************/
extern void CBCLimitCurPrepare(void);
extern void HardWareOverUDCDeal(void);	
extern void BrakeResControl(void);
extern void RunCaseDcBrake(void);
extern void RunCaseLowPower(void);
//extern void ParameterChange2Ms(void);	
extern void RunCaseShortGnd(void);
extern void ChangeCurrent(void);
extern void ChangeMotorPar(void);
extern void SystemParChg2Ms();
extern void RunStateParChg2Ms();
extern void SystemParChg05Ms();
extern void RunStateParChg05Ms();
extern void InvCalcPower();


extern void EncodeLineDiscProtect();
#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================


