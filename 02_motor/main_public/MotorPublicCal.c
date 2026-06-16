/****************************************************************
文件功能：包含驱动部分与控制方式无关的独立模块
文件版本：
更新日期：

****************************************************************/
#include "MotorPublicCalInclude.h"
/************************************************************/
/********************全局变量定义****************************/
DC_BRAKE_STRUCT			gDCBrake;	//直流制动用变量
BRAKE_CONTROL_STRUCT	gBrake;		//制动电阻控制用变量
SHORT_GND_STRUCT		gShortGnd;
JUDGE_POWER_LOW			gLowPower;	    //上电缓冲判断使用数据结构
PM_POS_EST_STRUCT       gPMPosEst;
extern BEFORE_RUN_PHASE_LOSE_STRUCT gBforeRunPhaseLose;
void GetPMPosEst(void);
void GetEMFUamp(void);
void GetPMPosSpeedObst(void);
extern void ADCEndIsrTuneR1(void);
/************************************************************
	上电对地短路判断
************************************************************/
void RunCaseShortGnd(void)
{
	gSubCommand.bit.OutputLostBeforeRun = 0;// gSubCommand.bit.OutputLost; 上电对地短路, 不检测输出缺相
	gExtendCmd.bit.ShortGndBeforeRun	= 1;
	//gShortGnd.BaseUDC = gUDC.uDC;
	OutputPhaseLoseAndShortGndDetect();
}

/************************************************************
	变频器的上电处理过程：判断母线电压稳定，屏蔽上电缓冲电阻
判断依据：1）母线电压大于欠压点 
       && 2）母线电压没有再增加 
       && 3）持续200ms
		  4）判断母线电压稳定后再延时200ms，进入停机状态。
************************************************************/
void RunCaseLowPower(void)
{
	Uint uDCLowLimt;
    Uint AftConRelay_Delay;
	s16 m_Return;
    static u16 m_DataBak = 0;
    
    ResetADCEndIsr();

	if((m_DataBak == 0)&&(gPGData.PGType == PG_TYPE_SPECIAL_UVW))    // 在继电器没有吸合之前记录省线式编码器位置
	{
	    gUVWPG.UVWExtData = (EXT_UVW_PG_U()<<2) + (EXT_UVW_PG_V()<<1) + EXT_UVW_PG_W();
		m_DataBak = 1;
    }	
	uDCLowLimt = gInvInfo.InvLowUDC + 200;
	if(INV_VOLTAGE_220V == gInvInfo.InvVoltageType)
	{
		uDCLowLimt = gInvInfo.InvLowUDC + 160;
	}

    if(gInvInfo.InvTypeApply > 18)
    {
        AftConRelay_Delay = 130;    //机型大于等于37kW,延时60ms作为吸合的机械缓冲时间
    }
    else
    {
        AftConRelay_Delay = 115;    //机型小于37kW,延时30ms作为吸合的机械缓冲时间            
    }
	switch(gMainStatus.SubStep)
	{
		case 2://判断母线电压大于欠压点&&电压没再增加&&持续200ms
			if((gUDC.uDCFilter > uDCLowLimt) && 
			   (gUDC.uDCFilter <= gLowPower.UDCOld))
			{
				if((gLowPower.WaiteTime++) >= 100)
				{
					gExcursionInfo.EnableCount = 199;
					gMainStatus.SubStep ++;	
					gLowPower.WaiteTime = 0;			
				}
			}
			else
			{
				gLowPower.WaiteTime = 0;
				gLowPower.UDCOld = gUDC.uDCFilter;
			}

			break;

		case 3:	//200ms延时
            IPMCheckInitPos();                             // 记录上次掉电时的初始位置
            m_Return = IPMPosAdjustStop();                   // 通过UVW或者CD信号校正角度
           
			if((m_Return == 1)&&((gLowPower.WaiteTime++) >= 100) && 
			   (gExcursionInfo.EnableCount >= 200))
			{
			    if(gLowPower.WaiteTime >= AftConRelay_Delay) 
				{			    
    				gMainStatus.RunStep = STATUS_STOP;
	    			gMainStatus.SubStep = 1;
                    gMainStatus.StatusWord.bit.LowUDC = 1;
			    	gMainStatus.StatusWord.bit.RunEnable = 1;
        		    if((gError.ErrorCode.all & ERROR_LOW_UDC) == ERROR_LOW_UDC)
                    {
                        gError.ErrorCode.all = 0;                   //欠压后需要清除故障?
		                if(gMainStatus.ErrFlag.bit.OvCurFlag == 1)  //修改欠压后无法进入过流中断的错误
		                {
			                gMainStatus.ErrFlag.bit.OvCurFlag = 0;
			                EALLOW;
			                EPwm2Regs.TZCLR.bit.OST = 1;
    			            EPwm3Regs.TZCLR.bit.OST = 1;
	    		            EPwm1Regs.TZCLR.bit.OST = 1;
		    	            EPwm1Regs.TZCLR.bit.INT = 1;
			                EDIS;
		                }     
        		    }
				    gBuffResCnt += 30000;
				    gPhase.IMPhase = GetTime() << 12;	//上电后随机选择初始相位
			    }
				ConnectRelay();				
//				gBuffResCnt += 30000;
//				gPhase.IMPhase = GetTime() << 12;	//上电后随机选择初始相位
			}
			break;

		default:
			DisConnectRelay();	
			ResetADCEndIsr();
            gError.ErrorCode.all |= ERROR_LOW_UDC;				//出错标志
        	gMainStatus.StatusWord.bit.LowUDC = 0;
			gMainStatus.StatusWord.bit.StartStop = 0;
			gLowPower.WaiteTime = 0;
			gLowPower.UDCOld = gUDC.uDCFilter;
			gMainStatus.SubStep = 2;
			break;
	}
}

