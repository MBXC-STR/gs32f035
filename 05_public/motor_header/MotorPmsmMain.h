/***************************************************************
文件功能：
文件版本：VERSION 1.0
最新更新：
************************************************************/
#ifndef MOTOR_PMSM_INCLUDE_H
#define MOTOR_PMSM_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif


//**************************************************************************
typedef struct IPM_POS_CHECK_STRUCT_DEF {
	Uint	FirstPos;				//第一次位置
	Uint	Cnt;					//计数器
	long	TotalErr;				//位置偏差累加
	Ulong	TotalErrAbs;			//位置偏差的绝对值累加
	s16     UvwStopErr;
    s16     UvwStopErr_deg;              // uvw的误差角度
    Uint    UvwRevCnt;              // 被UVW信号修正次数计数器
}IPM_POS_CHECK_STRUCT;         //永磁同步电机上电检测当前绝对位置角的数据结构

typedef struct IPM_POSITION_STRUCT_DEF {
    Uint	InitPosMethod;          //初始磁极位置检测方法
    
	Uint   	RotorPos;				//转子位置角标么值
	Uint    RotorPosBak;            // 记录上一拍转子位置值，在空载辨识时使用
	Uint	RealPos;				//转子实时角度(360.0度范围)

    Uint    InitPos;                // 磁极辨识得到的位置
    Uint    InitAngle_deg;          // 辨识得到的磁极位置，0-360.0deg

	Uint	CompPosFun;			// 从功能传递的补偿角度, 0.0~359.9
	Uint	CompPos;				// 实际的补偿角度, Q16, pu
	
	Uint	RotorZero;				//编码器零点位置角（65536范围）
	Uint	RotorZeroGet;			//编码器零点位置角(360.0度范围)
	Uint    ZeroPosLast;            // 配合ABZ编码器时手动修改零点位置角

    Uint    PowerOffPos;            // 上次掉电视的角度
	Uint	PowerOffPosDeg;			//上次下电时候的角度(360.0度范围)
	
	long	QepBak;					//上一拍QEP累加数值
	long	QepTotal;
    
	Uint    ZErrCnt;			    //Z信号错误计数器
	s16     AbzErrPos;   
    s16     AbzErrPos_deg;
    
    u16     ZSigNumSet;             //Z信号中断响应次数累积
    u16     ZSigNum;                //主循环中， 基准Z信号计数器
    
    Uint32  QepCntBak;              //Z信号中断中记录QPOSCNT的值
	u32     QepCnt;
    Uint32  QepCntPosCal;
    Uint32  QepCntPosCalBak;        //Z信号中断之前，计算转子位置使用的QPOSCNT的值
    u16     ZBakRotorPos;           //Z信号中断之前，记录的转子位置
    
    u16	    ZBakUVW;                //Z信号中断中记录UVW管脚状态
    u16     ZPosWindage;            //Z信号发生时，位置的随机变动
    s16     ZResetFlag;             // 基准Z信号到达时候，角度是否复位的标志
                                    // 0:表示角度偏差在一定范围时复位角度；
                                    // 1:任何时候无条件复位角度
                                    // 2:表示Z信号到的时候，不复位角度    
   s16     ZDetaPos;
   s16     ZBakDetaPos;
   s16     AdjustZDetaAngle;
   u16     ZIntFlag;                //用于标记Z信号到达时，主中断是否处于位置计算状态
   u16     ZSigEnable;             // Z信号校正使能标志位，1为使能
   u16     ZSigEnableApply;             // Z信号校正使能使用标志位，1使能，主要用来互锁
   s16     DetaPosShow;             // 用于显示实际位置和参考位置的偏差
   u16     LoadTuneErrorEnable;         // 带在零点位置辨识故障使能 
   u16     TuneDetaPosDisable;      // 用于屏蔽编码器线数设错报故障使能

}IPM_POSITION_STRUCT; //永磁同步电机和转子角度相关的结构

