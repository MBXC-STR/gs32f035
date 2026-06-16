/****************************************************************
文件功能：和功能模块的数据交互程序，单CPU下使用数组拷贝，双CPU下通讯
文件版本： 
最新更新： 
	
****************************************************************/
#include "MotorInclude.h"
#include "MotorIPMSVC.h"

//------------------性能版本号显示--------//
// 定义通用软件版本信息
/************************************************************************
F7-10:性能版本号,F7-11:功能版本号,F7-15:性能临时版本号,F7-16:功能临时版本号
注意:版本升级号不能超过相应的位数,例如"通用软件版本升级号"不能超过8位二进制
*************************************************************************/
#if (SOFTSERIES == MD500SOFT) 
	#define CORE_SOFTWARE_VERSION           00        // 通用软件版本
	#define CORE_SOFTWARE_VERSION_TEMP      57        // 通用软件版本升级过程  

	// 定义非标软件版本信息
	#define CORE_NON_STANDARD_VERSION       0        // 非标号
	#define CORE_NON_STANDARD_VERSION_TEMP  0        // 非标的升级过程

	// 定义临时版本软件信息
	#define CORE_TEMP_SOFTWARE_NO           0        // 临时软件号
	#define CORE_TEMP_SOFTWARE_VERSION      0        // 临时软件升级过程

	#define CORE_SOFT_TYPE                  0         // 软件类型       0:通用    1:非标
	#define CORE_IS_TEMP_SOFTWARE           0         // 是否为临时软件 0:非临时  1:临时  
#else
	#define CORE_SOFTWARE_VERSION           00        // 通用软件版本
	#define CORE_SOFTWARE_VERSION_TEMP      04        // 通用软件版本升级过程 

	// 定义非标软件版本信息
	#define CORE_NON_STANDARD_VERSION       0        // 非标号
	#define CORE_NON_STANDARD_VERSION_TEMP  0        // 非标的升级过程

	// 定义临时版本软件信息
	#define CORE_TEMP_SOFTWARE_NO           0        // 临时软件号
	#define CORE_TEMP_SOFTWARE_VERSION      0        // 临时软件升级过程

	#define CORE_SOFT_TYPE                  0         // 软件类型       0:通用    1:非标
	#define CORE_IS_TEMP_SOFTWARE           0         // 是否为临时软件 0:非临时  1:临时
#endif
	// // 全局变量定义
	//#define     NUM_2MS_F2M     151     // 功能2ms传递给驱动
	#define     NUM_2ms_FUNC_TO_MOTOR           160   //
	#define     NUM_2ms_FUNC_TO_MOTOR_debug     30
	//#define     NUM_2MS_M2F     60      // 驱动2ms传递给功能
#if(AIRCOMPRESSOR == 1)
    #define     NUM_2ms_MOTOR_TO_FUNC           38  
#else
	#define     NUM_2ms_MOTOR_TO_FUNC           36  
#endif
	#define     NUM_2ms_MOTOR_TO_FUNC_debug     30
	#define     NUM_05MS_F2M    14      // 功能05ms传递给驱动
	#define     NUM_05MS_M2F    5       // 驱动05ms传递给功能
	#define     NUM_TUNE_M2F    20      // 驱动调谐时传递给功能(2ms)


Uint gCoreSoftWareVesion = (CORE_SOFTWARE_VERSION << 8) + CORE_SOFTWARE_VERSION_TEMP;             // 高8位:通用软件，低8位:升级过程
Uint gCoreNonStandardVesion = (CORE_NON_STANDARD_VERSION << 6) + CORE_NON_STANDARD_VERSION_TEMP;  // 高10位:非标号，低6位:非标升级过程
Uint gCoreTmpsoftNoAndVersion = (CORE_TEMP_SOFTWARE_NO << 6) + CORE_TEMP_SOFTWARE_VERSION;        // 高10位:临时软件号，低6位:临时升级过程
Uint gCoreSoftType = (CORE_SOFT_TYPE << 8) + CORE_IS_TEMP_SOFTWARE;                               // 高8位:通用或非标，低8位:是否临时版本

