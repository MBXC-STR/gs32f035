/************************************************************
------------该文件是通用子程序模块程序的头文件---------------
************************************************************/
#ifndef SUBPRG_INCLUDE_H
#define SUBPRG_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemDefine.h"
#include "MotorDefine.h"

//#define FilterN(x,y) (y + (x-y)/N)   一阶滤波函数， 有静差
#define	Filter1(x,total)    (x)
#define	Filter2(x,total)   (( (((Ulong)total)<<16) + (((Ulong)x)<<15) - (((Ulong)total)<<15) )>>16)
#define	Filter4(x,total)   (( (((Ulong)total)<<16) + (((Ulong)x)<<14) - (((Ulong)total)<<14) )>>16)
#define	Filter8(x,total)   (( (((Ulong)total)<<16) + (((Ulong)x)<<13) - (((Ulong)total)<<13) )>>16)
#define	Filter16(x,total)  (( (((Ulong)total)<<16) + (((Ulong)x)<<12) - (((Ulong)total)<<12) )>>16)
#define	Filter32(x,total)  (( (((Ulong)total)<<16) + (((Ulong)x)<<11) - (((Ulong)total)<<11) )>>16)
#define	Filter64(x,total) (( (((Ulong)total)<<16) + (((Ulong)x)<<10) - (((Ulong)total)<<10) )>>16)
#define Filter128(x,total) (( (((Ulong)total)<<16) + (((Ulong)x)<<9 ) - (((Ulong)total)<<9 ) )>>16)
#define Filter256(x,total) (( (((Ulong)total)<<16) + (((Ulong)x)<<8 ) - (((Ulong)total)<<8 ) )>>16)

//取绝对值的函数(适用所有数据类型)
#define abs(x)  	((x>0)?(x):-(x))
//取大值函数(适用所有数据类型)
#define Max(x,y)	((x>y)?(x):(y))
//取小值函数(适用所有数据类型)
#define Min(x,y)	((x<y)?(x):(y))

/************************************************************
	结构定义
************************************************************/
typedef struct PID_STRUCT_DEF {
	long 	Total;			//积分累加值
	long 	Out;			//输出值
	s16  	Max;			//最大值限制
	s16  	Min;			//最小值限制
	s16  	Deta;			//偏差值
	s16  	KP;				//KP增益
	s16  	KI;				//KI增益
	s16  	KD;				//KD增益
	u16     Flag;            //标志位
	s16     OutFlag;         //饱和标志位

    Uint    QP;             // KP的放大倍数 KP = KP << QP
    Uint    QI;             // KI的放大倍数
    Uint    QD;             // KD的放大倍数
}PID_STRUCT;//PID计算用的数据结构(无量纲数据结构)
typedef struct PID_STRUCT_LONG_DEF {
	llong 	Total;			//积分累加值
	llong 	Out;			//输出值
	long  	Max;			//最大值限制
	long  	Min;			//最小值限制
	long  	Deta;			//偏差值
	Uint  	KP;				//KP增益
	Uint  	KI;				//KI增益
	Uint  	KD;				//KD增益

    s16     sVoltGain;
}PID_STRUCT_LONG;//PID计算用的数据结构(无量纲数据结构)

typedef struct PID32_STRUCT_DEF{
	long  	Deta;			//偏差值(Q24.7,最大为2.0)
	long 	Out;			//输出值(Q24.7)
	long 	Total;			//积分累加值(Q24.7)
	long  	Max;			//最大值限制(Q24.7,不要超过64)
	long  	Min;			//最小值限制(Q24.7,不要超过-64)
	long  	KP;				//KP增益(Q12.19) 相当于是Q12格式
	long  	KI;				//KI增益(Q16.15) 相当于是Q16格式
}PID32_STRUCT;

typedef struct BURR_FILTER_STRUCT_DEF{					
	long 	Input;			//本次采样数据
	long	Output;			//当前使用数据
	long 	Err;			//偏差限值
	long  	Max;			//最大偏差值
	long  	Min;			//最小偏差值
}BURR_FILTER_STRUCT;//去除毛刺滤波处理的数据结构

typedef struct CUR_LINE_STRUCT_DEF{					
	s16		FilterTime;		//滤波级别
	s16 	Input;			//本次输入数据
	s16		Output;			//本次预测数据
	s16		Data[16];		//保留历史数据（最多8ms数据）
}CUR_LINE_STRUCT_DEF;//曲线拟合相关的数据结构

typedef struct FILTER_1ST_ORDER_DEF{    
    s16 x0;

    s16 taoVsTs ;       // (tao / Ts)  //越大滤波越强
    s16 resFilt ;       // 静差
}FILTER_1ST;

typedef struct VFFILTER1ST_STRUCT_DEF{
	s16     InputLast;		        //前一拍输入值
	s16	    OutLast;			    //前一拍输出值
	s32     Total;                  //由于移位造成的余数, Q15
	s16     FiltimeVsSamptime;		//滤波时间常数和采样时间比值（tao/Ts）
}VFFILTER1ST_STRUCT; //一阶惯性滤波用的数据结构，用于int型数据
/************************************************************
	基本函数定义和引用
************************************************************/
extern Uint swap(Uint);				//
//extern s16 	abs(s16);				//
extern s16 	absl(long s16);			//r 这个函数存在问题
extern s16 	qsin(s16);				//正弦函数
extern s16 	qatan(long s16);		//反正切函数
//extern s16  atan(s16 x, s16 y);		//四象限反正切函数
extern s16 user_atan(s16 x, s16 y);
extern Uint qsqrt(Ulong);			//开方函数

extern Uint GetInvCurrent(Uint);	//根据机型查询变频器额定电流函数
extern void PID(PID_STRUCT * pid);
extern void BurrFilter(BURR_FILTER_STRUCT * );
extern s16 Filter(s16 , s16 , s16 );
extern s16 VFFilter1st(s16 , VFFILTER1ST_STRUCT *pFilter1st);
extern void PID32(PID32_STRUCT * pid);
extern s16 Filter_1st(s16 x, s16 y0, FILTER_1ST *pFilStr);
#ifdef __cplusplus
}
#endif /* extern "C" */


#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================
