/***************************************************************************
锟侥硷拷锟斤拷锟杰ｏ拷同锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷识锟斤拷锟斤拷锟斤拷锟斤拷始位锟矫硷拷猓拷锟斤拷锟斤拷锟斤拷锟斤拷位锟矫★拷锟斤拷锟斤拷锟绞�
锟侥硷拷锟芥本锟斤拷
锟斤拷锟铰革拷锟铰ｏ拷

***************************************************************************/
//锟侥硷拷锟斤拷锟杰ｏ拷
//    1. 同锟斤拷锟斤拷锟脚硷拷位锟矫硷拷锟斤拷锟叫断筹拷锟斤拷锟�2ms锟斤拷锟斤拷 锟斤拷锟斤拷锟睫革拷ePWM模锟斤拷实锟街革拷锟洁发锟斤拷锟斤拷
//    2. 锟斤拷锟脚硷拷位锟斤拷时顺锟斤拷锟绞讹拷锟酵拷锟斤拷锟斤拷叩锟叫ｏ拷d锟斤拷q锟斤拷锟叫ｏ拷
//    3. 锟斤拷锟轿伙拷媒堑目锟斤拷乇锟绞讹拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
//    4. 锟斤拷锟轿伙拷媒堑拇锟斤拷乇锟绞讹拷锟�(*** 注锟斤拷锟斤拷要锟矫伙拷锟斤拷锟侥憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷探)
//    5. 锟皆憋拷锟斤拷锟斤拷锟斤拷支锟街帮拷锟斤拷ABZ锟斤拷UVW, 锟斤拷转锟斤拷压锟斤拷锟斤拷
//       UVW锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷uvw锟脚号碉拷锟斤拷锟轿憋拷呛锟絬vw锟脚号凤拷锟斤拷
//    6. 同锟斤拷锟斤拷锟斤拷锟界动锟狡的憋拷识锟斤拷

#include "MotorInclude.h"
#include "MotorIPMSVC.h"

/*******************锟结构锟斤拷锟斤拷锟斤拷******************************************/
IPM_ZERO_POS_STRUCT		gIPMZero;         //锟斤拷锟斤拷同锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷位锟矫角碉拷锟斤拷锟捷结构
IPM_INITPOS_PULSE_STR	gIPMInitPos;      // pmsm 锟斤拷识锟脚硷拷位锟斤拷
PMSM_EST_BEMF           gEstBemf;         // PMSM锟斤拷识锟斤拷锟界动锟斤拷锟斤拷锟斤拷
PMSM_EST_PARAM_DATA		gPmParEst;        // 锟斤拷锟斤拷同锟斤拷锟斤拷锟斤拷识时锟侥和癸拷锟杰碉拷锟斤拷锟捷斤拷锟斤拷锟斤拷

/*******************锟斤拷睾锟斤拷锟斤拷锟斤拷锟�******************************************/
void InitSetPosTune(void);
void DetectZeroPosOnce(void);
void SynInitPosDetSetTs(void);
void SynCalLabAndLbc(void);
void SynInitPosDetCal(void);
//void ADCEndIsrTuneR1(void);
void ADCEndIsrTune_POLSE_POS(void);
void SetIPMRefPos(Uint Pos);
void RsIdentify_ADC_Over_isr(void);
PID_STRUCT gRsIdentifyPID;
extern PMSM_FLUX_WEAK_STRUCT   gPmFluxWeak;
/************************************************************
	锟斤拷始位锟矫角硷拷锟斤拷锟斤拷锟斤拷始锟斤拷识锟斤拷锟斤拷锟斤拷锟斤拷锟轿伙拷媒堑某锟绞硷拷锟斤拷锟斤拷锟�
************************************************************/
void InitSetPosTune(void)
{
	ResetParForVC();
	ResetADCEndIsr();

    gPGDir.ABDirCnt = 0;                    // 锟斤拷位锟斤拷锟叫憋拷锟斤拷锟斤拷锟斤拷锟斤拷
    gPGDir.UVWDirCnt = 0;
    gPGDir.CDDirCnt = 0;
    gPGDir.RtDirCnt = 0;
    
    gPGDir.ABAngleBak = gIPMPos.RotorPos;
    gPGDir.UVWAngleBak = gUVWPG.UVWAngle;
    
	gIPMZero.TotalErr 	= 0;
	gIPMZero.DetectCnt  = 0;
    
	gMainStatus.PrgStatus.all = 0;
    gIPMZero.DetectCnt = 0;
	//gIPMPos.ZSigEnableApply = gIPMPos.ZSigEnable;
}

/*************************************************************
	pm 锟脚硷拷位锟矫憋拷识, 锟斤拷要锟斤拷锟叫讹拷锟叫筹拷锟斤拷锟斤拷锟�
锟斤拷识锟斤拷锟斤拷: 锟斤拷锟斤拷锟窖癸拷锟�, dq锟斤拷锟斤拷锟斤拷顺锟斤拷玫锟斤拷锟�
锟斤拷识时锟斤拷锟斤拷锟皆硷拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷状态锟斤拷
*************************************************************/
void SynTuneInitPos(void)
{	
	switch(gGetParVarable.IdSubStep)
	{
		case 1:                                             // 锟斤拷始锟斤拷锟斤拷锟斤拷
		    SetADCEndIsr(ADCEndIsrTune_POLSE_POS);
			gIPMInitPos.Waite = 0;
            //gIPMInitPos.InitPWMTs	= (50 * DSP_CLOCK);	    //50us
            //gIPMInitPos.InitPWMTs	= (50 * DSP_CLOCK);	    //50us		
			gGetParVarable.IdSubStep++;
			break;
            
		case 2:                                             // 锟斤拷时锟饺达拷20ms, 锟劫筹拷始锟斤拷锟斤拷乇锟斤拷锟斤拷锟斤拷锟絇WM锟斤拷锟斤拷
			gIPMInitPos.Waite ++;
			if(gIPMInitPos.Waite > 10)			
			{
				gIPMInitPos.Waite = 0;
       			gIPMInitPos.Step = 1;                       // 锟矫撅拷态锟斤拷识锟斤拷志
       			//gMainStatus.PrgStatus.bit.PWMDisable = 1;   // 锟斤拷EPWM 锟斤拷锟斤拷
       			gGetParVarable.IdSubStep++;
			}
			break;
            
		case 3:                                             // 锟饺达拷锟叫讹拷锟叫撅拷态锟斤拷识锟斤拷锟�...
			if(gIPMInitPos.Step == 0)
			{
				gIPMPos.CompPos = ((Ulong)gIPMPos.CompPosFun << 16) / 3600;
				gIPMPos.InitPos = gIPMPos.InitPos + gIPMPos.CompPos;
				SetIPMPos((Uint)gIPMPos.InitPos);
				SetIPMRefPos((Uint)gIPMPos.InitPos);
                SvcSetRotorPos(gIPMPos.InitPos);
                gPhase.IMPhase  = (s16)gIPMPos.InitPos;
                gPhase.IMPhaseApply = ((long)gPhase.IMPhase << 16);					
				gGetParVarable.IdSubStep++;
			}
			break;
            
		case 4:
			DisableDrive();
//			SynCalLdAndLq(gIPMPos.InitPos);//SynCalLdAndLq(gIPMPos.RotorPos);
			IPMCalAcrPIDCoff();
			gGetParVarable.IdSubStep++;
            gIPMInitPos.Waite  = 0;
			break;
            
		case 5:
		    if(gIPMInitPos.Waite < 500)
            {
                gIPMInitPos.Waite  ++;
            }
            else
            {
			    ResetADCEndIsr();
    			InitSetPWM(); 
    			gGetParVarable.IdSubStep = 1;
    		    gGetParVarable.ParEstMstep++;               //锟叫伙拷锟斤拷锟斤拷一锟斤拷识锟斤拷锟斤拷
				//gPhase.IMPhase  = (s16)gIPMPos.RotorPos;
                //gPhase.IMPhaseApply = ((long)gPhase.IMPhase << 16);					
                gMainStatus.PrgStatus.all = 0;
            }
			break;	

        default:
            break;
	}
}