int  gSendToMotor05MsDataBuff[NUM_05MS_F2M];
int  gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + NUM_2ms_FUNC_TO_MOTOR_debug+3];
int  gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC + NUM_2ms_MOTOR_TO_FUNC_debug];
int  gRealTimeToFunctionDataBuff[NUM_05MS_M2F];
int  gParaIdToFunctionDataBuff[NUM_TUNE_M2F];

Uint uReservedData;              // 保留参数
Uint gSoftVersion = SOFT_VERSION;

extern PMSM_FLUX_WEAK_STRUCT   gPmFluxWeak;
extern MT_STRUCT_Q24           gUMTQ24; 
extern u16 InvCurForCutDown;
/**************************************************************
	0.5Ms循环中，从功能模块获取需要的所有参数(暂时不传该组参数)
*************************************************************/
void ParSend05Ms(void)
{
    //gRealTimeToFunctionDataBuff[0]	=	gUDC.uDC	;
    //gRealTimeToFunctionDataBuff[1]	=	gLineCur.CurBaseInv	;
    //gRealTimeToFunctionDataBuff[2]	=	gIAmpTheta.anglePF	;
    //gRealTimeToFunctionDataBuff[3]	=	gVfCsr.active	;
    //gRealTimeToFunctionDataBuff[4]	=	gOutVolt.VoltApply	;
}

void ParGet05Ms(void)
{
    gMainCmd.Command.all            =	gSendToMotor05MsDataBuff[0]	;
    gExtendCmd.all                  =	gSendToMotor05MsDataBuff[1]	;
    gGetParVarable.TuneType         =	(TUNE_FLOW_ENUM)gSendToMotor05MsDataBuff[2]	;
    gMotorInfo.MotorType            =	gSendToMotor05MsDataBuff[3]	;
    gMainCmd.FreqSet                =	gSendToMotor05MsDataBuff[4]	;
    gOutVolt.vfSplit                =	gSendToMotor05MsDataBuff[5]	;
    gMotorInfo.Votage               =	gSendToMotor05MsDataBuff[6]	;
    gMotorInfo.CurrentGet           =	gSendToMotor05MsDataBuff[7]	;
    gMotorInfo.Frequency            =	gSendToMotor05MsDataBuff[8]	;
    gVFPar.VFLineType               =	gSendToMotor05MsDataBuff[9]	;

    gVFPar.ovGain                   =    gSendToMotor05MsDataBuff[10]	;
    gVFPar.ovPoint                  =    gSendToMotor05MsDataBuff[11]	;
    gVFPar.ocGain                   =    gSendToMotor05MsDataBuff[12]	;
    gVFPar.ocPoint                  =    gSendToMotor05MsDataBuff[13]	;
}

