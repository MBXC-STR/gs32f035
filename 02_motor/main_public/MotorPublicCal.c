/****************************************************************
锟侥硷拷锟斤拷锟杰ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷品锟绞斤拷薰氐亩锟斤拷锟侥ｏ拷锟�
锟侥硷拷锟芥本锟斤拷
锟斤拷锟斤拷锟斤拷锟节ｏ拷

****************************************************************/
#include "MotorPublicCalInclude.h"
/************************************************************/
/********************全锟街憋拷锟斤拷锟斤拷锟斤拷****************************/
DC_BRAKE_STRUCT			gDCBrake;	//直锟斤拷锟狡讹拷锟矫憋拷锟斤拷
BRAKE_CONTROL_STRUCT	gBrake;		//锟狡讹拷锟斤拷锟斤拷锟斤拷锟斤拷帽锟斤拷锟�
SHORT_GND_STRUCT		gShortGnd;
JUDGE_POWER_LOW			gLowPower;	    //锟较电缓锟斤拷锟叫讹拷使锟斤拷锟斤拷锟捷结构
PM_POS_EST_STRUCT       gPMPosEst;
extern BEFORE_RUN_PHASE_LOSE_STRUCT gBforeRunPhaseLose;
void GetPMPosEst(void);
void GetEMFUamp(void);
void GetPMPosSpeedObst(void);
extern void ADCEndIsrTuneR1(void);
/************************************************************
	锟较碉拷缘囟锟铰凤拷卸锟�
************************************************************/
void RunCaseShortGnd(void)
{
	gSubCommand.bit.OutputLostBeforeRun = 0;// gSubCommand.bit.OutputLost; 锟较碉拷缘囟锟铰�, 锟斤拷锟斤拷锟斤拷锟斤拷缺锟斤拷
	gExtendCmd.bit.ShortGndBeforeRun	= 1;
	//gShortGnd.BaseUDC = gUDC.uDC;
	OutputPhaseLoseAndShortGndDetect();
}

/************************************************************
	锟斤拷频锟斤拷锟斤拷锟较电处锟斤拷锟斤拷锟教ｏ拷锟叫讹拷母锟竭碉拷压锟饺讹拷锟斤拷锟斤拷锟斤拷锟较电缓锟斤拷锟斤拷锟�
锟叫讹拷锟斤拷锟捷ｏ拷1锟斤拷母锟竭碉拷压锟斤拷锟斤拷欠压锟斤拷 
       && 2锟斤拷母锟竭碉拷压没锟斤拷锟斤拷锟斤拷锟斤拷 
       && 3锟斤拷锟斤拷锟斤拷200ms
		  4锟斤拷锟叫讹拷母锟竭碉拷压锟饺讹拷锟斤拷锟斤拷锟斤拷时200ms锟斤拷锟斤拷锟斤拷停锟斤拷状态锟斤拷
************************************************************/
void RunCaseLowPower(void)
{
	Uint uDCLowLimt;
    Uint AftConRelay_Delay;
	s16 m_Return;
    static u16 m_DataBak = 0;
    
    ResetADCEndIsr();

	if((m_DataBak == 0)&&(gPGData.PGType == PG_TYPE_SPECIAL_UVW))    // 锟节继碉拷锟斤拷没锟斤拷锟斤拷锟斤拷之前锟斤拷录省锟斤拷式锟斤拷锟斤拷锟斤拷位锟斤拷
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
        AftConRelay_Delay = 130;    //锟斤拷锟酵达拷锟节碉拷锟斤拷37kW,锟斤拷时60ms锟斤拷为锟斤拷锟较的伙拷械锟斤拷锟斤拷时锟斤拷
    }
    else
    {
        AftConRelay_Delay = 115;    //锟斤拷锟斤拷小锟斤拷37kW,锟斤拷时30ms锟斤拷为锟斤拷锟较的伙拷械锟斤拷锟斤拷时锟斤拷            
    }
	switch(gMainStatus.SubStep)
	{
		case 2://锟叫讹拷母锟竭碉拷压锟斤拷锟斤拷欠压锟斤拷&&锟斤拷压没锟斤拷锟斤拷锟斤拷&&锟斤拷锟斤拷200ms
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

		case 3:	//200ms锟斤拷时
            IPMCheckInitPos();                             // 锟斤拷录锟较次碉拷锟斤拷时锟侥筹拷始位锟斤拷
            m_Return = IPMPosAdjustStop();                   // 通锟斤拷UVW锟斤拷锟斤拷CD锟脚猴拷校锟斤拷锟角讹拷
           
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
                        gError.ErrorCode.all = 0;                   //欠压锟斤拷锟斤拷要锟斤拷锟斤拷锟斤拷锟�?
		                if(gMainStatus.ErrFlag.bit.OvCurFlag == 1)  //锟睫革拷欠压锟斤拷锟睫凤拷锟斤拷锟斤拷锟斤拷锟斤拷卸系拇锟斤拷锟�
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
				    gPhase.IMPhase = GetTime() << 12;	//锟较碉拷锟斤拷锟斤拷选锟斤拷锟绞硷拷锟轿�
			    }
				ConnectRelay();				
