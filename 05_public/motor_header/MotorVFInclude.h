/***************************************************************
文件功能：和VF控制相关的数据结构定义，变量申明
文件版本：VERSION 1.0
最新更新：2007.09.27
************************************************************/
#ifndef MOTOR_VF_INCLUDE_H
#define MOTOR_VF_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorDefine.h"
#include "SubPrgInclude.h"
#include "MotorCurrentTransform.h"

/************************************************************/
/***********************结构体定义***************************/
typedef struct VF_INFO_STRUCT_DEF {
	Uint 	VFLineType;				//VF曲线选择
	Uint 	VFTorqueUp;				//VF转矩提升增益
	Uint 	VFTorqueUpLim;			//VF转矩提升截至频率
	Uint 	VFOverExc;				//VF过励磁增益
	Uint 	VFWsComp;				//VF转差补偿增益
	Uint 	VFOvShock;				//VF抑制振荡增益
	Uint 	VFLineFreq1;			//多点VF频率1
	Uint 	VFLineVolt1;			//多点VF电压1
	Uint 	VFLineFreq2;			//多点VF频率2
	Uint 	VFLineVolt2;			//多点VF电压2
	Uint 	VFLineFreq3;			//多点VF频率3
	Uint 	VFLineVolt3;			//多点VF电压3
	Uint 	SVCTorqueUp;			//svc转矩提升增益
	Uint 	SVCTorqueUpLim;			//svc转矩提升截至频率

    Uint    vfResComp;      // 低频电阻压降是否重新分解电流
    Uint    ovGain;     // 过压失速增益
    Uint    ovPoint;    // 过压失速抑制点
    Uint    ovPointCoff;
    Uint    ocGain;     // 过流失速增益
    Uint    ocPoint;    // 过流失速点
    s16     FreqApply;  // vf 失速控制产生的频率

    s16     vfMode;     // Vf失速控制模式，0:280模式，1:新模式
    s16     tpLst;
}VF_INFO_STRUCT;	//VF参数设置数据结构

typedef struct  VF_AUTO_TORQUEBOOST_VAR_DEF{
    Uint        VfAutoTorqueEnable;     //  1- Execute auto torque boost
    s16         DestinationVolt;        //  自动转矩提升闭环的目标电压
    s16         VfCurrentIs;
    s16         VfReverseAngle;
    s16         VfRIsSinFai;
    s16         VfRVCosFai;
    s16         VfAngleSin;
    s16         VfReverseVolt;
    s16         VfTorqueEnableTime;
    PID_STRUCT  AutoTorquePID;
}VF_AUTO_TORQUEBOOST_VAR;	//自动转矩提升用变量结构

typedef struct OVER_SHOCK_STRUCT_DEF{
	s16				IMFilter;
    s16             IO;             //抑制振荡的励磁电流给定值应缓慢增加，否则大功率会在启动时引起一个电流尖峰
	PID_STRUCT		pid;
    Uint            TimesNub;       //启动后4s钟内，抑制振荡目标电流使用空载电流设定值
    Uint            oscMode;        //抑制振荡模式
    s16             ShockDecrease;  //对抑制振荡增益的削减，用于DPWM调制时逐步取消抑制振荡

    s16             OscVolt;
    //s16             OscPhase;
}OVER_SHOCK_STRUCT;

typedef struct VF_WS_COMP_STRUCT_DEF {
	Uint 	   Coff;
	long       CompFreq;
    s16        DelayTime;
    s16        FilterCoff; //滤波系数
    MT_STRUCT  WsCompMT;
    MT_STRUCT  WsCompMTApply;
}VF_WS_COMP_STRUCT;	

typedef struct VAR_AVR_STRUCT_DEF{
	s16		UDCFilter;
	s16		CoffApply;
	s16		Cnt;
}VAR_AVR_STRUCT;//过励磁模块(可调AVR功能)使用的数据结构