/************************************************************
锟较碉拷锟绞嘉伙拷媒羌锟斤拷锟斤拷锟�(锟斤拷锟斤拷锟斤拷锟叫断ｏ拷锟斤拷AD转锟斤拷锟斤拷锟斤拷锟叫讹拷锟斤拷执锟斤拷)锟斤拷
锟斤拷U锟斤拷锟脚猴拷VW锟斤拷锟脚达拷为锟斤拷说锟斤拷锟斤拷
	1锟斤拷PWM1B锟斤拷PWM2A锟斤拷PWM3A强锟狡关闭ｏ拷锟竭碉拷平锟斤拷锟斤拷
	2锟斤拷锟斤拷始强锟斤拷锟斤拷锟斤拷PWM1A锟斤拷PWM2B锟斤拷PWM3B为锟竭碉拷平
	3锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷为0 (锟斤拷锟轿硷拷锟斤拷锟饺∑斤拷锟街�)
	4) 锟斤拷锟斤拷gIPMInitPos.Step为1锟斤拷始锟斤拷锟斤拷锟斤拷止位锟矫碉拷谐, 锟斤拷珊锟斤拷锟斤拷锟�0
************************************************************/
void SynInitPosDetect(void)
{
	s16  m_Cur;
    s16  m_Index;
	static Uint m_Wait = 0;
	Uint Sec,Cur_Lim,i,temp0,temp1;
	s16 m_Data;
	Ulong temp;

	switch(gIPMInitPos.Step)
	{
		case 1:							                    //锟斤拷始锟斤拷锟斤拷
			gIPMInitPos.PeriodCnt = 0;
			gIPMInitPos.Section   = 0;
			gIPMInitPos.PWMTs 	  = gIPMInitPos.InitPWMTs;
			gIPMInitPos.WidthFlag = 0;

			for(i=0;i<12;i++)
			{
				gIPMInitPos.PWMTsBak[i] = 0;
			}
			gIPMInitPos.CurLimitBig = (u16)(((u32)gIPMInitPos.CurLimitAdjBig << 12)/100UL);
			gIPMInitPos.CurLimitSmall = (u16)(((u32)gIPMInitPos.CurLimitAdjSmall << 12)/100UL);

			if(gInvInfo.InvCurrent < gMotorInfo.Current)
        	{
                temp = (Ulong)gIPMInitPos.CurLimitBig * gInvInfo.InvCurrent;                
        		gIPMInitPos.CurLimitBig = temp / gMotorInfo.Current;				// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷诒锟斤拷锟斤拷锟斤拷位锟矫憋拷识锟斤拷凸锟斤拷锟斤拷锟斤拷锟斤拷员锟绞�
                temp = (Ulong)gIPMInitPos.CurLimitSmall * gInvInfo.InvCurrent;                
        		gIPMInitPos.CurLimitSmall = temp / gMotorInfo.Current;				// 小锟斤拷锟斤拷锟斤拷锟斤拷锟节碉拷屑锟斤拷锟斤拷锟斤拷锟饺憋拷锟斤拷锟�
        	}
/*
			temp0 = (Uint)((Ulong)gMotorInfo.Current * gIPMInitPos.CurLimitAdjSmall / 100);	// 小锟斤拷锟斤拷实锟斤拷值
			temp1 = (Uint)((Ulong)gInvInfo.InvCurrent * 50 / 100);								// 锟斤拷频锟斤拷锟斤拷锟斤拷50%
			if(temp0 < temp1)									// 小锟斤拷锟斤拷锟斤拷小锟节憋拷频锟斤拷锟筋定锟斤拷锟斤拷锟斤拷50%, 锟斤拷锟斤拷锟节碉拷锟斤拷疃拷锟斤拷锟�
			{
				if(gMotorInfo.Current < temp1)
				{
					gIPMInitPos.CurLimitSmall = 4096;
				}
				else
				{
	        		gIPMInitPos.CurLimitSmall = (Uint)(((Ulong)gInvInfo.InvCurrent << 11) / gMotorInfo.Current);
				}
			}
*/
			SynInitPosDetSetTs();
			SynInitPosDetSetPwm(7);
			gIPMInitPos.PWMTs = 0;									// 为 case2 锟叫碉拷一锟轿凤拷锟斤拷锟斤拷准锟斤拷锟斤拷锟斤拷证锟斤拷一锟轿凤拷锟斤拷锟斤拷锟斤拷时

            gIPMInitPos.Step ++;
			break;

		case 2:							                            //锟斤拷锟斤拷锟绞碉拷锟斤拷锟斤拷锟斤拷锟�
			EnableDrive();
			
			if(gIPMInitPos.Section > 5)								// 确锟斤拷要锟斤拷锟侥碉拷压矢锟斤拷
			{
				Sec = gIPMInitPos.Section - 6;
				Cur_Lim = gIPMInitPos.CurLimitBig;
			}
			else
			{
				Sec = gIPMInitPos.Section;
				Cur_Lim = gIPMInitPos.CurLimitSmall;
			}

			if(Sec < 2) 											// 确锟斤拷要锟斤拷锟斤拷锟侥碉拷锟斤拷
			{
				m_Cur  = gIPMInitPos.CurIU;
			}
			else if(Sec < 4)
			{
				m_Cur  = gIPMInitPos.CurIV;
			}
			else
			{
				m_Cur  = gIPMInitPos.CurIW;
			}


			switch(gIPMInitPos.PeriodCnt)
			{
				case 0:
					m_Wait ++;

					if(m_Wait > (gIPMInitPos.PWMTs / 600))			// 锟斤拷锟斤拷锟斤拷前锟斤拷锟斤拷锟斤拷时
					{
						m_Wait = 0;
						SynInitPosDetSetPwm(Sec);                   // 锟斤拷锟斤拷锟斤拷
						gIPMInitPos.PeriodCnt ++;
					}

					break;
				case 1:
					if(abs(m_Cur) >= Cur_Lim)						// 锟斤拷锟斤拷锟斤拷锟斤拷锏斤拷瓒ㄖ�
					{
						gIPMInitPos.WidthFlag = 1;
					}

					if(gIPMInitPos.WidthFlag == 1)					// 锟斤拷獾揭伙拷锟斤拷锟斤拷锟�
					{
						gIPMInitPos.WidthFlag = 0;
						SynInitPosDetSetPwm(7);
						gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[gIPMInitPos.Section];		// 为锟斤拷锟劫次凤拷锟斤拷之前锟斤拷锟斤拷时
						gIPMInitPos.Section ++;												// 锟斤拷12锟斤拷锟斤拷锟斤拷锟斤拷为12锟斤拷锟斤拷
						if(gIPMInitPos.Section < 12)
						{
							gIPMInitPos.PeriodCnt = 0;
						}
						else
						{
							gIPMInitPos.PeriodCnt = 2;
						}
					}
					else											// 锟斤拷锟斤拷锟接筹拷锟斤拷锟斤拷时锟斤拷
					{
						gIPMInitPos.PWMTsBak[gIPMInitPos.Section] += gIPMInitPos.InitPWMTs;
						if(gIPMInitPos.PWMTsBak[gIPMInitPos.Section] >= 60000)				// 锟斤拷止锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷薹锟�
						{
							gIPMInitPos.PWMTsBak[gIPMInitPos.Section] = 60000;
							gIPMInitPos.WidthFlag = 1;
						}

	                    if((gIPMInitPos.PWMTsBak[gIPMInitPos.Section] >= 55000) && (m_Cur < (Cur_Lim>>2)&&(gIPMInitPos.ErrorIintPosEnable == 1)))  
	                    {
	                        gError.ErrorCode.all |= ERROR_INIT_POS;
	                        gError.ErrorInfo[3].bit.Fault3 = 1;
							gIPMInitPos.InitPWMTs = (20 * DSP_CLOCK);
	                    }
					}

					break;

				case 2:
					m_Wait ++;

					if(m_Wait > (gIPMInitPos.PWMTs / 600))							// 锟饺达拷锟斤拷锟斤拷锟铰斤拷锟斤拷0
					{
						m_Wait = 0;
					 	gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[0];

						for(i=1;i<6;i++)
						{
							if(gIPMInitPos.PWMTs > gIPMInitPos.PWMTsBak[i])			// 选锟斤拷锟斤拷锟绞碉拷小锟斤拷锟斤拷
							{
								gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[i];
							}
						}

						SynInitPosDetSetTs();
						gIPMInitPos.PeriodCnt = 0;
						gIPMInitPos.Section = 0;
						gIPMInitPos.Step++;
					}

					break;
			}
			break;

		case 3:													// 锟斤拷锟斤拷锟斤拷锟斤拷录锟斤拷锟斤拷(小)
			gIPMInitPos.PeriodCnt++;
			
			if(gIPMInitPos.PeriodCnt >= 3)  // step: 0-2
			{
				gIPMInitPos.PeriodCnt = 0;
			}

			if(gIPMInitPos.Section <= 1)
			{
				m_Cur  = gIPMInitPos.CurIU;
			}
			else if(gIPMInitPos.Section <= 3)
			{
				m_Cur  = gIPMInitPos.CurIV;
			}
			else
			{
				m_Cur  = gIPMInitPos.CurIW;
			}

			switch(gIPMInitPos.PeriodCnt)
			{
				case 0:
					SynInitPosDetSetPwm(7);
					m_Cur = m_Cur - gIPMInitPos.CurBase;
					gIPMInitPos.Cur[gIPMInitPos.Section] = abs(m_Cur);

                    gIPMInitPos.Section++;
					if(gIPMInitPos.Section == 6)
					{
						gIPMInitPos.Section = 0;
						gIPMInitPos.PWMTs4L = gIPMInitPos.PWMTs;	// 锟斤拷锟斤拷时锟斤拷锟斤拷锟节碉拷屑锟斤拷锟�
						gIPMInitPos.PWMTs = 6000;					// 锟斤拷一锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷泻锟叫★拷锟斤拷锟斤拷锟斤拷锟秸拷锟斤拷锟斤拷锟斤拷悴伙拷甑硷拷路锟斤拷锟斤拷锟揭伙拷锟斤拷锟斤拷始哟锟斤拷锟绞�
						SynInitPosDetSetTs();
						gIPMInitPos.Step++;
					}
					break;

				case 1:
					SynInitPosDetSetPwm(gIPMInitPos.Section);
					break;

				case 2:
				    SynInitPosDetSetPwm(7);
					gIPMInitPos.CurBase = m_Cur;
					break;
			}
			break;

		case 4:							// 锟斤拷锟斤拷锟叫猴拷凸锟斤拷锟绞ｏ拷锟叫断碉拷锟斤拷锟斤拷锟�
			SynCalLabAndLbc();
			SynCalLdAndLq();
			
			gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[6];

			for(i=7;i<12;i++)
			{
				if(gIPMInitPos.PWMTs > gIPMInitPos.PWMTsBak[i])
				{
					gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[i];
				}
			}
			
			gIPMInitPos.PWMTsBak[0] = gIPMInitPos.PWMTs;		// 为缺锟斤拷锟解保锟斤拷锟斤拷锟斤拷
			
			SynInitPosDetSetTs();
			gIPMInitPos.PeriodCnt = 0;
			gIPMInitPos.Section = 0;
			gIPMInitPos.Step ++;
			break;

		case 5:										// 锟斤拷锟斤拷锟斤拷锟斤拷录锟斤拷锟斤拷(锟斤拷)
			gIPMInitPos.PeriodCnt++;
			if(gIPMInitPos.PeriodCnt >= 3)  // step: 0-2
			{
				gIPMInitPos.PeriodCnt = 0;
			}

			if(gIPMInitPos.Section <= 1)
			{
				m_Cur  = gIPMInitPos.CurIU;
			}
			else if(gIPMInitPos.Section <= 3)
			{
				m_Cur  = gIPMInitPos.CurIV;
			}
			else
			{
				m_Cur  = gIPMInitPos.CurIW;
			}
			
			switch(gIPMInitPos.PeriodCnt)
			{
				case 0:
					SynInitPosDetSetPwm(7);
					m_Cur = m_Cur - gIPMInitPos.CurBase;
					m_Data = gIPMInitPos.Section + 6;
					gIPMInitPos.Cur[m_Data] = abs(m_Cur);

                    gIPMInitPos.Section++;
					if(gIPMInitPos.Section == 6)
					{
						gIPMInitPos.Section = 0;
						gIPMInitPos.Step++;
					}
					break;

				case 1:
					SynInitPosDetSetPwm(gIPMInitPos.Section);
					break;

				case 2:
				    SynInitPosDetSetPwm(7);
					gIPMInitPos.CurBase = m_Cur;
					break;
			}
			break;

		case 6:
			SynInitPosDetCal();
			
            if((gMainStatus.RunStep == STATUS_GET_PAR)||
            ((gUVCoff.RsTune == 2)&&(gUVCoff.BeforeRunTuneEnable == 1))||
            (gUVCoff.BeforeRunTuneEnable == 2))  //锟斤拷锟叫呈币诧拷卸锟饺憋拷锟�
            {
				gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[0];			// 为缺锟斤拷锟斤拷指锟斤拷锟斤拷锟�
				SynInitPosDetSetTs();
				SynInitPosDetSetPwm(7);
				gIPMInitPos.PhsChkStep = 0;
                gIPMInitPos.Step ++; // 锟斤拷锟斤拷同锟斤拷锟斤拷缺锟斤拷锟斤拷
            }
			else
            {
                gIPMInitPos.Step = 0;
                DisableDrive();
            }
			
			break;

        case 7:
            switch (gIPMInitPos.PhsChkStep)
            {
                case 0:
                    SynInitPosDetSetPwm(0);     // A+, B-
                    gIPMInitPos.PhsChkStep ++;
                    break;
				case 1:
                case 4:
                    SynInitPosDetSetPwm(7);
                    gIPMInitPos.PhsChkStep ++;
                    break;
                case 2:
                    SynInitPosDetSetPwm(7);
                    if(abs(gIPMInitPos.CurIU) < (gIPMInitPos.CurLimitBig>>3))    // U锟斤拷缺锟斤拷
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault4 = 11;
                        gIPMInitPos.Step ++;
                    }
                    if(abs(gIPMInitPos.CurIV) < (gIPMInitPos.CurLimitBig>>3))    // V锟斤拷缺锟斤拷
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault4 = 12;
                        gIPMInitPos.Step ++;
                    }
                    gIPMInitPos.PhsChkStep ++;
                    break;
                case 3:
                    SynInitPosDetSetPwm(2);     // B+ C-
                    gIPMInitPos.PhsChkStep ++;
                    break;
                case 5:
                    SynInitPosDetSetPwm(7);
                    if(abs(gIPMInitPos.CurIW) < (gIPMInitPos.CurLimitBig>>3))    // W锟斤拷缺锟斤拷
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault4 = 13;
                        gIPMInitPos.Step ++;
                    }
                    if(abs(gIPMInitPos.CurIV) < (gIPMInitPos.CurLimitBig>>3))    // V锟斤拷缺锟斤拷
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault4 = 12;
                        gIPMInitPos.Step ++;
                    }
                    gIPMInitPos.PhsChkStep ++;
                    break;

                default:
                    DisableDrive();
                    SynInitPosDetSetPwm(6);         // recove PWM regester
                    gIPMInitPos.Step ++;
                    break; 
            }   
            break;

		default:
			gIPMInitPos.Step = 0;           // 锟叫讹拷执锟斤拷锟斤拷锟�
			DisableDrive();
			break;
	}
}