//				gBuffResCnt += 30000;
//				gPhase.IMPhase = GetTime() << 12;	//锟较碉拷锟斤拷锟斤拷选锟斤拷锟绞硷拷锟轿�
			}
			break;

		default:
			DisConnectRelay();	
			ResetADCEndIsr();
            gError.ErrorCode.all |= ERROR_LOW_UDC;				//锟斤拷锟斤拷锟斤拷志
        	gMainStatus.StatusWord.bit.LowUDC = 0;
			gMainStatus.StatusWord.bit.StartStop = 0;
			gLowPower.WaiteTime = 0;
			gLowPower.UDCOld = gUDC.uDCFilter;
			gMainStatus.SubStep = 2;
			break;
	}
}

/************************************************************
	锟叫讹拷UVW锟斤拷锟斤拷锟角凤拷锟斤拷锟斤拷锟斤拷锟斤拷状态锟斤拷锟斤拷锟斤拷锟斤拷锟矫憋拷志
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
    	if(EPwm1Regs.TZFLG.bit.CBC == 1) 		//锟斤拷示U锟洁处锟斤拷锟斤拷锟斤拷锟斤拷状态
    	{
    		EPwm1Regs.TZCLR.bit.CBC = 1;
    		gCBCProtect.Flag.bit.CBC_U = 1;
    	}
    	
    	if(EPwm2Regs.TZFLG.bit.CBC == 1) 		//锟斤拷示V锟洁处锟斤拷锟斤拷锟斤拷锟斤拷状态
    	{
    		EPwm2Regs.TZCLR.bit.CBC = 1;
    		gCBCProtect.Flag.bit.CBC_V = 1;
    	}

    	if(EPwm3Regs.TZFLG.bit.CBC == 1) 		//锟绞網锟洁处锟斤拷锟斤拷锟斤拷锟斤拷状态
    	{
    		EPwm3Regs.TZCLR.bit.CBC = 1;
    		gCBCProtect.Flag.bit.CBC_W = 1;
    	} 
    }
	EDIS;
}

/*************************************************************
    锟斤拷锟斤拷锟斤拷牡锟斤拷锟斤拷锟斤拷锟�
*************************************************************/
Uint OcCurrentInfo(void)
{
    s16   Int_Iu,Int_Iv,Int_Iw;

    GetCurrentInfo();   

    Int_Iu = (s16)(gIUVWQ24.U >> 12);
    Int_Iv = (s16)(gIUVWQ24.V >> 12);
    Int_Iw = (s16)(gIUVWQ24.W >> 12);

    Int_Iu = (Int_Iu>0)?Int_Iu:(-Int_Iu);
    Int_Iv = (Int_Iv>0)?Int_Iv:(-Int_Iv);
    Int_Iw = (Int_Iw>0)?Int_Iw:(-Int_Iw);

    Int_Iu = (Int_Iu>Int_Iv)?Int_Iu:Int_Iv;
    Int_Iu = (Int_Iu>Int_Iw)?Int_Iu:Int_Iw;   

	return ((u16)Int_Iu);
}
/*************************************************************
	选锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷械锟斤拷锟斤拷值
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
	锟斤拷锟斤拷锟叫断达拷锟斤拷锟斤拷锟津（匡拷锟斤拷锟斤拷锟叫断ｏ拷锟斤拷平锟斤拷锟斤拷锟斤拷

28035 也锟斤拷锟斤拷锟角癸拷压锟叫断ｏ拷锟斤拷要锟斤拷锟斤拷锟叫断ｏ拷
28035 锟节凤拷锟斤拷锟斤拷压锟襟，伙拷频锟斤拷锟侥斤拷锟斤拷锟斤拷卸希锟� 锟斤拷证锟斤拷锟斤拷没锟斤拷锟斤拷锟皆碉拷锟斤拷锟斤拷
*************************************************************/
void HardWareErrorDeal()
{
    s16 counter = 0;
#if (SOFTSERIES == MD380SOFT)
    s16 sum = 0, i;
#endif
    	
    TurnOffBrake();    
    gMainStatus.ErrFlag.bit.OvCurFlag = 1;		//锟斤拷锟斤拷锟剿癸拷锟斤拷锟叫断的憋拷志   

    gMainCmd.FreqToFunc = gMainCmd.FreqSyn;

//......................................................  
    //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷AD锟斤拷锟斤拷锟斤拷锟斤拷取锟斤拷锟斤拷锟阶既凤拷锟斤拷锟斤拷锟�
    EALLOW;
	#ifdef TARGET_GS32
	Interrupt_disable(INT_ADCA1);		//ADC缁撴潫涓柇
	#else
    PieCtrlRegs.PIEIER1.bit.INTx1 = 0;           //  ADC1INT
	#endif
	ADC_RESET_SEQUENCE;		    //锟斤拷位AD锟侥硷拷锟斤拷锟斤拷
	ADC_CLEAR_INT_FLAG;			//锟斤拷锟斤拷锟斤拷锟斤拷卸媳锟街疚�
	ADC_START_CONVERSION;		//锟斤拷锟斤拷锟斤拷锟斤拷AD
	EDIS;  
    
    while(ADC_END_CONVERSIN == 0)   // 锟饺达拷AD转锟斤拷锟斤拷锟�
    {
        counter ++;
        if(counter > 50)
        {
            break;
        }
	}

    EALLOW;
    ADC_CLEAR_INT_FLAG;         // 锟斤拷锟紸D锟叫讹拷
    ADC_RESET_SEQUENCE;
	#ifdef TARGET_GS32
	Interrupt_enable(INT_ADCA1);		//ADC缁撴潫涓柇
	#else
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;           //  ADC1INT    
	#endif
    EDIS;  
    
//......................................................  

#if (SOFTSERIES == MD380SOFT)
    for(i = 0; i < 10; i++)                     // 锟斤拷要2us锟斤拷 母锟竭肯讹拷锟角诧拷锟斤拷
    {
        sum += GpioDataRegs.AIODAT.bit.AIO2;
    }
    if((sum < 5)&&(gUDC.uDC > gInvInfo.InvLowUDC))  // io锟斤拷为锟斤拷锟斤拷母锟竭碉拷压锟斤拷锟斤拷值>欠压值 锟叫讹拷为锟斤拷压锟斤拷
    {          
        HardWareOverUDCDeal();                  //锟斤拷压锟斤拷锟较凤拷锟斤拷锟斤拷只锟斤拷锟街讹拷锟斤拷位tz锟叫断憋拷志位
        return;         // 锟斤拷压锟斤拷AD锟叫讹拷锟叫达拷锟斤拷, 2ms锟斤拷锟叫达拷锟斤拷          
    }   
#endif

    //锟叫讹拷为锟斤拷锟斤拷锟斤拷锟斤拷       
	if((gMainStatus.RunStep == STATUS_SHORT_GND)||
	(gMainStatus.RunStep == STATUS_BEFORE_RUN_DETECT))
	{
		gShortGnd.ocFlag = 1;						//锟较碉拷缘囟锟铰�
	}
	else if((gError.ErrorCode.all & ERROR_OVER_CURRENT) != ERROR_OVER_CURRENT)
	{
	    if(gMainStatus.RunStep == STATUS_LOW_POWER)    // 欠压锟铰诧拷锟斤拷锟斤拷锟斤拷
        {
            ;
        }
		else
		{
#if (SOFTSERIES == MD380SOFT)
			if(gBrake.BreResShotDetFlag == 1) //锟斤拷锟斤拷锟狡讹拷锟斤拷锟斤拷锟铰� 停锟斤拷状态锟秸硷拷锟�                       
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
			gLineCur.ErrorShow = OcCurrentInfo();	//硬锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷录锟斤拷锟较碉拷锟斤拷
		}
	}
}

