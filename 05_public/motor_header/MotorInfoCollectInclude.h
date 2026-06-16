/***************************************************************
文件功能：模拟量采样
文件版本：VERSION 1.0
最新更新：2009.09.27
************************************************************/
#ifndef MOTORINFO_COLLECT_INCLUDE_H
#define  MOTORINFO_COLLECT_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "MotorInclude.h"
#include "MotorDefine.h"
// // 结构体定义 
typedef struct CUR_EXCURSION_STRUCT_DEF{
	long	TotalIu;
	long	TotalIv;
	int		Iu;                             // 去零漂前
	int		Iv;
    long    IuValue;                        //用于参数辨识
    long    IvValue;                        //用于参数辨识
	int		ErrIu;				            //U相零漂大小
	int		ErrIv;				            //V相零漂大小
	int  	Count;
	int  	EnableCount;
	int  	ErrCnt;
#if (SOFTSERIES == MD500SOFT)
	int		Iw;
	int		ErrIw;				            //W相零漂大小
	long	TotalIw;
	Ulong	TotalIb;
	Uint    Ib;
	Uint	ErrIb;
	int  	IbCount;
	int		IbOkFlag ;
	Uint    IbExcursionOkFlag;
#endif

}CUR_EXCURSION_STRUCT;                  //检测零漂使用的结构

//	MD500的温度检测范围有负值，所以温度的变量要定义成有符号类型。
#if (SOFTSERIES == MD500SOFT)
typedef struct TEMPLETURE_STRUCT_DEF{
	Uint	TempAD;				            //AD获取值，由于温度查表
	int	    Temp;				            //用度表示的实际温度值
	int	    TempBak;			            //用度表示的实际温度值
	Uint	ErrCnt;
	Uint    OverTempPoint;                  //变频器过温点
}TEMPLETURE_STRUCT;                     //和变频器温度相关的数据结构
#else
typedef struct TEMPLETURE_STRUCT_DEF{
	Uint	TempAD;				            //AD获取值，由于温度查表
	int	Temp;				            //用度表示的实际温度值
	int	TempBak;			            //用度表示的实际温度值
	Uint	ErrCnt;
	int OverTempPoint;                  //过温点
}TEMPLETURE_STRUCT;                     //和变频器温度相关的数据结构
#endif

typedef struct AI_STRUCT_DEF {
	Uint 	gAI1;
	Uint 	gAI2;
    Uint    gAI3;

    Ulong   ai1Total;
    Ulong   ai2Total;
    Ulong   ai3Total;
#if(AIRCOMPRESSOR == 1)
    Uint    gAI4;
	Uint    gAI5;
    Ulong   ai4Total;
	Ulong   ai5Total;
#endif
    int     aiCounter;
}AI_STRUCT;	//
typedef struct ADC_ADJ_STRUCT_DEF{
    Uint XL;
    Uint XH;
    Uint     YL;
    Uint     YH;
    Ulong YLTotal;
    Ulong YHTotal;
    Uint     Slope;
    int     Offset;
    Uint     SlopePU;//q14
    Uint     Ticker;
}ADC_ADJ_STRUCT; // ADC校正结构体
// // 供外部引用变量声明 
extern ADC_STRUCT				gADC;		//ADC数据采集结构
extern UDC_STRUCT				gUDC;		//母线电压数据
extern IUVW_SAMPLING_STRUCT	    gCurSamp;
extern UVW_STRUCT				gIUVWQ12;	//定子三相坐标轴电流
extern UVW_STRUCT_Q24           gIUVWQ24;   //Q24格式的三相定子电流
extern ALPHABETA_STRUCT		    gIAlphBeta;	//定子两相坐标轴电流
extern ALPHABETA_STRUCT		    gIAlphBetaQ12;	//定子两相坐标轴电流
extern MT_STRUCT				gIMTQ12;    //MT轴系下的电流
extern MT_STRUCT                gIMTSetQ12;
extern MT_STRUCT_Q24            gIMTQ24;
extern AMPTHETA_STRUCT			gIAmpTheta;	//极坐标表示的电流
extern LINE_CURRENT_STRUCT		gLineCur;	
extern CUR_EXCURSION_STRUCT	    gExcursionInfo;//检测零漂使用的结构
extern TEMPLETURE_STRUCT		gTemperature;
extern AI_STRUCT				gAI;
extern ADC_ADJ_STRUCT           gADCAdjust;
extern PID_STRUCT               gWspid;

// // 供外部引用函数声明 
void GetCurExcursion(void);
void GetUDCInfo(void);
void ADCProcess(void);
void GetCurrentInfo(void);

#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================