typedef struct PM_INIT_POSITION_DEF
{
	s16 SubStep;
	s16 PeriodCnt;
	s16 Section;	
	s16 CurFirst;
	s16 Cur[12];
	s16 Timer;
	u16 PWMTs;
	u16 PWMTGet;
}PM_INIT_POSITION;

typedef struct PM_FLUX_WEAK_DEF
{
    Uint    Mode;              // 弱磁模式选择 0: 不弱磁；1: 弱磁直接计算；2: 弱磁自动调整
    s16     DecoupleMode;      // 解耦模式 0: 不解耦；1: 解耦
    s16     CsrGainMode;        // 电流环参数修正模式 0: 不修正，1: 修正
    s16     CoefFlux;          // 弱磁系数
	long     VoltCoef;          // voltage coefficient (%)
    long    FluxD;             // d轴总磁链 PhiD = PhiSd + FluxRotor
    s16     FluxSd;            // PhiSd = Ld * gIMTQ12.M
	long     IqLpf;
    s16     VoltLpf;            // 输出电压滤波值
    s16     AbsFrqLpf;
    s16     IdSet;
    s16     IdMax;              // 弱磁d 轴电流限制 1%
    s16     Vd;
    s16     Vq;
	s16     AdjustId;           // 模式2 调整量
    s16     AdjustLimit;        // 模式2 调整量上限
	s16     CoefAdj;            // 模式2 调整系数 
    s16     CoefKI;             // 弱磁时电流环积分增益调整系数

    //另外增加的变量
    long    VoltMax;
    long    CurrCoef;
    long    Iq;   // Q轴电流 单位为uH ，也可当做Q24格式
    long    Omg;  // 实际角速度，Q10格式
    long    Id;   // 弱磁电流，Q12格式表么值
    long    IqFBLpf;// Q轴滤波电流,Q12格式表么值
    long    IqLimit;
    long    TorqeCurr;//程序中没有使用
    long    AdjustId1;// 程序中没有使用
    s16     CoefIdComp;// IS300 中gSendToMotorDataBuff1[14]
    long    IqErr;     // Q轴电流偏差，Q12格式表么值
    s16     IqErrAbs;
    long    OmgQ15;   // 实际频率的滤波值
    s16     Ratio;   
    s16     VoltOut;


    long    data0;
    long    data1;
    long    data2;
    long    data3;
    long    data4;
    long    data5;
    long    data6;
    long    data7;
    long    data8;
    long    data9;
    long    ud;
    long    uq;
   
        
}PM_FLUX_WEAK;

typedef struct PM_DECOUPLE_DEF{
    s16 Omeg;   // Q15
    s16 Isd;    // Q12
    s16 Isq;    // Q12
	s16 Is;     // Q12
    s16 PhiSd;  // Q12      d轴全磁链
    s16 PhiSq;  // Q12
    s16 RotVd;  // Q12
    s16 RotVq;  // Q12
    s16 EnableDcp;
	//Uint Amp;   // Q12      幅值
	s16 IsdSet;    // Q12
    s16 IsqSet;    // Q12
    s16 PhiSdSet;  // Q12      d轴全磁链设定值
    s16 PhiSqSet;  // Q12
    s16 RotVdSet;  // Q12
    s16 RotVqSet;  // Q12
	s16 IsqSetMax;
	s16 EMF;       //Q12    反电动势前馈量
	u16 Us;        //Q12    输出电压滤波值
}PM_DECOUPLE;

