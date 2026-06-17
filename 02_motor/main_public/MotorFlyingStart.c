/****************************************************************
锟侥硷拷锟斤拷锟杰ｏ拷同锟斤拷锟斤拷转锟劫革拷锟劫匡拷锟狡筹拷锟斤拷
锟侥硷拷锟芥本锟斤拷 
锟斤拷锟铰革拷锟铰ｏ拷 
	
****************************************************************/
//lint
#include "MotorInclude.h"
#include "MotorFlyingStart.h"

PMSM_FLYING_START_STRUCT gFlyingStart;
extern void TurnToStopStatus(void);
extern void PrepareParForRun(void);
extern void RunCaseRun2Ms(void);
s16 gTest0,gTest1,gTest2,gTest3,gTest4,gTest5,gTest6;

void RunCaseFlyingStart(void)
{
	if((gError.ErrorCode.all != 0) || 
	   (gMainCmd.Command.bit.Start == 0))
	{
		DisableDrive();
	    SynInitPosDetSetPwm(6);		    //同锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷识锟街革拷锟侥达拷锟斤拷锟斤拷锟斤拷
		gFlyingStart.Step = 0;
		ResetADCEndIsr();
		TurnToStopStatus();
		return;
	}

    switch(gMainStatus.SubStep)
    {
        case 1:
		    SetADCEndIsr(ADCEndIsrFlying_Start);
			if(gFlyingStart.Step == 0)		
			{
       			gFlyingStart.Step = 1;                       // 锟矫撅拷态锟斤拷识锟斤拷志
       			gMainStatus.SubStep ++;
			}
			else
			{
			    gFlyingStart.Step = 0;
			}
            break;

        case 2:
            if(gFlyingStart.Step == 0)
        	{
        		if(gFlyingStart.StartStop == 1)		// 锟斤拷锟劫碉拷锟劫度诧拷为0, 为锟斤拷锟斤拷锟斤拷准锟斤拷
        		{
					gFlyingStart.FcChgFlag = 1;
					gFlyingStart.FcChgCnt = 10;
        		}
				else
				{
					gFlyingStart.FcChgFlag = 0;
					gFlyingStart.FcChgCnt = 0;
				}

        		DisableDrive();
				ResetADCEndIsr();
        		gMainStatus.SubStep ++;
        	}   //else waiting interrupt deal
            break;
            
        case 3:
			InitSetPWM();
			InitSetAdc();
			SetInterruptEnable();	            // 锟斤拷锟斤拷锟绞讹拷锟侥匡拷锟酵撅拷顺锟斤拷锟斤拷卸锟斤拷锌锟斤拷锟斤拷枪乇盏模锟斤拷锟斤拷诖舜锟�
			gMainStatus.SubStep ++; 
			break;
                
        case 4:
			if(gFlyingStart.StartStop == 1)			// 锟斤拷锟劫碉拷频锟绞诧拷为0, 转锟斤拷锟斤拷锟斤拷状态
			{
	            PrepareParForRun();
				FlyingStratPreparePar();
				EnableDrive();
				gFlyingStart.Flag = 0;				// 锟铰达拷锟金动硷拷锟斤拷锟斤拷锟斤拷转锟劫革拷锟斤拷
				
				if(SYNC_SVC == gCtrMotorType)
				{
				    gMainStatus.StatusWord.bit.RunStart = 1;   //锟斤拷锟斤拷锟斤拷锟杰碉拷锟劫讹拷指锟斤拷
				}

				gMainStatus.RunStep = STATUS_RUN;
				gMainStatus.PrgStatus.all = 0;            
				gMainStatus.SubStep = 1;
		        gBrake.DelayClose = 0;  
		        RunCaseRun2Ms();        // 锟脚伙拷锟斤拷锟斤拷时锟戒，锟节革拷锟侥撅拷锟杰凤拷锟斤拷
			}
			else										// 锟斤拷锟劫碉拷频锟斤拷为0, 转锟斤拷停锟斤拷状态
			{
				gMainStatus.StatusWord.bit.SpeedSearchOver = 1;
				gMainCmd.FreqToFunc = 0;
	            gMainStatus.RunStep = STATUS_STOP;
	            gMainStatus.SubStep = 0;
	            gMainStatus.PrgStatus.all = 0;		//锟斤拷锟叫匡拷锟斤拷锟斤拷效
			}
            break;
            
        default:
            gError.ErrorCode.all |= ERROR_TUNE_FAIL;
            gError.ErrorInfo[4].bit.Fault1 = 3;
            break;
    }
}

