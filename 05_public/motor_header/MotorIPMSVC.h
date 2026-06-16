/***********************************Inovance***********************************
功能描述（Function Description）:
最后修改日期（Date）：
修改日志（History）:（以下记录为第一次转测试后，开始记录）
	作者 		时间 		更改说明
1 	xx 		xxxxx 		xxxxxxx
2 	yy 		yyyyy 		yyyyyyy
************************************Inovance***********************************/
#ifndef MOTOR_IPM_SVC_INCLUDE_H
#define MOTOR_IPM_SVC_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DataTypeDef.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct PMSM_ROTOR_POS_EST_STRUCT_DEF{
	u16		Ka;                 //Q7   
	u16     Kb;		            //Q4   	
	u16		Kc;                 //Q10  固定值768	 
    s32     TDivLdCoef;         //Q10
    s32     TDivLqCoef;
    s32   	KiForEmf;			//Q17
	s32		KiForSpeed;			//Q15
	s32		InvOfKf;            //Q10    
    s32     FcSetPer;           //载波频率标么值 Q10格式
    s32 	TsPer;              //时间标么值 Q20    
	//s16     Id;
	s32  	IdLast;             //D轴电流上次采样值，Q12
	//s16     Iq;
	s32 	IqLast; 
	//s16     Ud;
	s32  	UdLast;             //D轴电压上次计算值，Q12
	//s16     Uq;
	s32 	UqLast;
    s32 	IdEstValue;         //Id估计值 Q12		
	s32 	IqEstValue;		
    s32 	DetaId;             //估算值与真实值的偏差，Q12	
	s32 	DetaIq;	    
	s32   	BmfEstValue;        //反电动势估算值，Q12
	s32   	BmfEstValueLast;    
	s32 	SpeedEstValue;	    //速度估算值，Q15
    s32 	SpeedEstValueLast;    
	s32 	SpeedEstValueLpf;      
	s32 	SvcRotorSpeed;      //用于速度环的运行频率估计值，Q15       
	s32 	SvcRotorPos;        // 转子位置估计值，Q32
    u16     FcLow;
	u16     FcLowPoint;
	u16 	SvcSpeedLpfTs;	     //速度滤波时间常数
	u16 	SvcSpeedLpfCoef;	 //速度滤波系数	
	u16     FcSetlpf;            //滤波后的载波频率
    s16     SvcIdMaxForLowSpeed; //低速时D轴电流最大值 百分比 电机额定电流为基值 范围 0至80
    s16     SvcIdSetForLowSpeed; //低速时D轴电流给定值 Q12 电机额定电流为基值 
    u32     TimeBak;
    u32     SampleTime;
    u16     FcCoff;              //一个载波周期调整两次和调整一次的系数

    s16     PosError;
    s16     SpeedError;

    s16     SvcLowSpeedMode;     //低频运行方式。0- 正常的SVC, 1-只在减速时有效,2-低于某一频率值即为有效，也就是对加速，恒速均如此
    s16     ImBrake;
    s16     BrakeFreq;
    s16     BrakeFlagD;         // 1-低频运行处理方式有效
    s32     CurFreqSetReal;         // 0.0001Hz
    s16     Step;               // // 0.0001Hz  每0.5ms变化步长
	u16     SpeedCheckEnable;  // 开启SVC速度跟踪模式
	s32     Ud;
    s32     Uq;
	u16     FCMode;
}PMSM_ROTOR_POS_EST_STRUCT;

extern void PmsmSvcCalFc(void);
//extern void ResetParForPmsmSvc(void);
extern void PmsmSvcCtrl(void);
extern void PmsmSvcCalImForLowSpeed();
extern void ResetParForPmsmSvc(void);

extern PMSM_ROTOR_POS_EST_STRUCT   gPmsmRotorPosEst;
#define SvcSetRotorPos(x)   gPmsmRotorPosEst.SvcRotorPos = (((s32)(x))<<16)    //重置gPmsmRotorPosEst.SvcRotorPos

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

/******************************* END OF FILE***********************************/

