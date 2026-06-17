/****************************************************************
文件功能：载频和载波周期相关计算,包括调制方式的确定
文件版本:
更新日期：

****************************************************************/
#include "MotorInclude.h"
#include "MotorEncoder.h"
#include "MotorPwmInclude.h"
#include "MotorIPMSVC.h"

// 全局变量定义
FC_CAL_STRUCT			gFcCal;
SYN_PWM_STRUCT			gSynPWM;
ANGLE_STRUCT			gPhase;		//角度结构
DEAD_BAND_STRUCT		gDeadBand;
// 同步调制频率分段表、载波比表、每载波周期步长表
Uint const gSynFreqTable[10] =
{
		75,		113, 	169, 	250, 	355, 
//NC=	80		60		42		30		20
		535, 	854, 	1281, 	1922, 	65535
//NC=	12		9		6		3
};

Uint const gSynNcTable[10] = 
{
		80,		60,		42,		30,		20,
		12,		9,		6,		3,		1
};

Ulong const gSynStepAngleTable[10] = 	//65536*65536/N
{
	53687091,	71582788,	102261126,	143165576,	214748365,	
	357913941,	477218588,	715827883,	1431655765, 4294967295 
};

Uint const gTempLowLimitTable[19] =
{
        30, 30, 30,30,30,         //16-20
        30, 30, 30,30,30,             //21-25
        30, 30, 30,30,30,             //26-30
        30, 30, 30,30,             //31-34
};
// 文件内部函数声明
void AsynPWMAngleCal(Ulong);
void SynPWMAngleCal(void);
s16 CompOnePhase(s16 phase, Uint Comp);

