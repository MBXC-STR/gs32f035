/***************************************************************
文件功能：和矢量控制相关的数据结构定义，变量申明
文件版本：VERSION 1.0
最新更新：2007.09.27

************************************************************/
#ifndef MOTOR_VC_INCLUDE_H
#define MOTOR_VC_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorDefine.h"
#include "SubPrgInclude.h"
#include "MotorInclude.h"
/************************************************************/
/************************数据结构定义************************/
typedef struct VC_INFO_STRUCT_DEF {
	Uint 	ASRKpLow;				//低频速度环KP
	Uint 	ASRTILow;				//低频速度环TI
	Uint 	ASRKpHigh;				//高频速度环KP
	Uint 	ASRTIHigh;				//高频速度环TI
	Uint 	ASRKpZero;				//零频速度环KP
	Uint 	ASRTIZero;				//零频速度环TI
	Uint 	ASRSwitchLow;			//低频切换频率
	Uint 	ASRSwitchHigh;			//高频切换频率
	Uint    ASRSwitchZero;			//零频切换频率
	Uint    ZeroServer;             //零伺服使能
	Uint    AntiDirEnable;          //停机防反转功能使能

	//Uint 	ACRKpLow;				//电流环KP
	//Uint 	ACRKiLow;				//电流环KI

    Uint    AcrImKp;        // 同步机和异步机电流环参数
    Uint    AcrImKi;        // M 轴电流环积分
    Uint    AcrItKp;        // T轴电流环增益
    Uint    AcrItKi;        // T轴电流环积分
    Uint    SvcMode;
    
    Uint    VcOverExc;              //矢量过励磁增益
	Uint 	PosVCTorqueLim;			//VC转矩限定
	Uint    NegVCTorqueLim;         //发电转矩限定
	Uint 	VCWsCoff;				//VC转差补偿
	Uint    VCSpeedFilter;			//VC速度环滤波时间
	Uint 	VCTorqFilter;			//SVC转矩滤波系数
	Uint    FreqWsCalMode;          //转差修正量计算模式
	Uint    FreqWsVolSub;           //按T轴电压计算转差修正量时，电压修正量
	Uint    FreqWsVolModify;        //按T轴电压计算转差修正量时，修正量设定值	
	s16     TorMasToFol;
	s32     AsrOut;                 //速度环输出 
}VC_INFO_STRUCT;	//和矢量控制相关的参数设置数据结构
typedef struct UDC_LIMIT_IT_STRUCT_DEF {
    s16    UDCLimit;           //由于母线电压上升，限制的转矩上限
    s16    UDCBak;
    s16    UDCBakCnt;
    s16    UDCDeta;
    s16    FirstOvUdcFlag;     //第一次母线电压上升到限制值
    PID_STRUCT  UdcPid;
}UDC_LIMIT_IT_STRUCT;	//速度环调节数据结构
typedef struct ASR_STRUCT_DEF {
	PID_STRUCT	Asr;
    Uint        AsrQpi;             // 按位，个位:Qp；十位:Qi；
    PID_STRUCT  TorqueLimitPid;     // 非转矩控制，最大电流限制pid
	s16  	KPHigh;
	s16  	KPLow;
	u16     KPLowCoff;              // 不同编码器下低速Kp系数不同
	s16     KPZero;
	s16  	KIHigh;
	s16  	KILow;
	s16     KIZero;
	s16  	SwitchHigh;
	s16  	SwitchLow;
	s16  	TorqueLimit;
	s16     NegTorqueLimit;
	s16     PosTorqueLimit;
	long    DetaPos;
	s32     FreqFeed;               /*反馈频率*/
    s32     FreqFeedFilter;         /*反馈频率*/
    s32     FreqSet;                /*设定频率*/
    s32     DetaFreq;               /*频率偏差*/

    s32     Total;                  /*积分累加量*/
    s32     Out;                    /*输出*/

	s32 	FreqSetStep;            /*设定频率每载波周期变化步长*/
	s32 	PosSet;                 /*设定频率的积分(设定电角度)*/
	s32     PosSetAdd;              /*增加防反转功能运行角度*/

    s16     Kp;                     /*当前使用的KP*/
    s16     Ki;                     /*当前使用的KI*/
    s16     KiPos;                  /*速度环变形后的KI*/
    s16     KiPosBak;
    s16     OutFlag;                /*输出标志:0不饱和，1正饱和，2负饱和*/
    
	s16  	KpZero;
	s16  	KiZero;
	u16     Mode;                   // 速度环模式
	s16     FreqSetMaxLimitByUdc;
}ASR_STRUCT;	//速度环调节数据结构
typedef struct VC_CSR_PARA_DEF{
    long   ImModify;            //开环矢量电压计算运行时间  
    Uint   ExecSetMode;         // 0 励磁电流反比速度设定；1 根据最大电压调整
    Uint   ImAcrKp;             //励磁电流给定值调节器KP
    Uint   ImAcrKi;             //励磁电流给定值调节器KI
    Uint   ImModefyAdd;         //励磁电流给定值正向修正量
    Uint   ImModefySub;         //励磁电流给定值反向修正量
    Uint   LmGain;              //弱磁区互感变化增益
}VC_CSR_PARA;

