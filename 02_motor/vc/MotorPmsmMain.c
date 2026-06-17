/****************************************************************
锟侥硷拷锟斤拷锟杰ｏ拷锟斤拷锟斤拷同锟斤拷锟斤拷锟秸伙拷矢锟斤拷锟斤拷锟狡诧拷锟街ｏ拷锟斤拷锟脚碉拷
锟侥硷拷锟芥本锟斤拷同锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
锟斤拷锟铰革拷锟铰ｏ拷

****************************************************************/
//锟侥硷拷锟斤拷锟斤拷: 
//    1. 同锟斤拷锟斤拷停锟斤拷状态位锟矫硷拷椋�
//    2. 同锟斤拷锟斤拷锟斤拷锟斤拷前锟侥筹拷始位锟斤拷确锟斤拷锟斤拷
//    3. 同锟斤拷锟斤拷ABZ锟斤拷锟斤拷锟斤拷锟脚硷拷位锟矫碉拷锟桔加硷拷锟�?
//    4. 同锟斤拷锟斤拷锟斤拷屑锟斤拷悖拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
//    5. 同锟斤拷锟斤拷锟斤拷锟斤拷锟脚达拷锟斤拷锟斤拷
//    6. ABZ/UVW锟斤拷锟斤拷锟斤拷z锟叫断的达拷锟斤 拷锟斤拷

#include "MotorVCInclude.h"
#include "MotorInclude.h"
#include "MotorIPMSVC.h"
//************************************************************
IPM_POSITION_STRUCT		gIPMPos;          //锟斤拷锟斤拷同锟斤拷锟斤拷锟斤拷锟阶拷咏嵌锟斤拷锟截的结构
PM_FLUX_WEAK            gFluxWeak;
PM_INIT_POSITION        gPMInitPos;
IPM_POS_CHECK_STRUCT	gIPMPosCheck;   //锟斤拷锟斤拷同锟斤拷锟斤拷锟斤拷系锟斤拷獾鼻帮拷锟斤拷锟轿伙拷媒堑锟斤拷锟斤拷萁峁�
PM_DECOUPLE             gPmDecoup;
PM_FW_IN	            gPMFwIn;
PM_FW		            gPMFwCtrl;
PM_FW_OUT	            gPMFwOut;
PMSM_FLUX_WEAK_STRUCT   gPmFluxWeak;
PM_CSR2_DATA            gPmCsr2;
s16 gOutVoltVoltPhaseApplyOld;

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
	同锟斤拷锟斤拷锟铰憋拷锟斤拷锟斤拷锟侥伙拷准锟叫断碉拷锟斤拷拇锟斤拷锟斤拷锟斤拷锟�(锟斤拷锟矫筹拷锟斤拷)
