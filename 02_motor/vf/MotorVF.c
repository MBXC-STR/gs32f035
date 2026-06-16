/****************************************************************
锟侥硷拷锟斤拷锟杰ｏ拷VF锟斤拷锟斤拷锟斤拷爻锟斤拷颍锟斤拷锟絍F锟斤拷锟竭硷拷锟姐、转锟斤拷锟斤拷锟斤拷锟斤拷转锟筋补锟斤拷锟斤拷锟斤拷锟斤拷锟今荡等达拷锟斤拷
锟侥硷拷锟芥本锟斤拷 
锟斤拷锟铰革拷锟铰ｏ拷 
	
*************************************************************/

#include "MotorVFInclude.h"
#include "MotorInclude.h"

// 全锟街憋拷锟斤拷锟斤拷锟斤拷
VF_INFO_STRUCT			gVFPar;		//VF锟斤拷锟斤拷
VF_AUTO_TORQUEBOOST_VAR gVFAutoVar;

VF_WS_COMP_STRUCT		gWsComp;
VAR_AVR_STRUCT			gVarAvr;
//VF_CURRENT_CONTROL      gVfCsr;

OVER_SHOCK_STRUCT		gVfOsc;	//锟斤拷锟斤拷锟斤拷锟矫结构锟斤拷锟斤拷
THREE_ORDER_FILTER      gOscAmp;

// Vf 失锟劫匡拷锟斤拷锟斤拷乇锟斤拷锟�
VF_OVER_UDC_DAMP        gVfOverUdc;
OVER_CURRENT_DAMP2	    gOvCur2;
VF_FREQ_DEAL            gVfFreq2;

OVER_CURRENT_DAMP	    gOvCur;
OVER_UDC_CTL_STRUCT		gOvUdc;

HVF_OSC_DAMP_STRUCT     gHVfOscDamp;
HVF_OSC_JUDGE_INDEX     gHVfOscIndex;           // HVf 锟斤拷系锟斤拷锟侥硷拷锟斤拷
HVF_DB_COMP_OPT         gHVfDeadBandCompOpt; 
MT_STRUCT				gHVfCur;

// 锟侥硷拷锟节诧拷锟斤拷锟斤拷锟斤拷锟斤拷
void CalTorqueUp(void);
void ResDropComp(MT_STRUCT * );

int VfOverCurDeal2(int step);
int VfOverUdcDeal2(int step);

void VFOVUdcLimit(void);

/************************************************************
锟斤拷锟斤拷锟斤拷锟斤拷:锟斤拷
锟斤拷锟斤拷锟斤拷锟�:锟斤拷
锟斤拷锟斤拷位锟斤拷:锟较碉拷锟绞硷拷锟斤拷锟斤拷锟斤拷锟街�
锟斤拷锟斤拷锟斤拷锟斤拷:锟斤拷
锟斤拷锟斤拷锟斤拷锟斤拷:锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷锟矫碉拷锟侥憋拷锟斤拷
************************************************************/
void VfVarInitiate(void)  //VF锟斤拷锟叫憋拷锟斤拷锟斤拷始锟斤拷锟斤拷锟斤拷
{
    gVFPar.FreqApply = 0;
    gVfFreq2.freqSet = 0;
    
	if(gMainCmd.FreqDesired > 0)		//锟斤拷锟斤拷目锟斤拷频锟绞凤拷锟斤拷确锟斤拷锟斤拷始锟斤拷位锟斤拷
	{
		gOutVolt.VoltPhaseApply = 16384;
	}
	else
	{
		gOutVolt.VoltPhaseApply = -16384;
	}		
	gPhase.OutPhase = gPhase.IMPhase + gOutVolt.VoltPhaseApply;

//VF锟皆讹拷转锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷始锟斤拷
	gVFAutoVar.VfCurrentIs = 0; 
	gVFAutoVar.VfReverseAngle = 0;
    gVFAutoVar.AutoTorquePID.Total = 0;
    gVFAutoVar.AutoTorquePID.Out   = 0;
    gVFAutoVar.VfReverseVolt = 0;
    gVFAutoVar.VfTorqueEnableTime = 0;
    gWsComp.Coff = gVFPar.VFWsComp;
    gWsComp.DelayTime = 0;
    gWsComp.WsCompMT.M = 0;
    gWsComp.WsCompMT.T = 0;
    gWsComp.WsCompMTApply.M = 0;
    gWsComp.CompFreq = 0;
    gVFAutoVar.DestinationVolt = 0;
    gOutVolt.detVfVoltUp = 0;
    gVFAutoVar.VfAutoTorqueEnable = 0;

    gVfOsc.TimesNub = 0;
    gVfOsc.IO = 0;
    gVfOsc.ShockDecrease = 0;

    gVfFreq2.preSpdFlag = gMainCmd.Command.bit.SpeedFlag; 
}

/************************************************************
	锟斤拷锟斤拷锟叫筹拷锟斤拷锟斤拷锟斤拷压锟斤拷锟斤拷
************************************************************/
void VFOVUdcLimit(void)
{
	int	m_Mid;

	m_Mid = ((Ulong)gOvUdc.Limit * 3932L)>>12;			//0.96
	
	if((gUDC.uDCFilter > m_Mid) && (gOvUdc.StepApply <= 0) )
	{
		gOvUdc.OvUdcLimitTime++;
		if(gOvUdc.OvUdcLimitTime > 10000)
		{
			gOvUdc.OvUdcLimitTime = 10000;
		}
	}
	else
	{
		gOvUdc.OvUdcLimitTime = 0;
	}
}

