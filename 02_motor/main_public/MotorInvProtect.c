/************************************************************
文件功能:电机和变频器保护程序
文件版本： 
最新更新： 

*************************************************************/
#include "MotorInvProtectInclude.h"

// // 全局变量定义
OVER_LOAD_PROTECT		gOverLoad;
PHASE_LOSE_STRUCT		gPhaseLose;
BEFORE_RUN_PHASE_LOSE_STRUCT gBforeRunPhaseLose;
INPUT_LOSE_STRUCT		gInLose;
#if (SOFT_INPUT_DETECT == STLEN)
SOFT_INPUT_LOSE_STRUCT		gSoftInLose;
#endif
LOAD_LOSE_STRUCT		gLoadLose;
FAN_CTRL_STRUCT			gFanCtrl;
Ulong					gBuffResCnt;	//缓冲电阻保护变量
CBC_PROTECT_STRUCT		gCBCProtect;
struct FAULT_CODE_INFOR_STRUCT_DEF  gError;
extern void FanSpeedCtrl(void);
//变频器和电机的过载表
Uint const gInvOverLoadTable[10] =      /*表长度10；表步长9%；表最小值:115%；表最大值:196%*/
{
		36000,				//115%变频器电流   		1小时过载  
		18000,				//124%变频器电流	  	30分钟过载
        6000,				//133%变频器电流	  	10分钟过载
        1800,				//142%变频器电流	  	3分钟过载 
        600,				//151%变频器电流   		1分钟过载  
        200,				//160%变频器电流   		20秒过载   
        120,				//169%变频器电流   		12秒过载    
        20,					//178%变频器电流   		6秒过载    改为178% 2S过载
        20,					//187%变频器电流   		2秒过载    
        5,					//196%变频器电流   		0.5秒过载   增加一个最大值      2011-10-21-chzq
};
#if(SOFTSERIES == MD380SOFT)
Uint const gInvOverLoadTableForP[10] =       /*表长度10；表步长4%；表最小值:105%；表最大值:141%*/
{
		36000,				//105%变频器电流   		1小时过载  
		15000,				//109%变频器电流	  	25分钟过载
        6000,				//113%变频器电流	  	10分钟过载
        1800,				//117%变频器电流	  	3分钟过载 
        600,				//121%变频器电流   		1分钟过载  
        300,				//125%变频器电流   		30秒过载   
        100,				//129%变频器电流   		10秒过载    
        30,					//133%变频器电流   		3秒过载    
        10,					//137%变频器电流   		1秒过载  
        5,					//141%变频器电流   		0.5秒过载     增加一个最大值   2011-10-21-chzq
};
#else
Uint const gInvOverLoadTableForP[9] =       /*表长度10；表步长4%；表最小值:105%；表最大值:141%*/
{
		18000,				//110%变频器电流   30.0分钟
        6000,				//117%变频器电流   10.0分钟
        1800,				//124%变频器电流   3.0分钟
        600,				//131%变频器电流   1.0分钟
        400,				//138%变频器电流   40 秒
        250,				//145%变频器电流   25 秒
        90,					//152%变频器电流   9 秒
        50,					//159%变频器电流   5 秒
        20					//166%变频器电流   2 秒
};
#endif
//变频器过载累计量的消除系数
Ulong const gInvOverLoadDecTable[12] =
{
        (65536L*60/7),      //0%变频器电流    0.7分钟消除过载
        (65536L*60/8),		//10%变频器电流   0.8分钟消除过载
        (65536L*60/9),		//20%变频器电流   0.9分钟消除过载
        (65536L*60/10),		//30%变频器电流   1.0分钟消除过载
        (65536L*60/11),		//40%变频器电流   1.1分钟消除过载
        (65536L*60/13),		//50%变频器电流   1.3分钟消除过载
        (65536L*60/16),		//60%变频器电流   1.6分钟消除过载
        (65536L*60/19),		//70%变频器电流   1.9分钟消除过载
        (65536L*60/24),		//80%变频器电流   2.4分钟消除过载
        (65536L*60/34),		//90%变频器电流   3.4分钟消除过载
		(65536L*60/56),		//100%变频器电流  5.6分钟消除过载
};
Uint const gInvBreakOverLoadTable[5] = 
{
    1,                      // 100% 10S最大电流 10s报过载   110%  2s过载
    4,                      // 90% 10S最大电流 40s报过载    120%  0.5s过载
    30,                     // 80% 10S最大电流 300s报过载
    200,                    // 70% 10S最大电流 2000s报过载
    2000,                   // 60% 10S最大电流 20000s报过载
};
/*#if (SOFTSERIES == MD500SOFT)
Uint const gInvBreakOverLoadTableFor185And22[10] = 
{
	1,//0,						//	120%  0.5s过载
	2,//0.2,						//	110%  0.2s过载
    100,//10,                 		// 	100%最大电流 10s报过载   
    500,//50,                 		// 	99%最大电流 50s报过载    
    820,//82,                	 	// 	98%最大电流 82s报过载
    1380,//138,                 	// 	97%最大电流 138s报过载
    2450,//245,                   	// 	96%最大电流 245s报过载
    5000,//500,					// 	95%最大电流 500s报过载
    10000,//1000					// 
    10000,//1000					// 
};
Uint const gInvBreakOverLoadTableFor30[10] = 
{
	13,// 1.3,					//	120%  1.3s过载
	50,// 5,						//	110%  5s过载
    100,// 10,                 	//	100%最大电流 10s报过载   
    160,// 16,                 	// 	98% 最大电流 16s报过载    
    230,//23,                 		// 	96% 最大电流 23s报过载
    300,//30,                 		// 	94% 最大电流 30s报过载
    440,//44,                   	// 	92% 最大电流 44s报过载
    580,//58,					// 	90% 最大电流 58s报过载
    2870,//287					// 	85% 最大电流 278s报过载
    5000,//500					//   
};
Uint const gInvBreakOverLoadTableFor37[10] = 
{
	25,// 2.5,					//	120%  2.5s过载
	75,// 7.5,					//	110%  7.5s过载
    100,// 10,                 	// 	100% 最大电流 10s报过载   
    180,// 18,                 	// 	98% 最大电流 18s报过载    
    250,// 25,                 	// 	96% 最大电流 25s报过载
    300,// 30,                 	// 	94% 最大电流 30s报过载
    360,// 36,                   	// 	92% 最大电流 36报过载
	480,// 48,					// 	90% 最大电流 48s报过载
	1000,//	100,					// 	90% 最大电流 48s报过载
	3400,//	340,					// 	90% 最大电流 48s报过载
};

Uint const gInvBreakOverLoadTableFor45And55[10] = 
{
	1,//0,						//	120% 最大电流 0s报过载  
	10,// 1,						//	110% 最大电流 1s报过载  
    100,// 10,                 	// 	100% 最大电流 10s报过载   
    290,// 29,                 	// 	98%  最大电流 29s报过载    
    580,// 58,                 	// 	96%  最大电流 58s报过载
    900,// 90,                 	// 	94%  最大电流 90s报过载
    1500,// 150,            		// 	92%  最大电流 150s报过载
    3000,//300					//
    5000,//500					//
    10000,//1000					//
};
Uint const gInvBreakOverLoadTableFor75[10] = 
{
	5,// 0.5,					//	120% 最大电流 0.5s过载
	30,// 3,						//	110% 最大电流 3s过载
    100,// 10,                 	// 	100%	最大电流 10s报过载   
    180,// 18,                 	// 	98% 	最大电流 18s报过载    
    270,// 27,                 	// 	96% 	最大电流 27s报过载
    390,// 39,                 	// 	94% 	最大电流 39s报过载
    530,// 53,                   	// 	92% 	最大电流 53s报过载
	1000,// 100,					// 	90% 	最大电流 100s报过载
	5000,//500					//
	10000,//1000					//
};
#endif
*/
#define C_MOTOR_OV_TAB_NUM      14
Uint const gMotorOverLoadBaseTable[C_MOTOR_OV_TAB_NUM] =
{
		480,			//115%电机电流  1小时20分钟过载
		240,			//125%电机电流  40分钟过载
		90,				//135%电机电流  15分钟过载 
		36,				//145%电机电流  6分钟过载
		24,				//155%电机电流  4分钟过载
		15,				//165%电机电流  2.5分钟过载
		12,				//175%电机电流  2分钟过载

		 9,				//185%电机电流  1.5分钟过载
		 6,				//195%电机电流  1分钟过载
		 5,				//205%电机电流  50S过载
		 4,				//215%电机电流  40S过载
		 3,				//225%电机电流  30S过载
		 2,				//235%电机电流  20S过载
		 1				//245%电机电流  10S过载
};
Uint gMotorOverLoadTable[C_MOTOR_OV_TAB_NUM] =
{
		48000,				//115%电机电流  1小时20分钟过载
		24000,				//125%电机电流  40分钟过载
		9000,				//135%电机电流  15分钟过载 
		3600,				//145%电机电流  6分钟过载
		2400,				//155%电机电流  4分钟过载
		1500,				//165%电机电流  2.5分钟过载
		1200,				//175%电机电流  2分钟过载

		 900,				//185%电机电流  1.5分钟过载
		 600,				//195%电机电流  1分钟过载
		 500,				//205%电机电流  50S过载
		 400,				//215%电机电流  40S过载
		 300,				//225%电机电流  30S过载
		 200,				//235%电机电流  20S过载
		 100				//245%电机电流  10S过载
};

#define C_MOTOR_OV_MAX_CUR      2450
#define C_MOTOR_OV_MIN_CUR      1150
#define C_MOTOR_OV_STEP_CUR     100

//电机过载累计量的消除系数
Ulong const gMotorOverLoadDecTable[12] =
{
        (65536L*60/30),     //0%电机电流    3.0分钟消除过载
        (65536L*60/40),		//10%电机电流   4.0分钟消除过载
        (65536L*60/50),		//20%电机电流   5.0分钟消除过载
        (65536L*60/60),		//30%电机电流   6.0分钟消除过载
        (65536L*60/70),		//40%电机电流   7.0分钟消除过载
        (65536L*60/80),		//50%电机电流   8.0分钟消除过?
        (65536L*60/90),		//60%电机电流   9.0分钟消除过载
        (65536L*60/100),	//70%电机电流   10.0分钟消除过载
        (65536L*60/110),	//80%电机电流   11.0分钟消除过载
        (65536L*60/120),	//90%电机电流   12.0分钟消除过载
		(65536L*60/130),	//100%电机电流  13.0分钟消除过载
};

