/****************************************************************
文件功能：处理同步机闭环矢量控制部分，弱磁等
文件版本：同步机运行主程序相关
最新更新：

****************************************************************/
//文件内容: 
//    1. 同步机停机状态位置检查；
//    2. 同步机运行前的初始位置确定；
//    3. 同步机ABZ编码器磁极位置的累加检测?
//    4. 同步机电感计算，电流环参数整定；
//    5. 同步机的弱磁处理；
//    6. ABZ/UVW编码器z中断的处理；

#include "MotorVCInclude.h"
#include "MotorInclude.h"
#include "MotorIPMSVC.h"
//************************************************************
IPM_POSITION_STRUCT		gIPMPos;          //永磁同步电机和转子角度相关的结构
PM_FLUX_WEAK            gFluxWeak;
PM_INIT_POSITION        gPMInitPos;
IPM_POS_CHECK_STRUCT	gIPMPosCheck;   //永磁同步电机上电检测当前绝对位置角的数据结构
PM_DECOUPLE             gPmDecoup;
PM_FW_IN	            gPMFwIn;
PM_FW		            gPMFwCtrl;
PM_FW_OUT	            gPMFwOut;
PMSM_FLUX_WEAK_STRUCT   gPmFluxWeak;
PM_CSR2_DATA            gPmCsr2;
int gOutVoltVoltPhaseApplyOld;

s16  PmsmMaxTorqCtrl(void);
//s16  PmsmFwcCalMethod(void);
extern void ADCEndIsrTune_POLSE_POS(void);
void PMFluxWeak05Ms(void);
void FluxWeak05Ms(void);
void IMFluxWeak2(void);
extern u16 IPMCalAbsPos(void);
extern void TurnToStopStatus(void);
extern void SetIPMRefPos(Uint Pos);
extern void PrepareParForRun(void);
/*************************************************************
	同步机下编码器的基准中断到达的处理程序(调用程序)
*************************************************************/
interrupt void PG_Zero_isr(void)
{
	if((*EQepRegs).QFLG.bit.IEL == 1)
    {
        EALLOW;
        (*EQepRegs).QCLR.bit.IEL = 1;
		(*EQepRegs).QCLR.bit.INT = 1;
      	PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;	// Acknowledge this interrupt
        EDIS;
	    if((gIPMZero.zFilterCnt < 4)||(gIPMPos.ZIntFlag == 1))            // Z 信号的滤波处理，要求两个Z间隔超过8ms(7500rpm),或者如果主中断正在计算位置，不进入Z中断
		{
			return;
		} 
	    gIPMZero.zFilterCnt = 0;
	       
	    gIPMPos.ZSigNumSet++;    
	    gIPMPos.ZBakRotorPos    = gIPMPos.RotorPos;
	    gIPMPos.QepCntBak       = GetQepCnt();
	    gIPMPos.QepCntPosCalBak = gIPMPos.QepCntPosCal;
#if(AIRCOMPRESSOR == 0)
	    gIPMPos.ZBakUVW         = Get_UVW_PG_U() + Get_UVW_PG_V() + Get_UVW_PG_W();                         /*QEP的Z信号中断，程序简短，不打开中断*/
#else
        gIPMPos.ZBakUVW         = 0;
#endif
    }
}
/*************************************************************
计算Z信号发生时，与最近一次中断函数之间的位置偏差
计算方式
在Z信号中断中，记录最近一次计算转子位置使用的QEPCNT值，和Z信号发生时的QEPCNT值，计算二者之间的偏差
*************************************************************/
void IPMPosCalZWindage(void)
{
    s32   m_DetaCnt;
    s32   m_Data;
    m_DetaCnt           = (s32)(gIPMPos.QepCntBak - gIPMPos.QepCntPosCalBak);
    m_Data              = ((m_DetaCnt<<14) * (s32)gMotorExtInfo.Poles);
    gIPMPos.ZPosWindage = (u16)(m_Data / (s32)gPGData.PulseNum);
}

/*************************************************************
	基准位置到达时候，同步机编码器位置角校正函数(假定编码器的
	Z信号接入了DSP，而不是用U信号的上升沿作为Z信号)
	(2ms程序中执行，能够满足30000RPM需求，再高速会丢失Z信号)
*************************************************************/
void IPMPosAdjustZIndex(void)
{
	s16 m_DetaPos;
	s32 m_DetaPosShow;
    static u16 m_ZErrCnt = 0;	

    if(gIPMPos.ZSigNum == gIPMPos.ZSigNumSet)
    {        
        return;                                                 // 没有Z信号到达，直接返回
    }

	if(gPGData.PGType == PG_TYPE_UVW)
	{
	    if((gIPMPos.ZBakUVW & 0x03) != 1)
	    {
	        return;                                             // UVW编码器要求Z信号到达的时候，V信号是0，W信号是1，否则认为是错误信号.
	    }   
	}
    gIPMPos.ZSigNum = gIPMPos.ZSigNumSet;
	
    // 开始判断Z信号到达的时候，记录的转子位置和希望复位的转子位置是否吻合(偏差不大)

    IPMPosCalZWindage();
    
    m_DetaPos = (s16)(gIPMPos.RotorZero - (gIPMPos.ZBakRotorPos + gIPMPos.ZPosWindage));
    gIPMPos.ZBakDetaPos = m_DetaPos;                            // Z信号到的时候，希望复位的角度和当前角度的差值

    m_DetaPosShow = ((s16)gIPMPos.RotorPos - (s16)(gPGData.RefPos >> 8));
	gIPMPos.DetaPosShow = (s16)(m_DetaPosShow * 1800L >>15);    // 在ABZ编码器时借用旋变位置角来显示位置偏差，wyk

    if((gIPMPos.ZResetFlag == C_Z_DONT_RESET_POS)||(gIPMPos.ZSigEnable == 0))   //不使用Z信号校正
    {
        return;                                                 // Z信号到的时候，不需要复位角度。
    }
  
    if((abs(m_DetaPos) > C_MAX_DETA_POS_Z) && (gIPMPos.ZResetFlag == C_Z_RESET_POS_LIMIT)&&(gMainStatus.RunStep != STATUS_GET_PAR))
    {
        m_ZErrCnt++;
        if(m_ZErrCnt > 5)
        {
   			m_ZErrCnt = 0;
   			gError.ErrorCode.all |= ERROR_ENCODER;
            gError.ErrorInfo[4].bit.Fault3 = 6;                      // Z信号到达时刻持续出错，认为角度已经偏离，报故障。
        }
        return;                                               
    }
	else
	{
	    m_ZErrCnt = 0;
	}
    gIPMPos.ZDetaPos    = m_DetaPos;
}
    

