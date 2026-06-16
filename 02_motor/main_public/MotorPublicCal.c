/****************************************************************
�ļ����ܣ�����������������Ʒ�ʽ�޹صĶ���ģ��
�ļ��汾��
�������ڣ�

****************************************************************/
#include "MotorPublicCalInclude.h"
/************************************************************/
/********************ȫ�ֱ�������****************************/
DC_BRAKE_STRUCT			gDCBrake;	//ֱ���ƶ��ñ���
BRAKE_CONTROL_STRUCT	gBrake;		//�ƶ���������ñ���
SHORT_GND_STRUCT		gShortGnd;
JUDGE_POWER_LOW			gLowPower;	    //�ϵ绺���ж�ʹ�����ݽṹ
PM_POS_EST_STRUCT       gPMPosEst;
extern BEFORE_RUN_PHASE_LOSE_STRUCT gBforeRunPhaseLose;
void GetPMPosEst(void);
void GetEMFUamp(void);
void GetPMPosSpeedObst(void);
extern void ADCEndIsrTuneR1(void);
/************************************************************
	�ϵ�Եض�·�ж�
************************************************************/
void RunCaseShortGnd(void)
{
	gSubCommand.bit.OutputLostBeforeRun = 0;// gSubCommand.bit.OutputLost; �ϵ�Եض�·, ��������ȱ��
	gExtendCmd.bit.ShortGndBeforeRun	= 1;
	//gShortGnd.BaseUDC = gUDC.uDC;
	OutputPhaseLoseAndShortGndDetect();
}

/************************************************************
	��Ƶ�����ϵ紦�����̣��ж�ĸ�ߵ�ѹ�ȶ��������ϵ绺�����
�ж����ݣ�1��ĸ�ߵ�ѹ����Ƿѹ�� 
       && 2��ĸ�ߵ�ѹû�������� 
       && 3������200ms
		  4���ж�ĸ�ߵ�ѹ�ȶ�������ʱ200ms������ͣ��״̬��
************************************************************/
void RunCaseLowPower(void)
{
	Uint uDCLowLimt;
    Uint AftConRelay_Delay;
	s16 m_Return;
    static u16 m_DataBak = 0;
    
    ResetADCEndIsr();

	if((m_DataBak == 0)&&(gPGData.PGType == PG_TYPE_SPECIAL_UVW))    // �ڼ̵���û������֮ǰ��¼ʡ��ʽ������λ��
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
        AftConRelay_Delay = 130;    //���ʹ��ڵ���37kW,��ʱ60ms��Ϊ���ϵĻ�е����ʱ��
    }
    else
    {
        AftConRelay_Delay = 115;    //����С��37kW,��ʱ30ms��Ϊ���ϵĻ�е����ʱ��            
    }
	switch(gMainStatus.SubStep)
	{
		case 2://�ж�ĸ�ߵ�ѹ����Ƿѹ��&&��ѹû������&&����200ms
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

		case 3:	//200ms��ʱ
            IPMCheckInitPos();                             // ��¼�ϴε���ʱ�ĳ�ʼλ��
            m_Return = IPMPosAdjustStop();                   // ͨ��UVW����CD�ź�У���Ƕ�
           
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
                        gError.ErrorCode.all = 0;                   //Ƿѹ����Ҫ�������?
		                if(gMainStatus.ErrFlag.bit.OvCurFlag == 1)  //�޸�Ƿѹ���޷���������жϵĴ���
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
				    gPhase.IMPhase = GetTime() << 12;	//�ϵ�����ѡ���ʼ��λ
			    }
				ConnectRelay();				
//				gBuffResCnt += 30000;
//				gPhase.IMPhase = GetTime() << 12;	//�ϵ�����ѡ���ʼ��λ
			}
			break;

		default:
			DisConnectRelay();	
			ResetADCEndIsr();
            gError.ErrorCode.all |= ERROR_LOW_UDC;				//������־
        	gMainStatus.StatusWord.bit.LowUDC = 0;
			gMainStatus.StatusWord.bit.StartStop = 0;
			gLowPower.WaiteTime = 0;
			gLowPower.UDCOld = gUDC.uDCFilter;
			gMainStatus.SubStep = 2;
			break;
	}
}

