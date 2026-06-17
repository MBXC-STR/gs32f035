/****************************************************************
锟侥硷拷锟斤拷锟杰ｏ拷矢锟斤拷锟斤拷锟狡碉拷锟斤拷要锟斤拷锟津部凤拷
锟侥硷拷锟芥本锟斤拷 
锟斤拷锟铰革拷锟铰ｏ拷 
	
****************************************************************/
#include "MotorVCInclude.h"
#include "MotorEncoder.h"
#include "MotorPmsmParEst.h"
#include "MotorPmsmMain.h"

// // 全锟街憋拷锟斤拷锟斤拷锟斤拷
VC_INFO_STRUCT			gVCPar;			//VC锟斤拷锟斤拷
MT_STRUCT_Q24           gIMTSet;        //MT锟斤拷系锟铰碉拷锟借定锟斤拷锟斤拷锟斤拷Q24锟斤拷示
MT_STRUCT_Q24           gIMTSetApply;	//MT锟斤拷系锟铰的碉拷锟斤拷指锟斤拷值
MT_STRUCT				gUMTSet;		//MT锟斤拷系锟铰碉拷锟借定锟斤拷压
AMPTHETA_STRUCT			gUAmpTheta;		//锟斤拷锟斤拷锟斤拷锟斤拷锟借定锟斤拷压

PID32_STRUCT        gImAcrQ24;
PID32_STRUCT        gItAcrQ24;
PID32_STRUCT        gIMAcr;
PID32_STRUCT        gITAcr;

ASR_STRUCT				gAsr;			//锟劫度伙拷
VC_CSR_PARA             gVcCsrPara;  //锟秸伙拷矢锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟矫碉拷锟侥诧拷锟斤拷
PID_STRUCT              gIMSetAcr;   //锟斤拷锟斤拷锟斤拷锟脚碉拷锟斤拷锟斤拷锟斤拷值
PID_STRUCT              gWspid;      //锟届步锟斤拷锟秸伙拷矢锟斤拷锟劫度伙拷转锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
MODIFYWS_STRUCT         gModifyws;   //锟届步锟斤拷锟秸伙拷矢锟斤拷转锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
MT_STRUCT               gPWMVAlphBeta;//锟斤拷锟斤拷锟斤拷锟絇WM锟斤拷锟斤拷锟斤拷alfa锟斤拷beta锟斤拷锟窖�
UDC_LIMIT_IT_STRUCT     gUdcLimitIt;
extern PMSM_FLUX_WEAK_STRUCT   gPmFluxWeak;
s16 gTorLimit_SVC = 0;
// // 锟侥硷拷锟节诧拷锟斤拷锟斤拷锟斤拷锟斤拷 
void PrepareAsrPar(void);
void PrepareAsrPar1(void);
void ResetAsrPar(void);
void ResetCsrPar(void);
void VcAsrControl(void);
void VcAsrControl1(void);
void CalTorqueLimitPar(void);
s16 CalUdcLimitIT(s16 Torque);
extern void IMFluxWeak2(void);
extern void ResetParForPmsmFwc(void);
extern void PID32(PID32_STRUCT * pid);
/*************************************************************
	矢锟斤拷锟斤拷锟狡碉拷锟斤拷锟捷革拷位锟斤拷锟斤拷锟斤拷停锟斤拷锟阶讹拷执锟斤拷
*************************************************************/
void ResetParForVC(void)
{
	ResetAsrPar();
	ResetCsrPar();
	ResetParForPmsmFwc();
}

/*************************************************************
	矢锟斤拷锟斤拷锟狡碉拷锟劫度伙拷锟斤拷锟斤拷锟斤拷位
*************************************************************/
void ResetAsrPar(void)
{
	gAsr.Asr.Total = 0;
    gAsr.TorqueLimitPid.Total = 0;   
	gTorLimit_SVC = 0; 
    gWspid.Total = 0;
	gIMTSet.T = 0;
	gIMTSet.M = 0;
	gAsr.FreqSet = ((s32)gMainCmd.FreqSet<<9);
	gAsr.PosSet = gPGData.RefPos;
}

