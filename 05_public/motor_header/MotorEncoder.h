/***************************************************************************
文件说明：
文件功能：
文件版本：
最新更新：
更新日志：
***************************************************************************/
#ifndef MOTOR_ENCODER_INCLUDE_H
#define MOTOR_ENCODER_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif


//***************************************************************************
#include "MotorInclude.h"
#include "SystemDefine.h"
#include "MotorDefine.h"


//***************************************************************************
#define C_MAX_C_ZERO            200             //正余弦编码器Z信号到达时候C信号最大电平
#define C_MIN_D_ZERO            19000           //正余弦编码器Z信号到达时候D信号最小电平
#define C_UVW_FOR_ZERO          5
#define C_UVW_BACK_ZERO         6

#define DIR_FORWARD   1
#define DIR_BACKWARD  -1
#define DIR_ERROR     0

//#define	RCK	(GpioDataRegs.GPBDAT.bit.GPIO34) // 与74HC594/165的配合
#define GetQepCnt()			(*EQepRegs).QPOSCNT
#define SetQepCnt(X)		    (*EQepRegs).QPOSCNT = X;


// UVW编码器的UVW信号定义,通过模拟量ABC的电平判断UVW信号的高低(UVW信号也被抬升了1.5V)
#define Get_UVW_PG_U()	            (((u16)UVW_PG_U > 39719)?4:0)   //bit2 //65536对应3.3V ,39719对应2V
#define Get_UVW_PG_V()	            (((u16)UVW_PG_V > 39719)?2:0)   //bit1
#define Get_UVW_PG_W()	    	    (((u16)UVW_PG_W > 39719)?1:0)   //bit0
// 省线方式UVW编码器的UVW定义 = ABZ端口
#define EXT_UVW_PG_U()	         GpioDataRegs.GPADAT.bit.GPIO20
#define EXT_UVW_PG_V()	         GpioDataRegs.GPADAT.bit.GPIO21
#define EXT_UVW_PG_W()	    	 GpioDataRegs.GPADAT.bit.GPIO23

#define C_Z_RESET_POS_LIMIT         0       // PG_DATA_STRUCT结构的ZResetFlag幅值参数
#define C_Z_RESET_POS_NOLIMIT       1   
#define C_Z_DONT_RESET_POS          2
#define C_MAX_DETA_POS_Z            (65536*45/360)      // 基准Z信号到达，期望复位角度和当前角度偏差的阀值
#define C_MAX_DETA_POS_ABS          (65536*40/360)      // 通过UVW信号检测的绝对位置和当前位置偏差的阀值
/***********************结构体定义***************************/
typedef enum PG_TYPE_ENUM_STRUCT_DEF{  
    PG_TYPE_ABZ,                //普通ABZ编码器
    PG_TYPE_UVW,                //带UVW信号的ABZ差分编码器
    PG_TYPE_RESOLVER,           //旋转变压器
    PG_TYPE_SC,                 //带正余弦信号的ABZ差分编码器 
    PG_TYPE_SPECIAL_UVW,        //省线方式UVW编码器
    PG_TYPE_NULL=100            //没有接编码器
}PG_TYPE_ENUM_STRUCT;       //编码器类型

typedef enum QEP_INDEX_ENUM_STRUCT_DEF{
    QEP_SELECT_1,               //选定外设QEP1用于测速
    QEP_SELECT_2,               //选定外设QEP2用于测速
    QEP_SELECT_PULSEIN,         //使用PULSE输入测速
    QEP_SELECT_NONE=100         //未选定用来测速的QEP模块
}QEP_INDEX_ENUM_STRUCT;