#if 0
typedef struct VF_CURRENT_CONTROL_DEF{
    Uint      disVfCsr;

    Uint      ocPoint;
    Uint      currentLimit;
    Uint      active;      // 电流环起作用标志
    Uint      vfCsrKP;
    Uint      vfCsrKI;
    Uint      vfCsrKD;
    
    int16       detaVolt;
    PID_STRUCT	pid;	
}VF_CURRENT_CONTROL;
#endif

typedef struct THREE_ORDER_FILTER_DEF{ 
    long    OutK1;
    long    OutK2;
    long    OutK3;
    
    long    InK1;
    long    InK2;
    long    InK3;
    
    long    OutData1;
    long    OutData2;
    long    OutData3;
    
    long    InData1;
    long    Indata2;
    long    Indata3;    
}THREE_ORDER_FILTER; //三阶滤波函数使用的系数

typedef struct VF_VAR_CALC_DEF{
    s16     vfTq;       // Q12
    s16     vfPt;       // Q12
}VF_VAR_CALC;

typedef struct VF_OVER_UDC_DAMP_DEF
{
    //int16 mStepUdc;   // 运行频率变化增量(电压)
    int16 uDcBrakePt;
    int16 vfOvUdcPt;
    
    PID_STRUCT pidUdc;
    PID_STRUCT pidPower;

    int16       powerFdb;
    int16       powerSet;
    Uint    maxPower;
} VF_OVER_UDC_DAMP;

typedef struct VF_FREQ_DEAL_DEF
{
    s16    stepCur;
    s16    stepUdc;
    s16    stepSet;
    s16    stepEnd;
    Uint    freqDir;
    Uint    freqSet;
    Uint    freqAim;
    Uint    freqApply;
    Uint    preSpdFlag; 
    Uint    spedChg;
}VF_FREQ_DEAL;

typedef struct OVER_CURRENT_DAMP_DEF2{
	s16		LowFreq;			//低频转折点 (SI)

	s16		stepApply;			//实际使用的步长
	s16		stepLAmp;			//电流小于0.88限流点的步长
	s16		stepLAmpLim;			//电流小于0.88限流点的步长上限
	s16		stepHAmp;			//电流大于0.88限流点的步长
	s16		CurBak;				//上一拍电流值

	s16		maxStepLF;			//低于低频转折点的最大步长
	s16		maxStepHF;		//高于低频转折点的最大步长
	s16		ocPointQ12;
	s16		subStep;			//电流大于0.88限流点时步长减小速度
	s16     addStep;
	s16		Flag;				//标志
								//BIT0	=1 表示速度已经追上设定速度
								//BIT15 =1 表示电流已经超过限流点的0.88倍
} OVER_CURRENT_DAMP2;//过流抑制模块使用的数据结构

typedef struct OVER_CURRENT_DAMP_DEF{
	s16		StepApply;			//实际使用的步长
	s16		StepLow;			//电流小于0.88限流点的步长
	s16		StepLowLim;			//电流小于0.88限流点的步长上限
	s16		StepHigh;			//电流大于0.88限流点的步长
	s16		CurBak;				//上一拍电流值
	s16		LowFreq;			//低频转折点
	s16		MaxStepLow;			//低于低频转折点的最大步长
	s16		MaxStepHigh;		//高于低频转折点的最大步长
	s16		CurLim;
	s16		SubStep;			//电流大于0.88限流点时步长减小速度
	s16		Flag;				//标志
								//BIT0	=1 表示速度已经追上设定速度
								//BIT15 =1 表示电流已经超过限流点的0.88倍
}OVER_CURRENT_DAMP;//过流抑制模块使用的数据结构

typedef struct OVER_UDC_CTL_STRUCT_DEF{
	s16		CoffApply;
	s16		CoffAdd;
	s16		Limit;              // 过压抑制点
	s16		StepApply;
    s16     LastStepApply;
	s16		StepBak;
	s16		ExeCnt;
	s16		UdcBak;
	s16		Flag;
	s16		FreqMax;			//减速第一拍的频率
	s16     AccTimes;           //过压抑制导致的频率增加次数
	s16		OvUdcLimitTime;

    //s16     PreStepApply;
}OVER_UDC_CTL_STRUCT;//过流抑制模块使用的数据结构