/*************************************************************
	矢锟斤拷锟斤拷锟狡的碉拷锟斤拷锟斤拷锟斤拷位
*************************************************************/
void ResetCsrPar(void)
{
	gIMTSetApply.M = 0;
	gIMTSetApply.T = 0;
	
	gUMTSet.M = 0;
	gUMTSet.T = 0;

    gIMSetAcr.Total = 0;
	gPmFluxWeak.AdIdIntg = 0;

    gImAcrQ24.Total = (long)gPmDecoup.RotVd << 12;
    gItAcrQ24.Total = (long)gPmDecoup.RotVq << 12;	    		

	gUAmpTheta.Amp   = 0;
	gUAmpTheta.Theta = 0;
    gOutVolt.VoltApply = 0;
	gOutVolt.VoltPhaseApply = 0;
    //锟斤拷锟较猴拷锟斤拷锟斤拷
    gMainCmd.FreqSyn = gRotorSpeed.SpeedApply;

    gPmCsr2.Csr1Flag = 0;
	gPmCsr2.Csr1Flag1 = 0;
    gPmCsr2.FluxWeakFlag = 0;
}
/*************************************************************
	矢锟斤拷锟斤拷锟狡碉拷转锟截硷拷锟姐（要锟斤拷转锟截碉拷锟斤拷锟斤拷锟斤拷锟脚碉拷锟斤拷矢锟斤拷锟酵诧拷锟斤拷锟斤拷2锟斤拷频锟斤拷锟斤拷锟斤拷2锟斤拷锟斤拷
同锟斤拷锟斤拷锟斤拷锟斤拷觳斤拷锟斤拷锟斤拷锟斤拷转锟截匡拷锟狡筹拷锟斤拷
*************************************************************/
void CalTorqueLimitPar(void)
{
	s16 m_TorLimit,m_MaxLimit,m_NegTorqueLimit;
	s16 m_InvIM;
	Ulong m_Long;

	//m_InvIM = ((long)(gIMTSet.M>>12) * (long)gMotorInfo.Current)/gInvInfo.InvCurrForP; //使锟斤拷锟矫伙拷锟借定锟斤拷锟酵讹拷应锟侥碉拷锟斤拷
	m_InvIM = ((long)(gIMTSet.M>>12) * (long)gMotorInfo.Current)/gInvInfo.InvCurrent; //使锟斤拷锟矫伙拷锟借定锟斤拷锟酵讹拷应锟侥碉拷锟斤拷
	m_InvIM = ((long)m_InvIM * 1000)>>12;
//	m_Long  = (1800L * 1800L) - ((long)m_InvIM * (long)m_InvIM);    //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷矢锟斤拷锟斤拷小锟节憋拷频锟斤拷锟筋定锟斤拷锟斤拷2锟斤拷
    if(15 == gInvInfo.InvTypeApply)
	{
        gLineCur.MaxCurLimit = Min(gLineCur.MaxCurLimit,1700);
	}
	else
	{
	    gLineCur.MaxCurLimit = Min(gLineCur.MaxCurLimit,1800);
	}

    m_Long  = ((long)gLineCur.MaxCurLimit * (long)gLineCur.MaxCurLimit) - ((long)m_InvIM * (long)m_InvIM);

    m_MaxLimit = qsqrt(m_Long);					
	
    m_TorLimit = (gVCPar.PosVCTorqueLim > m_MaxLimit)?m_MaxLimit : gVCPar.PosVCTorqueLim;   											  
	m_TorLimit = ((long)m_TorLimit * (long)gInvInfo.InvCurrForP)/gMotorInfo.Current;
	m_TorLimit = ((long)m_TorLimit<<12)/1000;	//转锟斤拷为锟斤拷锟斤拷值
	//gAsr.TorqueLimit = m_TorLimit;          // PM 锟斤拷 IM锟斤拷锟斤拷转锟截匡拷锟斤拷T锟斤拷锟斤拷锟斤拷瓒�
    gAsr.PosTorqueLimit = m_TorLimit; 

	if(((SYNC_SVC == gCtrMotorType)&&(gMainCmd.FreqReal < 100))||(gFluxWeak.CoefKI == 0))     // 1Hz锟斤拷锟铰诧拷锟斤拷锟街电动锟酵凤拷锟斤拷转锟斤拷
	{
	    m_TorLimit = (gVCPar.PosVCTorqueLim > m_MaxLimit)?m_MaxLimit : gVCPar.PosVCTorqueLim;  
	}
	else
	{
        m_TorLimit = (gVCPar.NegVCTorqueLim > m_MaxLimit)?m_MaxLimit : gVCPar.NegVCTorqueLim;   
    }											  
	m_TorLimit = ((long)m_TorLimit * (long)gInvInfo.InvCurrForP)/gMotorInfo.Current;
	m_TorLimit = ((long)m_TorLimit<<12)/1000;	//转锟斤拷为锟斤拷锟斤拷值
	gAsr.TorqueLimit = m_TorLimit;           // PM 锟斤拷 IM锟斤拷锟斤拷转锟截匡拷锟斤拷T锟斤拷锟斤拷锟斤拷瓒�
	m_NegTorqueLimit = CalUdcLimitIT(gAsr.TorqueLimit);
    gAsr.NegTorqueLimit = (s16)(((s32)m_TorLimit * (s32)m_NegTorqueLimit) >> 12);
}
/*************************************************************
	矢锟斤拷锟斤拷锟狡碉拷锟劫度伙拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷转锟斤拷锟斤拷预锟斤拷转锟斤拷锟矫诧拷锟斤拷锟斤拷
准锟斤拷锟斤拷锟斤拷:(锟斤拷锟斤拷选锟斤拷锟斤拷锟�)
 Kp = (Kp_func << 8)* (f_base/10 >>12) << 3 := Kp_func * 100 |(f_base == 80.00Hz)
 Ki = (Kp_func/Ki_func << 10) * (f_base/10 >>12) << 3 := Kp_func/Ki_func * 1600 |(f_base == 80.00Hz)
*************************************************************/
void PrepareAsrPar(void)
{
    s16	  m_AbsFreq, m_FreqUp;
    s16   m_DetaKP, m_DetaKI, m_DetaFreq;
    //s16   m_KpLimit;
    long  tempKp;
    long  tempKi;

    // 准锟斤拷锟叫伙拷锟斤拷锟斤拷  
    gAsr.KPHigh = gVCPar.ASRKpHigh<<8; 
    gAsr.KPLow  = gVCPar.ASRKpLow<<8;  
	gAsr.KPLow  = (s16)((s32)gAsr.KPLow * gAsr.KPLowCoff / 10L);    // 锟斤拷同锟斤拷锟斤拷锟斤拷Kp值锟斤拷一锟斤拷
	if((gVCPar.ASRKpHigh>>5) >= gVCPar.ASRTIHigh)
	{
		gAsr.KIHigh = 32767;
	}
	else
	{
       gAsr.KIHigh = ((Ulong)gVCPar.ASRKpHigh<<10)/gVCPar.ASRTIHigh;  
	}

	if((gVCPar.ASRKpLow>>5) >= gVCPar.ASRTILow)
	{
		gAsr.KILow = 32767;
	}
	else
	{ 
		gAsr.KILow = ((Ulong)gVCPar.ASRKpLow<<10)/gVCPar.ASRTILow;
	}

	/*m_It = Filter64(gIMTSetQ12.T,m_It);    // 锟截癸拷锟斤拷院锟缴撅拷锟�
	if((SYNC_SVC == gCtrMotorType)&&((s32)gRotorSpeed.SpeedApply * m_It < 0)&&(abs(m_It)>500))   // SVC锟斤拷锟斤拷状态锟斤拷锟斤拷锟劫度伙拷
     {
	     gAsr.KPLow  = gVCPar.ASRKpLow<<7;
		 gAsr.KILow  = gAsr.KILow >> 2;
     }*/

	gAsr.SwitchHigh = ((Ulong)gVCPar.ASRSwitchHigh<<15)/gBasePar.FullFreq;
	gAsr.SwitchLow  = ((Ulong)gVCPar.ASRSwitchLow<<15)/gBasePar.FullFreq;

	
    // 选锟斤拷锟斤拷锟�
	m_DetaKP   = gAsr.KPHigh - gAsr.KPLow;
	m_DetaKI   = gAsr.KIHigh - gAsr.KILow;
    m_AbsFreq  = abs(gMainCmd.FreqSyn);
    if(m_AbsFreq <= gAsr.SwitchLow)
	{
		gAsr.Asr.KP = gAsr.KPLow;
		gAsr.Asr.KI = gAsr.KILow;
	}
	else if(m_AbsFreq >= gAsr.SwitchHigh)
	{
		gAsr.Asr.KP = gAsr.KPHigh;
		gAsr.Asr.KI = gAsr.KIHigh;
	}
	else
	{
		m_FreqUp    = m_AbsFreq - gAsr.SwitchLow;
		m_DetaFreq  = gAsr.SwitchHigh - gAsr.SwitchLow;
		gAsr.Asr.KP = ((long)m_DetaKP * (long)m_FreqUp)/m_DetaFreq + gAsr.KPLow;
		gAsr.Asr.KI = ((long)m_DetaKI * (long)m_FreqUp)/m_DetaFreq + gAsr.KILow;
	}
    
	
	tempKp = ( (long)gAsr.Asr.KP * (long)gBasePar.FullFreq01 / 10L)>>12;
    tempKi = ( (long)gAsr.Asr.KI * (long)gBasePar.FullFreq01 / 10L)>>12;

    gAsr.Asr.KP = __IQsat(tempKp, 32767, 1);        // 锟斤拷锟角伙拷锟斤拷锟�
    gAsr.Asr.KI = __IQsat(tempKi, 32767, 1);
    
// 锟斤拷锟街凤拷锟斤拷拇锟斤拷锟�
   /* m_KpLimit = ((long)abs(gMainCmd.FreqDesired - gRotorSpeed.SpeedApply) *(long)gAsr.Asr.KP) >>16;
    if(( 128 < m_KpLimit ) &&                            // 锟斤拷锟斤拷锟斤拷锟斤拷锟叫★拷锟斤拷锟斤拷锟斤拷址锟斤拷锟�
        (0 == gMainCmd.Command.bit.TorqueCtl) &&        // 转锟截匡拷锟狡碉拷时锟斤拷锟斤拷锟斤拷锟街凤拷锟斤拷
        (1 == gMainCmd.Command.bit.IntegralDiscrete))
    {
        //gAsr.Asr.KI = 0;		
    }*/
    gAsr.Asr.KD = 0;    
    gAsr.Asr.QP = 2;     // 十位
    gAsr.Asr.QI = 3;    // 380锟劫度伙拷锟斤拷锟斤拷锟斤拷锟斤拷为0.5锟斤拷锟斤拷320锟斤拷4锟斤拷     
}
/***************************************************************
                   为锟斤拷锟劫度伙拷准锟斤拷锟斤拷锟斤拷
****************************************************************/
void PrepareAsrPar1(void)
{
	s32 m_Long;
	s16 m_DetaKP,m_DetaKI,m_DetaFreq;
	s16	m_AbsFreq,m_FreqUp;
	s32 m_KP,m_KI;
	u16 m_UData;

    m_Long = ((s32)gVCPar.ASRKpZero * (s32)gBasePar.FullFreq01 / 10)>>3;                /*0.5ms锟劫度伙拷锟斤拷锟斤拷锟斤拷锟斤拷碌锟終P*/
    m_Long = Max(m_Long,10);
    gAsr.KPZero = Min(m_Long,32767);                                   
	m_Long = ((s32)gAsr.KPZero<<4)/(gVCPar.ASRTIZero*10);                              /*KP锟斤拷KI锟斤拷Q锟斤拷式锟斤拷?位*/
    m_Long = Max(m_Long,1);
    gAsr.KIZero = Min(m_Long,32767);

    m_Long = ((s32)gVCPar.ASRKpLow * (s32)gBasePar.FullFreq01 / 10)>>3;                /*0.5ms锟劫度伙拷锟斤拷锟斤拷锟斤拷锟斤拷碌锟終P*/
    m_Long = Max(m_Long,10);
    gAsr.KPLow = Min(m_Long,32767);                                   
	m_Long = ((s32)gAsr.KPLow<<4)/(gVCPar.ASRTILow*10);                              /*KP锟斤拷KI锟斤拷Q锟斤拷式锟斤拷锟�4位*/
    m_Long = Max(m_Long,1);
    gAsr.KILow = Min(m_Long,32767);

    m_Long = ((s32)gVCPar.ASRKpHigh * (s32)gBasePar.FullFreq01 / 10)>>3;               /*0.5ms锟劫度伙拷锟斤拷锟斤拷锟斤拷锟斤拷碌锟終P*/
    m_Long = Max(m_Long,10);
    gAsr.KPHigh = Min(m_Long,32767);                                   
	m_Long = ((s32)gAsr.KPHigh<<4)/(gVCPar.ASRTIHigh*10);                            /*KP锟斤拷KI锟斤拷Q锟斤拷式锟斤拷锟�4位*/
    m_Long = Max(m_Long,1);
    gAsr.KIHigh = Min(m_Long,32767);

	m_UData = abs(gMainCmd.FreqFeed);
	gMainCmd.FreqFeedReal = (s16)(((u32)m_UData * (u32)gBasePar.FullFreq + (1<<14))>>15);
	
	m_AbsFreq  = abs(gMainCmd.FreqFeedReal);
	m_DetaKP   = gAsr.KPHigh - gAsr.KPLow;
	m_DetaKI   = gAsr.KIHigh - gAsr.KILow;

    if((m_AbsFreq <= gVCPar.ASRSwitchZero) && (gVCPar.ZeroServer == 1))
	{
	    m_KP = gAsr.KPZero;
		m_KI = gAsr.KIZero;
	}
	else if(m_AbsFreq <= gVCPar.ASRSwitchLow)
	{
		m_KP = gAsr.KPLow;
		m_KI = gAsr.KILow;
	}
	else if(m_AbsFreq >= gVCPar.ASRSwitchHigh)
	{
		m_KP = gAsr.KPHigh;
		m_KI = gAsr.KIHigh;
	}
	else
	{
		m_FreqUp    = m_AbsFreq - gVCPar.ASRSwitchLow;
		m_DetaFreq  = gVCPar.ASRSwitchHigh - gVCPar.ASRSwitchLow;
		m_KP = ((s32)m_DetaKP * (s32)m_FreqUp)/m_DetaFreq + gAsr.KPLow;
		m_KI = ((s32)m_DetaKI * (s32)m_FreqUp)/m_DetaFreq + gAsr.KILow;
	}
    m_KP = Min(m_KP,32767);
    m_KI = Min(m_KI,32767);
    
    //锟劫度伙拷锟斤拷锟斤拷锟斤拷锟侥伙拷锟戒处锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟酵伙拷锟�
    m_DetaKP = m_KP - gAsr.Kp;
    if(abs(m_DetaKP) <= 1000)
    {
        gAsr.Kp = m_KP;
    }
    else if(m_DetaKP > 1000)
    {
        gAsr.Kp += 1000;
    }
    else
    {
        gAsr.Kp -= 1000;
    }
    m_DetaKI = m_KI - gAsr.Ki;
    if(abs(m_DetaKI) <= 500)
    {
        gAsr.Ki = m_KI;
    }
    else if(m_DetaKI > 500)
    {
        gAsr.Ki += 500;
    }
    else
    {
        gAsr.Ki -= 500;
    }
    // 位锟矫凤拷式锟斤拷锟劫度伙拷锟斤拷锟街碉拷锟斤拷锟斤拷锟斤拷值
    gAsr.KiPosBak = gAsr.Ki;  
}
/*************************************************************
	矢锟斤拷锟斤拷锟狡的癸拷压锟斤拷锟狡癸拷锟杰ｏ拷通锟斤拷母锟竭碉拷压锟斤拷锟斤拷锟斤拷锟阶拷锟�(锟斤拷锟斤拷转锟斤拷)锟斤拷锟斤拷锟街�
*************************************************************/
s16 CalUdcLimitIT(s16 Torque)
{
    s16 m_Deta;    
    s32 m_OutKP,m_OutKi,m_Out;
    if(gVFPar.ovGain != 0)//矢锟斤拷锟侥癸拷压锟斤拷锟狡癸拷锟斤拷
    {     
        gUdcLimitIt.UdcPid.KP = 10*gVFPar.ovGain;
        gUdcLimitIt.UdcPid.KI = 3*gVFPar.ovGain;
 
	    m_Deta = gOvUdc.Limit - (s16)gUDC.uDCFilter;
	    if(m_Deta >= 0)
	    {
	        gUdcLimitIt.UdcPid.Flag = 0;
	    }
	    else if(gUdcLimitIt.UdcPid.Flag == 0)
	    {
	        gUdcLimitIt.UdcPid.Flag = 1;
	        gUdcLimitIt.UdcPid.Total = ((s32)abs(gIMTSet.T)/Torque)<<12;
	    }    
	    
	    m_OutKP = ((s32)gUdcLimitIt.UdcPid.KP * m_Deta)<<4;
	    if(gUdcLimitIt.UdcPid.OutFlag * m_Deta <= 0)
	    {
	        m_OutKi = ((s32)gUdcLimitIt.UdcPid.KI * m_Deta);
	        gUdcLimitIt.UdcPid.Total += m_OutKi;
	    }
	    m_Out = gUdcLimitIt.UdcPid.Total + m_OutKP;

	    gUdcLimitIt.UdcPid.OutFlag = 0;
	    if(m_Out >= (4096L<<12))
	    {
	        m_Out = (4096L<<12);             //锟斤拷锟斤拷Q12锟斤拷式锟斤拷1
	        gUdcLimitIt.UdcPid.OutFlag = 1;
	    }
	    else if(m_Out <=( -400L<<12))    // 锟斤拷锟斤拷为-10%
	    {
	        m_Out = (-400L<<12);                                  //锟斤拷锟斤拷0
	        gUdcLimitIt.UdcPid.OutFlag = -1;
	    }
	    return ((s16)(m_Out>>12)); 
    }
    else
    {        
        return(4096);                                     //锟斤拷压锟斤拷锟狡诧拷锟斤拷效
    }
}

