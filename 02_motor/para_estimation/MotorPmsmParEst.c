/***************************************************************************
�ļ����ܣ�ͬ����������ʶ��������ʼλ�ü�⣬���������λ�á������ʶ
�ļ��汾��
���¸��£�

***************************************************************************/
//�ļ����ܣ�
//    1. ͬ�����ż�λ�ü����жϳ����2ms���� �����޸�ePWMģ��ʵ�ָ��෢����
//    2. ���ż�λ��ʱ˳���ʶ��ͬ�����ߵ�У�d��q���У�
//    3. ���λ�ýǵĿ��ر�ʶ��������������
//    4. ���λ�ýǵĴ��ر�ʶ��(*** ע����Ҫ�û����ı�����������̽)
//    5. �Ա�������֧�ְ���ABZ��UVW, ��ת��ѹ����
//       UVW����������uvw�źŵ����Ϊֱ�Ǻ�uvw�źŷ���
//    6. ͬ�������綯�Ƶı�ʶ��

#include "MotorInclude.h"
#include "MotorIPMSVC.h"

/*******************�ṹ������******************************************/
IPM_ZERO_POS_STRUCT		gIPMZero;         //����ͬ����������������λ�ýǵ����ݽṹ
IPM_INITPOS_PULSE_STR	gIPMInitPos;      // pmsm ��ʶ�ż�λ��
PMSM_EST_BEMF           gEstBemf;         // PMSM��ʶ���綯������
PMSM_EST_PARAM_DATA		gPmParEst;        // ����ͬ������ʶʱ�ĺ͹��ܵ����ݽ�����

/*******************��غ�������******************************************/
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
	��ʼλ�ýǼ�������ʼ��ʶ���������λ�ýǵĳ�ʼ������
************************************************************/
void InitSetPosTune(void)
{
	ResetParForVC();
	ResetADCEndIsr();

    gPGDir.ABDirCnt = 0;                    // ��λ���б���������
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
	pm �ż�λ�ñ�ʶ, ��Ҫ���ж��г������
��ʶ����: �����ѹ��, dq������˳��õ���
��ʶʱ�����Լ���������������״̬��
*************************************************************/
void SynTuneInitPos(void)
{	
	switch(gGetParVarable.IdSubStep)
	{
		case 1:                                             // ��ʼ������
		    SetADCEndIsr(ADCEndIsrTune_POLSE_POS);
			gIPMInitPos.Waite = 0;
            //gIPMInitPos.InitPWMTs	= (50 * DSP_CLOCK);	    //50us
            //gIPMInitPos.InitPWMTs	= (50 * DSP_CLOCK);	    //50us		
			gGetParVarable.IdSubStep++;
			break;
            
		case 2:                                             // ��ʱ�ȴ�20ms, �ٳ�ʼ����ر�������PWM����
			gIPMInitPos.Waite ++;
			if(gIPMInitPos.Waite > 10)			
			{
				gIPMInitPos.Waite = 0;
       			gIPMInitPos.Step = 1;                       // �þ�̬��ʶ��־
       			//gMainStatus.PrgStatus.bit.PWMDisable = 1;   // ��EPWM ����
       			gGetParVarable.IdSubStep++;
			}
			break;
            
		case 3:                                             // �ȴ��ж��о�̬��ʶ���...
			if(gIPMInitPos.Step == 0)
			{
				gIPMPos.CompPos = ((Ulong)gIPMPos.CompPosFun << 16) / 3600;
				gIPMPos.InitPos = gIPMPos.InitPos + gIPMPos.CompPos;
				SetIPMPos((Uint)gIPMPos.InitPos);
				SetIPMRefPos((Uint)gIPMPos.InitPos);
                SvcSetRotorPos(gIPMPos.InitPos);
                gPhase.IMPhase  = (int)gIPMPos.InitPos;
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
    		    gGetParVarable.ParEstMstep++;               //�л�����һ��ʶ����
				//gPhase.IMPhase  = (int)gIPMPos.RotorPos;
                //gPhase.IMPhaseApply = ((long)gPhase.IMPhase << 16);					
                gMainStatus.PrgStatus.all = 0;
            }
			break;	

        default:
            break;
	}
}

/************************************************************
�ϵ��ʼλ�ýǼ�����(�������жϣ���ADת�������ж���ִ��)��
��U���ź�VW���Ŵ�Ϊ��˵����
	1��PWM1B��PWM2A��PWM3Aǿ�ƹرգ��ߵ�ƽ����
	2����ʼǿ������PWM1A��PWM2B��PWM3BΪ�ߵ�ƽ
	3����������������Ϊ0 (���μ����ȡƽ��ֵ)
	4) ����gIPMInitPos.StepΪ1��ʼ������ֹλ�õ�г, ��ɺ�����0
************************************************************/
void SynInitPosDetect(void)
{
	int  m_Cur;
    int  m_Index;
	static Uint m_Wait = 0;
	Uint Sec,Cur_Lim,i,temp0,temp1;
	int m_Data;
	Ulong temp;

	switch(gIPMInitPos.Step)
	{
		case 1:							                    //��ʼ����
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
        		gIPMInitPos.CurLimitBig = temp / gMotorInfo.Current;				// ����������ڱ������λ�ñ�ʶ��͹��������Ա�ʶ
                temp = (Ulong)gIPMInitPos.CurLimitSmall * gInvInfo.InvCurrent;                
        		gIPMInitPos.CurLimitSmall = temp / gMotorInfo.Current;				// С���������ڵ�м�������ȱ����
        	}
/*
			temp0 = (Uint)((Ulong)gMotorInfo.Current * gIPMInitPos.CurLimitAdjSmall / 100);	// С����ʵ��ֵ
			temp1 = (Uint)((Ulong)gInvInfo.InvCurrent * 50 / 100);								// ��Ƶ������50%
			if(temp0 < temp1)									// С������С�ڱ�Ƶ���������50%, �����ڵ�������
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
			gIPMInitPos.PWMTs = 0;									// Ϊ case2 �е�һ�η�����׼������֤��һ�η�������ʱ

            gIPMInitPos.Step ++;
			break;

		case 2:							                            //�����ʵ��������
			EnableDrive();
			
			if(gIPMInitPos.Section > 5)								// ȷ��Ҫ���ĵ�ѹʸ��
			{
				Sec = gIPMInitPos.Section - 6;
				Cur_Lim = gIPMInitPos.CurLimitBig;
			}
			else
			{
				Sec = gIPMInitPos.Section;
				Cur_Lim = gIPMInitPos.CurLimitSmall;
			}

			if(Sec < 2) 											// ȷ��Ҫ�����ĵ���
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

					if(m_Wait > (gIPMInitPos.PWMTs / 600))			// ������ǰ������ʱ
					{
						m_Wait = 0;
						SynInitPosDetSetPwm(Sec);                   // ������
						gIPMInitPos.PeriodCnt ++;
					}

					break;
				case 1:
					if(abs(m_Cur) >= Cur_Lim)						// ��������ﵽ�趨ֵ
					{
						gIPMInitPos.WidthFlag = 1;
					}

					if(gIPMInitPos.WidthFlag == 1)					// ��⵽һ������
					{
						gIPMInitPos.WidthFlag = 0;
						SynInitPosDetSetPwm(7);
						gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[gIPMInitPos.Section];		// Ϊ���ٴη���֮ǰ����ʱ
						gIPMInitPos.Section ++;												// ��12��������Ϊ12����
						if(gIPMInitPos.Section < 12)
						{
							gIPMInitPos.PeriodCnt = 0;
						}
						else
						{
							gIPMInitPos.PeriodCnt = 2;
						}
					}
					else											// �����ӳ�����ʱ��
					{
						gIPMInitPos.PWMTsBak[gIPMInitPos.Section] += gIPMInitPos.InitPWMTs;
						if(gIPMInitPos.PWMTsBak[gIPMInitPos.Section] >= 60000)				// ��ֹ������������������޷�
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

					if(m_Wait > (gIPMInitPos.PWMTs / 600))							// �ȴ������½���0
					{
						m_Wait = 0;
					 	gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[0];

						for(i=1;i<6;i++)
						{
							if(gIPMInitPos.PWMTs > gIPMInitPos.PWMTsBak[i])			// ѡ�����ʵ�С����
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

		case 3:													// ��������¼����(С)
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
						gIPMInitPos.PWMTs4L = gIPMInitPos.PWMTs;	// ����ʱ�����ڵ�м���
						gIPMInitPos.PWMTs = 6000;					// ��һ��������������к�С��������խ�������㲻�굼�·�����һ�����ʼӴ���ʱ
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

		case 4:							// �����к�͹���ʣ��жϵ������            
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
			
			gIPMInitPos.PWMTsBak[0] = gIPMInitPos.PWMTs;		// Ϊȱ���Ᵽ������
			
			SynInitPosDetSetTs();
			gIPMInitPos.PeriodCnt = 0;
			gIPMInitPos.Section = 0;
			gIPMInitPos.Step ++;
			break;

		case 5:										// ��������¼����(��)
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
            (gUVCoff.BeforeRunTuneEnable == 2))  //���гʱҲ�ж�ȱ��
            {
				gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[0];			// Ϊȱ����ָ�����
				SynInitPosDetSetTs();
				SynInitPosDetSetPwm(7);
				gIPMInitPos.PhsChkStep = 0;
                gIPMInitPos.Step ++; // ����ͬ����ȱ����
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
                    if(abs(gIPMInitPos.CurIU) < (gIPMInitPos.CurLimitBig>>3))    // U��ȱ��
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault4 = 11;
                        gIPMInitPos.Step ++;
                    }
                    if(abs(gIPMInitPos.CurIV) < (gIPMInitPos.CurLimitBig>>3))    // V��ȱ��
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
                    if(abs(gIPMInitPos.CurIW) < (gIPMInitPos.CurLimitBig>>3))    // W��ȱ��
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault4 = 13;
                        gIPMInitPos.Step ++;
                    }
                    if(abs(gIPMInitPos.CurIV) < (gIPMInitPos.CurLimitBig>>3))    // V��ȱ��
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
			gIPMInitPos.Step = 0;           // �ж�ִ�����
			DisableDrive();
			break;
	}
}

/*************************************************************
	��ת�Ӵż�λ��ǿ������ΪPos�ĺ���
*************************************************************/
void SetIPMPos(Uint Pos)
{	
    DINT;
	gIPMPos.QepBak   = GetQepCnt();	
    gIPMPos.RotorPos = Pos;
    EINT;
}
/*************************************************************
	����gPGData.RefPos
*************************************************************/
void SetIPMRefPos(Uint Pos)
{	
    gPGData.RefPos = ((u32)Pos<<8);
    gAsr.PosSet = gPGData.RefPos;
}

/************************************************************
	LAB��LBC��LCA���м��㺯��
	gIPMInitPos.Cur���棺IU+��IU-��IV+��IV-��IW+��IW-��
	                     IU+��IU-�IV+��IV-��IW+��IW-
************************************************************/
void SynCalLabAndLbc(void)
{
	Uint  m_Index,m_Sel;
	Uint  m_DetaI,m_Cur,m_UDC;
	Ullong m_LTemp;

	//���LAB��LBC��LCA
	for(m_Index = 0;m_Index < 3;m_Index++)
	{
		m_Sel = m_Index<<1;
		m_Cur = (gIPMInitPos.Cur[m_Sel] <= gIPMInitPos.Cur[m_Sel+1]) ?
						gIPMInitPos.Cur[m_Sel] : gIPMInitPos.Cur[m_Sel+1];
		m_DetaI = m_Cur;
		m_DetaI = ((Ulong)m_DetaI * (Ulong)gMotorInfo.Current) >> 12;	//(��λ0.01A or 0.1A)
		m_UDC	= gUDC.uDCFilter - (Ulong)m_DetaI * (Ulong)gMotorExtReg.RsPm / 10000;      // 0.1V
		m_LTemp = (Ulong)gIPMInitPos.PWMTs4L * 2L*100L/DSP_CLOCK;				            //(��λ10ns)
		m_LTemp = (Ulong)m_UDC * (Ulong)m_LTemp;
		//L=((��ѹ/10)*(ʱ��/10^5)/(����/100))*100 (0.01mH)
		//L = ��ѹ*ʱ��/(����*100)  (0.01mH)
		gIPMInitPos.LPhase[m_Index] = (m_LTemp)/((Ulong)m_DetaI * 100);
	}
}

/************************************************************
	��ʼλ�ü��㺯����ͬʱ���������С�
************************************************************/
void SynInitPosDetCal(void)
{
	static int  m_X,m_Y;
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
    }*///��Ч��:�������ƫ��֮��С�ڵ���������1/40����Ϊ�޷�ʶ��ż���ʼλ��

	gIPMPos.InitPos = (Uint)user_atan(m_X, m_Y) - 5461;   // 30deg
	gIPMPos.InitAngle_deg = (Ulong)gIPMPos.InitPos * 3600 >> 16;
}

/************************************************************
	�����ز����ڼĴ�������
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
	��ʼλ�ü��׶Σ�����PWM�Ķ����Ĵ���������PWM�͵�ƽ��Ч
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

		case 6:									//�ָ�PWMģ��ļĴ�������
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

	case 8:									// A-, B-, C-, ����ת�ٸ���, �����������Ų���
			EPwm1Regs.AQCSFRC.all = 0x06;
			//pPWMForU->AQCSFRC.bit.CSFB = 2;
			EPwm2Regs.AQCSFRC.all = 0x06;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x06;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		default:								//ͬ������ʼλ�ýǼ���ʼ���Ĵ���
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
* Function Name  : ��������µ�г��������㲹���Ǻͱ������Ľ��߷�ʽ
* Description    : 	��
����ת������ʶ��������Ľ��߷�ʽ�������������λ�ýǡ�
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

            gPhase.IMPhase = (int)gIPMPos.InitPos;
            gIMTSet.M = 0L <<12;
            gIMTSet.T = 0;
            gIPMZero.CurLimit = 3000UL * (Ulong)gMotorInfo.CurrentGet / gMotorInfo.Current;  //�����70%����
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
            
            if(gIMTSet.M > ((long)gIPMZero.CurLimit <<12))      // M ������������̴���, 200ms
            {
                gIMTSet.M = (long)gIPMZero.CurLimit << 12;
                gGetParVarable.IdSubStep++;                
            }
			break;
                        
	 	case 3:		
            if(gMainCmd.FreqSet >= 0)
            {
                gPhase.IMPhase += (gMotorExtInfo.Poles * 11);       // Լ5RPM���ٶ�תһȦ
                gPGData.MoveDircetion = DIR_FORWARD;             //�����趨Ƶ��Ϊ���Ӱ��
            }
            else
            {
                gPhase.IMPhase -= (gMotorExtInfo.Poles * 11);       // Լ5RPM���ٶ�תһȦ
                gPGData.MoveDircetion = DIR_BACKWARD;
            }
            m_WaiteCnt++;
			
			m_Dir1 = JudgeABDir();											
            m_Dir2 = JudgeUVWDir();	

            if(m_WaiteCnt > 6000)                                   // תһȦ���жϽ���
	        {
                m_WaiteCnt = 0;
                if(m_Dir1 == DIR_ERROR)    
                {
                    gError.ErrorCode.all |= ERROR_ENCODER;
                    gError.ErrorInfo[4].bit.Fault3 = 1;         // abz �źŷ������
					m_WaiteCnt = 0;
                }      	        
                else if(m_Dir1 * gPGData.MoveDircetion < 0)              //AB����ȡ��
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
                        gError.ErrorInfo[4].bit.Fault3 = 2;         // abz �źŷ������
                        m_WaiteCnt = 0;
                    }
                    else if(m_Dir2 * gPGData.MoveDircetion < 0)             //����λ�÷���ȡ��
                    {
                        gPmParEst.UvwDir = !gPmParEst.UvwDir;
    			    }  
                }
                
			    gGetParVarable.IdSubStep++;	
                if((gPGData.PGType == PG_TYPE_RESOLVER)||(gIPMPos.ZSigEnable == 0))
                {
                    gGetParVarable.IdSubStep++;                               // ���䲻��Ҫ�ȴ�Z�ź�
                    gIPMPos.RotorPosBak = gIPMPos.RotorPos;             // ��¼���ת��λ��
					m_WaiteCnt = 0;
					m_WaiteCnt1 = 0;
                }
                gIPMPos.ZResetFlag = C_Z_RESET_POS_NOLIMIT;         // ��ʾ��ʶ��ʱ������Z�ź�
                m_ZNum = gIPMPos.ZSigNum;
            }
			break;
            
		case 4:
            if(gMainCmd.FreqSet >= 0)
            {
                gPhase.IMPhase += (gMotorExtInfo.Poles * 22);       // Լ10RPM���ٶ�תһȦ
            }
            else
            {
                gPhase.IMPhase -= (gMotorExtInfo.Poles * 22);       // Լ10RPM���ٶ�תһȦ
            }
	        m_WaiteCnt++;
	        if(m_ZNum != gIPMPos.ZSigNum)                           // �ȴ�Z�źŰѵ��ת��λ�ýǸ�λΪ���������λ�ý�
	        {
	            m_WaiteCnt = 0;
				m_WaiteCnt1 = 0;
                gIPMPos.RotorPosBak = gIPMPos.RotorPos;             // ��¼���ת��λ��
	            gGetParVarable.IdSubStep++;
	        }
	        else if(m_WaiteCnt > 9000)                       // 3Ȧ��û�յ�Z�źţ�������
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
				 gIPMPos.RotorPosBak = gIPMPos.RotorPos;   // Ϊ�Ƕ�ƫ���ʼ��
		         break;
		    }
		    else
		    {
		        m_WaiteCnt1 = 5;                
		    }  
		     
            if(gMainCmd.FreqSet >= 0)
            {
                gPhase.IMPhase += (gMotorExtInfo.Poles * 6);       // Լ5RPM���ٶ�תһȦ
				m_Data = (s16)(gIPMPos.RotorPos - gIPMPos.RotorPosBak);     // �ۻ�������������λ�ã����պ͸�����λ�ý��бȽϣ��жϱ������Ƿ�δ�ӻ�����������
				m_PosDeta += m_Data;				
                gIPMPos.RotorPosBak = gIPMPos.RotorPos;
            }
            else
            {
                gPhase.IMPhase -= (gMotorExtInfo.Poles * 6);       // Լ5RPM���ٶ�תһȦ
				m_Data = (s16)(gIPMPos.RotorPosBak - gIPMPos.RotorPos);
				m_PosDeta += m_Data;
                gIPMPos.RotorPosBak = gIPMPos.RotorPos;
            }
            m_DetaPos = gPhase.IMPhase - (s16)gIPMPos.RotorPos;		// ���������λ�ý�=�ż���ʵ�Ƕ�-�����������Ƕ�
    		m_WaiteCnt++;

            if(m_WaiteCnt == 2000)
            {
            	gIPMZero.FirstPos = m_DetaPos;                  //�����˲�����ʶ֮ǰ���������訵����λ�ýǶ?
                m_TotalPosErr = 0;
            }
            else if(m_WaiteCnt > 2000)
            {
            	m_TotalPosErr += (m_DetaPos - gIPMZero.FirstPos);
    	    	if(m_WaiteCnt > (2000 + 8192))
    	    	{
				    m_s32 = m_PosDeta - (s32)gMotorExtInfo.Poles * 61152L;      // 61152 = 10192*6
				    if((abs(m_s32) > 8192L)&&(gIPMPos.TuneDetaPosDisable == 0))            // 8192��Ӧ45��Ƕ�,�����������Ĺ��ܣ����ӹ��������δ˹���
					{
						gError.ErrorCode.all |= ERROR_ENCODER;
	                    gError.ErrorInfo[4].bit.Fault3 = 7;         // �����źŹ���,����δ�����俨,���߱������������ô���
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
            
		case 6:  //�ӳ�һ��ʱ��ȴ�ת��λ���ȶ�����ָ���Ƕȣ���¼��ʱ�ĽǶȢ��QepTotal ��ֵ
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
		    gGetParVarable.ParEstMstep++;       //�л�����һ��ʶ����
		    break;
	}
	
}
/************************************************************
	���ر��������λ�ñ�ʶ
* ��Ҫ�ջ�ʸ�����У�
* ��Ҫ������ϸ����Ӽ��ٹ���Ƶ�ʸ���
* Ŀ��Ƶ�ʣ� �Ӽ���ʱ�����û����ã�
* ������������Ҫ�û����ã��粻�ܳɹ�����Ҫ������̽��
* uvw������ʱ��uvw�źŵķ��������ã�����ֱ�ӱ�ʶ�ó���
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

    /*ִ��ͬ�����ջ�ʸ���ٶȻ�*/

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
				if(m_Cnt < 5)      // ����������ȵ�10ms���Ȳ��������ٽ��д��ر�ʶ,wyk
				{
				    m_Cnt++;
					break;
				}
				else
				{
				    if(m_Cnt < 15)
					{
					    m_Cnt++;
					    m_RTPosAdd += gRotorTrans.RTPos;     // �ۻ���ƽ�������ȸ��ߣ�wyk
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
			m_CurLimit = Min(m_CurLimit , 7373UL * (Ulong)gMotorInfo.CurrentGet / gMotorInfo.Current);  //��������180%����
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
            gGetParVarable.StatusWord = TUNE_ACC;    //��ʼ����
             
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
            else if(abs(gPGDir.ABDirCnt) > 36)           // ���ת��һȦ���жϾ���λ���ź��Ƿ���ȷ
            {
                if(m_Dir2 == DIR_ERROR)
                {
                    gError.ErrorCode.all |= ERROR_ENCODER;
                    gError.ErrorInfo[4].bit.Fault3 = 4;         // abz �źŷ������
                }
                if(m_Dir2 * gMainCmd.FreqSet < 0)                  
                {
                    gPmParEst.UvwDir = !gPmParEst.UvwDir;
			    }    
                m_DirFlag = 1;
            }		
            m_It = Filter64(abs(gIMTSet.T >> 12),m_It);  //���˲�,��ֹ��
            if((((m_It >= m_CurLimit)&&(abs(gRotorSpeed.SpeedApply) <= 10))||(abs(gRotorSpeed.SpeedApply) >= (1000L<<15/gBasePar.FullFreq01)))&&(gIPMPos.LoadTuneErrorEnable == 1))
			{
				gPGData.PGDir = !gPGData.PGDir;
				EQepRegs->QDECCTL.bit.SWAP = gPGData.PGDir;
				DisableDrive();                         //ֹͣ����
				ResetAsrPar();
			    gGetParVarable.IdSubStep = 5;
			    gGetParVarable.StatusWord = TUNE_DEC;    //��ʼ����	
			    m_WaiteCnt = 0;
				m_WaiteCnt1 = 0;
				m_TuneNum++;
			    break;		
			}
            
            if(abs(gPGDir.ABDirCnt) > 180 * gMotorExtInfo.Poles)  // ��е��5Ȧ
            {
                gError.ErrorCode.all |= ERROR_ENCODER;
                gError.ErrorInfo[4].bit.Fault3 = 5;         
            }
            if((m_DirFlag == 1) && (m_ZFlag == 1))
            {
        		m_WaiteCnt = 0;
				if(m_TuneNum == 1)                    // �ı䷽���г�ɹ��������г������wyk
				{
				    m_TuneNum = 0;
				}
                gGetParVarable.StatusWord = TUNE_DEC;    //��ʼ����
			    gGetParVarable.IdSubStep++;
            }
	        break;

		case 3:                                 // ���ٽ׶�
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
    			gGetParVarable.ParEstMstep++; //�л�����һ��ʶ����
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
				if(m_WaiteCnt > 1000)   // �ȴ��������ڻ������г����ʶ��λ�ø�׼��wyk
	            {
	                m_WaiteCnt = 0;
	                gGetParVarable.ParEstMstep--; //���½��г�ʼλ�ýǱ�ʶ�ʹ��ر�ʶ
					gGetParVarable.IdSubStep = 1; 					
	            }
			}
			else
			{
			    m_WaiteCnt1++;
				if(m_WaiteCnt1 > 5000)   // �ٶȲ�Ϊ0������10��
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
	ͬ�������綯�Ʊ�ʶ
* ��Ҫ�����ջ���Ϸ�M�������
* ���������ΪΪ: IdSet = 1500, 3000(Q12)��
* �����ϼ�¼d�������Լ�����ߵ�����q�������С��
* �����ϼ�¼q���ѹ��Լ��?��)��ѹ��d���ѹ��С��
* �򻯷ų�: Uq �� w(Ld * Id + Phi_r);

* �Ӽ��ٵļӼ��ٹ���Ƶ��ָ�� �ɱ�ʶ���������
* Ŀ��Ƶ�ʣ��Ӽ���ʱ���ɹ��ܲ���������:
    Ŀ��Ƶ��:   40% ����ת�٣�
    �Ӽ���ʱ��: ����22����: 30sec������22����: 50sec��
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
	            //gGetParVarable.StatusWord = TUNE_ACC;       // ���ܿ�ʼ����
	            gEstBemf.TuneFreqAim = (long)gMotorInfo.FreqPer * 2L / 5L;      // 40% ����Ƶ��
	            gEstBemf.FreqRem = 0;                                           // Ƶ�ʲ�����������
	            gEstBemf.AccDecTick = (gInvInfo.InvTypeApply <= 20) ? (30L*1000L/2) : (60L*1000L/2);
	                                                                    // ����22����: 30sec ���ٵ��Ƶ��
	                                                                    // ����22����: 60sec ���ٵ��Ƶ��
	            //gEstBemf.TuneFreqSet = 0;
	            //EnableDrive();
	            gGetParVarable.IdSubStep ++;
			}
            break;
            
        case 2:                        // ���ڼ��ٹ���
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

        case 3:                         // ���ֵ����͵�ѹ----1
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt <= 2048)
            {
                gEstBemf.TotalId1 += gLineCur.CurPer;           // �������Ϊ��ֵ
                gEstBemf.TotalVq1 += gOutVolt.VoltApply;        // �����ѹΪ��ֵ
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

        case 4:                     // �޸ĵ���ֵ
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

        case 5:                         // ���ֵ�����ѹ2
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt <= 2048)
            {
                gEstBemf.TotalId2 += gLineCur.CurPer;           // �������Ϊ��ֵ
                gEstBemf.TotalVq2 += gOutVolt.VoltApply;        // �����ѹΪ��ֵ
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

        case 6:                         // �ȴ�����
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
            if(gEstBemf.Cnt < 500)          // �ȴ�ͣ��ͣ��
            {
                gEstBemf.Cnt ++;
            }
            else
            {        
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 8:                         // ���㷴�綯��ϵ������ɱ�ʶ           
            temp1 = (Ulong)gEstBemf.TotalVq2 * gEstBemf.TotalId1;
            temp2 = (Ulong)gEstBemf.TotalVq1 * gEstBemf.TotalId2;
            temp3 = (gEstBemf.TotalId2 - gEstBemf.TotalId1) * gEstBemf.TuneFreq;
            fluxRotor = (((Ullong)temp2 - (Ullong)temp1) <<15) /temp3; // Q: 12+12-15 + 15-12 = Q12

            temp3 = fluxRotor * abs(gMotorInfo.FreqPer) >> 12;        // Q15
            gEstBemf.BemfVolt = temp3 * gMotorInfo.Votage * 10L >> 15;              // 0.1 V

            gMainStatus.PrgStatus.all = 0;
            gGetParVarable.IdSubStep = 1;
			gGetParVarable.ParEstMstep ++; //�л�����һ��ʶ����

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
�������ܣ����������?
*****************************************************************/
/*void RsIdentify(void)
{
	Uint  m_UData1,m_UData2,m_UData3,m_MaxCur;
	Ulong m_ULong1,m_ULong2;
 
    SetADCEndIsr(ADCEndIsrTuneR1);

    m_MaxCur = ((Ulong)gInvInfo.InvCurrent * 3277)>>12;     //80%��Ƶ�����?
	switch(gGetParVarable.IdSubStep)
	{
		case 1:     // ��ʶ���ӵ���ʱ�����Ĵ���, EPWMģ�������, �����ĳ�ʼ��
#if (SOFTSERIES == MD380SOFT)
		    if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD_1140; /*1140V���ӵ����ʿ��Ƶ�̶�1K,2011.5.7 L1082*/
/*              }
            else if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD_690; /*690V���ӵ����ʿ��Ƶ�̶�1.5K��2011.6.20 L1082 */
/*              }
            else
#endif
              {
               gUVCoff.Rs_PRD = TUNE_Rs_PRD;         /*380V���ӵ����ʿ��Ƶ�̶�2K��2011.7.19  L1082 */     
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
            //EPwm1Regs.ETSEL.bit.INTEN = 0;  //���ӵ����ʶʱ����ֹ�����ж�
			EPwm1Regs.TBPRD = gUVCoff.Rs_PRD;
			//EPwm1Regs.CMPB  = EPwm1Regs.TBPRD - gADC.DelayApply;
			EPwm2Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm3Regs.TBPRD = gUVCoff.Rs_PRD;

			EPwm1Regs.CMPA.half.CMPA = gUVCoff.Comper;
			EPwm2Regs.CMPA.half.CMPA = 0;//gUVCoff.Comper;

			//EPwm3Regs.AQCSFRC.bit.CSFA   = 2;
			//EPwm3Regs.AQCSFRC.bit.CSFB   = 2;
			EPwm3Regs.AQCSFRC.all = 0x0A;		//�ر�3�ű�
			EPwm3Regs.DBCTL.all   = 0x0000;
			EDIS;
            
			gGetParVarable.IdSubStep = 2;               
			break;

		case 2:         // ���α�ʶʱ��ʱ֮���ٿ�������
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
		 	    m_UData1 = (2048UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;//��Ƶ��������?�����ϵ�������
			}

			if(0 == gUVCoff.IdRsCnt)            // ��һ�α�ʶ���Լ�С��������ֹ����
			{ 
			    m_UData1 = (512UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current;     
			}

			if(gGetParVarable.IdSubStep == 5)
            {                
                if( 16 < gInvInfo.InvTypeApply )
                {
                    m_UData1 = (3500UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;  //������ʱ������
                }
				else
				{
				    m_UData1 = (4096UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;
				}
            }
            if( gMotorInfo.Current > m_MaxCur )  //�����������ڱ�Ƶ�������ʱ���Ա�Ƶ�������Ϊ׼
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
                    m_ULong1 =  m_ULong1 * 10;//��ѹ��һ��С���㣬���?��С���㣬����Ҫ����10
					//m_ULong1 = ��ѹ��(V)��10��2^14 
					m_UData2 = (gUVCoff.TotalI - gUVCoff.TotalIL)>>7;
					m_ULong2 = ((Ulong)m_UData2 * (Ulong)gMotorInfo.Current)>>10;
					//m_ULong2 = ������(A)��10��2^4
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
                        gUVCoff.IdRsDelay = 1000; //�ڶ��α�ʶ֮ǰ����ʱ2s
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
            InitSetPWM();                       //�ָ��޸ĵļĴ�������
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
	if(STATUS_RS_TUNE == gMainStatus.RunStep)  //������ʶʱ�ӳ����ӵ����ʶʱ��
	{
	    if(gInvInfo.InvTypeApply > 18)   //37Kw�������Ӷ��ӵ����ʶʱ��
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
                gUVCoff.Rs_PRD =SHORT_GND_PERIOD_1140; /*1140V���ӵ����ʿ��Ƶ�̶�1K,2011.5.7 L1082*/
            }
            else if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
            {
                gUVCoff.Rs_PRD =SHORT_GND_PERIOD_690; /*690V���ӵ����ʿ��Ƶ�̶�1.5K��2011.6.20 L1082 */
            }
            else
#endif
            {
                gUVCoff.Rs_PRD = TUNE_Rs_PRD;         /*380V���ӵ����ʿ��Ƶ�̶�2K��2011.7.19  L1082 */     
            }  		

            if( gMotorInfo.Current > gInvInfo.InvCurrent )  //�����������ڱ�Ƶ�������ʱ���Ա�Ƶ�������Ϊ׼
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
			EPwm1Regs.TBPRD = gUVCoff.Rs_PRD;  //������Ч
			EPwm2Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm3Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm1Regs.AQSFRC.all  = 0xC0;
            EPwm2Regs.AQSFRC.all  = 0xC0;
            EPwm3Regs.AQSFRC.all  = 0xC0;
			EPwm1Regs.AQCSFRC.all 	= 0x00;		//������Ч			
			EPwm1Regs.DBCTL.all 	= 0x0007;
			
			EPwm2Regs.CMPA.half.CMPA= 0;			//����������Ч
			EPwm2Regs.AQCSFRC.all 	= 0x0;	
			EPwm2Regs.DBCTL.all 	= 0x0007;      //������Ч
			
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
				if( gMotorInfo.Current > gInvInfo.InvCurrent )  //�����������ڱ�Ƶ�������, �Ա�Ƶ�������Ϊ׼
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
				if( gMotorInfo.Current > gInvInfo.InvCurrent )  //�����������ڱ�Ƶ�������ʱ���Ա�Ƶ�������Ϊ׼
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
//				m_UData0 = (gUVCoff.TotalI << 4) / (gUVCoff.TotalIV >> 8);			// û��

			    if(STATUS_GET_PAR == gMainStatus.RunStep)					// ������ʱ�� m_UData7
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
				m_UData1	= gUVCoff.TotalComperL- m_UData7;				// Case3 ��ʱ�� - ����ʱ��
				m_UData2 	= (gUVCoff.TotalVoltL<<3)/gUVCoff.Rs_PRD;	// ʱ�仮��
				m_ULong1 	= ((Ullong)m_UData1 * m_UData2)>>7;				// ��ȥ�����ĵ�ѹ, 2^8	>>11
				
				//m_UData3	= gUVCoff.TotalComper;
				m_UData3	= gUVCoff.TotalComper - m_UData7;
				m_UData4 	= (gUVCoff.TotalVolt<<3)/gUVCoff.Rs_PRD;
				m_ULong2 	= ((Ullong)m_UData3 * m_UData4)>>7;				// >>11
				m_ULong3 	=  m_ULong2 - m_ULong1;//��ѹ����һ��С���㣬����2��С���㣬����Ҫ����10
				m_ULong4    = m_ULong3 *10;			// m_ULong4 ��ѹ֮��, 0.01V, 2^8
				//gUVCoff.m_ULong4 = gUVCoff.m_ULong2 *10;
				//m_ULong1 = ��ѹ��(V)��10��2^14 
                if(STATUS_GET_PAR == gMainStatus.RunStep)						// m_UData5 �������ֵ
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
				//m_ULong2 = ������(A)��10��2^4
				/*while((m_ULong2>>16) != 0)
				{
					m_ULong1 = m_ULong1>>1;
					m_ULong2 = m_ULong2>>1;
				}*/
				m_ULong5 = m_ULong4 / m_ULong6;											// ��ѹ�� / ������		2^10
				m_UData2 = (m_ULong5 * (Ullong)1000)>>11;							// 2^0/2	// >>7

				gMotorExtReg.RsPm = m_UData2;											// ����ʵ��ֵ
				m_UData3 = ((Ulong)gMotorExtReg.RsPm * (Ulong)gMotorInfo.Current)/gMotorInfo.Votage;	
                gMotorExtPer.Rpm = ((Ulong)m_UData3 * 18597)>>14;		
				gUVCoff.Flag = 0;
				gUVCoff.RsTune = 2;
				DisableDrive();
				ResetADCEndIsr(); 
				InitSetPWM();                       //�ָ��޸ĵļĴ�����	
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
		            gMainStatus.PrgStatus.all = 0;		//���п�����Ч
				}
			}
		    break;
	}	
}

void BeforeRunRsIdentifyICal(void)
{
    s16 m_Data;
    if( gMotorInfo.Current > gInvInfo.InvCurrent )  //�����������ڱ�Ƶ�������ʱ���Ա�Ƶ�������Ϊ׼
    {
        m_Data = ((long)gInvInfo.InvCurrent *500L) / gMotorInfo.Current;
    }
	else
	{
	    m_Data = 500; 
	}

	gRsIdentifyPID.Deta = (int)(gUVCoff.CurComperCoff - gUVCoff.Temper);

    if((gRsIdentifyPID.Deta > 0)&&(gUVCoff.Flag == 0))		// �������ڷ���
	{
	    if(gInvInfo.InvTypeApply > 16)							// ���ӿ���ʱ��
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
	else if(gRsIdentifyPID.Deta < - m_Data)						// �����ȷ���С500
	{
	    if(gUVCoff.Comper > (gDeadBand.DeadBand>>1) + 5)		// ���������������
		{
	        if(gInvInfo.InvTypeApply > 16)						// ��С����ʱ��
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
		gUVCoff.Flag = 1;													// ˵������ʱ�����ڼ�С
	}
	else if((gRsIdentifyPID.Deta > m_Data)&&(gUVCoff.Flag == 1))		// �����ȷ�����500, �ҿ������ڼ�С
	{
	    if(gInvInfo.InvTypeApply > 16)									// ���ӿ���ʱ��
        {         
           gUVCoff.Comper += 1;         
        }
        else
        {
		   gUVCoff.Comper += 2;
        }
        gUVCoff.Number = 0;
	}
	
	if(gUVCoff.Number == 0)  // ˵��������������߼�С, �����е�ѹ_�����ۻ�
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
	else						// �ѽ�����̬, ��ʼ�ۼƵ�ѹ_����_ʱ��
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
    if(EPwm1Regs.TBSTS.bit.CTRDIR == 1)  // ������
	{
        return;	
	}
#if (SOFTSERIES == MD380SOFT)
	if(GetOverUdcFlag())                    //��ѹ����
    {
       HardWareOverUDCDeal();				
    }
#endif    
    GetUDCInfo();							//��ȡĸ�ߵ�ѹ��������    
	GetCurrentInfo();						//��ȡ��������, �Լ��¶ȡ�ĸ�ߵ�ѹ�Ĳ���	
	ChangeCurrent();
 	BrakeResControl(); 

	gUVCoff.Temper  = abs(gIUVWQ24.U>>12);	
	gUVCoff.TemperV = abs(gIUVWQ24.V>>12);
	BeforeRunRsIdentifyICal();	
}
/*******************************************************************************
* Function Name  : ��ʶ���ӵ���ʱ���ADC�����жϴ�������
* Description    : IfPWMPeriodInt()�����ڵ�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void ADCEndIsrTuneR1(void)
{
	if(EPwm1Regs.TBSTS.bit.CTRDIR == 1)  // ������
	{
        return;	
	}
#if (SOFTSERIES == MD380SOFT)
	if(GetOverUdcFlag())                    //��ѹ����
    {
       HardWareOverUDCDeal();				
    }
#endif    
    GetUDCInfo();							//��ȡĸ���ѹ�������?   
	GetCurrentInfo();						//��ȡ������? �Լ��¶ȡ�ĸ�ߵ�ѹ�Ĳ���	
	ChangeCurrent();						//������ֳ����µĵ�����
	BrakeResControl(); 
}
*/
/*******************************************************************************
* Function Name  : ��ʶ��ʼλ�ýǵ�ʱ��ADC�����жϦ�����
* Description    : IfPWMPeriodInt()�����ڵ�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADCEndIsrTune_POLSE_POS(void)
{
    EALLOW;             //28035��ΪEALLOW����
    ADC_CLEAR_INT_FLAG;
    EDIS;
    EINT;
#if (SOFTSERIES == MD380SOFT)
    if(GetOverUdcFlag())                    //��ѹ����
    {
       HardWareOverUDCDeal();				
    }
#endif    					
	GetUDCInfo();							//��ȡĸ�ߵ�ѹ��������    
	GetCurrentInfo();						//��ȡ��������, �Լ��¶ȡ�ĸ�ߵ�ѹ�Ĳ���	
	 	 	
	if(EPwm1Regs.TBSTS.bit.CTRDIR == 0)
	{
		if(gIPMInitPos.Step != 0)               //��ʼλ�ýǱ�ʶ
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