/*************************************************************
	锟斤拷转锟接磁硷拷位锟斤拷强锟斤拷锟斤拷锟斤拷为Pos锟侥猴拷锟斤拷
*************************************************************/
void SetIPMPos(Uint Pos)
{	
    DINT;
	gIPMPos.QepBak   = GetQepCnt();	
    gIPMPos.RotorPos = Pos;
    EINT;
}
/*************************************************************
	锟斤拷锟斤拷gPGData.RefPos
*************************************************************/
void SetIPMRefPos(Uint Pos)
{	
    gPGData.RefPos = ((u32)Pos<<8);
    gAsr.PosSet = gPGData.RefPos;
}

/************************************************************
	LAB锟斤拷LBC锟斤拷LCA锟斤拷锟叫硷拷锟姐函锟斤拷
	gIPMInitPos.Cur锟斤拷锟芥：IU+锟斤拷IU-锟斤拷IV+锟斤拷IV-锟斤拷IW+锟斤拷IW-锟斤拷
	                     IU+锟斤拷IU-锟絀V+锟斤拷IV-锟斤拷IW+锟斤拷IW-
************************************************************/
void SynCalLabAndLbc(void)
{
	Uint  m_Index,m_Sel;
	Uint  m_DetaI,m_Cur,m_UDC;
	Ullong m_LTemp;

	//锟斤拷锟絃AB锟斤拷LBC锟斤拷LCA
	for(m_Index = 0;m_Index < 3;m_Index++)
	{
		m_Sel = m_Index<<1;
		m_Cur = (gIPMInitPos.Cur[m_Sel] <= gIPMInitPos.Cur[m_Sel+1]) ?
						gIPMInitPos.Cur[m_Sel] : gIPMInitPos.Cur[m_Sel+1];
		m_DetaI = m_Cur;
		m_DetaI = ((Ulong)m_DetaI * (Ulong)gMotorInfo.Current) >> 12;	//(锟斤拷位0.01A or 0.1A)
		m_UDC	= gUDC.uDCFilter - (Ulong)m_DetaI * (Ulong)gMotorExtReg.RsPm / 10000;      // 0.1V
		m_LTemp = (Ulong)gIPMInitPos.PWMTs4L * 2L*100L/DSP_CLOCK;				            //(锟斤拷位10ns)
		m_LTemp = (Ulong)m_UDC * (Ulong)m_LTemp;
		//L=((锟斤拷压/10)*(时锟斤拷/10^5)/(锟斤拷锟斤拷/100))*100 (0.01mH)
		//L = 锟斤拷压*时锟斤拷/(锟斤拷锟斤拷*100)  (0.01mH)
		gIPMInitPos.LPhase[m_Index] = (m_LTemp)/((Ulong)m_DetaI * 100);
	}
}

/************************************************************
	锟斤拷始位锟矫硷拷锟姐函锟斤拷锟斤拷同时锟斤拷锟斤拷锟斤拷锟斤拷锟叫★拷
************************************************************/
void SynInitPosDetCal(void)
{
	static s16  m_X,m_Y;
	Uint  m_Index;

	for(m_Index = 6;m_Index<12;)
	{
		gIPMInitPos.Cur[m_Index] = gIPMInitPos.Cur[m_Index+1] - gIPMInitPos.Cur[m_Index];
		m_Index = m_Index + 2;
	}

	m_X = gIPMInitPos.Cur[6] - (gIPMInitPos.Cur[8]>>1) - (gIPMInitPos.Cur[10]>>1);
	m_Y = ((long)(gIPMInitPos.Cur[8] - gIPMInitPos.Cur[10]) * 28378L)>>15;
    /*if((abs(m_X) + abs(m_Y) < (4096/40))&&(gIPMInitPos.ErrorIintPosEnable == 1))
    {
        gError.ErrorCode.all |= ERROR_INIT_POS;
        gError.ErrorInfo[3].bit.Fault3 = 2;
    }*///锟斤拷效锟斤拷:锟斤拷锟斤拷锟斤拷锟狡拷锟街拷锟叫★拷诘锟斤拷锟筋定锟斤拷锟斤拷锟斤拷1/40锟斤拷锟斤拷为锟睫凤拷识锟斤拷偶锟斤拷锟绞嘉伙拷锟�

	gIPMPos.InitPos = (Uint)user_atan(m_X, m_Y) - 5461;   // 30deg
	gIPMPos.InitAngle_deg = (Ulong)gIPMPos.InitPos * 3600 >> 16;
}

/************************************************************
	锟斤拷锟斤拷锟截诧拷锟斤拷锟节寄达拷锟斤拷锟斤拷锟斤拷
************************************************************/
void SynInitPosDetSetTs(void)
{
	EALLOW;
	EPwm1Regs.TBPRD = gIPMInitPos.PWMTs;
	//EPwm1Regs.CMPB  = gIPMInitPos.PWMTs;
	EPwm2Regs.TBPRD = gIPMInitPos.PWMTs;
	//EPwm2Regs.CMPB  = gADC.DelayApply;
	EPwm3Regs.TBPRD = gIPMInitPos.PWMTs;
	EDIS;
}

/************************************************************
	锟斤拷始位锟矫硷拷锟阶段ｏ拷锟斤拷锟斤拷PWM锟侥讹拷锟斤拷锟侥达拷锟斤拷锟斤拷锟斤拷锟斤拷PWM锟酵碉拷平锟斤拷效
************************************************************/
void SynInitPosDetSetPwm(Uint Section)
{
	Uint m_Section;
    
	m_Section = Section;

	EALLOW;
	switch(m_Section)
	{
		case 0:									// A+, B-
			EPwm1Regs.AQCSFRC.all = 0x09;       
			//pPWMForU->AQCSFRC.bit.CSFB = 2;	
			EPwm2Regs.AQCSFRC.all = 0x06;
			//pPWMForV->AQCSFRC.bit.CSFA = 2;
			EPwm3Regs.AQCSFRC.all = 0x0A;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		case 1:									// A-, B+
			EPwm1Regs.AQCSFRC.all = 0x06;
			//pPWMForU->AQCSFRC.bit.CSFA = 2;	
			EPwm2Regs.AQCSFRC.all = 0x09;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x0A;
			//pPWMForW->AQCSFRC.bit.CSFB = 2;
			break;

		case 2:									// B+, C-
			EPwm1Regs.AQCSFRC.all = 0x0A;
			//pPWMForU->AQCSFRC.bit.CSFA = 2;	
			EPwm2Regs.AQCSFRC.all = 0x09;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x06;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		case 3:									// B-, C+
			EPwm1Regs.AQCSFRC.all = 0x0A;
			//pPWMForU->AQCSFRC.bit.CSFB = 2;	
			EPwm2Regs.AQCSFRC.all = 0x06;
			//pPWMForV->AQCSFRC.bit.CSFA = 2;
			EPwm3Regs.AQCSFRC.all = 0x09;
			//pPWMForW->AQCSFRC.bit.CSFB = 2;
			break;

		case 4:									// A-, C+
			EPwm1Regs.AQCSFRC.all = 0x06;
			//pPWMForU->AQCSFRC.bit.CSFA = 2;	
			EPwm2Regs.AQCSFRC.all = 0x0A;
			//pPWMForV->AQCSFRC.bit.CSFA = 2;
			EPwm3Regs.AQCSFRC.all = 0x09;
			//pPWMForW->AQCSFRC.bit.CSFB = 2;
			break;

		case 5:									// A+, C-
			EPwm1Regs.AQCSFRC.all = 0x09;
			//pPWMForU->AQCSFRC.bit.CSFB = 2;
			EPwm2Regs.AQCSFRC.all = 0x0A;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x06;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		case 6:									//锟街革拷PWM模锟斤拷募拇锟斤拷锟斤拷锟斤拷锟�
			EPwm1Regs.DBCTL.all 	= 0x0007;
			EPwm1Regs.AQCTLA.all 	= 0x0090;
			EPwm1Regs.AQCTLB.all 	= 0x00;
			EPwm1Regs.AQCSFRC.all	= 0x00;

			EPwm2Regs.DBCTL.all 	= 0x0007;
			EPwm2Regs.AQCTLA.all 	= 0x0090;
			EPwm2Regs.AQCTLB.all 	= 0x00;
			EPwm2Regs.AQCSFRC.all 	= 0x00;

			EPwm3Regs.DBCTL.all 	= 0x0007;
			EPwm3Regs.AQCTLA.all 	= 0x0090;
			EPwm3Regs.AQCTLB.all 	= 0x00;
			EPwm3Regs.AQCSFRC.all 	= 0x00;
			break;

	case 8:									// A-, B-, C-, 锟斤拷锟斤拷转锟劫革拷锟斤拷, 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟脚诧拷锟斤拷
			EPwm1Regs.AQCSFRC.all = 0x06;
			//pPWMForU->AQCSFRC.bit.CSFB = 2;
			EPwm2Regs.AQCSFRC.all = 0x06;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x06;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		default:								//同锟斤拷锟斤拷锟斤拷始位锟矫角硷拷锟斤拷始锟斤拷锟侥达拷锟斤拷
			EPwm1Regs.AQCSFRC.all 	= 0x0A;		
			EPwm1Regs.DBCTL.all 	= 0;			
			//EPwm1Regs.AQCTLA.all 	= 0x000C;
			//EPwm1Regs.AQCTLB.all 	= 0x000C;

			EPwm2Regs.AQCSFRC.all 	= 0x0A;
			EPwm2Regs.DBCTL.all 	= 0;
			//EPwm2Regs.AQCTLA.all 	= 0x000C;
			//EPwm2Regs.AQCTLB.all 	= 0x000C;

			EPwm3Regs.AQCSFRC.all 	= 0x0A;
			EPwm3Regs.DBCTL.all 	= 0;
			//EPwm3Regs.AQCTLA.all 	= 0x000C;
			//EPwm3Regs.AQCTLB.all 	= 0x000C;
			break;
	}
	EDIS;
}