/*************************************************************
	矢锟斤拷锟斤拷锟狡碉拷锟劫度伙拷锟斤拷锟斤拷锟劫度伙拷锟斤拷锟斤拷执锟斤拷
set: gMainCmd.FreqSet锟斤拷
fbk: gRotorSpeed.SpeedApply锟斤拷

*************************************************************/
void VcAsrControl(void)
{
	s32 m_Long,m_Deta;
       
	if(gRotorSpeed.SpeedApply >= 0)
    {
        gAsr.Asr.Max = gAsr.PosTorqueLimit;
        gAsr.Asr.Min = -gAsr.NegTorqueLimit;
    }/*锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷锟阶拷锟斤拷锟斤拷锟�*/
    else
    {
        gAsr.Asr.Max = gAsr.NegTorqueLimit;
        gAsr.Asr.Min = -gAsr.PosTorqueLimit;
	}

	m_Long = (s32)gMainCmd.FreqSetApply - (s32)gRotorSpeed.SpeedApply;
    gAsr.Asr.Deta = __IQsat(m_Long, 16383, -16383);

	if((gSubCommand.bit.VCFolFlag == 1)&&(1 == gMainCmd.Command.bit.TorqueCtl))
    {                                                   //锟接伙拷转锟截匡拷锟斤拷
       m_Deta = ((long)gRotorSpeed.FreWindow<<15)/gBasePar.FullFreq01;
       gAsr.Asr.KI = 0; 
       if(gAsr.Asr.Deta > m_Deta)       //锟劫度伙拷锟斤拷锟斤拷锟斤拷锟接达拷锟斤拷锟斤拷锟斤拷为0锟斤拷锟斤拷锟斤拷锟斤拷锟皆变化
       {
            gAsr.Asr.Deta -=  m_Deta;
       }
       else if(gAsr.Asr.Deta < (-m_Deta))
       {
            gAsr.Asr.Deta += m_Deta;
       }
       else
       {
            gAsr.Asr.Deta = 0;          
       }       
    }
	PID((PID_STRUCT *)&gAsr.Asr);
    gVCPar.AsrOut = gAsr.Asr.Out>>(16-12);
	//========================锟劫度伙拷转锟斤拷锟斤拷锟斤拷锟斤拷锟�==========================
    // 1锟斤拷 锟斤拷锟接匡拷锟斤拷锟斤拷为锟接伙拷时锟斤拷要锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷踊锟斤拷锟斤拷锟阶拷锟�
    if((gSubCommand.bit.VCFolFlag == 1)&&(1 == gMainCmd.Command.bit.TorqueCtl))
    {
        m_Deta = ((long)gVCPar.TorMasToFol<<12)/1000;// 1位小锟斤拷锟姐，锟皆碉拷锟斤拷疃拷锟斤拷锟轿拷锟街碉拷陌俜直锟�
        m_Deta = (gVCPar.AsrOut>>12) + m_Deta;
        m_Deta = __IQsat(m_Deta,gAsr.Asr.Max,gAsr.Asr.Min);
        gIMTSet.T = m_Deta<<12;     
        //gIMTSet.T = FilterL(gIMTSet.T,(m_Deta<<12),((32-gVCPar.VCTorqFilter)<<10)) ;
    }
    else
    {
        gIMTSet.T = gVCPar.AsrOut;
		//gIMTSet.T = FilterL(gIMTSet.T,gVCPar.AsrOut,((32-gVCPar.VCTorqFilter)<<10)) ;
    }	
}