typedef struct HVF_OSC_DAMP_STRUCT_DEF
{
#if 0
    s16 FreqSet;                // 功能设定同步频率
    s16 FreqSynApply;           // 实际同步频率
    s16 FreqSpliEst;            // 估计转差
    s16 VoltOsc;
    s16 CompLeakageLs;          // 是否补偿漏感电压
#endif

    s16 OscDampGain;            // 振荡抑制增益
    s16 VoltSmSet;              // m轴电压
    s16 VoltEmf;                // vvvf反电势，vers freq
    
    s16 VoltAmp;                // 输出电压幅值
    s16 VoltPhase;              // 电压相位

    s16	VoltPhaseFilt;		//滤波后的角度
    FILTER_1ST  VoltPhaseFilter ;
#if 0
//debug
    s16 detVoltT;
    s16 detVoltM;
    // gama 模型
    s16 Rs;
    s16 Lt;             //Q format
    s16 Lg;             // 漏感
    s16 LgPerSet;       // 漏感系数设定
    s16 CurM;
    s16 CurMDealy;      // 延迟滤波
    s16 taoCurMDealy;
    s16 CurTDealy;
#endif

    s16 Rs;
    s16 CurMagSet;          // 励磁电流设定
    s16 VoltAmpAdjActive;    // 电压幅值是否调整， 小功率调整有利于转矩提升
    
}HVF_OSC_DAMP_STRUCT ;

typedef struct HVF_OSC_JUDGE_INDEX_DEF
{
    s16 AnglePowerFactor;      // 功率因素角
    
    s16 wCntUse;
    s16 wCntRltm;
    s16 maxAngle;
    s16 minAngle;
    s16 oscIndex;
    
}HVF_OSC_JUDGE_INDEX;

typedef struct HVF_DB_COMP_OPT_DEF
{
    s16     HVfDbCompOptActive;     // :=1 开启优化
    s16     PhasPredictGain;        // 相位预测增益
    
    s16     CurPhaseFeed;
    s16     CurPhaseFeed_pre;  // 上一拍
    s16     CurPhasePredict;   // 预测相位
    s16     StepPhaseSet;       // 每个中断的相位step(Q15)
    s16     CurPhaseStepFed;    // 检测得到的电流相位变化
    s16     CurPhaseStepPredict;// 预测相位变化量
    s16     PhaseFwdFedCoeff;   // 前馈系数

    s16     DbOptActHFreq;      // 优化生效上限频率点
    s16     DbOptActLFreq;      // 优化生效下限频率点

    s16     DbCompCpwmWidth;    // 
}HVF_DB_COMP_OPT;

/************************************************************/
/*******************供外部引用变量声明***********************/
extern VF_INFO_STRUCT			gVFPar;		//VF参数
extern VF_AUTO_TORQUEBOOST_VAR  gVFAutoVar;
extern OVER_SHOCK_STRUCT		gVfOsc;	//抑制振荡用结构变量
extern VF_WS_COMP_STRUCT		gWsComp;
extern VAR_AVR_STRUCT			gVarAvr;

//extern VF_CURRENT_CONTROL       gVfCsr;
extern MT_STRUCT				gHVfCur;

extern THREE_ORDER_FILTER      gOscAmp;
extern OVER_UDC_CTL_STRUCT		gOvUdc;

extern HVF_OSC_DAMP_STRUCT      gHVfOscDamp;
extern HVF_OSC_JUDGE_INDEX      gHVfOscIndex;
extern HVF_DB_COMP_OPT          gHVfDeadBandCompOpt;

/*******************供外部引用函数声明***********************/

s16  CalOutVotInVFStatus(s16);
void OverShockControl(void);
void VFOverMagneticControl(void);
void VfVarInitiate(void);
void VFWsTorqueBoostComm( void );
void VFWSCompControl(void);		
void VfFreqDeal2(void);
void VFSpeedControl(void);
void VFAutoTorqueBoost(void);

void VfFreqDeal(void);
void VfOverCurDeal();
void VfOverUdcDeal();
void VFSpeedControl();
void CalTorqueUp(void);
void HVfOscDampDeal();




#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================