/************************************************************
锟斤拷锟斤拷锟斤拷锟斤拷:锟斤拷么值频锟斤拷Q15,锟斤拷锟絍F使锟斤拷频锟斤拷实锟斤拷值
锟斤拷锟斤拷锟斤拷锟�:锟斤拷锟斤拷锟窖筈12,
锟斤拷锟斤拷位锟斤拷:锟斤拷锟斤拷状态锟斤拷2ms循锟斤拷
锟斤拷锟斤拷锟斤拷锟斤拷:
锟斤拷锟斤拷锟斤拷锟斤拷:锟斤拷锟捷革拷锟斤拷频锟绞猴拷VF锟斤拷锟竭ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟窖�
************************************************************/
int CalOutVotInVFStatus(int freq)
{
	Uint m_Freq2,m_MotorFreq2;
	int  m_AbsFreq;
    int  mVolt;
	long m_DetaFreq,m_LowFreq,m_HighFreq,m_LowVolt,m_HighVolt;
    long m_VFLineFreq1,m_VFLineFreq2,m_VFLineFreq3,m_Frequency;
    
	m_AbsFreq = abs(freq);

	//gOutVolt.VoltPhase = 0;    
    gVFAutoVar.DestinationVolt = 0;

//...锟斤拷转锟斤拷锟斤拷锟斤拷

    if(gVFPar.VFLineType == 1)	//锟斤拷锟絍F锟斤拷锟竭ｏ拷 锟斤拷锟斤拷转锟斤拷锟斤拷锟斤拷
	{
	    m_VFLineFreq1 = (long)gVFPar.VFLineFreq1 * (long)gMainCmd.pu2siCoeff;
        m_VFLineFreq2 = (long)gVFPar.VFLineFreq2 * (long)gMainCmd.pu2siCoeff;
        m_VFLineFreq3 = (long)gVFPar.VFLineFreq3 * (long)gMainCmd.pu2siCoeff;
        m_Frequency   = (long)gMotorInfo.Frequency * (long)gMainCmd.pu2siCoeff;    

		if(gMainCmd.FreqReal < m_VFLineFreq1)
		{
			m_LowFreq  = 0;	
			m_HighFreq = m_VFLineFreq1;
			m_LowVolt  = gVFPar.VFTorqueUp;	
			m_HighVolt = gVFPar.VFLineVolt1;
		}
		else if(gMainCmd.FreqReal < m_VFLineFreq2)
		{
			m_LowFreq  = m_VFLineFreq1;
			m_HighFreq = m_VFLineFreq2;
			m_LowVolt  = gVFPar.VFLineVolt1;
			m_HighVolt = gVFPar.VFLineVolt2;
		}
		else if(gMainCmd.FreqReal < m_VFLineFreq3)
		{
			m_LowFreq  = m_VFLineFreq2;
			m_HighFreq = m_VFLineFreq3;
			m_LowVolt  = gVFPar.VFLineVolt2;
			m_HighVolt = gVFPar.VFLineVolt3;
		}
		else
		{
			m_LowFreq  = m_VFLineFreq3;
			m_HighFreq = m_Frequency;//锟斤拷锟絍F锟斤拷锟竭碉拷锟斤拷锟揭伙拷锟斤拷缘锟斤拷锟斤拷锟斤拷锟斤拷尾
			m_LowVolt  = gVFPar.VFLineVolt3;
			m_HighVolt = 1000;
		}
		m_DetaFreq = gMainCmd.FreqReal - m_LowFreq;
		m_HighFreq = m_HighFreq - m_LowFreq;
		m_HighVolt = m_HighVolt - m_LowVolt;
		m_LowVolt  = (((long)m_LowVolt)<<12)/1000;
		m_HighVolt = (((long)m_HighVolt)<<12)/1000;	//锟矫段的碉拷压锟斤拷围

		mVolt = (((long)m_HighVolt * (long)m_DetaFreq)/m_HighFreq) + m_LowVolt;
		mVolt = (mVolt > 4096) ? 4096 : mVolt;
        return (mVolt);     // 锟斤拷锟絭f锟斤拷锟斤拷转锟斤拷锟斤拷锟斤拷
	}	
//平锟斤拷VF锟斤拷锟斤拷
	else if(gVFPar.VFLineType == 2)				
	{
		m_Freq2 = ((Ulong)m_AbsFreq * (Ulong)m_AbsFreq)>>15;
		m_MotorFreq2  = ((Ulong)gMotorInfo.FreqPer * (Ulong)gMotorInfo.FreqPer)>>15;
        
		mVolt = ((Ulong)m_Freq2 * 4096L)/m_MotorFreq2;
	}
// VF锟斤拷锟斤拷锟窖癸拷锟斤拷锟�
    else if(gVFPar.VFLineType == 10 || gVFPar.VFLineType == 11)
    {
        mVolt = ((Ulong)gOutVolt.vfSplit << 12) / gMotorInfo.Votage;
    }
    else  // 锟斤拷锟斤拷VF锟斤拷锟斤拷
    {
		mVolt = ((Ulong)m_AbsFreq * 4096L)/gMotorInfo.FreqPer;
	}


// 锟斤拷锟斤拷VF锟斤拷锟酵斤拷锟斤拷Vf锟斤拷锟竭的边界处锟斤拷
    
    if(m_AbsFreq == 0)          // 0频锟斤拷锟斤拷锟斤拷锟窖刮�0锟斤拷 VF锟斤拷锟斤拷也锟斤拷锟斤拷锟�
	{
		mVolt = 0;
		return (mVolt);		
	}
    else if(m_AbsFreq >= gMotorInfo.FreqPer)
	{
	    if(gVFPar.VFLineType < 10)                  // 锟斤拷锟斤拷锟斤拷坪愎︼拷锟斤拷锟�
        {   
		    mVolt = (mVolt > 4096) ? 4096 : mVolt;  
        }
        else// 锟斤拷然Vf锟斤拷锟斤拷锟绞憋拷锟斤拷锟斤拷锟斤拷锟�
        {
            mVolt = (mVolt < gOutVolt.MaxOutVolt) ? mVolt : gOutVolt.MaxOutVolt;
        }                        
		return (mVolt);		
	}	
    
    //转锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	//CalTorqueUp();		// generate gOutVolt.detVfVoltUp		
	//mVolt += gOutVolt.detVfVoltUp;
	return (mVolt);
}

/*************************************************************
	转锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷I*R锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷压锟斤拷锟斤拷锟斤拷锟斤拷压锟斤拷VF锟斤拷锟斤拷锟窖故革拷锟斤拷锟�
*************************************************************/
void CalTorqueUp(void)
{
	int m_DetaFreq, m_VoltUp, m_ZeroUp, m_MaxFreqUp;
    
// some case Vf_curve needn't torque up compensation
    if((gVFPar.VFLineType == 1) || (gVFPar.VFLineType >= 10) ||
        (gMainCmd.FreqSetApply == 0) ||
        (gMainStatus.RunStep == STATUS_GET_PAR))
    {
        gOutVolt.detVfVoltUp = 0;
    }
    else
    {
    m_VoltUp = 0;
	m_ZeroUp = (((Ulong)gVFPar.VFTorqueUp)<<12)/1000; //0Hz时锟斤拷锟斤拷锟斤拷锟斤拷锟窖�
	m_MaxFreqUp = (((Ulong)gVFPar.VFTorqueUpLim)<<15)/gBasePar.FullFreq; //锟斤拷么值锟斤拷示锟斤拷转锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷   
	m_DetaFreq = m_MaxFreqUp - abs(gMainCmd.FreqSyn);
    if(( m_DetaFreq > 0 ) && ( 0 == gExtendCmd.bit.SpeedSearch )) //转锟劫革拷锟斤拷时锟斤拷锟斤拷远锟阶拷锟斤拷锟斤拷?
    {
	    if((gVFPar.VFTorqueUp == 0)&&(gVFPar.VFOvShock == 0))		//锟皆讹拷转锟斤拷锟斤拷锟斤拷,锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷效 2011.5.14 L1082
	    {
            if( gMainCmd.FreqDesiredReal > 40 )  //锟借定频锟绞碉拷锟斤拷0.4HZ锟斤拷转锟斤拷锟斤拷锟斤拷锟斤拷效
			{
                gVFAutoVar.DestinationVolt = gOutVolt.Volt;
    	    	m_VoltUp = (int)(gVFAutoVar.AutoTorquePID.Out >>16);  //锟斤拷压锟斤拷锟斤拷锟酵猴拷0.5ms锟斤拷锟角凤拷锟斤拷锟接帮拷锟斤拷?
			}
            else
            {
                gVFAutoVar.VfTorqueEnableTime = 0;
                m_VoltUp = 0;
            }
	    }
    	else							//锟街讹拷锟斤拷锟斤拷锟斤拷
	    {          
           m_VoltUp = ((Ulong)m_DetaFreq * (Ulong)m_ZeroUp)/(Uint)m_MaxFreqUp; 
		}
      }  
    gOutVolt.detVfVoltUp = m_VoltUp; 
    }
    gOutVolt.Volt = gOutVolt.Volt + gOutVolt.detVfVoltUp;
}

