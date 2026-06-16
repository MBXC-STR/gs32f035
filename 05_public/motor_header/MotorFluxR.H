/***************************************************************
文件功能：异步电机转子磁通观测
文件版本：VERSION 1.0
最新更新：2011.06.1
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

typedef struct FLUX_STRUCT_DEF{
    Ulong   Amp;
	long    Theta;
}FLUX_ROTOR_STRUCT;
/************************************************************/
/*******************供外部引用变量声明***********************/
//extern MT_STRUCT                gABVoltSet;		//αβ坐标轴下定子给定电压
//extern ALPHABETA_STRUCT		    gABFluxS;		//αβ坐标轴下定子磁通
//extern ALPHABETA_STRUCT		        gABFluxR;		//αβ坐标轴下转子磁通
//extern FLUX_ROTOR_STRUCT            gFluxR;
//extern ALPHABETA_STRUCT		    gABFluxR_obs;	    //αβ坐标轴下转子磁通观测值
//extern ALPHABETA_STRUCT		    gABFluxS_obs;	    //αβ坐标轴下转子磁通观测值
//extern ALPHABETA_STRUCT		    gABFluxR_cmd;

//extern SVC_FLUX_CAL_STRUCT		gCalFlux;		
//extern PID_STRUCT_LONG          gVoltSVCCalPID;
//extern PID_STRUCT               gSvcVoltAdj;
/************************************************************/
#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