/*******************************************************************************
* Function Name  : 锟斤拷锟斤拷锟斤拷锟斤拷碌锟叫筹拷锟斤拷锟斤拷锟斤拷锟姐补锟斤拷锟角和憋拷锟斤拷锟斤拷锟侥斤拷锟竭凤拷式
* Description    : 	锟斤拷
锟斤拷锟斤拷转锟斤拷锟斤拷锟斤拷识锟斤拷锟斤拷锟斤拷锟斤拷慕锟斤拷叻锟绞斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷位锟矫角★拷
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SynTunePGZero_No_Load(void)
{
	s16	 m_Dir1,m_Dir2;
    s16  m_DetaPos,m_Data;
	u16  m_ZeroPos;
	s32  m_s32;
    static u16 m_ZNum = 0;
    static u16 m_WaiteCnt = 0;
	static u16 m_WaiteCnt1 = 0;
	static s32 m_TotalPosErr = 0;
	static s32 m_PosDeta = 0;
    
    gCtrMotorType = RUN_SYNC_TUNE;
	//PrepPmsmCsrPrar();	
    switch(gGetParVarable.IdSubStep)
	{
        case 1:
		    m_WaiteCnt = 0;
			m_PosDeta = 0;
            gPGData.PGDir    = gPGData.SpeedDir;
            gPmParEst.UvwDir = gUVWPG.UvwDir;
            gIPMPos.ZResetFlag = C_Z_DONT_RESET_POS;
    		InitSetPosTune();

            gPhase.IMPhase = (s16)gIPMPos.InitPos;
            gIMTSet.M = 0L <<12;
            gIMTSet.T = 0;
            gIPMZero.CurLimit = 3000UL * (Ulong)gMotorInfo.CurrentGet / gMotorInfo.Current;  //锟筋定锟斤拷锟斤拷70%锟斤拷锟斤拷
            if(gInvInfo.InvCurrent < gMotorInfo.Current)
            {
            	Ulong temp; 
                temp = (Ulong)gIPMZero.CurLimit * gInvInfo.InvCurrent;                
            	gIPMZero.CurLimit = temp / gMotorInfo.Current;
            }
			gGetParVarable.IdSubStep++;
            break;

        case 2:            
            EnableDrive();
            gIMTSet.M += (10L << 12);
            
            if(gIMTSet.M > ((long)gIPMZero.CurLimit <<12))      // M 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷檀锟斤拷锟�, 200ms
            {
                gIMTSet.M = (long)gIPMZero.CurLimit << 12;
                gGetParVarable.IdSubStep++;                
            }
			break;
                        
	 	case 3:		
            if(gMainCmd.FreqSet >= 0)
            {
                gPhase.IMPhase += (gMotorExtInfo.Poles * 11);       // 约5RPM锟斤拷锟劫讹拷转一圈
                gPGData.MoveDircetion = DIR_FORWARD;             //锟斤拷锟斤拷锟借定频锟斤拷为锟斤拷锟接帮拷锟�
            }
            else
            {
                gPhase.IMPhase -= (gMotorExtInfo.Poles * 11);       // 约5RPM锟斤拷锟劫讹拷转一圈
                gPGData.MoveDircetion = DIR_BACKWARD;
            }
            m_WaiteCnt++;
			
			m_Dir1 = JudgeABDir();											
            m_Dir2 = JudgeUVWDir();	

            if(m_WaiteCnt > 6000)                                   // 转一圈锟斤拷锟叫断斤拷锟斤拷
	        {
                m_WaiteCnt = 0;
                if(m_Dir1 == DIR_ERROR)    
                {
                    gError.ErrorCode.all |= ERROR_ENCODER;
                    gError.ErrorInfo[4].bit.Fault3 = 1;         // abz 锟脚号凤拷锟斤拷锟斤拷锟�
					m_WaiteCnt = 0;
                }      	        
                else if(m_Dir1 * gPGData.MoveDircetion < 0)              //AB锟斤拷锟斤拷取锟斤拷
                {                
                    gPGData.PGDir = !gPGData.PGDir;
					EQepRegs->QDECCTL.bit.SWAP = gPGData.PGDir;
					m_WaiteCnt = 0;
			    }   
                
                if(PG_TYPE_UVW == gPGData.PGType)
                {
                    if(m_Dir2 == DIR_ERROR)
                    {
                        gError.ErrorCode.all |= ERROR_ENCODER;
                        gError.ErrorInfo[4].bit.Fault3 = 2;         // abz 锟脚号凤拷锟斤拷锟斤拷锟�
                        m_WaiteCnt = 0;
                    }
                    else if(m_Dir2 * gPGData.MoveDircetion < 0)             //锟斤拷锟斤拷位锟矫凤拷锟斤拷取锟斤拷
                    {
                        gPmParEst.UvwDir = !gPmParEst.UvwDir;
    			    }  
                }
                
			    gGetParVarable.IdSubStep++;	
                if((gPGData.PGType == PG_TYPE_RESOLVER)||(gIPMPos.ZSigEnable == 0))
                {
                    gGetParVarable.IdSubStep++;                               // 锟斤拷锟戒不锟斤拷要锟饺达拷Z锟脚猴拷
                    gIPMPos.RotorPosBak = gIPMPos.RotorPos;             // 锟斤拷录锟斤拷锟阶拷锟轿伙拷锟�
					m_WaiteCnt = 0;
					m_WaiteCnt1 = 0;
                }
                gIPMPos.ZResetFlag = C_Z_RESET_POS_NOLIMIT;         // 锟斤拷示锟斤拷识锟斤拷时锟斤拷锟斤拷锟斤拷Z锟脚猴拷
                m_ZNum = gIPMPos.ZSigNum;
            }
			break;
            
		case 4:
            if(gMainCmd.FreqSet >= 0)
            {
                gPhase.IMPhase += (gMotorExtInfo.Poles * 22);       // 约10RPM锟斤拷锟劫讹拷转一圈
            }
            else
            {
                gPhase.IMPhase -= (gMotorExtInfo.Poles * 22);       // 约10RPM锟斤拷锟劫讹拷转一圈
            }
	        m_WaiteCnt++;
	        if(m_ZNum != gIPMPos.ZSigNum)                           // 锟饺达拷Z锟脚号把碉拷锟阶拷锟轿伙拷媒歉锟轿晃拷锟斤拷锟斤拷锟斤拷锟斤拷位锟矫斤拷
	        {
	            m_WaiteCnt = 0;
				m_WaiteCnt1 = 0;
                gIPMPos.RotorPosBak = gIPMPos.RotorPos;             // 锟斤拷录锟斤拷锟阶拷锟轿伙拷锟�
	            gGetParVarable.IdSubStep++;
	        }
	        else if(m_WaiteCnt > 9000)                       // 3圈锟斤拷没锟秸碉拷Z锟脚号ｏ拷锟斤拷锟斤拷锟斤拷
	        {
                gError.ErrorCode.all |= ERROR_ENCODER;
                gError.ErrorInfo[4].bit.Fault3 = 3;        
				m_WaiteCnt = 0;
	        }
			break;
            
		case 5:		
		    if(m_WaiteCnt1 < 5) 
		    {
		         m_WaiteCnt1++;
				 gIPMPos.RotorPosBak = gIPMPos.RotorPos;   // 为锟角讹拷偏锟斤拷锟绞硷拷锟�
		         break;
		    }
		    else
		    {
		        m_WaiteCnt1 = 5;                
		    }  
		     
            if(gMainCmd.FreqSet >= 0)
            {
                gPhase.IMPhase += (gMotorExtInfo.Poles * 6);       // 约5RPM锟斤拷锟劫讹拷转一圈
				m_Data = (s16)(gIPMPos.RotorPos - gIPMPos.RotorPosBak);     // 锟桔伙拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷位锟矫ｏ拷锟斤拷锟秸和革拷锟斤拷锟斤拷位锟矫斤拷锟叫比较ｏ拷锟叫断憋拷锟斤拷锟斤拷锟角凤拷未锟接伙拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
				m_PosDeta += m_Data;				
                gIPMPos.RotorPosBak = gIPMPos.RotorPos;
            }
            else
            {
                gPhase.IMPhase -= (gMotorExtInfo.Poles * 6);       // 约5RPM锟斤拷锟劫讹拷转一圈
				m_Data = (s16)(gIPMPos.RotorPosBak - gIPMPos.RotorPos);
				m_PosDeta += m_Data;
                gIPMPos.RotorPosBak = gIPMPos.RotorPos;
            }
            m_DetaPos = gPhase.IMPhase - (s16)gIPMPos.RotorPos;		// 锟斤拷锟斤拷锟斤拷锟斤拷锟轿伙拷媒锟�=锟脚硷拷锟斤拷实锟角讹拷-锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟角讹拷
    		m_WaiteCnt++;

            if(m_WaiteCnt == 2000)
            {
            	gIPMZero.FirstPos = m_DetaPos;                  //锟斤拷锟斤拷锟剿诧拷锟斤拷锟斤拷识之前锟斤拷锟斤拷锟斤拷锟斤拷锟借ǖ锟斤拷锟斤拷位锟矫角�?
                m_TotalPosErr = 0;
            }
            else if(m_WaiteCnt > 2000)
            {
            	m_TotalPosErr += (m_DetaPos - gIPMZero.FirstPos);
    	    	if(m_WaiteCnt > (2000 + 8192))
    	    	{
				    m_s32 = m_PosDeta - (s32)gMotorExtInfo.Poles * 61152L;      // 61152 = 10192*6
				    if((abs(m_s32) > 8192L)&&(gIPMPos.TuneDetaPosDisable == 0))            // 8192锟斤拷应45锟斤拷嵌锟�,锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟侥癸拷锟杰ｏ拷锟斤拷锟接癸拷锟斤拷锟斤拷锟斤拷锟轿此癸拷锟斤拷
					{
						gError.ErrorCode.all |= ERROR_ENCODER;
	                    gError.ErrorInfo[4].bit.Fault3 = 7;         // 锟斤拷锟斤拷锟脚号癸拷锟斤拷,锟斤拷锟斤拷未锟斤拷锟斤拷锟戒卡,锟斤拷锟竭憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟矫达拷锟斤拷
                        m_PosDeta = 0;
                        m_WaiteCnt = 0;
					}
					else
					{
					    m_PosDeta = 0;
					}
    	    		gIPMPos.RotorZero += (gIPMZero.FirstPos + (s16)(m_TotalPosErr>>13));
                    m_ZeroPos = (u16)(((u32)gIPMPos.RotorZero * 3600L)>>16);                   
                    gPmParEst.CoderPos_deg = m_ZeroPos;

    	    		m_WaiteCnt = 0;
    	    		gGetParVarable.IdSubStep++;

                    gIPMPos.ZResetFlag = C_Z_RESET_POS_LIMIT;
    	    	}
            }
			break;
            
		case 6:  //锟接筹拷一锟斤拷时锟斤拷却锟阶拷锟轿伙拷锟斤拷榷锟斤拷锟斤拷锟街革拷锟斤拷嵌龋锟斤拷锟铰硷拷锟绞憋拷慕嵌娶锟斤拷QepTotal 锟斤拷值
    		m_WaiteCnt++;
			if(m_WaiteCnt >= 250)
			{
    			m_WaiteCnt = 0;
        		DisableDrive();
    	        gIMTSet.M = 0;           
				SetIPMPos(gPhase.IMPhase);
				SetIPMRefPos(gPhase.IMPhase);	
				gPhase.IMPhaseApply = ((long)gPhase.IMPhase << 16);			
		        gIPMInitPos.Flag = 1;
        		gGetParVarable.IdSubStep++;
			}
			break;
            
		default:
			gGetParVarable.IdSubStep = 1;
		    gGetParVarable.ParEstMstep++;       //锟叫伙拷锟斤拷锟斤拷一锟斤拷识锟斤拷锟斤拷
		    break;
	}
	
}
/************************************************************
	锟斤拷锟截憋拷锟斤拷锟斤拷锟斤拷锟轿伙拷帽锟绞�
* 锟斤拷要锟秸伙拷矢锟斤拷锟斤拷锟叫ｏ拷
* 锟斤拷要锟斤拷锟斤拷锟斤拷细锟斤拷锟斤拷蛹锟斤拷俟锟斤拷锟狡碉拷矢锟斤拷锟�
* 目锟斤拷频锟绞ｏ拷 锟接硷拷锟斤拷时锟斤拷锟斤拷锟矫伙拷锟斤拷锟矫ｏ拷
* 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷要锟矫伙拷锟斤拷锟矫ｏ拷锟界不锟杰成癸拷锟斤拷锟斤拷要锟斤拷锟斤拷锟斤拷探锟斤拷
* uvw锟斤拷锟斤拷锟斤拷时锟斤拷uvw锟脚号的凤拷锟斤拷锟斤拷锟斤拷锟矫ｏ拷锟斤拷锟斤拷直锟接憋拷识锟矫筹拷锟斤拷
************************************************************/
void SynTunePGZero_Load(void)
{
	s16	 m_Dir2;
	u16  m_ZeroPos;
	u32  m_temp; 
    static u16 m_ZNum = 0;
    static u16 m_WaiteCnt = 0;
	static u16 m_WaiteCnt1 = 0;
    static u16 m_ZFlag = 0;
    static u16 m_DirFlag = 0;
	static u16 m_TuneNum = 0;
	static u16 m_Cnt = 0;
    static u16 m_CurLimit = 0;
	static u32 m_RTPosAdd = 0;
	static u16 m_It = 0;

    /*执锟斤拷同锟斤拷锟斤拷锟秸伙拷矢锟斤拷锟劫度伙拷*/

    CalTorqueLimitPar();
	PrepareAsrPar();
	gIMTSet.M = 0;
	
    gCtrMotorType = RUN_SYNC_TUNE;
    gPmParEst.SynTunePGLoad = 1;

    switch(gGetParVarable.IdSubStep)
	{
	    case 1:		   
	        if(gPGData.PGType == PG_TYPE_RESOLVER)
            {
				if(m_Cnt < 5)      // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷鹊锟�10ms锟斤拷锟饺诧拷锟斤拷锟斤拷锟斤拷锟劫斤拷锟叫达拷锟截憋拷识,wyk
				{
				    m_Cnt++;
					break;
				}
				else
				{
				    if(m_Cnt < 15)
					{
					    m_Cnt++;
					    m_RTPosAdd += gRotorTrans.RTPos;     // 锟桔伙拷锟斤拷平锟斤拷锟斤拷锟斤拷锟饺革拷锟竭ｏ拷wyk
						break;
					}
					else
					{
					    m_Cnt = 0;
		                gIPMPos.RotorZero = gIPMPos.InitPos - (u16)(m_RTPosAdd/10L);
	                    m_RTPosAdd = 0;
		                m_ZeroPos = (u16)(((u32)gIPMPos.RotorZero * 3600L)>>16);              
		                gPmParEst.CoderPos_deg = m_ZeroPos;
					}
				}
			}
			m_CurLimit = Max(abs(gAsr.PosTorqueLimit) - 200,0);   
			m_CurLimit = Min(m_CurLimit , 7373UL * (Ulong)gMotorInfo.CurrentGet / gMotorInfo.Current);  //锟斤拷锟斤拷疃拷锟斤拷锟�180%锟斤拷锟斤拷
		    if(gInvInfo.InvCurrent < gMotorInfo.Current)
		    {    	
		        m_temp = (Ulong)m_CurLimit * gInvInfo.InvCurrent;                
		    	m_CurLimit = (u16)(m_temp / gMotorInfo.Current);
		    }
		    		
            gPmParEst.UvwDir = gUVWPG.UvwDir;
            gIPMPos.ZResetFlag = C_Z_DONT_RESET_POS;
    		InitSetPosTune();
            m_ZNum = gIPMPos.ZSigNum;
            m_ZFlag = 0;
            m_DirFlag = 0;
            m_WaiteCnt = 0;
			m_It = 0;
    		EnableDrive();
            gGetParVarable.StatusWord = TUNE_ACC;    //锟斤拷始锟斤拷锟斤拷
             
            gGetParVarable.IdSubStep++;

            break;

	    case 2:
            m_Dir2 = JudgeUVWDir();	
            JudgeABDir();
            if((gIPMPos.ZSigNum - m_ZNum) >=3)
            {
                if(gPGData.PGType != PG_TYPE_RESOLVER)
				{
	                gIPMPos.RotorZero = gIPMPos.ZBakRotorPos;
	                m_ZeroPos = ((s32)gIPMPos.RotorZero * 3600L)>>16;               
	                gPmParEst.CoderPos_deg = m_ZeroPos;
				}
                m_ZNum = gIPMPos.ZSigNum;
                m_ZFlag = 1;
            }
            if(PG_TYPE_UVW != gPGData.PGType)
            {
                m_DirFlag = 1;
            }
            else if(abs(gPGDir.ABDirCnt) > 36)           // 锟斤拷锟阶拷锟揭蝗︼拷锟斤拷卸暇锟斤拷锟轿伙拷锟斤拷藕锟斤拷欠锟斤拷锟饺�
            {
                if(m_Dir2 == DIR_ERROR)
                {
                    gError.ErrorCode.all |= ERROR_ENCODER;
                    gError.ErrorInfo[4].bit.Fault3 = 4;         // abz 锟脚号凤拷锟斤拷锟斤拷锟�
                }
                if(m_Dir2 * gMainCmd.FreqSet < 0)                  
                {
                    gPmParEst.UvwDir = !gPmParEst.UvwDir;
			    }    
                m_DirFlag = 1;
            }		
            m_It = Filter64(abs(gIMTSet.T >> 12),m_It);  //锟斤拷锟剿诧拷,锟斤拷止锟斤拷
            if((((m_It >= m_CurLimit)&&(abs(gRotorSpeed.SpeedApply) <= 10))||(abs(gRotorSpeed.SpeedApply) >= (1000L<<15/gBasePar.FullFreq01)))&&(gIPMPos.LoadTuneErrorEnable == 1))
			{
				gPGData.PGDir = !gPGData.PGDir;
				EQepRegs->QDECCTL.bit.SWAP = gPGData.PGDir;
				DisableDrive();                         //停止锟斤拷锟斤拷
				ResetAsrPar();
			    gGetParVarable.IdSubStep = 5;
			    gGetParVarable.StatusWord = TUNE_DEC;    //锟斤拷始锟斤拷锟斤拷
			    m_WaiteCnt = 0;
				m_WaiteCnt1 = 0;
				m_TuneNum++;
			    break;		
			}
            
            if(abs(gPGDir.ABDirCnt) > 180 * gMotorExtInfo.Poles)  // 锟斤拷械锟斤拷5圈
            {
                gError.ErrorCode.all |= ERROR_ENCODER;
                gError.ErrorInfo[4].bit.Fault3 = 5;         
            }
            if((m_DirFlag == 1) && (m_ZFlag == 1))
            {
        		m_WaiteCnt = 0;
				if(m_TuneNum == 1)                    // 锟侥变方锟斤拷锟叫筹拷晒锟斤拷锟斤拷锟斤拷锟斤拷谐锟斤拷锟斤拷锟斤拷wyk
				{
				    m_TuneNum = 0;
				}
                gGetParVarable.StatusWord = TUNE_DEC;    //锟斤拷始锟斤拷锟斤拷
			    gGetParVarable.IdSubStep++;
            }
	        break;

		case 3:                                 // 锟斤拷锟劫阶讹拷
			if(abs(gMainCmd.FreqSet) <= 10)
            {
                m_WaiteCnt++;
                if(m_WaiteCnt > 250)
                {
                    m_WaiteCnt = 0;
					DisableDrive();
                    gGetParVarable.IdSubStep++;
                }
            }
			break;
            
		case 4:
            if(m_WaiteCnt < 10)
            {
                m_WaiteCnt++;
            }
            else
            {
    			gIPMInitPos.Flag = 1;               //rt 
                m_WaiteCnt = 0;
                gIPMPos.ZResetFlag = C_Z_RESET_POS_LIMIT;
                gMainStatus.PrgStatus.all = 0;
    			gGetParVarable.ParEstMstep++; //锟叫伙拷锟斤拷锟斤拷一锟斤拷识锟斤拷锟斤拷
    			gGetParVarable.IdSubStep = 1;   
    			gPmParEst.SynTunePGLoad = 0;             
            }                
	        break;

		case 5:
		    if(abs(gRotorSpeed.SpeedApply) <= 10)
			{
			    m_WaiteCnt++;
			    if(m_TuneNum >= 2)
				{
				    gError.ErrorCode.all |= ERROR_TUNE_FAIL;
					gError.ErrorInfo[4].bit.Fault2 = 1; 
					m_TuneNum = 0;
					gPmParEst.SynTunePGLoad = 0;
				}				
				if(m_WaiteCnt > 1000)   // 锟饺达拷锟斤拷锟斤拷锟斤拷锟节伙拷锟斤拷锟斤拷锟叫筹拷锟斤拷锟绞讹拷锟轿伙拷酶锟阶硷拷锟絯yk
	            {
	                m_WaiteCnt = 0;
	                gGetParVarable.ParEstMstep--; //锟斤拷锟铰斤拷锟叫筹拷始位锟矫角憋拷识锟酵达拷锟截憋拷识
					gGetParVarable.IdSubStep = 1; 					
	            }
			}
			else
			{
			    m_WaiteCnt1++;
				if(m_WaiteCnt1 > 5000)   // 锟劫度诧拷为0锟斤拷锟斤拷锟斤拷10锟斤拷
				{
				    gError.ErrorCode.all |= ERROR_TUNE_FAIL;
					gError.ErrorInfo[4].bit.Fault2 = 2; 
					m_WaiteCnt1 = 0;
					gPmParEst.SynTunePGLoad = 0;
				}
			}

    	default:
    	    break;
	}
}

