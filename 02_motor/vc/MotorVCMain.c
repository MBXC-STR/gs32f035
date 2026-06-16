/****************************************************************
�ļ����ܣ�ʸ�����Ƶ���Ҫ���򲿷�
�ļ��汾�� 
���¸��£� 
	
****************************************************************/
#include "MotorVCInclude.h"
#include "MotorEncoder.h"
#include "MotorPmsmParEst.h"
#include "MotorPmsmMain.h"

// // ȫ�ֱ�������
VC_INFO_STRUCT			gVCPar;			//VC����
MT_STRUCT_Q24           gIMTSet;        //MT��ϵ�µ��趨������Q24��ʾ
MT_STRUCT_Q24           gIMTSetApply;	//MT��ϵ�µĵ���ָ��ֵ
MT_STRUCT				gUMTSet;		//MT��ϵ�µ��趨��ѹ
AMPTHETA_STRUCT			gUAmpTheta;		//���������趨��ѹ

PID32_STRUCT        gImAcrQ24;
PID32_STRUCT        gItAcrQ24;
PID32_STRUCT        gIMAcr;
PID32_STRUCT        gITAcr;

ASR_STRUCT				gAsr;			//�ٶȻ�
VC_CSR_PARA             gVcCsrPara;  //�ջ�ʸ�������������õ��Ĳ���
PID_STRUCT              gIMSetAcr;   //�������ŵ�������ֵ
PID_STRUCT              gWspid;      //�첽���ջ�ʸ���ٶȻ�ת�������������
MODIFYWS_STRUCT         gModifyws;   //�첽���ջ�ʸ��ת����������
MT_STRUCT               gPWMVAlphBeta;//�������PWM������alfa��beta���ѹ
UDC_LIMIT_IT_STRUCT     gUdcLimitIt;
extern PMSM_FLUX_WEAK_STRUCT   gPmFluxWeak;
int gTorLimit_SVC = 0;
// // �ļ��ڲ��������� 
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
	ʸ�����Ƶ����ݸ�λ������ͣ���׶�ִ��
*************************************************************/
void ResetParForVC(void)
{
	ResetAsrPar();
	ResetCsrPar();
	ResetParForPmsmFwc();
}