void ParSend2Ms(void)
{
    gSendToFunctionDataBuff[0]	=	gMainStatus.StatusWord.all	;
    gSendToFunctionDataBuff[1]	=	gGetParVarable.StatusWord	;
    gSendToFunctionDataBuff[2]	=	gError.ErrorCode.ErrorCodeStruct.ErrorCode1	;
    gSendToFunctionDataBuff[3]	=	gError.ErrorCode.ErrorCodeStruct.ErrorCode2	;
    gSendToFunctionDataBuff[4]	=	gError.ErrorInfo[0].all	;
    gSendToFunctionDataBuff[5]	=	gError.ErrorInfo[1].all	;
    gSendToFunctionDataBuff[6]	=	gError.ErrorInfo[2].all	;
    gSendToFunctionDataBuff[7]	=	gError.ErrorInfo[3].all	;
    gSendToFunctionDataBuff[8]	=	gError.ErrorInfo[4].all	;
    gSendToFunctionDataBuff[9]	=	gLineCur.ErrorShow	;
    gSendToFunctionDataBuff[10]	=	gMainCmd.FreqToFunc	;
    gSendToFunctionDataBuff[11]	=	gOutVolt.VoltDisplay	;
    gSendToFunctionDataBuff[12]	=	gTemperature.Temp	;
    gSendToFunctionDataBuff[13]	=	gMotorInfo.Current	;
    gSendToFunctionDataBuff[14]	=	gSoftVersion	;
    gSendToFunctionDataBuff[15]	=	gAI.gAI1	;
    gSendToFunctionDataBuff[16]	=	gAI.gAI2	;
    gSendToFunctionDataBuff[17]	=	gAI.gAI3	;
    gSendToFunctionDataBuff[18]	=	gUDC.uDCShow	;
    gSendToFunctionDataBuff[19]	=	gLineCur.CurTorque	;
    gSendToFunctionDataBuff[20]	=	gLineCur.CurPerShow	;
    gSendToFunctionDataBuff[21]	=	gIPMPos.RealPos	;
    gSendToFunctionDataBuff[22]	=	gPowerTrq.TrqOutHoAndFo_pu	;	//主从控制中使用的转矩电流
    gSendToFunctionDataBuff[23]	=	gRotorSpeed.SpeedEncoder;
    gSendToFunctionDataBuff[24] =   gPowerTrq.InvPower_si;
    gSendToFunctionDataBuff[25] =   gPowerTrq.TrqOut_pu;
    gSendToFunctionDataBuff[26] =   gRotorTrans.AbsRotPos;              // ABZ编码器时用来显示参考位置和实际转子位置偏差
    gSendToFunctionDataBuff[27] =   gPowerTrq.anglePF;
    gSendToFunctionDataBuff[28] =   gIPMPos.ZSigNumSet;
    gSendToFunctionDataBuff[29] =   gInvInfo.InvTypeApply;      // u0- 59

// 版本号信息交互
    gSendToFunctionDataBuff[30] =   gCoreSoftWareVesion;
    gSendToFunctionDataBuff[31] =   gCoreNonStandardVesion;
    gSendToFunctionDataBuff[32] =   gCoreTmpsoftNoAndVersion;
    gSendToFunctionDataBuff[33] =   gCoreSoftType;
    gSendToFunctionDataBuff[34] =   gEstBemf.BemfVoltDisPlay;   //在线反电动势辨识值
    gSendToFunctionDataBuff[35] =   gOverLoad.TotalPercent;
#if(AIRCOMPRESSOR == 1)
	gSendToFunctionDataBuff[36] =   gAI.gAI4; 
	gSendToFunctionDataBuff[37] =   gAI.gAI5; 
#endif


// UF 参数组
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC + 0 ] =   gBasePar.FcSetApply;												// uf-00 实际载频, 0.1Hz
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC + 1 ] =   gMainCmd.FreqSet;													// uf-01 功能传递的设定频率, Q15
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	2 ] =   gMainCmd.FreqSetApply;												// uf-02 性能实际设定频率, Q15
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	3 ] =   gRotorSpeed.SpeedEncoder;											// uf-03 编码器反馈频率, Q15
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	4 ] =   gOverLoad.TotalPercent;											// uf-04 过载程度系数
    
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	5 ] =   gIMTSetApply.M >> 12;												// uf-05 给定励磁电流, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	6 ] =   gIMTSetApply.T >> 12;												// uf-06 给定力矩电流, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	7 ]	=	gIMTQ24.M >> 12;													// uf-07 反馈励磁电流, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	8 ]	=	gIMTQ24.T >> 12;													// uf-08 反馈力矩电流, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	9 ]	=	gRotorSpeed.SpeedApply;											// uf-09 反馈频率, Q15
    
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	10]	=	gOutVolt.LimitOutVoltPer;											// uf-10 极限输出电压, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	11] =   gPmFluxWeak.VoltLpf;												// uf-11 输出电压滤波值, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	12]	=	gOutVolt.MaxOutVoltPer;											// uf-12 最大输出电压, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	13]	=	gIAmpTheta.Amp;													// uf-13 线电流有效值, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	14]	=	gIAmpTheta.Theta;													// uf-14 线电流角度, Q15
    
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	15]	=	gUMTSet.M	;														// uf-15 d轴电压给定, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	16]	=	gUMTSet.T 	;														// uf-16 q轴电压给定, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	17]	=	((Ulong)gPmsmRotorPosEst.SvcRotorPos >> 16) * 3600 >> 16;		// uf-17 SVC估算角度, 0.1度
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	18]	=	(Ulong)gIPMPos.RotorPos * 3600 >> 16;							// uf-18 转子位置角度, 0.1度
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	19]	=	gUDC.uDC;															// uf-19 母线电压, Q12
    
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	20]	=   gPhase.IMPhase;													// uf-20 d轴相位, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	21]	=   (Ulong)gIPMPos.InitPos * 3600 >> 16;								// uf-21 磁极初始位置角, 0.1度
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	22]	=   gMainStatus.RunStep;												// uf-22 主步骤
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	23]	=   gUAmpTheta.Theta;													// uf-23 电压角度
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	24]	=	gPmsmRotorPosEst.SvcRotorPos >> 16;								// uf-24
    
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	25]	=	gIUVWQ12.U;														// uf-25 U相电流, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	26]	=	gIUVWQ12.V;														// uf-26 V相电流, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	27]	=	gIUVWQ12.W;														// uf-27 W相电流, Q12
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	28]	=	gOverLoad.InvTotal.half.MSW;										// uf-28
    gSendToFunctionDataBuff[NUM_2ms_MOTOR_TO_FUNC +	29]	=	EPwm1Regs.TBPRD;													// uf-29
}