/************************************************************
     VF锟皆讹拷转锟斤拷锟斤拷锟斤拷锟斤拷转锟筋补锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
************************************************************/
void VFWsTorqueBoostComm(void)
{
    int    m_CurrentPro,m_AngleCos,m_ResistanceVolt;
    int    m_RVcos;
    long   m_lTempVar;
    int    m_iTempVar1,m_iTempVar2;
    MT_STRUCT_Q24  m_NewMT;

    m_CurrentPro = gLineCur.CurPer;//gIAmpTheta.Amp    
    m_iTempVar1  = abs(gIMTQ12.T);
    gVFAutoVar.VfCurrentIs = Filter4(m_CurrentPro, gVFAutoVar.VfCurrentIs);

    if( m_iTempVar1 > gVFAutoVar.VfCurrentIs )
	{
        m_iTempVar1 = gVFAutoVar.VfCurrentIs;
	}
    m_AngleCos = (((long)m_iTempVar1)<<14) / gVFAutoVar.VfCurrentIs;  //锟斤拷锟姐功锟斤拷锟斤拷锟斤拷锟角碉拷锟斤拷锟斤拷
    m_lTempVar =  ( 0x4000L<<14) - (long)m_AngleCos * (long)m_AngleCos;  
    gVFAutoVar.VfAngleSin = qsqrt(m_lTempVar);  
    
    m_ResistanceVolt = ((Ulong)gMotorExtPer.R1 * (Ulong)gVFAutoVar.VfCurrentIs)>>16;  //锟斤拷锟姐定锟接碉拷锟斤拷锟较碉拷压锟斤拷,Q16锟斤拷示
    m_RVcos = ((long)m_ResistanceVolt * (long)m_AngleCos)>>14;
    gVFAutoVar.VfRIsSinFai = ((long)m_ResistanceVolt * (long)gVFAutoVar.VfAngleSin)>>14;
    gVFAutoVar.VfRVCosFai  = gOutVolt.VoltApply - 20 - m_RVcos; //锟斤拷锟斤拷锟窖癸拷锟矫粗碉拷锟饺�20锟斤拷锟斤拷为锟斤拷锟斤拷
    
    if(0 > gVFAutoVar.VfRVCosFai)
	{
        gVFAutoVar.VfRVCosFai = 0;
    }

    m_iTempVar2 = user_atan(gVFAutoVar.VfRVCosFai,gVFAutoVar.VfRIsSinFai);
    gVFAutoVar.VfReverseAngle = Filter128(m_iTempVar2,gVFAutoVar.VfReverseAngle);//锟斤拷锟斤拷锟窖癸拷锟斤拷锟斤拷锟�
    m_iTempVar2 = user_atan( abs(gIMTQ12.M), abs(gIMTQ12.T) );
    if( gVFAutoVar.VfReverseAngle > m_iTempVar2 )
	{
        gVFAutoVar.VfReverseAngle = m_iTempVar2;  //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷谴锟斤拷诠锟斤拷锟斤拷锟斤拷锟斤拷堑锟斤拷锟角ｏ拷锟斤拷使锟矫癸拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷谴锟斤拷锟�
	}

    gWsComp.Coff = gVFPar.VFWsComp;
    if( gWsComp.Coff > 1400 )
    {                           //转锟筋补锟斤拷锟斤拷锟斤拷锟斤拷锟�1400锟斤拷直锟斤拷锟斤拷MT锟斤拷锟斤拷锟斤拷锟斤拷锟阶拷畈癸拷锟斤拷锟�
        gWsComp.Coff = gVFPar.VFWsComp - 500;
        gWsComp.WsCompMT.M = abs( gIMTQ12.M );
        gWsComp.WsCompMT.T = abs( gIMTQ12.T );
    }
    else
    {                          //转锟筋补锟斤拷锟斤拷锟斤拷锟斤拷锟�1400锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷慕嵌锟斤拷锟斤拷录锟斤拷锟組T锟斤拷锟斤拷锟�
        m_iTempVar1 = gPhase.IMPhase + gVFAutoVar.VfReverseAngle; 
        if( gMainCmd.FreqSet < 0 )
		{
            m_iTempVar1 = gPhase.IMPhase - gVFAutoVar.VfReverseAngle;
        }
        
        AlphBetaToDQ((ALPHABETA_STRUCT *)&gIAlphBeta, m_iTempVar1, (MT_STRUCT_Q24 *)&m_NewMT);
        gWsComp.WsCompMT.T = Filter128( abs(m_NewMT.T>>12), gWsComp.WsCompMT.T);
        if( gLineCur.CurPer < 4096 )	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷诙疃拷锟斤拷锟斤拷锟斤拷锟斤拷偌锟斤拷锟斤拷锟斤拷诺锟斤拷锟�
        {       
            gWsComp.WsCompMT.M = Filter8( abs(m_NewMT.M>>12), gWsComp.WsCompMT.M);            
        }
    }
}

/************************************************************
锟斤拷锟斤拷锟斤拷锟斤拷:锟斤拷锟斤拷VF锟斤拷锟竭硷拷锟斤拷锟斤拷锟斤拷锟斤拷压锟斤拷Q12;MT锟斤拷锟斤拷浠伙拷锟侥碉拷锟斤拷,Q12
锟斤拷锟斤拷锟斤拷锟�:VF锟斤拷频锟斤拷压锟斤拷锟斤拷锟斤拷锟斤拷Q12锟斤拷式
锟斤拷锟斤拷位锟斤拷:锟斤拷锟斤拷状态锟斤拷2ms循锟斤拷
锟斤拷锟斤拷锟斤拷锟斤拷:
锟斤拷锟斤拷锟斤拷锟斤拷:VF锟皆讹拷转锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟捷碉拷锟斤拷锟斤拷锟斤拷锟窖癸拷锟斤拷锟斤拷锟�
************************************************************/
void VFAutoTorqueBoost(void)
{
    long  m_lTempVar1,m_lVoltDeta;
    int   m_iEsVolt,m_iEsnVolt,m_LowerLimit,m_TempVolt;
    
    m_lTempVar1  = (long)gVFAutoVar.VfRIsSinFai * (long)gVFAutoVar.VfRIsSinFai;
    m_lTempVar1 += (long)gVFAutoVar.VfRVCosFai * (long)gVFAutoVar.VfRVCosFai;
    m_iEsVolt    = qsqrt(m_lTempVar1);   //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷压锟斤拷去锟斤拷锟接碉拷锟斤拷压锟斤拷锟斤拷锟斤拷锟揭ｏ拷锟诫定锟接碉拷锟斤拷压锟斤拷锟斤拷锟斤拷锟揭碉拷平锟斤拷锟斤拷
    m_iEsnVolt   = ((long)gOutVolt.VoltApply * (long)gVFAutoVar.VfAngleSin)>>14;//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟窖癸拷锟斤拷怨锟斤拷锟斤拷锟斤拷锟斤拷堑锟斤拷锟斤拷锟�   				
    if(15 >= gInvInfo.InvTypeApply)        m_LowerLimit = 50;//锟斤拷锟捷伙拷锟酵硷拷锟斤拷PI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟睫ｏ拷锟斤拷锟斤拷锟金动伙拷锟斤拷锟斤拷
    else if(27 >= gInvInfo.InvTypeApply)   m_LowerLimit = 35;
    else                                     m_LowerLimit = 0;

    m_LowerLimit = (long)(1000 - gMainCmd.FreqReal) * (long)m_LowerLimit / 1000; //锟斤拷锟斤拷锟街讹拷锟斤拷锟斤拷锟斤拷锟斤拷频锟绞硷拷锟斤拷锟接︼拷锟斤拷蓿锟斤拷锟斤拷锟狡碉拷锟斤拷锟轿拷锟�10HZ

    m_TempVolt = m_iEsnVolt;  			//锟斤拷锟捷碉拷前锟斤拷锟斤拷频锟绞和碉拷锟借，锟斤拷锟斤拷锟斤拷锟界动锟狡硷拷锟姐方式
    if(27 < gInvInfo.InvTypeApply) m_TempVolt = m_iEsVolt;  //锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷一锟斤拷锟斤拷锟斤拷锟斤拷值锟杰达拷锟斤拷锟阶憋拷锟斤拷锟斤拷 
    
    if(gMainCmd.FreqReal > 150)         //  1.5Hz锟斤拷锟斤拷, 锟斤拷锟斤拷为锟姐；
    {
        m_LowerLimit = 0;				
        m_TempVolt = m_iEsVolt;

        if(gMainCmd.FreqReal < 2000)	//锟斤拷锟斤拷1.5HZ,小锟斤拷20HZ
        {       
            m_TempVolt = m_iEsnVolt;
            if(gMotorExtInfo.R1 < (2000 * gMotorExtInfo.UnitCoff))//锟斤拷锟接碉拷锟斤拷小锟斤拷2欧姆
            {   
                m_TempVolt = m_iEsVolt;
                if(gMainCmd.FreqReal < 950)//锟斤拷锟接碉拷锟斤拷小锟斤拷2欧姆锟斤拷锟斤拷锟斤拷频锟绞碉拷锟斤拷9.5Hz
                {   
                    m_TempVolt = (m_iEsnVolt + m_iEsVolt) >>1;
                }            
            }
        }
    }
    
    if((gMotorExtInfo.R1 < (2000 * gMotorExtInfo.UnitCoff)) && (350 > gVFAutoVar.VfTorqueEnableTime))             
    {      
        gVFAutoVar.VfTorqueEnableTime++;
        m_TempVolt = m_iEsVolt;
    }
    m_lVoltDeta = ((long)(gVFAutoVar.DestinationVolt - m_TempVolt))<<5; 	// 锟斤拷锟解精锟斤拷锟斤拷失锟斤拷锟斤拷Q5锟斤拷示偏锟斤拷
    m_TempVolt = __IQsat(m_lVoltDeta,32767,-32767);
    gVFAutoVar.VfReverseVolt = Filter32(m_TempVolt, gVFAutoVar.VfReverseVolt);
    
    if(19 < gInvInfo.InvTypeApply)
    {
       gVFAutoVar.AutoTorquePID.Deta = (long)gVFAutoVar.VfReverseVolt>>5;
    }
    else if(13 < gInvInfo.InvTypeApply)
    {
       gVFAutoVar.AutoTorquePID.Deta = ((long)gVFAutoVar.VfReverseVolt>>3) & 0xFFFC;
    }
    else
    {
       gVFAutoVar.AutoTorquePID.Deta = ((long)gVFAutoVar.VfReverseVolt>>1) & 0xFFF0;
    }
    gVFAutoVar.AutoTorquePID.Max  = 1228 ; 			//30% Torque Boost
    gVFAutoVar.AutoTorquePID.Min  = m_LowerLimit;
    gVFAutoVar.AutoTorquePID.KP   = 56;  		//PID锟斤拷锟斤拷实锟斤拷锟斤拷锟斤拷锟较版本锟斤拷锟斤拷锟斤拷同
    gVFAutoVar.AutoTorquePID.KI   = 49;
    gVFAutoVar.AutoTorquePID.QP = 0;
    gVFAutoVar.AutoTorquePID.QI = 0;
    PID((PID_STRUCT *)&gVFAutoVar.AutoTorquePID); 
 }