void SynFlyingStart(Ulong FullFreq01, Uint InvCurrent, Uint MotorCurrent)
{
	u16        m_Cur,m_CurLowLimit;
	static s32 m_ZeroCurrentCnt = 0,m_ZeroCurrentFlag = 0,m_DelayCnt = 0,m_TscCnt = 0;
	u32        temp;

	switch(gFlyingStart.Step)
	{
		case 1:							                    // 锟斤拷始锟斤拷锟斤拷锟斤拷
			gFlyingStart.SubStep = 0;
			gFlyingStart.PWMTs = FLYING_START_TIME_BASE;
			gFlyingStart.Tsc = 1;
			m_TscCnt = 0;
			gFlyingStart.Ts[0] = 0;
			gFlyingStart.Ts[1] = 0;
			m_ZeroCurrentCnt = 0;
			m_ZeroCurrentFlag = 0;
			m_DelayCnt = 0;
			
			gFlyingStart.CurLimit = (u16)(((u32)gFlyingStart.CurLimitAdj << 12) / 100UL);
			//m_CurLowLimit = (Ulong)gInvInfo.InvCurrent * 1200UL / gMotorInfo.Current;		// 锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷30%
			m_CurLowLimit = (Ulong)InvCurrent * 400UL / MotorCurrent;		// 锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷10%

			gFlyingStart.Delay1 = 600000000UL / (FLYING_START_TIME_BASE * FullFreq01);
			gFlyingStart.Delay2 = 1250000000UL / (FullFreq01 * FLYING_START_TIME_BASE);
			gFlyingStart.Delay2 = Max(gFlyingStart.Delay2, (15000 / FLYING_START_TIME_BASE));			// 锟斤拷小锟斤拷40us, 1000Hz
				
			if(InvCurrent < MotorCurrent)
			{
                temp = (Ulong)gFlyingStart.CurLimit * InvCurrent;
        		gFlyingStart.CurLimit = temp / MotorCurrent;
			}

			if(gFlyingStart.CurLimit < m_CurLowLimit)
			{
				gFlyingStart.CurLimit = m_CurLowLimit;			// 锟斤拷识锟斤拷锟斤拷锟斤拷锟斤拷小锟节憋拷频锟斤拷锟筋定锟斤拷10%, 锟斤拷锟斤拷锟斤拷锟斤拷锟杰筹拷锟斤拷
			}

			SynFlyingStartSetTs();
			SynInitPosDetSetPwm(7);
			gFlyingStart.Step ++;
			break;

		case 2:							                            // 确锟斤拷锟斤拷始锟斤拷锟斤拷为0
		case 4:
		case 6:

			if(gFlyingStart.Step != 2)
			{
				gFlyingStart.Ts[(gFlyingStart.Step >> 1) - 2] += 1;
				m_DelayCnt = gFlyingStart.Ts[(gFlyingStart.Step >> 1) - 2];
			}
			else
			{
				m_DelayCnt = gFlyingStart.Delay1 + gFlyingStart.Tsc;
			}

			m_Cur = Max(abs(gFlyingStart.CurIU),abs(gFlyingStart.CurIV));
			m_Cur = Max(m_Cur,abs(gFlyingStart.CurIW));
			m_CurLowLimit = (Ulong)InvCurrent * 100UL / MotorCurrent;		// 锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷2.5%

			if(m_Cur < m_CurLowLimit)
			{
				m_ZeroCurrentCnt ++;											// 锟桔计碉拷锟斤拷为0锟斤拷锟斤拷
				if(m_ZeroCurrentCnt >= 5)
				{
					m_ZeroCurrentCnt = 5;
					m_ZeroCurrentFlag = 1;
				}
			}
			else
			{
				if(m_ZeroCurrentFlag != 1)
				{
					m_ZeroCurrentCnt = 0;
					//m_ZeroCurrentCnt--;
				}
			}
			gTest6 = m_ZeroCurrentCnt;
			
			if((m_DelayCnt >= (gFlyingStart.Delay1 + gFlyingStart.Tsc)) && (m_ZeroCurrentFlag == 1))
			{	
				if(gFlyingStart.Step == 6)								// 锟斤拷证锟节讹拷锟轿硷拷锟绞憋拷锟斤拷锟节碉拷一锟斤拷500us锟斤拷锟斤拷, 锟斤拷锟斤拷呒锟斤拷憔拷
				{
					temp = Max(gFlyingStart.Tsc,gFlyingStart.Delay2);
					if(gFlyingStart.Ts[1] < (gFlyingStart.Ts[0] + temp))		// 500us, 1000Hz
					{
						break;
					}
				}

				m_ZeroCurrentCnt = 0;										// 锟斤拷锟斤拷锟桔计达拷锟斤拷, 锟皆憋拷锟斤拷锟斤拷奂锟
				m_ZeroCurrentFlag = 0;										// 锟斤拷锟斤拷锟桔计憋拷志, 锟皆憋拷锟斤拷锟斤拷奂锟
				gFlyingStart.SubStep = 0;
				gFlyingStart.Step ++;
				m_TscCnt = 0;
			}

			break;

		case 3:																// 寻锟揭猴拷锟绞的讹拷路时锟斤拷, 锟斤拷锟斤拷录锟斤拷一锟轿讹拷路锟斤拷锟斤拷

			EnableDrive();
			
			m_Cur = Max(abs(gFlyingStart.CurIU),abs(gFlyingStart.CurIV));
			m_Cur = Max(m_Cur,abs(gFlyingStart.CurIW));
			//m_CurLowLimit = (Ulong)gInvInfo.InvCurrent * 1200UL / gMotorInfo.Current;		// 锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷30%
			m_CurLowLimit = (Ulong)InvCurrent * 400UL / MotorCurrent;		// 锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷10%
	
			switch(gFlyingStart.SubStep)
			{
				case 0:
					SynInitPosDetSetPwm(8);								// 锟斤拷始锟斤拷路
					gFlyingStart.SubStep ++;
					break;
					
				case 1:
					if(m_Cur >= gFlyingStart.CurLimit)					// 锟斤拷锟斤拷锟斤拷锟斤拷锏斤拷瓒
					{
						SynInitPosDetSetPwm(7);							// 停止锟斤拷路
						gFlyingStart.SubStep++;
					}
					else
					{
						//if(gFlyingStart.Tsc > (300000L / FLYING_START_TIME_BASE))						// 锟斤拷路时锟斤拷10ms, 锟斤拷锟斤拷锟斤拷没锟叫达到锟借定值
						if(gFlyingStart.Tsc > (3000000L / FLYING_START_TIME_BASE))						// 锟斤拷路时锟斤拷100ms, 锟斤拷锟斤拷锟斤拷没锟叫达到锟借定值
						{	
							if(m_Cur < m_CurLowLimit)						// 锟斤拷路锟斤拷锟斤拷小锟节憋拷频锟斤拷锟筋定锟斤拷10%, 锟斤拷锟斤拷锟叫硷拷锟斤拷 
							{
							#if 0							// 锟斤拷锟斤拷锟斤拷锟斤拷怨锟斤拷慰锟
								gFlyingStart.CurRemU[0] = gFlyingStart.CurIU;
								gFlyingStart.CurRemV[0] = gFlyingStart.CurIV;
								gFlyingStart.CurRemW[0] = gFlyingStart.CurIW;
							#endif
								gFlyingStart.PWMTs = 20 * DSP_CLOCK;		// 锟斤拷止锟街革拷锟斤拷频锟斤拷执锟斤拷时锟戒不锟斤拷, 锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷
								SynFlyingStartSetTs();
								SynInitPosDetSetPwm(7);					// 停止锟斤拷路
								gFlyingStart.Flag = 1;					// 锟斤拷锟劫癸拷锟斤拷锟窖撅拷锟斤拷锟
								gFlyingStart.Step = 9;					// 停止锟斤拷锟斤拷, 锟斤拷锟斤拷锟叫硷拷锟斤拷
								gFlyingStart.Freq01 = 0;					// 锟斤拷为锟劫讹拷为0
								gFlyingStart.FreqPer = 0;
								gFlyingStart.StartStop = 0;				// 锟劫度很碉拷, 锟斤拷锟斤拷要锟劫次憋拷识, 直锟斤拷锟斤拷
							}
							else											// 锟斤拷锟斤拷锟斤拷锟节憋拷频锟斤拷锟筋定锟斤拷30%, 锟斤拷锟斤拷没锟叫达到锟借定值, 锟斤拷实锟绞诧拷锟斤拷锟侥碉拷锟斤拷锟斤拷锟斤拷
							{
								SynInitPosDetSetPwm(7);							// 停止锟斤拷路
								gFlyingStart.Tsc++;
								gFlyingStart.SubStep++;
							}
						}
						else
						{
							gFlyingStart.Tsc++;
						}
					}
					break;

				case 2:
					gFlyingStart.CurRemU[0] = gFlyingStart.CurIU;
					gFlyingStart.CurRemV[0] = gFlyingStart.CurIV;
					gFlyingStart.CurRemW[0] = gFlyingStart.CurIW;
					gFlyingStart.SubStep = 0;
					gFlyingStart.Step ++;
					break;
					
				default:
					break;
			}
			break;

		case 5:																// 锟斤拷路锟斤拷锟斤拷录锟斤拷路锟斤拷锟斤拷
		case 7:

			switch(gFlyingStart.SubStep)
			{
				case 0:
					SynInitPosDetSetPwm(8);                   			// 锟斤拷始锟斤拷路
					gFlyingStart.SubStep ++;
					break;

				case 1:
					m_TscCnt++;
					
					if(m_TscCnt < gFlyingStart.Tsc)
					{
						break;
					}
					else
					{
						SynInitPosDetSetPwm(7);								// 停止锟斤拷路
						gFlyingStart.SubStep ++;
						break;
					}
					
				case 2:														// 锟斤拷录锟斤拷路锟斤拷锟斤拷
					gFlyingStart.CurRemU[(gFlyingStart.Step - 3) >> 1] = gFlyingStart.CurIU;
					gFlyingStart.CurRemV[(gFlyingStart.Step - 3) >> 1] = gFlyingStart.CurIV;
					gFlyingStart.CurRemW[(gFlyingStart.Step - 3) >> 1] = gFlyingStart.CurIW;
					
					if(gFlyingStart.Step == 7)
					{
						gFlyingStart.Ts[0] = gFlyingStart.Ts[0] + gFlyingStart.Tsc + 2;
						gFlyingStart.Ts[1] = gFlyingStart.Ts[1] + gFlyingStart.Tsc + 2;
						gFlyingStart.PWMTs = 400 * DSP_CLOCK;		// 为锟斤拷锟斤拷锟劫度和角度憋拷锟斤拷时锟斤拷
					}

					SynFlyingStartSetTs();
					gFlyingStart.SubStep = 0;
					gFlyingStart.Step ++;
					break;
						
				default:
					break;
			}
			break;
			
		case 8:
			SynFlyingStartCalc(gBasePar.FullFreq01);
			gFlyingStart.Step ++;
			break;
			
		default:			
			gFlyingStart.Step = 0;
			DisableDrive();
			break;
	}
}