*************************************************************/
#ifdef TARGET_GS32
__interrupt void PG_Zero_isr(void)
#else
interrupt void PG_Zero_isr(void)
#endif
{
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
	if((*EQepRegs).QFLG.bit.IEL == 1)
    {
        EALLOW;
        (*EQepRegs).QCLR.bit.IEL = 1;
		(*EQepRegs).QCLR.bit.INT = 1;
#ifdef TARGET_GS32
    
#else
      	PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;	// Acknowledge this interrupt
#endif
		EDIS;
	    if((gIPMZero.zFilterCnt < 4)||(gIPMPos.ZIntFlag == 1))            // Z 锟脚号碉拷锟剿诧拷锟斤拷锟斤拷锟斤拷要锟斤拷锟斤拷锟斤拷Z锟斤拷锟斤拷锟斤拷锟�8ms(7500rpm),锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷卸锟斤拷锟斤拷诩锟斤拷锟轿伙拷茫锟斤拷锟斤拷锟斤拷锟絑锟叫讹拷
		{
			return;
		} 
	    gIPMZero.zFilterCnt = 0;
	       
	    gIPMPos.ZSigNumSet++;    
	    gIPMPos.ZBakRotorPos    = gIPMPos.RotorPos;
	    gIPMPos.QepCntBak       = GetQepCnt();
	    gIPMPos.QepCntPosCalBak = gIPMPos.QepCntPosCal;
#if(AIRCOMPRESSOR == 0)
	    gIPMPos.ZBakUVW         = Get_UVW_PG_U() + Get_UVW_PG_V() + Get_UVW_PG_W();                         /*QEP锟斤拷Z锟脚猴拷锟叫断ｏ拷锟斤拷锟斤拷锟教ｏ拷锟斤拷锟斤拷锟叫讹拷*/
#else
        gIPMPos.ZBakUVW         = 0;
#endif
    }
#ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}
/*************************************************************
锟斤拷锟斤拷Z锟脚号凤拷锟斤拷时锟斤拷锟斤拷锟斤拷锟揭伙拷锟斤拷卸虾锟斤拷锟街拷锟斤拷位锟斤拷偏锟斤拷
锟斤拷锟姐方式
锟斤拷Z锟脚猴拷锟叫讹拷锟叫ｏ拷锟斤拷录锟斤拷锟揭伙拷渭锟斤拷锟阶拷锟轿伙拷锟绞癸拷玫锟絈EPCNT值锟斤拷锟斤拷Z锟脚号凤拷锟斤拷时锟斤拷QEPCNT值锟斤拷锟斤拷锟斤拷锟斤拷锟街拷锟斤拷偏锟斤拷
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
	锟斤拷准位锟矫碉拷锟斤拷时锟斤拷同锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷位锟矫斤拷校锟斤拷锟斤拷锟斤拷(锟劫讹拷锟斤拷锟斤拷锟斤拷锟斤拷
	Z锟脚号斤拷锟斤拷锟斤拷DSP锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷U锟脚号碉拷锟斤拷锟斤拷锟斤拷锟斤拷为Z锟脚猴拷)
	(2ms锟斤拷锟斤拷锟斤拷执锟叫ｏ拷锟杰癸拷锟斤拷锟斤拷30000RPM锟斤拷锟斤拷锟劫革拷锟劫会丢失Z锟脚猴拷)
*************************************************************/
void IPMPosAdjustZIndex(void)
{
	s16 m_DetaPos;
	s32 m_DetaPosShow;
    static u16 m_ZErrCnt = 0;	

    if(gIPMPos.ZSigNum == gIPMPos.ZSigNumSet)
    {        
        return;                                                 // 没锟斤拷Z锟脚号碉拷锟斤，直锟接凤拷锟斤拷
    }

	if(gPGData.PGType == PG_TYPE_UVW)
	{
	    if((gIPMPos.ZBakUVW & 0x03) != 1)
	    {
	        return;                                             // UVW锟斤拷锟斤拷锟斤拷要锟斤拷Z锟脚号碉拷锟斤拷锟绞憋拷锟絍锟脚猴拷锟斤拷0锟斤拷W锟脚猴拷锟斤拷1锟斤拷锟斤拷锟斤拷锟斤拷为锟角达拷锟斤拷锟脚猴拷.
	    }   
	}
    gIPMPos.ZSigNum = gIPMPos.ZSigNumSet;
	
    // 锟斤拷始锟叫讹拷Z锟脚号碉拷锟斤拷锟绞憋拷颍锟铰硷拷锟阶拷锟轿伙拷煤锟较ｏ拷锟斤拷锟轿伙拷锟阶拷锟轿伙拷锟斤拷欠锟斤拷呛锟�(偏锟筋不锟斤拷)

    IPMPosCalZWindage();
    
    m_DetaPos = (s16)(gIPMPos.RotorZero - (gIPMPos.ZBakRotorPos + gIPMPos.ZPosWindage));
    gIPMPos.ZBakDetaPos = m_DetaPos;                            // Z锟脚号碉拷锟斤拷时锟斤拷希锟斤拷锟斤拷位锟侥角度和碉拷前锟角度的诧拷值

    m_DetaPosShow = ((s16)gIPMPos.RotorPos - (s16)(gPGData.RefPos >> 8));
	gIPMPos.DetaPosShow = (s16)(m_DetaPosShow * 1800L >>15);    // 锟斤拷ABZ锟斤拷锟斤拷锟斤拷时锟斤拷锟斤拷锟斤拷锟斤拷位锟矫斤拷锟斤拷锟斤拷示位锟斤拷偏锟筋，wyk

    if((gIPMPos.ZResetFlag == C_Z_DONT_RESET_POS)||(gIPMPos.ZSigEnable == 0))   //锟斤拷使锟斤拷Z锟脚猴拷校锟斤拷
    {
        return;                                                 // Z锟脚号碉拷锟斤拷时锟津，诧拷锟斤拷要锟斤拷位锟角度★拷
    }
  
    if((abs(m_DetaPos) > C_MAX_DETA_POS_Z) && (gIPMPos.ZResetFlag == C_Z_RESET_POS_LIMIT)&&(gMainStatus.RunStep != STATUS_GET_PAR))
    {
        m_ZErrCnt++;
        if(m_ZErrCnt > 5)
        {
   			m_ZErrCnt = 0;
   			gError.ErrorCode.all |= ERROR_ENCODER;
            gError.ErrorInfo[4].bit.Fault3 = 6;                      // Z锟脚号碉拷锟斤拷时锟教筹拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷为锟角讹拷锟窖撅拷偏锟诫，锟斤拷锟斤拷锟较★拷
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
    锟斤拷录锟较次碉拷锟斤拷时锟斤拷为锟斤拷始位锟斤拷
************************************************************************/
void IPMCheckInitPos(void)
{    
	//锟斤拷锟斤拷锟较次碉拷锟斤拷嵌龋锟斤拷锟斤拷锟绞讹拷鸨４锟角讹拷锟角凤拷锟斤拷锟�
    if(gMainStatus.ParaCalTimes == 0)  //锟较碉拷只锟斤拷锟斤拷一锟轿的诧拷锟斤拷转锟斤拷
	{
        gMainStatus.ParaCalTimes = 1;        
        
	    gIPMPos.PowerOffPosDeg = ((Ulong)gIPMPos.PowerOffPos << 16)/3600;	//识锟斤拷锟绞硷拷嵌锟�,锟斤拷取锟较达拷锟铰碉拷嵌锟�
	    SetIPMPos((Uint)gIPMPos.PowerOffPosDeg);
		SetIPMRefPos((Uint)gIPMPos.PowerOffPosDeg);
//		if(IDC_SVC_CTL == gMainCmd.Command.bit.ControlMode)       //svc锟较碉拷锟斤拷录锟斤拷一锟轿磁硷拷位锟斤拷
	    {
	        SvcSetRotorPos(gIPMPos.PowerOffPosDeg);
	    }
    }        
}
/************************************************************
	锟斤拷锟斤拷同锟斤拷锟斤拷锟脚硷拷锟斤拷始位锟矫角硷拷锟阶讹拷
************************************************************/
void RunCaseIpmInitPos(void)
{
	if((gError.ErrorCode.all != 0) || 
	   (gMainCmd.Command.bit.Start == 0))
	{
		DisableDrive();
        //if(gIPMPos.InitPosMethod == INIT_POS_VOLT_PULSE)
        //{
	    SynInitPosDetSetPwm(6);		    //同锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷识锟街革拷锟侥达拷锟斤拷锟斤拷锟斤拷
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
       			gIPMInitPos.Step = 1;                       // 锟矫撅拷态锟斤拷识锟斤拷志
       			gMainStatus.SubStep ++;
			}
			else
			{
			    gIPMInitPos.Step = 0;
			}
           
            break;

        case 2:
            if(gIPMInitPos.Step == 0)           // 锟叫断憋拷识锟斤拷锟�      
        	{ 
				gIPMPos.CompPos = ((Ulong)gIPMPos.CompPosFun << 16) / 3600;
				gIPMPos.InitPos = gIPMPos.InitPos + gIPMPos.CompPos;
    			SetIPMPos((Uint)gIPMPos.InitPos);
				SetIPMRefPos((Uint)gIPMPos.InitPos);
                SvcSetRotorPos(gIPMPos.InitPos);   

        		if(abs((s16)(gIPMPos.PowerOffPosDeg - gIPMPos.RotorPos)) < 3641)
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
        		gMainStatus.SubStep ++;             //锟斤拷锟皆讹拷锟揭伙拷模锟斤拷锟絇WM锟街革拷锟斤拷锟�
        	}   //else waiting interrupt deal
            break;
            
        case 3:
            	InitSetPWM();
   	            InitSetAdc();
                SetInterruptEnable();	            // 锟斤拷锟斤拷锟绞讹拷锟侥匡拷锟酵撅拷顺锟斤拷锟斤拷卸锟斤拷锌锟斤拷锟斤拷枪乇盏模锟斤拷锟斤拷诖舜锟�
                gMainStatus.SubStep ++; 
                break;
                
        case 4:
		    //ResetADCEndIsr(); 
            PrepareParForRun();
            gMainStatus.RunStep = STATUS_STOP;
            gMainStatus.SubStep = 0;
            gMainStatus.PrgStatus.all = 0;		//锟斤拷锟叫匡拷锟斤拷锟斤拷效

            break;
            
        default:
            gError.ErrorCode.all |= ERROR_TUNE_FAIL;
            gError.ErrorInfo[4].bit.Fault1 = 4;
            
            break;   
    }
}
#if 0
/************************************************************
	LD锟斤拷LQ锟斤拷锟叫硷拷锟姐函锟斤拷
	LAB = A-B*Cos(2*Theta+4*pi/3)	:= gIPMInitPos.LPhase[0]
	LBC = A-B*Cos(2*Theta)		     := gIPMInitPos.LPhase[1]
	LCA = A-B*Cos(2*Theta-4*pi/3)	:= gIPMInitPos.LPhase[2]
************************************************************/
void SynCalLdAndLq(Uint m_Pos)
{
	s16 m_Cos1,m_Cos2,m_Cos3;
	s32 m_CoffA,m_CoffB;
	Uint m_Angle;
	s32 m_Deta1,m_Deta2,m_DetaCos;
//	u16 m_L1,m_L2;
	u32 m_L1,m_L2;

	m_Angle = ((m_Pos + 32768)<<1);
	m_Cos1 = qsin(16384 - (s16)m_Angle);        // cos(2*theta)
	m_Angle += 43691;
	m_Cos2 = qsin(16384 - (s16)m_Angle);        // cos(2*theta + 4*pi/3)
	m_Angle = (m_Pos<<1) - 43691;
    m_Cos3 = qsin(16384 - (s16)m_Angle);        // cos(2*theta - 4*pi/3)

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
	LD锟斤拷LQ锟斤拷锟叫硷拷锟姐函锟斤拷
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

//	gIPMInitPos.InitPosSrc = user_atan(m_L2,m_L1) / 2;				// 锟斤拷锟斤拷锟斤拷锟斤拷偶锟斤拷嵌龋锟斤拷锟轿癸拷锟斤拷锟斤拷直锟斤拷锟叫断硷拷锟皆ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷录锟斤拷锟�

    gMotorExtReg.LD = gIPMInitPos.Ld;
    gMotorExtReg.LQ = gIPMInitPos.Lq;
}
#endif
/************************************************************
	矢锟斤拷锟斤拷锟狡的碉拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷转锟斤拷锟斤拷预锟斤拷转锟斤拷锟矫诧拷锟斤拷锟斤拷
	时锟斤拷幕锟街滴�:10/(2*pi*gBasePar.FullFreq), gBasePar.FullFreq01为0.01Hz锟斤拷位锟斤拷时锟斤拷锟街碉拷锟轿�:锟斤拷
    锟饺帮拷锟斤拷5KHz锟截诧拷频锟绞硷拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷秃锟绞憋拷锟轿猅Delay = 0.75Tc = 150us
    150us锟斤拷应锟斤拷锟斤拷值为: (150 * 2 * pi * gBasePar.FullFreq)/(1000000*100) = gBasePar.FullFreq/106100
    (频锟绞的伙拷值为gBasePar.FullFreq锟斤拷锟斤拷锟斤拷锟斤拷小锟斤拷锟斤拷)
    Kp = Ls / 2 * TDelay
       = 53050 * Ls/gBasePar.FullFreq        Q12锟斤拷式
       
    ki = (Rs/2*TDelay)*Tc/2(锟斤拷散锟斤拷锟斤拷每锟截诧拷锟斤拷锟节碉拷锟斤拷2锟斤拷)
       = Rs/3; 
************************************************************/
void IPMCalAcrPIDCoff(void)
{
    u32 m_Long; 
    Uint m_UData,m_BaseL;
	Ulong m_Ulong;
    
	//锟斤拷谢锟街滴拷锟斤拷杩癸拷锟�?2*pi*锟斤拷锟狡碉拷锟�
	m_BaseL = ((Ulong)gMotorInfo.Votage * 3678)/gMotorInfo.Current;
	m_BaseL = ((Ulong)m_BaseL * 5000)/gBasePar.FullFreq01; 

	m_UData = ((Ulong)gMotorExtReg.RsPm * (Ulong)gMotorInfo.Current)/gMotorInfo.Votage;	
    gMotorExtPer.Rpm = ((Ulong)m_UData * 18597)>>14;
	m_Ulong = (((Ulong)gMotorExtReg.LD <<15) + m_BaseL) >>1;
    gMotorExtPer.LD = m_Ulong / m_BaseL / 10;                   // 同锟斤拷锟斤拷d锟斤拷锟斤拷Q13
    m_Ulong = (((Ulong)gMotorExtReg.LQ <<15) + m_BaseL) >>1;
    gMotorExtPer.LQ = m_Ulong / m_BaseL / 10;   

	m_Long = (53050UL * (u32)(gMotorExtPer.LD>>1))/gBasePar.FullFreq01;/*>>锟斤拷示Q锟斤拷式锟斤拷Q13转锟斤拷为Q12锟斤拷式*/
    gPmParEst.IdKp  = (u16)Min(m_Long,8000);  // 锟斤拷锟街碉拷锟斤拷锟轿�8000,wyk
    gPmParEst.IqKp  =  gPmParEst.IdKp;      // IqKp使锟斤拷D锟斤拷锟斤拷
    //m_Long = (53050UL * (u32)(gMotorExtPer.LQ>>1))/gBasePar.FullFreq01;
    //gPmParEst.IqKp  = (u16)Min(m_Long,16383);  // 32768   

    gPmParEst.IdKi  = (gMotorExtPer.Rpm/3);                            /*Q16锟斤拷式*/
    gPmParEst.IqKi  = gPmParEst.IdKi; 

	if(gUVCoff.RsTune == 2)
	{
		if(gBasePar.FcSetApply > C_DOUBLE_ACR_MAX_FC)
		{
	        gImAcrQ24.KP = (long)gPmParEst.IdKp * gBasePar.FcSetApply / 100;         // 锟斤拷识时锟斤拷小Kp,一锟轿凤拷锟斤拷锟斤拷小Kp锟斤拷Ki       
	        gItAcrQ24.KP = (long)gPmParEst.IqKp * gBasePar.FcSetApply / 100;
	        gImAcrQ24.KI = (s32)gPmParEst.IdKi ;
	        gItAcrQ24.KI = (s32)gPmParEst.IqKi ;
		}
		else
		{
			gImAcrQ24.KP = (long)gPmParEst.IdKp * gBasePar.FcSetApply / 50;         // 锟斤拷锟斤拷锟斤拷识时锟斤拷小Kp锟斤拷Ki       
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
   // s16     sGain;  // 锟斤拷锟街碉拷锟斤拷锟斤拷锟斤拷
    
// 同锟斤拷锟斤拷锟斤拷锟斤拷锟截诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
    if(gBasePar.FcSetApply > C_DOUBLE_ACR_MAX_FC)    // 一锟轿凤拷锟斤拷锟斤拷要锟斤拷锟斤拷PI
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
// 同锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷悖拷锟斤拷锟斤拷锟斤拷锟斤拷锟阶拷缍拷锟�
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
    锟斤拷同锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟矫的筹拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫筹拷始锟斤拷锟斤拷锟斤拷锟斤拷锟叫匡拷始时执锟斤拷一锟斤拷
*************************************************************/
void ResetParForPmsmFwc(void)
{
    u32 m_Current;

    gPmFluxWeak.IqLpf    = 0;

    //锟斤拷锟斤拷锟斤拷锟斤拷锟脚憋拷锟斤拷锟斤拷始锟斤拷
    gPmFluxWeak.AdId     = 0;			
	gPmFluxWeak.AdIdIntg = 0;
    gPmFluxWeak.IdMixAdjFlag = 0;
	gPmFluxWeak.AdFreq   = 0;
	gPmFluxWeak.AdFreqIntg = 0;
    
    m_Current = (((u32)gMotorExtPer.FluxRotor << 13) / gMotorExtPer.LD) - 1000;  // 锟斤拷一锟斤拷锟斤拷锟斤拷锟斤拷		
    m_Current = ((m_Current > 4500UL)?4500UL : m_Current);	
	if( gMotorInfo.Current > gInvInfo.InvCurrent)                 
    {
        m_Current = (((u32)gInvInfo.InvCurrent) * m_Current) / gMotorInfo.Current;
    }
	gPmFluxWeak.IdMax = (s16)m_Current;

	gPmFluxWeak.FreqMax = (s16)(gMotorInfo.FreqPer / 2);

    gPmFluxWeak.SalientRate = ((u32)gMotorExtInfo.LQ * 10UL)/gMotorExtInfo.LD; //锟斤拷锟斤拷锟斤拷凸锟斤拷锟斤拷
}
/*************************************************************
    同锟斤拷锟斤拷锟皆讹拷锟斤拷锟斤拷锟斤拷式锟斤拷锟脚匡拷锟斤拷
*************************************************************/
s32 PmsmFwcAdjMethod(void)
{
	s32 m_s32;
    s16 m_Deta;
    u16 m_DetaAbs,m_Ki;

	//gPmFluxWeak.VoltLpf = Filter16(gUAmpTheta.Amp,gPmFluxWeak.VoltLpf);  //锟斤拷锟斤拷锟窖癸拷栓锟�?Q12
    
    if(gFluxWeak.CoefFlux != 0)
    {
    	m_Deta = gOutVolt.LimitOutVoltPer - gPmFluxWeak.VoltLpf;
        m_Deta = __IQsat(m_Deta,200,-1000);
        m_DetaAbs = abs(m_Deta);
        m_Ki =  gFluxWeak.CoefFlux * m_DetaAbs;                       //锟斤拷叩锟斤拷锟斤拷锟斤拷锟斤拷锟�
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
    同锟斤拷锟斤拷锟皆讹拷锟斤拷锟斤拷锟斤拷锟叫碉拷?
*************************************************************/
s16 PmsmFreqAdjMethod(void)
{
	s32 m_s32;
    s16 m_Deta;
    u16 m_DetaAbs,m_Ki;

	//gPmFluxWeak.VoltLpf = Filter16(gUAmpTheta.Amp,gPmFluxWeak.VoltLpf);  //锟斤拷锟斤拷锟窖癸拷瞬锟街� Q12
    
    if(gFluxWeak.CoefFlux != 0)
    {
    	m_Deta = gOutVolt.LimitOutVoltPer - gPmFluxWeak.VoltLpf;
        m_Deta = __IQsat(m_Deta,200,-1000);
        m_DetaAbs = abs(m_Deta);
        m_Ki = gFluxWeak.CoefFlux * m_DetaAbs;                       //锟斤拷叩锟斤拷锟斤拷锟斤拷锟斤拷锟�
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
    锟斤拷锟斤拷凸锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷转锟截碉拷锟斤拷锟饺匡拷锟狡筹拷锟斤拷凸锟斤拷锟绞达拷锟斤拷1.5时锟脚斤拷
    锟斤拷锟斤拷锟阶拷氐锟斤拷锟斤拷瓤锟斤拷?
*************************************************************/
s16 PmsmMaxTorqCtrl(void)
{
    s16 m_Id;
    s32 m_Current,m_s32;   
    
    m_s32 = gIMTSetApply.T >> 12;	                                            //T锟斤拷锟斤拷锟斤拷锟斤拷锟街碉拷锟絈12
	gPmFluxWeak.IqLpf = Filter16(m_s32,gPmFluxWeak.IqLpf); 

    if((gPmFluxWeak.SalientRate > 15)&&(gPmFluxWeak.PmsmMaxTorqCtrlEnable == 1)&&(gCtrMotorType == SYNC_FVC))                 // 凸锟斤拷锟斤拷1.5锟斤拷锟斤拷锟斤拷锟斤拷锟阶拷氐锟斤拷锟斤拷锟斤拷锟�
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
    同锟斤拷锟斤拷锟斤拷锟脚匡拷锟斤拷--锟斤拷锟姐方锟斤拷锟斤拷锟斤拷锟斤拷锟脚碉拷锟斤拷锟斤拷锟斤拷
*************************************************************/
/*s16 PmsmFwcCalMethod(void)
{    
	s32 m_s32,m_Ud,m_Uq;
    s32 m_Id,m_Id1;
    u16 m_Freq; 

    //m_s32 = gIMTSetApply.T>>12;	
	//gPmFluxWeak.IqLpf = Filter16(m_s32,gPmFluxWeak.IqLpf);

    //锟斤拷转锟斤拷锟斤拷锟斤拷锟脚碉拷锟斤拷 
    m_Id = PmsmMaxTorqCtrl();                                                   // 锟斤拷锟阶拷氐锟斤拷锟斤拷瓤锟斤拷锟�

    //锟姐功锟斤拷锟斤拷锟斤拷锟脚碉拷锟斤拷 
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
    m_Id1 = m_s32;             // 取锟斤拷锟斤拷锟斤拷锟斤拷wyk

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
    前锟斤拷+锟斤拷锟斤拷锟侥匡拷锟狡凤拷式:锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟轿拷锟绞憋拷锟轿拷锟斤拷惴绞�
*************************************************************/
/*s32 PmsmFwcMixMethod(void)
{
    s16 m_IdCal,m_Deta;
    s32 m_s32;

    m_IdCal = PmsmFwcCalMethod();
    	
	//gPmFluxWeak.VoltLpf = Filter16(gUAmpTheta.Amp,gPmFluxWeak.VoltLpf);  //锟斤拷锟斤拷锟窖癸拷瞬锟街� Q12
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
	(锟较电缓锟斤拷锟斤拷珊锟街达拷锟�)停锟斤拷锟斤拷锟斤拷拢锟酵拷锟斤拷锟斤拷锟斤拷锟斤拷锟轿伙拷媒锟叫ｏ拷锟斤拷锟斤拷锟�
	锟皆撅拷锟斤拷位锟斤拷取16锟斤拷平锟斤拷值锟斤拷锟斤拷锟斤拷锟叫断ｏ拷锟斤拷证锟缴匡拷锟斤拷
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
    }// 确锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷系锟斤拷只执锟斤拷一锟轿ｏ拷锟斤拷锟劫讹拷锟街达拷锟�
    
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
        }// 通锟斤拷UVW锟斤拷锟斤拷CD锟脚号硷拷锟斤拷位锟矫角和碉拷前位锟矫斤拷偏锟斤拷锟斤拷锟叫ｏ拷锟斤拷嵌锟�
        if(abs(m_AbsPos - (u16)(gPGData.RefPos>>8)) > C_MAX_DETA_POS_ABS)
        {
            SetIPMRefPos(m_AbsPos);
        }// 通锟斤拷UVW锟斤拷锟斤拷CD锟脚号硷拷锟斤拷位锟矫角和碉拷前位锟矫斤拷偏锟斤拷锟斤拷锟叫ｏ拷锟斤拷嵌锟�  
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


void CalUVWVoltSet(s16 Phase)
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