/************************************************************
	判断UVW三相是否处于逐波限流状态，并且设置标志
************************************************************/
void CBCLimitCurPrepare(void)
{
	EALLOW;
#if (SOFTSERIES == MD380SOFT)
    if((INV_VOLTAGE_690V == gInvInfo.InvVoltageType)||(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)) 
    {
        if((EPwm1Regs.TZFLG.bit.CBC == 1) || (EPwm2Regs.TZFLG.bit.CBC == 1) || (EPwm3Regs.TZFLG.bit.CBC == 1))
        {
            EPwm1Regs.TZCLR.bit.CBC = 1;
            EPwm2Regs.TZCLR.bit.CBC = 1;
            EPwm3Regs.TZCLR.bit.CBC = 1;

            gCBCProtect.CbcTimes++;
            gCBCProtect.CbcFlag = 1;
        }    
    }
    else
#endif
    {
    	if(EPwm1Regs.TZFLG.bit.CBC == 1) 		//表示U相处于逐波限流状态
    	{
    		EPwm1Regs.TZCLR.bit.CBC = 1;
    		gCBCProtect.Flag.bit.CBC_U = 1;
    	}
    	
    	if(EPwm2Regs.TZFLG.bit.CBC == 1) 		//表示V相处于逐波限流状态
    	{
    		EPwm2Regs.TZCLR.bit.CBC = 1;
    		gCBCProtect.Flag.bit.CBC_V = 1;
    	}

    	if(EPwm3Regs.TZFLG.bit.CBC == 1) 		//硎網相处于逐波限流状态
    	{
    		EPwm3Regs.TZCLR.bit.CBC = 1;
    		gCBCProtect.Flag.bit.CBC_W = 1;
    	} 
    }
	EDIS;
}