typedef struct PG_DATA_STRUCT_DEF {
	Uint 	PGType;
    QEP_INDEX_ENUM_STRUCT   QEPIndex;   //当前测速使用的QEP模块，要求在280xDSP的两个QEP模块之间切换

    Uint    PGMode;                     //区分增量式编码器和非增量式。0为增量式
    Uint    PGTypeGetFromFun;           //功能传递的PG卡类型
	Uint 	PulseNum;				    //编码器线数
    Uint    SpeedDir;                   //ABZ,UVW编码器代表AB信号，旋变代表旋变信号
    Uint    SpeedDirLast;               //
    Uint    PGDir;                      //辨识得到的编码器速度正反向
    Uint    PGErrorFlag;                //辨识得到的编码器错误信息 0-正常;1-未检测到编码器;2-编码器线数设定错误 

    Uint    imPgEstTick;                // im 编码器辨识计数器
    s16     imDirAdder;
    long    imFreqErr;                  // 积分编码器测速误差
    long    imFrqEncoder;               // 
    u16 	PGTypeBak;              /*编码器类型*/
    //s16     ZeroPos;                /*编码器零点位置(编码器安装角度,同步机用)*/
    s32     RefPos;                 /*编码器相对位置,折算到电角度，基准位置到达后不复位角度，用于速度环*/
    s32     RefPosLast;             /*用于记录上一拍的电角度*/
    s16     RotorPos;               /*编码器绝对位置,折算到电角度，同步机控制用*/
	s16     ZRotorPos;              /*z信号到达时编码器的位置*/
	u32     ZQepBak;                /*为Z信号到达记录脉冲数*/
	s16     ZRemainAbs;             /*为Z信号到达记录脉冲余数*/
    u16     Poles;                  /*编码器极对数(只对旋变有用)*/
    u16     PGErrDetTime;           /*编码器断线检测时间*/
    s16     MoveDircetion;              //记录功能设定的运行方向
    u16     DiscDetectDelayTime;    /*编码器断线检测时间*/
	u16     Cnt;
}PG_DATA_STRUCT;	            //编码器相关参数

typedef struct ROTOR_SPEED_SMOOTH_DEF {
    s16     LastSpeed;
    s16     SpeedMaxErr;
}ROTOR_SPEED_SMOOTH;

typedef struct ROTOR_SPEED_STRUCT_DEF {
	s16 	SpeedApply;			//实际使用转子速度
	s16     SpeedBigFilter;     // 用于键盘显示计算的速度
	s16 	SpeedEncoder;		// 通过编码器检测到的转子频率, 传动比折算后
	Uint    OverSpeedEnable;    //反向超速使能
	s16     SpeedTemp;          // 折算前编码器测速值
	s16		SpeedApplyFilter;
	s16		Flag;
    s16     DetaTimer;
    s16     DetaPhase;
    Uint    MTZeroCnt;		//
	//SVC测速变量
	s16		SvcLastFluxPos;
	s16		SvcSynSpeed;
	s16		SvcWs;
	s16		SvcRotorSpeed;
	Uint	SvcSignal;
	Ulong	SvcLastTime;
	Ulong	Timer;

    Uint    IntCnt;
    Uint    IntNum;
    
    Uint    TransRatio;         //电机测速传动比
	Uint	FirstCnt;
	s16     SpeedFeedQ12;           //速度检测值  Q15格式
	s32     SpeedFeedFilter;        //滤波后的速度检测值  Q24格式
    s32     SpeedFeed;              //速度检测值  Q24格式
	s32     SpeedFeedLast;          //记录上次的速度检测值
    u32     PulseNum;               //测速用的编码器线速
    u32     Pos;                    //当前位置
    u32     Time;                   //当前时间
    s16     DetaPos;                //位置偏差
	s16     DetaPosC;               //位置偏差记录值
	s16     DetaPosLast;            //上次的位置偏差
    u32     DetaTime;               //时间偏差
	u32     DetaTimeLast;           //上次的时间偏差
    u32     DetaTimeAdd;            //第一次捕捉到脉冲需要增加的时间
    u16     CapTimeLast;            //用于记录上次读取位置时的捕获值
    u16     CapTimeLast1;           //
    u16     PrescaFlag;
    u16     ChangeFlag;
    s32     Speed;
	s16     FreWindow;
	u16     SvcSpeed;              // svc模式下是否开通编码器测速
}ROTOR_SPEED_STRUCT;	//速度反馈部分数据结构

typedef struct IPM_UVW_PG_STRUCT_DEF {
	Uint	UVWData;
	Uint    UVWExtData;     // 省线式编码器上电初始角度
	Uint	UVWAngle;       // UVW信号绝对位置角度, 由于编码器和电机极对数相等，因此表示电角度
	
	Uint    U_Value;
	Uint    V_Value;
	Uint    W_Value;
	Uint	ZeroCnt;

    Uint    LogicU;         // U信号逻辑
    Uint    LogicV;         // V信号逻辑
    Uint    LogicW;         // W信号逻辑

    Uint	UvwDir;                 // UVW信号正反向

    Uint    debugdeta1;
    Uint    debugdeta2;
    
    Uint    lastAgl;
    Uint    NewAgl;
    Uint    ErrAgl;
    Uint    TuneFlag;
    
    llong   TotalErr;
    Ulong   UvwCnt;

    s16     UvwZIntErr;      // Z中断uvw误差
    s16     UvwZIntErr_deg;
    
    Uint    UvwZeroPos;     // uvw的零点位置角度
    Uint    UvwZeroPos_deg;

    Uint    UvwEstStep;
    s16     UvwCnt2;        // 机械周期计数器
	u16     ErrorEnable;    // UVW故障使能
}IPM_UVW_PG_STRUCT;//永磁同步电机上UVW编码器的数据结构