#if (SOFTSERIES == MD500SOFT)
// //温度曲线表
Uint const gTempTableBSMXX[37] =
{
        2488,2441,2388,2328,2261,       //0-4
        2188,2108,2023,1933,1839,       //5-9
        1742,1644,1545,1446,1349,       //10-14
        1255,1164,1077,994, 916,        //15-19
        843, 775, 711, 653, 599,        //20-24
        541, 496, 454, 417, 382,        //25-29
        346, 318, 292, 268, 247,        //30-34
        228, 210                         //35-36

};
Uint const gTempTableWAIZHI[37] =
{
        3721,3632,3529,3411,3278,       //0-4
        3132,2973,2804,2626,2443,       //5-9
        2258,2073,1893,1719,1555,       //10-14
        1400,1258,1127,1008, 900,        //15-19
        804, 718, 641, 572, 511,        //20-24
        457, 409, 366, 328, 295,        //25-29
        265, 239, 215, 195, 176,        //30-34
        160, 146                         //35-36
};
//MD500不同机型的过温点不一致
Uint const gTempProtectTable[19] =
{
        95,95,110,100,100,         //16-20
        90, 95, 100,95,95,             //21-25
        100, 90, 90,95,90,             //26-30
        93, 97, 103 ,103,             //31-34
};
int const gFanSpeedCtrlTempreture[9]=
{
        -50,         // 132kw 25
        70,          // 160kw 26
        -50,         // 200kw 27
        70,          // 220kw 28
        -50,         // 250kw 29
        70,          // 280kw 30
        -50,         // 315kw 31
        70,          // 355kw 32
        70           // 400kw 33
};

#else //MD380
// //温度曲线表
Uint const gTempTableP44X[23] =
{
		624,614,603,590,576,561,		//6
		544,525,506,485,464,442,		//6
		419,395,373,350,327,305,		//6
		284,264,244,226,208				//5
};
Uint const gTempTableP8XX[23] =
{
		475,451,426,400,374,348,		//6
		323,299,275,253,232,212,		//6
		193,176,161,146,133,121,		//6
		110,100,91, 83, 76				//5
};
Uint const gTempTableBSMXX[23] =
{
		486,461,435,412,386,361,		//6
		337,313,291,269,248,228,		//6
		209,193,176,161,148,135,		//6
		123,113,103,94, 86				//5		
};
Uint const gTempTableSEMIKON[23] =
{
		558,519,480,451,418,392,		//6
		369,350,331,314,302,288,		//6
		278,269,262,254,247,243,		//6
		237,233,229,226,224				//5
};
Uint const gTempTableWAIZHI[23] =
{
		655,609,563,518,473,430,		//6
		389,350,314,282,251,224,		//6
		199,177,158,140,124,111,		//6
		99, 88, 78, 70, 62				//5		
};
#endif


// //文件内部函数声明
void    SoftWareErrorDeal(void);	
void	TemperatureCheck(void);					//温度检查
void	OutputPhaseLoseDetect(void);			//输出缺相检测
void 	OutputLoseReset(void);		
void	InputPhaseLoseDetect(void);				//输入缺相检测

#if (SOFT_INPUT_DETECT == STLEN)
void    SoftInputPhaseLoseUdcCal(void);
void    SoftInputPhaseLoseDetect(void);
#endif
void	ControlFan(void);						//风扇控制
void	OverLoadProtect(void);					//过载保护
void	CBCLimitCurProtect(void);
void 	SetCBCEnable(void);
void 	SetCBCDisable(void);
void 	BufferResProtect(void);
void BeforeRunOutputPhaseLoseDetect(void)	;
void ResetPhaseLoseDetect(void);	
Uint	OutputPhaseLoseComperCalculate(Uint motorRs,Uint PhaseLoseDetPRD);
void	BeforeRunShortGndDetect(void);
void OutputPhaseLoseAndShortGndDetect(void);
void ShortGnd_PhaseLoseICal(void);
void    BrakeResShortProtect(void);
void  BrakeOverloadProtect(void);
interrupt void ShortGnd_ADC_Over_isr(void);
/************************************************************
	变频器保护处理
************************************************************/
//电机过载曲线生成函数
void MotorOverLoadTimeGenerate(void)
{
	u16 i;
	u32 mMotorOverLoadTime;

	for(i=0;i<14;i++)
	{
		mMotorOverLoadTime = gMotorOverLoadBaseTable[i]*(u32)gComPar.MotorOvLoad;

		if(mMotorOverLoadTime > 48000)//上限80分钟
        {
			mMotorOverLoadTime = 48000;
		}
		else if(mMotorOverLoadTime < 100)//下限10S
		{
        	mMotorOverLoadTime = 100;
		}

		gMotorOverLoadTable[i] = mMotorOverLoadTime;
	}
}
void InvDeviceControl(void)			
{
	SoftWareErrorDeal();				//出错处理
	TemperatureCheck();					//温度检查
	OutputPhaseLoseDetect();			//输出缺相检测
#if (SOFTSERIES == MD380SOFT)
	InputPhaseLoseDetect();				//输入缺相检测
#endif
	ControlFan();						//风扇控制
#if (SOFTSERIES == MD500SOFT)
	CutDownCurForOverLoad();            //过载将额处理
	BrakeOverloadProtect();
#endif
	OverLoadProtect();					//过载保护

    CBCLimitCurPrepare();					//逐波限流保护程序
	CBCLimitCurProtect();				//逐波限流情况下的过载判断

	BufferResProtect();
#if (SOFTSERIES == MD380SOFT)
    BrakeResShortProtect();             //制动电阻短路保护处理
#endif
    EncodeLineDiscProtect();
}

/*************************************************************
	软件故障处理
停机状态也可能报过流故障、过压故障
*************************************************************/
void SoftWareErrorDeal(void)					
{
// 开始判断软件故障
#if (SOFTSERIES == MD380SOFT)     
	if((gUDC.uDCFilter > gInvInfo.InvUpUDC) || //过压判断,使用大滤波电压
	    GetOverUdcFlag())
	{
	    DisableDrive(); //停机也允许报过压，提醒用户输入电压过高
	    gError.ErrorCode.all |= ERROR_OVER_UDC;
        if((gUDC.uDCFilter > gInvInfo.InvUpUDC) && (!GetOverUdcFlag())
            &&(gMainStatus.ErrFlag.bit.OvCurFlag == 0))
        {
            gError.ErrorInfo[0].bit.Fault2 = 2;//软件过压
        }
        else
        {
            gError.ErrorInfo[0].bit.Fault2 = 1; //硬件过压
        }
	}
#else
	if((gUDC.uDCFilter > gInvInfo.InvUpUDC)||(ADC_UDC > 55125))    //55125对应860V
	{
	    DisableDrive(); //停机也允许报过压，提醒用户输入电压过高
	    gError.ErrorCode.all |= ERROR_OVER_UDC;
        if(gUDC.uDCFilter > gInvInfo.InvUpUDC)
		{
            gError.ErrorInfo[0].bit.Fault2 = 1; //软件过压
		}
		else
		{
		    gError.ErrorInfo[0].bit.Fault2 = 2;
		}
#if(AIRCOMPRESSOR == 1)
		if((gMainCmd.RestarCnt <= 3)&&(gOverLoad.OverLoadPreventEnable == 1))
		{
		    gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
		}
#endif
	}
#endif
	else if(gUDC.uDCBigFilter < gInvInfo.InvLowUDC) //欠压判断,使用大滤波电压
	{
	    DisableDrive();
        if(STATUS_GET_PAR == gMainStatus.RunStep)   //避免参数辨识过程中欠压导致寄存器配置修改后没有恢复
        {
            EndOfParIdentify();
        }         
        gMainStatus.RunStep = STATUS_LOW_POWER;
        gMainStatus.SubStep = 0;
		DisConnectRelay();	
		gError.ErrorCode.all |= ERROR_LOW_UDC;
        gMainStatus.StatusWord.bit.LowUDC = 0;
#if(AIRCOMPRESSOR == 1)
		if((gMainCmd.RestarCnt <= 3)&&(gMainCmd.Command.bit.Start == 1)&&(gOverLoad.OverLoadPreventEnable == 1))   //运行过程中报欠压才上传标志位
		{
		    gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
		}
#endif
	}

	if(//(gMainStatus.RunStep == STATUS_LOW_POWER) ||                         // 欠压
	   (gError.ErrorCode.all & ERROR_SHORT_EARTH) == ERROR_SHORT_EARTH)
	{
		gUDC.uDCBigFilter = gUDC.uDCFilter;
		return;										//欠压状态下不判断软件故障
	}
	if((STATUS_STOP == gMainStatus.RunStep) &&
        (gError.LastErrorCode != gError.ErrorCode.all) && 
        (gError.ErrorCode.all != 0))
	{
	    gError.LastErrorCode = gError.ErrorCode.all;
        if(0 != gError.ErrorCode.all)
        {
			gPhase.IMPhase += 0x4000L;				//故障复位后运行角度发生变化
        }
    }
   
// 开始故障复位
	if(gSubCommand.bit.ErrorOK == 1) 			
	{
		gError.ErrorCode.all = 0;
        gError.ErrorInfo[0].all = 0;
        gError.ErrorInfo[1].all = 0;
        gError.ErrorInfo[2].all = 0;
        gError.ErrorInfo[3].all = 0;
        gError.ErrorInfo[4].all = 0;
        gBrake.ErrCode = 0;
        gLineCur.ErrorShow = 0;
        //CBC中断复位
        EALLOW;
        EPwm2Regs.TZCLR.bit.CBC = 1;
        EPwm2Regs.TZCLR.bit.INT = 1;
        EDIS;
		if(gMainStatus.ErrFlag.bit.OvCurFlag == 1)
		{
			gMainStatus.ErrFlag.bit.OvCurFlag = 0;
			//gPhase.IMPhase += 16384;
			EALLOW;
			EPwm2Regs.TZCLR.bit.OST = 1;
			EPwm3Regs.TZCLR.bit.OST = 1;
			EPwm1Regs.TZCLR.bit.OST = 1;
			EPwm1Regs.TZCLR.bit.INT = 1;
			EDIS;
		}
	}


}

//	MD500的温度检测函数
#if (SOFTSERIES == MD500SOFT)
void TemperatureCheck(void)
{
	Uint    * m_pTable,*m_ErrTempTable;
	long    m_TempAD,m_IndexLow,m_IndexHigh,m_Index,m_ErrTemp;
	Uint    m_LimtCnt;
    int    mType,m_TempMax,m_TempMin;

    mType = gInvInfo.InvTypeApply;

    if((gInvInfo.InvTypeSet>100) && (gInvInfo.InvTypeSet<200))    //380V改制为220V机型温度曲线处理
    {
        mType = gInvInfo.InvTypeGaiZhi; 
    }
    m_ErrTempTable = (Uint *)gTempProtectTable;
    
// 保护温度点的确定 18.5KW-110KW需要查表
    if((gInvInfo.InvTypeApply > 15)&&(gInvInfo.InvTypeApply <= 34))
    {
        m_ErrTemp = *(m_ErrTempTable + gInvInfo.InvTypeApply - 16);
    }
    else
    {
        m_ErrTemp = 95;
    }
	gTemperature.OverTempPoint = m_ErrTemp;
	
// 温度曲线的选定
// MD500的功率范围内，才进行温度计算，否则过温处理	
	if((mType <= 34) && (mType >= 16))    // [16, 25]
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
		else 
		{
			m_pTable = (Uint *)gTempTableWAIZHI;
		}
		
		// 硬件采样，以及采样电路的修正
	    m_TempAD = gTemperature.TempAD>>4;//温度AD最高采样为12位
	
		// 开始查询温度表
		m_IndexLow  = 0;
		m_IndexHigh = 36;
		m_Index = 18;//温度表格二分法的中简值
	    m_TempMax = 124;//最高温度
	    m_TempMin = -20;//最低温度
		if(m_TempAD >= m_pTable[m_IndexLow])
		{	
			mType = m_TempMin * 16;
		}
		else if(m_TempAD <= m_pTable[m_IndexHigh])
		{
			mType = m_TempMax * 16;//Q4
		}
		else
		{
			m_LimtCnt = 0;
			while(m_LimtCnt < 8)//8为二分法查询最多需要次数
			{
				m_LimtCnt++;					//避免死循环
				if(m_TempAD == m_pTable[m_Index])
				{
					mType = (m_Index<<6) + m_TempMin * 16;
					break;
				}
				else if(m_IndexLow+1 == m_IndexHigh)
				{
					mType = (m_IndexLow<<6) + m_TempMin * 16 
								+ (((long)m_pTable[m_IndexLow] - m_TempAD)<<6)
					                /((long)m_pTable[m_IndexLow] - (long)m_pTable[m_IndexHigh]);
					break;
				}
				
				if(m_TempAD > m_pTable[m_Index])
				{
					m_IndexHigh = m_Index;
				}
				else
				{
					m_IndexLow = m_Index;
				}
				m_Index = (m_IndexLow + m_IndexHigh)>>1;
			}
		}
		if(abs(mType - gTemperature.TempBak) >= 8)			//温度变化超过0.5度才赋值
		{
			gTemperature.TempBak = mType;
			gTemperature.Temp = mType>>4;
		}
	}
	else
	{
		gTemperature.Temp = 150;
		gTemperature.TempBak = 2400;
	}
	    //gTemperature.Temp = gTestDataReceive.TestData28;
