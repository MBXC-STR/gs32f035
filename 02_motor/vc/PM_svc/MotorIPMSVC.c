/***********************************Inovance***********************************
鍔熻兘鎻忚堪锛團unction Description锛�:鍚屾鏈烘棤閫熷害浼犳劅鍣ㄦ帶鍒�
鏈�鍚庝慨鏀规棩鏈燂紙Date锛夛細
淇敼鏃ュ織锛圚istory锛�:锛堜互涓嬭褰曚负绗竴娆¤浆娴嬭瘯鍚庯紝寮�濮嬭褰曪級
	浣滆�� 		鏃堕棿 		鏇存敼璇存槑
1 	xx 		xxxxx 		xxxxxxx
2 	yy 		yyyyy 		yyyyyyy
************************************Inovance***********************************/

/* Includes ------------------------------------------------------------------*/
#include "MotorInclude.h"
#include "MotorIPMSVC.h"

/* Private variables ---------------------------------------------------------*/
PMSM_ROTOR_POS_EST_STRUCT   gPmsmRotorPosEst;
extern MT_STRUCT_Q24           gUMTQ24; 
/* Private function prototypes -----------------------------------------------*/
void PrepareParForPmsmSvc(void);
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : PmsmSvc鍙傛暟鍒濆鍖栧嚱鏁�
* Description    : 鍒濆鍖栧悓姝ユ満SVC闇�瑕佺敤鍒扮殑鍙橀噺锛屽悓鏃惰绠楀弬鏁板垵濮嬪��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ResetParForPmsmSvc(void)
{       
    /*澶嶄綅鍚屾鏈篠VC鐢ㄥ埌鐨勫彉閲�*/
    gPmsmRotorPosEst.Kc = 768;	        //瀵瑰簲0.75

	gPmsmRotorPosEst.IdLast = 0;
	gPmsmRotorPosEst.IqLast = 0;

	gPmsmRotorPosEst.DetaId = 0;
	gPmsmRotorPosEst.DetaIq = 0;

    gPmsmRotorPosEst.FcSetlpf = gPmsmRotorPosEst.FcLow * 100;
    gPmsmRotorPosEst.SvcIdSetForLowSpeed = (((s32)gPmsmRotorPosEst.SvcIdMaxForLowSpeed)<<12)/100;
    gPmsmRotorPosEst.SvcSpeedLpfCoef = 30;

    gPmsmRotorPosEst.CurFreqSetReal = 0;
	gPmsmRotorPosEst.FCMode = 0;

	gPmsmRotorPosEst.UdLast = 0;
	gPmsmRotorPosEst.UqLast = 0;
	gPmsmRotorPosEst.SpeedEstValueLpf =0;
	gPmsmRotorPosEst.SpeedEstValue =0;
	gPmsmRotorPosEst.SvcRotorSpeed = 0;
    gPmsmRotorPosEst.SpeedEstValueLast = 0;

	gPmsmRotorPosEst.BmfEstValueLast = 0;
	gPmsmRotorPosEst.BmfEstValue = 0;	
   /**********************鍒濆鍖栧弽鐢靛姩鍔跨浉鍏冲彉閲�******************/
    if((gUVCoff.OnlineTuneBemfVoltEnable == 1)&&(INV_VOLTAGE_380V == gInvInfo.InvVoltageType))
	{
	    gEstBemf.BemfVoltDisPlay = gEstBemf.BemfVoltFilter;
		gEstBemf.BemfVoltOnline = gEstBemf.BemfVoltFilter;
		gEstBemf.BemfVoltDisPlayLast = gEstBemf.BemfVoltFilter;		
	}
	/***************************END********************************/
    /*鍒濆鍖栬浆閫熶及璁＄敤鍒扮殑绯绘暟*/
    PrepareParForPmsmSvc();
}