/***********************************************************************
    记录上次掉电时作为初始位置
************************************************************************/
void IPMCheckInitPos(void)
{    
	//接收上次掉电角度，并且识别保存角度是否合理
    if(gMainStatus.ParaCalTimes == 0)  //上电只计算一次的参数转换
	{
        gMainStatus.ParaCalTimes = 1;        
        
	    gIPMPos.PowerOffPosDeg = ((Ulong)gIPMPos.PowerOffPos << 16)/3600;	//识别初始角度,获取上次下电角度
	    SetIPMPos((Uint)gIPMPos.PowerOffPosDeg);
		SetIPMRefPos((Uint)gIPMPos.PowerOffPosDeg);
//		if(IDC_SVC_CTL == gMainCmd.Command.bit.ControlMode)       //svc上电后记录上一次磁极位置
	    {
	        SvcSetRotorPos(gIPMPos.PowerOffPosDeg);
	    }
    }        
}
/************************************************************
	永磁同步机磁极初始位置角检测阶段
************************************************************/
void RunCaseIpmInitPos(void)
{
	if((gError.ErrorCode.all != 0) || 
	   (gMainCmd.Command.bit.Start == 0))
	{
		DisableDrive();
        //if(gIPMPos.InitPosMethod == INIT_POS_VOLT_PULSE)
        //{
	    SynInitPosDetSetPwm(6);		    //同步机参数辨识恢复寄存器设置
        //}
		gIPMInitPos.Step = 0;
		ResetADCEndIsr();
		TurnToStopStatus();
		FlyingStartInitDeal();
		return;
	}

    switch(gMainStatus.SubStep)
    {
        case 1:
		    SetADCEndIsr(ADCEndIsrTune_POLSE_POS);
			if(gIPMInitPos.Step == 0)		
			{
       			gIPMInitPos.Step = 1;                       // 置静态辨识标志
       			gMainStatus.SubStep ++;
			}
			else
			{
			    gIPMInitPos.Step = 0;
			}
           
            break;

        case 2:
            if(gIPMInitPos.Step == 0)           // 中断辨识完成      
        	{ 
				gIPMPos.CompPos = ((Ulong)gIPMPos.CompPosFun << 16) / 3600;
				gIPMPos.InitPos = gIPMPos.InitPos + gIPMPos.CompPos;
    			SetIPMPos((Uint)gIPMPos.InitPos);
				SetIPMRefPos((Uint)gIPMPos.InitPos);
                SvcSetRotorPos(gIPMPos.InitPos);   

        		if(abs((int)(gIPMPos.PowerOffPosDeg - gIPMPos.RotorPos)) < 3641)
        		{
        			SetIPMPos((Uint)gIPMPos.PowerOffPosDeg);	    //rt
				    SetIPMRefPos((Uint)gIPMPos.PowerOffPosDeg);
                 //   SvcSetRotorPos(gIPMPos.PowerOffPosDeg);
        		}

                gIPMInitPos.Flag = 1;
        		DisableDrive();
				ResetADCEndIsr(); 
				if(gUVCoff.RsTune == 2) 
				{
//					SynCalLdAndLq(gIPMPos.InitPos);//SynCalLdAndLq(gIPMPos.RotorPos);
			  	    IPMCalAcrPIDCoff();
				}
        		gMainStatus.SubStep ++;             //可以多等一拍，让PWM恢复完成
        	}   //else waiting interrupt deal
            break;
            
        case 3:
            	InitSetPWM();
   	            InitSetAdc();
                SetInterruptEnable();	            // 如果辨识项目中途退出，中断有可能是关闭的，须在此打开
                gMainStatus.SubStep ++; 
                break;
                
        case 4:
		    //ResetADCEndIsr(); 
            PrepareParForRun();
            gMainStatus.RunStep = STATUS_STOP;
            gMainStatus.SubStep = 0;
            gMainStatus.PrgStatus.all = 0;		//所有控制有效

            break;
            
        default:
            gError.ErrorCode.all |= ERROR_TUNE_FAIL;
            gError.ErrorInfo[4].bit.Fault1 = 4;
            
            break;   
    }
}
#if 0
/************************************************************
	LD、LQ轴电感计算函数
	LAB = A-B*Cos(2*Theta+4*pi/3)	:= gIPMInitPos.LPhase[0]
	LBC = A-B*Cos(2*Theta)		     := gIPMInitPos.LPhase[1]
	LCA = A-B*Cos(2*Theta-4*pi/3)	:= gIPMInitPos.LPhase[2]
************************************************************/
void SynCalLdAndLq(Uint m_Pos)
{
	int m_Cos1,m_Cos2,m_Cos3;
	s32 m_CoffA,m_CoffB;
	Uint m_Angle;
	s32 m_Deta1,m_Deta2,m_DetaCos;
//	u16 m_L1,m_L2;
	u32 m_L1,m_L2;

	m_Angle = ((m_Pos + 32768)<<1);
	m_Cos1 = qsin(16384 - (int)m_Angle);        // cos(2*theta)
	m_Angle += 43691;
	m_Cos2 = qsin(16384 - (int)m_Angle);        // cos(2*theta + 4*pi/3)
	m_Angle = (m_Pos<<1) - 43691;
    m_Cos3 = qsin(16384 - (int)m_Angle);        // cos(2*theta - 4*pi/3)

    m_Deta1 = labs((s32)m_Cos3 - (s32)m_Cos1);
	m_Deta2 = labs((s32)m_Cos2 - (s32)m_Cos1);

	if(m_Deta1 <= m_Deta2)
	{
	    m_L2 = gIPMInitPos.LPhase[0];
        m_L1 = gIPMInitPos.LPhase[1];
	    m_DetaCos = ((s32)m_Cos1 - (s32)m_Cos2);
	}
	else
	{
	    m_L2 = gIPMInitPos.LPhase[2];
        m_L1 = gIPMInitPos.LPhase[1];
	    m_DetaCos = ((s32)m_Cos1 - (s32)m_Cos3);
	}

	m_CoffB = (s32)m_L2 - (s32)m_L1;
	m_CoffB = (m_CoffB << 15) / m_DetaCos;
	m_CoffA = (s32)gIPMInitPos.LPhase[1] + ((m_CoffB * (long)m_Cos1)>>15);

	m_CoffA = m_CoffA>>1;
	m_CoffB = m_CoffB>>1;
	gIPMInitPos.Ld = (u16)(m_CoffA + m_CoffB);
	gIPMInitPos.Lq = (u16)(m_CoffA - m_CoffB);

    gMotorExtReg.LD = gIPMInitPos.Ld;
    gMotorExtReg.LQ = gIPMInitPos.Lq;
}
#elif 1
/************************************************************
	LD、LQ轴电感计算函数
	LAB = A-B*Cos(2*Theta+4*pi/3)	:= gIPMInitPos.LPhase[0]
	LBC = A-B*Cos(2*Theta)		     := gIPMInitPos.LPhase[1]
	LCA = A-B*Cos(2*Theta-4*pi/3)	:= gIPMInitPos.LPhase[2]
************************************************************/
void SynCalLdAndLq(void)
{
	long m_L0,m_L1,m_L2,m_DetaL;

	m_L0 = (gIPMInitPos.LPhase[0] + gIPMInitPos.LPhase[1] + gIPMInitPos.LPhase[2]) / 6;
	m_L1 = (((llong)(gIPMInitPos.LPhase[0] - gIPMInitPos.LPhase[2])) << 14) / 28378L;
	m_L2 = gIPMInitPos.LPhase[1] - m_L0 * 2;
	m_DetaL = -(((long)qsqrt(m_L1 * m_L1 + m_L2 * m_L2)) / 2);

	gIPMInitPos.Ld = (u16)(m_L0 + m_DetaL);
	gIPMInitPos.Lq = (u16)(m_L0 - m_DetaL);

//	gIPMInitPos.InitPosSrc = atan(m_L2,m_L1) / 2;				// 初步计算磁极角度，若为凸极电机直接判断极性，表贴电机重新计算

    gMotorExtReg.LD = gIPMInitPos.Ld;
    gMotorExtReg.LQ = gIPMInitPos.Lq;
}
#endif
/************************************************************
	矢量控制的电流环调节器参数转换（预先转换好参数）
	时间的基值为:10/(2*pi*gBasePar.FullFreq), gBasePar.FullFreq01为0.01Hz单位，时间基值单位:秒
    先按照5KHz载波频率计算电流环参数，电流环整个滞后时间为TDelay = 0.75Tc = 150us
    150us对应标幺值为: (150 * 2 * pi * gBasePar.FullFreq)/(1000000*100) = gBasePar.FullFreq/106100
    (频率的基值为gBasePar.FullFreq，不考虑小数点)
    Kp = Ls / 2 * TDelay
       = 53050 * Ls/gBasePar.FullFreq        Q12格式
       
    ki = (Rs/2*TDelay)*Tc/2(离散化，每载波周期调整2次)
       = Rs/3; 
************************************************************/
void IPMCalAcrPIDCoff(void)
{
    u32 m_Long; 
    Uint m_UData,m_BaseL;
	Ulong m_Ulong;
    
	//电感基值为：阻抗基?2*pi*最大频率
	m_BaseL = ((Ulong)gMotorInfo.Votage * 3678)/gMotorInfo.Current;
	m_BaseL = ((Ulong)m_BaseL * 5000)/gBasePar.FullFreq01; 

	m_UData = ((Ulong)gMotorExtReg.RsPm * (Ulong)gMotorInfo.Current)/gMotorInfo.Votage;	
    gMotorExtPer.Rpm = ((Ulong)m_UData * 18597)>>14;
	m_Ulong = (((Ulong)gMotorExtReg.LD <<15) + m_BaseL) >>1;
    gMotorExtPer.LD = m_Ulong / m_BaseL / 10;                   // 同步机d轴电感Q13
    m_Ulong = (((Ulong)gMotorExtReg.LQ <<15) + m_BaseL) >>1;
    gMotorExtPer.LQ = m_Ulong / m_BaseL / 10;   

	m_Long = (53050UL * (u32)(gMotorExtPer.LD>>1))/gBasePar.FullFreq01;/*>>表示Q格式从Q13转换为Q12格式*/
    gPmParEst.IdKp  = (u16)Min(m_Long,8000);  // 最大值限制为8000,wyk
    gPmParEst.IqKp  =  gPmParEst.IdKp;      // IqKp使用D轴电感
    //m_Long = (53050UL * (u32)(gMotorExtPer.LQ>>1))/gBasePar.FullFreq01;
    //gPmParEst.IqKp  = (u16)Min(m_Long,16383);  // 32768   

    gPmParEst.IdKi  = (gMotorExtPer.Rpm/3);                            /*Q16格式*/
    gPmParEst.IqKi  = gPmParEst.IdKi; 

	if(gUVCoff.RsTune == 2)
	{
		if(gBasePar.FcSetApply > C_DOUBLE_ACR_MAX_FC)
		{
	        gImAcrQ24.KP = (long)gPmParEst.IdKp * gBasePar.FcSetApply / 100;         // 辨识时减小Kp,一次发波减小Kp和Ki       
	        gItAcrQ24.KP = (long)gPmParEst.IqKp * gBasePar.FcSetApply / 100;
	        gImAcrQ24.KI = (s32)gPmParEst.IdKi ;
	        gItAcrQ24.KI = (s32)gPmParEst.IqKi ;
		}
		else
		{
			gImAcrQ24.KP = (long)gPmParEst.IdKp * gBasePar.FcSetApply / 50;         // 参数辨识时减小Kp和Ki       
	        gItAcrQ24.KP = (long)gPmParEst.IqKp * gBasePar.FcSetApply / 50;
	        gImAcrQ24.KI = (s32)gPmParEst.IdKi ;
	        gItAcrQ24.KI = (s32)gPmParEst.IqKi ;
		}
	}
}

