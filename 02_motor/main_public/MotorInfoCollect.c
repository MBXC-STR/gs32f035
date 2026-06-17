/****************************************************************
文件功能: AD模块初始化，模拟量采样和转换
文件版本：
最新更新：
	
****************************************************************/
#include "MotorInfoCollectInclude.h"

// // 全局变量定义
ADC_STRUCT				gADC;		    //ADC数据采集结构
UDC_STRUCT				gUDC;		    //母线电压数据
IUVW_SAMPLING_STRUCT	gCurSamp;

UVW_STRUCT				gIUVWQ12;		//定子三相坐标轴电流
UVW_STRUCT_Q24          gIUVWQ24;       //Q24格式的三相定子电流
MT_STRUCT				gIMTQ12;		//MT轴系下的电流,Q12表示
MT_STRUCT_Q24           gIMTQ24;        //MT轴系下的电流,Q24表示
MT_STRUCT               gIMTSetQ12;
ALPHABETA_STRUCT		gIAlphBeta;	    //定子两相坐标轴电流
ALPHABETA_STRUCT		gIAlphBetaQ12;	    //定子两相坐标轴电流
AMPTHETA_STRUCT			gIAmpTheta;	    //极坐标表示的电流
LINE_CURRENT_STRUCT		gLineCur;	
CUR_EXCURSION_STRUCT	gExcursionInfo; //检测零漂使用的结构
TEMPLETURE_STRUCT		gTemperature;
AI_STRUCT				gAI;
ADC_ADJ_STRUCT          gADCAdjust;
#if (SOFTSERIES == MD500SOFT)
/************************************************************
	MD500计算零漂程序
	多W相零漂计算及制动电流采样零漂
************************************************************/
void GetCurExcursion(void)
{
	s16 m_ErrIu,m_ErrIv,m_ErrIw;
	Uint m_ErrIb;
	
	if((gMainStatus.RunStep != STATUS_LOW_POWER) && 
	   (gMainStatus.RunStep != STATUS_STOP))
	{
		gExcursionInfo.EnableCount = 0;
		return;
	}

	gExcursionInfo.EnableCount++;
	gExcursionInfo.EnableCount = (gExcursionInfo.EnableCount>200)?200:gExcursionInfo.EnableCount;
	if((gExcursionInfo.EnableCount < 200))
	{
		gExcursionInfo.TotalIu = 0;
		gExcursionInfo.TotalIv = 0;
		gExcursionInfo.TotalIw = 0;
		gExcursionInfo.Count = 0;
		gExcursionInfo.IbCount = 0;
		gExcursionInfo.IbOkFlag = 0;
		return;		
	}
	gExcursionInfo.TotalIu += gExcursionInfo.Iu;
	gExcursionInfo.TotalIv += gExcursionInfo.Iv;
	gExcursionInfo.TotalIw += gExcursionInfo.Iw;
	gExcursionInfo.Count++;
	gExcursionInfo.TotalIb += gExcursionInfo.Ib;
	gExcursionInfo.IbCount++;
	
	if(gExcursionInfo.Count >= 32)					//每32拍检测一次零漂
	{
		m_ErrIu = gExcursionInfo.TotalIu >> 5;
		m_ErrIv = gExcursionInfo.TotalIv >> 5;
		m_ErrIw = gExcursionInfo.TotalIw >> 5;
        if(-32768 == m_ErrIu)                       //防止取绝对值时溢出
            m_ErrIu = -32767;
        if(-32768 == m_ErrIv)
            m_ErrIv = -32767;	
        if(-32768 == m_ErrIw)
            m_ErrIw = -32767;	
		gExcursionInfo.TotalIu = 0;
		gExcursionInfo.TotalIv = 0;
		gExcursionInfo.TotalIw = 0;
		gExcursionInfo.Count = 0;
		
		if( (abs(m_ErrIu) < 5120) && (abs(m_ErrIv) < 5120) && (abs(m_ErrIw) < 5120))
		{
			gExcursionInfo.ErrIu = m_ErrIu;
			gExcursionInfo.ErrIv = m_ErrIv;
			gExcursionInfo.ErrIw = m_ErrIw;
			gExcursionInfo.ErrCnt = 0;
			gMainStatus.StatusWord.bit.RunEnable = 1;
		}
		else if((gExcursionInfo.ErrCnt++) > 5)		//连续5次零漂检测过大才报18故障
		{
			gError.ErrorCode.all |= ERROR_CURRENT_CHECK;
			gExcursionInfo.ErrCnt = 0;
			gExcursionInfo.EnableCount = 0;
			gMainStatus.StatusWord.bit.RunEnable = 0;
		}
	}

	if(gExcursionInfo.IbCount >= 512) 				//每512拍检测一次零漂
	{	    		
		if(gExcursionInfo.IbExcursionOkFlag == 0)   //只上电检测一次
		{
			m_ErrIb = gExcursionInfo.TotalIb >> 9;
			gExcursionInfo.TotalIb = 0;
			gExcursionInfo.IbCount = 0;
			//gExcursionInfo.Test = ((Ulong)m_ErrIb * 330L) >> 16;
			//	如果检测到在范围内的值认为是零漂，不再在此处判断是否直通
			//	在专门的判断直通函数中判断，那边用的值是去除零漂的
			if (m_ErrIb < 3971)  //200mV
			{
				gExcursionInfo.ErrIb = m_ErrIb;						
				gExcursionInfo.IbOkFlag = 1;
				gExcursionInfo.IbExcursionOkFlag = 1;
			}
			else if (gExcursionInfo.IbOkFlag == 0)
			{
				gError.ErrorCode.all |= ERROR_IGBT_SHORT_BRAKE;
	        	gError.ErrorInfo[4].bit.Fault1 = 1;	
				gExcursionInfo.EnableCount = 0;
				gExcursionInfo.IbOkFlag = 0;
			}
		}
		else
		{
		    gExcursionInfo.IbOkFlag = 1;
			gExcursionInfo.TotalIb = 0;
			gExcursionInfo.IbCount = 0;
		}
	}
}