/*************************************************************
	ʸ�����Ƶ��ٶȻ�������λ
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
	ʸ�����Ƶĵ�������λ
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
    //���Ϻ�����
    gMainCmd.FreqSyn = gRotorSpeed.SpeedApply;

    gPmCsr2.Csr1Flag = 0;
	gPmCsr2.Csr1Flag1 = 0;
    gPmCsr2.FluxWeakFlag = 0;
}
/*************************************************************
	ʸ�����Ƶ�ת�ؼ��㣨Ҫ��ת�ص��������ŵ���ʸ���Ͳ�����2��Ƶ������2����
ͬ��������첽�������ת�ؿ��Ƴ���
*************************************************************/
void CalTorqueLimitPar(void)
{
	int m_TorLimit,m_MaxLimit,m_NegTorqueLimit;
	int m_InvIM;
	Ulong m_Long;

	//m_InvIM = ((long)(gIMTSet.M>>12) * (long)gMotorInfo.Current)/gInvInfo.InvCurrForP; //ʹ���û��趨���Ͷ�Ӧ�ĵ���
	m_InvIM = ((long)(gIMTSet.M>>12) * (long)gMotorInfo.Current)/gInvInfo.InvCurrent; //ʹ���û��趨���Ͷ�Ӧ�ĵ���
	m_InvIM = ((long)m_InvIM * 1000)>>12;
//	m_Long  = (1800L * 1800L) - ((long)m_InvIM * (long)m_InvIM);    //����������ʸ����С�ڱ�Ƶ�������2��
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
	m_TorLimit = ((long)m_TorLimit<<12)/1000;	//ת��Ϊ����ֵ
	//gAsr.TorqueLimit = m_TorLimit;          // PM �� IM����ת�ؿ���T������趨
    gAsr.PosTorqueLimit = m_TorLimit; 

	if(((SYNC_SVC == gCtrMotorType)&&(gMainCmd.FreqReal < 100))||(gFluxWeak.CoefKI == 0))     // 1Hz���²����ֵ綯�ͷ���ת��
	{
	    m_TorLimit = (gVCPar.PosVCTorqueLim > m_MaxLimit)?m_MaxLimit : gVCPar.PosVCTorqueLim;  
	}
	else
	{
        m_TorLimit = (gVCPar.NegVCTorqueLim > m_MaxLimit)?m_MaxLimit : gVCPar.NegVCTorqueLim;   
    }											  
	m_TorLimit = ((long)m_TorLimit * (long)gInvInfo.InvCurrForP)/gMotorInfo.Current;
	m_TorLimit = ((long)m_TorLimit<<12)/1000;	//ת��Ϊ����ֵ
	gAsr.TorqueLimit = m_TorLimit;           // PM �� IM����ת�ؿ���T������趨
	m_NegTorqueLimit = CalUdcLimitIT(gAsr.TorqueLimit);
    gAsr.NegTorqueLimit = (s16)(((s32)m_TorLimit * (s32)m_NegTorqueLimit) >> 12);
}
/*************************************************************
	ʸ�����Ƶ��ٶȻ�����������ת����Ԥ��ת���ò�����
׼������:(����ѡ�����)
 Kp = (Kp_func << 8)* (f_base/10 >>12) << 3 := Kp_func * 100 |(f_base == 80.00Hz)
 Ki = (Kp_func/Ki_func << 10) * (f_base/10 >>12) << 3 := Kp_func/Ki_func * 1600 |(f_base == 80.00Hz)
*************************************************************/
void PrepareAsrPar(void)
{
    int	  m_AbsFreq, m_FreqUp;
    int   m_DetaKP, m_DetaKI, m_DetaFreq;
    //int   m_KpLimit;
    long  tempKp;
    long  tempKi;

    // ׼���л�����  
    gAsr.KPHigh = gVCPar.ASRKpHigh<<8; 
    gAsr.KPLow  = gVCPar.ASRKpLow<<8;  
	gAsr.KPLow  = (s16)((s32)gAsr.KPLow * gAsr.KPLowCoff / 10L);    // ��ͬ������Kpֵ��һ��
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

	/*m_It = Filter64(gIMTSetQ12.T,m_It);    // �ع���Ժ�ɾ��
	if((SYNC_SVC == gCtrMotorType)&&((s32)gRotorSpeed.SpeedApply * m_It < 0)&&(abs(m_It)>500))   // SVC����״̬�����ٶȻ�
     {
	     gAsr.KPLow  = gVCPar.ASRKpLow<<7;
		 gAsr.KILow  = gAsr.KILow >> 2;
     }*/

	gAsr.SwitchHigh = ((Ulong)gVCPar.ASRSwitchHigh<<15)/gBasePar.FullFreq;
	gAsr.SwitchLow  = ((Ulong)gVCPar.ASRSwitchLow<<15)/gBasePar.FullFreq;

	
    // ѡ�����
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

    gAsr.Asr.KP = __IQsat(tempKp, 32767, 1);        // ���ǻ����
    gAsr.Asr.KI = __IQsat(tempKi, 32767, 1);
    
// ���ַ���Ĵ���
   /* m_KpLimit = ((long)abs(gMainCmd.FreqDesired - gRotorSpeed.SpeedApply) *(long)gAsr.Asr.KP) >>16;
    if(( 128 < m_KpLimit ) &&                            // ���������С�������ַ���
        (0 == gMainCmd.Command.bit.TorqueCtl) &&        // ת�ؿ��Ƶ�ʱ�������ַ���
        (1 == gMainCmd.Command.bit.IntegralDiscrete))
    {
        //gAsr.Asr.KI = 0;		
    }*/
    gAsr.Asr.KD = 0;    
    gAsr.Asr.QP = 2;     // ʮλ
    gAsr.Asr.QI = 3;    // 380�ٶȻ���������Ϊ0.5����320��4��     
}
/***************************************************************
                   Ϊ���ٶȻ�׼������
****************************************************************/
void PrepareAsrPar1(void)
{
	s32 m_Long;
	s16 m_DetaKP,m_DetaKI,m_DetaFreq;
	s16	m_AbsFreq,m_FreqUp;
	s32 m_KP,m_KI;
	u16 m_UData;

    m_Long = ((s32)gVCPar.ASRKpZero * (s32)gBasePar.FullFreq01 / 10)>>3;                /*0.5ms�ٶȻ���������µ�KP*/
    m_Long = Max(m_Long,10);
    gAsr.KPZero = Min(m_Long,32767);                                   
	m_Long = ((s32)gAsr.KPZero<<4)/(gVCPar.ASRTIZero*10);                              /*KP��KI��Q��ʽ��?λ*/
    m_Long = Max(m_Long,1);
    gAsr.KIZero = Min(m_Long,32767);

    m_Long = ((s32)gVCPar.ASRKpLow * (s32)gBasePar.FullFreq01 / 10)>>3;                /*0.5ms�ٶȻ���������µ�KP*/
    m_Long = Max(m_Long,10);
    gAsr.KPLow = Min(m_Long,32767);                                   
	m_Long = ((s32)gAsr.KPLow<<4)/(gVCPar.ASRTILow*10);                              /*KP��KI��Q��ʽ���4λ*/
    m_Long = Max(m_Long,1);
    gAsr.KILow = Min(m_Long,32767);

    m_Long = ((s32)gVCPar.ASRKpHigh * (s32)gBasePar.FullFreq01 / 10)>>3;               /*0.5ms�ٶȻ���������µ�KP*/
    m_Long = Max(m_Long,10);
    gAsr.KPHigh = Min(m_Long,32767);                                   
	m_Long = ((s32)gAsr.KPHigh<<4)/(gVCPar.ASRTIHigh*10);                            /*KP��KI��Q��ʽ���4λ*/
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
    
    //�ٶȻ��������Ļ��䴦���������������ͻ��
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
    // λ�÷�ʽ���ٶȻ����ֵ�������ֵ
    gAsr.KiPosBak = gAsr.Ki;  
}
/*************************************************************
	ʸ�����ƵĹ�ѹ���ƹ��ܣ�ͨ��ĸ�ߵ�ѹ�������ת��(����ת��)�����ֵ
*************************************************************/
s16 CalUdcLimitIT(s16 Torque)
{
    s16 m_Deta;    
    s32 m_OutKP,m_OutKi,m_Out;
    if(gVFPar.ovGain != 0)//ʸ���Ĺ�ѹ���ƹ���
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
	        m_Out = (4096L<<12);             //����Q12��ʽ��1
	        gUdcLimitIt.UdcPid.OutFlag = 1;
	    }
	    else if(m_Out <=( -400L<<12))    // ����Ϊ-10%
	    {
	        m_Out = (-400L<<12);                                  //����0
	        gUdcLimitIt.UdcPid.OutFlag = -1;
	    }
	    return ((s16)(m_Out>>12)); 
    }
    else
    {        
        return(4096);                                     //��ѹ���Ʋ���Ч
    }
}

