/***************************************************************
锟侥硷拷锟斤拷锟杰ｏ拷锟斤拷锟斤拷锟斤拷锟侥ｏ拷锟斤拷锟斤拷锟酵凤拷募锟�.锟斤拷要锟斤拷锟矫碉拷锟斤拷全锟街憋拷锟斤拷锟酵猴拷锟斤拷锟斤拷锟斤拷锟斤拷.
          锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷MotorVar.c锟侥硷拷锟叫的讹拷锟斤拷顺锟斤拷锟斤拷同
锟侥硷拷锟芥本锟斤拷VERSION 1.0
锟斤拷锟铰革拷锟铰ｏ拷2007.09.27
************************************************************/
#ifndef MOTOR_INCLUDE_H
#define MOTOR_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorDefine.h"
#include "MotorCurrentTransform.h"
#include "SubPrgInclude.h"
#include "MotorVFInclude.h"
#include "MotorVCInclude.h"
#include "MotorSvcInclude.h"
#include "MotorInvProtectInclude.h"
#include "MotorPwmInclude.h"
#include "MotorStructDefine.h"
#include "MotorInfoCollectInclude.h"
#include "MotorParaIDinclude.h"
#include "MotorPublicCalInclude.h"
#include "MotorSpeedCheceInclude.h"
#include "MotorDataExchange.h"
#include "MotorPmsmMain.h"
#include "MotorEncoder.h"
#include "MotorPmsmParEst.h"
#include "MotorIPMSVC.h"
#include "PmFluxWeakCsr2.h"
#include "MotorFlyingStart.h"
/************************************************************
    锟斤拷锟斤拷锟斤拷锟斤拷 BEGIN
************************************************************/
/*****************锟斤拷锟斤拷为锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷*************************/
extern INV_STRUCT 				gInvInfo;		//锟斤拷频锟斤拷锟斤拷息
extern CONTROL_MOTOR_TYPE_ENUM  gCtrMotorType;  //锟斤拷锟斤拷锟斤拷秃涂锟斤拷锟侥Ｊ斤拷锟斤拷锟斤拷
extern MOTOR_STRUCT 			gMotorInfo;		//锟斤拷锟斤拷锟较�
extern MOTOR_EXTERN_STRUCT		gMotorExtInfo;	//锟斤拷锟斤拷锟秸癸拷锟较拷锟绞碉拷锟街碉拷锟绞撅拷锟�
extern MOTOR_EXTERN_STRUCT		gMotorExtPer;	//锟斤拷锟斤拷锟秸癸拷锟较拷锟斤拷锟矫粗碉拷锟绞撅拷锟�
extern RUN_STATUS_STRUCT 		gMainStatus;	//锟斤拷锟斤拷锟斤拷状态
extern BASE_COMMAND_STRUCT		gMainCmd;		//锟斤拷锟斤拷锟斤拷
extern MAIN_COMMAND_EXTEND_UNION gExtendCmd;   //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷展
extern SUB_COMMAND_UNION         gSubCommand;	//锟斤拷锟斤拷锟斤拷锟街结构
/************************************************************/

/**********锟斤拷锟斤拷为锟酵碉拷锟斤拷锟斤拷锟斤拷锟斤拷锟借定锟斤拷锟斤拷锟斤拷锟斤拷******************/
extern BASE_PAR_STRUCT			gBasePar;	//锟斤拷锟斤拷锟斤拷锟叫诧拷锟斤拷
extern COM_PAR_INFO_STRUCT		gComPar;	//锟斤拷锟斤拷锟斤拷锟斤拷
extern MOTOR_POWER_TORQUE       gPowerTrq;
//extern PMSM_FLUX_WEAK_STRUCT    gPmFluxWeak;  //锟斤拷锟脚匡拷锟狡诧拷锟斤拷
extern PM_POS_EST_STRUCT       gPMPosEst;
/**********************锟斤拷锟斤拷模锟斤拷锟斤拷锟斤拷锟斤拷锟�**********************/
extern CPU_TIME_STRUCT			 gCpuTime;
extern MOTOR_DEBUG_DATA_RECEIVE_STRUCT     gTestDataReceive;//预锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟街碉拷锟皆碉拷锟斤拷锟斤拷
extern MOTOR_DEBUG_DATA_DISPLAY_STRUCT     gTestDataDisplay;//预锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷示锟斤拷锟斤拷锟斤拷锟街碉拷锟斤拷锟斤拷锟斤拷

/************************************************************/
/**********************锟斤拷锟斤拷为锟斤拷锟斤拷锟斤拷锟斤拷************************/
extern Uint const gDeadBandTable[];
extern Uint const gDeadCompTable[];
extern Uint const gInvCurrentTable220S[];
//extern Uint const gInvCurrentTable220T[];
extern Uint const gInvTypeTable380To220T[];
extern Uint const gInvCurrentTable380T[];
extern Uint const gInvCurrentTable690T[];
extern Uint const gInvCurrentTable1140T[];
extern Uint const gInvCurrentTable220T[];
extern Uint  const gInvVoltageInfo220S[]; 
extern Uint  const gInvVoltageInfo220T[];
extern Uint  const gInvVoltageInfo380T[];
extern Uint  const gInvVoltageInfo480T[];
extern Uint  const gInvVoltageInfo690T[]; 
extern Uint  const gInvVoltageInfo1140T[];
/**********************锟斤拷锟斤拷锟矫憋拷锟斤拷*************************/
extern s16 * pVD1;
extern s16 * pVD2;
extern s16 * pVD3;
extern s16 * pVD4;
extern s16 * pVD5;
extern s16 * pVD6;
extern s16   startSave;

/**********************锟斤拷锟皆猴拷锟斤拷说锟斤拷*************************/
extern void SaveDebugData16(Uint);
extern void SaveDebugData32(unsigned long);
extern void ResetDebugBuffer(void);

/**********************锟斤拷锟斤拷锟斤拷锟叫断筹拷锟斤拷说锟斤拷*********************/
#ifdef TARGET_GS32
extern __interrupt void ADC_Over_isr(void);
extern __interrupt void EPWM1_TZ_isr(void);
extern __interrupt void EPWM1_zero_isr(void);
extern __interrupt void PG_Zero_isr(void);
extern __interrupt void EPWM2_TZ_isr(void);
#else
extern interrupt void ADC_Over_isr(void);
extern interrupt void EPWM1_TZ_isr(void);
extern interrupt void EPWM1_zero_isr(void);
extern interrupt void PG_Zero_isr(void);
extern interrupt void EPWM2_TZ_isr(void);
#endif
/*********************系统锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷*********************/
extern void InitSysCtrl(void);   
extern void InitInterrupt(void);   
extern void InitPeripherals(void);   
extern void InitForMotorApp(void);
extern void InitForFunctionApp(void);   
extern void SetInterruptEnable(void);
extern void EnableDog(void);
extern void DisableDog(void);
extern void KickDog(void);
extern void ResetADCEndIsr(void);
extern  void SetADCEndIsr(void (*p_mADCIsr)());
extern void InitSetAdc(void);

/************************************************************
锟斤拷锟斤拷锟斤拷锟斤拷 END
************************************************************/

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================

