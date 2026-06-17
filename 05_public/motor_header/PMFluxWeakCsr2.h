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
	s16     ImMin;

	long 	DeltM;
	long 	DeltT;
	long    PhaseMax;
	long    PhaseMin;
	s16     FreqSyn;
	long    It;
	long    Im;

	s16     VoltOut;
	s16     PhaseOut;
	long 	UMOut;
	long    UTOut;
	long    UMTotal;
	long    UTTotal;
	long    UMComp;
	long    UTComp;

	s16     OutFlag;
	s16     OutFlag1;
	s16     Csr1Flag;
	s16     Csr1Flag1;
	s16     ITSet;
	s16     CSREnable;
    s16     FluxWeakFlag;
}PM_CSR2_DATA;

typedef struct UVW_VOLT_STRUCT_DEF{					
	s16  	U;					
	s16  	V;
	s16  	W;
//    long     Zero;
	s16  	Alph;					
	s16  	Beta;
    s16     UdQ;
    s16    UqQ;
}UVW_VOLT_STRUCT;

extern PM_CSR2_DATA    gPmCsr2;
extern UVW_VOLT_STRUCT gVoltUVW;


extern void PMCsr2(void);
extern void CalUVWVoltSet(s16 Phase);
#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================