/*************************************************************
    鍒濆鍖栬浆閫熷拰浣嶇疆浼扮畻涓渶瑕佺敤鍒扮殑绯绘暟锛岄粯璁よ浇棰�2.0K
    浣跨敤鐨勫閮ㄥ弬鏁�:
    ggBasePar.FullFreq01:棰戠巼鍩哄�硷紝鍗曚綅0.01HZ
    gBasePar.FullFreq涓巊MotorInfo.Frequency灏忔暟鐐逛綅鏁扮浉鍚�
    gBasePar.FcSetApply:杞芥尝棰戠巼锛屽崟浣�100HZ
    gMotorExtPer.LQ:Q13
    gMotorExtInfo.EMF:绾垮弽鐢靛姩鍔挎湁鏁堝�硷紝鍗曚綅0.1V
*************************************************************/
void PrepareParForPmsmSvc(void)
{
    s32 m_s32;
//	static u16 m_Flag = 0;
      
    //杞芥尝棰戠巼 
    gPmsmRotorPosEst.FcSetPer  = 65189865L/gBasePar.FullFreq01;  

    //璁＄畻绯绘暟T/L 
    m_s32 = 33554432L/gPmsmRotorPosEst.FcSetPer;
//    gPmsmRotorPosEst.TDivLqCoef = ((m_s32<<8)/gMotorExtPer.LQ);
    gPmsmRotorPosEst.TDivLqCoef = ((m_s32<<8)/gMotorExtPer.LD);
    m_s32 = 33554432L/gPmsmRotorPosEst.FcSetPer;
    gPmsmRotorPosEst.TDivLdCoef = ((m_s32<<8)/gMotorExtPer.LD);

    //璁＄畻绯绘暟KiForEmf=Ka*L/T 
//	m_s32 = (((s32)gMotorExtPer.LQ * (s32)gPmsmRotorPosEst.FcSetPer)>>8);
    m_s32 = (((s32)gMotorExtPer.LD * (s32)gPmsmRotorPosEst.FcSetPer)>>8); 
    gPmsmRotorPosEst.KiForEmf = (((s32)gPmsmRotorPosEst.Ka * m_s32)>>5);
	  
    //璁＄畻纾侀�氭爣涔堝�� 1/Kf 
	if((gUVCoff.OnlineTuneBemfVoltEnable == 1)&&(INV_VOLTAGE_380V == gInvInfo.InvVoltageType))
	{
	    m_s32 = ((s32)gEstBemf.BemfVoltFilter<<10)/gMotorInfo.Votage;
	}
	else
	{
        m_s32 = ((s32)gMotorExtInfo.BemfVolt<<10)/gMotorInfo.Votage;
	}
    m_s32 = (m_s32 * (s32)gBasePar.FullFreq)/((s32)gMotorInfo.Frequency * 10);
	gPmsmRotorPosEst.InvOfKf = 1048576L/m_s32; 
 
    //璁＄畻绯绘暟KiForSpeed=Kb*L/(T*Kf)
	m_s32 = ((s32)gMotorExtPer.LD * (s32)gPmsmRotorPosEst.FcSetPer)>>10;
	gPmsmRotorPosEst.KiForSpeed = ((m_s32 * (s32)gPmsmRotorPosEst.Kb)>>13) * gPmsmRotorPosEst.InvOfKf;  // kb鍑忓皬涓�鍗�

    //璁＄畻瑙掑害鍙樻崲绯绘暟
	gPmsmRotorPosEst.TsPer = ((s32)gBasePar.FullFreq01 * 524L)/200;   
}