/*************************************************************
    过流点的电流计算
*************************************************************/
Uint OcCurrentInfo(void)
{
    int   Int_Iu,Int_Iv,Int_Iw;

    GetCurrentInfo();   

    Int_Iu = (int)(gIUVWQ24.U >> 12);
    Int_Iv = (int)(gIUVWQ24.V >> 12);
    Int_Iw = (int)(gIUVWQ24.W >> 12);

    Int_Iu = (Int_Iu>0)?Int_Iu:(-Int_Iu);
    Int_Iv = (Int_Iv>0)?Int_Iv:(-Int_Iv);
    Int_Iw = (Int_Iw>0)?Int_Iw:(-Int_Iw);

    Int_Iu = (Int_Iu>Int_Iv)?Int_Iu:Int_Iv;
    Int_Iu = (Int_Iu>Int_Iw)?Int_Iu:Int_Iw;   

	return ((u16)Int_Iu);
}
/*************************************************************
	选择三相电流中的最大值
*************************************************************/
/*Uint MaxUVWCurrent(void)
{
	Uint m_IU_Immed,m_IU_Prd,m_IU_Zero;
    Uint m_Max;

    m_IU_Immed = OcCurrentInfo();
    m_IU_Prd = OcCurrentInfo();   
    m_IU_Zero = OcCurrentInfo();

    m_Max = (m_IU_Immed >= m_IU_Prd)?m_IU_Immed:m_IU_Prd;    
    m_Max = (m_Max >= m_IU_Zero)?m_Max:m_IU_Zero;

	return m_Max;
}*/
/*************************************************************
	过流中断处理程序（可屏蔽中断，电平触发）

28035 也可能是过压中断，需要软件判断；
28035 在发生过压后，会频繁的进入该中断， 验证这样没有明显的问题
*************************************************************/
void HardWareErrorDeal()
{
    int counter = 0;
#if (SOFTSERIES == MD380SOFT)
    int sum = 0, i;
#endif
    	
    TurnOffBrake();    
    gMainStatus.ErrFlag.bit.OvCurFlag = 1;		//发生了过流中断的标志   

    gMainCmd.FreqToFunc = gMainCmd.FreqSyn;

//......................................................  
    //软件立即触发AD采样，争取获得最准确的数据
    EALLOW;
    PieCtrlRegs.PIEIER1.bit.INTx1 = 0;           //  ADC1INT
	ADC_RESET_SEQUENCE;		    //复位AD的计数器
	ADC_CLEAR_INT_FLAG;			//首先清除中断标志位
	ADC_START_CONVERSION;		//软件启动AD
	EDIS;  
    
    while(ADC_END_CONVERSIN == 0)   // 等待AD转换完成
    {
        counter ++;
        if(counter > 50)
        {
            break;
        }
	}

    EALLOW;
    ADC_CLEAR_INT_FLAG;         // 清除AD中断
    ADC_RESET_SEQUENCE;
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;           //  ADC1INT    
    EDIS;  
    
//......................................................  

#if (SOFTSERIES == MD380SOFT)
    for(i = 0; i < 10; i++)                     // 需要2us， 母线肯定是不变
    {
        sum += GpioDataRegs.AIODAT.bit.AIO2;
    }
    if((sum < 5)&&(gUDC.uDC > gInvInfo.InvLowUDC))  // io口为低且母线电压采样值>欠压值 判断为过压，
    {          
        HardWareOverUDCDeal();                  //过压故障发生后只能手动复位tz中断标志位
        return;         // 过压在AD中断中处理, 2ms中有处理          
    }   
#endif

    //判断为过流故障       
	if((gMainStatus.RunStep == STATUS_SHORT_GND)||
	(gMainStatus.RunStep == STATUS_BEFORE_RUN_DETECT))
	{
		gShortGnd.ocFlag = 1;						//上电对地短路
	}
	else if((gError.ErrorCode.all & ERROR_OVER_CURRENT) != ERROR_OVER_CURRENT)
	{
	    if(gMainStatus.RunStep == STATUS_LOW_POWER)    // 欠压下不报过流
        {
            ;
        }
		else
		{
#if (SOFTSERIES == MD380SOFT)
			if(gBrake.BreResShotDetFlag == 1) //需检测制动电阻短路 停机状态照检测                       
			{                                      
	            gBrake.ErrCode = ERROR_OVER_CURRENT; 
	        }
		    else 
#endif
		    {
	            gError.ErrorCode.all |= ERROR_OVER_CURRENT ;    
	            gError.ErrorInfo[0].bit.Fault1 = 1;
#if(AIRCOMPRESSOR == 1)
				if((gMainCmd.RestarCnt <= 3)&&(gOverLoad.OverLoadPreventEnable == 1))
				{
				    gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
				}
#endif
	        }
			gLineCur.ErrorShow = OcCurrentInfo();	//硬件过流，记录故障电流
		}
	}
}