// 开始作温度判断和报警处理
	gTemperature.ErrCnt++;
	if(gTemperature.Temp < m_ErrTemp)
	{	
		gTemperature.ErrCnt = 0;
	}

	if(gTemperature.ErrCnt >= 5)
	{
		gTemperature.ErrCnt = 0;
		gError.ErrorCode.all |= ERROR_INV_TEMPERTURE;		//过热报警
	}
}
#else
/************************************************************
MD380温度检查,说明如下:

1、机型小于等于10       (2.2Kw 以?
	温度曲线:	     1		TABLE_P44X
				2		TABLE_P8XX
				3		TABLE_SEMIKON
				4		TABLE_BSMXX

2、机型在（11～18之间 (11Kw 到 30Kw)
	温度曲线:	     1		TABLE_BSMXX
				2		TABLE_P44X
				3~4		TABLE_SEMIKON

3、机型在（19～26之间） (37Kw 到 200Kw,并且包含37, 不包含200)
	温度采样外置：		TABLE_WAIZHI

4、机型大于等于27		TABLE_BSMXX

5、机型大于等于27时，温度采样电路不同，需要进行3.3V和3V的转换；

6、机型大于等于19的85度保护；其他机型95度保护；

表格排列方式：每4度作一个数据，起始地址数据为12度，数据值为AD采样值
AD采样值：AD_RESULT>>6
************************************************************/
void TemperatureCheck(void)
{
	Uint    * m_pTable;
	Uint    m_TempAD,m_IndexLow,m_IndexHigh,m_Index,m_ErrTemp;
	Uint    m_LimtCnt;
    Uint    mType;

	//...准备温度表,690V的温度采样和保护与380V机型大于27的一致
	if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
    {
       mType = 28;
    }
    else  if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
    {
       
       mType = 20;  /*1140V 过温点和温度曲线设定为外置温度曲线 2011.7.26 L1082*/
    }
    else
    {
        mType = gInvInfo.InvTypeApply;
    }

    if((gInvInfo.InvTypeSet>100) && (gInvInfo.InvTypeSet<200))    //380V改制为220V机型温度曲线处理
    {
        mType = gInvInfo.InvTypeGaiZhi; 
    }

// 保护温度点的确定
      m_ErrTemp = 95;

// 温度曲线的选定
	if(mType <= 10)
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP8XX;
		}
		else if(gInvInfo.TempType == 3)
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
		else    // gInvInfo.TempType == 4
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
	}
	else if(mType <= 18)    // [11, 18]
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else if(gInvInfo.TempType == 3)
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
		else 
		{
			m_pTable = (Uint *)gTempTableWAIZHI;
		}
	}
    else if(mType <= 26)    // [19, 26]
    {
        if(gInvInfo.TempType == 1)
        {
            m_pTable = (Uint *)gTempTableWAIZHI;
        }
        else    
        {
            m_pTable = (Uint *)gTempTableBSMXX;                
        }

        m_ErrTemp = 85;
    }
    else        // mType >= 27
    {
        m_pTable = (Uint *)gTempTableBSMXX;
    }
    gTemperature.OverTempPoint = m_ErrTemp;

// 硬件采样，以及采样电路的修正
    m_TempAD = (gTemperature.TempAD>>6);	

#ifdef  TMS320F2808
    if(mType >= 27) // 3V和3.3v 采样电路的转换
    {
        m_TempAD = ((long)gTemperature.TempAD * 465)>>15; 
    }
#endif

// 开始查询温度表
	m_IndexLow  = 0;
	m_IndexHigh = 22;
	m_Index = 11;
	if(m_TempAD >= m_pTable[m_IndexLow])
	{	
		mType = 12 * 16;
	}
	else if(m_TempAD <= m_pTable[m_IndexHigh])
	{
		mType = 100 * 16;
	}
	else
	{
		m_LimtCnt = 0;
		while(m_LimtCnt < 7)
		{
			m_LimtCnt++;					//避免死循环
			if(m_TempAD == m_pTable[m_Index])
			{
				mType = (m_Index<<6) + (12 * 16);
				break;
			}
			else if(m_IndexLow+1 == m_IndexHigh)
			{
				mType = (m_IndexLow<<6) + (12 * 16) 
							+ ((m_pTable[m_IndexLow] - m_TempAD)<<6)
				                /(m_pTable[m_IndexLow] - m_pTable[m_IndexHigh]);
				break;
			}
			
			if(m_TempAD > m_pTable[m_Index])
			{
				m_IndexHigh = m_Index;
			}
			else
			{
				m_IndexLow = m_Index;
			}
			m_Index = (m_IndexLow + m_IndexHigh)>>1;
		}
	}
	if(mType - gTemperature.TempBak >= 8)			//温度变化超过0.5度才赋值
	{
		gTemperature.TempBak = mType;
		gTemperature.Temp = mType>>4;
	}

// 开始作温度判断和报警处理
	gTemperature.ErrCnt++;
	if(gTemperature.Temp < m_ErrTemp)
	{	
		gTemperature.ErrCnt = 0;
	}

	if(gTemperature.ErrCnt >= 5)
	{
		gTemperature.ErrCnt = 0;
		gError.ErrorCode.all |= ERROR_INV_TEMPERTURE;		//过热报警
	}

}
#endif
/************************************************************
	输出缺相检测
************************************************************/
void OutputPhaseLoseDetect(void)			
{
	Uint m_U,m_V,m_W;
	Uint m_Max,m_Min;

	/*gPhaseLose.TotalU += abs(gIUVWQ24.U >> 12);
	gPhaseLose.TotalV += abs(gIUVWQ24.V >> 12);
	gPhaseLose.TotalW += abs(gIUVWQ24.W >> 12);

    //gPhaseLose.Time   += gPWM.gPWMPrdApply;
    //gPhaseLose.Time += 2000L * DSP_CLOCK;
	gPhaseLose.Time += gMainCmd.FreqReal;
	*/
    //gPhaseLose.Cnt ++;
    
	if((gSubCommand.bit.OutputLost == 0) ||	
	   (gMainCmd.FreqReal < 80) ||					//0.8Hz以下不检测
	   (gMainCmd.FreqReal > 50000)||                //500Hz以上也不检测
	   (gMainCmd.Command.bit.StartDC == 1) ||		//直流制动状态下不检测
	   (gMainCmd.Command.bit.StopDC  == 1) ||
	   ((gMainStatus.RunStep != STATUS_RUN) && 		//不是运行或者速度搜索阶段
	    (gMainStatus.RunStep != STATUS_SPEED_CHECK)))
	{
		OutputLoseReset();
		return;
	}    

	if((gPhaseLose.Time < 4000000)||(gPhaseLose.Cnt<400))    // 20个电流周期且至少需要200ms后判断输出缺相 
	{		
		return;
	}

	m_U = gPhaseLose.TotalU/gPhaseLose.Cnt;
	m_V = gPhaseLose.TotalV/gPhaseLose.Cnt;
	m_W = gPhaseLose.TotalW/gPhaseLose.Cnt;

	m_Max = (m_U > m_V) ? m_U : m_V;
	m_Min = (m_U < m_V) ? m_U : m_V;
	m_Max = (m_Max > m_W) ? m_Max : m_W;
	m_Min = (m_Min < m_W) ? m_Min : m_W;
	m_Min = Max(m_Min ,1);

	if((m_Max > 500) && (m_Max/m_Min > 10))
	{
		gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
        gError.ErrorInfo[1].bit.Fault4 = 1;
#if(AIRCOMPRESSOR == 1)
		if((gMainCmd.RestarCnt <= 3)&&(gOverLoad.OverLoadPreventEnable == 1))
		{
		    gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
		}
#endif
	}
	//gPhaseLose.errMaxCur = m_Max;
    //gPhaseLose.errMinCur = m_Min;
	OutputLoseReset();
}
void OutputLoseAdd(void)		//输出缺相检测累加电流处理
{
	gPhaseLose.TotalU += abs(gIUVWQ24.U >> 12);
	gPhaseLose.TotalV += abs(gIUVWQ24.V >> 12);
	gPhaseLose.TotalW += abs(gIUVWQ24.W >> 12);

    gPhaseLose.Time += gMainCmd.FreqReal;
	gPhaseLose.Cnt ++;
}

void OutputLoseReset(void)		//输出缺相检测复位寄存器处理
{
	gPhaseLose.Cnt = 0;
	gPhaseLose.TotalU = 0;
	gPhaseLose.TotalV = 0;
	gPhaseLose.TotalW = 0;
	gPhaseLose.Time   = 0;
}