/*************************************************************
	ʸ�����Ƶ��ٶȻ������ٶȻ�����ִ��
set: gMainCmd.FreqSet��
fbk: gRotorSpeed.SpeedApply��

*************************************************************/
void VcAsrControl(void)
{
	s32 m_Long,m_Deta;
       
	if(gRotorSpeed.SpeedApply >= 0)
    {
        gAsr.Asr.Max = gAsr.PosTorqueLimit;
        gAsr.Asr.Min = -gAsr.NegTorqueLimit;
    }/*��������������ʱ���ת������*/
    else
    {
        gAsr.Asr.Max = gAsr.NegTorqueLimit;
        gAsr.Asr.Min = -gAsr.PosTorqueLimit;
	}

	m_Long = (s32)gMainCmd.FreqSetApply - (s32)gRotorSpeed.SpeedApply;
    gAsr.Asr.Deta = __IQsat(m_Long, 16383, -16383);

	if((gSubCommand.bit.VCFolFlag == 1)&&(1 == gMainCmd.Command.bit.TorqueCtl))
    {                                                   //�ӻ�ת�ؿ���
       m_Deta = ((long)gRotorSpeed.FreWindow<<15)/gBasePar.FullFreq01;
       gAsr.Asr.KI = 0; 
       if(gAsr.Asr.Deta > m_Deta)       //�ٶȻ��������Ӵ�������Ϊ0���������Ա仯
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
	//========================�ٶȻ�ת���������==========================
    // 1�� ���ӿ�����Ϊ�ӻ�ʱ��Ҫ����������ӻ�����ת��
    if((gSubCommand.bit.VCFolFlag == 1)&&(1 == gMainCmd.Command.bit.TorqueCtl))
    {
        m_Deta = ((long)gVCPar.TorMasToFol<<12)/1000;// 1λС���㣬�Ե�������Ϊ��ֵ�İٷֱ�
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
	�µ��ٶȻ�
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
    }/*��������������ʱ���ת������*/
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
        gAsr.FreqSet += (m_DetaFreq>>1);                    /*ƽ���˲�����,ʹ���ٶ�ƫ��ƽ��*/
    }
    m_DetaFreq          = gAsr.FreqSet - gAsr.FreqFeedFilter;
    gMainCmd.DetaFreq   = m_DetaFreq>>9;
    m_DetaFreq          = Min(m_DetaFreq,(16383L<<9));      /*�޶�Ƶ��ƫ������ֵΪ���Ƶ�ʵ�һ��*/
    m_DetaFreq          = Max(m_DetaFreq,(-16383L<<9));     /*�޶�Ƶ��ƫ������ֵΪ���Ƶ�ʵ�һ��*/
    m_KpOut = (gAsr.Kp * (s64)m_DetaFreq)>>5;               /*ʹKp�൱��Q12��ʽ*/

    DINT;
    m_DetaPos = gAsr.PosSet - gPGData.RefPos;
	
	if(gAsr.OutFlag * m_DetaPos > 0)                    /*���ַ��봦��:��ʾƫ����޷�ͬ��ţ����Ҳ��?*/
	{
	    gAsr.PosSet = gPGData.RefPos + gAsr.DetaPos;
		m_DetaPos = gAsr.DetaPos;
	}

    gAsr.DetaPos = m_DetaPos;
    m_KiOut = ((s64)gAsr.DetaPos * (s64)gAsr.KiPos)>>3; /*ʹKi�൱��Q7��ʽ*/

    if(gAsr.KiPos != gAsr.KiPosBak)
    {
        gAsr.PosSet = ((s64)gAsr.KiPos * (s64)m_DetaPos)/gAsr.KiPosBak + gPGData.RefPos;
        gAsr.KiPos = gAsr.KiPosBak;
    }//�����л��Ĵ�����ʹ���������ƽ����ͻ��
    
    m_Out = m_KpOut + m_KiOut;
    EINT;

    /*��������жϣ��������ñ��ͱ�־*/
    gAsr.OutFlag = 0;
    if(m_Out < m_Min)
    {
        m_Out = m_Min;              
        gAsr.OutFlag = -1;                                  /*������*/
    }
    else if(m_Out > m_Max)
    {
        m_Out = m_Max;
        gAsr.OutFlag = 1;                                   /*������*/
    }
    gAsr.Out = m_Out;    	
	gIMTSet.T = gAsr.Out>>4; 		 // Q24��ʽ 
}