#if (SOFTSERIES == MD380SOFT)
/*************************************************************
	过压中断处理程序（不可屏蔽中断，上升沿触发）
*************************************************************/
void HardWareOverUDCDeal(void)					
{
	DisableDrive();								//封锁输出
	if((gMainStatus.RunStep == STATUS_SHORT_GND)||
	(gMainStatus.RunStep == STATUS_BEFORE_RUN_DETECT))
	{
		gShortGnd.ocFlag = 2;						//上电对地短路检测阶段的标志
	}
	else if(gUDC.uDC > gInvInfo.InvLowUDC)
	{
		gError.ErrorCode.all |= ERROR_OVER_UDC;		//过压处理
		if(gMainStatus.ErrFlag.bit.OvCurFlag == 0) //没进入过过压中断,过压时不区分制动电阻短路故障     
		{
            gError.ErrorInfo[0].bit.Fault2 = 2;
        }
        else                                       //硬件中断过压
        {
            gError.ErrorInfo[0].bit.Fault2 = 1;
        }
	}
    
    EALLOW;
	EPwm1Regs.TZCLR.bit.OST = 1;
	EPwm2Regs.TZCLR.bit.OST = 1;
	EPwm3Regs.TZCLR.bit.OST = 1;
	EDIS;

}
#endif