/************************************************************
	锟斤拷锟斤拷锟今荡达拷锟斤拷
************************************************************/
/*void OverShockControl(void)
{
    int m_DelayTime,m_DestCurrent;
    int gainOsc;

    gainOsc = gVFPar.VFOvShock;
    gVfOsc.IMFilter = gIMTQ12.M;
    
    gVfOsc.TimesNub++;
    if( 2000 < gVfOsc.TimesNub )
    {
        gVfOsc.TimesNub = 2001;
    }

//锟斤拷锟斤拷锟斤拷模式为0锟斤拷2时锟斤拷DPWM锟斤拷锟斤拷时取锟斤拷锟斤拷锟斤拷锟斤拷
    if((gVfOsc.oscMode == 0) && (MODLE_DPWM == gPWM.PWMModle)) 
    {
        gVfOsc.ShockDecrease++;
        if(800 < gVfOsc.ShockDecrease)
        {
            gVfOsc.ShockDecrease = 808;
        }
        
        m_DelayTime = gVfOsc.ShockDecrease>>3;
        
        if(gVFPar.VFOvShock <= m_DelayTime)
        {
            gainOsc = 0;
        }
        else
        {
            gainOsc = gVFPar.VFOvShock - m_DelayTime;
        }
    }
    else
    {
        gVfOsc.ShockDecrease = 0;
    }

// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	if((gainOsc == 0) || (gMainCmd.FreqReal <= 50) )//|| speed_DEC) // dec    // 0.50Hz
	{										
		gVfOsc.pid.Out -= gVfOsc.pid.Out>>5;
		gVfOsc.pid.Total = 0;
		return;
	}
    gVfOsc.pid.KP   = (gainOsc<<3);     // 280xp锟芥本锟斤拷锟斤拷锟斤拷7位锟斤拷 锟斤拷锟斤拷PID锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷4位
	gVfOsc.pid.KI   = 0;			    //没锟叫伙拷锟斤拷
	gVfOsc.pid.KD   = 0;
    gVfOsc.pid.QP = 0;
    gVfOsc.pid.QI = 0;
	gVfOsc.pid.Max  = (gainOsc<<2)+100;
	gVfOsc.pid.Min  = -gVfOsc.pid.Max;
    if(gVfOsc.TimesNub < 1500) //锟斤拷锟斤拷锟斤拷实锟斤拷锟斤拷锟斤拷锟绞憋拷锟斤拷锟绞硷拷锟角段伙拷锟斤拷锟�
    {
        gVfOsc.IO = Filter256(gMotorExtPer.I0, gVfOsc.IO);
    }
//锟斤拷锟斤拷锟斤拷模式为1锟斤拷2锟斤拷3时锟斤拷锟斤拷锟斤拷锟借定锟侥匡拷锟截碉拷锟斤拷
    else// if(( 1500 < gVfOsc.TimesNub ) && (gVfOsc.oscMode >= 1))
    {
        gVfOsc.IO = gMotorExtPer.I0;
    }

	gVfOsc.pid.Deta = gVfOsc.IO - gVfOsc.IMFilter;    
	PID((PID_STRUCT *)&gVfOsc.pid);
    gVfOsc.OscVolt = gVfOsc.pid.Out >> 16; 
	gOutVolt.Volt += gVfOsc.OscVolt;
    if(gVFPar.FreqApply == 0)       // 锟斤拷锟狡碉拷锟轿�0, 锟斤拷锟斤拷锟斤拷转锟筋补锟斤拷
    {
        gOutVolt.Volt  = 0;         // disabel torque-up part and osc-damp part
    }
	gOutVolt.Volt = __IQsat(gOutVolt.Volt, 32767, 0);
    
}
*/
/************************************************************
	转锟筋补锟斤拷锟斤拷锟斤拷
************************************************************/
void VFWSCompControl(void)
{
    int m_WsWindage, m_WsCurrentIm;

    m_WsWindage = 0;
    if(( gMainCmd.FreqDesiredReal < 40  ) ||
        ( 16 < gInvInfo.InvTypeApply)) 	//锟借定频锟斤拷小锟斤拷0.4Hz,锟斤拷锟竭伙拷锟酵达拷?6
 	{	
        gWsComp.CompFreq = 0;
        gMainCmd.FreqSyn = gMainCmd.FreqSetApply;
        return;
	}

    if(speed_CON && (0 != gVFPar.VFWsComp)) 
    {
        if( gWsComp.DelayTime > 100 )
        {
            gWsComp.WsCompMTApply.T = abs(gWsComp.WsCompMT.T);

            if(( 0 == gVFPar.VFOvShock ) && ( 20 <= gBasePar.FcSetApply ))
            {
                gWsComp.WsCompMTApply.M = Filter16(gWsComp.WsCompMT.M, gWsComp.WsCompMTApply.M);
                m_WsCurrentIm = abs(gWsComp.WsCompMTApply.M);
            }
            else
    		{
                m_WsCurrentIm = gMotorExtPer.I0;  //锟斤拷锟斤拷锟斤拷锟斤拷效锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷2K锟斤拷锟斤拷锟脚碉拷锟斤拷使锟矫匡拷锟截碉拷锟斤拷
            }

            if( 0 == m_WsCurrentIm )	m_WsCurrentIm = 1;
                   
            m_WsWindage = (3038L) * (long)gWsComp.WsCompMTApply.T / (long)m_WsCurrentIm;
            m_WsWindage = (long)m_WsWindage * (long)gWsComp.Coff / 1000L;
            if( m_WsWindage > 0x2000 )	m_WsWindage = 0x2000; 
            if( m_WsWindage < 0 )		m_WsWindage = 0;//锟斤拷蟛钩锟斤拷锟斤拷锟斤拷疃ㄗ拷锟�
                    
            m_WsWindage = ( (long)m_WsWindage * (long)gMotorExtPer.RatedComp )>>12;
        }
        else	
        {
        	gWsComp.DelayTime++;   //锟斤拷态锟斤拷锟斤拷要锟斤拷时0.2s锟斤拷始转锟筋补锟斤拷 
	    }
                      
    }
    else
    {
    	gWsComp.DelayTime = 0;  //锟接硷拷锟劫癸拷锟斤拷锟叫ｏ拷锟斤拷锟斤拷转锟筋补锟斤拷
    }

    gWsComp.FilterCoff = 100;    //200ms锟剿诧拷
    if(( (gWsComp.CompFreq>>16) + 2 ) < m_WsWindage ) //锟剿诧拷锟斤拷锟斤拷偏锟斤拷为2
    {
        gWsComp.CompFreq = (( (long)m_WsWindage <<16 ) - gWsComp.CompFreq ) / (long)( gWsComp.FilterCoff + 1 ) + gWsComp.CompFreq;
    }
    else if(( (gWsComp.CompFreq>>16) - 2 ) >m_WsWindage )
    {
       gWsComp.CompFreq = ( (long long)gWsComp.CompFreq - ( (long long)m_WsWindage <<16 )) * (long long)gWsComp.FilterCoff / 
                             (long)( gWsComp.FilterCoff + 1) + ((long)m_WsWindage <<16);
    }
    else
    {
       gWsComp.CompFreq = ((long)m_WsWindage)<<16;
    }
}