/*************************************************************
	FVCʸ�����Ƶĵ�����, ����ͬ�������첽��
*************************************************************/
void VCCsrControl(void)
{
	Ulong 		m_Long;
	MT_STRUCT	m_UMT;
	s16         m_UmLimit,m_AbsUTSet;    
	s16         m_MaxVolt;
	//m_MaxVolt = 5079L * (long)gInvInfo.InvVolt / gMotorInfo.Votage; //ʸ�������ѹ����Ϊ��Ƶ�����ѹ��1.24��
	//MT����������Ľ��䴦��
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
    gUMTSet.M = (int)(gImAcrQ24.Out >> 12);
// Csr axis-T
    gItAcrQ24.Max = ((s32)m_MaxVolt << 12);
    gItAcrQ24.Min = - gItAcrQ24.Max;
    gItAcrQ24.Deta = gIMTSetApply.T - gIMTQ24.T;
    
    PID32(&gItAcrQ24);
    gUMTSet.T = (int)(gItAcrQ24.Out >> 12);   

    // ͬ�����������
	if(SYNC_FVC == gCtrMotorType)
	{
	    if(gPmDecoup.EnableDcp == 1)                   //��ѹǰ��
	    {
	        gUMTSet.T += gPmDecoup.EMF; 
	    }
		else if(gPmDecoup.EnableDcp == 2)              //���綯��ǰ��
		{		   
		    gUMTSet.T += gPmDecoup.RotVq;
	        gUMTSet.M += gPmDecoup.RotVd;
		}
	}

	gUMTSet.M = __IQsat(gUMTSet.M, m_MaxVolt, -m_MaxVolt);
    gUMTSet.T = __IQsat(gUMTSet.T, m_MaxVolt, -m_MaxVolt);
   
	//���������ѹ��ֵ
	m_Long = (((long)gUMTSet.M * (long)gUMTSet.M) + 
	          ((long)gUMTSet.T * (long)gUMTSet.T));
	gUAmpTheta.Amp = (Uint)qsqrt(m_Long);
    
	gOutVolt.VoltApply = gUAmpTheta.Amp;
    gOutVolt.VoltFilter = Filter16(gOutVolt.VoltApply, gOutVolt.VoltFilter);
	m_UMT.M = gUMTSet.M;
	m_UMT.T = gUMTSet.T;

	//����MT���ѹ�нǣ������ͷ�ʽ��        .... 
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

