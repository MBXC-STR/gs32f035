/***************************************************************
文件功能：开环矢量控制
文件版本：VERSION 1.0
最新更新：2009.09.27
************************************************************/
#ifndef MOTOR_SVC_INCLUDE_H
#define MOTOR_SVC_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MotorInclude.h"
/************************************************************/
/***********************结构体定义***************************/
typedef struct SVC_FLUX_CAL_STRUCT_DEF {
	long 	FilterTime;
	Uint 	SampleTime;
}SVC_FLUX_CAL_STRUCT;	//计算磁通用的变量集合

typedef struct FLUX_STRUCT_DEF {
	Uint	Amp;
	Uint	Theta;
}FLUX_STRUCT;	//磁通观测结果用的变量集合

/************************************************************/
//	同频波动滤波
typedef struct  {
	long  PutIn_Q24;
	long  Delta_Q24;
	long  PutOut_Q24;
	long  PutOut_II_Q24;
	long  PutOutLast_Q24;
	long  PutOut_I_Q24;
	long  Ts_Q24;
	long  K1_Q0;
	long  K2_Q4;
	long  FreqSyn_Mpy100;
	void  (*calc)(void *);   
}P_FILTER;

#define P_FILTER_DEF {0,0,0,0,0,0,0,0,0,0,\
	(void (*)(void *))PFilter}
extern P_FILTER gPfilter_ObsIT;
void PFilter(P_FILTER *p);

/************************************************************/


#ifdef __cplusplus
}
#endif /* extern "C" */
#endif  // end of definition