#if (SOFTSERIES == MD500SOFT)
/*************************************************************************
函数名称：载频处理
功能：主要包括SVC低频处理，最大最小载频处理以及随温度降载频功能
随温度降载频，在过温点减10度时降到4K5段发波，达到过温点减5度时减到3K5段，同时只要设定载频低于4.5K或者
降载频时低于4.5K都按5段发波，否则是7段发波，降载频的下限是3K5段
*************************************************************************/
void CalCarrierWaveFreq(void)
{
    Uint    m_MaxFc;
    Uint    m_MinFc;
    s16	    m_TempLim;
	Uint    m_FcApply,m_LowLimitFc,m_MiddleFc;
    Uint    * m_TempLowLimitTable;
	static Uint m_VarFcByTem = 0;
	// 确定载波频率
	gFcCal.Time++;
//    gFcCal.OverLoadTime++;
    
	gFcCal.Cnt += gBasePar.FcSetApply;                //载频低的时候，调整的慢一些
	if(gFcCal.Cnt < 100)				
	{
		return;
	}
	gFcCal.Cnt = 0;
	
	//...此处作FC和温度关系的计算
	m_TempLim = gTemperature.OverTempPoint - 15;  
	m_TempLim = Max(m_TempLim,70);   //限幅
	m_TempLowLimitTable = (Uint *)gTempLowLimitTable;    //载频下限
	m_MiddleFc = 40;
	if((gInvInfo.InvTypeApply > 15)&&(gInvInfo.InvTypeApply <= 34))
    {
        m_LowLimitFc = *(m_TempLowLimitTable + gInvInfo.InvTypeApply - 16);
    }
    else
    {
        m_LowLimitFc = 30;
    }

	if((gSubCommand.bit.VarFcByTem != 0) &&         //载频随温度调整功能选择
	   (gMainStatus.RunStep != STATUS_GET_PAR) &&   // 调谐时，载波不调整
	   (gTemperature.Temp > (m_TempLim-5)) &&       //温度大于（限制值-5）度
	   (m_LowLimitFc < gBasePar.FcSet))                       //载频随温度降低的下限是1K，设定频率低于1K不调整
	{
		if(gFcCal.Time >= 10000)				    //每20秒调整一次
		{
			gFcCal.Time = 0;
			if(gTemperature.Temp <= m_TempLim)	    //载波频率逐渐回升
			{
				gFcCal.FcBak += 5;
			}
			else if((gTemperature.Temp >= (m_TempLim+5))&&(gFcCal.FcBak > m_MiddleFc))		
			{
			    gFcCal.FcBak -= 5;				    //载波频率逐渐降低
				gFcCal.FcBak = (gFcCal.FcBak < m_MiddleFc) ? m_MiddleFc : gFcCal.FcBak;
				m_VarFcByTem = 1;
			}
			else if(gTemperature.Temp >= (m_TempLim+10))
			{
				gFcCal.FcBak -= 5;				    //载波频率逐渐降低
				gFcCal.FcBak = (gFcCal.FcBak < m_LowLimitFc) ? m_LowLimitFc : gFcCal.FcBak;
				m_VarFcByTem = 1;
			}
		}
		if(gFcCal.FcBak >= 45)
		{
		    m_VarFcByTem = 0;
		}
	}
	else										    //载波频率不需调整情况
	{
		gFcCal.Time = 0;
		gFcCal.FcBak = gBasePar.FcSet;
		m_VarFcByTem = 0;
	}


    gFcCal.FcBak = (gFcCal.FcBak > gBasePar.FcSet) ? gBasePar.FcSet : gFcCal.FcBak;

//...开始作FC的最大和最小限制
	m_MinFc = 8;								    //最小0.8KHz载波频率
	m_MaxFc = 120;								    //最大12KHz载波频率

	//MD500修改
	if(gInvInfo.InvTypeApply > 22)		
	{
		m_MaxFc = 60;							    //笥?2机型(75KW)最大6KHz载波频率
        if(gTestDataReceive.TestData4 == 1)
		{
		    m_MaxFc = 80;
		}
	}
    if(gMainCmd.Command.bit.ControlMode != IDC_VF_CTL)
	{        
	    if((gTestDataReceive.TestData4 == 1)&&(gMainCmd.Command.bit.ControlMode != IDC_SVC_CTL))
		{
	        m_MaxFc = (m_MaxFc>80)?80:m_MaxFc;          //VC运行6KHz最大载波频率限制
		}
		else
		{
		    m_MaxFc = (m_MaxFc>60)?60:m_MaxFc; 
		}
		m_MinFc = 20;							    //VC运行2KHz最小载波频率限制
	}

	gFcCal.FcBak = __IQsat(gFcCal.FcBak,m_MaxFc,m_MinFc);
    if((SYNC_SVC == gCtrMotorType)&&(INV_VOLTAGE_1140V != gInvInfo.InvVoltageType))
    {
        PmsmSvcCalFc();                     
        m_MinFc = Min(gPmsmRotorPosEst.FcLow,m_MinFc);   // 增加了零速载频处理
        m_MinFc = Max(m_MinFc,10);
		gFcCal.FcBak = __IQsat(gFcCal.FcBak,m_MaxFc,m_MinFc);
    }//同步机SVC控制低速下的载波频率会自动调整（减小）

    m_FcApply = gFcCal.FcBak;

	if((gBasePar.FcSet < 45)||(m_VarFcByTem == 1))     //载频低于4.5K时改为5段发波
	{
	    if(gMainCmd.FreqReal > gPWM.PwmModeSwitchLF + 300)
		{ 
	        gPWM.PWMModle = MODLE_DPWM;
		}
		else if(gMainCmd.FreqReal < gPWM.PwmModeSwitchLF)
		{
		    gPWM.PWMModle = MODLE_CPWM;
		}
	}
	else
	{
	    gPWM.PWMModle = MODLE_CPWM;
	}

	//	CPWM时载频是DPWM(设定载频)的2/3
    if((gPWM.PWMModle == MODLE_CPWM)&&(gTestDataReceive.TestData4 != 1)) 
    {	
        if((gFcCal.FcBak <= 30) && (gFcCal.FcBak >= 20))
        {
        	m_FcApply = 20;   
        }
		//	3k以上乘2/3
        else if (gFcCal.FcBak > 30)
        {
        	m_FcApply = (gFcCal.FcBak * 171) >> 8; 
        }
            
    }

//...开始调整载波频率，每40ms调整0.1KHz
	if(m_FcApply > gBasePar.FcSetApply)
	{
		gBasePar.FcSetApply ++;
	}
	else if(m_FcApply < gBasePar.FcSetApply)
	{
		gBasePar.FcSetApply --;
	}

// 同步调制与异步调制的选择
	gSynPWM.AbsFreq = gMainCmd.FreqReal/100;
    // 自动切换

    gSynPWM.ModuleApply = 0;
    gSynPWM.FcApply = ((Ulong)gBasePar.FcSetApply)<<9;

// 根据运行频率计算输出电压相位角度变化步长 : gPhase.StepPhase
	AsynPWMAngleCal(gSynPWM.FcApply);

	if(gPWM.SoftPWMTune !=0)    // 根据情况限值随机PWM深度上限
	{
        if(50 < gBasePar.FcSetApply )  
	    {
	        gPWM.SoftPWMTuneApply = Min(gPWM.SoftPWMTune,3);
	    }
		else if( 30 < gBasePar.FcSetApply ) 
		{
		    gPWM.SoftPWMTuneApply = Min(gPWM.SoftPWMTune,5);
		} 
		else
		{
		    gPWM.SoftPWMTuneApply = Min(gPWM.SoftPWMTune,6);
		}
	}  
}
#else
/*************************************************************************
函数名称：载频处理
功能：主要包括SVC低频处理，最大最小载频处理以及随温度降载频功能
随温度降载频，在过温点减10度时降到4K5段发波，达到过温点减5度时减到3K5段，同时只要设定载频低于4.5K或者
降载频时低于4.5K都按5段发波，否则是7段发波，降载频的下限是3K5段
*************************************************************************/
void CalCarrierWaveFreq(void)
{
    Uint    m_MaxFc;
    Uint    m_MinFc;
    s16	    m_TempLim;
	Uint    m_FcApply,m_LowLimitFc,m_MiddleFc;
//    Uint    * m_TempLowLimitTable;
	static Uint m_VarFcByTem = 0;
	static u16 m_FreqRealFlag = 0;

	// 确定载波频率
	gFcCal.Time++;
//    gFcCal.OverLoadTime++;
    gBasePar.FcSet = (gBasePar.FcSetToMotor*3)>>1;    //降功能传过来的载频乘以1.5倍
	gFcCal.Cnt += gBasePar.FcSetApply;                //载频低的时候，调整的慢一些
	if(gFcCal.Cnt < 100)				
	{
		return;
	}
	gFcCal.Cnt = 0;
	
	//...此处作FC和温度关系的计算
	m_TempLim = gTemperature.OverTempPoint - 15;  
	m_TempLim = Max(m_TempLim,70);   //限幅
	//m_TempLowLimitTable = (Uint *)gTempLowLimitTable;    //载频下限
	m_MiddleFc = 40;
    m_LowLimitFc = 30;

	if((gSubCommand.bit.VarFcByTem != 0) &&         //载频随温度调整功能选择
	   (gMainStatus.RunStep != STATUS_GET_PAR) &&   // 调谐时，载波不调整
	   (gTemperature.Temp > (m_TempLim-5)) &&       //温度大于（限制值-5）度
	   (m_LowLimitFc < gBasePar.FcSet))                       //载频随温度降低的下限是1K，设定频率低于1K不调整
	{
		if(gFcCal.Time >= 10000)				    //每20秒调整一次
		{
			gFcCal.Time = 0;
			if(gTemperature.Temp <= m_TempLim)	    //载波频逐渐回升
			{
				gFcCal.FcBak += 5;
			}
			else if((gTemperature.Temp >= (m_TempLim+5))&&(gFcCal.FcBak > m_MiddleFc))		
			{
			    gFcCal.FcBak -= 5;				    //载波频率逐渐降低
				gFcCal.FcBak = (gFcCal.FcBak < m_MiddleFc) ? m_MiddleFc : gFcCal.FcBak;
				m_VarFcByTem = 1;
			}
			else if(gTemperature.Temp >= (m_TempLim+10))
			{
				gFcCal.FcBak -= 5;				    //载波频率逐渐降低
				gFcCal.FcBak = (gFcCal.FcBak < m_LowLimitFc) ? m_LowLimitFc : gFcCal.FcBak;
				m_VarFcByTem = 1;
			}
		}
		if(gFcCal.FcBak >= 45)
		{
		    m_VarFcByTem = 0;
		}
	}
	else										    //载波频率不需调整情况
	{
		gFcCal.Time = 0;
		gFcCal.FcBak = gBasePar.FcSet;
		m_VarFcByTem = 0;
	}


    gFcCal.FcBak = (gFcCal.FcBak > gBasePar.FcSet) ? gBasePar.FcSet : gFcCal.FcBak;
    //根据当前的过载情况减小载波频率，超过过载的50％后开始降低载波频率，到6KHz(4K七段)
	if(gOverLoad.InvTotal.half.MSW < 16000)
	{ 
		gFcCal.FcLimitOvLoad = gBasePar.FcSet*10;
	}
	else if(gOverLoad.InvTotal.half.MSW > 18000)
	{
		gFcCal.FcLimitOvLoad--;
		if(gFcCal.FcLimitOvLoad < 600)	gFcCal.FcLimitOvLoad = 600;
	}
	if(gFcCal.FcBak > (gFcCal.FcLimitOvLoad/10))
	{
		gFcCal.FcBak = (gFcCal.FcLimitOvLoad/10);
	}


//...开始作FC的最大和最小限制
	m_MinFc = 8;								    //最小0.8KHz载波频率
	m_MaxFc = 120;								    //最大12KHz载波频率

	//MD500修改
	if(gInvInfo.InvTypeApply > 22)		
	{
		m_MaxFc = 60;							    //笥?2机型(75KW)最大6KHz载波频率
	}
    if(gMainCmd.Command.bit.ControlMode != IDC_VF_CTL)
	{        	    
	    if(gMainCmd.Command.bit.ControlMode == IDC_SVC_CTL)
		{
	        m_MaxFc = (m_MaxFc>90)?90:m_MaxFc; 		
		}
		m_MinFc = 20;							    //VC运行2KHz最小载波频率限制
	}

    if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
    {
        m_MaxFc = (m_MaxFc>60)?60:m_MaxFc;		    //690V最大载频4K，最小2K
	    m_MinFc = 20; 
    } 

	if((gMainCmd.FreqReal < 800)&&(m_FreqRealFlag == 1))					
	{
	    m_MaxFc = m_MaxFc >> 1;		    //8Hz以下最大载波频率限制为设定载频的一半
    }
	else if(gMainCmd.FreqReal < 700)    //增加回差，防止在8Hz运行时载频来回变化
	{
	    m_FreqRealFlag = 1;
	}
	else
	{
	    m_FreqRealFlag = 0;
	}  
	
	   
    if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
    {
		m_MaxFc = (m_MaxFc>15)?15:m_MaxFc;		    //1140V最大载频1K，最小0.8K
		m_MinFc = 8;							    //2011.5.7 L1082
    }



	gFcCal.FcBak = __IQsat(gFcCal.FcBak,m_MaxFc,m_MinFc);
    if((SYNC_SVC == gCtrMotorType)&&(INV_VOLTAGE_1140V != gInvInfo.InvVoltageType))
    {
        PmsmSvcCalFc();                     
        m_MinFc = Min(gPmsmRotorPosEst.FcLow,m_MinFc);   // 增加了零速载频处理
        m_MinFc = Max(m_MinFc,10);
		gFcCal.FcBak = __IQsat(gFcCal.FcBak,m_MaxFc,m_MinFc);
    }//同步机SVC控制低速下的载波频率会自动调整（减小）

    m_FcApply = gFcCal.FcBak;

    if((INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)||(INV_VOLTAGE_690V == gInvInfo.InvVoltageType))
	{
	    gPWM.PWMModle = MODLE_CPWM;
	}
	else
	{
		if((gBasePar.FcSet < 45)||(m_VarFcByTem == 1))     //载频低于4.5K时改为5段发波
		{
		    if(gMainCmd.FreqReal > gPWM.PwmModeSwitchLF + 300)
			{ 
		        gPWM.PWMModle = MODLE_DPWM;
			}
			else if(gMainCmd.FreqReal < gPWM.PwmModeSwitchLF)
			{
			    gPWM.PWMModle = MODLE_CPWM;
			}
		}
		else
		{
		    gPWM.PWMModle = MODLE_CPWM;
		}
	}
	//	CPWM时载频是DPWM(设定载频)的2/3
    if((gPWM.PWMModle == MODLE_CPWM)&&(gTestDataReceive.TestData4 != 1)) 
    {	
        if((gFcCal.FcBak <= 30) && (gFcCal.FcBak >= 20))
        {
        	m_FcApply = 20;   
        }
		//	3k以上乘2/3
        else if (gFcCal.FcBak > 30)
        {
        	m_FcApply = (gFcCal.FcBak * 171) >> 8; 
        }           
    }
    if(gMainCmd.Command.bit.ControlMode == IDC_SVC_CTL)
	{
        m_FcApply = Min(m_FcApply,60);         // 最大载频限制为6.0K
	}
	else
	{
	    m_FcApply = Min(m_FcApply,80);         // 最大载频限制为8.0K
	}
//...开始调整载波频率，每40ms调整0.1KHz
	if(m_FcApply > gBasePar.FcSetApply)
	{
		gBasePar.FcSetApply ++;
	}
	else if(m_FcApply < gBasePar.FcSetApply)
	{
		gBasePar.FcSetApply --;
	}

// 同步调制与异步调制的选择
	gSynPWM.AbsFreq = gMainCmd.FreqReal/100;
    // 自动切换

    gSynPWM.ModuleApply = 0;
    gSynPWM.FcApply = ((Ulong)gBasePar.FcSetApply)<<9;

// 根据运行频率计算输出电压相位角度变化步长 : gPhase.StepPhase
	AsynPWMAngleCal(gSynPWM.FcApply);

	if(gPWM.SoftPWMTune !=0)    // 根据情况限值随机PWM深度上限
	{
        if(50 < gBasePar.FcSetApply )  
	    {
	        gPWM.SoftPWMTuneApply = Min(gPWM.SoftPWMTune,3);
	    }
		else if( 30 < gBasePar.FcSetApply ) 
		{
		    gPWM.SoftPWMTuneApply = Min(gPWM.SoftPWMTune,5);
		} 
		else
		{
		    gPWM.SoftPWMTuneApply = Min(gPWM.SoftPWMTune,6);
		}
	}  
}
#endif