void ParGet2Ms(void)
{    
    gSubCommand.all              =	gSendToMotor2MsDataBuff[0]	;  
    gMainCmd.FreqDesired         =	gSendToMotor2MsDataBuff[1]	;  
    gBasePar.MaxFreq             =	gSendToMotor2MsDataBuff[2]	;  
#if (SOFTSERIES == MD500SOFT)
    gBasePar.FcSet               =	gSendToMotor2MsDataBuff[3]	;  
#else
    gBasePar.FcSetToMotor        =	gSendToMotor2MsDataBuff[3]	;  
#endif
    gMotorInfo.Power             =	gSendToMotor2MsDataBuff[4]	;  
    gMotorExtInfo.Rpm            =	gSendToMotor2MsDataBuff[5]	;  
    gMotorExtInfo.R1             =	gSendToMotor2MsDataBuff[6]	;  
    gMotorExtInfo.R2             =	gSendToMotor2MsDataBuff[7]	;  
    gMotorExtInfo.L0             =	gSendToMotor2MsDataBuff[8]	;  
    gMotorExtInfo.LM             =	gSendToMotor2MsDataBuff[9]	;  
    gMotorExtInfo.I0             =	gSendToMotor2MsDataBuff[10]	;  
    gMotorExtInfo.RsPm           =	gSendToMotor2MsDataBuff[11]	;  
    gMotorExtInfo.LD             =	gSendToMotor2MsDataBuff[12]	;  
    gMotorExtInfo.LQ             =	gSendToMotor2MsDataBuff[13]	;  
    gVCPar.ASRKpLow              =	gSendToMotor2MsDataBuff[14]	;  
    gVCPar.ASRTILow              =	gSendToMotor2MsDataBuff[15]	;  
    gVCPar.ASRKpHigh             =	gSendToMotor2MsDataBuff[16]	;  
    gVCPar.ASRTIHigh             =	gSendToMotor2MsDataBuff[17]	;  
    gVCPar.ASRSwitchLow          =	gSendToMotor2MsDataBuff[18]	;  
    gVCPar.ASRSwitchHigh         =	gSendToMotor2MsDataBuff[19]	;  
    gVCPar.VCWsCoff              =	gSendToMotor2MsDataBuff[20]	;  
    gVCPar.VCTorqFilter          =	gSendToMotor2MsDataBuff[21]	;  //SVC输出转矩滤波系数
    gPGData.PulseNum             =	gSendToMotor2MsDataBuff[22]	;  
    gVCPar.VcOverExc             =	gSendToMotor2MsDataBuff[23]	;  
    gLoadLose.ChkLevel           =  gSendToMotor2MsDataBuff[24]	; 
    gLoadLose.ChkTime            =  gSendToMotor2MsDataBuff[25]	; 
    gVCPar.PosVCTorqueLim        =	gSendToMotor2MsDataBuff[26]	;  
    gVFPar.VFTorqueUp            =	gSendToMotor2MsDataBuff[27]	;  
    gVFPar.VFTorqueUpLim         =	gSendToMotor2MsDataBuff[28]	;  
    gVFPar.VFLineFreq1           =	gSendToMotor2MsDataBuff[29]	;  
    gVFPar.VFLineVolt1           =	gSendToMotor2MsDataBuff[30]	;  
    gVFPar.VFLineFreq2           =	gSendToMotor2MsDataBuff[31]	;  
    gVFPar.VFLineVolt2           =	gSendToMotor2MsDataBuff[32]	;  
    gVFPar.VFLineFreq3           =	gSendToMotor2MsDataBuff[33]	;  
    gVFPar.VFLineVolt3           =	gSendToMotor2MsDataBuff[34]	;  
    gVFPar.VFWsComp              =	gSendToMotor2MsDataBuff[35]	;  
    gVFPar.VFOverExc             =	gSendToMotor2MsDataBuff[36]	;  
    gVFPar.VFOvShock             =	gSendToMotor2MsDataBuff[37]	;  
    gComPar.StartDCBrakeCur      =	gSendToMotor2MsDataBuff[38]	;  
    gComPar.StopDCBrakeCur       =	gSendToMotor2MsDataBuff[39]	;  
    gComPar.BrakeCoff            =	gSendToMotor2MsDataBuff[40]	;  
    gComPar.MotorOvLoad          =	gSendToMotor2MsDataBuff[41]	;  
    gComPar.PerMotorOvLoad       =	gSendToMotor2MsDataBuff[42]	;  
    gPWM.SoftPWMTune             =	gSendToMotor2MsDataBuff[43]	;  
    gADC.DelaySet                =	gSendToMotor2MsDataBuff[44]	;  
    gInvInfo.LowUdcCoff          =	gSendToMotor2MsDataBuff[45]	;  
    gPGData.PGTypeGetFromFun     =	gSendToMotor2MsDataBuff[46]	;  
    gRotorSpeed.TransRatio       =	gSendToMotor2MsDataBuff[47]	;  
    gInvInfo.InvTypeSet          =	gSendToMotor2MsDataBuff[48]	;  
    gInvInfo.GpTypeSet           =	gSendToMotor2MsDataBuff[49]	;  
    gInvInfo.TempType            =	gSendToMotor2MsDataBuff[50]	;   
    gInvInfo.UDCCoff             =	gSendToMotor2MsDataBuff[51]	;  
    gInvInfo.CurrentCoff         =	gSendToMotor2MsDataBuff[52]	;  
    gUVCoff.UDivVGet             =	gSendToMotor2MsDataBuff[53]	;  
    gVCPar.SvcMode               =	gSendToMotor2MsDataBuff[54]	;  
    gDeadBand.DeadTimeSet        =	gSendToMotor2MsDataBuff[55]	; 
//#if (SOFTSERIES == MD380SOFT) 
    //gVCPar.NegVCTorqueLim        =	gSendToMotor2MsDataBuff[56]	;  // 修改为发电转矩上限，wyk
//#endif   
    gComPar.SpdSearchTimeSet     =	gSendToMotor2MsDataBuff[57]	;  
    gIPMPos.PowerOffPos          =	gSendToMotor2MsDataBuff[58]	;       // 上次掉电同步机角度
    gRotorTrans.Poles            =	gSendToMotor2MsDataBuff[59]	;   
    gMotorExtInfo.BemfVolt       =	gSendToMotor2MsDataBuff[60]	;  
    gVCPar.AcrImKp               =	gSendToMotor2MsDataBuff[61]	;  
    gVCPar.AcrImKi               =	gSendToMotor2MsDataBuff[62]	;  
    gVCPar.AcrItKp               =	gSendToMotor2MsDataBuff[63]	;  
    gVCPar.AcrItKi               =	gSendToMotor2MsDataBuff[64]	;   
    gPGData.SpeedDir             =	gSendToMotor2MsDataBuff[65]	;  
    gIPMPos.RotorZeroGet         =	gSendToMotor2MsDataBuff[66]	;    
    gPWM.PwmModeSwitchLF         =    gSendToMotor2MsDataBuff[67] ;
    gUVWPG.UvwDir                           =	gSendToMotor2MsDataBuff[68]	;  
    gUVWPG.UvwZeroPos_deg                   =	gSendToMotor2MsDataBuff[69]	;  
    gFluxWeak.Mode                          =	gSendToMotor2MsDataBuff[70]	;  
    gFluxWeak.CoefFlux                      =	gSendToMotor2MsDataBuff[71]	;  
    gFluxWeak.IdMax                         =	gSendToMotor2MsDataBuff[72]	;  
    gFluxWeak.CoefAdj                       =	gSendToMotor2MsDataBuff[73]	;  
    gFluxWeak.CoefKI                        =	gSendToMotor2MsDataBuff[74]	; 

	gFlyingStart.KpRatio                    = gSendToMotor2MsDataBuff[108];
	gFlyingStart.KiRatio                    = gSendToMotor2MsDataBuff[109];
	gFlyingStart.CurLimitAdj                = gSendToMotor2MsDataBuff[110];
		
    gVCPar.NegVCTorqueLim                   =	gSendToMotor2MsDataBuff[119]	;    
    gPmFluxWeak.FluxWeakDepth               =	gSendToMotor2MsDataBuff[120]	;
    gIPMInitPos.CurLimitAdjBig			  =	gSendToMotor2MsDataBuff[121]	;
    gIPMInitPos.InitTestFlag                =	gSendToMotor2MsDataBuff[122]	;
    gAsr.Mode                               =	gSendToMotor2MsDataBuff[123]	;
    gPmFluxWeak.SalientRateCoff             =   gSendToMotor2MsDataBuff[124]	;
    gPmFluxWeak.PmsmMaxTorqCtrlEnable       =   gSendToMotor2MsDataBuff[125]	;
	gPmDecoup.EnableDcp                     =	gSendToMotor2MsDataBuff[126]	;
	gPmParEst.IsKpK                         =	gSendToMotor2MsDataBuff[127]	;
	gPmParEst.IsKiK                         =	gSendToMotor2MsDataBuff[128]	;
    gIPMPos.ZSigEnable                      =	gSendToMotor2MsDataBuff[129]	;
    gPmsmRotorPosEst.SvcSpeedLpfTs          =	gSendToMotor2MsDataBuff[130]	;
    gPmsmRotorPosEst.Kb                     =	gSendToMotor2MsDataBuff[131]	;
    gPmsmRotorPosEst.Ka                     =	gSendToMotor2MsDataBuff[132]	;
    gPmsmRotorPosEst.SvcIdMaxForLowSpeed    =	gSendToMotor2MsDataBuff[133]	;
    gPmsmRotorPosEst.FcLow                  =	gSendToMotor2MsDataBuff[134]	;
    gPmsmRotorPosEst.SvcLowSpeedMode        =	gSendToMotor2MsDataBuff[135]	;
	gPmsmRotorPosEst.BrakeFreq              =	gSendToMotor2MsDataBuff[136]	;
	gPmsmRotorPosEst.Step                   =	gSendToMotor2MsDataBuff[137]	;
	gIPMInitPos.CurLimitAdjSmall			  =	gSendToMotor2MsDataBuff[138]	;
    gPmsmRotorPosEst.SpeedCheckEnable       =	gSendToMotor2MsDataBuff[139]	;
	gVCPar.ZeroServer                       =	gSendToMotor2MsDataBuff[140]	;
	gVCPar.ASRSwitchZero                    =	gSendToMotor2MsDataBuff[141]	;
	gVCPar.ASRKpZero                        =	gSendToMotor2MsDataBuff[142]	;
	gVCPar.ASRTIZero                        =	gSendToMotor2MsDataBuff[143]	;
	gVCPar.AntiDirEnable                    =	gSendToMotor2MsDataBuff[144]	;
	gAsr.PosSetAdd                          =	gSendToMotor2MsDataBuff[145]	;
    //gVCPar.NegVCTorqueLim                   =	gSendToMotor2MsDataBuff[146]	;
    gBrake.Ontimeset                        =	gSendToMotor2MsDataBuff[147]	;
    gUVWPG.ErrorEnable                      =	gSendToMotor2MsDataBuff[148]	;
    gIPMInitPos.ErrorIintPosEnable          =	gSendToMotor2MsDataBuff[149]	;
    gIPMPos.LoadTuneErrorEnable             =	gSendToMotor2MsDataBuff[150]	;

	gUVCoff.BeforeRunTuneEnable             =	gSendToMotor2MsDataBuff[151]	;
	gUVCoff.OnlineTuneBemfVoltEnable        =	gSendToMotor2MsDataBuff[152]	;
	gOverLoad.OverLoadPreventEnable         =	gSendToMotor2MsDataBuff[153]	;
#if (SOFT_INPUT_DETECT == STLEN)
    gSoftInLose.HalfInvPowerPU              =	gSendToMotor2MsDataBuff[154]	;
#endif
	gMainCmd.XiuMianFreq                    =	gSendToMotor2MsDataBuff[155]	;
	gPGData.DiscDetectDelayTime             =   gSendToMotor2MsDataBuff[156]	;
//	gOverLoad.GPOverLoadLine                =   gSendToMotor2MsDataBuff[157]	;
    gIPMPos.CompPosFun			             =   gSendToMotor2MsDataBuff[158]	;    
    gPWM.OverModuleCoff                     =   gSendToMotor2MsDataBuff[159]	;

    gRotorSpeed.SvcSpeed             =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 0 ]	;  //svc下测速使能
    gTestDataReceive.TestData1       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 1 ]	;  //输出功率显示方式
    gTestDataReceive.TestData2       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 2 ]	;  //5段发波使能
    gTestDataReceive.TestData3       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 3 ]	;  
    gTestDataReceive.TestData4       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 4 ]	;  
    gTestDataReceive.TestData5       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 5 ]	;  
    gTestDataReceive.TestData6       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 6 ]	; // SVC励磁电流提升
    gTestDataReceive.TestData7       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 7 ]	; // SVC励磁电流提升截止频率
    gVCPar.VCSpeedFilter             =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 8 ]	; 
    gIPMPos.TuneDetaPosDisable       =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 9 ]	; // SVC转矩限幅方式选择
    gTestDataReceive.TestData10      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 10]	; 
    gTestDataReceive.TestData11      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 11]	; 
    gTestDataReceive.TestData12      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 12]	; 
    gTestDataReceive.TestData13      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 13]	; 
    gTestDataReceive.TestData14      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 14]	; 
    gTestDataReceive.TestData15      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 15]	; 
    gTestDataReceive.TestData16      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 16]	; 
    gTestDataReceive.TestData17      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 17]	; 
    gTestDataReceive.TestData18      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 18]	; 
    gTestDataReceive.TestData19      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 19]	; 
    gTestDataReceive.TestData20      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 20]	; 
    gTestDataReceive.TestData21      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 21]	; 
    gTestDataReceive.TestData22      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 22]	; 
    gTestDataReceive.TestData23      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 23]	; 
    gTestDataReceive.TestData24      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 24]	; 
    gTestDataReceive.TestData25      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 25]	; 
    gTestDataReceive.TestData26      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 26]	; 
    gTestDataReceive.TestData27      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 27]	; 
    gTestDataReceive.TestData28      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 28]	; 
    gTestDataReceive.TestData29      =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + 29]	;  // 防反转时使用此参数   
 
    gRotorSpeed.FreWindow            =	gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + NUM_2ms_FUNC_TO_MOTOR_debug]; 
    gVCPar.TorMasToFol               =  gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + NUM_2ms_FUNC_TO_MOTOR_debug +1];  
	gBrake.VoltageLim                =  gSendToMotor2MsDataBuff[NUM_2ms_FUNC_TO_MOTOR + NUM_2ms_FUNC_TO_MOTOR_debug +2];
}