/*************************************************************
	同锟斤拷锟斤拷锟斤拷锟界动锟狡憋拷识
* 锟斤拷要锟斤拷锟斤拷锟秸伙拷锟斤拷戏锟組锟斤拷锟斤拷锟斤拷锟�
* 锟斤拷锟斤拷锟斤拷锟斤拷锟轿�: IdSet = 1500, 3000(Q12)锟斤拷
* 锟斤拷锟斤拷锟较硷拷录d锟斤拷锟斤拷锟斤拷锟皆硷拷锟斤拷锟斤拷叩锟斤拷锟斤拷锟絨锟斤拷锟斤拷锟斤拷锟叫★拷锟�
* 锟斤拷锟斤拷锟较硷拷录q锟斤拷锟窖癸拷锟皆硷拷锟�?锟斤拷)锟斤拷压锟斤拷d锟斤拷锟窖癸拷锟叫★拷锟�
* 锟津化放筹拷: Uq 锟斤拷 w(Ld * Id + Phi_r);

* 锟接硷拷锟劫的加硷拷锟劫癸拷锟斤拷频锟斤拷指锟斤拷 锟缴憋拷识锟斤拷锟斤拷锟斤拷锟斤拷锟�
* 目锟斤拷频锟绞ｏ拷锟接硷拷锟斤拷时锟斤拷锟缴癸拷锟杰诧拷锟斤拷锟斤拷锟斤拷锟斤拷:
    目锟斤拷频锟斤拷:   40% 锟斤拷锟斤拷疃ㄗ拷伲锟�
    锟接硷拷锟斤拷时锟斤拷: 锟斤拷锟斤拷22锟斤拷锟斤拷: 30sec锟斤拷锟斤拷锟斤拷22锟斤拷锟斤拷: 50sec锟斤拷
*************************************************************/
void SynTuneBemf()
{
    Ulong temp1, temp2, temp3;
    Ulong fluxRotor;
	static u16 m_Waite = 0;

    gCtrMotorType = RUN_SYNC_TUNE;       //

    switch(gGetParVarable.IdSubStep)
    {
        case 1:
            m_Waite++;
			gEstBemf.IdSet = 0;
			gEstBemf.IdSetFilt = 0; 
			gEstBemf.TuneFreqSet = 0;
			gIPMPos.ZResetFlag = C_Z_DONT_RESET_POS;
			gEstBemf.Cnt = 0;
			if(m_Waite > 10)
			{
			    m_Waite = 0;
	            gEstBemf.TotalId1 = 0;
	            gEstBemf.TotalId2 = 0;
	            gEstBemf.TotalVq1 = 0;
	            gEstBemf.TotalVq2 = 0;

	            gEstBemf.IdSet = 1500;      // Q12;
	            gEstBemf.IqSet = 0;
	            //gEstBemf.IdSetFilt = 0; 	                       
	            if(gInvInfo.InvCurrent < gMotorInfo.Current)
	            {
	                temp1 = (Ulong)gEstBemf.IdSet * gInvInfo.InvCurrent;
	                gEstBemf.IdSet = temp1 / gMotorInfo.Current;
	            }
	            ResetParForVC();
				ResetADCEndIsr();

	            //gMainStatus.PrgStatus.bit.ASRDisable = 1;
	            //gGetParVarable.StatusWord = TUNE_ACC;       // 锟斤拷锟杰匡拷始锟斤拷锟斤拷
	            gEstBemf.TuneFreqAim = (long)gMotorInfo.FreqPer * 2L / 5L;      // 40% 锟斤拷锟斤拷疃ㄆ碉拷锟�
	            gEstBemf.FreqRem = 0;                                           // 频锟绞诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	            gEstBemf.AccDecTick = (gInvInfo.InvTypeApply <= 20) ? (30L*1000L/2) : (60L*1000L/2);
	                                                                    // 锟斤拷锟斤拷22锟斤拷锟斤拷: 30sec 锟斤拷锟劫碉拷锟筋定频锟斤拷
	                                                                    // 锟斤拷锟斤拷22锟斤拷锟斤拷: 60sec 锟斤拷锟劫碉拷锟筋定频锟斤拷
	            //gEstBemf.TuneFreqSet = 0;
	            //EnableDrive();
	            gGetParVarable.IdSubStep ++;
			}
            break;
            
        case 2:                        // 锟斤拷锟节硷拷锟劫癸拷锟斤拷
            EnableDrive();
            //if(speed_CON)
            gEstBemf.FreqStep = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) / gEstBemf.AccDecTick;
            gEstBemf.FreqRem = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) % gEstBemf.AccDecTick;
            
            if(gEstBemf.TuneFreqSet < gEstBemf.TuneFreqAim)
            {
                gEstBemf.TuneFreqSet += gEstBemf.FreqStep;
            }
            else
            {
                gEstBemf.TuneFreqSet = gEstBemf.TuneFreqAim;
                gEstBemf.Cnt ++;              
            }
            if(gEstBemf.Cnt > 500)
            {
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 3:                         // 锟斤拷锟街碉拷锟斤拷锟酵碉拷压----1
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt <= 2048)
            {
                gEstBemf.TotalId1 += gLineCur.CurPer;           // 锟斤拷锟斤拷锟斤拷锟轿拷锟街�
                gEstBemf.TotalVq1 += gOutVolt.VoltApply;        // 锟斤拷锟斤拷锟窖刮拷锟街�
            }
            else
            {
                gEstBemf.TotalId1 = gEstBemf.TotalId1 >> 11;
                gEstBemf.TotalVq1 = gEstBemf.TotalVq1 >> 11;

                gEstBemf.TuneFreq = abs(gMainCmd.FreqSyn);
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 4:                     // 锟睫改碉拷锟斤拷值
            gEstBemf.IdSet = 3000;
            if(gInvInfo.InvCurrent < gMotorInfo.Current)
            {
                temp1 = (Ulong)gEstBemf.IdSet * gInvInfo.InvCurrent;
                gEstBemf.IdSet = temp1 / gMotorInfo.Current;
            }
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt > 500)
            {
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 5:                         // 锟斤拷锟街碉拷锟斤拷锟斤拷压2
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt <= 2048)
            {
                gEstBemf.TotalId2 += gLineCur.CurPer;           // 锟斤拷锟斤拷锟斤拷锟轿拷锟街�
                gEstBemf.TotalVq2 += gOutVolt.VoltApply;        // 锟斤拷锟斤拷锟窖刮拷锟街�
            }
            else
            {
                gEstBemf.TotalId2 = gEstBemf.TotalId2 >> 11;
                gEstBemf.TotalVq2 = gEstBemf.TotalVq2 >> 11;

                gEstBemf.Cnt = 0;
                //gGetParVarable.StatusWord = TUNE_DEC;
                gEstBemf.TuneFreqAim = 0;
                gEstBemf.FreqRem = 0;     
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 6:                         // 锟饺达拷锟斤拷锟斤拷
            gEstBemf.FreqStep = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) / gEstBemf.AccDecTick;
            gEstBemf.FreqRem = ((long)gMotorInfo.FreqPer + gEstBemf.FreqRem) % gEstBemf.AccDecTick;
            
            if(gEstBemf.TuneFreqSet > gEstBemf.TuneFreqAim)
            {
                gEstBemf.TuneFreqSet -= gEstBemf.FreqStep;
            }
            else
            {
                gEstBemf.TuneFreqSet = 0;
                DisableDrive();
                gEstBemf.Cnt  = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;
            
        case 7:
            if(gEstBemf.Cnt < 500)          // 锟饺达拷停锟斤拷停锟斤拷
            {
                gEstBemf.Cnt ++;
            }
            else
            {        
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 8:                         // 锟斤拷锟姐反锟界动锟斤拷系锟斤拷锟斤拷锟斤拷杀锟绞�
            temp1 = (Ulong)gEstBemf.TotalVq2 * gEstBemf.TotalId1;
            temp2 = (Ulong)gEstBemf.TotalVq1 * gEstBemf.TotalId2;
            temp3 = (gEstBemf.TotalId2 - gEstBemf.TotalId1) * gEstBemf.TuneFreq;
            fluxRotor = (((Ullong)temp2 - (Ullong)temp1) <<15) /temp3; // Q: 12+12-15 + 15-12 = Q12

            temp3 = fluxRotor * abs(gMotorInfo.FreqPer) >> 12;        // Q15
            gEstBemf.BemfVolt = temp3 * gMotorInfo.Votage * 10L >> 15;              // 0.1 V

            gMainStatus.PrgStatus.all = 0;
            gGetParVarable.IdSubStep = 1;
			gGetParVarable.ParEstMstep ++; //锟叫伙拷锟斤拷锟斤拷一锟斤拷识锟斤拷锟斤拷

            SetIPMPos(gIPMPos.RotorPos);
			gIPMPos.ZResetFlag = C_Z_RESET_POS_LIMIT;
			break;

        default:
            break;
    }            
    gMainCmd.FreqSyn = gEstBemf.TuneFreqSet;
    gEstBemf.IdSetFilt = Filter8(gEstBemf.IdSet, gEstBemf.IdSetFilt);
    gIMTSet.M = (long)gEstBemf.IdSetFilt << 12;       // Q12->Q24
    gMainCmd.FreqToFunc = gMainCmd.FreqSyn;
}