#if (SOFTSERIES == MD500SOFT)
//G型机变频器额定电流降额曲线表，载频为5段发波载频
/*************************************************															
Phd(kW)	3kHz		4kHz		5kHz		6kHz		7kHz		8kHz		9kHz		10kHz	11kHz	12kHz	13kHz	14kHz	15kHz	16kHz	17kHz	18kHz
18.5		100.0%	100.0%	100.0%	100.0%	95.7%	91.6%	87.9%	84.3%	81.0%	77.9%	74.9%	72.1%	69.4%	66.9%	64.5%	62.3%
22		100.0%	100.0%	100.0%	100.0%	95.5%	91.4%	87.6%	83.9%	80.5%	77.3%	74.2%	71.3%	68.6%	66.0%	63.6%	61.3%
30		100.0%	100.0%	100.0%	100.0%	93.5%	87.7%	82.4%	77.2%	72.5%	68.3%	64.1%	60.3%	56.8%	53.6%	50.6%	47.9%
37		100.0%	100.0%	100.0%	94.4%	89.0%	84.0%	79.4%	74.9%	70.8%	67.1%	63.4%	60.1%	56.9%	54.0%	51.3%	48.7%
45		100.0%	100.0%	100.0%	94.0%	88.6%	83.7%	79.2%	75.2%	71.5%	68.1%	65.0%	62.2%	59.6%	57.1%	54.9%	52.8%
55		100.0%	100.0%	94.1%	88.9%	84.0%	79.5%	75.5%	71.7%	68.3%	65.2%	62.3%	59.7%	57.2%	55.0%	52.8%	50.9%
75		100.0%	91.8%	84.2%	77.5%	70.9%	65.2%	60.1%	55.1%	50.6%	46.7%	42.8%	39.4%	36.2%	33.3%	30.6%	28.2%
90		100.0%	90.8%	82.3%	74.9%	67.7%	61.5%	56.0%	50.8%	46.2%	42.3%	38.5%	35.1%	32.0%	29.4%	26.8%	24.6%
110		100.0%	91.9%	84.4%	77.7%	71.3%	65.5%	60.4%	55.4%	51.1%	47.2%	43.5%	40.2%	37.1%	34.4%	31.9%	29.7%
*************************************************/
#define MD500MININVTYPE 16//最小功率18.5kw
#define MD500MAXINVTYPE 33//最大功率400kw
#define OUTTYPERANGE 1//超过功率范围标志
#define CUTDOWMFCSTEP 10//每个降额点的间隔
#define CUTDOWMFCMIN 30//降额最低载频
#define CUTDOWMFCMAX 120//降额最高载频
#define FCNUMPERTYPE 10//每个功率等级的降额点数
Uint const gInvCutDownCurTableG[180] =
{
//	3k	  4k   5k   6k   7k   8k   9k   10k  11k  12k
	1000, 1000,1000,1000,957, 916,	879, 843, 810, 779,// 18.5
	1000, 1000,1000,1000,955, 914,	876, 839, 805, 773,// 22
	1000, 1000,1000,1000,935, 877,	824, 772, 725, 683,// 30
	1000, 1000,1000,944, 890, 840,	794, 749, 708, 671,// 37
	1000, 1000,1000,940, 886, 837,	792, 752, 715, 681,// 45
	1000, 1000,941, 889, 840, 795, 755, 717, 683, 652, // 55
	1000, 1000,842, 775, 709, 652, 601, 551, 506, 467, // 75  4K不降额
	1000, 1000,823, 749, 677, 615, 560, 508, 462, 423, // 90  4K不降额
	1000, 1000,844, 777, 713, 655, 604, 554, 511, 472, // 110 4K不降额
	1000, 1000,867, 808, 751, 700, 653, 607, 566, 529, // 132 4K不降额
	1000, 926, 857, 797, 739, 688, 642, 598, 558, 523, // 160
	1000, 900, 810, 731, 657, 592, 536, 483, 437, 398, // 200
    1000, 908, 825, 751, 682, 620, 566, 515, 470, 431, // 220
    1000, 912, 832, 762, 696, 638, 587, 538, 494, 456, // 250
    1000, 918, 843, 778, 716, 660, 612, 565, 523, 486, // 280
    1000, 917, 839, 770, 703, 644, 592, 541, 496, 457, // 315
    1000, 923, 852, 787, 725, 669, 619, 570, 527, 489, // 355
    1000, 914, 836, 768, 704, 647, 597, 548, 506, 468  // 400
};
#endif