typedef struct PM_FW_IN_DEF
{
	s16  Time;			//弱磁控制时间间隔,0.5MS
	s16  Bemf;			//1mv/(rad/s),反电动势系数的峰值
	s16  Udc;			//0.1V，母线电压
	s16  R;				//1mohm,相电阻
	s16  IsSetMax;      //设定的最大输出相电流有效值，以电机额定电流为基值，Q12
	s16  IsAsrSetMax;	//设定的速度环PI调节最大相电流有效值，以电机额定电流为基值，Q12
	s16  IqSet;			//Q轴设定电流有效值，以电机额定电流为基值，Q12
	s16  IqFeed;		//Q轴反馈电流有效值，以电机额定电流为基值，Q12	
	s16  IdSet;			//Q轴设定电流有效值，以电机额定电流为基值，Q12
	s16  IdFeed;		//Q轴反馈电流有效值，以电机额定电流为基值，Q12
	s16  VolCsrOut;		//电流环PI调节输出的线电压有效，以电机额定线电压为基值，Q12
	s16  UdCsrOut;		//电流环D轴PI调节输出的线电压有效，以电机额定线电压为基值，Q12
	s16  UqCsrOut;		//电流环Q轴PI调节输出的线电压有效，以电机额定线电压为基值，Q12

	s16  IsAsrOut;		//速度环PI调节输出的相电流有效值，以电机额定电流为基值，Q12
	s16  MotorPoles;	//电机磁极对数
	s16  FullFreq;		//0.1HZ，电机基准频率
	s16  FreqFeed;		//电机反馈速度，以FullFreq为基值，Q15
	s16  MotorVol;		//1V,电机额定电压
	long MotorCurr;		//0.01A,电机额定电流

	s16  MaxSetUdc;		//0.1V,设定的最大输出母线电压，为0时输出直流母线电压
	s16  RatioSet;		//弱磁控制时输出电压设定值，用于确定弱磁电流
						//以变频器能输出的最大电压为基值，Q12
						//必须小于4096，推荐值3700（Udc==540V)
						//母线电压越小该值应该设的越小
	long KpId;			//弱磁电流调节KP

	long MaxDelId;		//每次最大减小ID给定的数值;
	s16  AdjuMode;		//自整定模式，0：不整定，1：整定最大转矩电流,
						//2:整定最大弱磁电流，3：整定最大转矩电流、弱磁电流

	s16  UdForIqMax;	//弱磁控制时Q轴输出电压设定值,用于确定最大转矩电流
						//以变频器能输出的最大电压为基值，Q12
						//必须大于RatioSet，推荐值3900（Udc==540V)
						//母线电压越小该值应该设的越小

	long KpIqMax;		//最大转矩电流减小调节KP
	long KpIqMax1;		//最大转矩电流增大调节KP
							
	s16  UqForIdMax;	//弱磁控制时Q轴输出电压设定值，用于确定最大弱磁电流
						//以变频器能输出的最大电压为基值，Q12
						//推荐值100（Udc==540V)，母线电压越小该值应该设的越大
	long KpIdMax;		//最大弱磁电流调节KP
	s16  CsrMaxVolt;	//电流环D、Q轴最大输出电压
	
}PM_FW_IN;