/****************************************************************
锟斤拷锟斤拷锟斤拷锟杰ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟�?
*****************************************************************/
/*void RsIdentify(void)
{
	Uint  m_UData1,m_UData2,m_UData3,m_MaxCur;
	Ulong m_ULong1,m_ULong2;
 
    SetADCEndIsr(ADCEndIsrTuneR1);

    m_MaxCur = ((Ulong)gInvInfo.InvCurrent * 3277)>>12;     //80%锟斤拷频锟斤拷疃拷锟斤拷?
	switch(gGetParVarable.IdSubStep)
	{
		case 1:     // 锟斤拷识锟斤拷锟接碉拷锟斤拷时锟斤拷锟斤拷锟侥达拷锟斤拷, EPWM模锟斤拷锟斤拷锟斤拷锟�, 锟斤拷锟斤拷锟侥筹拷始锟斤拷
#if (SOFTSERIES == MD380SOFT)
		    if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD_1140; /*1140V锟斤拷锟接碉拷锟斤拷锟绞匡拷锟狡碉拷潭锟�1K,2011.5.7 L1082*/
/*              }
            else if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD_690; /*690V锟斤拷锟接碉拷锟斤拷锟绞匡拷锟狡碉拷潭锟�1.5K锟斤拷2011.6.20 L1082 */
/*              }
            else
#endif
              {
               gUVCoff.Rs_PRD = TUNE_Rs_PRD;         /*380V锟斤拷锟接碉拷锟斤拷锟绞匡拷锟狡碉拷潭锟�2K锟斤拷2011.7.19  L1082 */