//MD500的随载频降额曲线选择，在计算过载时要考虑降额，载频越高，同样的电流下报过载时间越短。
//软件实现方法是计算过载用的电流要除以降额系数。
#if (SOFTSERIES == MD500SOFT)	
Uint InvCurForCutDown;
void CutDownCurForOverLoad(void)
{
    Uint m_InvTypeMin,m_InvTypeMax;
    Uint m_FcApply,m_Index;
    Uint *m_CutDownCurLine;
	Uint m_OutRangeFlag = 0;
    	
	// 最大和最小机型，P型机和G型机不一样
	m_InvTypeMin = MD500MININVTYPE + gInvInfo.GPType - 1;
	m_InvTypeMax = MD500MAXINVTYPE + gInvInfo.GPType - 1;

	if (gInvInfo.InvType >= m_InvTypeMin && gInvInfo.InvType <= m_InvTypeMax)
	{
		if (gInvInfo.GPType == 1)
		{// G型机的降额
			m_CutDownCurLine = (Uint *)gInvCutDownCurTableG;
		}
		else
		{// P型机的降额
			m_CutDownCurLine = (Uint *)gInvCutDownCurTableG;
		}	
	}
	else
	{
		m_OutRangeFlag = OUTTYPERANGE;
	}
	
    m_FcApply = gBasePar.FcSetApply;
	if(gPWM.PWMModle == MODLE_CPWM)
	{
	    m_FcApply = (gBasePar.FcSetApply * 192)>>7;     
	}
	
    if(m_FcApply > CUTDOWMFCMAX)
	{
		m_FcApply = CUTDOWMFCMAX;
	}     
	else if (m_FcApply < CUTDOWMFCMIN)
	{
		m_FcApply = CUTDOWMFCMIN;
	}	

	if (m_OutRangeFlag == OUTTYPERANGE)
	{
		InvCurForCutDown = 1000;
	}
	else
	{	
		m_Index = (m_FcApply - CUTDOWMFCMIN) / CUTDOWMFCSTEP \
				+ (gInvInfo.InvType - m_InvTypeMin) * FCNUMPERTYPE;
		InvCurForCutDown = *(m_CutDownCurLine + m_Index) -
		                 (((long)(*(m_CutDownCurLine + m_Index)) - (*(m_CutDownCurLine + m_Index + 1))) * 
				          (long)(m_FcApply % CUTDOWMFCSTEP))/CUTDOWMFCSTEP;
	}
	// 计算变频器额定电流为基值的标么值电流
	if(gTestDataReceive.TestData4 == 1)   //不降额
	{
	    gLineCur.CurBaseInvOverload = gLineCur.CurBaseInv;
	}
	else     // 75kw~132Kw4K载频不降额
	{
	    gLineCur.CurBaseInvOverload = (Ulong)gLineCur.CurBaseInv * 1000 / InvCurForCutDown;
	}

}
#endif