/*******************************************************************************
* Function Name  : 鍚屾鏈篠VC杞�熷拰浣嶇疆浼扮畻锛屽湪涓柇绋嬪簭涓墽琛�
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PmsmSvcCtrl(void)
{
    s16 m_Id,m_Iq,m_Ud,m_Uq;
	u16 m_FcSet;
    s32 m_Long,m_Long1;
    s32 m_TDivLqCoef,m_TDivLdCoef,m_TsPer,m_KiForSpeed,m_KiForEmf;


    if((1 == EPwm1Regs.TBSTS.bit.CTRDIR)||(gBasePar.FcSetApply > C_DOUBLE_ACR_MAX_FC))
	{
        m_FcSet = 3000000ul / (u32)gPWM.gPWMPrdApplyLast;    //姣斿疄闄呭�兼斁澶т簡10鍊嶏紝鎵�浠PmsmRotorPosEst.FcCoff涔熷彉澶�10鍊�
    }
	else 
	{
	    m_FcSet = 3000000ul / (u32)gPWM.gPWMPrdApply; 
	}

    m_TDivLdCoef = (gPmsmRotorPosEst.TDivLdCoef * (s32)gPmsmRotorPosEst.FcCoff)/m_FcSet;
    //m_TDivLdCoef = (((gPmsmRotorPosEst.TDivLdCoef * (s32)gPmsmRotorPosEst.FcCoff)/100L) * (s32)gPWM.gPWMPrdApplyLast)/3000L;
    m_TDivLqCoef = (gPmsmRotorPosEst.TDivLqCoef * (s32)gPmsmRotorPosEst.FcCoff)/m_FcSet; 
    //m_TDivLqCoef = (((gPmsmRotorPosEst.TDivLqCoef * (s32)gPmsmRotorPosEst.FcCoff)/100L) * (s32)gPWM.gPWMPrdApplyLast)/3000L; 
    m_TsPer      = (gPmsmRotorPosEst.TsPer * (s32)gPmsmRotorPosEst.FcCoff)/m_FcSet;
    //m_TsPer      = (((gPmsmRotorPosEst.TsPer * (s32)gPmsmRotorPosEst.FcCoff)/100L) * (s32)gPWM.gPWMPrdApplyLast)/3000L;
    m_KiForEmf   = (gPmsmRotorPosEst.KiForEmf * (s32)m_FcSet)/(s32)gPmsmRotorPosEst.FcCoff;
    //m_KiForEmf   = (gPmsmRotorPosEst.KiForEmf * 3000L)/(((s32)gPmsmRotorPosEst.FcCoff * (s32)gPWM.gPWMPrdApplyLast) / 100L);
    m_KiForSpeed = (gPmsmRotorPosEst.KiForSpeed * (s32)m_FcSet)/(s32)gPmsmRotorPosEst.FcCoff;
    //m_KiForSpeed = (gPmsmRotorPosEst.KiForSpeed * 3000L)/(((s32)gPmsmRotorPosEst.FcCoff * (s32)gPWM.gPWMPrdApplyLast) / 100L);
    //寰楀埌褰撳墠鐢垫祦銆佺數鍘嬬殑鏈夋晥鍊�	
    m_Id = gIMTQ12.M;
    m_Iq = gIMTQ12.T;

   // m_Ud = (s32)gOutVolt.VoltApply * (s32)qsin(16384 - gOutVolt.VoltPhaseApply)>>15;
   // m_Uq = (s32)gOutVolt.VoltApply * (s32)qsin(gOutVolt.VoltPhaseApply)>>15;

    m_Ud = gPmsmRotorPosEst.Ud;
    m_Uq = gPmsmRotorPosEst.Uq;

    //璁＄畻D銆丵杞寸數娴佷及璁″��
    //m_Long  = ((s32)gMotorExtPer.LQ * (s32)gPmsmRotorPosEst.IqLast)>>10;  
    m_Long  = ((s32)gMotorExtPer.LD * (s32)gPmsmRotorPosEst.IqLast)>>10;             
    m_Long  = (m_Long * (s32)gPmsmRotorPosEst.SpeedEstValueLast)>>15;              
    m_Long1 = ((s32)gPmsmRotorPosEst.UdLast)<<3; 
    m_Long  = m_Long + m_Long1;                                                    
    m_Long1 = ((s32)gMotorExtPer.Rpm * (s32)gPmsmRotorPosEst.IdLast)>>13;            
    m_Long  = m_Long - m_Long1;             
    m_Long  = (m_Long * (s32)m_TDivLdCoef)>>13;                                      
    gPmsmRotorPosEst.IdEstValue = m_Long + gPmsmRotorPosEst.IdLast;               
    
    m_Long  = ((s32)gMotorExtPer.LD * (s32)gPmsmRotorPosEst.IdLast)>>10;                    
    m_Long  = (m_Long * (s32)gPmsmRotorPosEst.SpeedEstValueLast)>>15;                                
    m_Long1 = ((s32)gPmsmRotorPosEst.UqLast - gPmsmRotorPosEst.BmfEstValueLast)<<3;     
    m_Long  = m_Long1 - m_Long;                                                           
    m_Long1 = ((s32)gMotorExtPer.Rpm * (s32)gPmsmRotorPosEst.IqLast)>>13;             
    m_Long  = m_Long - m_Long1;      
    m_Long  = (m_Long * (s32)m_TDivLqCoef)>>13;       
    gPmsmRotorPosEst.IqEstValue = m_Long + gPmsmRotorPosEst.IqLast;

    //璁＄畻鐢垫祦鍋忓樊鍜屽弽鐢靛姩鍔夸及璁″��
    gPmsmRotorPosEst.DetaId = gPmsmRotorPosEst.DetaId + ((gPmsmRotorPosEst.IdEstValue - m_Id)>>3) - (gPmsmRotorPosEst.DetaId>>3);
    gPmsmRotorPosEst.DetaIq = gPmsmRotorPosEst.DetaIq + ((gPmsmRotorPosEst.IqEstValue - m_Iq)>>1)-(gPmsmRotorPosEst.DetaIq>>1);
    gPmsmRotorPosEst.BmfEstValue = gPmsmRotorPosEst.BmfEstValue + ((s32)gPmsmRotorPosEst.DetaIq * (s32)m_KiForEmf>>17);

    m_Long = gPmsmRotorPosEst.Kc;

    if(gPmsmRotorPosEst.DetaId > 0)
    {
        m_Long = -m_Long;
    }
    m_Long  = 1024 + m_Long;

    m_Long1 = m_KiForSpeed;
	if(gPmsmRotorPosEst.BmfEstValueLast >0)
    {
        m_Long1 = -m_Long1;
    }
    m_Long = (m_Long * m_Long1)>>10;                      

    m_Long = (m_Long * gPmsmRotorPosEst.DetaId)>>12;   

    //浼拌杞扛鑷�鎭拷
    gPmsmRotorPosEst.SpeedEstValue    = m_Long + (((s32)gPmsmRotorPosEst.InvOfKf * (s32)gPmsmRotorPosEst.BmfEstValue)>>7);
    gPmsmRotorPosEst.SpeedEstValueLpf = gPmsmRotorPosEst.SpeedEstValueLpf + (gPmsmRotorPosEst.SpeedEstValue/gPmsmRotorPosEst.SvcSpeedLpfCoef) 
                                        -(gPmsmRotorPosEst.SpeedEstValueLpf /gPmsmRotorPosEst.SvcSpeedLpfCoef);
    
    gPmsmRotorPosEst.SvcRotorSpeed = __IQsat(gPmsmRotorPosEst.SpeedEstValueLpf,32767,-32767);      // 瀵归�熷害涓婇檺澧炲姞闄愬箙
    //gPmsmRotorPosEst.SvcRotorSpeed  = gPmsmRotorPosEst.SpeedEstValueLpf;
    
    gPmsmRotorPosEst.SvcRotorPos    = gPmsmRotorPosEst.SvcRotorPos + (((s64)gPmsmRotorPosEst.SpeedEstValue * (s64)m_TsPer)>>3);

    gPmsmRotorPosEst.BmfEstValueLast   = gPmsmRotorPosEst.BmfEstValue;
    gPmsmRotorPosEst.SpeedEstValueLast = gPmsmRotorPosEst.SpeedEstValue;
    
    gPmsmRotorPosEst.IdLast = m_Id;
    gPmsmRotorPosEst.IqLast = m_Iq;
    gPmsmRotorPosEst.UdLast = m_Ud;
    gPmsmRotorPosEst.UqLast = m_Uq;   
}


/*******************************************************************************
* Function Name  : 浣庨�熸椂瀵硅浇棰戠殑澶勭悊
* Description    : 涓昏鐢ㄤ簬鍑忓皬姝诲尯鐨勫奖?
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PmsmSvcCalFc(void)
{
/*	s16 m_Freq,m_FreqLow,m_FreqHigh;
    u16 m_FcLow,m_FcHigh,m_FcSet,m_FcSetlpf;
    s32 m_Long;
*/
    s16 m_Freq;
	s32 m_Long,m_FcLow,Fc;

	m_FcLow = gPmsmRotorPosEst.FcLow;
	m_Freq = abs(gRotorSpeed.SpeedBigFilter);

	if(gPmsmRotorPosEst.FCMode == 0)
	{
		if(m_Freq > (gMotorInfo.FreqPer>>2))
		{
			gPmsmRotorPosEst.FCMode = 1;
		}
	}
	else
	{
		if(m_Freq < (gMotorInfo.FreqPer>>3))
		{
			gPmsmRotorPosEst.FCMode = 0;
		}
	}
	if(gPmsmRotorPosEst.FCMode == 0)
	{
		Fc = m_FcLow;
   //     gFcCal.MinFc = gPmsmRotorPosEst.FcLow;
	}
	else
	{
		Fc = gFcCal.FcBak;
	}
	gFcCal.FcBak = __IQsat(Fc,100,10);
