/****************************************************************
�ļ����ܣ�ͬ����ת�ٸ��ٿ��Ƴ���
�ļ��汾�� 
���¸��£� 
	
****************************************************************/
//lint
#include "MotorInclude.h"
#include "MotorFlyingStart.h"

PMSM_FLYING_START_STRUCT gFlyingStart;
extern void TurnToStopStatus(void);
extern void PrepareParForRun(void);
extern void RunCaseRun2Ms(void);
int gTest0,gTest1,gTest2,gTest3,gTest4,gTest5,gTest6;

void RunCaseFlyingStart(void)
{
	if((gError.ErrorCode.all != 0) || 
	   (gMainCmd.Command.bit.Start == 0))
	{
		DisableDrive();
	    SynInitPosDetSetPwm(6);		    //ͬ����������ʶ�ָ��Ĵ�������
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
       			gFlyingStart.Step = 1;                       // �þ�̬��ʶ��־
       			gMainStatus.SubStep ++;
			}
			else
			{
			    gFlyingStart.Step = 0;
			}
            break;

        case 2:
            if(gFlyingStart.Step == 0)           // �жϱ�ʶ���
        	{
        		if(gFlyingStart.StartStop == 1)		// ���ٵ��ٶȲ�Ϊ0, Ϊ������׼��
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
        		gMainStatus.SubStep ++;             //���Զ��һ�ģ���PWM�ָ����
        	}   //else waiting interrupt deal
            break;
            
        case 3:
			InitSetPWM();
			InitSetAdc();
			SetInterruptEnable();	            // �����ʶ��Ŀ��;�˳����ж��п����ǹرյģ����ڴ˴�
			gMainStatus.SubStep ++; 
			break;
                
        case 4:
			if(gFlyingStart.StartStop == 1)			// ���ٵ�Ƶ�ʲ�Ϊ0, ת������״̬
			{
	            PrepareParForRun();
				FlyingStratPreparePar();
				EnableDrive();
				gFlyingStart.Flag = 0;				// �´��𶯼�������ת�ٸ���
				
				if(SYNC_SVC == gCtrMotorType)
				{
				    gMainStatus.StatusWord.bit.RunStart = 1;   //�������ܵ��ٶ�ָ��
				}

				gMainStatus.RunStep = STATUS_RUN;
				gMainStatus.PrgStatus.all = 0;            
				gMainStatus.SubStep = 1;
		        gBrake.DelayClose = 0;  
		        RunCaseRun2Ms();        // �Ż�����ʱ�䣬�ڸ��ľ��ܷ���
			}
			else										// ���ٵ�Ƶ��Ϊ0, ת��ͣ��״̬
			{
				gMainStatus.StatusWord.bit.SpeedSearchOver = 1;
				gMainCmd.FreqToFunc = 0;
	            gMainStatus.RunStep = STATUS_STOP;
	            gMainStatus.SubStep = 0;
	            gMainStatus.PrgStatus.all = 0;		//���п�����Ч
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
		case 1:							                    // ��ʼ������
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
			//m_CurLowLimit = (Ulong)gInvInfo.InvCurrent * 1200UL / gMotorInfo.Current;		// ��Ƶ��������30%
			m_CurLowLimit = (Ulong)InvCurrent * 400UL / MotorCurrent;		// ��Ƶ��������10%

			gFlyingStart.Delay1 = 600000000UL / (FLYING_START_TIME_BASE * FullFreq01);
			gFlyingStart.Delay2 = 1250000000UL / (FullFreq01 * FLYING_START_TIME_BASE);		// ���Ϊ1.2�����Ƶ��
			gFlyingStart.Delay2 = Max(gFlyingStart.Delay2, (15000 / FLYING_START_TIME_BASE));			// ��С��40us, 1000Hz
				
			if(InvCurrent < MotorCurrent)		// ��Ƶ������С�ڵ������, �Ա�Ƶ������Ϊ��׼
			{
                temp = (Ulong)gFlyingStart.CurLimit * InvCurrent;
        		gFlyingStart.CurLimit = temp / MotorCurrent;
			}

			if(gFlyingStart.CurLimit < m_CurLowLimit)
			{
				gFlyingStart.CurLimit = m_CurLowLimit;			// ��ʶ��������С�ڱ�Ƶ�����10%, ���������ܳ���
			}

			SynFlyingStartSetTs();
			SynInitPosDetSetPwm(7);
			gFlyingStart.Step ++;
			break;

		case 2:							                            // ȷ����ʼ����Ϊ0
		case 4:
		case 6:

			if(gFlyingStart.Step != 2)							// ��¼���ʱ��
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
			m_CurLowLimit = (Ulong)InvCurrent * 100UL / MotorCurrent;		// ��Ƶ��������2.5%

			if(m_Cur < m_CurLowLimit)
			{
				m_ZeroCurrentCnt ++;											// �ۼƵ���Ϊ0����
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
				if(gFlyingStart.Step == 6)								// ��֤�ڶ��μ��ʱ����ڵ�һ��500us����, ����߼��㾫��
				{
					temp = Max(gFlyingStart.Tsc,gFlyingStart.Delay2);
					if(gFlyingStart.Ts[1] < (gFlyingStart.Ts[0] + temp))		// 500us, 1000Hz
					{
						break;
					}
				}

				m_ZeroCurrentCnt = 0;										// �����ۼƴ���, �Ա�����ۼ�
				m_ZeroCurrentFlag = 0;										// �����ۼƱ�־, �Ա�����ۼ�
				gFlyingStart.SubStep = 0;
				gFlyingStart.Step ++;
				m_TscCnt = 0;
			}

			break;

		case 3:																// Ѱ�Һ��ʵĶ�·ʱ��, ����¼��һ�ζ�·����

			EnableDrive();
			
			m_Cur = Max(abs(gFlyingStart.CurIU),abs(gFlyingStart.CurIV));
			m_Cur = Max(m_Cur,abs(gFlyingStart.CurIW));
			//m_CurLowLimit = (Ulong)gInvInfo.InvCurrent * 1200UL / gMotorInfo.Current;		// ��Ƶ��������30%
			m_CurLowLimit = (Ulong)InvCurrent * 400UL / MotorCurrent;		// ��Ƶ��������10%
	
			switch(gFlyingStart.SubStep)
			{
				case 0:
					SynInitPosDetSetPwm(8);								// ��ʼ��·
					gFlyingStart.SubStep ++;
					break;
					
				case 1:
					if(m_Cur >= gFlyingStart.CurLimit)					// ��������ﵽ�趨ֵ
					{
						SynInitPosDetSetPwm(7);							// ֹͣ��·
						gFlyingStart.SubStep++;
					}
					else
					{
						//if(gFlyingStart.Tsc > (300000L / FLYING_START_TIME_BASE))						// ��·ʱ��10ms, ������û�дﵽ�趨ֵ
						if(gFlyingStart.Tsc > (3000000L / FLYING_START_TIME_BASE))						// ��·ʱ��100ms, ������û�дﵽ�趨ֵ
						{	
							if(m_Cur < m_CurLowLimit)						// ��·����С�ڱ�Ƶ�����10%, �����м��� 
							{
							#if 0							// ��������Թ��ο�
								gFlyingStart.CurRemU[0] = gFlyingStart.CurIU;
								gFlyingStart.CurRemV[0] = gFlyingStart.CurIV;
								gFlyingStart.CurRemW[0] = gFlyingStart.CurIW;
							#endif
								gFlyingStart.PWMTs = 20 * DSP_CLOCK;		// ��ֹ�ָ���Ƶ��ִ��ʱ�䲻��, ��������ʱ��
								SynFlyingStartSetTs();
								SynInitPosDetSetPwm(7);					// ֹͣ��·
								gFlyingStart.Flag = 1;					// ���ٹ����Ѿ����		
								gFlyingStart.Step = 9;					// ֹͣ����, �����м���
								gFlyingStart.Freq01 = 0;					// ��Ϊ�ٶ�Ϊ0
								gFlyingStart.FreqPer = 0;
								gFlyingStart.StartStop = 0;				// �ٶȺܵ�, ����Ҫ�ٴα�ʶ, ֱ����
							}
							else											// �������ڱ�Ƶ�����30%, ����û�дﵽ�趨ֵ, ��ʵ�ʲ����ĵ�������
							{
								SynInitPosDetSetPwm(7);							// ֹͣ��·
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

		case 5:																// ��·����¼��·����
		case 7:

			switch(gFlyingStart.SubStep)
			{
				case 0:
					SynInitPosDetSetPwm(8);                   			// ��ʼ��·
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
						SynInitPosDetSetPwm(7);								// ֹͣ��·
						gFlyingStart.SubStep ++;
						break;
					}
					
				case 2:														// ��¼��·����
					gFlyingStart.CurRemU[(gFlyingStart.Step - 3) >> 1] = gFlyingStart.CurIU;
					gFlyingStart.CurRemV[(gFlyingStart.Step - 3) >> 1] = gFlyingStart.CurIV;
					gFlyingStart.CurRemW[(gFlyingStart.Step - 3) >> 1] = gFlyingStart.CurIW;
					
					if(gFlyingStart.Step == 7)
					{
						gFlyingStart.Ts[0] = gFlyingStart.Ts[0] + gFlyingStart.Tsc + 2;
						gFlyingStart.Ts[1] = gFlyingStart.Ts[1] + gFlyingStart.Tsc + 2;
						gFlyingStart.PWMTs = 400 * DSP_CLOCK;		// Ϊ�����ٶȺͽǶȱ���ʱ��
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
	// ���ζ�·ÿ�ε������
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
		
		for(i=0;i<3;i++)				// ���ζ�·ÿ���������ֵ���
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
				gFlyingStart.Step = 0;		// ִ����ú�����Step���1, �ڴ˸�ֵ0�պÿ��Է���
			}
			return;
		}
		else
		{
			gFlyingStart.PhaseLoseCnt = 0;
		}
	}

	if((gFlyingStart.Ts[0] >= (15 * gFlyingStart.Delay1)) && ((gFlyingStart.Ts[0] - 2 * gFlyingStart.Tsc - 2) > gFlyingStart.Delay1))		// ����Ts[0]�Ĳ�����
	{
		gFlyingStart.Record = 3;
		gFlyingStart.Step = 0;		// ִ����ú�����Step���1, �ڴ˸�ֵ0�պÿ��Է���
		return;
	}

	for(i = 0;i < 3;i++)				// �������任��Alpha - Beta��ϵ����Ƕ�
	{
		gFlyingStart.CurRemAlpha[i] = ((long)gFlyingStart.CurRemU[i] * 23170L)>>15;
		gFlyingStart.CurRemBeta[i]  = (((long)(gFlyingStart.CurRemV[i] - gFlyingStart.CurRemW[i])) * 13377L) >> 15;
		gFlyingStart.ThetaAB[i] = user_atan(gFlyingStart.CurRemAlpha[i],gFlyingStart.CurRemBeta[i]);
	}
	
	m_DetaTheta = gFlyingStart.ThetaAB[0] + gFlyingStart.ThetaAB[2] - 2 * gFlyingStart.ThetaAB[1];
	m_DetaT = gFlyingStart.Ts[1] - gFlyingStart.Ts[0];
	
	if((gFlyingStart.Tsc < gFlyingStart.Delay2) && (m_DetaT > gFlyingStart.Delay2))		// ��ֹ��DetaT��ʱ������ת����180��
	{
		gFlyingStart.Record = 4;
		gFlyingStart.Step = 0;		// ִ����ú�����Step���1, �ڴ˸�ֵ0�պÿ��Է���
		return;
	}
	
	// ������ȷ��Ƶ�ʷ�Χ��Ƶ��
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
	else		// �޷��ж���ת����, ���³�ʼ��, ���·�������
	{
		gFlyingStart.Record = 5;
		gFlyingStart.Step = 0;		// ִ����ú�����Step���1, �ڴ˸�ֵ0�պÿ��Է���
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
	if(((Ullong)m_DetaFreqPer << 2) > gMotorInfo.FreqPer)// ������ƽ��������Ƶ��������, ���·�������
	{
		gFlyingStart.Record = 6;
		gFlyingStart.Step = 0;		// ִ����ú�����Step���1, �ڴ˸�ֵ0�պÿ��Է���
		return;
	}

	gFlyingStart.Record = 0;
	
	gFlyingStart.Freq01 = (gFlyingStart.Freq[0] >> 1) + (gFlyingStart.Freq[1] >> 1);
	gFlyingStart.FreqPer = (long)(((llong)gFlyingStart.Freq01 << 15) / ((llong)FullFreq01));

//	m_OmegaT = (int)(((llong)gFlyingStart.Freq01 * (llong)gFlyingStart.Tsc  * FLYING_START_TIME_BASE * (llong)733) >> 25);// ��ֵ�����Զ��ѽǶ�ת����-180��180�ȷ�Χ 65536 * 2 / 100 / 60000000 = 733 >> 25 
	m_OmegaT = (int)((((llong)gFlyingStart.Freq01 * (llong)gFlyingStart.Tsc  * FLYING_START_TIME_BASE) << 9) / ((long)390625L * DSP_CLOCK));// ��ֵ�����Զ��ѽǶ�ת����-180��180�ȷ�Χ 65536 * 2 / 100 / 60000000 = 733 >> 25 
	m_Sin = ((long)qsin(m_OmegaT) * 1000) >> 15;
	m_Cos = ((long)qsin(16384 - m_OmegaT) * 1000) >> 15;
	m_X = -(((long)gMotorExtInfo.LQ * (1000 - m_Cos)) >> 10);
	m_Y = -(((long)gMotorExtInfo.LD * m_Sin) >> 10);
	m_ThetaDQ = user_atan(m_X,m_Y);				// ��·������dq��ϵ�µĽǶ�
	gFlyingStart.Theta = gFlyingStart.ThetaAB[2] - m_ThetaDQ;
	gFlyingStart.StartStop = 1;
}

void FlyingStratPreparePar(void)
{
	s16 m_DetaTheta;

	// ��ǿ������
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
	m_DetaTheta = (int)((llong)gFlyingStart.DetaT * gFlyingStart.Freq01 * 65536 / ((llong)100000000L * DSP_CLOCK));
	gFlyingStart.Theta = gFlyingStart.Theta + m_DetaTheta;
	gFlyingStart.ThetaEncoder = (int)gIPMPos.RotorPos;
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
	    gAsr.Asr.KP = __IQsat(gAsr.Asr.KP, 32767, 1);        // ���ǻ����
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
    if(GetOverUdcFlag())                    //��ѹ����
    {
       HardWareOverUDCDeal();				
    }
#endif
	GetUDCInfo();							//��ȡĸ�ߵ�ѹ��������    
	GetCurrentInfo();						//��ȡ��������, �Լ��¶ȡ�ĸ�ߵ�ѹ�Ĳ���	
	 	 	
	if(EPwm1Regs.TBSTS.bit.CTRDIR == 0)			// ���ڵ�
	{
		if(gFlyingStart.Step != 0)               // ת�ٸ���
	    {
			SynFlyingStart(gBasePar.FullFreq01,gInvInfo.InvCurrent,gMotorInfo.Current);
		}
	}    
	else											// ��������
	{
		gFlyingStart.CurIU = (s16)(gIUVWQ24.U >> 12);
		gFlyingStart.CurIV = (s16)(gIUVWQ24.V >> 12);
		gFlyingStart.CurIW = (s16)(gIUVWQ24.W >> 12);
	} 
}