void ParSendTune(void)
{
    gParaIdToFunctionDataBuff[0]	=	gMotorExtReg.R1	;
    gParaIdToFunctionDataBuff[1]	=	gMotorExtReg.R2	;
    gParaIdToFunctionDataBuff[2]	=	gMotorExtReg.L0	;
    gParaIdToFunctionDataBuff[3]	=	gMotorExtReg.LM	;
    gParaIdToFunctionDataBuff[4]	=	gMotorExtReg.I0	;
    
    gParaIdToFunctionDataBuff[5]	=	gMotorExtReg.RsPm ;
    gParaIdToFunctionDataBuff[6]	=	gMotorExtReg.LD	;
    gParaIdToFunctionDataBuff[7]	=	gMotorExtReg.LQ	;
    gParaIdToFunctionDataBuff[8]	=	uReservedData	;
    gParaIdToFunctionDataBuff[9]	=	gEstBemf.BemfVolt	;
    
    gParaIdToFunctionDataBuff[10]	=	gPmParEst.IdKp	;
    gParaIdToFunctionDataBuff[11]	=	gPmParEst.IdKi	;
    gParaIdToFunctionDataBuff[12]	=	gPmParEst.IqKp	;
    gParaIdToFunctionDataBuff[13]	=	gPmParEst.IqKi	;
    gParaIdToFunctionDataBuff[14]	=	uReservedData ;
    
    gParaIdToFunctionDataBuff[15]	=	gPGData.PGDir	;
    gParaIdToFunctionDataBuff[16]	=	gPmParEst.CoderPos_deg	;
//    gParaIdToFunctionDataBuff[17]	=	gUVCoff.UDivVSave	;
    gParaIdToFunctionDataBuff[18]	=	gPmParEst.UvwDir	;
    gParaIdToFunctionDataBuff[19]	=	gPmParEst.UvwZeroAng_deg	;
}