/*              }  
			gUVCoff.Comper     = 0;//gUVCoff.Rs_PRD/2;
			gUVCoff.Number     = 0;
			gUVCoff.TotalU     = 0;
			gUVCoff.TotalV     = 0;
			gUVCoff.TotalI     = 0;
			gUVCoff.TotalIL    = 0;
			gUVCoff.TotalVolt  = 0;
			gUVCoff.TotalVoltL = 0;
            
			EALLOW;
            //EPwm1Regs.ETSEL.bit.INTEN = 0;  //锟斤拷锟接碉拷锟斤拷锟绞妒憋拷锟斤拷锟街癸拷锟斤拷锟斤拷卸锟�
			EPwm1Regs.TBPRD = gUVCoff.Rs_PRD;
			//EPwm1Regs.CMPB  = EPwm1Regs.TBPRD - gADC.DelayApply;
			EPwm2Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm3Regs.TBPRD = gUVCoff.Rs_PRD;

			EPwm1Regs.CMPA.half.CMPA = gUVCoff.Comper;
			EPwm2Regs.CMPA.half.CMPA = 0;//gUVCoff.Comper;

			//EPwm3Regs.AQCSFRC.bit.CSFA   = 2;
			//EPwm3Regs.AQCSFRC.bit.CSFB   = 2;
			EPwm3Regs.AQCSFRC.all = 0x0A;		//锟截憋拷3锟脚憋拷
			EPwm3Regs.DBCTL.all   = 0x0000;
			EDIS;
            
			gGetParVarable.IdSubStep = 2;               
			break;

		case 2:         // 锟斤拷锟轿憋拷识时锟斤拷时之锟斤拷锟劫匡拷锟斤拷锟斤拷锟斤拷
            if(0 == gUVCoff.IdRsDelay)
            {
			    gGetParVarable.IdSubStep = 3;
			    EnableDrive();
            }
            else
            {
                gUVCoff.IdRsDelay--;
            }
			break;
            
		case 4:
			gUVCoff.Number++;
			if(gUVCoff.Number >= 512)
			{
				gUVCoff.TotalVoltL += gUDC.uDCFilter;
				gUVCoff.TotalIL    += abs(gIUVWQ12.U);
			}
			if(gUVCoff.Number >= 1024)
			{
				gUVCoff.ComperL = gUVCoff.Comper;
				gGetParVarable.IdSubStep = 5;
			}
			break;
			
		case 3:
		case 5:
            
			if(16 < gInvInfo.InvTypeApply)
			{
			    m_UData1 = (1024UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;
			}
			else
			{
		 	    m_UData1 = (2048UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;//锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷?锟斤拷锟斤拷锟较碉拷锟斤拷疃拷锟斤拷锟�
			}

			if(0 == gUVCoff.IdRsCnt)            // 锟斤拷一锟轿憋拷识锟斤拷锟皆硷拷小锟斤拷锟斤拷锟斤拷锟斤拷止锟斤拷锟斤拷
			{ 
			    m_UData1 = (512UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current;     
			}

			if(gGetParVarable.IdSubStep == 5)
            {                
                if( 16 < gInvInfo.InvTypeApply )
                {
                    m_UData1 = (3500UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;  //锟斤拷锟斤拷锟斤拷时锟斤拷锟斤拷锟斤拷
                }
				else
				{
				    m_UData1 = (4096UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;
				}
            }
            if( gMotorInfo.Current > m_MaxCur )  //锟斤拷锟斤拷疃拷锟斤拷锟斤拷锟斤拷诒锟狡碉拷锟斤拷疃拷锟斤拷锟绞憋拷锟斤拷员锟狡碉拷锟斤拷疃拷锟斤拷锟轿�
            {
                m_UData1 = (((long)m_MaxCur) * (long)m_UData1) / gMotorInfo.Current;
            }
            
			gUVCoff.Number = 0;
			if(abs(gIUVWQ12.U) >= m_UData1)
			{
				gGetParVarable.IdSubStep++;
			}
			else
			{
			    if(gInvInfo.InvTypeApply > 16 )
                {         
                   gUVCoff.Comper += 1;         
                }
                else
                {
				   gUVCoff.Comper += 2;
                }
				if(gUVCoff.Comper >= gUVCoff.Rs_PRD)      // 125us
				{
			        gUVCoff.Comper = 0;//gUVCoff.Rs_PRD/2;  
			        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                    gError.ErrorInfo[1].bit.Fault4 = 10;
				}
				else
				{
					EALLOW;
					EPwm1Regs.CMPA.half.CMPA = gUVCoff.Comper;
					//EPwm2Regs.CMPA.half.CMPA = gUVCoff.Rs_PRD - gUVCoff.Comper;
					EDIS;
				}
			}
			break;

		case 6:
			gUVCoff.TotalU += abs(gIUVWQ12.U);
			gUVCoff.TotalV += abs(gIUVWQ12.V);
			gUVCoff.Number++;
			if(gUVCoff.Number >= 512)
			{
				gUVCoff.TotalVolt += gUDC.uDCFilter;
				gUVCoff.TotalI += abs(gIUVWQ12.U);

				if(gUVCoff.Number >= 1024)
				{
                    DisableDrive();
					gUVCoff.Number = 0;
					m_UData1 = (gUVCoff.TotalU << 4) / (Uint)(gUVCoff.TotalV >> 8);

					m_UData2 = gUVCoff.Rs_PRD;
					m_UData3 = gDeadBand.DeadBand / 2;
					m_UData1 = (gUVCoff.TotalVoltL<<5)/m_UData2;
					m_ULong1 = ((Ulong)m_UData1 * (Ulong)(gUVCoff.ComperL - m_UData3));
					m_UData1 = (gUVCoff.TotalVolt<<5)/m_UData2;
					m_ULong1 = ((Ulong)m_UData1 * (Ulong)(gUVCoff.Comper  - m_UData3)) - m_ULong1;
                    m_ULong1 =  m_ULong1 * 10;//锟斤拷压锟斤拷一锟斤拷小锟斤拷锟姐，锟斤拷锟�?锟斤拷小锟斤拷锟姐，锟斤拷锟斤拷要锟斤拷锟斤拷10
					//m_ULong1 = 锟斤拷压锟斤拷(V)锟斤拷10锟斤拷2^14
					m_UData2 = (gUVCoff.TotalI - gUVCoff.TotalIL)>>7;
					m_ULong2 = ((Ulong)m_UData2 * (Ulong)gMotorInfo.Current)>>10;
					//m_ULong2 = 锟斤拷锟斤拷锟斤拷(A)锟斤拷10锟斤拷2^4
					while((m_ULong2>>16) != 0)
					{
						m_ULong1 = m_ULong1>>1;
						m_ULong2 = m_ULong2>>1;
					}
					m_UData2 = m_ULong1/(Uint)m_ULong2;
                    m_UData2 = ((Ulong)m_UData2 * (Ulong)1000)>>11;
                    if(0 == gUVCoff.IdRsCnt)
                    {
                        gUVCoff.IdRsBak = m_UData2;
                        gGetParVarable.IdSubStep = 1;
                        gUVCoff.IdRsCnt++;
                        gUVCoff.IdRsDelay = 1000; //锟节讹拷锟轿憋拷识之前锟斤拷锟斤拷时2s
                    }
                    else
                    {
                        gMotorExtReg.RsPm = m_UData2;
					    gGetParVarable.IdSubStep = 7;
                    }
				}
			}
			break;
            
		case 7:		
		    ResetADCEndIsr(); 		
		default:
			DisableDrive();
  			gGetParVarable.IdSubStep = 1;
            gGetParVarable.ParEstMstep++; 
            InitSetPWM();                       //锟街革拷锟睫改的寄达拷锟斤拷锟斤拷锟斤拷
			break;
	}	
}
*/
void BeforeRunRsIdentify(void)
{
   //	Ulong Data1;
	Ulong  m_UData1,m_UData2,m_UData3,m_UData4,m_UData5,m_UData7;
	Ullong m_ULong1,m_ULong2,m_ULong3,m_ULong4,m_ULong5,m_ULong6;
	Uint m_Data;
 //	Uint  m_MaxCur;
//	static Uint m_TimeAdd = 0;
	//m_MaxCur = ((Ulong)gInvInfo.InvCurrent * 3277)>>12; 
	if(STATUS_RS_TUNE == gMainStatus.RunStep)  //锟斤拷锟斤拷锟斤拷识时锟接筹拷锟斤拷锟接碉拷锟斤拷锟绞妒憋拷锟�
	{
	    if(gInvInfo.InvTypeApply > 18)   //37Kw锟斤拷锟斤拷锟斤拷锟接讹拷锟接碉拷锟斤拷锟绞妒憋拷锟�
		{
		    m_Data = 1024;	// 2^10
		}
		else
		{
		    m_Data = 512;  //512, 2^9
		}
	} 
	else
	{
		    m_Data = 4096;	// 2^12
	}
	if(gMainCmd.Command.bit.Start == 0)	
	{
		gMainStatus.RunStep = STATUS_STOP;
		gUVCoff.RsTune = 0;
		gUVCoff.Flag = 0;
	}
	switch(gGetParVarable.IdSubStep)
	{
		case 1:
			gUVCoff.Comper	= gDeadBand.DeadBand>>1;
			gUVCoff.Number	= 0;
			gUVCoff.TotalU	= 0;
			gUVCoff.TotalV     	= 0;
			gUVCoff.TotalI     	= 0;
			gUVCoff.TotalIL    	= 0;
			gUVCoff.TotalVolt  	= 0;
			gUVCoff.TotalVoltL	=0;
			gUVCoff.TotalComperL	= 0;
			gUVCoff.TotalComper	= 0;
//			gUVCoff.Rs_PRD = TUNE_Rs_PRD;    // 2K
#if (SOFTSERIES == MD380SOFT)
		    if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
            {
                gUVCoff.Rs_PRD =SHORT_GND_PERIOD_1140; /*1140V锟斤拷锟接碉拷锟斤拷锟绞匡拷锟狡碉拷潭锟�1K,2011.5.7 L1082*/
            }
            else if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
            {
                gUVCoff.Rs_PRD =SHORT_GND_PERIOD_690; /*690V锟斤拷锟接碉拷锟斤拷锟绞匡拷锟狡碉拷潭锟�1.5K锟斤拷2011.6.20 L1082 */
            }
            else
#endif
            {
                gUVCoff.Rs_PRD = TUNE_Rs_PRD;         /*380V锟斤拷锟接碉拷锟斤拷锟绞匡拷锟狡碉拷潭锟�2K锟斤拷2011.7.19  L1082 */
            }  		

            if( gMotorInfo.Current > gInvInfo.InvCurrent )  //锟斤拷锟斤拷疃拷锟斤拷锟斤拷锟斤拷诒锟狡碉拷锟斤拷疃拷锟斤拷锟绞憋拷锟斤拷员锟狡碉拷锟斤拷疃拷锟斤拷锟轿�
            {
                gUVCoff.CurComperCoff = ((long)gInvInfo.InvCurrent *600L) / gMotorInfo.Current;
            }
			else
			{
			    gUVCoff.CurComperCoff = 600; 
			}
			DisableDrive();
			SetADCEndIsr(RsIdentify_ADC_Over_isr);
			EALLOW;    
			EPwm1Regs.TBPRD = gUVCoff.Rs_PRD;  //锟斤拷锟斤拷锟斤拷效
			EPwm2Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm3Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm1Regs.AQSFRC.all  = 0xC0;
            EPwm2Regs.AQSFRC.all  = 0xC0;
            EPwm3Regs.AQSFRC.all  = 0xC0;
			EPwm1Regs.AQCSFRC.all 	= 0x00;		//锟斤拷锟斤拷锟斤拷效
			EPwm1Regs.DBCTL.all 	= 0x0007;
			
			EPwm2Regs.CMPA.half.CMPA= 0;			//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷效
			EPwm2Regs.AQCSFRC.all 	= 0x0;	
			EPwm2Regs.DBCTL.all 	= 0x0007;      //锟斤拷锟斤拷锟斤拷效
			
			EPwm3Regs.AQCSFRC.all 	= 0x0A;
			EPwm3Regs.DBCTL.all 	= 0x0000;
				
			EPwm1Regs.CMPA.half.CMPA = gUVCoff.Comper;	
			//EPwm3Regs.CMPA.half.CMPA= 0;		
			EDIS;
			gGetParVarable.IdSubStep++;
			//EnableDrive();
			break;

		case 2:		
            EnableDrive();
			if(gUVCoff.Number > (m_Data>>1))			
			{				
				if( gMotorInfo.Current > gInvInfo.InvCurrent )  //锟斤拷锟斤拷疃拷锟斤拷锟斤拷锟斤拷诒锟狡碉拷锟斤拷疃拷锟斤拷锟�, 锟皆憋拷频锟斤拷锟筋定锟斤拷锟斤拷为准
	            {
	                gUVCoff.CurComperCoff = ((long)gInvInfo.InvCurrent <<11) / gMotorInfo.Current;
	            }
				else
				{
				    gUVCoff.CurComperCoff = 2048; 
				}
				gGetParVarable.IdSubStep++;
				gUVCoff.Number = 0;
			}
			break;
			
		case 3:
            if(gUVCoff.Number > m_Data)	//512		
			{
				if( gMotorInfo.Current > gInvInfo.InvCurrent )  //锟斤拷锟斤拷疃拷锟斤拷锟斤拷锟斤拷诒锟狡碉拷锟斤拷疃拷锟斤拷锟绞憋拷锟斤拷员锟狡碉拷锟斤拷疃拷锟斤拷锟轿�
	            {
	                gUVCoff.CurComperCoff = ((long)gInvInfo.InvCurrent <<12) / gMotorInfo.Current;
	            }
				else
				{
				    gUVCoff.CurComperCoff = 4096; 
				}
				gGetParVarable.IdSubStep++;
				gUVCoff.Number = 0;
			    gUVCoff.TotalVolt   = 0;
				gUVCoff.TotalI      = 0;
				gUVCoff.TotalComper = 0;
			}

            break;
		case 4:
			if(gUVCoff.Number > m_Data)	//512		
			{	
//				m_UData0 = (gUVCoff.TotalI << 4) / (gUVCoff.TotalIV >> 8);			// 没锟斤拷

			    if(STATUS_GET_PAR == gMainStatus.RunStep)					// 锟斤拷锟斤拷锟斤拷时锟斤拷 m_UData7
				{
				    m_UData7	= ((long)gDeadBand.DeadBand)<<11; 		// 2^12 / 2
				}
				else
				{
				    if(gInvInfo.InvTypeApply > 18)
					{
					    m_UData7	= ((long)gDeadBand.DeadBand)<<9;
					}
					else
					{
					    m_UData7	= ((long)gDeadBand.DeadBand)<<8;
					}
				}
				m_UData1	= gUVCoff.TotalComperL- m_UData7;				// Case3 锟斤拷时锟斤拷 - 锟斤拷锟斤拷时锟斤拷
				m_UData2 	= (gUVCoff.TotalVoltL<<3)/gUVCoff.Rs_PRD;	// 时锟戒划锟斤拷
				m_ULong1 	= ((Ullong)m_UData1 * m_UData2)>>7;				// 锟斤拷去锟斤拷锟斤拷锟侥碉拷压, 2^8	>>11
				
				//m_UData3	= gUVCoff.TotalComper;
				m_UData3	= gUVCoff.TotalComper - m_UData7;
				m_UData4 	= (gUVCoff.TotalVolt<<3)/gUVCoff.Rs_PRD;
				m_ULong2 	= ((Ullong)m_UData3 * m_UData4)>>7;				// >>11
				m_ULong3 	=  m_ULong2 - m_ULong1;//锟斤拷压锟斤拷锟斤拷一锟斤拷小锟斤拷锟姐，锟斤拷锟斤拷2锟斤拷小锟斤拷锟姐，锟斤拷锟斤拷要锟斤拷锟斤拷10
				m_ULong4    = m_ULong3 *10;			// m_ULong4 锟斤拷压之锟斤拷, 0.01V, 2^8
				//gUVCoff.m_ULong4 = gUVCoff.m_ULong2 *10;
				//m_ULong1 = 锟斤拷压锟斤拷(V)锟斤拷10锟斤拷2^14
                if(STATUS_GET_PAR == gMainStatus.RunStep)						// m_UData5 锟斤拷锟斤拷锟斤拷锟街�
				{
				    m_UData5	= (gUVCoff.TotalI - gUVCoff.TotalIL)>>4;	// 2^8
				}
				else
				{
					if(gInvInfo.InvTypeApply > 18)
					{
					    m_UData5 = (gUVCoff.TotalI - gUVCoff.TotalIL)>>6;  // 2^6
					}
					else
					{
					    m_UData5 = (gUVCoff.TotalI - gUVCoff.TotalIL)>>7;	// 2^7
					}
				}
				//gUVCoff.m_UData5 = (gUVCoff.TotalI)>>7;
				m_ULong6 = ((Ullong)m_UData5 * (Ullong)gMotorInfo.Current)>>10;		// 2^-2
				//m_ULong2 = 锟斤拷锟斤拷锟斤拷(A)锟斤拷10锟斤拷2^4
				/*while((m_ULong2>>16) != 0)
				{
					m_ULong1 = m_ULong1>>1;
					m_ULong2 = m_ULong2>>1;
				}*/
				m_ULong5 = m_ULong4 / m_ULong6;											// 锟斤拷压锟斤拷 / 锟斤拷锟斤拷锟斤拷		2^10
				m_UData2 = (m_ULong5 * (Ullong)1000)>>11;							// 2^0/2	// >>7

				gMotorExtReg.RsPm = m_UData2;											// 锟斤拷锟斤拷实锟斤拷值
				m_UData3 = ((Ulong)gMotorExtReg.RsPm * (Ulong)gMotorInfo.Current)/gMotorInfo.Votage;	
                gMotorExtPer.Rpm = ((Ulong)m_UData3 * 18597)>>14;		
				gUVCoff.Flag = 0;
				gUVCoff.RsTune = 2;
				DisableDrive();
				ResetADCEndIsr(); 
				InitSetPWM();                       //锟街革拷锟睫改的寄达拷锟斤拷锟斤拷
				//gGetParVarable.IdSubStep = 1;			
				if(STATUS_GET_PAR == gMainStatus.RunStep)
				{
				    gGetParVarable.IdSubStep = 1;
	                gGetParVarable.ParEstMstep++; 
				}
				else
				{
			  	    PrepareParForRun();
		            gMainStatus.RunStep = STATUS_STOP;
		            gMainStatus.SubStep = 0;
		            gMainStatus.PrgStatus.all = 0;		//锟斤拷锟叫匡拷锟斤拷锟斤拷效
				}
			}
		    break;
	}	
}