#else  //MD380

/************************************************************
	计算零漂程序
************************************************************/
void GetCurExcursion(void)
{
	s16 m_ErrIu,m_ErrIv;
	
	if((gMainStatus.RunStep != STATUS_LOW_POWER) && 
	   (gMainStatus.RunStep != STATUS_STOP))
	{
		gExcursionInfo.EnableCount = 0;
		return;
	}

	gExcursionInfo.EnableCount++;
	gExcursionInfo.EnableCount = (gExcursionInfo.EnableCount>200)?200:gExcursionInfo.EnableCount;
	if((gExcursionInfo.EnableCount < 200))
	{
		gExcursionInfo.TotalIu = 0;
		gExcursionInfo.TotalIv = 0;
		gExcursionInfo.Count = 0;
		return;		
	}
	gExcursionInfo.TotalIu += gExcursionInfo.Iu;
	gExcursionInfo.TotalIv += gExcursionInfo.Iv;
	gExcursionInfo.Count++;

	if(gExcursionInfo.Count >= 32)					//每32拍检测一次零漂
	{
		m_ErrIu = gExcursionInfo.TotalIu >> 5;
		m_ErrIv = gExcursionInfo.TotalIv >> 5;
        if(-32768 == m_ErrIu)                       //防止取绝对值时溢出
            m_ErrIu = -32767;
        if(-32768 == m_ErrIv)
            m_ErrIv = -32767;		
		gExcursionInfo.TotalIu = 0;
		gExcursionInfo.TotalIv = 0;
		gExcursionInfo.Count = 0;
		
		gMainStatus.StatusWord.bit.RunEnable = 1;
		if( (abs(m_ErrIu) < 5120) && (abs(m_ErrIv) < 5120) )
		{
			gExcursionInfo.ErrIu = m_ErrIu;
			gExcursionInfo.ErrIv = m_ErrIv;
			gExcursionInfo.ErrCnt = 0;
		}
		else if((gExcursionInfo.ErrCnt++) > 5)		//连续5次零漂检测过大才报18故障
		{
			gError.ErrorCode.all |= ERROR_CURRENT_CHECK;
			gExcursionInfo.ErrCnt = 0;
			gExcursionInfo.EnableCount = 0;
		}
	}
}
#endif
/****************************************************************
	获取母线电压数据，输出gUDC
*****************************************************************/
void GetUDCInfo(void)
{
	Uint m_uDC;
	//s16	 m_DetaUdc;

   	m_uDC = ((Uint32)ADC_UDC * gUDC.Coff)>>16;                  //9
   	gUDC.uDC = (gUDC.uDC + m_uDC)>>1;
   	gUDC.uDCFilter = gUDC.uDCFilter - (gUDC.uDCFilter>>3) + (gUDC.uDC>>3);
    gUDC.uDCShow = Filter32(gUDC.uDC,gUDC.uDCShow);  // 用于键盘显示的母线电压
	gUDC.uDCBigFilter = Filter128(gUDC.uDC,gUDC.uDCBigFilter);   // Wc = 1Hz; trise > 1ms
#if (SOFT_INPUT_DETECT == STLEN)
    gSoftInLose.MaxUdc_5s = Max(gSoftInLose.MaxUdc_5s ,gUDC.uDC);
    gSoftInLose.MinUdc_5s = Min(gSoftInLose.MinUdc_5s ,gUDC.uDC);
	gSoftInLose.MaxUdcTemper = Max(gSoftInLose.MaxUdcTemper ,gUDC.uDC);
	gSoftInLose.MinUdcTemper = Min(gSoftInLose.MinUdcTemper ,gUDC.uDC);
#endif 
}