/************************************************************
VF锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	锟斤拷锟劫癸拷锟斤拷锟叫ｏ拷锟窖撅拷锟斤拷锟斤拷锟斤拷锟斤拷锟较碉拷锟斤拷螅锟斤拷锟侥革拷叩锟窖癸拷糯锟斤拷锟斤拷系锟斤拷
锟斤拷锟斤拷锟斤拷锟�: 锟斤拷锟斤拷系锟斤拷 gRatio;
锟斤拷锟斤拷锟斤拷锟�: 锟斤拷锟斤拷系锟斤拷 gRatio;
************************************************************/
void VFOverMagneticControl()
{
	Uint    m_UDCDesired;
    Uint    m_AVR;
    //Uint    m_Ratio;

    if((gMainStatus.RunStep == STATUS_SPEED_CHECK) ||   // 转锟斤拷追锟劫诧拷锟斤拷要锟斤拷锟斤拷锟斤拷
        (gVFPar.VFLineType >= 10))                      // Vf锟斤拷锟诫不锟斤拷要锟斤拷锟斤拷锟斤拷
    {
        gOutVolt.VoltApply = gOutVolt.Volt;
        return;
    }
    
// 锟斤拷锟劫癸拷锟斤拷锟叫的癸拷锟斤拷锟脚达拷锟斤拷
	m_UDCDesired = gInvInfo.BaseUdc;
	if(speed_DEC)
	{
		m_UDCDesired = gUDC.uDC;
	}

	if(abs(gVarAvr.UDCFilter - m_UDCDesired) < 10)
	{
		gVarAvr.UDCFilter = m_UDCDesired;
	}
	else if(gVarAvr.UDCFilter > m_UDCDesired)
	{
		gVarAvr.UDCFilter --;
	}
	else
	{
		gVarAvr.UDCFilter += 3;		
		//gVarAvr.UDCFilter = Filter4(m_UDCDesired, gVarAvr.UDCFilter) + 10;
	}
	gVarAvr.UDCFilter = (gVarAvr.UDCFilter < gInvInfo.BaseUdc)?gInvInfo.BaseUdc:gVarAvr.UDCFilter;
    //gVarAvr.UDCFilter = __IQsat(gVarAvr.UDCFilter, 32767, gInvInfo.BaseUdc);
    
	m_AVR = (((long)gVarAvr.UDCFilter)<<12)/gInvInfo.BaseUdc - 4096;
	m_AVR = ((long)m_AVR * (long)gVarAvr.CoffApply)>>6;
    gOutVolt.VoltApply = ((long)m_AVR + 4096L) * gOutVolt.Volt>> 12;
}

/************************************************************
VF 锟斤拷锟狡癸拷锟斤拷:
    频锟斤拷锟斤拷锟斤拷:   gMainCmd.FreqSetApply
    锟斤拷压锟斤拷锟斤拷:   gOutVolt.Volt
************************************************************/
void VFSpeedControl()
{
    int slipComp;
//    int mVolt;

//锟斤拷锟斤拷VF锟斤拷锟狡碉拷剩锟� 转锟筋补锟斤拷
    slipComp = gWsComp.CompFreq >>16;
    slipComp = (gMainCmd.FreqSet > 0) ? slipComp : (-slipComp);
    gMainCmd.FreqSyn = gVFPar.FreqApply + slipComp;
    
//锟斤拷锟斤拷VF锟斤拷锟斤拷锟窖�
    gOutVolt.Volt = CalOutVotInVFStatus(gMainCmd.FreqSyn);
    if(gMainCmd.FreqSetApply == 0)
    {
        gOutVolt.Volt = 0;
    }    
 
}

    
// 锟斤拷锟斤拷280xp锟斤拷失锟劫匡拷锟斤拷
void VfOverCurDeal()
{
	int  m_Cur,m_DetaCur,m_Coff;
	int	 m_LimHigh,m_LimLow,m_MaxStep,m_AddStep;
	int	 m_Data;

	//锟斤拷锟斤拷锟斤拷前状态锟叫讹拷
	m_Cur = gLineCur.CurBaseInv;
	m_Coff = gVFPar.ocGain;

    // 锟叫断加硷拷锟劫碉拷一锟斤拷
    gVfFreq2.spedChg = (gVfFreq2.preSpdFlag != gMainCmd.Command.bit.SpeedFlag) ? 1 : 0;
    gVfFreq2.preSpdFlag = gMainCmd.Command.bit.SpeedFlag; 

	//if(0x8000 == (gMainCmd.SpeedFalg & 0x8000))	//锟接硷拷锟劫憋拷志锟侥憋拷牡锟揭伙拷锟�
	if(gVfFreq2.spedChg)
	{
		//gMainCmd.SpeedFalg &= 0x7FFF;
		gOvCur.Flag = 0;
		gOvCur.StepLowLim = 1;
		gOvCur.StepLow = 1;
		gOvCur.StepHigh = 0;
		//if((m_Coff < 10) && ((gMainCmd.SpeedFalg & 0x7FFF) != C_SPEED_FLAG_CON))
		if((m_Coff < 10) && (!speed_CON))
		{
			gOvCur.StepLowLim = (20 - m_Coff)<<3;
			gOvCur.StepLow = gOvCur.StepLowLim;
		}
	}

	m_DetaCur = m_Cur - gOvCur.CurBak;
	gOvCur.LowFreq = (((long)(m_Coff + 50) * 10)<<15)/gBasePar.FullFreq01;
	if(m_Coff < 20)		
	{
		gOvCur.SubStep = 1;
		m_AddStep = (20 - m_Coff);//>>1;				//锟斤拷锟斤拷锟斤拷锟斤拷小锟斤拷20锟斤拷锟斤拷小锟斤拷锟皆革拷锟斤拷
	}
	else
	{
		gOvCur.SubStep = 2;
		m_AddStep = 1;
	}

	if(m_Coff < 40)
	{
		gOvCur.MaxStepLow = 10000 / 6 / m_Coff;
		gOvCur.MaxStepHigh = 10000;
	}
	else
	{
		m_Data = (13200-100 * m_Coff);
		gOvCur.MaxStepLow = m_Data/ 6 / m_Coff + 10;
		gOvCur.MaxStepHigh = m_Data>>4;
	}
    if(15 == gInvInfo.InvTypeApply) /*15KW锟斤拷锟斤拷锟斤拷锟截憋拷锟斤拷锟斤拷锟斤拷锟狡碉拷锟斤拷*2011.5.8 L1082*/
     {
       gOvCur.CurLim = ((long)170L <<12) / 100 ;
     }
    else
     {
       gOvCur.CurLim = ((long)gVFPar.ocPoint <<12) / 100 ;
      }
    
	m_LimLow  = ((long)gOvCur.CurLim * 3605L)>>12;
	m_LimHigh = ((long)gOvCur.CurLim * 3891L)>>12;

	if(m_Cur < m_LimLow)					//小锟斤拷0.88锟斤拷锟斤拷锟斤拷
	{
		gOvCur.StepHigh = 0;
		if((gOvCur.Flag & 0x01) == 0)
		{
			gOvCur.StepLow += m_AddStep;
			gOvCur.StepLowLim += m_AddStep;
		}
		if(abs(gMainCmd.FreqSyn) < gOvCur.LowFreq)	
		{
			m_MaxStep = gOvCur.MaxStepLow;
		}
		else
		{
			m_MaxStep = gOvCur.MaxStepHigh;
		}
		if(gOvCur.StepLowLim > m_MaxStep)
		{
			gOvCur.StepLowLim -= (gOvCur.StepLowLim - m_MaxStep)>>4;
		}
		//gOvCur.StepLowLim = (gOvCur.StepLowLim > m_MaxStep)?m_MaxStep:gOvCur.StepLowLim;
		gOvCur.StepLow = (gOvCur.StepLow > gOvCur.StepLowLim)?gOvCur.StepLowLim:gOvCur.StepLow;

		gOvCur.StepApply = gOvCur.StepLow;
		gOvCur.Flag &= 0x7FFF;				//锟斤拷锟斤拷小锟斤拷0.88锟斤拷锟斤拷锟斤拷锟街�
	}
	else if(m_Cur < m_LimHigh)				//0.88锟斤拷0.95锟斤拷锟斤拷锟斤拷之锟斤拷
	{
		if(m_DetaCur < 0)	
		{
			gOvCur.StepHigh --;
			gOvCur.StepHigh = (gOvCur.StepHigh<-5000)?-5000:gOvCur.StepHigh;
		}
		if(gOvCur.StepHigh >= 0)
		{
			gOvCur.StepLow -= gOvCur.SubStep;
			gOvCur.StepLow = (gOvCur.StepLow < 0)?0:gOvCur.StepLow;
		}
		else
		{
			gOvCur.StepLow = (gOvCur.StepLow < (-gOvCur.StepHigh))?(-gOvCur.StepHigh):gOvCur.StepLow;
		}
		gOvCur.StepApply = -gOvCur.StepHigh;
		gOvCur.Flag |= 0x8000;				//锟斤拷锟斤拷锟斤拷锟斤拷0.88锟斤拷锟斤拷锟斤拷锟街�
	}
	else									//锟斤拷锟斤拷0.95锟斤拷锟斤拷锟斤拷
	{
		gOvCur.StepLow -= gOvCur.SubStep;
		gOvCur.StepLow = (gOvCur.StepLow < 0)?0:gOvCur.StepLow;
		if(m_DetaCur < 0)	gOvCur.StepHigh = 0;
		else
		{
			gOvCur.StepHigh += (4 + (m_Coff>>4));
			gOvCur.StepHigh = (gOvCur.StepHigh>10000)?10000:gOvCur.StepHigh;
		}

		gOvCur.StepApply = -gOvCur.StepHigh;
		gOvCur.Flag |= 0x8000;				//锟斤拷锟斤拷锟斤拷锟斤拷0.88锟斤拷锟斤拷锟斤拷锟街�
	}
	gOvCur.CurBak = m_Cur;
	gOvCur.Flag &= 0xFFFE;					//锟斤拷锟斤拷锟斤拷锟教拷锟斤拷志

}