void OutputPhaseLoseAndShortGndDetect(void)			
{
	Uint Data1,Data2;
	if((gError.ErrorCode.all != 0) || 
	((gMainCmd.Command.bit.Start == 0)&&(gMainStatus.RunStep != STATUS_SHORT_GND)))
	{
		gMainStatus.SubStep = 1;
		gBforeRunPhaseLose.CheckOverFlag = 1;
		ResetPhaseLoseDetect();
		return;
	}
	
	switch(gMainStatus.SubStep)
	{
		case 1:
			gBforeRunPhaseLose.Counter	= 0;
			gBforeRunPhaseLose.OverFlag	= 0;
		//	gMainStatus.PrgStatus.bit.PWMDisable = 1; // 禁止下溢中断发波
			DisableDrive();
			// 启动前对地短路检测及启动前输出缺相检测电流阀值的计算
			// 默认为2048(Q12格式)，但必须小于80%变频器额定电流，小于2倍电机额定电流
			Data1 = ((Ulong)gInvInfo.InvCurrent*3277)/gMotorInfo.Current;			// 80%变频器额定电流, Q12
			Data2 = ((Ulong)gInvInfo.InvCurrent<<12)/gMotorInfo.Current;			// 变频器额定电流, Q12
			gBforeRunPhaseLose.CurComperCoff = (Data1 < 2048) ? Data1 : 2048;		// min(50%基值电流, 80% 变频器额定电流)
            gBforeRunPhaseLose.CurComperCoffLimit = (Data2 < 3277) ? Data2 : 3277;	// min(80%基值电流, 变频器额定电流)
			//gBforeRunPhaseLose.CurComperCoff = (gBforeRunPhaseLose.CurComperCoff<Data2)?gBforeRunPhaseLose.CurComperCoff:Data2;
			EALLOW;
			PieVectTable.ADCINT1	= &ShortGnd_ADC_Over_isr; // 启动前对地短路和输出缺相检测AD中断  
			EPwm1Regs.TBPRD = 600;	// AD中断周期为10us
			EPwm2Regs.TBPRD = 600;
			EPwm3Regs.TBPRD = 600;
			EPwm1Regs.DBCTL.all	= 0x0;
			EPwm2Regs.DBCTL.all	= 0x0;
			EPwm3Regs.DBCTL.all	= 0x0;
			EPwm1Regs.AQCSFRC.all 	= 0x0A;
			EPwm2Regs.AQCSFRC.all 	= 0x0A;			
			EPwm3Regs.AQCSFRC.all 	= 0x0A;
			if((gBforeRunPhaseLose.OverFlag !=1)&&
			(gShortGnd.ocFlag == 0))
			{
				EnableDrive();
			}
			if(gExtendCmd.bit.ShortGndBeforeRun != 1)	// 启动前不检测对地短路, 直接检测输出缺相, PWM寄存器配置
			{
				EPwm1Regs.AQCSFRC.all 	= 0x09;		// A+, B-
				EPwm2Regs.AQCSFRC.all 	= 0x06;			
				EPwm3Regs.AQCSFRC.all 	= 0x0A;

			}
			else // 启动前对地短路检测, 标志位设置, PWM寄存器配置
			{
				gBforeRunPhaseLose.ShortGndDetectFlag = 1;
				EPwm1Regs.AQCSFRC.all 	= 0x09;		// A+
				EPwm2Regs.AQCSFRC.all 	= 0x0A;			
				EPwm3Regs.AQCSFRC.all 	= 0x0A;
			}
			EDIS;
			gMainStatus.SubStep++;
			break;
			
		case 2:
		case 3:
			if((gExtendCmd.bit.ShortGndBeforeRun == 1)&&
			(gMainStatus.SubStep == 2))
			{
				BeforeRunShortGndDetect();	// 启动前对地短路检测判断, 未完成检测前, 循环执行
				gMainStatus.StatusWord.bit.ShortGndOver = 1;
			}
			if((gError.ErrorCode.all == 0)&&
			(gSubCommand.bit.OutputLostBeforeRun == 1))
			{
				if((gBforeRunPhaseLose.OverFlag !=1)&&
				(gShortGnd.ocFlag == 0)&&
				(gMainStatus.ErrFlag.bit.OvCurFlag == 0))
				{
					EnableDrive();
				}
				gBforeRunPhaseLose.Counter++;
				BeforeRunOutputPhaseLoseDetect(); //	启动前输出缺相检测判断
			}
			else
			{
				gBforeRunPhaseLose.CheckOverFlag = 1;
				ResetPhaseLoseDetect();	
			}
			break;
		
		case 4:
		default:
			gBforeRunPhaseLose.CheckOverFlag = 1;
			ResetPhaseLoseDetect();	
			break;
	}
 
}
void ResetPhaseLoseDetect(void)	
{
	DisableDrive();
	gMainStatus.SubStep = 1;
	gBforeRunPhaseLose.maxIU = 0;
	gBforeRunPhaseLose.maxIV = 0;
	gBforeRunPhaseLose.maxIW = 0;
	gBforeRunPhaseLose.Counter = 0;
	gBforeRunPhaseLose.UWPhaseLoseFlag = 0;
	gBforeRunPhaseLose.ShortGndDetectFlag = 0;
	gShortGnd.ocFlag = 0;
//	gMainStatus.PrgStatus.bit.PWMDisable = 0;	// 恢复下溢中断发波
	gMainStatus.RunStep = STATUS_STOP;
	EALLOW;
	PieVectTable.ADCINT1     = &ADC_Over_isr;
	EDIS;
	InitSetPWM();                       //恢复修改的寄存器配置
}

void BeforeRunShortGndDetect(void)
{
	DisableDrive();
//	gBforeRunPhaseLose.ShortGndDetectFlag = 0;
	if((gShortGnd.ocFlag != 0)||
	(gBforeRunPhaseLose.OverFlag == 1)||
//	(gBforeRunPhaseLose.maxIU > (30 * 32))||
	//(gUDC.uDC > gShortGnd.BaseUDC + 650)|| // 暂时删除母线电压判断条件
	(gCBCProtect.CBCIntFlag == 1))
	{
		EALLOW;
		EPwm1Regs.AQCSFRC.all 	= 0x0A;
		EPwm2Regs.AQCSFRC.all 	= 0x0A;			
		EPwm3Regs.AQCSFRC.all 	= 0x0A;
		EDIS;
		gError.ErrorCode.all |= ERROR_SHORT_EARTH;
		gBforeRunPhaseLose.CheckOverFlag = 1;
		if(abs(gShortGnd.ShortCur) > (30 * 32)||
		(gBforeRunPhaseLose.OverFlag == 1)) 
		{
			gError.ErrorInfo[2].bit.Fault4 =3; //软件过流 
		}
		/*if(gUDC.uDC > gShortGnd.BaseUDC + 650)  暂时删除母线电压判断条件
		{
			gError.ErrorInfo[2].bit.Fault4 = 4; //软件过压  
		}*/
		if(1 == gShortGnd.ocFlag) 
		{
			gError.ErrorInfo[2].bit.Fault4 = 1; //硬件过流,硬件优先级高
		}
		if(2 == gShortGnd.ocFlag) 
		{
			gError.ErrorInfo[2].bit.Fault4 = 2; //硬件过压
		}
		gLineCur.ErrorShow = gBforeRunPhaseLose.maxIU;
		gMainStatus.SubStep = 4;					// 确定对地短路后, 直接结束检测并进行复位
	}
	else											// 没有对地短路, 按输出缺相检测配置寄存器, 进入下一步检测
	{
		gMainStatus.SubStep++;
		gBforeRunPhaseLose.maxIU 	= 0;
		gBforeRunPhaseLose.maxIV	= 0;
		gBforeRunPhaseLose.maxIW	= 0;
		EALLOW;
		EPwm1Regs.AQCSFRC.all 	= 0x09;
		EPwm2Regs.AQCSFRC.all 	= 0x06;			
		EPwm3Regs.AQCSFRC.all 	= 0x0A;
		EDIS;
	}
	gBforeRunPhaseLose.ShortGndDetectFlag = 0;
}
void BeforeRunOutputPhaseLoseDetect(void)	
{
	if((gBforeRunPhaseLose.OverFlag == 1)&&	// 	变频器三相输出电流均满足电流阀值则认为未缺相
	(gBforeRunPhaseLose.maxIU > gBforeRunPhaseLose.CurComperCoff))
	{
		if(gBforeRunPhaseLose.UWPhaseLoseFlag == 0)
		{
			gBforeRunPhaseLose.maxIU = 0;
			gBforeRunPhaseLose.maxIV = 0;
			gBforeRunPhaseLose.maxIW = 0;
			gBforeRunPhaseLose.Counter = 0;
			gBforeRunPhaseLose.UWPhaseLoseFlag = 1;
			gBforeRunPhaseLose.OverFlag =0;
			if((gBforeRunPhaseLose.OverFlag !=1)&&
			(gShortGnd.ocFlag == 0)&&
			(gMainStatus.ErrFlag.bit.OvCurFlag == 0))
			{
				EnableDrive();
			}
			EALLOW;
			EPwm1Regs.AQCSFRC.all 	= 0x09;
			EPwm2Regs.AQCSFRC.all 	= 0x0A;			
			EPwm3Regs.AQCSFRC.all 	= 0x06;
			EDIS;
		}
		else
		{
			gMainStatus.SubStep++;
			gBforeRunPhaseLose.CheckOverFlag = 1;
			ResetPhaseLoseDetect();
		}
	}
	else
	{
		if(gBforeRunPhaseLose.Counter >= 6)
		{
			DisableDrive();
			gMainStatus.SubStep++;
			if(gMainStatus.ErrFlag.bit.OvCurFlag == 0)
			{
				gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;//报输出缺相
				gError.ErrorInfo[1].bit.Fault4 = 3;
				gBforeRunPhaseLose.Counter = 0;
			}
		}
	}
}

interrupt void ShortGnd_ADC_Over_isr(void)
{
	EALLOW;             //28035改为EALLOW保护
	ADC_CLEAR_INT_FLAG;
	EDIS;
	EINT;

	ShortGnd_PhaseLoseICal();
	DINT;
	EALLOW;
	ADC_RESET_SEQUENCE;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;	// Acknowledge this interrupt
	EDIS;
	
}

