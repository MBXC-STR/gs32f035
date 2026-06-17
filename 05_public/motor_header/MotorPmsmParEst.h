/***************************************************************************
文件说明： 
文件功能：
文件版本：V1.0
最新更新：2009.08.26
更新日志：
zhuozhe : linxib
***************************************************************************/
#ifndef MOTOR_PM_INCLUDE_H
#define MOTOR_PM_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif


//同步机初始磁极位置推断方式选择
#define INIT_POS_NULL			0		//硬件检测方式
#define INIT_POS_VOLT_PULSE		1		//电压脉冲法


/*******************结构体定义******************************************/
typedef struct IPM_ZERO_POS_STRUCT_DEF {
	s16	FirstPos;				//
	Uint	Cnt;
	//Uint	ABAngleBak;
	long	QepBak;
	s16		DetectCnt;
    //s16     UvwCnt;
	long	TotalErr;				//上次下电时候的角度

    Uint    CurLimit;
    Uint    time;
	Uint    zFilterCnt; 
	Uint    Flag;
}IPM_ZERO_POS_STRUCT;          //永磁同步电机检测编码器零点位置角的数据结构

typedef struct IPM_INITPOS_PULSE_STRUCT_DEF {
	Uint	Waite;
	Uint	Step;					// 标志， 设置Step为1起动静态辨识，结束后为0
	Uint	Flag;					// 1已经经过磁极初始位置检测的标志
	Uint    OkFlag;                 // 初始位置角辨识结束，避免重复辨识
	Uint	PeriodCnt;
	Uint	Section;
	Uint	PWMTs;
	Uint	PWMTs4L;
	Uint	PWMTsBak[12];
	Uint	InitPWMTs;
//	Uint	LPhase[3];				// 顺便检测出电机相电感
	long	LPhase[3];				// 防止溢出
	Uint    Ld;						// 机型 >22，单位0.001mH，机型 <22，单位0.01mH
	Uint    Lq;						// 机型 >22，单位0.001mH，机型 <22，单位0.01mH
//	Uint	CurLimit;
	Uint	CurLimitSmall;
	Uint	CurLimitBig;
	u16     CurLimitAdjSmall;            // 初始位置角检测电流系数
	u16     CurLimitAdjBig;            // 初始位置角检测电流系数
	s16		Cur[12];
    s16     PhsChkStep;             // 同步机缺相检测步骤
    Uint    InitTestFlag;           // 启动前是否需要磁极位置检测 0-检测，1-不检测,2-上电第一次检测
    u16     ErrorIintPosEnable;
    s16     CurBase;
	//s16     CurBaseShow;
	s16     CurIU;
    s16     CurIV;
	s16     CurIW;
	Uint	WidthFlag;
	s16		InitPosSrc;
	s16		CurRem[24];
}IPM_INITPOS_PULSE_STR;        //永磁同步电机上电初始位置检测的数据结构(电压脉冲法)

typedef struct PMSM_EST_PARAM_DEF
{
	Uint    IdKp;               // 电流环pi参数
	Uint    IdKi;
	Uint    IqKp;
	Uint    IqKi;
	u16     IsKpK;              // PI参数修正量
	u16     IsKiK;

    Uint    CoderPos_deg;       // 编码器零点位置角；
    Uint    EstZero;            // 带载辨识时使用的零点位置角
    
    //Uint    UvwZeroPhase_deg;   // UVW信号零点位置角；
    Uint    UvwDir;             // 编码器绝对位置方向正反， UVW编码器表示UVW信号正反
    Uint    UvwZeroAng;         // uvw 零点角度
    Uint    UvwZeroAng_deg;

    Uint    UvwZPos;
	Uint    SynTunePGLoad;  //同步机带载辨识状态
}PMSM_EST_PARAM_DATA;

typedef struct PMSM_EST_BEMF_DEF
{
    Uint    TuneFreq;           // 记录反电动势辨识运行频率
    Ulong   TotalId1;           // 积分第一点电流值
    Ulong   TotalId2;           // 积分第二点电流值
    Ulong   TotalVq1;           // 积分第一点电压值
    Ulong   TotalVq2;           // 积分第二点电压值

    s16     IdSet;              // M 轴电流设定
    s16     IdSetFilt;
    s16     IqSet;
    s16     TuneFreqSet;        // 实时的速度给定
    s16     TuneFreqAim;
    Ulong   AccDecTick;         // 加减速时间, 2ms Ticker个数
    s16     FreqStep;
    long    FreqRem;
    
    Uint    BemfVolt;           // 反电动势， 标么化即为转子磁链
    Uint    BemfVoltDisPlay;
    Uint    BemfVoltDisPlayLast;
    Uint    BemfVoltOnline;
	Uint    BemfVoltFilter;
	Ulong   BemfVoltDisPlayTotal;
    Uint    Cnt;
}PMSM_EST_BEMF;


//***************************************************************************
extern IPM_ZERO_POS_STRUCT		gIPMZero;
extern IPM_INITPOS_PULSE_STR	gIPMInitPos;
extern PMSM_EST_PARAM_DATA		gPmParEst;
extern PMSM_EST_BEMF            gEstBemf;


//***************************************************************************
extern void SynInitPosDetSetPwm(Uint Section);
extern void SynTuneInitPos(void);
extern void SynInitPosDetect(void);
extern void SynTunePGZero_No_Load(void);
extern void SynTunePGZero_Load(void);
extern void SetIPMPos(Uint Pos);
extern void SetIPMRefPos(Uint Pos);
extern void SynTuneBemf(void);
extern void PrepareParForRun(void);
extern void PrepareParForTuneBeforeRun(void);
extern void EndOfParIdentify(void);
extern void BeforeRunRsIdentify(void);


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

/*===========================================================================*/
// End of file.
/*===========================================================================*/