void SynFlyingStartCalc(Ulong FullFreq01)
{
	s16 i,m_DetaTheta;
	u32 m_DetaFreqPer;
	s16 m_OmegaT,m_Sin,m_Cos,m_X,m_Y,m_ThetaDQ;
	s32 m_DetaT,m_Freq01,m_Freq[4],m_FreqComp[2],m_Theta[2],m_DetaFrq1,m_DetaFrq2;
	s16 m_TotalU,m_TotalV,m_TotalW;

	gFlyingStart.DetaT = GetTime();

	if(gTestDataReceive.TestData26 == 0)
	{
	// 锟斤拷锟轿讹拷路每锟轿碉拷锟斤拷锟斤拷锟
		m_TotalU = gFlyingStart.CurRemU[0] + gFlyingStart.CurRemV[0] + gFlyingStart.CurRemW[0];
		m_TotalV = gFlyingStart.CurRemU[1] + gFlyingStart.CurRemV[1] + gFlyingStart.CurRemW[1];
		m_TotalW = gFlyingStart.CurRemU[2] + gFlyingStart.CurRemV[2] + gFlyingStart.CurRemW[2];

		if((abs(m_TotalU) > 500) && (abs(m_TotalV) > 500) && (abs(m_TotalW) > 500))
		{
			gFlyingStart.Record = 1;
			gError.ErrorCode.all |= ERROR_SHORT_EARTH;
			gError.ErrorInfo[2].bit.Fault4 = 4;
			return;
		}
	}

	if(gTestDataReceive.TestData25 == 0)
	{
		m_TotalU = 0;
		m_TotalV = 0;
		m_TotalW = 0;
		
		for(i=0;i<3;i++)				// 锟斤拷锟轿讹拷路每锟斤拷锟斤拷锟斤拷锟斤拷锟街碉拷锟斤拷
		{
			m_TotalU += abs(gFlyingStart.CurRemU[i]);
			m_TotalV += abs(gFlyingStart.CurRemV[i]);
			m_TotalW += abs(gFlyingStart.CurRemW[i]);
		}
		
		if((m_TotalU < 300) || (m_TotalV < 300) || (m_TotalW < 300))
		{
			gFlyingStart.Record = 2;
			gFlyingStart.PhaseLoseCnt ++;
			if(gFlyingStart.PhaseLoseCnt >= 3)
			{
				gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
				gError.ErrorInfo[1].bit.Fault4 = 14;
			}
			else
			{
				gFlyingStart.Step = 0;		// 执锟斤拷锟斤拷煤锟斤拷锟斤拷锟絊tep锟斤拷, 锟节此革拷值0锟秸好匡拷锟皆凤拷锟斤拷
			}
			return;
		}
		else
		{
			gFlyingStart.PhaseLoseCnt = 0;
		}
	}

	if((gFlyingStart.Ts[0] >= (15 * gFlyingStart.Delay1)) && ((gFlyingStart.Ts[0] - 2 * gFlyingStart.Tsc - 2) > gFlyingStart.Delay1))		// 锟斤拷锟斤拷Ts[0]锟侥诧拷锟斤拷锟斤拷
	{
		gFlyingStart.Record = 3;
		gFlyingStart.Step = 0;		// 执锟斤拷锟斤拷煤锟斤拷锟斤拷锟絊tep锟, 锟节此革拷值0锟秸好匡拷锟皆凤拷锟斤拷
		return;
	}

	for(i = 0;i < 3;i++)				// 锟斤拷锟斤拷锟斤拷锟戒换锟斤拷Alpha - Beta锟斤拷系锟
	{
		gFlyingStart.CurRemAlpha[i] = ((long)gFlyingStart.CurRemU[i] * 23170L)>>15;
		gFlyingStart.CurRemBeta[i]  = (((long)(gFlyingStart.CurRemV[i] - gFlyingStart.CurRemW[i])) * 13377L) >> 15;
		gFlyingStart.ThetaAB[i] = user_atan(gFlyingStart.CurRemAlpha[i],gFlyingStart.CurRemBeta[i]);
	}
	
	m_DetaTheta = gFlyingStart.ThetaAB[0] + gFlyingStart.ThetaAB[2] - 2 * gFlyingStart.ThetaAB[1];
	m_DetaT = gFlyingStart.Ts[1] - gFlyingStart.Ts[0];
	
	if((gFlyingStart.Tsc < gFlyingStart.Delay2) && (m_DetaT > gFlyingStart.Delay2))		// 锟斤拷止锟斤拷DetaT锟斤拷时锟斤拷锟斤拷锟斤拷转锟斤拷锟斤拷180锟斤拷
	{
		gFlyingStart.Record = 4;
		gFlyingStart.Step = 0;		// 执锟斤拷锟斤拷煤锟斤拷锟斤拷锟絊tep锟斤拷, 锟节此革拷值0锟秸好匡拷锟皆凤拷锟斤拷
		return;
	}
	
	// 锟斤拷锟斤拷锟斤拷确锟斤拷频锟绞凤拷围锟斤拷频锟斤拷
//	m_Freq01 = ((llong)m_DetaTheta * 5859375L / (m_DetaT  * FLYING_START_TIME_BASE)) >> 7;
	m_Freq01 = ((llong)m_DetaTheta * DSP_CLOCK * 390625L / (m_DetaT  * FLYING_START_TIME_BASE)) >> 9;
	gTest3 = m_Freq01;
	m_Theta[0] = gFlyingStart.ThetaAB[1] - gFlyingStart.ThetaAB[0];
	m_Theta[1] = gFlyingStart.ThetaAB[2] - gFlyingStart.ThetaAB[1];
	
	if(m_DetaTheta > 0)
	{
		if(m_Theta[0] < 0)
		{
			m_Theta[0] = m_Theta[0] + 65536L;
		}
		
		if(m_Theta[1] < 0)
		{
			m_Theta[1] = m_Theta[1] + 65536L;
		}
	}
	else if(m_DetaTheta < 0)
	{
		if(m_Theta[0] > 0)
		{
			m_Theta[0] = m_Theta[0] - 65536L;
		}
		
		if(m_Theta[1] > 0)
		{
			m_Theta[1] = m_Theta[1] - 65536L;
		}
	}
	else		// 锟睫凤拷锟叫讹拷锟斤拷转锟斤拷锟斤拷, 锟斤拷锟铰筹拷始锟斤拷, 锟斤拷锟铰凤拷锟斤拷锟斤拷锟斤拷
	{
		gFlyingStart.Record = 5;
		gFlyingStart.Step = 0;		// 执锟斤拷锟斤拷煤锟斤拷锟斤拷锟絊tep锟斤拷锟, 锟节此革拷值0锟秸好匡拷锟皆凤拷锟斤拷
		return;
	}
	
	for(i = 0;i < 2;i++)
	{
//		m_Freq[2 * i] = ((llong)m_Theta[i] * 5859375L / (gFlyingStart.Ts[i] * FLYING_START_TIME_BASE)) >> 7;	// 5859375 >> 7 = 60000000 * 100 / 65536 / 2
		m_Freq[2 * i] = ((llong)m_Theta[i] * DSP_CLOCK * 390625L / (gFlyingStart.Ts[i] * FLYING_START_TIME_BASE)) >> 9;	// 390625 >> 9 = 1000000 * 100 / 65536 / 2
		m_Freq[2 * i + 1] = m_Freq[2 * i];
//		m_FreqComp[i] = ((llong)3000000000LL) / (gFlyingStart.Ts[i] * FLYING_START_TIME_BASE);
		m_FreqComp[i] = ((llong)50000000LL * DSP_CLOCK) / (gFlyingStart.Ts[i] * FLYING_START_TIME_BASE);

		if(m_Freq[2 * i] < m_Freq01)
		{
			while(m_Freq[2 * i + 1] < m_Freq01)
			{
				m_Freq[2 * i] = m_Freq[2 * i + 1];
				m_Freq[2 * i + 1] = m_Freq[2 * i + 1] + m_FreqComp[i];
			}
		}
		else if(m_Freq[2 * i] > m_Freq01)
		{
			while(m_Freq[2 * i + 1] > m_Freq01)
			{
				m_Freq[2 * i] = m_Freq[2 * i + 1];
				m_Freq[2 * i + 1] = m_Freq[2 * i + 1] - m_FreqComp[i];
			}
		}

		if(abs(m_Freq[2 * i] - m_Freq01) > abs(m_Freq[2 * i + 1] - m_Freq01))
		{
			gFlyingStart.Freq[i] = m_Freq[2 * i + 1];
		}
		else
		{
			gFlyingStart.Freq[i] = m_Freq[2 * i];
		}
	}
	
	#if 1
	gTest1 = m_Freq[0] >> 5;
	gTest2 = m_Freq[1] >> 5;
	gTest4 = m_Freq[2] >> 5;
	gTest5 = m_Freq[3] >> 5;
	#endif
	
	m_DetaFreqPer = abs(gFlyingStart.Freq[0] - gFlyingStart.Freq[1]);
	m_DetaFreqPer = (Ulong)(((Ullong)m_DetaFreqPer << 15) / ((Ullong)FullFreq01));
	gTest0 = m_DetaFreqPer;
	if(((Ullong)m_DetaFreqPer << 2) > gMotorInfo.FreqPer)// 锟斤拷锟斤拷锟斤拷平锟斤拷锟斤拷锟斤拷锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷, 锟斤拷锟铰凤拷锟斤拷锟斤拷锟斤拷
	{
		gFlyingStart.Record = 6;
		gFlyingStart.Step = 0;		// 执锟斤拷锟斤拷煤锟斤拷锟斤拷锟絊tep锟斤1, 锟节此革拷值0锟秸好匡拷锟皆凤拷锟斤拷
		return;
	}

	gFlyingStart.Record = 0;
	
	gFlyingStart.Freq01 = (gFlyingStart.Freq[0] >> 1) + (gFlyingStart.Freq[1] >> 1);
	gFlyingStart.FreqPer = (long)(((llong)gFlyingStart.Freq01 << 15) / ((llong)FullFreq01));

//	m_OmegaT = (s16)(((llong)gFlyingStart.Freq01 * (llong)gFlyingStart.Tsc  * FLYING_START_TIME_BASE * (llong)733) >> 25);// 锟斤拷值锟斤拷锟斤拷锟皆讹拷锟窖角讹拷转锟斤拷锟斤拷-180锟斤拷180锟饺凤拷围 65536 * 2 / 100 / 60000000 = 733 >> 25 
	m_OmegaT = (s16)((((llong)gFlyingStart.Freq01 * (llong)gFlyingStart.Tsc  * FLYING_START_TIME_BASE) << 9) / ((long)390625L * DSP_CLOCK));// 锟斤拷值锟斤拷锟斤拷锟皆讹拷锟窖角讹拷转锟斤拷锟斤拷-180锟斤拷180锟饺凤拷围 65536 * 2 / 100 / 60000000 = 733 >> 25 
	m_Sin = ((long)qsin(m_OmegaT) * 1000) >> 15;
	m_Cos = ((long)qsin(16384 - m_OmegaT) * 1000) >> 15;
	m_X = -(((long)gMotorExtInfo.LQ * (1000 - m_Cos)) >> 10);
	m_Y = -(((long)gMotorExtInfo.LD * m_Sin) >> 10);
	m_ThetaDQ = user_atan(m_X,m_Y);				// 锟斤拷路锟斤拷锟斤拷锟斤拷dq锟斤拷系锟铰的角讹拷
	gFlyingStart.Theta = gFlyingStart.ThetaAB[2] - m_ThetaDQ;
	gFlyingStart.StartStop = 1;
}