/*void VfOverUdcDeal()
{
	int m_High,m_Mid,m_Low,m_Coff;
	int	m_Udc,m_DetaU,m_Add1,m_Add2;

	m_Udc = gUDC.uDC;
	m_Coff = gOvUdc.CoffApply;


	//..锟接硷拷锟劫憋拷志锟侥憋拷牡锟揭伙拷锟�
	//if((0x8000 == (gMainCmd.SpeedFalg & 0x8000)) ||
	//   ((gMainCmd.SpeedFalg & 0x7FFF) != C_SPEED_FLAG_DEC) ||
	//   (m_Coff == 0))
	if((!speed_DEC) ||     // acc or cons
	    (gVfFreq2.spedChg) ||               // 1st step of acc or dec
	    (m_Coff == 0))
	{
		//gMainCmd.SpeedFalg &= 0x7FFF;
		gOvUdc.StepApply = 0;
        gOvUdc.LastStepApply = 0;
        gOvUdc.AccTimes = 0;
		gOvUdc.Flag = 0;
		gOvUdc.StepBak = 0;
		gOvUdc.ExeCnt = m_Coff;
		gOvUdc.UdcBak = m_Udc;
		gOvUdc.CoffApply = gVFPar.ovGain;
		gOvUdc.FreqMax = gVFPar.FreqApply;
		gOvUdc.OvUdcLimitTime = 0;
		return;
	}

	if(abs(gVFPar.FreqApply) > abs(gOvUdc.FreqMax))
	{
		gVFPar.FreqApply = gOvUdc.FreqMax;
		gOvUdc.StepApply = 0;
		gOvUdc.Flag = 0;
		gOvUdc.StepBak = 0;
		return;
	}	

    
	m_High = gOvUdc.Limit;
	m_Mid = ((Ulong)gOvUdc.Limit * 3932L)>>12;			//0.96
	//if(gInvInfo.InvTypeApply < MAX_220V_INV)	m_Low = 3522;
	//else										m_Low = 6100;
	m_Low = 6100;//锟斤拷要锟睫改ｏ拷锟剿达拷锟斤拷默锟斤拷锟斤拷值只锟斤拷锟斤拷锟�380V锟斤拷压锟饺硷拷锟斤拷锟斤拷锟斤拷锟斤拷压锟饺硷拷?

    // if((m_Udc <= m_Mid) && ((gOvUdc.Flag & 0x01) == 0x01))	
    /*if((m_Udc <= m_Mid) || ((gOvUdc.Flag & 0x01) == 0x01))	
    {
        gOvUdc.ExeCnt = 200;
    }*/
/*    if((m_Udc <= m_Mid) && ((gOvUdc.Flag & 0x01) == 0x01))	
	{
		gOvUdc.StepApply = gOvUdc.StepBak;
		gOvUdc.Flag &= 0xFFFE;					//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷玫谋锟街�
		return;
	}

	gOvUdc.ExeCnt += 6;
	if(gOvUdc.ExeCnt < m_Coff)		
	{
		gOvUdc.StepApply = 0;
		//gOvUdc.Flag &= 0xFFFE;					//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷玫谋锟街�
		return;
	}
    else 
    {
        gOvUdc.ExeCnt = 0;
    }
	m_DetaU = m_Udc - gOvUdc.UdcBak;
	gOvUdc.UdcBak = m_Udc;

	if((m_Udc <= m_Mid) && ((gOvUdc.Flag & 0x01) == 0x01))	
	{
		gOvUdc.StepApply = gOvUdc.StepBak;
		gOvUdc.Flag &= 0xFFFE;					//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷玫谋锟街�
		return;
	}
    
	m_Add2 = 2;
	m_Add1 = 1;
	if(m_Coff < 10)	
	{
		m_Add1 = 1 + (10 - m_Coff);
		m_Add2 = m_Add1<<1;
	}

	if(m_Udc <= m_Low)
	{
	    gOvUdc.FreqMax = gVFPar.FreqApply;
		if(m_DetaU < 0)			gOvUdc.StepBak += m_Add2;
		else					gOvUdc.StepBak += m_Add1;
	}
	else if(m_Udc <= m_Mid)
	{
		if(m_DetaU < 0)			gOvUdc.StepBak += m_Add1;
	}
	else if(m_Udc <= m_High)
	{
		if(m_DetaU > 5)			
		{
			gOvUdc.StepBak -= 1;
			gOvUdc.Flag |= 0x02;
			gOvUdc.CoffAdd++;				//锟斤拷锟斤拷锟斤拷压锟斤拷值时准锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
		}
		else if(m_DetaU < -5)	gOvUdc.StepBak += m_Add1;
	}
	else
	{
		if(m_DetaU > 3)			
		{
			gOvUdc.StepBak -= 1;
			gOvUdc.Flag |= 0x02;
			gOvUdc.CoffAdd += 2;				//锟斤拷锟斤拷锟斤拷压锟斤拷值时准锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
		}
	}
	gOvUdc.CoffAdd = (gOvUdc.CoffAdd > 100)?100:gOvUdc.CoffAdd;
	gOvUdc.StepBak = (gOvUdc.StepBak > 20000)?20000:gOvUdc.StepBak;
	gOvUdc.StepBak = (gOvUdc.StepBak < -20000)?-20000:gOvUdc.StepBak;

	gOvUdc.StepApply = gOvUdc.StepBak;
	if(((gOvUdc.Flag & 0x02) == 0) &&		//锟斤拷压锟斤拷锟酵猴拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
	   (gOvUdc.CoffAdd != 0) && 
	   (gVFPar.ovGain >= 10))		//锟斤拷压锟斤拷锟斤拷锟斤拷锟斤拷小锟斤拷10锟斤拷锟斤拷锟斤拷
	{	
		gOvUdc.CoffApply += 6;
		gOvUdc.CoffApply = (gOvUdc.CoffApply > 200)?200:gOvUdc.CoffApply;
		gOvUdc.CoffAdd -= 6;
		//gOvUdc.CoffAdd --;
		gOvUdc.CoffAdd = (gOvUdc.CoffAdd < 0)?0:gOvUdc.CoffAdd;
	}
	gOvUdc.Flag &= 0xFFFC;					//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷玫谋锟街�
}*/