/************************************************************
0.5ms调用:
	根据当前载波频率和运行频率计算载波周期内相位变化步长，
	区分同步调制和异步调制

1. 在VC，SVC控制时使用异步调制；
2. 运行频率小于75Hz使用异步调制；
3. 手动选择异步调制有效；
4. 只有在Vf控制，并且运行频率大于75Hz时，并且手动选择同步调制才切换到同步调制；
5. 75Hz切换时，存在10Hz的滞环；
************************************************************/

/************************************************************
	异步调制情况下根据当前载波频率和运行频率计算载波周期内相位变化步?
Input:    gFCApply  gSpeed.SpeedGivePer
OutPut:   gPhase.StepPhase  gPhase.CompPhase gPWMTc
TC/2 = 250000/Fc  
StepPhase = (TC/2)*f*Maxf*1125900 >> 32
************************************************************/
void AsynPWMAngleCal(Ulong FcApply)
{
	Uint  m_HalfTc,m_AbsFreq,m_Fc;

	Ulong m_LData = (Ulong)PWM_CLOCK_HALF * 10000L * 512L;
	if(FcApply > 61440)
	{
		m_Fc = FcApply>>1;
		m_LData = m_LData>>1;
	}
	else
	{
		m_Fc = FcApply;
	}
	m_HalfTc = (Uint)(m_LData/m_Fc);

	m_AbsFreq  = abs(gMainCmd.FreqSyn);
    m_LData = ((Ullong)m_AbsFreq * (Ullong)gBasePar.FullFreq01 * 439804651L / m_Fc)>>16;


	m_LData = m_LData/2;
	
	DINT;
	if(gMainCmd.FreqSyn >= 0)
	{
		gPhase.StepPhase = m_LData;	
		gPhase.CompPhase = (m_LData>>15);// + COM_PHASE_DEADTIME;
	}
	else
	{
		gPhase.StepPhase = -m_LData;
		gPhase.CompPhase = -(m_LData>>15);// - COM_PHASE_DEADTIME;
	}
    
	gPWM.gPWMPrd = m_HalfTc;
	EINT;
}