void FlyingStratPreparePar(void)
{
	s16 m_DetaTheta;

	// 锟斤拷强锟斤拷锟斤拷锟斤拷
	gImAcrQ24.KP = gImAcrQ24.KP * gFlyingStart.KpRatio / 10;
	gItAcrQ24.KP = gItAcrQ24.KP * gFlyingStart.KpRatio / 10;
	gImAcrQ24.KI = gImAcrQ24.KI * gFlyingStart.KiRatio / 10;
	gItAcrQ24.KI = gItAcrQ24.KI * gFlyingStart.KiRatio / 10;

	gFlyingStart.BmfEstValue = (long)(((llong)gMotorExtInfo.BemfVolt * (llong)gFlyingStart.FreqPer * (llong)gBasePar.FullFreq / ((llong)gMotorInfo.Frequency * (llong)gMotorInfo.Votage * (llong)10)) >> 3);
	gPmsmRotorPosEst.BmfEstValue = gFlyingStart.BmfEstValue;
	gPmsmRotorPosEst.BmfEstValueLast = gFlyingStart.BmfEstValue;
	gPmsmRotorPosEst.SpeedEstValueLast = gFlyingStart.FreqPer;
	gPmsmRotorPosEst.SpeedEstValueLpf = gFlyingStart.FreqPer;
	gPmsmRotorPosEst.SvcRotorSpeed = gFlyingStart.FreqPer;
	gPmsmRotorPosEst.UdLast = 0; 
//	gPmsmRotorPosEst.UqLast = gFlyingStart.BmfEstValue;
	gPmsmRotorPosEst.UqLast = 0;

	gImAcrQ24.Total = 0;
	gItAcrQ24.Total = gFlyingStart.BmfEstValue << 12;

	gRotorSpeed.SpeedBigFilter = gFlyingStart.FreqPer;
	gPmsmRotorPosEst.SpeedEstValue = gFlyingStart.FreqPer;

	gFlyingStart.DetaT = gFlyingStart.DetaT - GetTime() + (FLYING_START_TIME_BASE * 2) + 400 * DSP_CLOCK;
	m_DetaTheta = (s16)((llong)gFlyingStart.DetaT * gFlyingStart.Freq01 * 65536 / ((llong)100000000L * DSP_CLOCK));
	gFlyingStart.Theta = gFlyingStart.Theta + m_DetaTheta;
	gFlyingStart.ThetaEncoder = (s16)gIPMPos.RotorPos;
	gPmsmRotorPosEst.SvcRotorPos = ((long)gFlyingStart.Theta) << 16;
}