/************************************************************
	�ж�UVW�����Ƿ���������״̬���������ñ�־
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
    	if(EPwm1Regs.TZFLG.bit.CBC == 1) 		//��ʾU�ദ��������״̬
    	{
    		EPwm1Regs.TZCLR.bit.CBC = 1;
    		gCBCProtect.Flag.bit.CBC_U = 1;
    	}
    	
    	if(EPwm2Regs.TZFLG.bit.CBC == 1) 		//��ʾV�ദ��������״̬
    	{
    		EPwm2Regs.TZCLR.bit.CBC = 1;
    		gCBCProtect.Flag.bit.CBC_V = 1;
    	}

    	if(EPwm3Regs.TZFLG.bit.CBC == 1) 		//�ʾW�ദ��������״̬
    	{
    		EPwm3Regs.TZCLR.bit.CBC = 1;
    		gCBCProtect.Flag.bit.CBC_W = 1;
    	} 
    }
	EDIS;
}

/*************************************************************
    ������ĵ�������
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
	ѡ����������е����ֵ
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
	�����жϴ������򣨿������жϣ���ƽ������

28035 Ҳ�����ǹ�ѹ�жϣ���Ҫ�����жϣ�
28035 �ڷ�����ѹ�󣬻�Ƶ���Ľ�����жϣ� ��֤����û�����Ե�����
*************************************************************/
void HardWareErrorDeal()
{
    int counter = 0;
#if (SOFTSERIES == MD380SOFT)
    int sum = 0, i;
#endif
    	
    TurnOffBrake();    
    gMainStatus.ErrFlag.bit.OvCurFlag = 1;		//�����˹����жϵı�־   

    gMainCmd.FreqToFunc = gMainCmd.FreqSyn;

//......................................................  
    //������������AD��������ȡ�����׼ȷ������
    EALLOW;
	#ifdef TARGET_GS32
	Interrupt_disable(INT_ADCA1);		//ADC结束中断
	#else
    PieCtrlRegs.PIEIER1.bit.INTx1 = 0;           //  ADC1INT
	#endif
	ADC_RESET_SEQUENCE;		    //��λAD�ļ�����
	ADC_CLEAR_INT_FLAG;			//��������жϱ�־λ
	ADC_START_CONVERSION;		//��������AD
	EDIS;  
    
    while(ADC_END_CONVERSIN == 0)   // �ȴ�ADת�����
    {
        counter ++;
        if(counter > 50)
        {
            break;
        }
	}

    EALLOW;
    ADC_CLEAR_INT_FLAG;         // ���AD�ж�
    ADC_RESET_SEQUENCE;
	#ifdef TARGET_GS32
	Interrupt_enable(INT_ADCA1);		//ADC结束中断
	#else
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;           //  ADC1INT    
	#endif
    EDIS;  
    
//......................................................  

#if (SOFTSERIES == MD380SOFT)
    for(i = 0; i < 10; i++)                     // ��Ҫ2us�� ĸ�߿϶��ǲ���
    {
        sum += GpioDataRegs.AIODAT.bit.AIO2;
    }
    if((sum < 5)&&(gUDC.uDC > gInvInfo.InvLowUDC))  // io��Ϊ����ĸ�ߵ�ѹ����ֵ>Ƿѹֵ �ж�Ϊ��ѹ��
    {          
        HardWareOverUDCDeal();                  //��ѹ���Ϸ�����ֻ���ֶ���λtz�жϱ�־λ
        return;         // ��ѹ��AD�ж��д���, 2ms���д���          
    }   
#endif

    //�ж�Ϊ��������       
	if((gMainStatus.RunStep == STATUS_SHORT_GND)||
	(gMainStatus.RunStep == STATUS_BEFORE_RUN_DETECT))
	{
		gShortGnd.ocFlag = 1;						//�ϵ�Եض�·
	}
	else if((gError.ErrorCode.all & ERROR_OVER_CURRENT) != ERROR_OVER_CURRENT)
	{
	    if(gMainStatus.RunStep == STATUS_LOW_POWER)    // Ƿѹ�²�������
        {
            ;
        }
		else
		{
#if (SOFTSERIES == MD380SOFT)
			if(gBrake.BreResShotDetFlag == 1) //�����ƶ������· ͣ��״̬�ռ��                       
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
			gLineCur.ErrorShow = OcCurrentInfo();	//Ӳ����������¼���ϵ���
		}
	}
}