void ShortGnd_PhaseLoseICal(void)
{
//	gCpuTime.ADCIntBase = GetTime();
    s32 OverCur;
    				
	GetCurrentInfo();
	if(gBforeRunPhaseLose.ShortGndDetectFlag == 1)   // 对地短路检测
    {
        OverCur = (gIUVWQ24.U >> 12) + (gIUVWQ24.V >> 12);
        OverCur = OverCur + (gIUVWQ24.W >> 12);
        gBforeRunPhaseLose.CurTotal = abs((s16)OverCur);					// 求三相电流之和, Q12, pu
        gBforeRunPhaseLose.maxIU    = abs(gIUVWQ24.U >> 12);
    }
    else		// 输出缺相检测
    {
		gBforeRunPhaseLose.maxIU	= Max(gBforeRunPhaseLose.maxIU,abs(gIUVWQ24.U >> 12));		// U相最大电流
        gBforeRunPhaseLose.CurTotal = gBforeRunPhaseLose.maxIU;
	}
	if((gBforeRunPhaseLose.maxIU > gBforeRunPhaseLose.CurComperCoff)		// 对于对地短路检测, U相电流大于50%, 且三相电流之和大于50%, 认为对地短路
	 &&(gBforeRunPhaseLose.CurTotal > gBforeRunPhaseLose.CurComperCoff))	// 对于输出缺相检测, U相最大电流大于50%, 
	{
		DisableDrive();
		EALLOW;
		EPwm1Regs.AQCSFRC.all 	= 0x0A;
		EDIS;
		gBforeRunPhaseLose.OverFlag = 1;
	}
	else if((gBforeRunPhaseLose.maxIU > gBforeRunPhaseLose.CurComperCoffLimit)
	&&(gBforeRunPhaseLose.CurTotal < (gBforeRunPhaseLose.CurComperCoffLimit>>1))
	&&(gBforeRunPhaseLose.ShortGndDetectFlag == 1))             // 对地短路检测, 当检测到U相电流大于80%，而三相和小于40%时不再发波检测
	{
	    DisableDrive();
		EALLOW;
		EPwm1Regs.AQCSFRC.all 	= 0x0A;
		EDIS;
	}
//	gCpuTime.ADCInt = gCpuTime.ADCIntBase - GetTime();
}
/************************************************************
	输入缺相检测
	信号的高低电平是指的排线上的信号，不是到DSP的信号，到DSP的信号电平相反
缺相检测原理1: MD380的
    继电器故障信号与缺相信号复合， 正常的时候一直为低；
    若PL一直为高，则继电器没有吸合；
    若PL为方波，则缺相；    

缺相检测原理2（MD500硬件修改后）zzb1812：
//需要放在0.5ms中执行
// 输入缺相720Hz ,1s应该有720个周期，实际有偏差到500Hz
// 超过450个周期就认为缺相，450Hz
// 缺1相时信号不连续，会出现6ms左右的低电平
// 缺3相时为连续的720Hz信号

// 缓冲电阻300Hz，一般比较准确，有300个周期
// 持续高电平为缓冲电阻故障
18.5kw 以上才有缺相电路
************************************************************/
#if (SOFTSERIES == MD500SOFT)
//#if (HARD_WARE_INPUT_DETECT == STLEN)
void InputPhaseLoseReset()
{
	gInLose.LowLevelCnt = 0;
	gInLose.PLFreqCnt    = 0;
	gInLose.HighLevelCnt = 0;
	gInLose.PLFreqTotal  = 0;
	gInLose.LongLowLevelCnt = 0;
}
void InputPhaseLoseDetect(void)			
{
	if((gMainCmd.Command.bit.Start == 0)              //停机状态\故障状态\18.5KW以下不检测输入缺相
		 ||(gInvInfo.InvTypeApply < gInLose.ForeInvType)
		 ||(gError.ErrorCode.all != 0))
	{
		gInLose.HighLevelCnt = 0;
		gInLose.LongLowLevelCnt = 0;
		gInLose.LowLevelCnt  = 0;
		gInLose.PLFreq       = 0;
		gInLose.PLFreqCnt    = 0;
		gInLose.PLFreqTotal = 0;
		return;
	}

	if(PL_INPUT_HIGH)
	{
		Uint m_Data;
				
		if(gInLose.HighLevelCnt && gInLose.LowLevelCnt)
		{					
			m_Data = (gInLose.HighLevelCnt + gInLose.LowLevelCnt);//0.5ms中执行
			gInLose.PLFreqTotal += m_Data;	//累积
			if (gInLose.LowLevelCnt > 8)//4ms以上的低电平计数
			{
				gInLose.LongLowLevelCnt ++;
			}
			gInLose.HighLevelCnt = 0;
			gInLose.LowLevelCnt  = 0;
			gInLose.PLFreqCnt++;			
		}
		else if((gInLose.HighLevelCnt > 250)&&(gSubCommand.bit.RelayEnalbe == 1))//持续0.5s钟高电平，则认为缓冲电阻吸合故障
		{
			gError.ErrorCode.all |= ERROR_RESISTANCE_CONTACK;		
			InputPhaseLoseReset();
			return;
		}
		gInLose.HighLevelCnt++;
	}
	else if(gInLose.HighLevelCnt)
	{
		gInLose.LowLevelCnt++;
		if (gInLose.LowLevelCnt > 6000)//0.5ms执行3s清除，防止误脉冲累积
		{
			InputPhaseLoseReset();
		}
	}
	
	if(gInLose.PLFreqTotal > 2000) //连续1s
	{	
		if(gInLose.PLFreqCnt > 450 || gInLose.LongLowLevelCnt > 20) 					            
		{
#if (HARD_WARE_INPUT_DETECT == STLEN)
			if (gSubCommand.bit.InputLost >= 1)
			{
				gError.ErrorCode.all |= ERROR_INPUT_LACK_PHASE;	
    #if(AIRCOMPRESSOR == 1)
				if((gMainCmd.RestarCnt <= 3)&&(gOverLoad.OverLoadPreventEnable == 1))
				{
				    gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
				}	
    #endif
			}
#endif
		}
		// 缓冲电阻300Hz，一般比较准确，有150个周期
		else if(gInLose.PLFreqCnt > 200) 
		{
			if (gSubCommand.bit.RelayEnalbe == 1)
			{
				gError.ErrorCode.all |= ERROR_RESISTANCE_CONTACK;	
			}
		}	
		InputPhaseLoseReset();
	}
}
	#if (SOFT_INPUT_DETECT == STLEN)
/**********************************************
软件输入缺相检测
**********************************************/
void SoftInputPhaseLoseUdcCal(void)
{
    Uint    temp;
    Uint    m_Udc;
 
    gSoftInLose.MaxUdcFlag++;
    if(gSoftInLose.MaxUdcFlag > 30)
    {
        m_Udc           = (gSoftInLose.MaxUdc_5s + gSoftInLose.MinUdc_5s)>>1;
        gSoftInLose.uDCAverage =  Filter32(m_Udc,gSoftInLose.uDCAverage);
        gSoftInLose.MaxUdcFlag = 30;
    }
    gSoftInLose.A_uDC       = gUDC.uDC;
    gSoftInLose.uDCFilter   = gSoftInLose.uDCAverage;
    
    gSoftInLose.Count_5s++;
    if(gSoftInLose.Count_5s >= 10000)										// 5s计时
    {
        if(abs(gSoftInLose.MaxUdc_5s - gSoftInLose.MinUdc_5s) >= 300)
        {
            gSoftInLose.Count_5s    = 0;
            temp = gSoftInLose.MaxUdc_5s;
            gSoftInLose.MaxUdc_5s   = gSoftInLose.MinUdc_5s;
            gSoftInLose.MinUdc_5s   = temp;
            gSoftInLose.MaxUdcFlag  = 0;
        }
        else
        {
            gSoftInLose.Count_5s = 10001;
        }
    } 

    gSoftInLose.Count_100ms++;

	if(gSoftInLose.Count_100ms >= 200)
	{
		gSoftInLose.MaxUdc_100ms    = gSoftInLose.MaxUdcTemper;
		gSoftInLose.MinUdc_100ms    = gSoftInLose.MinUdcTemper;
		gSoftInLose.MaxUdcTemper    = gSoftInLose.MinUdc_100ms;
		gSoftInLose.MinUdcTemper    = gSoftInLose.MaxUdc_100ms;
		gSoftInLose.Count_100ms     = 0;
	}
}
void SoftInputPhaseLoseDetect(void)
{														 
    u32  u_dvalve,deta_t;
    u32  deta_t1,deta_t2;
    static s16  m_DetaCount = 0;
    Uint    m_InLoseTime;
        
    if((gMainStatus.RunStep != STATUS_RUN) || 		//不是运行阶段不检测
       (gSubCommand.bit.InputLost != 1)||
       (gSoftInLose.MaxUdc_100ms - gSoftInLose.MinUdc_100ms <= gSoftInLose.MaxDetaU)||
       (gPowerTrq.InvPower_si <= ((gSoftInLose.HalfInvPowerPU*200)>>10)))     //||         //变频器额定功率的50%=2048*1.732
	{
		gSoftInLose.Flag = 0;
        gSoftInLose.time1 = 0; 
        gSoftInLose.time2 = 0;
        gSoftInLose.time3 = 0;
        gSoftInLose.time4 = 0;
	
	}

    else
    {
        gSoftInLose.uDC = gSoftInLose.A_uDC; 
        u_dvalve = gSoftInLose.uDCFilter;
        if(	gSoftInLose.uDC <= u_dvalve )
        {
            if(gSoftInLose.Flag == 0)
            {	//	第一次低于比较电压
               gSoftInLose.time1 = GetTime();
               gSoftInLose.Flag = 1;
            }
            else if(gSoftInLose.Flag ==2) 
            {	//	第二次低于比较电压
               gSoftInLose.time3 = GetTime();
               gSoftInLose.Flag = 3;
            }       
        }
        else if(gSoftInLose.uDC  > u_dvalve)
        {
            if(gSoftInLose.Flag ==1)
            {	//	第一次低电压恢复
               gSoftInLose.Flag = 2;  
               gSoftInLose.time2 = GetTime();            
            }	//	第二次低电压恢复
            else if(gSoftInLose.Flag ==3)
            {
                gSoftInLose.time4 = GetTime();   
                gSoftInLose.Flag = 0;  
    			//	计算谐波的频率
                deta_t1 = ((gSoftInLose.time1 - gSoftInLose.time3)&0xFFFFFFFF)>>1;
                deta_t2 = ((gSoftInLose.time2 - gSoftInLose.time4)&0xFFFFFFFF)>>1;
                deta_t = ((deta_t1 + deta_t2)/DSP_CLOCK);
                deta_t = Max(deta_t ,3000);
                deta_t = Min(deta_t ,15000);
                
                gSoftInLose.wait_r += deta_t;
    			//	连续波动十次算谐波频率的平均值
                m_DetaCount ++;
                if(m_DetaCount < 10)
                {
                    return;
                }
                deta_t = gSoftInLose.wait_r / 10;              
                m_DetaCount = 0;
                gSoftInLose.wait_r = 0;

              
                
                if((DOWNSTATE <= deta_t)&&(deta_t <= UPSTATE))
                {
                    if(gPowerTrq.InvPower_si <= (gSoftInLose.HalfInvPowerPU>>1))
                    {
                        gSoftInLose.InLoseAddTime = 1;     //  5 MIN 
                    }
                    else if(gPowerTrq.InvPower_si <= gSoftInLose.HalfInvPowerPU)
                    {
                        gSoftInLose.InLoseAddTime = 150;   //  2S
                    }
                    else
                    {
                        gSoftInLose.InLoseAddTime = 750;   //  400 MS
                    }
                    gSoftInLose.Flose = gSoftInLose.Flose+gSoftInLose.InLoseAddTime;
                	//	连续10次都在范围内，则报缺相
                    if(gSoftInLose.Flose >= 1500)
                    {
                        gError.ErrorCode.all |= ERROR_INPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault3 = 2;                                       
                       
                        gSoftInLose.time1 = 0;
                        gSoftInLose.time2 = 0;
                        gSoftInLose.time3 = 0;
                        gSoftInLose.time4 = 0; 
                    }
                }
                else
                {    
                    gSoftInLose.Flose = 0;
                    gSoftInLose.time1 = 0;
                    gSoftInLose.time2 = 0;
                    gSoftInLose.time3 = 0;
                    gSoftInLose.time4 = 0; 
                }
            }
        } 
    }
    
    if(gSoftInLose.FloseLast == gSoftInLose.Flose)
    {
    // 两个周期判断标志相等，清零  20S 清零
        if(gSoftInLose.FloseSameTime ++ > 800)           
        {
            if(gSoftInLose.Flose >= 30)
            {
                gSoftInLose.Flose = gSoftInLose.Flose - 30;
            }
			else
			{
				gSoftInLose.Flose = 0;
			}
        }
    }
    else
    {
        gSoftInLose.FloseLast = gSoftInLose.Flose;
        gSoftInLose.FloseSameTime =0;
    }

    if(gMainStatus.StatusWord.bit.LowUDC == 0)
    {
        gSoftInLose.MinUdc_5s = gSoftInLose.MaxUdc_5s;
    } 
    if((gError.ErrorCode.all & ERROR_OVER_UDC) == ERROR_OVER_UDC)
    {
        gSoftInLose.MaxUdc_5s = gSoftInLose.MinUdc_5s; 
    }
 }
	#endif