/************************************************************
直流制动程序：在载波周期中，通过PI调节器计算直流制动下的输出电?
直流制动的时候，只有一相开关管动作
************************************************************/
void RunCaseDcBrake(void)		
{
	int m_BrakeCur;
	Uint m_Udata,m_MaxCur;

    m_MaxCur = ((Ulong)gInvInfo.InvCurrent * 3277)>>12;     //80%变频器额定电流

    m_Udata = (41UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;
    if( gMotorInfo.Current > m_MaxCur )  //电机额定电流大于变频器额定电流时，以变频器额定电流为准
    {
        m_Udata = (((long)m_MaxCur) * (long)m_Udata) / gMotorInfo.Current;
    }
	if(gMainCmd.Command.bit.StartDC == 1)
	{		
		m_BrakeCur = gComPar.StartDCBrakeCur * m_Udata;			//4096/100 ~= 41
	}
	else if(gMainCmd.Command.bit.StopDC == 1)
	{
		m_BrakeCur = gComPar.StopDCBrakeCur * m_Udata;			//4096/100 ~= 41
	}

	gDCBrake.Time++;
	if(gDCBrake.Time < 2)		//直流制动的前几拍，按照定子电阻和电流计算电压
	{
		gOutVolt.Volt = ((Ulong)m_BrakeCur * (Ulong)gMotorExtPer.Rpm)>>16;  //采用同步机定子电阻计算
		gDCBrake.PID.Total = ((long)gOutVolt.Volt<<16);
	}
	else						//通过PI调节器控制直流制动电流
	{
		gDCBrake.Time = 10;
		gDCBrake.PID.Deta = m_BrakeCur - (int)gLineCur.CurPer;
        gDCBrake.PID.KP   = 1600/16;
		//gDCBrake.PID.KP   = 1600;
		gDCBrake.PID.KI   = 300;
        if( 16 < gInvInfo.InvTypeApply )  //大功率直流制动时KI减小一半，防止电流振荡。
        {
            gDCBrake.PID.KI = 150;
        }
        gDCBrake.PID.QP = 0;
        gDCBrake.PID.QI = 0;
		gDCBrake.PID.KD   = 0;
		gDCBrake.PID.Max  = 4096;
		gDCBrake.PID.Min  = 0;
		PID((PID_STRUCT *)&gDCBrake.PID);
		gOutVolt.Volt = gDCBrake.PID.Out>>16;		
	}
	gOutVolt.VoltApply = gOutVolt.Volt;  	
                                        //直流制动情况下，不修改输出电压相位角, 电压相位不更新
}

/************************************************************
	制动电阻控制
************************************************************/
void BrakeResControl(void)
{
#if (SOFTSERIES == MD380SOFT)
    if( gBrake.ErrCode != 0)//制动管故障状态不再开启制动管
#else
    if(((gLineCur.CurBaseInv < 6144)&&((gError.ErrorCode.all & ERROR_OVER_CURRENT)== ERROR_OVER_CURRENT))
	||((gError.ErrorCode.all & ERROR_OVERLOAD_BRAKE) == ERROR_OVERLOAD_BRAKE)
	||((gError.ErrorCode.all & ERROR_IGBT_SHORT_BRAKE) == ERROR_IGBT_SHORT_BRAKE))
#endif
    { 

        TurnOffBrake();				// 如果是制动管过流或者过载必须关断输出	
#if (SOFTSERIES == MD380SOFT)
        gBrake.Ontimetotal = 0;  //0
#endif
    }   
    else
    {      
        if (gMainCmd.Command.bit.Start == 0)    //同步机延迟关断，2K载频的话是10s
        {            
            if (gBrake.DelayClose > 40000)   
            {
                TurnOffBrake();	
                gBrake.DelayClose = 50000;
#if (SOFTSERIES == MD380SOFT)
                gBrake.Ontimetotal = 0;
#endif               
                return;
            }
        }

        if(gComPar.BrakeCoff == 100)
        {
            gBrake.OnCnt = 65535;
            gBrake.OffCnt = 0;
        }
        else if(gComPar.BrakeCoff == 0)
        {
            gBrake.OnCnt = 0;
            gBrake.OffCnt = 65535;
        }
        else if(gComPar.BrakeCoff <= 50)
        {
            gBrake.OnCnt = 1;
            gBrake.OffCnt = (100 - gComPar.BrakeCoff) / gComPar.BrakeCoff;  // 这样虽然不精确，但是简单
        }
        else    // 50-100
        {
            gBrake.OnCnt = (gComPar.BrakeCoff) / (100 - gComPar.BrakeCoff);
            gBrake.OffCnt = 1;
        }
        
    // 
    	if(gBrake.OnCnt == 0)
    	{
    		TurnOffBrake();
#if (SOFTSERIES == MD380SOFT)
            gBrake.Ontimetotal = 0;
#endif
    		return;
    	}
       
    // 判断本次是应该开通还是关断
    	if(gUDC.uDC < (gBrake.VoltageLim - 20))	
    	{
    		gBrake.Flag &= 0x0FFFC;		//清除0、1bit
    		gBrake.Cnt = 0;
    	}
    	else if((gBrake.Flag & 0x01) == 0)      //开通第一拍，置0、1bit
    	{
    		if(gUDC.uDC > gBrake.VoltageLim)	
    		{
    			gBrake.Flag |= 0x03;		
    			gBrake.Cnt = 0;
    		}
    	}
    	else        // on or off
    	{
    		gBrake.Cnt++;
    		if((gBrake.Flag & 0x02) == 0)       // off
    		{
    			if(gBrake.Cnt > gBrake.OffCnt)
    			{
    				gBrake.Flag |= 0x02;
    				gBrake.Cnt = 0;
    			}
    		}
    		else        // bit1 != 0, switch on
    		{
    			if(gBrake.Cnt > gBrake.OnCnt)
    			{
    				gBrake.Flag &= 0x0FFFD;
    				gBrake.Cnt = 0;
    			}
    		}
    	}

    // 开始执行制动电阻的导通和关断
    	if((gBrake.Flag & 0x02) == 0x02)        // bit1
    	{
    		TurnOnBrake();	
    		#if (SOFTSERIES == MD500SOFT)			//...开通    
    		gBrake.OnCounter ++;        	
    		#endif	
    	}
    	else
    	{
    		TurnOffBrake();				//...关断		
#if (SOFTSERIES == MD380SOFT)
            gBrake.Ontimetotal = 0;
#endif
    	}
		#if (SOFTSERIES == MD500SOFT)
		gBrake.TotalCounter ++;
        #endif
    }
}