/*************************************************************
	锟铰碉拷锟劫度伙拷
*************************************************************/
void VcAsrControl1(void)
{
    s32 m_Max,m_Min;
    s32 m_DetaFreq;
    s64 m_KpOut,m_KiOut,m_Out;
    s32 m_DetaPos;

	if(gRotorSpeed.SpeedApply >= 0)
    {
        m_Max = (s32)gAsr.PosTorqueLimit << 16;
        m_Min = -(s32)gAsr.NegTorqueLimit << 16;
    }/*锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷锟阶拷锟斤拷锟斤拷锟�*/
    else
    {
        m_Max = (s32)gAsr.NegTorqueLimit << 16;
        m_Min = -(s32)gAsr.PosTorqueLimit << 16;
	}
        
    m_DetaFreq = ((s32)gMainCmd.FreqSet<<9) - gAsr.FreqSet;
    if(abs(m_DetaFreq) < (1L<<9))
    {
        gAsr.FreqSet = ((s32)gMainCmd.FreqSet<<9);
    }
    else
    {
        gAsr.FreqSet += (m_DetaFreq>>1);                    /*平锟斤拷锟剿诧拷锟斤拷锟斤拷,使锟斤拷锟劫讹拷偏锟斤拷平锟斤拷*/
    }
    m_DetaFreq          = gAsr.FreqSet - gAsr.FreqFeedFilter;
    gMainCmd.DetaFreq   = m_DetaFreq>>9;
    m_DetaFreq          = Min(m_DetaFreq,(16383L<<9));      /*锟睫讹拷频锟斤拷偏锟斤拷锟斤拷锟斤拷值为锟斤拷锟狡碉拷实锟揭伙拷锟�*/
    m_DetaFreq          = Max(m_DetaFreq,(-16383L<<9));     /*锟睫讹拷频锟斤拷偏锟斤拷锟斤拷锟斤拷值为锟斤拷锟狡碉拷实锟揭伙拷锟�*/
    m_KpOut = (gAsr.Kp * (s64)m_DetaFreq)>>5;               /*使Kp锟洁当锟斤拷Q12锟斤拷式*/

    DINT;
    m_DetaPos = gAsr.PosSet - gPGData.RefPos;
	
	if(gAsr.OutFlag * m_DetaPos > 0)                    /*锟斤拷锟街凤拷锟诫处锟斤拷:锟斤拷示偏锟斤拷锟斤拷薹锟酵拷锟脚ｏ拷锟斤拷锟揭诧拷锟�?*/
	{
	    gAsr.PosSet = gPGData.RefPos + gAsr.DetaPos;
		m_DetaPos = gAsr.DetaPos;
	}

    gAsr.DetaPos = m_DetaPos;
    m_KiOut = ((s64)gAsr.DetaPos * (s64)gAsr.KiPos)>>3; /*使Ki锟洁当锟斤拷Q7锟斤拷式*/

    if(gAsr.KiPos != gAsr.KiPosBak)
    {
        gAsr.PosSet = ((s64)gAsr.KiPos * (s64)m_DetaPos)/gAsr.KiPosBak + gPGData.RefPos;
        gAsr.KiPos = gAsr.KiPosBak;
    }//锟斤拷锟斤拷锟叫伙拷锟侥达拷锟斤拷锟斤拷使锟斤拷锟斤拷锟斤拷锟斤拷锟狡斤拷锟斤拷锟酵伙拷锟�
    
    m_Out = m_KpOut + m_KiOut;
    EINT;

    /*锟斤拷锟斤拷锟斤拷锟斤拷卸希锟斤拷锟斤拷锟斤拷锟斤拷帽锟斤拷捅锟街�*/
    gAsr.OutFlag = 0;
    if(m_Out < m_Min)
    {
        m_Out = m_Min;              
        gAsr.OutFlag = -1;                                  /*锟斤拷锟斤拷锟斤拷*/
    }
    else if(m_Out > m_Max)
    {
        m_Out = m_Max;
        gAsr.OutFlag = 1;                                   /*锟斤拷锟斤拷锟斤拷*/
    }
    gAsr.Out = m_Out;    	
	gIMTSet.T = gAsr.Out>>4; 		 // Q24锟斤拷式 
}