#else
//MD380
void InputPhaseLoseDetect(void)			
{
	if(((STATUS_RUN!=gMainStatus.RunStep)&&(STATUS_SPEED_CHECK!=gMainStatus.RunStep)  //运行或转速跟踪状态才检测
	    &&(STATUS_GET_PAR!=gMainStatus.RunStep))||
	   (gInvInfo.InvTypeApply < gInLose.ForeInvType))
	{
		gInLose.Cnt = 0;
		gInLose.UpCnt = 0;
		gInLose.ErrCnt = 0;
		gInLose.CntRes = 0;
		gInLose.UpCntRes = 0;
		return;
	}
    
	if(PL_INPUT_HIGH)           // PL是高电平		
	{
		gInLose.UpCnt ++;
		gInLose.UpCntRes ++;
	}
    
	if(gInLose.UpCntRes != 0)	// 持续500ms的PL低电平判断为继电器故障
	{
		gInLose.CntRes++;	
		if(gInLose.CntRes >= 250)       // 500ms
		{
			if((gSubCommand.bit.RelayEnalbe == 1) &&          //F9-12-十位
				(gInLose.UpCntRes >= 249) && (gInvInfo.InvTypeApply >= 19))
			{
			    gError.ErrorCode.all |= ERROR_RESISTANCE_CONTACK;
			}
			gInLose.CntRes = 0;
			gInLose.UpCntRes = 0;
		}
	}

	gInLose.Cnt++;	
	if(gInLose.Cnt < 500)       //  缺相检测1sec一个循环 
    {
        return;
    }

	if((gSubCommand.bit.InputLost == 1) &&              //F9-12-个位
	   (gInLose.UpCnt > 5) && (gInLose.UpCnt < 485))    // 1sec内PL存在低电平，毕喾讲?
	{
		gInLose.ErrCnt++;
		if(gInLose.ErrCnt >= 2)
		{
			gError.ErrorCode.all |= ERROR_INPUT_LACK_PHASE;
			gInLose.ErrCnt = 0;
		}
	}
	else
	{
		gInLose.ErrCnt = 0;
	}
	gInLose.Cnt = 0;
	gInLose.UpCnt = 0;
}

#endif

/************************************************************
	风扇刂?
1）	上电缓冲状态结束后1秒钟内风扇不运行；
2）	运行状态，风扇运行；
3）	直流制动等待期间，风扇运行
4）	模块温度低于40度，风扇停止；温度高于42度风扇运行；40～42度之间不变化。
5)	风扇启动后至少10秒才关闭
************************************************************/
#if (SOFTSERIES == MD500SOFT)
void ControlFan(void)						
{
	if(gMainStatus.RunStep == STATUS_LOW_POWER)
	{
		TurnOffFan();
		gFanCtrl.EnableCnt = 0;
		return;
	}

	gFanCtrl.FanDetaTime = ((gFanCtrl.FanTime - GetTime()) & 0xFFFFFFUL)/DSP_CLOCK;
    gFanCtrl.FanTime = GetTime();    

    if(gFanCtrl.FanDetaTime > 2200)     //执行间隔大于2.2ms 认为时基无法保证
    {
        gFanCtrl.ErrTimes++;
        if(gFanCtrl.ErrTimes > 10)
        {
            gFanCtrl.ErrTimes = 20;
        }
    }


	gFanCtrl.EnableCnt++;
	if(gFanCtrl.EnableCnt < 500)
	{
		TurnOffFan();
		return;
	}
	gFanCtrl.EnableCnt = 500;

    
	if((gMainCmd.Command.bit.Start == 1) ||
	   (gTemperature.Temp > 42) ||
	   (gSubCommand.bit.FanNoStop == 1))
	{
		gFanCtrl.RunCnt = 0;
        if(gFanCtrl.ErrTimes > 10)
        {
            TurnOnFan();
        }
        else
        {
            FanSpeedCtrl();
        }
	}
	else 
	{
        if(gTemperature.Temp < 40)
        {
            gFanCtrl.RunCnt++;
        }
		
		if(gFanCtrl.RunCnt >= 5000)
		{
			gFanCtrl.RunCnt = 5000;
            gFanCtrl.ErrTimes = 0;
			TurnOffFan();
		}
        else
        {
            if(gFanCtrl.ErrTimes > 10)
            {
                TurnOnFan();
            }
            else
            {
                FanSpeedCtrl();
            }
        }
	}
}
void FanSpeedCtrl(void)
{
    static u16 m_FanFlag = 0;
    static u16 m_FanStatus = 0;
    int m_AdjSpeedTemp;
    if((gInvInfo.InvTypeApply >= 25) && (gInvInfo.InvTypeApply <= 34))
    {
        m_AdjSpeedTemp = gFanSpeedCtrlTempreture[gInvInfo.InvTypeApply - 25];
        if(gTemperature.Temp >= m_AdjSpeedTemp)
        {
            TurnOnFan();
            m_FanStatus = 1;
        }
        else if(gTemperature.Temp < m_AdjSpeedTemp - 20)
        {
            if((m_FanFlag == 0) || (gTemperature.Temp <= -20))
            {
                TurnOnFan();
                m_FanFlag = 1;
            }
            else
            {
                TurnOffFan();
                m_FanFlag = 0;
            }
            m_FanStatus = 0;
        }
        else
        {
            if(m_FanStatus == 1)
            {
                TurnOnFan();
            }
            else
            {
                if(m_FanFlag == 0)
                {
                    TurnOnFan();
                    m_FanFlag = 1;
                }
                else
                {
                    TurnOffFan();
                    m_FanFlag = 0;
                }
            }
        }
    }
    else
    {
        TurnOnFan();
    }
}
#else
void ControlFan(void)						
{
	if(gMainStatus.RunStep == STATUS_LOW_POWER)
	{
		TurnOffFan();
		gFanCtrl.EnableCnt = 0;
		return;
	}

	gFanCtrl.EnableCnt++;
	if(gFanCtrl.EnableCnt < 500)
	{
		TurnOffFan();
		return;
	}
	gFanCtrl.EnableCnt = 500;

	if((gMainCmd.Command.bit.Start == 1) ||
	   (gTemperature.Temp > 42) ||
	   (gSubCommand.bit.FanNoStop == 1))
	{
		gFanCtrl.RunCnt = 0;
		TurnOnFan();
	}
	else if(gTemperature.Temp < 40)
	{
		gFanCtrl.RunCnt++;
		if(gFanCtrl.RunCnt >= 5000)
		{
			gFanCtrl.RunCnt = 5000;
			TurnOffFan();
		}
	}
}
#endif
/************************************************************
	变频器和电机过载保护
************************************************************/
void OverLoadProtect(void)				
{
	Ulong m_LDeta = 0;
	Uint m_Cur,m_Data,m_Index,m_CurBaseInv,m_MotorOvLoad;
	Uint m_TorCurBottom,m_TorCurUpper,m_TorCurStep,m_TorCurData;
    Uint *m_TorCurLine;
	Uint m_Limit;
    long m_OverLoadMargin,m_MaxCurLimit,m_Long;
	
#if (SOFTSERIES == MD500SOFT)	
    m_CurBaseInv = gLineCur.CurBaseInvOverload;
#else
    m_CurBaseInv = gLineCur.CurBaseInv;
#endif

/*	if( 28 <= gInvInfo.InvTypeApply )
    {
        //m_CurBaseInv = (s32)gLineCur.CurBaseInv * (s32)gInvInfo.InvCurrent / gInvInfo.InvOlCurrent;
        m_CurBaseInv = (long)gLineCur.CurBaseInv * (long)gInvInfo.InvCurrent / gInvInfo.InvCurrForP;
    }
*/    
	gOverLoad.FilterInvCur = Filter16(m_CurBaseInv,gOverLoad.FilterInvCur);
	gOverLoad.FilterMotorCur = Filter16(gLineCur.CurPer,gOverLoad.FilterMotorCur);
	gOverLoad.FilterRealFreq = Filter16(gMainCmd.FreqReal,gOverLoad.FilterRealFreq);

	if(gMainStatus.RunStep == STATUS_LOW_POWER)
	{
		gOverLoad.InvTotal.all = 0;
		gOverLoad.MotorTotal.all = 0;
#if (SOFTSERIES == MD500SOFT)	
		gOverLoad.BreakTotal = 0;
#endif
		gOverLoad.Cnt = 0;
		gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
		return;
	}

	/************************空压机休眠和堵转处理**********************************/
    if(gOverLoad.OverLoadPreventEnable != 0)
	{
	    if((gOverLoad.InvTotal.half.MSW >= 30000L)&&(gMainStatus.RunStep == STATUS_RUN))
		{		    
		    if((gMainCmd.FreqReal < gMainCmd.XiuMianFreq - 200)&&(gMainCmd.XiuMianFreq > 200))  //下限频率减2Hz
			{
				gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
			}
			else
			{
				if(gMainStatus.StatusWord.bit.OverLoadPreventState != 2)
				{
				    gMainStatus.StatusWord.bit.OverLoadPreventState = 1;
				}
			}
		}
	/*	else
		{
			if(gOverLoad.OverLoadPreventEnable == 1)
			{
			    gMainStatus.StatusWord.bit.OverLoadPreventState = 0;
			}
		}*/
	}
	/**************************end************************************************/
	gOverLoad.Cnt++;
	if(gOverLoad.Cnt < 5)		
	{
        return;		    //每10ms判弦淮?
	}
	gOverLoad.Cnt = 0;

	////////////////////////////////////////////////////////////////
	//选择过载曲线

    if(1 == gInvInfo.GPType)        //G型机过载曲线，恒转矩负载机型
    {
        m_TorCurLine    = (Uint *)gInvOverLoadTable;
        m_TorCurBottom  = 1150;     //115%变频器电流
        m_TorCurUpper   = 1960;     //最大值196%过载    // 更改曲线最大值,2011-10-21-chzq
        m_TorCurStep    = 90;
        m_TorCurData    = 5;        //>=196%,0.5s过载
    }
#if(SOFTSERIES == MD380SOFT)
    else                            //P型机过载曲线，风机、水泵类负载机型
    {       
        m_TorCurLine    = (Uint *)gInvOverLoadTableForP;
        m_TorCurBottom  = 1050;
        m_TorCurUpper   = 1410;     //最大值141%过载    // 更改曲线最大值,2011-10-21-chzq
        m_TorCurStep    = 40;
        m_TorCurData    = 5;        //>=141%,0.5s过载
    }
#else
    else                            //P型机过载曲线，风机、水泵类负载机型
    {       
        m_TorCurLine    = (Uint *)gInvOverLoadTableForP;
        m_TorCurBottom  = 1100;
        m_TorCurUpper   = 1660;     //最大值166%过载   
        m_TorCurStep    = 70;
        m_TorCurData    = 20;        //>=166%,2.0s过载
    }
#endif

    m_MaxCurLimit = 5000;       // 500% 保证不受限制
	////////////////////////////////////////////////////////////////
	//开始判断变频器的过载
	m_Cur = ((Ulong)gOverLoad.FilterInvCur * 1000L) >> 12;

	if(m_Cur < m_TorCurBottom)
    {
		if(gOverLoad.InvTotal.half.MSW < 10)
		{
			gOverLoad.InvTotal.all = 0;
		}
        else if(gMainStatus.RunStep == STATUS_STOP)
		{
			gOverLoad.InvTotal.all -= gInvOverLoadDecTable[0];  
		}
		else if(m_Cur < 1000)       /*小于变频器额定电流，按照当前电流大小消除过载累计量*/
		{
			gOverLoad.InvTotal.all -= gInvOverLoadDecTable[m_Cur/100 + 1];
		}
	}
	else
	{
		if(gOverLoad.FilterRealFreq < 500)		
		{
			m_Data = gOverLoad.FilterRealFreq * 13 + 26214;
			m_Cur  = (((Ulong)m_Cur)<<15)/m_Data;
            if(gOverLoad.OverLoadPreventEnable != 0)
            {
                if(gOverLoad.InvTotal.half.MSW > 10800)     //当过载30%，低频将过载限制在170%
                {
                    m_MaxCurLimit = (1700UL * (Ulong)m_Data)>>15;
                }
            }
		}
		if(m_Cur >= m_TorCurUpper)
		{
			m_Data = m_TorCurData;
		}
		else
		{
			m_Index = (m_Cur - m_TorCurBottom)/m_TorCurStep;
			m_Data = *(m_TorCurLine + m_Index) -
			         (((long)(*(m_TorCurLine + m_Index)) - (*(m_TorCurLine + m_Index + 1))) * 
					  (long)(m_Cur - m_TorCurBottom - m_Index * m_TorCurStep))/m_TorCurStep;
		}
		m_LDeta = ((Ulong)3600<<16)/(Uint)m_Data;
		gOverLoad.InvTotal.all += m_LDeta;
	    	
		if(gOverLoad.InvTotal.half.MSW >= 36000)
		{
			gOverLoad.InvTotal.half.MSW = 36000;
			//AddOneError(ERROR_INV_OVER_LOAD,1);

			gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
			gError.ErrorCode.all |= ERROR_INV_OVER_LAOD;
            gError.ErrorInfo[1].bit.Fault1 = 2;
		}
	}
	//gOverLoad.InvTotal.half.MSW = gTestDataReceive.TestData28;
	gOverLoad.TotalPercent = (Ulong)gOverLoad.InvTotal.half.MSW * 100L/36000L;
    /*****************防过载处理***********************************/
	if((gOverLoad.OverLoadPreventEnable != 0)&&(gMainStatus.StatusWord.bit.OverLoadPreventState != 2))  //   报休眠故障后不限制电流
	{
	    m_OverLoadMargin = 30000L - (long)gOverLoad.InvTotal.half.MSW;
	    if(m_OverLoadMargin < 720)
	    {
	        m_Limit = 1800;
	        if(m_OverLoadMargin < 400)
	        {
	            m_Limit = 1700;
	        }
	        m_Long = 1350 + (m_OverLoadMargin<<1);
	        if(m_MaxCurLimit > m_Long)
	        {
	            m_MaxCurLimit = m_Long;
	        }
	        m_MaxCurLimit = __IQsat(m_MaxCurLimit,m_Limit,950);
	    }
	    //gLineCur.OverLoadMargin = m_OverLoadMargin; //测试用
	    gLineCur.MaxCurLimit = m_MaxCurLimit;
	    gLineCur.MaxCurLimit =  __IQsat(gLineCur.MaxCurLimit,1800,250);
	}
	else
	{
	    gLineCur.MaxCurLimit = 1800;
	}
    /****************************end******************************/
    //变频器过载预报警处理
	if(((gOverLoad.InvTotal.all + m_LDeta * 1000UL)>>16) > 36000)
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 1;
	}
	else
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
	}


	MotorOverLoadTimeGenerate();//电机过载曲线生成函数
	////////////////////////////////////////////////////////////////
	//开始判断电机的过载
	//if(gMainCmd.SubCmd.bit.MotorOvLoadEnable == 0)
	if(gSubCommand.bit.MotorOvLoad == 0)
	{
		gOverLoad.MotorTotal.all = 0;
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
		return;
	}

	m_Cur = ((Ulong)gOverLoad.FilterMotorCur * 1000L)>>12;
	//m_Cur = ((Ulong)m_Cur * 100L)/100;//gComPar.MotorOvLoad;          // 根据过载保护系数计算出保护电流，
                                                            	//然后用该保护电流查询过载保护曲线
	m_LDeta = (Ulong)m_Cur * (Ulong)gMotorInfo.CurBaseCoff;
	if(m_LDeta >= (C_MOTOR_OV_MAX_CUR * 256L))
	{
		m_Cur = C_MOTOR_OV_MAX_CUR;
	}
	else
	{
		m_Cur = m_LDeta>>8;
	}
    

	m_MotorOvLoad = __IQsat(gComPar.MotorOvLoad,150,100);


	if(m_Cur < (u16)((u32)m_MotorOvLoad*C_MOTOR_OV_MIN_CUR/100UL))
	{
		if(gOverLoad.MotorTotal.half.MSW < 10)
		{
			gOverLoad.MotorTotal.all = 0;
		}
        else if(gMainStatus.RunStep == STATUS_STOP)
		{
			gOverLoad.MotorTotal.all -= gMotorOverLoadDecTable[0];  
		}
		else if(m_Cur < m_MotorOvLoad*10)                   /*小于100%额定电流按照电流消除电机过载*/
		{
			gOverLoad.MotorTotal.all -= gMotorOverLoadDecTable[m_Cur/m_MotorOvLoad + 1];  
		}
	}
	else
	{
		if(m_Cur >= C_MOTOR_OV_MAX_CUR)
		{
			m_Data = gMotorOverLoadTable[C_MOTOR_OV_TAB_NUM - 1];
		}
		else
		{
			m_Index = (m_Cur - (u16)((u32)m_MotorOvLoad*C_MOTOR_OV_MIN_CUR/100UL))/C_MOTOR_OV_STEP_CUR;
			m_Data = gMotorOverLoadTable[m_Index] -
			         ((long)(gMotorOverLoadTable[m_Index] - gMotorOverLoadTable[m_Index+1]) * 
					  (long)(m_Cur - (u16)((u32)m_MotorOvLoad*C_MOTOR_OV_MIN_CUR/100UL) - m_Index * C_MOTOR_OV_STEP_CUR))/C_MOTOR_OV_STEP_CUR;
		}
		m_LDeta = ((Ulong)3600<<16)/(Uint)m_Data;
		gOverLoad.MotorTotal.all += m_LDeta;

		if(gOverLoad.MotorTotal.half.MSW > 36000)
		{
			gOverLoad.MotorTotal.half.MSW = 36000;
			//AddOneError(ERROR_MOTOR_OVER_LOAD,1);
			gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
			gError.ErrorCode.all |= ERROR_MOTOR_OVER_LOAD;
		}		
	}
    //电机过载预报?  
	//if(gOverLoad.MotorTotal.half.MSW > gBasePar.PerMotorOvLoad * 360)
	if(gOverLoad.MotorTotal.half.MSW > gComPar.PerMotorOvLoad * 360)
	{
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 1;
	}
	else
	{
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
	}
}