/****************************************************************
	获取电流采样数据，输出gCurSamp
*****************************************************************/
void GetCurrentInfo(void)
{
	long  m_Iu,m_Iv,m_TempT;
#if (SOFTSERIES == MD500SOFT)
	long m_Iw;
//	Uint m_AvgU,m_AvgV,m_AvgW;
#endif	

#if (SOFTSERIES == MD500SOFT)
/*#ifdef CURRENT_ADJUST		// 运行过程中AD零漂矫正，非MD500项目可以不处理
	long  m_TempU,m_TempV,m_TempW;

    // 已知斜率和偏置，通过AD值先反求电压，然后根据满电压对应65536来求得理想AD值
    m_TempU = (long)ISamp.iuAver - (long)gADCAdjust.Offset;
    m_TempU = (m_TempU * (long)gADCAdjust.SlopePU) >> 14;   
	m_TempU = (m_TempU < 0) ? 0: m_TempU;
	m_AvgU = (Uint)((m_TempU > 65535) ? 65535:m_TempU);

	m_TempV= (long)ISamp.ivAver - (long)gADCAdjust.Offset;
    m_TempV = (m_TempV * (long)gADCAdjust.SlopePU) >> 14;   
	m_TempV = (m_TempV < 0) ? 0: m_TempV;
	m_AvgV= (Uint)((m_TempV > 65535) ? 65535: m_TempV);

	m_TempW = (long)ISamp.iwAver - (long)gADCAdjust.Offset;
    m_TempW = (m_TempW * (long)gADCAdjust.SlopePU) >> 14;   
	m_TempW = (m_TempW < 0) ? 0: m_TempW;
	m_AvgW = (Uint)((m_TempW > 65535) ? 65535: m_TempW); 
#endif*/
#endif

	gExcursionInfo.Iu = (s16)(ADC_IU - (Uint)32768);		
	m_Iu = (long)gExcursionInfo.Iu - (long)gExcursionInfo.ErrIu;	//去除零漂
    gExcursionInfo.IuValue = m_Iu;                                  //用于参数辨识，参数辨识优化后将不再使用该变量                       
	gShortGnd.ShortCur = Filter32(m_Iu, gShortGnd.ShortCur);
	m_Iu = (m_Iu * gCurSamp.Coff) >> 3;
	m_Iu = __IQsat(m_Iu, C_MAX_PER, -C_MAX_PER);   

    
	gExcursionInfo.Iv = (s16)(ADC_IV - (Uint)32768);		
	m_Iv = (long)gExcursionInfo.Iv - (long)gExcursionInfo.ErrIv;	//去除零漂
    gExcursionInfo.IvValue = m_Iv;    
	//m_Iv = (m_Iv * gUVCoff.UDivV) >> 12;						    //纠正增益偏差
	m_Iv = (m_Iv * gCurSamp.Coff) >> 3;	
	m_Iv = __IQsat(m_Iv, C_MAX_PER, -C_MAX_PER);

    gIUVWQ24.U = m_Iu;                      /*不使用剔除毛刺滤波函数2011.05.07 L1082*/
    gIUVWQ24.V = m_Iv;
	
#if (SOFTSERIES == MD500SOFT)
	gExcursionInfo.Iw = (s16)(ADC_IW - (Uint)32768);		
	m_Iw = (long)gExcursionInfo.Iw - (long)gExcursionInfo.ErrIw;	//去除零漂
	m_Iw = (m_Iw * gCurSamp.Coff) >> 3;
	m_Iw = __IQsat(m_Iw, C_MAX_PER, -C_MAX_PER); 
	gIUVWQ24.W = m_Iw;
	
	//Q16->2*InvBreakMaxCurrent
	gExcursionInfo.Ib = ADC_IBREAK;
	if (gExcursionInfo.Ib > gExcursionInfo.ErrIb)
	{
		//Q12->InvBreakMaxCurrent
		gBrake.IBreakQ12 = ((Ulong)(gExcursionInfo.Ib - gExcursionInfo.ErrIb)*282L) >> 11;
	}
	else
	{
		gBrake.IBreakQ12 = 0;
	}
	//	制动电阻电流计算
	if(0 == EPwm1Regs.TBSTS.bit.CTRDIR && gExcursionInfo.IbOkFlag == 1)
	{			
		gBrake.IBreak = ((Uint32)gBrake.IBreakQ12 * gInvInfo.InvBreakMaxCurrent) >> 12;
		if((gBrake.Flag & 0x02) == 0x02)
		{
		    gBrake.IBreakTotal = gBrake.IBreak;
		}
	}
#else
	gIUVWQ24.W = - (gIUVWQ24.U + gIUVWQ24.V);
	gIUVWQ24.W = __IQsat(gIUVWQ24.W,C_MAX_PER,-C_MAX_PER);  
#endif	

#if (SOFTSERIES == MD500SOFT)
    m_TempT = (long)ADC_TEMP - (long)gADCAdjust.Offset;
    m_TempT = (m_TempT * (long)gADCAdjust.SlopePU) >> 14;   
	m_TempT = (m_TempT < 0) ? 0: m_TempT;
	m_TempT = (Uint)((m_TempT > 65535) ? 65535:m_TempT);
#else           
    m_TempT = ADC_TEMP;
#endif        
    gTemperature.TempAD = Filter16((m_TempT &0xFFF0), gTemperature.TempAD);

    gAI.ai1Total += (ADC_AI1);
    gAI.ai2Total += (ADC_AI2);
    gAI.ai3Total += (ADC_AI3);
#if(AIRCOMPRESSOR == 1)
    gAI.ai4Total += (ADC_AI4);
	gAI.ai5Total += (ADC_AI5);
#endif
    gAI.aiCounter ++;
}