void VfFreqDeal()
{
	int  m_Deta,m_Step,m_StepOI,m_StepOU,m_Freq,m_Dir;//m_Flag;
	int VfStepSet;

	VFOVUdcLimit();
	VfStepSet = abs((int)abs(gMainCmd.FreqSet) - (int)abs(gVFPar.FreqApply));    
	m_StepOI = gOvCur.StepApply;		
	m_StepOU = gOvUdc.StepApply;	
    
	//m_Flag = (gMainCmd.SpeedFalg & 0x7FFF);
    //if(C_SPEED_FLAG_DEC == m_Flag)
    if(speed_DEC)
    {
       m_StepOI = (m_StepOI>1)?m_StepOI:1;  //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟睫凤拷停锟斤拷锟斤拷锟斤拷锟斤拷
       if(0 != gOvUdc.StepApply)
       {
            if( (((long)gOvUdc.LastStepApply * (long)gOvUdc.StepApply) < 0) &&
                (gOvUdc.StepApply < 0))
                gOvUdc.AccTimes++;
            if( 10 < gOvUdc.AccTimes )
            {
                gOvUdc.OvUdcLimitTime = 6000;
                gOvUdc.AccTimes = 11;
            }
            gOvUdc.LastStepApply = gOvUdc.StepApply;
       }
    }
	if(gVFPar.ocGain == 0)		m_StepOI = 32767;
	if((gVFPar.ovGain == 0) || 
	   (gOvUdc.OvUdcLimitTime > 5000) ||
	   (!speed_DEC) ||
	   (abs(gVFPar.FreqApply) - abs(gMainCmd.FreqDesired) < 37))
	{
		m_StepOU = 32767;		
	}

	if(m_StepOI < m_StepOU)	
	{
		m_Step = m_StepOI;
 		gOvUdc.Flag |= 1;
	}
	else
	{
		m_Step = m_StepOU;
		gOvCur.Flag |= 1;
	}

	if(m_Step == 32767)
	{
		gVFPar.FreqApply = gMainCmd.FreqSet;
		gOvCur.Flag |= 1;
		gOvUdc.Flag |= 1;
		return;		
	}

    if((speed_DEC) && (VfStepSet <= m_StepOU))
    {
        gOvUdc.StepBak = VfStepSet;
        gOvUdc.StepApply = VfStepSet;
        gOvUdc.Flag |= 1;
    }
    
	if(speed_DEC)
	{
		m_Step = - m_Step;
	}
	m_Freq = abs(gVFPar.FreqApply) + m_Step;


	m_Dir = gMainCmd.FreqSet;
	if(speed_DEC)
	{
		m_Dir = gVFPar.FreqApply;
	}
	if(m_Dir < 0)	m_Freq = - m_Freq;
	
	if(((long)m_Freq * (long)gVFPar.FreqApply) < 0)	m_Freq = 0;

	m_Deta = abs(m_Freq) - abs(gMainCmd.FreqSet);
	if(speed_DEC)
	{
		m_Deta = - m_Deta;
		if(gVFPar.FreqApply == 0)		m_Deta = 0;		//锟斤拷图锟斤拷俚锟�0
	}
	if(m_Deta >= 0)	
	{
		gVFPar.FreqApply = gMainCmd.FreqSet;
		gOvCur.Flag |= 1;
		gOvUdc.Flag |= 1;
	}
	else
	{
		gVFPar.FreqApply = m_Freq;
	}
}

/************************************************************
HVF:
    锟斤拷锟斤拷锟斤拷系锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷锟今荡程度ｏ拷
************************************************************/
/*void VfOscIndexCalc()
{
    int curPhase;
    
    if((gCtrMotorType != ASYNC_VF) ||
        (gMainCmd.Command.bit.Start == 0))
    {
        gHVfOscIndex.oscIndex = 0;
        return;
    }

    gHVfOscIndex.AnglePowerFactor = Filter4(abs(gIAmpTheta.PowerAngle), gHVfOscIndex.AnglePowerFactor);
    //gHVfOscIndex.wCntRltm = 1000L*100L*2L / freqRun;        // active window, 2* T_run
    gHVfOscIndex.wCntRltm = 1000L*100L*2L / gMainCmd.FreqReal;
    curPhase = gHVfOscIndex.AnglePowerFactor;
    
    gHVfOscIndex.wCntUse ++;    
    if(gHVfOscIndex.wCntUse < gHVfOscIndex.wCntRltm)
    {
        gHVfOscIndex.maxAngle = (gHVfOscIndex.maxAngle < curPhase) ? curPhase : gHVfOscIndex.maxAngle;
        gHVfOscIndex.minAngle = (gHVfOscIndex.minAngle > curPhase) ? curPhase : gHVfOscIndex.minAngle;
    }
    else                            // update osc-index
    {
        gHVfOscIndex.wCntUse = 0;
        gHVfOscIndex.oscIndex = (long)(gHVfOscIndex.maxAngle - gHVfOscIndex.minAngle) *180L >>15;        // diff(phi) / 90deg * 100%

        gHVfOscIndex.maxAngle = curPhase;
        gHVfOscIndex.minAngle = curPhase;
    }
    
}*/