typedef struct PM_FW_DEF
{	
	
	s16  IdMax;
	s16  IdMax1;
	s16  PhiPerLd;
	s16  UdcLpf;
	s16  UdLpf;
	s16  UqLpf;
	s16  RatioLpf;
	s16  UdForIqMax;
	long  RatioLpf1;
	s16  CurPerLpf;
	s16  TorqPerAmp;
	s16  IqErrLpf;
	s16  IdErrLpf;
	s16  IdForTorq;
	
	long AbsOmgPer;
	long Omg;
	long OmgLpf;
	long CurrCoef;
	long CoefIqMax;
	
	long AdId;
	long AdIdIntg;

	long AdIqMax;	
	long AdIqMaxIntg;
	long AdIqMaxIntgMax;
	long AdIqMaxIntgMin;
	long AdIdMaxIntg;
	long TorqueCurr;
	long TorqueCurrMax;
	long MinUqLpf;
	long IqMax;
	long IqMax1;
	long PowerLpf;
	long TorqueEst;	
	long Ld;
	long Lq;
	long Lq1;
	s16  TorqRevi;
	s16  TReviCoef1;
	s16  PosComp;
	s16  MaxPosComp;
	s16  MpcBack;
	s16  MinPosComp;
	long IqSetLpf;
	s16  UqCompStat;
	s16  UqRatio;
	s16  imSet;
	s16  itSet;
	s16  umSet;
	s16  utSet;
	s16  utSetLpf;
}PM_FW;
typedef struct PM_FW_OUT_DEF
{	
	
	s16  IdSet;			//电流环D轴给定值，以电定电流为基值，Q12
	s16  IqSet;			//电流环Q轴给定值，以电机额定电流为基值，Q12
	s16  IsLimit;	   //速度环PI调节输出限定值，以电机额定电流为基值，Q12
	s16  UqComp;		//Q轴补偿电压，以电机额定电压为基值，Q12
    s16  PosComp;
	s16  ClearKID;
}PM_FW_OUT;
typedef struct PM_POS_EST_STRUCT_DEF
{
	s16 Usr;
	s16 Ust;
    s16 Urt;

	s16 Ur;
	s16 Us;
	s16 Ut;

	s16 Ualfa;
	s16 Ubeta;
	
	Uint RotorPosEst;     //通过反电动势辨识的转子磁极位置
	Uint RotorPosEstLast;
    Uint RotorPosEstLastOne;
	Ulong RotorPosEstTotal;//零点位置角累加和
	Uint MotorFreq;      //电机运行频率

	long ITAcrTotal;     //用于电压前馈

	Uint Uamp;           //反电动势幅值

	Uint Cnt;             //零点位置累加计数器

	s16 JudgeDirFlag;    //旋转方向标志位 1：顺着RST  2：逆着RST
	s16 JudgeDirOKFlag;  //旋转方向判断结束标志位
	s16 OverFlag;        //零点位置角判断结束标志位
	s16 JudgeDirCnt;     //判断旋转方向计数器

	Uint IdentifyErrorFlag;//零点位置角未辨识完启动会报错
	Uint StartErrorFlag; //停机再启动时，低于10Hz启动会报错
	Uint ADErrorFlag;    //AD转换过程中用时过长会报错
	Uint SpeedObst;      //通过反电动式估计的速度
	Uint Flag;           //0为反电动势在周期中断里算，1为为反电动势在z中断里算
	Uint EstErrCnt;
	
}PM_POS_EST_STRUCT;//永磁同步电机零点位置角辨识相关的结构--ligang 2012.9


//**************************************************************************
extern IPM_POSITION_STRUCT		gIPMPos;
extern PM_INIT_POSITION         gPMInitPos;
extern PM_FLUX_WEAK             gFluxWeak;
extern PM_DECOUPLE              gPmDecoup;
extern IPM_POS_CHECK_STRUCT	    gIPMPosCheck;
extern PM_FW_IN				   gPMFwIn;
extern PM_FW				   gPMFwCtrl;
extern PM_FW_OUT			   gPMFwOut;
//PM_POS_EST_STRUCT       gPMPosEst;

//**************************************************************************
extern void IPMCheckInitPos(void);
extern void PmChkInitPosRest(void);
extern void RunCaseIpmInitPos(void);
//extern void SynCalLdAndLq(Uint m_Pos);
extern void SynCalLdAndLq(void);
extern void IPMCalAcrPIDCoff(void);
extern void PmFluxWeakDeal();
extern void PmDecoupleDeal(void);
extern void PMClacFluxWeakId(void);
extern void PMSetFwPara1(void);
extern void PMSetFwPara(void);
extern void TurnToStopStatus(void);
extern void ResetParForPmsmFwc(void);
extern s16 PmsmMaxTorqCtrl(void);
extern s16 PmsmFreqAdjMethod(void);
extern void PMFluxWeak05Ms(void);
extern void IPMPosAdjustZIndex(void);
extern s16 IPMPosAdjustStop(void);
extern void IMFluxWeak2(void);



#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================