void PrepPmsmCsrPrar()
{     
    long    ImKp, ImKi, ItKp, ItKi;
    long    m_Long,m_Long1;
   // int     sGain;  // 积分调整倍数
    
// 同步机根据载波调整电流环参数
    if(gBasePar.FcSetApply > C_DOUBLE_ACR_MAX_FC)    // 一次发波需要减弱PI
	{
		ImKp = (long)gVCPar.AcrImKp * gBasePar.FcSetApply  / 100L;   		      	    
	    ItKp = (long)gVCPar.AcrItKp * gBasePar.FcSetApply  / 100L;
		ImKi = gVCPar.AcrImKi / 2;
	    ItKi = gVCPar.AcrItKi / 2;
	}
	else
	{
	    ImKp = (long)gVCPar.AcrImKp * gBasePar.FcSetApply  / 50;         	    
	    ItKp = (long)gVCPar.AcrItKp * gBasePar.FcSetApply  / 50;
	    ImKi = gVCPar.AcrImKi;
	    ItKi = gVCPar.AcrItKi;	    
	}
	
	gImAcrQ24.KP = Min(ImKp,16383);
    gItAcrQ24.KP = Min(ItKp,16383);
	gImAcrQ24.KI = ImKi;
    gItAcrQ24.KI = ItKi;
	gPmCsr2.Kp = gImAcrQ24.KP;
	m_Long = gImAcrQ24.KI>>5;
	if(m_Long < 1)
	{
		m_Long = 1;
	}
	gPmCsr2.KiM = m_Long;

	m_Long1 = gImAcrQ24.KI;
    m_Long = gOutVolt.LimitOutVoltPer - gOutVolt.MaxOutVoltPer;
	if(m_Long > 100)
	{
		if(m_Long > 1024)
		{
			m_Long1 = gImAcrQ24.KI>>1;
		}
		else
		{
			m_Long1 = (gImAcrQ24.KI * (2048 - m_Long))>>11;
		}
	}
	m_Long1 = (m_Long1 * 10)>>4;
    if(gBasePar.FcSetApply < 50)
    {
    	m_Long1 = (s32)gBasePar.FcSetApply * m_Long1/50;
    }
    //m_Long1 = (m_Long1 * m_Kp)>>7;
    if(m_Long1 < 1)
	{
    	m_Long1 = 1;
	}
	gPmCsr2.Ki = m_Long1;
}
// 同步机解耦计算，计算两相旋转电动势
void PmDecoupleDeal()
{
    long temp;
    
    gPmDecoup.Omeg = Filter2(gRotorSpeed.SpeedApply, gPmDecoup.Omeg);
    gPmDecoup.Isd  = Filter2((gIMTQ24.M>>12), gPmDecoup.Isd);
    gPmDecoup.Isq  = Filter2((gIMTQ24.T>>12), gPmDecoup.Isq);

    temp = (long)gPmDecoup.Isd * gMotorExtPer.LD >> 13;                        // Q12
    gPmDecoup.PhiSd = temp + gMotorExtPer.FluxRotor;                          // Q12
    gPmDecoup.RotVq = (long)gPmDecoup.Omeg * gPmDecoup.PhiSd >> 15;        // Q12

    gPmDecoup.PhiSq = (long)gPmDecoup.Isq * gMotorExtPer.LQ >> 13;            // Q12
    gPmDecoup.RotVd = - (long)gPmDecoup.Omeg * gPmDecoup.PhiSq >> 15;      // Q12


    gPmDecoup.IsdSet  = gIMTSetApply.M>>12;  //Filter2((gIMTQ24.M>>12), gPmDecoup.Isd);
    gPmDecoup.IsqSet  = gIMTSetApply.T>>12;  //Filter2((gIMTQ24.T>>12), gPmDecoup.Isq);

    temp = (long)gPmDecoup.IsdSet * gMotorExtPer.LD >> 13;                        // Q12
    gPmDecoup.PhiSdSet = temp + gMotorExtPer.FluxRotor;                          // Q12
    gPmDecoup.RotVqSet = (long)gPmDecoup.Omeg * gPmDecoup.PhiSdSet >> 15;        // Q12
    gPmDecoup.EMF      = (s32)gMotorExtPer.FluxRotor * gPmDecoup.Omeg >> 15; 

    gPmDecoup.PhiSqSet = (long)gPmDecoup.IsqSet * gMotorExtPer.LQ >> 13;            // Q12
    gPmDecoup.RotVdSet = - (long)gPmDecoup.Omeg * gPmDecoup.PhiSqSet >> 15;      // Q12
}
/*************************************************************
    对同步机弱磁中用的常数、变量进行初始化，在运行开始时执行一次
*************************************************************/
void ResetParForPmsmFwc(void)
{
    u32 m_Current;

    gPmFluxWeak.IqLpf    = 0;

    //调整法弱磁变量初始化
    gPmFluxWeak.AdId     = 0;			
	gPmFluxWeak.AdIdIntg = 0;
    gPmFluxWeak.IdMixAdjFlag = 0;
	gPmFluxWeak.AdFreq   = 0;
	gPmFluxWeak.AdFreqIntg = 0;
    
    m_Current = (((u32)gMotorExtPer.FluxRotor << 13) / gMotorExtPer.LD) - 1000;  // 留一定的余量		
    m_Current = ((m_Current > 4500UL)?4500UL : m_Current);	
	if( gMotorInfo.Current > gInvInfo.InvCurrent)                 
    {
        m_Current = (((u32)gInvInfo.InvCurrent) * m_Current) / gMotorInfo.Current;
    }
	gPmFluxWeak.IdMax = (s16)m_Current;

	gPmFluxWeak.FreqMax = (s16)(gMotorInfo.FreqPer / 2);

    gPmFluxWeak.SalientRate = ((u32)gMotorExtInfo.LQ * 10UL)/gMotorExtInfo.LD; //计算电机凸极率
}
/*************************************************************
    同步机自动调整方式弱磁控制
*************************************************************/
s32 PmsmFwcAdjMethod(void)
{
	s32 m_s32;
    s16 m_Deta;
    u16 m_DetaAbs,m_Ki;

	//gPmFluxWeak.VoltLpf = Filter16(gUAmpTheta.Amp,gPmFluxWeak.VoltLpf);  //输出电压滤ㄖ?Q12
    
    if(gFluxWeak.CoefFlux != 0)
    {
    	m_Deta = gOutVolt.LimitOutVoltPer - gPmFluxWeak.VoltLpf;
        m_Deta = __IQsat(m_Deta,200,-1000);
        m_DetaAbs = abs(m_Deta);
        m_Ki =  gFluxWeak.CoefFlux * m_DetaAbs;                       //提高调整快速性
        m_Ki = (m_Ki > 20000)?20000:m_Ki;
        m_s32 = gPmFluxWeak.AdIdIntg + (((s32)m_Ki * (s32)m_Deta));
        m_s32 = (m_s32 > 0)?0:m_s32;
        gPmFluxWeak.AdIdIntg = (m_s32 > -((long)gPmFluxWeak.IdMax)<<15)?m_s32:(-((long)gPmFluxWeak.IdMax)<<15);
        gPmFluxWeak.AdId = gPmFluxWeak.AdIdIntg>>15;
    }
    else
    {
        gPmFluxWeak.AdId = 0;
    }
    
    if(gPmFluxWeak.AdId > -20)
    {
    	gPmFluxWeak.IdForTorq = PmsmMaxTorqCtrl();
    }

	m_s32 = gPmFluxWeak.AdId + gPmFluxWeak.IdForTorq;
    m_s32 = Max(m_s32,-gPmFluxWeak.IdMax);
    return(m_s32<<12);
}
/************************************************************
    同步机自动调整运行德?
*************************************************************/
s16 PmsmFreqAdjMethod(void)
{
	s32 m_s32;
    s16 m_Deta;
    u16 m_DetaAbs,m_Ki;

	//gPmFluxWeak.VoltLpf = Filter16(gUAmpTheta.Amp,gPmFluxWeak.VoltLpf);  //输出电压滤波值 Q12
    
    if(gFluxWeak.CoefFlux != 0)
    {
    	m_Deta = gOutVolt.LimitOutVoltPer - gPmFluxWeak.VoltLpf;
        m_Deta = __IQsat(m_Deta,200,-1000);
        m_DetaAbs = abs(m_Deta);
        m_Ki = gFluxWeak.CoefFlux * m_DetaAbs;                       //提高调整快速性
        m_Ki = (m_Ki > 20000)?20000:m_Ki;
        m_s32 = gPmFluxWeak.AdFreqIntg + (((s32)m_Ki * (s32)m_Deta));
        m_s32 = (m_s32 > 0)?0:m_s32;
        gPmFluxWeak.AdFreqIntg = (m_s32 > -((long)gPmFluxWeak.FreqMax)<<15)?m_s32:(-((long)gPmFluxWeak.FreqMax)<<15);
        gPmFluxWeak.AdFreq = (s16)(gPmFluxWeak.AdFreqIntg>>15);
    }
    else
    {
        gPmFluxWeak.AdFreq = 0;
    }

    gPmFluxWeak.AdFreq = Max(gPmFluxWeak.AdFreq,-gPmFluxWeak.FreqMax);
    return(gPmFluxWeak.AdFreq);
}
/*************************************************************
    对于凸极电机，最大转矩电流比控制程序，凸极率大于1.5时才进
    行最笞氐缌鞅瓤刂?
*************************************************************/
s16 PmsmMaxTorqCtrl(void)
{
    s16 m_Id;
    s32 m_Current,m_s32;   
    
    m_s32 = gIMTSetApply.T >> 12;	                                            //T轴电流给定值，Q12
	gPmFluxWeak.IqLpf = Filter16(m_s32,gPmFluxWeak.IqLpf); 

    if((gPmFluxWeak.SalientRate > 15)&&(gPmFluxWeak.PmsmMaxTorqCtrlEnable == 1)&&(gCtrMotorType == SYNC_FVC))                 // 凸极率1.5倍进行最大转矩电流控制
    { 
        m_Current = ((s32)gMotorExtPer.FluxRotor << 12)/((s32)(gMotorExtPer.LQ - gMotorExtPer.LD));       //Q12                                 
        m_Current = (m_Current * gPmFluxWeak.SalientRateCoff)/100;
        m_s32 = m_Current * m_Current + (s32)gPmFluxWeak.IqLpf * gPmFluxWeak.IqLpf;
        m_Id  = (s16)(m_Current - (s32)qsqrt(m_s32));
    }
    else
    {
        m_Id = 0;
    }

    return(m_Id);  	
}