// 锟届步锟斤拷 HVf 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
void HVfDeadBandComp()
{
	int   phase,m_Com;
	long  tempL;
    int   tempSect;

    int   phase_sect;
    int   phase_sect_pre;
    int   temp;

// 确锟斤拷锟斤拷锟斤拷锟斤拷
	if(gMainCmd.FreqReal <= 40000)      m_Com = gDeadBand.Comp;
	else    m_Com = (int)(((long)gDeadBand.Comp * (long)(gMainCmd.FreqReal - 40000))>>15); 
	if((gMainCmd.Command.bit.StartDC == 1) || (gMainCmd.Command.bit.StopDC == 1))	
	{
        m_Com = 0;
	}
    
// 锟叫断碉拷锟斤拷锟斤拷锟斤拷
    gIAmpTheta.ThetaFilter = gIAmpTheta.Theta;

    gHVfDeadBandCompOpt.CurPhaseFeed_pre = gHVfDeadBandCompOpt.CurPhaseFeed;

    if(gMainCmd.Command.bit.ControlMode != IDC_SVC_CTL)
	{
	    gHVfDeadBandCompOpt.CurPhaseFeed = gPhase.IMPhase + gIAmpTheta.ThetaFilter + gPhase.CompPhase + 16384;
	}
	else              //SVC时取锟斤拷锟斤拷位锟斤拷锟斤拷
	{
	    gHVfDeadBandCompOpt.CurPhaseFeed = gPhase.IMPhase + gIAmpTheta.ThetaFilter  + 16384;
	}
    gHVfDeadBandCompOpt.CurPhaseStepFed = gHVfDeadBandCompOpt.CurPhaseFeed - gHVfDeadBandCompOpt.CurPhaseFeed_pre;// pos or neg

    if(gMainCmd.FreqReal <= 150 || gMainCmd.FreqReal >= gHVfDeadBandCompOpt.DbOptActHFreq) // 1.50Hz, 12.00Hz
    {
        gHVfDeadBandCompOpt.PhaseFwdFedCoeff = 0;    //__IQsat(gTestDataReceive.TestData6, 128, 0);           // filter coeff
    }
    else if(gMainCmd.FreqReal < 800)    // 8.00Hz
    {
        gHVfDeadBandCompOpt.PhaseFwdFedCoeff = 100;
    }
    else if(gMainCmd.FreqReal < 1200)   // 12.00Hz
    {
        temp = gHVfDeadBandCompOpt.DbOptActHFreq - gHVfDeadBandCompOpt.DbOptActLFreq + 1;     
        tempL = gHVfDeadBandCompOpt.DbOptActHFreq - gMainCmd.FreqReal;
        gHVfDeadBandCompOpt.PhaseFwdFedCoeff = (long)100L * tempL / temp;
    }

    gHVfDeadBandCompOpt.StepPhaseSet = gPhase.StepPhase >> 16;
    
    gHVfDeadBandCompOpt.PhaseFwdFedCoeff = 100;
    tempL = (long)gHVfDeadBandCompOpt.CurPhaseStepFed * (128 -gHVfDeadBandCompOpt.PhaseFwdFedCoeff);
    tempL += (long)gHVfDeadBandCompOpt.StepPhaseSet * gHVfDeadBandCompOpt.PhaseFwdFedCoeff;
    gHVfDeadBandCompOpt.CurPhaseStepPredict = tempL >> 7;
    
    tempSect = gHVfDeadBandCompOpt.CurPhaseFeed / 10922;        // 60deg
    tempSect = __IQsat(tempSect, 5, 0);
    phase_sect = gHVfDeadBandCompOpt.CurPhaseFeed - tempSect * 10922;           // present pos

    tempSect = gHVfDeadBandCompOpt.CurPhaseFeed_pre / 10922;
    tempSect = __IQsat(tempSect, 5, 0);
    phase_sect_pre = gHVfDeadBandCompOpt.CurPhaseFeed_pre - tempSect * 10922;   // previous pos

    if((phase_sect_pre <= 5461 && phase_sect >= 5461) ||            // sample point
        (phase_sect_pre >= 5461 && phase_sect <= 5461))
    {
        gHVfDeadBandCompOpt.CurPhasePredict = gHVfDeadBandCompOpt.CurPhaseFeed;
    }
    else           // go to predict
    {
        gHVfDeadBandCompOpt.CurPhasePredict += gHVfDeadBandCompOpt.CurPhaseStepPredict;
    }

    phase = gHVfDeadBandCompOpt.CurPhasePredict;

    gHVfDeadBandCompOpt.DbCompCpwmWidth = 1;

    if(abs(phase) < gHVfDeadBandCompOpt.DbCompCpwmWidth) gDeadBand.CompU = 0;
    else if(phase < 0)                          gDeadBand.CompU = m_Com;
    else if(phase > 0)                          gDeadBand.CompU = -m_Com;

	phase -= 21845;
    if(abs(phase) < gHVfDeadBandCompOpt.DbCompCpwmWidth) gDeadBand.CompV = 0;
    else if(phase < 0)                          gDeadBand.CompV = m_Com;
    else if(phase > 0)                          gDeadBand.CompV = -m_Com;

	phase -= 21845;
    if(abs(phase) < gHVfDeadBandCompOpt.DbCompCpwmWidth) gDeadBand.CompW = 0;
    else if(phase < 0)                          gDeadBand.CompW = m_Com;
    else if(phase > 0)                          gDeadBand.CompW = -m_Com;
}

void HVfOscDampDeal()
{
   // int  tempLg;
    int  tempVolt;
	long   VoltRs;
	
    gHVfOscDamp.CurMagSet = gMotorExtPer.IoVsFreq;
    tempVolt = (long)gMotorExtPer.R1 * (gHVfOscDamp.CurMagSet - (gIMTQ24.M>>12)) >>15;
    //gHVfOscDamp.VoltSmSet = (long)tempVolt * (int)gHVfOscDamp.OscDampGain /10L;
    gHVfOscDamp.VoltSmSet = (long)tempVolt * (int)gVFPar.VFOvShock /10L;

    gHVfOscDamp.VoltAmp = gOutVolt.Volt; // 锟窖硷拷转锟斤拷锟斤拷锟斤拷 
    gHVfOscDamp.VoltPhase = user_atan(gHVfOscDamp.VoltSmSet, gHVfOscDamp.VoltAmp);

#if (SOFTSERIES == MD380SOFT)
	if(((INV_VOLTAGE_690V == gInvInfo.InvVoltageType)&&(gInvInfo.InvTypeApply > 30)&&(gBasePar.FcSetApply <= 12))||
		((INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)&&(gInvInfo.InvTypeApply > 30)))	
	{
		VoltRs = (long)gMotorExtPer.R1 * gMotorExtPer.IoVsFreq >> 16 ;	
    	gHVfOscDamp.VoltSmSet +=  VoltRs;
    	gHVfOscDamp.VoltPhase = user_atan(gHVfOscDamp.VoltSmSet, gHVfOscDamp.VoltAmp);		
		
    	gHVfOscDamp.VoltPhaseFilter.taoVsTs = 100 ;//50
    	gHVfOscDamp.VoltPhaseFilt = Filter_1st(gHVfOscDamp.VoltPhase, gHVfOscDamp.VoltPhaseFilt, &gHVfOscDamp.VoltPhaseFilter) ;
        gHVfOscDamp.VoltPhase = gHVfOscDamp.VoltPhaseFilt;
	}
#endif	
    if(gMainCmd.FreqSyn < 0)      // 锟斤拷转
    {
        gHVfOscDamp.VoltPhase = - gHVfOscDamp.VoltPhase;
    }
}

/*********************************************************************
consider the stator resitance in low-frequency;
reconstruct the M-T current

Q-current:      Q12
Q-resistance:   Q16
Q-voltage:      Qxx
**********************************************************************/
void HVfCurReDecomp()
{
    int     m_CosPhi;     // Q15, angle phi is the angle of power factor
    int     m_SinPhi;     // Q15,
    long    m_AntiVolt;
    long    m_ResVolt;

    int m_phi2;
    int mDir;

    long temp1;
    long temp2;
    long temp3;
    
    // prepare reistance value, in p.u.
    // pi/2: 16384
    //m_CosPhi = qsin(gIAmpTheta.Theta);
    m_CosPhi = qsin(16384 - gIAmpTheta.PowerAngle);
    m_SinPhi = qsin(gIAmpTheta.PowerAngle);

    m_ResVolt = (long)((Ulong)gLineCur.CurPer * gMotorExtPer.R1 >> 16);   // voltage: Q12
    m_ResVolt = (m_ResVolt < gOutVolt.VoltApply) ? m_ResVolt : gOutVolt.VoltApply;
    
    temp1 = (long)gOutVolt.VoltApply * gOutVolt.VoltApply >> 12;
    temp2 = m_ResVolt * m_ResVolt >> 12;
    temp3 = gOutVolt.VoltApply * m_ResVolt >> 12;
    temp3 = temp3 * m_CosPhi >> 14;
    
    m_AntiVolt = (temp1 + temp2 - temp3) << 12;
    gOutVolt.antiVolt = qsqrt(m_AntiVolt);

    temp1 = ((long)gOutVolt.VoltApply * m_CosPhi >> 15) - m_ResVolt;
    temp2 = (long)gOutVolt.VoltApply * m_SinPhi >> 15;
    m_phi2 = user_atan((int)temp1, (int)temp2);

    // *generate results
    mDir = (gMainCmd.FreqSyn >= 0) ? 1 : -1;
    gHVfCur.M = (int)((long)gLineCur.CurPer * qsin(m_phi2) >> 15) * mDir;
    gHVfCur.T = (int)((long)gLineCur.CurPer * qsin(16384-m_phi2) >> 15) * mDir;

}