/*
#if (SOFTSERIES == MD500SOFT)
	m_FcLow = gPmsmRotorPosEst.FcLow * 100;
#else
    m_FcLow = gPmsmRotorPosEst.FcLow * 150;
#endif
	//m_FcHigh = gBasePar.FcSet * 100;
	if(gBasePar.FcSet < 20)
	{
	    m_FcHigh = 2000;
	}
	else
	{
	    m_FcHigh = gBasePar.FcSet * 100;
	}

	if((m_FcHigh <= m_FcLow)||(gPmsmRotorPosEst.FcLowPoint == 0))
	{
		m_FcSet = m_FcHigh;
		gPmsmRotorPosEst.FcSetlpf = m_FcSet;
	}
	else
	{
		if(1 == gMainCmd.Command.bit.TorqueCtl)     // 杞煩妯″紡涓嬬敤瀹為檯閫熷害鍒ゆ柇
		{
		    m_Freq = abs(gRotorSpeed.SpeedBigFilter);
		}
		else
		{
		    m_Freq = abs(gMainCmd.FreqSet);
		}
		m_FreqLow  = (Ulong)gMotorInfo.FreqPer*gPmsmRotorPosEst.FcLowPoint/250UL;
		m_FreqHigh = (Ulong)gMotorInfo.FreqPer*gPmsmRotorPosEst.FcLowPoint/100UL;

		if(m_Freq <= m_FreqLow)
		{
		    m_FcSet = m_FcLow;
		}
		else if(m_Freq > m_FreqHigh)
		{
			m_FcSet = m_FcHigh;
		}
		else
		{
			m_Long  = (s32)(m_Freq - m_FreqLow) * (m_FcHigh - m_FcLow);
			m_FcSet = m_FcLow + m_Long/(m_FreqHigh - m_FreqLow);	
		}
		gPmsmRotorPosEst.FcSetlpf = m_FcSet;//gPmsmRotorPosEst.FcSetlpf + (m_FcSet>>3) - (gPmsmRotorPosEst.FcSetlpf>>3);
	}
    //gPmsmRotorPosEst.FcSetlpf = gPmsmRotorPosEst.FcSetlpf + (m_FcSet>>3) - (gPmsmRotorPosEst.FcSetlpf>>3);
    m_FcSetlpf = gPmsmRotorPosEst.FcSetlpf/100;
    gFcCal.FcBak = Min(gFcCal.FcBak,m_FcSetlpf);
*/
    m_Long = ((s32)(gBasePar.FcSetApply * 100) * (s32)gPmsmRotorPosEst.SvcSpeedLpfTs)>>13;
    m_Long = m_Long * 200 / (s32)gPmsmRotorPosEst.FcCoff;  //鑰冭檻涓�涓浇娉㈠懆鏈熻皟鏁翠竴娆″拰涓ゆ锛屽婊ゆ尝鏃堕棿甯告暟鐨勫奖鍝�
	if(m_Freq < (gMotorInfo.FreqPer / 5))   // 浣庨�熶笅澧炲姞婊ゆ尝绯绘暟锛寃yk
	{
	    m_Long = 2*m_Long;
	}
    m_Long = Min(m_Long,512);
    m_Long = Max(m_Long,8);
	gPmsmRotorPosEst.SvcSpeedLpfCoef = m_Long;         
}