#if (SOFTSERIES == MD380SOFT)
/*************************************************************
	��ѹ�жϴ������򣨲��������жϣ������ش�����
*************************************************************/
void HardWareOverUDCDeal(void)					
{
	DisableDrive();								//�������
	if((gMainStatus.RunStep == STATUS_SHORT_GND)||
	(gMainStatus.RunStep == STATUS_BEFORE_RUN_DETECT))
	{
		gShortGnd.ocFlag = 2;						//�ϵ�Եض�·���׶εı�־
	}
	else if(gUDC.uDC > gInvInfo.InvLowUDC)
	{
		gError.ErrorCode.all |= ERROR_OVER_UDC;		//��ѹ����
		if(gMainStatus.ErrFlag.bit.OvCurFlag == 0) //û�������ѹ�ж�,��ѹʱ�������ƶ������·����     
		{
            gError.ErrorInfo[0].bit.Fault2 = 2;
        }
        else                                       //Ӳ���жϹ�ѹ
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
ֱ���ƶ��������ز������У�ͨ��PI����������ֱ���ƶ��µ������?
ֱ���ƶ���ʱ��ֻ��һ�࿪�عܶ���
************************************************************/
void RunCaseDcBrake(void)		
{
	int m_BrakeCur;
	Uint m_Udata,m_MaxCur;

    m_MaxCur = ((Ulong)gInvInfo.InvCurrent * 3277)>>12;     //80%��Ƶ�������

    m_Udata = (41UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;
    if( gMotorInfo.Current > m_MaxCur )  //�����������ڱ�Ƶ�������ʱ���Ա�Ƶ�������Ϊ׼
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
	if(gDCBrake.Time < 2)		//ֱ���ƶ���ǰ���ģ����ն��ӵ���͵��������ѹ
	{
		gOutVolt.Volt = ((Ulong)m_BrakeCur * (Ulong)gMotorExtPer.Rpm)>>16;  //����ͬ�������ӵ������
		gDCBrake.PID.Total = ((long)gOutVolt.Volt<<16);
	}
	else						//ͨ��PI����������ֱ���ƶ�����
	{
		gDCBrake.Time = 10;
		gDCBrake.PID.Deta = m_BrakeCur - (int)gLineCur.CurPer;
        gDCBrake.PID.KP   = 1600/16;
		//gDCBrake.PID.KP   = 1600;
		gDCBrake.PID.KI   = 300;
        if( 16 < gInvInfo.InvTypeApply )  //����ֱ���ƶ�ʱKI��Сһ�룬��ֹ�����񵴡�
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
                                        //ֱ���ƶ�����£����޸������ѹ��λ��, ��ѹ��λ������
}

/************************************************************
	�ƶ��������
************************************************************/
void BrakeResControl(void)
{
#if (SOFTSERIES == MD380SOFT)
    if( gBrake.ErrCode != 0)//�ƶ��ܹ���״̬���ٿ����ƶ���
#else
    if(((gLineCur.CurBaseInv < 6144)&&((gError.ErrorCode.all & ERROR_OVER_CURRENT)== ERROR_OVER_CURRENT))
	||((gError.ErrorCode.all & ERROR_OVERLOAD_BRAKE) == ERROR_OVERLOAD_BRAKE)
	||((gError.ErrorCode.all & ERROR_IGBT_SHORT_BRAKE) == ERROR_IGBT_SHORT_BRAKE))
#endif
    { 

        TurnOffBrake();				// ������ƶ��ܹ������߹��ر���ض����	
#if (SOFTSERIES == MD380SOFT)
        gBrake.Ontimetotal = 0;  //0
#endif
    }   
    else
    {      
        if (gMainCmd.Command.bit.Start == 0)    //ͬ�����ӳٹضϣ�2K��Ƶ�Ļ���10s
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
            gBrake.OffCnt = (100 - gComPar.BrakeCoff) / gComPar.BrakeCoff;  // ������Ȼ����ȷ�����Ǽ�
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
       
    // �жϱ�����Ӧ�ÿ�ͨ���ǹض�
    	if(gUDC.uDC < (gBrake.VoltageLim - 20))	
    	{
    		gBrake.Flag &= 0x0FFFC;		//���0��1bit
    		gBrake.Cnt = 0;
    	}
    	else if((gBrake.Flag & 0x01) == 0)      //��ͨ��һ�ģ���0��1bit
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

    // ��ʼִ���ƶ�����ĵ�ͨ�͹ض�
    	if((gBrake.Flag & 0x02) == 0x02)        // bit1
    	{
    		TurnOnBrake();	
    		#if (SOFTSERIES == MD500SOFT)			//...��ͨ    
    		gBrake.OnCounter ++;        	
    		#endif	
    	}
    	else
    	{
    		TurnOffBrake();				//...�ض�		
#if (SOFTSERIES == MD380SOFT)
            gBrake.Ontimetotal = 0;
#endif
    	}
		#if (SOFTSERIES == MD500SOFT)
		gBrake.TotalCounter ++;
        #endif
    }
}