/************************************************************
	处于逐波限流状态下的过载保护
//单管持续逐波限流时间超过5000ms保护，对应2500个2ms
三相合成逐波限流脉冲个数超过150个保护，10s内无脉冲，之前影响清除
************************************************************/
void CBCLimitCurProtect(void)
{
	int     m_CurU,m_CurV,m_CurW,m_Coff;
	int	    m_Max,m_Add,m_Sub;
    Uint    m_Limit;
    
	if((gSubCommand.bit.CBCEnable == 1) && (gMainStatus.RunStep != STATUS_GET_PAR) && (gMainStatus.RunStep != STATUS_IPM_INIT_POS) && (gMainStatus.RunStep != STATUS_FLYING_START))   //DQ轴电感和初始位置辨识时不进入逐波限流
	{
		if(gCBCProtect.EnableFlag == 0) SetCBCEnable();	//启用逐波限流功能
	}
	else
	{
		if(gCBCProtect.EnableFlag == 1)  SetCBCDisable();//取消逐波限流功能
	}
		
	//开始分别计算三相电流绝对值的积分
	m_Coff = (((long)gMotorInfo.Current)<<10) / gInvInfo.InvCurrent;
	m_CurU = (int)(((long)gIUVWQ12.U * (long)m_Coff)>>10);
	m_CurU = abs(m_CurU);
	m_CurV = (int)(((long)gIUVWQ12.V * (long)m_Coff)>>10);
	m_CurV = abs(m_CurV);
	m_CurW = (int)(((long)gIUVWQ12.W * (long)m_Coff)>>10);
	m_CurW = abs(m_CurW);
    
	//开始判断是否有一相持续大电流超过5秒
	if(m_CurU > 9267)	gCBCProtect.CntU++;             // 9267 = 4096 * 1.414 * 1.6
	else				gCBCProtect.CntU = gCBCProtect.CntU>>1;
	gCBCProtect.CntU = (gCBCProtect.CntU > 3000)?3000:gCBCProtect.CntU;

	if(m_CurV > 9267)	gCBCProtect.CntV++;
	else				gCBCProtect.CntV = gCBCProtect.CntV>>1;
	gCBCProtect.CntV = (gCBCProtect.CntV > 3000)?3000:gCBCProtect.CntV;

	if(m_CurW > 9267)	gCBCProtect.CntW++;
	else				gCBCProtect.CntW = gCBCProtect.CntW>>1;
	gCBCProtect.CntW = (gCBCProtect.CntW > 3000)?3000:gCBCProtect.CntW;
    
	if(gMainCmd.FreqReal > 20)
	{
		gCBCProtect.CntU = 0;
		gCBCProtect.CntV = 0;
		gCBCProtect.CntW = 0;
	}

	if((gCBCProtect.CntU > 2500) || 		//单相大电流报过载, 任何一相持续5000ms
	   (gCBCProtect.CntV > 2500) || 
	   (gCBCProtect.CntW > 2500) )
	{
		gError.ErrorCode.all |= ERROR_INV_OVER_LAOD;
        gError.ErrorInfo[1].bit.Fault1 = 1;
        gLineCur.ErrorShow = gLineCur.CurPer; //zzb1812
	}
    
	if(gCBCProtect.EnableFlag == 0)
	{
		gCBCProtect.TotalU = 0;
		gCBCProtect.TotalV = 0;
		gCBCProtect.TotalW = 0;
		return;
	}
#if (SOFTSERIES == MD380SOFT)
    if((INV_VOLTAGE_690V == gInvInfo.InvVoltageType)||(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)) 
    {
        if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
        {
            m_Limit = 10; 
        } 
        else
        {
            m_Limit = 150;  //逐波限流脉冲允许个数
        }
        
        if(0 == gCBCProtect.CbcFlag)              // 无逐波限流脉冲发生
        {
            gCBCProtect.NoCbcTimes++;
            if(5000 <= gCBCProtect.NoCbcTimes)   //持续10秒没有逐波限流发生，之前的影响可以清除
            {
                gCBCProtect.NoCbcTimes = 5000;
                gCBCProtect.CbcTimes = 0;
            }
        }
        else                                     // 有逐波限流脉冲发生
        {
            gCBCProtect.NoCbcTimes = 0;
            gCBCProtect.CbcFlag = 0;
        }

        if(m_Limit <= gCBCProtect.CbcTimes) 
        {
            gError.ErrorCode.all |= ERROR_TRIP_ZONE;
            gCBCProtect.CbcTimes = m_Limit - 1;      //避免逐波限流后无法复位
        } 
    }
    else
#endif
    {
        m_Limit = 500;     
    	m_Add = 2;
    	m_Sub = 1;
    	if(gMainStatus.RunStep == STATUS_STOP)
        {
            m_Sub = 2;
        }

    	if(gCBCProtect.Flag.bit.CBC_U == 1)   //计算U相电流的积分
    	{
    		gCBCProtect.TotalU += m_Add;
    	}
    	else //if(m_CurU < 8688)					//小于1.5倍变频器峰值电流后累加值递减 : 4096*1.5*sqrt(2)
    	{
    		gCBCProtect.TotalU -= m_Sub;
    	}

    	if(gCBCProtect.Flag.bit.CBC_V == 1) 	//计算V相电流的积分
    	{
    		gCBCProtect.TotalV += m_Add;
    	}
    	else //if(m_CurV < 8688)					//小于1.5倍变频器电流后累加值递减
    	{
    		gCBCProtect.TotalV -= m_Sub;
    	}

    	if(gCBCProtect.Flag.bit.CBC_W == 1) 	//计算W相电流的积分
    	{
    		gCBCProtect.TotalW += m_Add;
    	}
    	else //if(m_CurW < 8688)					//∮1.5倍变频器电流后累加值递减
    	{
    		gCBCProtect.TotalW -= m_Sub;
    	}

    	gCBCProtect.Flag.all = 0;

    	//电流积分值限幅
    	gCBCProtect.TotalU = __IQsat(gCBCProtect.TotalU, m_Limit, 0);
    	gCBCProtect.TotalV = __IQsat(gCBCProtect.TotalV, m_Limit, 0);
    	gCBCProtect.TotalW = __IQsat(gCBCProtect.TotalW, m_Limit, 0);

    	m_Max = (gCBCProtect.TotalU > gCBCProtect.TotalV) ? gCBCProtect.TotalU : gCBCProtect.TotalV;
    	m_Max = (m_Max > gCBCProtect.TotalW) ? m_Max : gCBCProtect.TotalW;
        if(m_Max >= m_Limit)      //逐波限流报40号故障
        {
            gError.ErrorCode.all |= ERROR_TRIP_ZONE;
        }          
    }
}