/*******************************************************************************
* Function Name  : 浣庨�熸椂瀵瑰姳纾佺數娴佺殑澶勭悊
* Description    : 褰撹緭鍑虹數鍘嬭緝浣庢椂锛岄�氳繃娉ㄥ叆鍔辩锛屽澶ц緭鍑虹數鍘嬶紝鍑忓皬姝诲尯鐨勫奖鍝�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PmsmSvcCalImForLowSpeed()
{
    s16 m_Freq,m_FreqLow,m_FreqHigh,m_IdSet,m_IdMax;
    s32 m_Long;

	m_Freq     = abs(gRotorSpeed.SpeedBigFilter);
	m_FreqLow  = gMotorInfo.FreqPer/10;
	m_FreqHigh = gMotorInfo.FreqPer/5;
    m_IdMax    = (((s32)gPmsmRotorPosEst.SvcIdMaxForLowSpeed)<<12)/100;

    if(m_Freq >= m_FreqLow)
    {
        if(m_Freq >= m_FreqHigh)
    	{
    		m_IdSet = 0;
    	}
    	else
    	{
    		m_Long  = (s32)(m_FreqHigh - m_Freq) * (s32)m_IdMax;
            m_IdSet = m_Long/(m_FreqHigh - m_FreqLow);
    	}
    }
    else  //閬垮厤棰濆畾棰戠巼寰堝皬鏃讹紝m_FreqZero鍜宮_FreqLow鐩哥瓑
    {
        m_IdSet =  m_IdMax;
    }

    gPmsmRotorPosEst.SvcIdSetForLowSpeed = m_IdSet;
    gPmsmRotorPosEst.SvcIdSetForLowSpeed = Max(gPmsmRotorPosEst.SvcIdSetForLowSpeed,0);
    gPmsmRotorPosEst.SvcIdSetForLowSpeed = Min(gPmsmRotorPosEst.SvcIdSetForLowSpeed,4000);

    if(gMotorInfo.Current > gInvInfo.InvCurrent)
    {
        gPmsmRotorPosEst.SvcIdSetForLowSpeed = (long)gPmsmRotorPosEst.SvcIdSetForLowSpeed * gInvInfo.InvCurrent / gMotorInfo.Current;
    }
    
}

/******************************* END OF FILE***********************************/