/************************************************************
	同步调制下计算载波周期和每载波周期的角度变化步长
************************************************************/
/*void SynPWMAngleCal(void)
{
	Uint  m_AbsFreq,m_HalfTc,m_Index;
	Ulong m_Fc,m_Long,m_LData;

	m_Index = 0;
	while(gSynPWM.AbsFreq >= gSynFreqTable[m_Index]) 	
	{
		m_Index++;
		if(m_Index >= 10)	break;
	}

	m_Index--;
	if((m_Index < gSynPWM.Index) && 
	   (gSynPWM.AbsFreq <= gSynFreqTable[gSynPWM.Index]-10))
	{
		gSynPWM.Flag  = 1;					//表示频率减小、载波频率突然增加的过渡过程
		gSynPWM.Index = m_Index;
	}
	else if((m_Index > gSynPWM.Index) && 
	        (gSynPWM.AbsFreq > gSynFreqTable[gSynPWM.Index+1]+10))
	{
		gSynPWM.Flag  = 2;					//表示频率增加、载波频率突然减小的过渡过程
		gSynPWM.Index = m_Index;
	}

	m_AbsFreq  = abs(gMainCmd.FreqSyn);
	m_Long = ((Ullong)gBasePar.FullFreq01 * (Ullong)m_AbsFreq)>>7;
	m_Long = ((Ullong)m_Long * (Ullong)gSynNcTable[gSynPWM.Index] / 100)>>8;
    
	m_Fc = (((Ullong)m_Long<<9))/100;			//当前载波频率

	if(gSynPWM.Flag == 1)
	{
		gSynPWM.FcApply += 100;				//每载波周期变化约0.02KHz载波频率
		if(gSynPWM.FcApply > m_Fc)
		{
			gSynPWM.Flag = 0;
		}
	}
	else if(gSynPWM.Flag == 2)
	{
		gSynPWM.FcApply -= 100;
		if(gSynPWM.FcApply < m_Fc)
		{
			gSynPWM.Flag = 0;
		}
	}
	else
	{
		gSynPWM.FcApply = m_Fc;
	}

	if(gSynPWM.Flag != 0)
	{
		AsynPWMAngleCal(gSynPWM.FcApply);
		return;
	}
    m_LData = (Ulong)PWM_CLOCK_HALF * 1000000L;
	m_HalfTc = m_LData/m_Long;

	DINT;
	if(gMainCmd.FreqSyn >= 0)
	{
		gPhase.StepPhase = gSynStepAngleTable[gSynPWM.Index];
		gPhase.CompPhase = (gSynStepAngleTable[gSynPWM.Index]>>15);// + COM_PHASE_DEADTIME;
	}
	else
	{
		gPhase.StepPhase = -gSynStepAngleTable[gSynPWM.Index];
		gPhase.CompPhase = -(gSynStepAngleTable[gSynPWM.Index]>>15);// - COM_PHASE_DEADTIME;
	}

	gPWM.gPWMPrd = m_HalfTc;
	EINT;
}
*/
/*************************************************************
	随机PWM处理，使载波周期和输出相位生效

随机PWN启用的条件是：
   异步调制 + 载频1k-6k之间 + Vf控制 + 手动选择启用；
*************************************************************/
/*void SoftPWMProcess(void)
{
	if(0 == EPwm1Regs.TBSTS.bit.CTRDIR)
	{
		gPWM.gPWMPrdApply = gPWM.gPWMPrd;
	}	        		
   	gPhase.StepPhaseApply = gPhase.StepPhase;	
  
}*/