/*************************************************************
	开启逐波限流功能
*************************************************************/
void SetCBCEnable(void)
{
	gCBCProtect.EnableFlag = 1;
    EALLOW;
    
#ifdef TMS320F2808	
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_ENABLE;		// EPWM1的逐波限流
	EPwm1Regs.TZSEL.bit.CBC4 = TZ_ENABLE;
        
	EPwm2Regs.TZSEL.bit.CBC2 = TZ_ENABLE;       // EPWM2
    EPwm2Regs.TZSEL.bit.CBC3 = TZ_ENABLE;
    EPwm2Regs.TZSEL.bit.CBC4 = TZ_ENABLE;

    EPwm3Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm3Regs.TZSEL.bit.CBC3 = TZ_ENABLE;
	EPwm3Regs.TZSEL.bit.CBC4 = TZ_ENABLE;       // EPWM3
#else
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm2Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm3Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
#endif

	EDIS;
}
/*************************************************************
	关闭逐波限流功能
*************************************************************/
void SetCBCDisable(void)
{
	gCBCProtect.EnableFlag = 0;

	EALLOW;
    EPwm2Regs.TZEINT.all = 0x0000;//禁止TZ2中断，逐波限流中断
#ifdef TMS320F2808
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_DISABLE;		// EPWM1的逐波限流
	EPwm1Regs.TZSEL.bit.CBC4 = TZ_DISABLE;
        
	EPwm2Regs.TZSEL.bit.CBC2 = TZ_DISABLE;       // EPWM2
    EPwm2Regs.TZSEL.bit.CBC3 = TZ_DISABLE;
    EPwm2Regs.TZSEL.bit.CBC4 = TZ_DISABLE;

    EPwm3Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm3Regs.TZSEL.bit.CBC3 = TZ_DISABLE;
	EPwm3Regs.TZSEL.bit.CBC4 = TZ_DISABLE;       // EPWM3
#else
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm2Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm3Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
#endif

	EDIS;
}
/*************************************************************
	缓冲电阻保护处理
	
持续的进入欠压状态，认为是缓冲电阻故障
*************************************************************/
void BufferResProtect(void)
{
	if(gBuffResCnt >= 150000L)			//缓冲电阻保护处理
	{
		gError.ErrorCode.all |= ERROR_RESISTER_HOT;
	}
	gBuffResCnt--;	
	gBuffResCnt = __IQsat(gBuffResCnt,200000L,0);					
}

#if (SOFTSERIES == MD500SOFT)	

/*************************************************************
制动IGBT直通判断，在中断中执行，每载波周期执行一次
没有开通标志时判断
没有开通时电流大于额定电流的37.5%报直通
*************************************************************/
void  BrakeIGBTProtect(void)
{
	Uint m_Mincurrent;
	if(((gBrake.Flag & 0x02) == 0x02) || (gExcursionInfo.IbOkFlag == 0))
	{
		gBrake.IgbtShotTicker = 0;
		return;
	}
	// 关断时检测是否有电流
	m_Mincurrent = (gInvInfo.InvBreakCurrent * 3) >> 3;
	if (gBrake.IBreak > m_Mincurrent)
	{
		gBrake.IgbtShotTicker += gPWM.gPWMPrdApply;
	}	
	else
	{
		gBrake.IgbtShotTicker = 0;
	}
	// 连续2s报警
	if (gBrake.IgbtShotTicker >= 60000000L) // 2s = 2 * 60 000 000/2 
	{	
		gBrake.IgbtShotTicker = 60000000L;
		gError.ErrorCode.all |= ERROR_IGBT_SHORT_BRAKE;	
        gError.ErrorInfo[4].bit.Fault1 = 2;	
	}
	/*制动管过流判断,从之前的制动管过载保护函数中移到该函数中*/
	if (gBrake.IBreak >= (gInvInfo.InvBreakMaxCurrent * 2))
	{
		gBrake.FilterTicker ++;			
	}
	//检测3次以上报过流
	if (gBrake.FilterTicker >= 3)
	{	
		gBrake.FilterTicker		= 0;
		gError.ErrorCode.all |= ERROR_OVER_CURRENT;
		gError.ErrorInfo[0].bit.Fault1 = 2;//制动电阻过流
		gLineCur.ErrorShow		= gLineCur.CurPer;
	}
}

/*************************************************************
1、制动电阻选取太小
2、制动电流较大时，时间较长

*************************************************************/
void  BrakeOverloadProtect(void)
{
    u16 m_CurrCoff;
	Ulong	IBreakrsm,Duty;
	int	m_Index,m_IndexData;

	if(((gBrake.OnCounter == 0)&&(gBrake.TotalCounter > 1))||(gBrake.TotalCounter == 0))
	{
		gBrake.IBreakrsm    = 0;
        gBrake.TotalCounter	= 0;
	}	
	else if(gBrake.TotalCounter > 1)
	{
		DINT;		
		Duty = ((Ulong)gBrake.OnCounter<<20)/gBrake.TotalCounter;
		gBrake.OnCounter 		= 0;
		gBrake.TotalCounter		= 0;
		EINT;
		IBreakrsm = gBrake.IBreakTotal;
		Duty = qsqrt(Duty);
		gBrake.IBreakrsm = IBreakrsm*Duty>>10;
	}
	/*制动管过载判断*/
	if (gBrake.IBreakrsm > gInvInfo.InvBreakCurrent)
	{	
        m_Index = (Ulong)gBrake.IBreakrsm * 100UL/gInvInfo.InvBreakMaxCurrent;
		if (m_Index >= 100)     
		{
			m_IndexData= (m_Index - 100)/10;
            m_IndexData= __IQsat(m_IndexData,1,0);
            if(m_Index < 120)
            {
    			m_CurrCoff = *(gBrake.pBreakOverLoad+(1 - m_IndexData));
            }
            else
            {
    			m_CurrCoff = 1;//累加的速度是1000倍，100ms报过载
            }
		}
		else if (m_Index >= 90)    
		{
            m_Index = ((int)m_Index - (int)gBrake.minPercent)/gBrake.OverLoadInterval;
            m_Index = __IQsat(m_Index,4,-1);
			
			m_Index = (6 - m_Index);
            
			m_CurrCoff = *(gBrake.pBreakOverLoad + m_Index);			
		}
		else
		{
			m_Index = (m_Index - 70)/5;
            m_Index = __IQsat(m_Index,3,1);
            
			m_CurrCoff = *(gBrake.pBreakOverLoad+(10 - m_Index));
		}
		gOverLoad.BreakTotal += (2000000L/m_CurrCoff);//累加的速度是10倍，10s报过载
	}	
	else if (gOverLoad.BreakTotal > gBrake.OverLoadDegradeCoff)  // 电流较小时过载计数值减小
	{
		gOverLoad.BreakTotal -= gBrake.OverLoadDegradeCoff; //减小时间90s
	}
	else
	{
		gBrake.FilterTicker = 0;
	}
	if (gOverLoad.BreakTotal >= 100000000L) 
	{
		gOverLoad.BreakTotal = 100000000L;
		gError.ErrorCode.all |= ERROR_OVERLOAD_BRAKE;
        gError.ErrorInfo[3].bit.Fault4 = 1;
        gLineCur.ErrorShow = gLineCur.CurPer; 
	}
    
}
#else
/*************************************************************
	制动电阻电短路保护处理
	
检测条?
1、sizeD\SizeE,即7.5KW-30KW才进行制动电阻短路保护检测
2、停机状态、制动单元没有打开也检测
判断原则:
1)机型为SizeD和SizeE
2)第一次进入过流中断，只记录过流故障但不上传故障，此时关断制动电阻同时进入停机状态
3)10ms后若TZ信号仍然为低电平，则认为是制动电阻短路，否则为过流故障
4)制动电阻短路故障不允许手动复位，只能掉电复位

*************************************************************/
void  BrakeResShortProtect(void)
{
    if((19 > gInvInfo.InvTypeApply)&&(12 < gInvInfo.InvTypeApply))
    {
        gBrake.BreResShotDetFlag = 1;            //7.5KW-30KW才需要检测
    }
    else
    {
        gBrake.BreResShotDetFlag = 0;
    } 

    if((gBrake.BreResShotDetFlag == 1)&&(gBrake.ErrCode == ERROR_OVER_CURRENT))//只有第一次为过流故障时需要区分制动电阻短路与过流
    {                                                          //若第一次记录为过压故障，则直接报过压 
        gBrake.WaitTime++;
        if(gBrake.WaitTime < 6)  //10ms
        {
            EALLOW;
        	EPwm1Regs.TZCLR.bit.OST = 1;
        	EDIS;
        }
        else
        {                                              
            if(EPwm1Regs.TZFLG.bit.OST == 1)
            {
                gError.ErrorCode.all |= ERROR_SHORT_BRAKE; //60 不能手动复位     
                //gError.ErrorInfo[0].bit.Fault1 = 1;
            }
            else
            {
                gError.ErrorCode.all |= ERROR_OVER_CURRENT ;       
                gError.ErrorInfo[0].bit.Fault1 = 1;
            }
            gBrake.WaitTime = 0;
            gBrake.ErrCode  = 0;
        }
    }
    else
    {
        gBrake.WaitTime = 0;
    }
}
#endif