/*************************************************************
	FVC矢锟斤拷锟斤拷锟狡的碉拷锟斤拷锟斤拷, 锟斤拷锟斤拷同锟斤拷锟斤拷锟斤拷锟届步锟斤拷
*************************************************************/
void VCCsrControl(void)
{
	Ulong 		m_Long;
	MT_STRUCT	m_UMT;
	s16         m_UmLimit,m_AbsUTSet;    
	s16         m_MaxVolt;
	//m_MaxVolt = 5079L * (long)gInvInfo.InvVolt / gMotorInfo.Votage; //矢锟斤拷锟斤拷锟斤拷锟窖癸拷锟斤拷锟轿拷锟狡碉拷锟斤拷疃拷锟窖癸拷锟�1.24锟斤拷
	//MT锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷慕锟斤拷浯︼拷锟�
	DINT;
	gIMTSetApply.T = gIMTSet.T;
	gIMTSetApply.M = gIMTSet.M;
    EINT;

    if(gFluxWeak.Mode == 2)
    {
        IMFluxWeak2();
    }
    if(gPmCsr2.FluxWeakFlag == 1)
    {
        return;
    }
	m_MaxVolt = gOutVolt.MaxOutVoltPer + 100;

// Csr axis-M
    gImAcrQ24.Max = ((s32)m_MaxVolt << 12);
    gImAcrQ24.Min = - gImAcrQ24.Max;
    gImAcrQ24.Deta = gIMTSetApply.M - gIMTQ24.M;   
    
    PID32(&gImAcrQ24);
    gUMTSet.M = (s16)(gImAcrQ24.Out >> 12);
// Csr axis-T
    gItAcrQ24.Max = ((s32)m_MaxVolt << 12);
    gItAcrQ24.Min = - gItAcrQ24.Max;
    gItAcrQ24.Deta = gIMTSetApply.T - gIMTQ24.T;
    
    PID32(&gItAcrQ24);
    gUMTSet.T = (s16)(gItAcrQ24.Out >> 12);   

    // 同锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
	if(SYNC_FVC == gCtrMotorType)
	{
	    if(gPmDecoup.EnableDcp == 1)                   //锟斤拷压前锟斤拷
	    {
	        gUMTSet.T += gPmDecoup.EMF; 
	    }
		else if(gPmDecoup.EnableDcp == 2)              //锟斤拷锟界动锟斤拷前锟斤拷
		{		   
		    gUMTSet.T += gPmDecoup.RotVq;
	        gUMTSet.M += gPmDecoup.RotVd;
		}
	}

	gUMTSet.M = __IQsat(gUMTSet.M, m_MaxVolt, -m_MaxVolt);
    gUMTSet.T = __IQsat(gUMTSet.T, m_MaxVolt, -m_MaxVolt);
   
	//锟斤拷锟斤拷锟斤拷锟斤拷锟窖癸拷锟街�
	m_Long = (((long)gUMTSet.M * (long)gUMTSet.M) + 
	          ((long)gUMTSet.T * (long)gUMTSet.T));
	gUAmpTheta.Amp = (Uint)qsqrt(m_Long);
    
	gOutVolt.VoltApply = gUAmpTheta.Amp;
    gOutVolt.VoltFilter = Filter16(gOutVolt.VoltApply, gOutVolt.VoltFilter);
	m_UMT.M = gUMTSet.M;
	m_UMT.T = gUMTSet.T;

	//锟斤拷锟斤拷MT锟斤拷锟窖癸拷薪牵锟斤拷锟斤拷锟斤拷头锟绞斤拷锟�        .... 
	if(gUAmpTheta.Amp > (u16)m_MaxVolt)   // + 500))	//rt	
	{
	    gOutVolt.VoltApply	= m_MaxVolt;
        
        m_UmLimit = m_MaxVolt - 100;
        m_UmLimit = ((m_UmLimit > 0) ? m_UmLimit:0);        
        if(m_UMT.M > m_UmLimit)  
        {
            m_UMT.M = m_UmLimit;
        }
        else if(m_UMT.M < -m_UmLimit)        
        {
            m_UMT.M = -m_UmLimit;
        }
       	m_Long = (((s32)m_MaxVolt * (s32)m_MaxVolt) - ((s32)m_UMT.M * (s32)m_UMT.M));
       	m_Long = Max(m_Long,100);
       	m_UMT.T = (u16)qsqrt(m_Long); 
        m_UMT.T = Max(m_UMT.T,100); 
		m_AbsUTSet = abs(gUMTSet.T);
        if(m_UMT.T > m_AbsUTSet)
        {
            m_UMT.T = m_AbsUTSet;
        }
       	m_UMT.T = ((gUMTSet.T > 0) ? (m_UMT.T) : (-m_UMT.T)); 
		gItAcrQ24.Total = ((s32)m_UMT.T << 12);
	}
	gUAmpTheta.ThetaOld = gUAmpTheta.Theta;		// wg
	gUAmpTheta.Theta = user_atan(m_UMT.M, m_UMT.T);
	gOutVolt.VoltPhaseApply = gUAmpTheta.Theta;
	gOutVolt.VoltPhaseApply1 = gUAmpTheta.ThetaOld + gPhase.IMPhaseOld;				// wg
	gOutVolt.VoltPhaseApply2 = gUAmpTheta.Theta + gPhase.IMPhase;		// wg
}