/*************************************************************
    同步机弱磁控制--计算方法给定励磁电流给定
*************************************************************/
/*s16 PmsmFwcCalMethod(void)
{    
	s32 m_s32,m_Ud,m_Uq;
    s32 m_Id,m_Id1;
    u16 m_Freq; 

    //m_s32 = gIMTSetApply.T>>12;	
	//gPmFluxWeak.IqLpf = Filter16(m_s32,gPmFluxWeak.IqLpf);

    //恒转矩区励磁电流 
    m_Id = PmsmMaxTorqCtrl();                                                   // 最大转矩电流比控制

    //恒功率区励磁电流 
    m_Freq = abs(gRotorSpeed.SpeedApply);
	m_Freq = Max(m_Freq,1000);
    m_s32 = ((s32)gMotorExtPer.LQ * gPmFluxWeak.IqLpf)>>13;
	m_Ud  = (m_s32*(s32)m_Freq) >> 15;
	m_s32 = (s32)gOutVolt.LimitOutVoltPer*(s32)gOutVolt.LimitOutVoltPer - m_Ud * m_Ud;
	
    m_s32 = Max(m_s32,1);                                      
	m_Uq  = (s32)qsqrt(m_s32);
	m_s32 = (m_Uq << 15)/(s32)m_Freq - (s32)gMotorExtPer.FluxRotor;
    m_s32 = (m_s32<<13)/gMotorExtPer.LD;
    //m_Id1 = (m_s32 * gPmFluxWeak.CalImAdjGain)/100;     
    m_Id1 = m_s32;             // 取消调整，wyk

    if(m_Id1 < m_Id)
    {
        gPmFluxWeak.IdMixAdjFlag = 1;
    }
    else
    {
        gPmFluxWeak.IdMixAdjFlag = 0;        
    }
    m_Id1  = Min(m_Id1,m_Id);
    m_Id1  = Min(m_Id1,0);
    m_Id1  = Max(m_Id1,-gPmFluxWeak.IdMax);
    return(m_Id1);    
}*/
/*************************************************************
    前馈+反馈的控制方式:当弱磁增益调为零时，为计算方式
*************************************************************/
/*s32 PmsmFwcMixMethod(void)
{
    s16 m_IdCal,m_Deta;
    s32 m_s32;

    m_IdCal = PmsmFwcCalMethod();
    	
	//gPmFluxWeak.VoltLpf = Filter16(gUAmpTheta.Amp,gPmFluxWeak.VoltLpf);  //输出电压滤波值 Q12
    if((gFluxWeak.CoefFlux != 0) && (gPmFluxWeak.IdMixAdjFlag == 1))
    {
    	m_Deta = gOutVolt.LimitOutVoltPer - gPmFluxWeak.VoltLpf;

        m_Deta = Max(m_Deta,-1000);
        m_Deta = Min(m_Deta,200);
        m_s32 = gPmFluxWeak.AdIdIntg + (((s32)gFluxWeak.CoefFlux * (s32)m_Deta)<<5);
        m_s32 = Min(m_s32,((s32)gPmFluxWeak.IdMax)<<13);
        gPmFluxWeak.AdIdIntg = Max(m_s32,-((s32)gPmFluxWeak.IdMax)<<13);
        gPmFluxWeak.AdId = gPmFluxWeak.AdIdIntg>>15;
    }
    else
    {
        gPmFluxWeak.AdId = 0;
    }
    m_s32 = m_IdCal + gPmFluxWeak.AdId;	
    m_s32 = Min(m_s32,0);
    m_s32 = Max(m_s32,(s32)(-gPmFluxWeak.IdMax));	 
    m_s32 = m_s32 << 12;
    return(m_s32);
}*/
/*************************************************************
	(上电缓冲完成后执行)停机情况下，同步机编码器位置角校正函数
	对绝对位置取16次平均值，再做判断，保证可靠。
*************************************************************/
s16 IPMPosAdjustStop(void)
{
    u16  m_AbsPosTemp,m_AbsPos;
    s16  m_Return = 0;
    static u16 m_FirstPos = 0;
    static u16 m_Cnt = 0;
    static s32 m_TotalPosErr = 0;

    if(m_Cnt > 100)
    {
        m_Cnt = 101;
        return 1;
    }// 确保这个处理上电后只执行一次，不再多次执行
    
    m_AbsPosTemp = IPMCalAbsPos();
    if(m_Cnt == 0)
    {
        m_FirstPos = m_AbsPosTemp;
        m_TotalPosErr = 0;
    }
    else
    {
        m_TotalPosErr += (m_AbsPosTemp - m_FirstPos);
    }
    if(m_Cnt == 100)
    {
        m_AbsPos = m_FirstPos + (u16)(m_TotalPosErr/100);
        if(abs(m_AbsPos - gIPMPos.RotorPos) > C_MAX_DETA_POS_ABS)
        {
            SetIPMPos(m_AbsPos);
        }// 通过UVW或者CD信号检测的位置角和当前位置角偏差过大，校正角度
        if(abs(m_AbsPos - (u16)(gPGData.RefPos>>8)) > C_MAX_DETA_POS_ABS)
        {
            SetIPMRefPos(m_AbsPos);
        }// 通过UVW或者CD信号检测的位置角和当前位置角偏差过大，校正角度  
        m_Return = 1;
    }
    m_Cnt++;
    return m_Return;
}
void PMFluxWeak05Ms(void)
{
	long data;

	gPmCsr2.ImMin = -4000;
	gPmCsr2.DeleteVMax = 1000;
	gPmCsr2.DeleteVm = 1L * 65536L;

	data = (s32)gMotorExtInfo.LQ * (s32)gMotorInfo.Current;
	/*if(gBasePar.SubCmd2.bit.Unite == 1)
	{
		data = data/10;
	}*/
	data = 340000000L/data;
	data = __IQsat(data,3000,600);
	gPmCsr2.DeltTMax = data;
	gPmCsr2.DeltTMin = -data;
	if(gPmCsr2.FreqSyn > 0)
	{
		gPmCsr2.PhaseMax = 30000;
		gPmCsr2.PhaseMin = 200;
	}
	else
	{
		gPmCsr2.PhaseMax = -200;
		gPmCsr2.PhaseMin = -30000;
	}

	FluxWeak05Ms();
}