void FlyingStartParaAdjust(void)
{
	if(gFlyingStart.FcChgFlag == 1)
	{
		gImAcrQ24.KP = gImAcrQ24.KP * gFlyingStart.KpRatio / 10;
		gItAcrQ24.KP = gItAcrQ24.KP * gFlyingStart.KpRatio / 10;
		gImAcrQ24.KI = gImAcrQ24.KI * gFlyingStart.KiRatio / 10;
		gItAcrQ24.KI = gItAcrQ24.KI * gFlyingStart.KiRatio / 10;
	    gAsr.Asr.KP = 0;
	    gAsr.Asr.KI = 0;
		gIMTSet.M = 0;
	}
	else if(gFlyingStart.FcChgFlag == 2)
	{
	    gAsr.Asr.KP = gAsr.Asr.KP >> 2;
	    gAsr.Asr.KI = gAsr.Asr.KI >> 2;
	    gAsr.Asr.KP = __IQsat(gAsr.Asr.KP, 32767, 1);        // 锟斤拷锟角伙拷锟斤拷锟
	    gAsr.Asr.KI = __IQsat(gAsr.Asr.KI, 32767, 1);
	}

	if(gFlyingStart.FcChgFlag != 0)
	{
		gFlyingStart.FcChgCnt --;
		
		if(gFlyingStart.FcChgCnt == 5)
		{
			gFlyingStart.FcChgFlag = 2;
			gMainStatus.StatusWord.bit.SpeedSearchOver = 1;
		}
		else if(gFlyingStart.FcChgCnt <= 0)
		{
			gFlyingStart.FcChgFlag = 0;
		}
	}
}