void BeforeRunRsIdentifyICal(void)
{
    s16 m_Data;
    if( gMotorInfo.Current > gInvInfo.InvCurrent )  //锟斤拷锟斤拷疃拷锟斤拷锟斤拷锟斤拷诒锟狡碉拷锟斤拷疃拷锟斤拷锟绞憋拷锟斤拷员锟狡碉拷锟斤拷疃拷锟斤拷锟轿�
    {
        m_Data = ((long)gInvInfo.InvCurrent *500L) / gMotorInfo.Current;
    }
	else
	{
	    m_Data = 500; 
	}

	gRsIdentifyPID.Deta = (s16)(gUVCoff.CurComperCoff - gUVCoff.Temper);

    if((gRsIdentifyPID.Deta > 0)&&(gUVCoff.Flag == 0))		// 锟斤拷锟斤拷锟斤拷锟节凤拷锟斤拷
	{
	    if(gInvInfo.InvTypeApply > 16)							// 锟斤拷锟接匡拷锟斤拷时锟斤拷
        {         
           gUVCoff.Comper += 1;         
        }
        else
        {
		   gUVCoff.Comper += 2;
        }
		gUVCoff.Number = 0;
		//gUVCoff.Flag = 1;
	}
	else if(gRsIdentifyPID.Deta < - m_Data)						// 锟斤拷锟斤拷锟饺凤拷锟斤拷小500
	{
	    if(gUVCoff.Comper > (gDeadBand.DeadBand>>1) + 5)		// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
		{
	        if(gInvInfo.InvTypeApply > 16)						// 锟斤拷小锟斤拷锟斤拷时锟斤拷
	        {         
	           gUVCoff.Comper -= 2;         
	        }
	        else
	        {
			   gUVCoff.Comper -= 5;
	        }
			gUVCoff.Number = 0;
		}
		else
		{
		    gUVCoff.Comper = (gDeadBand.DeadBand>>1) + 5;
		}
		gUVCoff.Flag = 1;													// 说锟斤拷锟斤拷锟斤拷时锟斤拷锟斤拷锟节硷拷小
	}
	else if((gRsIdentifyPID.Deta > m_Data)&&(gUVCoff.Flag == 1))		// 锟斤拷锟斤拷锟饺凤拷锟斤拷锟斤拷500, 锟揭匡拷锟斤拷锟斤拷锟节硷拷小
	{
	    if(gInvInfo.InvTypeApply > 16)									// 锟斤拷锟接匡拷锟斤拷时锟斤拷
        {         
           gUVCoff.Comper += 1;         
        }
        else
        {
		   gUVCoff.Comper += 2;
        }
        gUVCoff.Number = 0;
	}
	
	if(gUVCoff.Number == 0)  // 说锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷呒锟叫�, 锟斤拷锟斤拷锟叫碉拷压_锟斤拷锟斤拷锟桔伙拷
	{
	    if(gGetParVarable.IdSubStep == 3)
		{
		    gUVCoff.TotalVoltL   = 0;
			gUVCoff.TotalIL      = 0;
			gUVCoff.TotalComperL = 0;
		}
		else if(gGetParVarable.IdSubStep == 4)
		{
		    gUVCoff.TotalVolt   = 0;
			gUVCoff.TotalI      = 0;
			gUVCoff.TotalIV     = 0;
			gUVCoff.TotalComper = 0;
		}
	}
	else						// 锟窖斤拷锟斤拷锟斤拷态, 锟斤拷始锟桔计碉拷压_锟斤拷锟斤拷_时锟斤拷
	{
	    if(gGetParVarable.IdSubStep == 3)
		{
			gUVCoff.TotalVoltL += gUDC.uDCFilter;
			gUVCoff.TotalIL    += gUVCoff.Temper;
			gUVCoff.TotalComperL += gUVCoff.Comper;
		}
		else if(gGetParVarable.IdSubStep == 4)
		{
			gUVCoff.TotalVolt += gUDC.uDCFilter;
			gUVCoff.TotalI    += gUVCoff.Temper;
            gUVCoff.TotalIV   += gUVCoff.TemperV;
			gUVCoff.TotalComper += gUVCoff.Comper;
		}        
	}
    gUVCoff.Number++;

	if(gUVCoff.Comper >= gUVCoff.Rs_PRD)      // 125us
	{
        gUVCoff.Comper	= gDeadBand.DeadBand>>1; 
        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
        gError.ErrorInfo[1].bit.Fault4 = 10;
	}
	else
	{
		EALLOW;
		EPwm1Regs.CMPA.half.CMPA = gUVCoff.Comper;
		EDIS;
	}
}

void RsIdentify_ADC_Over_isr(void)
{
    if(EPwm1Regs.TBSTS.bit.CTRDIR == 1)  // 锟斤拷锟斤拷锟斤拷
	{
        return;	
	}
#if (SOFTSERIES == MD380SOFT)
	if(GetOverUdcFlag())                    //锟斤拷压锟斤拷锟斤拷
    {
       HardWareOverUDCDeal();				
    }
#endif    
    GetUDCInfo();							//锟斤拷取母锟竭碉拷压锟斤拷锟斤拷锟斤拷锟斤拷
	GetCurrentInfo();						//锟斤拷取锟斤拷锟斤拷锟斤拷锟斤拷, 锟皆硷拷锟铰度★拷母锟竭碉拷压锟侥诧拷锟斤拷
	ChangeCurrent();
 	BrakeResControl(); 

	gUVCoff.Temper  = abs(gIUVWQ24.U>>12);	
	gUVCoff.TemperV = abs(gIUVWQ24.V>>12);
	BeforeRunRsIdentifyICal();	
}
/*******************************************************************************
* Function Name  : 锟斤拷识锟斤拷锟接碉拷锟斤拷时锟斤拷锟紸DC锟斤拷锟斤拷锟叫断达拷锟斤拷锟斤拷锟斤拷
* Description    : IfPWMPeriodInt()锟斤拷锟斤拷锟节碉拷
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void ADCEndIsrTuneR1(void)
{
	if(EPwm1Regs.TBSTS.bit.CTRDIR == 1)  // 锟斤拷锟斤拷锟斤拷
	{
        return;	
	}
#if (SOFTSERIES == MD380SOFT)
	if(GetOverUdcFlag())                    //锟斤拷压锟斤拷锟斤拷
    {
       HardWareOverUDCDeal();				
    }
#endif    
    GetUDCInfo();							//锟斤拷取母锟斤拷锟窖癸拷锟斤拷锟斤拷锟斤拷?
	GetCurrentInfo();						//锟斤拷取锟斤拷锟斤拷锟斤拷? 锟皆硷拷锟铰度★拷母锟竭碉拷压锟侥诧拷锟斤拷
	ChangeCurrent();						//锟斤拷锟斤拷锟斤拷殖锟斤拷锟斤拷碌牡锟斤拷锟斤拷锟�
	BrakeResControl(); 
}
*/
/*******************************************************************************
* Function Name  : 锟斤拷识锟斤拷始位锟矫角碉拷时锟斤拷ADC锟斤拷锟斤拷锟叫断︼拷锟斤拷锟斤拷
* Description    : IfPWMPeriodInt()锟斤拷锟斤拷锟节碉拷
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADCEndIsrTune_POLSE_POS(void)
{
    EALLOW;             //28035锟斤拷为EALLOW锟斤拷锟斤拷
    ADC_CLEAR_INT_FLAG;
    EDIS;
    EINT;
#if (SOFTSERIES == MD380SOFT)
    if(GetOverUdcFlag())                    //锟斤拷压锟斤拷锟斤拷
    {
       HardWareOverUDCDeal();				
    }
#endif    					
	GetUDCInfo();							//锟斤拷取母锟竭碉拷压锟斤拷锟斤拷锟斤拷锟斤拷
	GetCurrentInfo();						//锟斤拷取锟斤拷锟斤拷锟斤拷锟斤拷, 锟皆硷拷锟铰度★拷母锟竭碉拷压锟侥诧拷锟斤拷
	 	 	
	if(EPwm1Regs.TBSTS.bit.CTRDIR == 0)
	{
		if(gIPMInitPos.Step != 0)               //锟斤拷始位锟矫角憋拷识
	    {
			SynInitPosDetect();
		}
	}    
	else
	{
		gIPMInitPos.CurIU = (s16)(gIUVWQ24.U >> 12);
		gIPMInitPos.CurIV = (s16)(gIUVWQ24.V >> 12);
		gIPMInitPos.CurIW = (s16)(gIUVWQ24.W >> 12);
	} 
	//gRotorSpeed.SpeedFeed = 0;
	//gRotorSpeed.SpeedFeedFilter = 0 ;
    DINT;
    EALLOW;
    ADC_RESET_SEQUENCE;
#ifdef TARGET_GS32
    
#else
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge this interrupt
#endif
	EDIS;
}