void FluxWeak05Ms(void)
{
	long data,data1;
	long Umax;
	data = (gPmCsr2.ITSet - (gIMTSetApply.T>>12));
	data = labs(data);
	if(data > 1024)
	{
		data1 = data - 1024;
		data1 =	gOutVolt.LimitOutVoltPer - data;
		data1 = __IQsat(data1,gPmCsr2.VoltMax,gOutVolt.MaxOutVoltPer);
		gPmCsr2.VoltMax = data1;
	}
	gPmCsr2.ITSet = (gIMTSetApply.T>>12);
	gPmCsr2.VoltMax = gPmCsr2.VoltMax - (gPmCsr2.VoltMax>>5) + (gOutVolt.LimitOutVoltPer>>5);
	gPmCsr2.VoltMax = __IQsat(gPmCsr2.VoltMax,gOutVolt.LimitOutVoltPer,gOutVolt.MaxOutVoltPer);

	//gPmCsr2.VoltMax = gOutVolt.LimitOutVoltPer;
	gPmCsr2.MaxOutVoltPer = gOutVolt.MaxOutVoltPer;

	gPmCsr2.FreqSyn = gMainCmd.FreqSyn;
	data = abs(gPmCsr2.FreqSyn);
	data = 1024L * (u32)data/gMotorInfo.FreqPer;
	data = __IQsat(data,2048,1024);
	gPmCsr2.KFreq = data;

	Umax = gOutVolt.MaxOutVoltPer - 1000;
	if(gPmCsr2.Csr1Flag1 == 0)
	{
		gPmCsr2.VoltMax = gOutVolt.LimitOutVoltPer;
		if(gOutVolt.VoltApply > Umax)
		{
			data = gUDC.uDCFilter;
			data = (s32)gMotorInfo.FreqPer * data / ((s16)gMotorInfo.Votage);
			data = (data * 55L)>>10;//58=0.8*1024/1.414/10
			if((abs(gMainCmd.FreqSyn) > data)&&(gOutVolt.VoltFilter > Umax))
			{
				gPmCsr2.Csr1Flag1 = 1;
			}
		}
	}

	if(gPmCsr2.Csr1Flag1 == 1)
	{
        gOutVolt.VoltFilter = Filter4(gOutVolt.VoltApply, gOutVolt.VoltFilter);
		data = gUDC.uDCFilter;
		data1 = ((s32)50L * (u32)data)/gMotorInfo.Votage;
		data = (data1 * (s32)gMotorInfo.FreqPer)>>10;//56L=0.8*1024/1.414/10;
		if((abs(gMainCmd.FreqSyn) <= data)&&(gOutVolt.VoltFilter < (Umax - 200)))
		{
			gPmCsr2.Csr1Flag1 = 0;
		}
	}
}