#if (SOFTSERIES == MD380SOFT)
/*************************************************************
	锟斤拷压锟叫断达拷锟斤拷锟斤拷锟津（诧拷锟斤拷锟斤拷锟斤拷锟叫断ｏ拷锟斤拷锟斤拷锟截达拷锟斤拷锟斤拷
*************************************************************/
void HardWareOverUDCDeal(void)					
{
	DisableDrive();								//锟斤拷锟斤拷锟斤拷锟�
	if((gMainStatus.RunStep == STATUS_SHORT_GND)||
	(gMainStatus.RunStep == STATUS_BEFORE_RUN_DETECT))
	{
		gShortGnd.ocFlag = 2;						//锟较碉拷缘囟锟铰凤拷锟斤拷锥蔚谋锟街�
	}
	else if(gUDC.uDC > gInvInfo.InvLowUDC)
	{
		gError.ErrorCode.all |= ERROR_OVER_UDC;		//锟斤拷压锟斤拷锟斤拷
		if(gMainStatus.ErrFlag.bit.OvCurFlag == 0) //没锟斤拷锟斤拷锟斤拷锟窖癸拷卸锟�,锟斤拷压时锟斤拷锟斤拷锟斤拷锟狡讹拷锟斤拷锟斤拷锟铰凤拷锟斤拷锟�     
		{
            gError.ErrorInfo[0].bit.Fault2 = 2;
        }
        else                                       //硬锟斤拷锟叫断癸拷压
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
直锟斤拷锟狡讹拷锟斤拷锟斤拷锟斤拷锟截诧拷锟斤拷锟斤拷锟叫ｏ拷通锟斤拷PI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷直锟斤拷锟狡讹拷锟铰碉拷锟斤拷锟斤拷锟�?
直锟斤拷锟狡讹拷锟斤拷时锟斤拷只锟斤拷一锟洁开锟截管讹拷锟斤拷
************************************************************/
void RunCaseDcBrake(void)		
{
	s16 m_BrakeCur;
	Uint m_Udata,m_MaxCur;

    m_MaxCur = ((Ulong)gInvInfo.InvCurrent * 3277)>>12;     //80%锟斤拷频锟斤拷锟筋定锟斤拷锟斤拷

    m_Udata = (41UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;
    if( gMotorInfo.Current > m_MaxCur )  //锟斤拷锟斤拷疃拷锟斤拷锟斤拷锟斤拷诒锟狡碉拷锟斤拷疃拷锟斤拷锟绞憋拷锟斤拷员锟狡碉拷锟斤拷疃拷锟斤拷锟轿�
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
	if(gDCBrake.Time < 2)		//直锟斤拷锟狡讹拷锟斤拷前锟斤拷锟侥ｏ拷锟斤拷锟秸讹拷锟接碉拷锟斤拷偷锟斤拷锟斤拷锟斤拷锟斤拷压
	{
		gOutVolt.Volt = ((Ulong)m_BrakeCur * (Ulong)gMotorExtPer.Rpm)>>16;  //锟斤拷锟斤拷同锟斤拷锟斤拷锟斤拷锟接碉拷锟斤拷锟斤拷锟�
		gDCBrake.PID.Total = ((long)gOutVolt.Volt<<16);
	}
	else						//通锟斤拷PI锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷直锟斤拷锟狡讹拷锟斤拷锟斤拷
	{
		gDCBrake.Time = 10;
		gDCBrake.PID.Deta = m_BrakeCur - (s16)gLineCur.CurPer;
        gDCBrake.PID.KP   = 1600/16;
		//gDCBrake.PID.KP   = 1600;
		gDCBrake.PID.KI   = 300;
        if( 16 < gInvInfo.InvTypeApply )  //锟斤拷锟斤拷直锟斤拷锟狡讹拷时KI锟斤拷小一锟诫，锟斤拷止锟斤拷锟斤拷锟今荡★拷
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
                                        //直锟斤拷锟狡讹拷锟斤拷锟斤拷拢锟斤拷锟斤拷薷锟斤拷锟斤拷锟斤拷压锟斤拷位锟斤拷, 锟斤拷压锟斤拷位锟斤拷锟斤拷锟斤拷
}

/************************************************************
	锟狡讹拷锟斤拷锟斤拷锟斤拷锟�
************************************************************/
void BrakeResControl(void)
{
#if (SOFTSERIES == MD380SOFT)
    if( gBrake.ErrCode != 0)//锟狡讹拷锟杰癸拷锟斤拷状态锟斤拷锟劫匡拷锟斤拷锟狡讹拷锟斤拷
#else
    if(((gLineCur.CurBaseInv < 6144)&&((gError.ErrorCode.all & ERROR_OVER_CURRENT)== ERROR_OVER_CURRENT))
	||((gError.ErrorCode.all & ERROR_OVERLOAD_BRAKE) == ERROR_OVERLOAD_BRAKE)
	||((gError.ErrorCode.all & ERROR_IGBT_SHORT_BRAKE) == ERROR_IGBT_SHORT_BRAKE))
#endif
    { 

        TurnOffBrake();				// 锟斤拷锟斤拷锟斤拷贫锟斤拷芄锟斤拷锟斤拷锟斤拷吖锟斤拷乇锟斤拷锟截讹拷锟斤拷锟�	
#if (SOFTSERIES == MD380SOFT)
        gBrake.Ontimetotal = 0;  //0
#endif
    }   
    else
    {      
        if (gMainCmd.Command.bit.Start == 0)    //同锟斤拷锟斤拷锟接迟关断ｏ拷2K锟斤拷频锟侥伙拷锟斤拷10s
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
            gBrake.OffCnt = (100 - gComPar.BrakeCoff) / gComPar.BrakeCoff;  // 锟斤拷锟斤拷锟斤拷然锟斤拷锟斤拷确锟斤拷锟斤拷锟角硷拷
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
       
    // 锟叫断憋拷锟斤拷锟斤拷应锟矫匡拷通锟斤拷锟角关讹拷
    	if(gUDC.uDC < (gBrake.VoltageLim - 20))	
    	{
    		gBrake.Flag &= 0x0FFFC;		//锟斤拷锟�0锟斤拷1bit
    		gBrake.Cnt = 0;
    	}
    	else if((gBrake.Flag & 0x01) == 0)      //锟斤拷通锟斤拷一锟侥ｏ拷锟斤拷0锟斤拷1bit
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

    // 锟斤拷始执锟斤拷锟狡讹拷锟斤拷锟斤拷牡锟酵拷凸囟锟�
    	if((gBrake.Flag & 0x02) == 0x02)        // bit1
    	{
    		TurnOnBrake();	
    		#if (SOFTSERIES == MD500SOFT)			//...锟斤拷通    
    		gBrake.OnCounter ++;        	
    		#endif	
    	}
    	else
    	{
    		TurnOffBrake();				//...锟截讹拷		
#if (SOFTSERIES == MD380SOFT)
            gBrake.Ontimetotal = 0;
#endif
    	}
		#if (SOFTSERIES == MD500SOFT)
		gBrake.TotalCounter ++;
        #endif
    }
}