typedef struct MODIFYWS_STRUCT_DEF{
	s16    Faiq;
    Uint   WsMax;   //调节器上限设定值
    Uint   Kp;      
    s16    Amp;
	s16    Wsout; 
    s16    Tmp;
	s16    Delta;
	s16    tc;
 	s16    tmp1;
 	s16 	tmp2;               // SVC 优化开启功能码
 	Uint    Theta;  
 	s16    Ea;
 	s16    Eb; 
 	s16    Utheta;  
	s16    Xztotal;
}MODIFYWS_STRUCT;

typedef struct PMSM_FLUX_WEAK_DEF  //同步机弱磁
{
    long  VoltCoef;           
    long  CurrCoef;
    s16  IqLpf;
    s16   VoltLpf;
    long  VoltMax;
    long  Omg;                // 实际角速度，Q10格式  
    Ulong Ld;
    Ulong Lq;
    Ulong Flux;
	s16   FreqMax;
	s16   IdMax;
    long  AdId;
	s16  AdFreq;
	long  IdForTorq;
    long  AdIdIntg;
	long  AdFreqIntg;
	s16   DecFluxCoff;
	Uint  FluxWeakDepth;
    long  AdIq;
    long  AdIqIntg;
    long  IdMixAdjFlag;
    Uint  SalientRate;
    s16   SalientRateCoff;           // 同步机凸极调整   
    s16   CalImAdjGain;                  // 同步机磁通减小增益 
	u16   PmsmMaxTorqCtrlEnable;        // 最大转矩电流比使能控制
}PMSM_FLUX_WEAK_STRUCT;
/************************************************************/
/************************变量引用申明************************/
extern VC_INFO_STRUCT			gVCPar;			//VC参数
extern MT_STRUCT_Q24            gIMTSet;		//MT轴系下的设定电流
extern MT_STRUCT_Q24            gIMTSetApply;	//MT轴系下的电流指令值
extern MT_STRUCT				gUMTSet;		//MT轴系下的设定电压
extern AMPTHETA_STRUCT			gUAmpTheta;		//极坐标下设定电压

extern PID32_STRUCT     gImAcrQ24;
extern PID32_STRUCT     gItAcrQ24;
extern PID32_STRUCT        gIMAcr;
extern PID32_STRUCT        gITAcr;

extern ASR_STRUCT				gAsr;			//速度环
extern VC_CSR_PARA              gVcCsrPara;
extern MODIFYWS_STRUCT          gModifyws;
extern MT_STRUCT               gPWMVAlphBeta;
extern UDC_LIMIT_IT_STRUCT      gUdcLimitIt;
/************************************************************/
/*******************供外部引用函数声明***********************/
extern void ResetParForVC(void);
extern void CalIMSet(void);
extern void VCSpeedControl(void);
extern void VcCalABVolt(void);
extern void PrepImCsrPara(void);
extern void PrepPmsmCsrPrar(void);
extern void PrepareCsrPara(void);
extern void VCCsrControl(void);

extern void CalTorqueLimitPar(void);
extern void VcAsrControl(void);
extern void VcAsrControl1(void);
extern void ResetAsrPar(void);
extern void PrepareAsrPar1(void);
extern void PrepareAsrPar(void);



#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================