void IMFluxWeak2(void)
{
	long data;

    gIMTSetApply.M = gIMTSet.M;
    gIMTSetApply.T = gIMTSet.T;

	gPmCsr2.DeltM = (gIMTSetApply.M - gIMTQ24.M)>>12;
	gPmCsr2.DeltT = (gIMTSetApply.T - gIMTQ24.T)>>12;
    gPmCsr2.DeltT = __IQsat(gPmCsr2.DeltT,4096,-4096);
        
	gPmCsr2.Im = gIMTQ24.M>>12;
	gPmCsr2.It = gIMTQ24.T>>12;
	gPmCsr2.UMComp = 0;
	data = (gPmCsr2.Kp * gPmCsr2.Im)>>13;
	data = (gPmCsr2.KFreq * data)>>10;
	if(gPmCsr2.FreqSyn > 0)
	{
		data = -data;
	}

	gPmCsr2.UTComp = data;
//***********************************************************************
	if((gPmCsr2.Csr1Flag == 0)&&(gPmCsr2.Csr1Flag1 == 1))
	{
		data = gImAcrQ24.Total - (gPmCsr2.UMComp<<12);
		gPmCsr2.UMTotal = data;
		data = gItAcrQ24.Total - (gPmCsr2.UTComp<<12);
		gPmCsr2.UTTotal = data;
		gPmCsr2.PhaseOut = gOutVolt.VoltPhaseApply;
	    gPmCsr2.OutFlag = 0;
		gPmCsr2.Csr1Flag = 1;
	}
	if(gPmCsr2.Csr1Flag == 1)
	{
		PMCsr2();
//*****************************************************
		gOutVoltVoltPhaseApplyOld = gOutVolt.VoltPhaseApply;
		gOutVolt.VoltPhaseApply = gPmCsr2.PhaseOut;
		gOutVolt.VoltApply = gPmCsr2.VoltOut;
		gOutVolt.VoltPhaseApply1 = gOutVoltVoltPhaseApplyOld + gPhase.IMPhaseOld;				// wg
		gOutVolt.VoltPhaseApply2 = gOutVolt.VoltPhaseApply + gPhase.IMPhase;		// wg
		gUMTSet.M = gPmCsr2.UMOut;
		gUMTSet.T = gPmCsr2.UTOut;
		gImAcrQ24.Total = (gPmCsr2.UMOut<<12) - ((s32)gPmCsr2.DeltM * (s32)gImAcrQ24.KP);
		gItAcrQ24.Total = (gPmCsr2.UTOut<<12) - ((s32)gPmCsr2.DeltT * (s32)gItAcrQ24.KP);
//*************************************************************************
		if(gPmCsr2.Csr1Flag1 == 0)
		{
			gPmCsr2.Csr1Flag = 0;
		}
	}
	gPmCsr2.FluxWeakFlag = gPmCsr2.Csr1Flag;
}