void SoftPWMProcess(void)
{
#if 0
	Uint  m_Coff;
#else
	long  m_Coff;
//	static long Seed  = 1;
//	long  m_temp1 = 0;
#endif

	if((gPWM.SoftPWMTune == 0) || (gSynPWM.ModuleApply >= 1) ||     // 同步调制下不启用随机PWM
	   (gBasePar.FcSetApply < 10) || (gBasePar.FcSetApply > 60)||
	   (STATUS_GET_PAR == gMainStatus.RunStep)) // 载波频率1K～６K之间有效
	{
	    if(0 == EPwm1Regs.TBSTS.bit.CTRDIR)
		{
		    gPWM.gPWMPrdApplyLast = gPWM.gPWMPrdApply;
		    gPWM.gPWMPrdApply = gPWM.gPWMPrd;	    
	    }    		
   	    gPhase.StepPhaseApply = gPhase.StepPhase;	
		return;
	}
  
#if 1   
    if(0 == EPwm1Regs.TBSTS.bit.CTRDIR)
    {  	  	    
		m_Coff = gPWM.SoftPWMTuneApply * 205;    //随机PWM的鹘诳矶�10对应50%;大于3K时，则对应30%
		gPWM.SoftPWMCoff = gPWM.SoftPWMCoff + 29517 * GetTime();    //产生一个Q15格式的随机数
		m_Coff = 4096 + (((long)gPWM.SoftPWMCoff * (long)m_Coff)>>15);
#else 
    if(0 == EPwm1Regs.TBSTS.bit.CTRDIR)
    {   
	    //0-1 distribution
	    Seed = Seed * (long)2045 + (long)1; //xn = 2045 * xn1 + 1;
		Seed = Seed & 0x000fffff; //xn = xn mod 2^20
			
		m_Coff = ((Seed>>8)*(long)gPWM.SoftPWMTuneApply)/10L + (2048L*(20L-gPWM.SoftPWMTuneApply))/10L;//0.5 -> 1.5

		//if use the coff the carrier is ?
		m_temp1 = (((long)gBasePar.FcSetApply * m_Coff)>>12);

		// up limit coff
		if(m_Coff > (long)4096)
		{
			if(m_temp1 > (long)100)
			{
				m_Coff = ((long)100<<12)/(long)gBasePar.FcSetApply;
			}
		}

		//down limit coff
		if(m_Coff < (long)4096)
		{
			if(m_temp1 < (long)10)
			{
				m_Coff = ((long)10<<12)/(long)gBasePar.FcSetApply;
			}
		}
		//change the coff gian from frequncy to period
		m_Coff = ((long)1<<24)/m_Coff;
#endif 				
        gPWM.gPWMPrdApplyLast = gPWM.gPWMPrdApply;
 	    gPWM.gPWMPrdApply = ((Ulong)gPWM.gPWMPrd * (Ulong)m_Coff)>>12;		
	}
 	gPhase.StepPhaseApply = (Ulong)(gPhase.StepPhase>>12) * (Ulong)m_Coff;  
}

/************************************************************
	计算输出电压的相位角：VF情况下直接累加
************************************************************/
void CalOutputPhase(void)
{ 
    s32    m_Long;
	    //计算设定频率每载波周期的变化步长	开闭环矢量都需要计算
	m_Long = ((long)gMainCmd.FreqSet * (long)gBasePar.FullFreq01)/gBasePar.FcSetApply;//gBasePar.FullFreq01就是设定的最大频率对应的0.01HZ单位的值，就是放大100倍。
	//gMainCmd.FreqSet为设定的转速频率，单位为1HZ，      gBasePar.FcSetApply //实际载波频率，默认在启动的时候为1.5KHZ，这个值就为15，切换到角度闭环后就为4KHZ，这个值就为40
	//gMainCmd.FreqSetStep = m_Long/10000;		//Q16	//(2^32*Fset*Full)/(Fc*100*100*2^15*2)
	gMainCmd.FreqSetStep = (((llong)m_Long<<8)*gMainCmd.StepCoeff)/10000;
	gAsr.PosSet += gMainCmd.FreqSetStep;//gAsr.PosSet 只用于新的速度环中，其实作用是应该类似于位置控制中的角度增量
	gPhase.IMPhaseOld = gPhase.IMPhase;		// wg：在计算新的d轴相位前，保存旧的d轴相位
    
	//if(DC_CONTROL == gCtrMotorType) 	//直流制动
	//{
	    // 电压幅度在2ms计算，异步机无需考虑电压相位；
	    // 同步机暂时按M轴方向发电压， 电压角度增量为0；
	    // gPhase.OutPhase 不更新
		//return;
	//}    
    if(gCtrMotorType == SYNC_SVC)  // SVC执行时间最长，减小判断过程，缩小中断周期
    {                              //相位计算在速度和转子位置观测之后
        //在SVC模式下也可以同时用编码器或者旋变来采集速度
        if(gRotorSpeed.SvcSpeed != 0)//观察器得到的电机转速不为零的情况下
		{
	        if(0 == gPGData.PGMode)                         // 增量式 uvw, Abz
	        {
	            gIPMPos.ZIntFlag = 1;                // 进入ABZ编码器位置采样置位
	            QEPGetRealPos();
	            gIPMPos.ZIntFlag = 0;                      
	        }
	        else if(gPGData.PGType == PG_TYPE_RESOLVER)             // 非增量式, 旋变
	        {
	            ReadRTPos();
	            gPGData.RefPos   = gRotorTrans.AddPos + ((s32)gRotorTrans.PosComp<<8);  /*Q24格式 考虑了补偿角*/     /*编码器相对位置,折算到电角度，基准位置到达后不复位角度，用于速度环*/
	            gIPMPos.RotorPos = gRotorTrans.RTPos + gIPMPos.RotorZero + gRotorTrans.PosComp;//这是最后实际用到的//转子位置角标么值
	        }
        }
       /* if((gPmsmRotorPosEst.SpeedCheckEnable != 1))
		{
        	ReadRTPos();
		}*/
        return;
    }
    // 同步机获取转子位置，进行转子磁场定向

	if(0 == gPGData.PGMode)                         // 增量式 uvw, Abz
	{
	    gIPMPos.ZIntFlag = 1;                // 进入ABZ编码器位置采样置位
	    QEPGetRealPos();
	    gIPMPos.ZIntFlag = 0;                      
	}
	else if(gPGData.PGType == PG_TYPE_RESOLVER)             // 非增量式, 旋变
	{
        ReadRTPos();
        gPGData.RefPos   = gRotorTrans.AddPos + ((s32)gRotorTrans.PosComp<<8);  /*Q24格式 考虑了补偿角*/
	    gIPMPos.RotorPos = gRotorTrans.RTPos + gIPMPos.RotorZero + gRotorTrans.PosComp;
	}

    if(STATUS_GET_PAR == gMainStatus.RunStep) 
    {
        if(TUNE_STEP_NOW == PM_EST_BEMF)          
        {
            gPhase.IMPhaseApply += (long)(gPhase.StepPhaseApply * gMainCmd.StepCoeff);
		    gPhase.IMPhase = (s16)(gPhase.IMPhaseApply >> 16);
        }
        else if(TUNE_STEP_NOW == PM_EST_WITH_LOAD) 
        {
            gPhase.IMPhase  = (s16)gIPMPos.RotorPos;
			gPhase.IMPhaseApply = ((s32)gPhase.IMPhase << 16);
        }
		gPhase.OutVoltPhaseCom = 0;    // 防止在参数辨识时由于发波补偿问题造成发波异常
		return;
    }
    
    if(gCtrMotorType == SYNC_FVC)                       //同步机FVC输出相位计算
	{
        gPhase.IMPhase  = (s16)gIPMPos.RotorPos;	// 正常运行
		gPhase.IMPhaseApply = ((s32)gPhase.IMPhase << 16);
    }     
    else
    {
        gPhase.IMPhaseApply += (gPhase.StepPhaseApply * gMainCmd.StepCoeff);
		gPhase.IMPhase = (s16)(gPhase.IMPhaseApply >> 16);
    }
	gPhase.OutVoltPhaseCom = gPhase.IMPhase - gPhase.IMPhaseBak;
	gPhase.IMPhaseBak = gPhase.IMPhase;
}

/*************************************************************
	电流死区补偿极性判断
*************************************************************/
void CalDeadBandComp(void)
{
	s16   phase,m_Com;
	//u16   m_InvCurFilter;
	//s16	  DetaPhase;

	if(gMainCmd.FreqReal <= 40000)	    // 400.00Hz
	{
		m_Com = gDeadBand.Comp;
	}
	else 
	{
		m_Com = 0;//高频率时，死区补偿取消，否则会出现错误
	}
    /*m_InvCurFilter = Filter(m_InvCurFilter,gLineCur.CurBaseInv,2048);
	if(m_InvCurFilter < 792)
	{
	    if(m_InvCurFilter < 280)
		{
		    m_Com = 0;
		}
		else
		{
		    m_Com = ((u32)m_Com * (m_InvCurFilter - 279))>>9;
		}
	}
	*/

	if(gDeadBand.InvCurFilter < 512)
	{
	    m_Com = ((long)m_Com * gDeadBand.InvCurFilter)>>9;
	}
      
	if((gMainCmd.Command.bit.StartDC == 1) || (gMainCmd.Command.bit.StopDC == 1))
        
	{
        m_Com = 0;
	}
    
    gIAmpTheta.ThetaFilter = gIAmpTheta.Theta;/*死区补偿为AD中补偿，MD320 0.5MS补偿取消，角度滤波取消 2011.5.7 L1082 */

	if(gMainCmd.Command.bit.ControlMode != IDC_SVC_CTL)  
	{
	    phase = gPhase.IMPhase + gIAmpTheta.ThetaFilter + gPhase.CompPhase + 16384;
	}
	else                  //SVC时取消相位补偿
	{
	    phase = gPhase.IMPhase + gIAmpTheta.ThetaFilter + 16384;
	}
    gDeadBand.CompU = CompOnePhase(phase,m_Com);

    phase -= 21845;    
    gDeadBand.CompV = CompOnePhase(phase,m_Com);

    phase -= 21845;
    gDeadBand.CompW = CompOnePhase(phase,m_Com);

}
/*************************************************************
	判断某一相的死区补偿大小,+-3度范围内减小死区补偿的大小，
	这样的线性补偿可以有效避免高载波频率的震荡。
*************************************************************/
#define C_LINE_DEAD_TIME_COMP         //是否使用电流过零点线性狼钩?
s16 CompOnePhase(s16 phase, Uint Comp)
{
	s16  m_Comp,m_Phase;
#ifdef 	C_LINE_DEAD_TIME_COMP
	Uint m_AbsPhase,m_LimitPhase,m_Shift;
#endif

	m_Comp = (s16)Comp;
	m_Phase = phase;

#ifdef 	C_LINE_DEAD_TIME_COMP
    if(gMainCmd.Command.bit.ControlMode == IDC_FVC_CTL)
    {
        m_LimitPhase = 512*4;
        m_Shift = (9+2);
    }//闭环噶浚辔还愕悖狼钩ハ咝员浠姆段П淇恚苊獾缪雇槐?
    else
    {
        m_LimitPhase = 512;
        m_Shift = 9;
    }
	m_AbsPhase = abs(m_Phase);
	if(m_AbsPhase < m_LimitPhase)
	{
		m_Comp = ((Ulong)Comp * (Ulong)m_AbsPhase)>>m_Shift;
	}
	else
	{
		m_AbsPhase = abs((s16)(32767 - (Uint)m_Phase));
		if(m_AbsPhase < m_LimitPhase)
		{
			m_Comp = ((Ulong)Comp * (Ulong)m_AbsPhase)>>m_Shift;
		}
	}
#endif

	if(m_Phase > 0)			
    {
        m_Comp = -m_Comp;               /*电流为负的时候死区补偿为负电压*/
	}

	return (m_Comp);
}


