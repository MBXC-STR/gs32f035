/***************************************************************
�ļ����ܣ��������ģ������ͷ�ļ�.��Ҫ���õ���ȫ�ֱ����ͺ���������.
          ��������������MotorVar.c�ļ��еĶ���˳����ͬ
�ļ��汾��VERSION 1.0
���¸��£�2007.09.27
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
    �������� BEGIN
************************************************************/
/*****************����Ϊ������������*************************/
extern INV_STRUCT 				gInvInfo;		//��Ƶ����Ϣ
extern CONTROL_MOTOR_TYPE_ENUM  gCtrMotorType;  //������ͺͿ���ģʽ�����
extern MOTOR_STRUCT 			gMotorInfo;		//�����Ϣ
extern MOTOR_EXTERN_STRUCT		gMotorExtInfo;	//�����չ��Ϣ��ʵ��ֵ��ʾ��
extern MOTOR_EXTERN_STRUCT		gMotorExtPer;	//�����չ��Ϣ����ôֵ��ʾ��
extern RUN_STATUS_STRUCT 		gMainStatus;	//������״̬
extern BASE_COMMAND_STRUCT		gMainCmd;		//������
extern MAIN_COMMAND_EXTEND_UNION gExtendCmd;   //����������չ
extern SUB_COMMAND_UNION         gSubCommand;	//�������ֽṹ
/************************************************************/

/**********����Ϊ�͵����������趨��������******************/
extern BASE_PAR_STRUCT			gBasePar;	//�������в���
extern COM_PAR_INFO_STRUCT		gComPar;	//��������
extern MOTOR_POWER_TORQUE       gPowerTrq;
//extern PMSM_FLUX_WEAK_STRUCT    gPmFluxWeak;  //���ſ��Ʋ���
extern PM_POS_EST_STRUCT       gPMPosEst;
/**********************����ģ���������**********************/
extern CPU_TIME_STRUCT			 gCpuTime;
extern MOTOR_DEBUG_DATA_RECEIVE_STRUCT     gTestDataReceive;//Ԥ���������������ֵ��Ե�����
extern MOTOR_DEBUG_DATA_DISPLAY_STRUCT     gTestDataDisplay;//Ԥ����������ʾ�������ֵ�������

/************************************************************/
/**********************����Ϊ��������************************/
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
/**********************�����ñ���*************************/
extern int * pVD1;
extern int * pVD2;
extern int * pVD3;
extern int * pVD4;
extern int * pVD5;
extern int * pVD6;
extern int   startSave;

/**********************���Ժ���˵��*************************/
extern void SaveDebugData16(Uint);
extern void SaveDebugData32(unsigned long);
extern void ResetDebugBuffer(void);

/**********************�������жϳ���˵��*********************/
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
/*********************ϵͳ��ʼ����������*********************/
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
�������� END
************************************************************/

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================