void CalUVWVoltSet(int Phase)
{
    s32  m_U,m_V,m_W;
    s32  m_Coff;
    s32  m_HalfTc;
	s32  m_Data;
	s32  m_Zero;

	s32  sin,cos;//m_Data;

    
	sin = qsin(Phase);                       
	cos = qsin(16384 - Phase);
    
    m_HalfTc = (s16)(gPWM.gPWMPrdApply>>1);
	m_Data = 16384L * 16384L/m_HalfTc;
    m_U = -gPWM.USet - m_HalfTc;
    m_V = -gPWM.VSet - m_HalfTc;
    m_W = -gPWM.WSet - m_HalfTc;   
   
	m_Zero = (m_U + m_V + m_W)/3;
	m_U = (m_U - m_Zero) * m_Data>>14;
    m_V = (m_V - m_Zero) * m_Data>>14;
    m_W = (m_W - m_Zero) * m_Data>>14;

    //m_Coff = (3344L * (s32)gUDC.uDC)/(gMotorInfo.Votage * 10);
	m_Coff = (3550L * (s32)gUDC.uDC)/(gMotorInfo.Votage * 10);

    gVoltUVW.U = ((s32)m_U * (s32)m_Coff)>>14; 
    gVoltUVW.V = ((s32)m_V * (s32)m_Coff)>>14; 
    gVoltUVW.W = ((s32)m_W * (s32)m_Coff)>>14; 

	gVoltUVW.Alph = ((s32)gVoltUVW.U * 23170L)>>15;	
	gVoltUVW.Beta = ((s32)(gVoltUVW.V - gVoltUVW.W) * 13377L)>>15;


    gVoltUVW.UdQ = (( (cos * (s32)(gVoltUVW.Alph)) + (sin * (s32)(gVoltUVW.Beta)))>>15);
	gVoltUVW.UqQ = ((-(sin * (s32)(gVoltUVW.Alph)) + (cos * (s32)(gVoltUVW.Beta)))>>15);

}