typedef struct IPM_PG_DIR_STRUCT_DEF {
	s16	    ABAngleBak;
    s16     ABDirCnt;
    s16     ABDir;
	s16	    UVWAngleBak;
    s16     UVWDirCnt;
    s16     UVWDir;
	s16	    CDAngleBak;
    s16     CDDirCnt;
    s16     CDDir;
    s16     CDErr;
    s16     RtPhaseBak;
    s16     RtDirCnt;
}IPM_PG_DIR_STRUCT; //永磁同步电机上用于识别编码器接线方向的数据结构

typedef struct ROTOR_TRANS_STRUCT_DEF{
    s16     RealTimeSpeed;
	s16     ConFlag;
    Uint    SimuZBack;
    Uint    SimuZBack2;

    Uint    IntNum;
    Uint    IntCnt;

	u32 	TimeBak;				//上一次测速的基准时间
	u32     SampleTime;
	u16     SampleRTPos;
	u32	    DetaTime;
    u16     ReadPos;
	u16		RTPos;                  // 机械角度
	s32     AddPos;                // 间接累加角(异步机用)
	u16		PosBak;
    u16		PosRTBak;
	s16		DetaPos;
    s16     Remain;
	s32		FreqFeed;
    s16     PosComp;
    
    u16    EstZero;
    u16    RtorBuffer; 
    u16    Poles;  
    u16    PolesRatio;  // 旋变极对数比例
    u16    RtRealPos;          // 旋变的时候角度，加了补偿角度之后

    s16    AbsRotPos;          // 旋变绝对位置，0-4096
	s16     PgPos;
    Ulong   QepBak; 
    u16    RtError;        // 旋变故障（主要是未接编码器）   
}ROTOR_TRANS_STRUCT; 	// 旋变数据结构


/* Private typedef -----------------------------------------------------------*/


#define QEPSetRefPos(x)     gPGData.RefPosLast = ((s32)(x)<<8)          //重置gPGData.RefPos
#define QEPSetRotorPos(x)   gPGData.RotorPos = x                    //重置gPGData.RotorPos
#define QEPGetRefPos()      (gPGData.RefPos)                        //返回gPGData.RefPos
#define QEPGetRotorPos()    (gPGData.RotorPos)                      //返回gPGData.RotorPos


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/
#define MIN_DETA_QEP            4                       /*测速最少捕获的脉冲个数*/

#define C_SPEED_M_METHOD        0                       /*测速方法选择，作为VCGetFeedBackSpeed()函数的输入值*/
#define C_SPEED_MT_METHOD       1
#define OLDSPEED
/* Private function prototypes -----------------------------------------------*/
extern void    ResetPGType(void);
extern void    ResetSpeedGetReg(void);
extern void    GetFeedRotorSpeed(void);
extern void    GetFeedSpeedQEP(void);
extern s32     CalFeedSpeed(s16 DetaPos, u32 DetaTime);
extern void    QEPGetRealPos(void);              
extern void    QEPIndexIsr(void);

//***************************************************************************
extern volatile struct EQEP_REGS        *EQepRegs;
extern IPM_UVW_PG_STRUCT        gUVWPG;
extern PG_DATA_STRUCT			gPGData;
extern IPM_PG_DIR_STRUCT        gPGDir;
extern BURR_FILTER_STRUCT		gSpeedFilter;
extern CUR_LINE_STRUCT_DEF		gSpeedLine;
extern ROTOR_TRANS_STRUCT		gRotorTrans;
extern ROTOR_SPEED_STRUCT		gRotorSpeed;

extern void GetUvwPhase(void);
extern s16  JudgeABDir(void);
extern s16  JudgeUVWDir(void);
extern void GetMTTimeNum(void);
extern void GetMDetaPos(void);
extern void RotorTransCalVel(void);
extern u16  GetRotorTransPos(void);
extern void RotorTransSamplePos(void);
extern void VCGetFeedBackSpeed(void);
extern void ReInitForPG(void);
extern void InitSetQEP(void);
extern void ReadRTPos(void);
extern u16 IPMCalAbsPos(void);

extern ROTOR_SPEED_STRUCT		gRotorSpeed;
extern PG_DATA_STRUCT			gPGData;
#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

/*===========================================================================*/
// End of file.
/*===========================================================================*/