#if (SOFTSERIES == MD500SOFT)
//#ifdef CURRENT_ADJUST
/*******************************************************************************
* Function Name  : 
* Description    : 获取电流采样校正斜率与偏置
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GetADCAdjustInfo(void)
{
	Uint m_Dleta;
	s16 m_Offset;
	Uint m_Slope;
	Uint m_YL;
	Uint m_YH;
    s16  m_AdjEn;
	
	gADCAdjust.XL = 1247;         // 1.25,以0.001V为单位
	gADCAdjust.XH = 2494;         // 2.5V,以0.001V为单位
	m_YL = ADC_1250;  // 1.25V对应的ADC采样值，Q16
	m_YH = PL_VOE_PROTECT; // 2.5V对应的ADC采样值，Q16

    m_AdjEn = 0;
    if((m_YL > 23824) && (m_YL < 25817))
    {
        m_AdjEn = 1;
    }
//	判断是否是高电平，如果变低则不能进行校正
    if(m_AdjEn == 1)
    {
    	if (PL_VOE_PROTECT > ADC_VOLTAGE_20)
    	{
    		gADCAdjust.Ticker ++;
    		if (gADCAdjust.Ticker <= 1024)
    		{
    			gADCAdjust.YLTotal += m_YL;
    			gADCAdjust.YHTotal += m_YH;		
    		}
    		else
    		{
    			gADCAdjust.Ticker = 0;
    			gADCAdjust.YL = gADCAdjust.YLTotal >> 10;
    			gADCAdjust.YH = gADCAdjust.YHTotal >> 10;
    			gADCAdjust.YLTotal = 0;
    			gADCAdjust.YHTotal = 0;
    		}	
    		m_Dleta = gADCAdjust.YH - gADCAdjust.YL;
    		m_Slope = 1000UL * m_Dleta/(gADCAdjust.XH - gADCAdjust.XL); // 斜率 Q16
    		m_Offset = (long)gADCAdjust.YL - ((long)gADCAdjust.XL * m_Slope / 1000L);
    		m_Offset = Min(m_Offset, 256);
    		m_Offset = Max(m_Offset, -256);
    		gADCAdjust.Offset = m_Offset;// 偏置 Q16 
    	}
    	else
    	{
    		m_Slope = gADCAdjust.Slope;
    	}
    }
    else
    {
        gADCAdjust.Offset = 0;
        m_Slope = 19859L;
    }
	gADCAdjust.Slope = Min(m_Slope, 20000);//19859
	gADCAdjust.Slope = Max(gADCAdjust.Slope, 19700);
	gADCAdjust.SlopePU = (19859L << 14) / gADCAdjust.Slope; 
}
//#endif
#endif