void FlyingStartFcDeal(void)
{
	if(gFlyingStart.FcChgFlag == 1)
	{
		gBasePar.FcSetApply = C_DOUBLE_ACR_MAX_FC;
		gPWM.PWMModle = MODLE_CPWM;
		gSynPWM.AbsFreq = gMainCmd.FreqReal/100;
		gSynPWM.ModuleApply = 0;
		gSynPWM.FcApply = ((Ulong)gBasePar.FcSetApply)<<9;
		AsynPWMAngleCal(gSynPWM.FcApply);
	}
}

void FlyingStartInitDeal(void)
{
	gFlyingStart.Flag = 0;
	gFlyingStart.FcChgFlag = 0;
	gFlyingStart.FcChgCnt = 0;
}

void SynFlyingStartSetTs(void)
{
	if(gFlyingStart.PWMTs < FLYING_START_TIME_BASE)
	{
		gFlyingStart.PWMTs = FLYING_START_TIME_BASE;
	}

	EALLOW;
	EPwm1Regs.TBPRD = gFlyingStart.PWMTs;
	EPwm2Regs.TBPRD = gFlyingStart.PWMTs;
	EPwm3Regs.TBPRD = gFlyingStart.PWMTs;
	EDIS;
}
	
void ADCEndIsrFlying_Start(void)
{     					
#if (SOFTSERIES == MD380SOFT)
    if(GetOverUdcFlag())                    //锟斤拷压锟斤拷锟斤拷
    {
       HardWareOverUDCDeal();				
    }
#endif
	GetUDCInfo();							//锟斤拷取母锟竭碉拷压锟斤拷锟斤拷锟斤拷锟斤拷    
	GetCurrentInfo();						//锟斤拷取锟斤拷锟斤拷锟斤拷锟斤拷, 锟皆硷拷锟铰母锟竭碉拷压锟侥诧拷锟斤拷
	 	 	
	if(EPwm1Regs.TBSTS.bit.CTRDIR == 0)			// 锟斤拷锟节碉拷
	{
		if(gFlyingStart.Step != 0)               // 转锟劫革拷锟斤拷
	    {
			SynFlyingStart(gBasePar.FullFreq01,gInvInfo.InvCurrent,gMotorInfo.Current);
		}
	}    
	else											// 锟斤拷锟斤拷锟斤拷锟斤拷
	{
		gFlyingStart.CurIU = (s16)(gIUVWQ24.U >> 12);
		gFlyingStart.CurIV = (s16)(gIUVWQ24.V >> 12);
		gFlyingStart.CurIW = (s16)(gIUVWQ24.W >> 12);
	} 
}

