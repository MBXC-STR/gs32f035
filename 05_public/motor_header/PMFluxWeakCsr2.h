/***************************************************************
文件功能：同步机弱磁算法头文件
文件版本： 
最新更新：2015.04.15

************************************************************/
#ifndef PM_FLUX_WEAK_CSR_H
#define PM_FLUX_WEAK_CSR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorDefine.h"
#include "SubPrgInclude.h"
#include "MotorInclude.h"
/************************************************************/
/************************数据结构定义************************/



#ifdef __cplusplus
}
#endif /* extern "C" */
typedef struct PM_CSR2_DATA_DEF
{
	long    Kp;
	long    Ki;
	long    KiM;
	long    KFreq;
	long 	DeltTMax;
	long 	DeltTMin;
	long    VoltMax;

	long    MaxOutVoltPer;
	long    DeleteVMax;
	long    DeleteVm;
	int     ImMin;

	long 	DeltM;
	long 	DeltT;
	long    PhaseMax;
	long    PhaseMin;
	int     FreqSyn;
	long    It;
	long    Im;

	int     VoltOut;
	int     PhaseOut;
	long 	UMOut;
	long    UTOut;
	long    UMTotal;
	long    UTTotal;
	long    UMComp;
	long    UTComp;

	int     OutFlag;
	int     OutFlag1;
	int     Csr1Flag;
	int     Csr1Flag1;
	int     ITSet;
	int     CSREnable;
    int     FluxWeakFlag;
}PM_CSR2_DATA;

typedef struct UVW_VOLT_STRUCT_DEF{					
	int  	U;					
	int  	V;
	int  	W;
//    long     Zero;
	int  	Alph;					
	int  	Beta;
    int     UdQ;
    int    UqQ;
}UVW_VOLT_STRUCT;

extern PM_CSR2_DATA    gPmCsr2;
extern UVW_VOLT_STRUCT gVoltUVW;


extern void PMCsr2(void);
extern void CalUVWVoltSet(int Phase);
#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================