void ParSendTuneBeforeRun(void)
{
    gParaIdToFunctionDataBuff[0]	=	gMotorExtReg.R1	;
    gParaIdToFunctionDataBuff[1]	=	gMotorExtReg.R2	;
    gParaIdToFunctionDataBuff[2]	=	gMotorExtReg.L0	;
    gParaIdToFunctionDataBuff[3]	=	gMotorExtReg.LM	;
    gParaIdToFunctionDataBuff[4]	=	gMotorExtReg.I0	;
    
    gParaIdToFunctionDataBuff[5]	=	gMotorExtReg.RsPm ;
    gParaIdToFunctionDataBuff[6]	=	gMotorExtReg.LD	;
    gParaIdToFunctionDataBuff[7]	=	gMotorExtReg.LQ	;
    gParaIdToFunctionDataBuff[8]	=	uReservedData	;
	gParaIdToFunctionDataBuff[9]    =   gMotorExtInfo.BemfVolt;
   
    gParaIdToFunctionDataBuff[10]	=	gPmParEst.IdKp	;
    gParaIdToFunctionDataBuff[11]	=	gPmParEst.IdKi	;
    gParaIdToFunctionDataBuff[12]	=	gPmParEst.IqKp	;
    gParaIdToFunctionDataBuff[13]	=	gPmParEst.IqKi	;
    gParaIdToFunctionDataBuff[14]	=	uReservedData ;
    
    gParaIdToFunctionDataBuff[15]	=	gPGData.PGDir	;
    gParaIdToFunctionDataBuff[16]	=	gPmParEst.CoderPos_deg	;
//    gParaIdToFunctionDataBuff[17]	=	gUVCoff.UDivVSave	;
    gParaIdToFunctionDataBuff[18]	=	gPmParEst.UvwDir	;
    gParaIdToFunctionDataBuff[19]	=	gPmParEst.UvwZeroAng_deg	;
}
