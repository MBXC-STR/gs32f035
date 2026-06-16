/***************************************************************************
文件说明：
文件功能：
文件版本：
最新更新：
更新日志：
***************************************************************************/
#ifndef MOTOR_FLYING_START_H
#define MOTOR_FLYING_START_H

#ifdef __cplusplus
extern "C" {
#endif


//***************************************************************************
#include "MotorInclude.h"
#include "SystemDefine.h"
#include "MotorDefine.h"


//***************************************************************************
#define	FLYING_START_TIME_BASE 1200
/***********************结构体定义***************************/
typedef struct PMSM_FLYING_START_STRUCT_DEF{
	u16		Step;							// 转速跟踪主步骤
	u16		SubStep;						// 转速跟踪辅步骤
	s16     CurIU;							// 短路U相电流
    s16     CurIV;							// 短路V相电流
	s16     CurIW;							// 短路W相电流
	s16		CurRemU[3];						// 用于记录三次短路电流峰值
	s16		CurRemV[3];						 
	s16		CurRemW[3];						 
	s16		CurRemAlpha[3];				// 三次短路电流进行坐标变换的结果
	s16		CurRemBeta[3];
	s16		ThetaAB[3];						// 三次短路电流在Alpha - Beta 轴系下的角度
	s32		Tsc;							// 短路时间
	s32		Ts[2];							// 两个短路时间间隔, 最大71.58s * 2 * 48
	u16		CurLimit;						// 实际电流限幅值, Q12
	u16		CurLimitAdj;					// 功能码设定的电流限幅值, 电机电流百分比
	u16		PWMTs;							// 赋值给PWM周期寄存器的值
	u16		Flag;							// 转速跟踪计算完成标志, 0-未完成, 1-已完成
	s32		Freq[2];						// 两个脉冲算出的频率, 0.01Hz
	s32		Freq01;							// 转速跟踪辨识的频率, 0.01Hz
	s32		FreqPer;						// 转速跟踪辨识的频率, Q15
	s16		Theta;							// 转速跟踪辨识的角度, Q16
	u32		DetaT;							// 辨识完成与实际发波间隔时间, 用于进行角度补偿
	s16		ThetaEncoder;					// 编码器读到的角度, 用于验证算法
	s32		BmfEstValue;					// 转速跟踪估计反电势, Q12
	s16		FcChgFlag;						// 辨识结束后载频, 电流环, 闭环电流处理标志, 1 - 需要处理, 0 - 无需处理
	s16		FcChgCnt;						// 辨识结束后特殊处理的时间计数, 考虑默认为25, 50ms
	s16		StartStop;						// 转速跟踪结束后进入停机/运行状态的标志, 0 - 停机, 1 - 运行
	u16		PhaseLoseCnt;					// 输出缺相判断次数计数
	s32		Delay1;							// 第一二次短路时间间隔基值
	s32		Delay2;							// 一二次、二三次短路时间间隔差基值
	u16		Record;							// 计算中返回原因记录
	u16		KpRatio;						// 起动时电流环Kp调整系数
	u16		KiRatio;						// 起动时电流环Ki调整系数
}PMSM_FLYING_START_STRUCT;

/* Private function prototypes -----------------------------------------------*/
extern void RunCaseFlyingStart(void);
extern void SynFlyingStart(Ulong FullFreq01, Uint InvCurrent, Uint MotorCurrent);
extern void SynFlyingStartCalc(Ulong FullFreq01);
extern void SynFlyingStartSetTs(void);
extern void ADCEndIsrFlying_Start(void);
extern void FlyingStratPreparePar(void);
extern void FlyingStartParaAdjust(void);
extern void FlyingStartFcDeal(void);
extern void FlyingStartInitDeal(void);
//***************************************************************************
extern PMSM_FLYING_START_STRUCT gFlyingStart;

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

/*===========================================================================*/
// End of file.
/*===========================================================================*/



