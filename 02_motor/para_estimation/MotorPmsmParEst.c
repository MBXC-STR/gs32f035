/***************************************************************************
文件功能：同步机参数辨识，包括初始位置检测，编码器零点位置、方向辨识
文件版本：
最新更新：

***************************************************************************/
//文件功能：
//    1. 同步机磁极位置检测的中断程序和2ms程序， 包括修改ePWM模块实现各相发波；
//    2. 检测磁极位置时顺便辨识出同步机线电感，d、q轴电感；
//    3. 零点位置角的空载辨识，检测编码器方向；
//    4. 零点位置角的带载辨识，(*** 注意需要用户更改编码器方向试探)
//    5. 对编码器的支持包括ABZ，UVW, 旋转变压器；
//       UVW编码器包括uvw信号的零点为直角和uvw信号方向；
//    6. 同步机反电动势的辨识；

#include "MotorInclude.h"
#include "MotorIPMSVC.h"

/*******************结构体声明******************************************/
IPM_ZERO_POS_STRUCT		gIPMZero;         //永磁同步电机检测编码器零点位置角的数据结构
IPM_INITPOS_PULSE_STR	gIPMInitPos;      // pmsm 辨识磁极位置
PMSM_EST_BEMF           gEstBemf;         // PMSM辨识反电动势数据
PMSM_EST_PARAM_DATA		gPmParEst;        // 永磁同步机辨识时的和功能的数据交互区

/*******************相关函数声明******************************************/
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
	初始位置角检测结束后开始辨识编码器零点位置角的初始化程序
************************************************************/
void InitSetPosTune(void)
{
	ResetParForVC();
	ResetADCEndIsr();

    gPGDir.ABDirCnt = 0;                    // 复位所有编码器方向
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
	pm 磁极位置辨识, 需要与中断中程序配合
辨识方法: 脉冲电压法, dq轴电感能顺便得到；
辨识时，由自己发波，处理程序状态字
*************************************************************/
void SynTuneInitPos(void)
{	
	switch(gGetParVarable.IdSubStep)
	{
		case 1:                                             // 初始化变量
		    SetADCEndIsr(ADCEndIsrTune_POLSE_POS);
			gIPMInitPos.Waite = 0;
            //gIPMInitPos.InitPWMTs	= (50 * DSP_CLOCK);	    //50us
            //gIPMInitPos.InitPWMTs	= (50 * DSP_CLOCK);	    //50us		
			gGetParVarable.IdSubStep++;
			break;
            
		case 2:                                             // 延时等待20ms, 再初始化相关变量，关PWM发波
			gIPMInitPos.Waite ++;
			if(gIPMInitPos.Waite > 10)			
			{
				gIPMInitPos.Waite = 0;
       			gIPMInitPos.Step = 1;                       // 置静态辨识标志
       			//gMainStatus.PrgStatus.bit.PWMDisable = 1;   // 关EPWM 发波
       			gGetParVarable.IdSubStep++;
			}
			break;
            
		case 3:                                             // 等待中断中静态辨识完成...
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
    		    gGetParVarable.ParEstMstep++;               //切换到下一辨识步骤
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
上电初始位置角检测程序(在周期中断－－AD转换结束中断中执行)，
以U上桥和VW下桥打开为例说明：
	1）PWM1B、PWM2A、PWM3A强制关闭（高电平），
	2）初始强制设置PWM1A、PWM2B、PWM3B为高电平
	3）死区和死区补偿为0 (两次检测求取平均值)
	4) 设置gIPMInitPos.Step为1开始启动静止位置调谐, 完成后将其置0
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
		case 1:							                    //初始参数
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
        		gIPMInitPos.CurLimitBig = temp / gMotorInfo.Current;				// 大电流，用于表贴电机位置辨识和凸极电机极性辨识
                temp = (Ulong)gIPMInitPos.CurLimitSmall * gInvInfo.InvCurrent;                
        		gIPMInitPos.CurLimitSmall = temp / gMotorInfo.Current;				// 小电流，用于电感计算和输出缺相检测
        	}
/*
			temp0 = (Uint)((Ulong)gMotorInfo.Current * gIPMInitPos.CurLimitAdjSmall / 100);	// 小电流实际值
			temp1 = (Uint)((Ulong)gInvInfo.InvCurrent * 50 / 100);								// 变频器电流50%
			if(temp0 < temp1)									// 小电流不小于变频器额定电流的50%, 不大于电机额定电流
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
			gIPMInitPos.PWMTs = 0;									// 为 case2 中第一次发波做准备，保证第一次发波不延时

            gIPMInitPos.Step ++;
			break;

		case 2:							                            //检测合适的脉冲宽度
			EnableDrive();
			
			if(gIPMInitPos.Section > 5)								// 确定要发的电压矢量
			{
				Sec = gIPMInitPos.Section - 6;
				Cur_Lim = gIPMInitPos.CurLimitBig;
			}
			else
			{
				Sec = gIPMInitPos.Section;
				Cur_Lim = gIPMInitPos.CurLimitSmall;
			}

			if(Sec < 2) 											// 确定要采样的电流
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

					if(m_Wait > (gIPMInitPos.PWMTs / 600))			// 发脉冲前进行延时
					{
						m_Wait = 0;
						SynInitPosDetSetPwm(Sec);                   // 发脉冲
						gIPMInitPos.PeriodCnt ++;
					}

					break;
				case 1:
					if(abs(m_Cur) >= Cur_Lim)						// 脉冲电流达到设定值
					{
						gIPMInitPos.WidthFlag = 1;
					}

					if(gIPMInitPos.WidthFlag == 1)					// 检测到一个脉宽
					{
						gIPMInitPos.WidthFlag = 0;
						SynInitPosDetSetPwm(7);
						gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[gIPMInitPos.Section];		// 为了再次发波之前的延时
						gIPMInitPos.Section ++;												// 发12个波，分为12个区
						if(gIPMInitPos.Section < 12)
						{
							gIPMInitPos.PeriodCnt = 0;
						}
						else
						{
							gIPMInitPos.PeriodCnt = 2;
						}
					}
					else											// 继续延长脉冲时间
					{
						gIPMInitPos.PWMTsBak[gIPMInitPos.Section] += gIPMInitPos.InitPWMTs;
						if(gIPMInitPos.PWMTsBak[gIPMInitPos.Section] >= 60000)				// 防止脉宽计数溢出，进行限幅
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

					if(m_Wait > (gIPMInitPos.PWMTs / 600))							// 等待电流下降到0
					{
						m_Wait = 0;
					 	gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[0];

						for(i=1;i<6;i++)
						{
							if(gIPMInitPos.PWMTs > gIPMInitPos.PWMTsBak[i])			// 选定合适的小脉宽
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

		case 3:													// 发波，记录电流(小)
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
						gIPMInitPos.PWMTs4L = gIPMInitPos.PWMTs;	// 保留时间用于电感计算
						gIPMInitPos.PWMTs = 6000;					// 下一步计算量大，若电感很小，脉宽很窄则会出现算不完导致发波少一个，故加大延时
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

		case 4:							// 计算电感和凸极率，判断电机种类            
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
			
			gIPMInitPos.PWMTsBak[0] = gIPMInitPos.PWMTs;		// 为缺相检测保留脉宽
			
			SynInitPosDetSetTs();
			gIPMInitPos.PeriodCnt = 0;
			gIPMInitPos.Section = 0;
			gIPMInitPos.Step ++;
			break;

		case 5:										// 发波，记录电流(大)
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
            (gUVCoff.BeforeRunTuneEnable == 2))  //免调谐时也判断缺相
            {
				gIPMInitPos.PWMTs = gIPMInitPos.PWMTsBak[0];			// 为缺相检测恢复脉宽
				SynInitPosDetSetTs();
				SynInitPosDetSetPwm(7);
				gIPMInitPos.PhsChkStep = 0;
                gIPMInitPos.Step ++; // 进入同步机缺相检测
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
                    if(abs(gIPMInitPos.CurIU) < (gIPMInitPos.CurLimitBig>>3))    // U相缺相
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault4 = 11;
                        gIPMInitPos.Step ++;
                    }
                    if(abs(gIPMInitPos.CurIV) < (gIPMInitPos.CurLimitBig>>3))    // V相缺相
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
                    if(abs(gIPMInitPos.CurIW) < (gIPMInitPos.CurLimitBig>>3))    // W相缺相
                    {
                        DisableDrive();
                        gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault4 = 13;
                        gIPMInitPos.Step ++;
                    }
                    if(abs(gIPMInitPos.CurIV) < (gIPMInitPos.CurLimitBig>>3))    // V相缺相
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
			gIPMInitPos.Step = 0;           // 中断执行完成
			DisableDrive();
			break;
	}
}

/*************************************************************
	把转子磁极位置强制设置为Pos的函数
*************************************************************/
void SetIPMPos(Uint Pos)
{	
    DINT;
	gIPMPos.QepBak   = GetQepCnt();	
    gIPMPos.RotorPos = Pos;
    EINT;
}
/*************************************************************
	重置gPGData.RefPos
*************************************************************/
void SetIPMRefPos(Uint Pos)
{	
    gPGData.RefPos = ((u32)Pos<<8);
    gAsr.PosSet = gPGData.RefPos;
}

/************************************************************
	LAB、LBC、LCA轴电感计算函数
	gIPMInitPos.Cur保存：IU+、IU-、IV+、IV-、IW+、IW-、
	                     IU+、IU-V+、IV-、IW+、IW-
************************************************************/
void SynCalLabAndLbc(void)
{
	Uint  m_Index,m_Sel;
	Uint  m_DetaI,m_Cur,m_UDC;
	Ullong m_LTemp;

	//检测LAB、LBC、LCA
	for(m_Index = 0;m_Index < 3;m_Index++)
	{
		m_Sel = m_Index<<1;
		m_Cur = (gIPMInitPos.Cur[m_Sel] <= gIPMInitPos.Cur[m_Sel+1]) ?
						gIPMInitPos.Cur[m_Sel] : gIPMInitPos.Cur[m_Sel+1];
		m_DetaI = m_Cur;
		m_DetaI = ((Ulong)m_DetaI * (Ulong)gMotorInfo.Current) >> 12;	//(单位0.01A or 0.1A)
		m_UDC	= gUDC.uDCFilter - (Ulong)m_DetaI * (Ulong)gMotorExtReg.RsPm / 10000;      // 0.1V
		m_LTemp = (Ulong)gIPMInitPos.PWMTs4L * 2L*100L/DSP_CLOCK;				            //(单位10ns)
		m_LTemp = (Ulong)m_UDC * (Ulong)m_LTemp;
		//L=((电压/10)*(时间/10^5)/(电流/100))*100 (0.01mH)
		//L = 电压*时间/(电流*100)  (0.01mH)
		gIPMInitPos.LPhase[m_Index] = (m_LTemp)/((Ulong)m_DetaI * 100);
	}
}

/************************************************************
	初始位置计算函数，同时计算绕组电感。
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
    }*///等效于:三相电流偏差之和小于电机额定电流的1/40，认为无法识别磁极初始位置

	gIPMPos.InitPos = (Uint)atan(m_X, m_Y) - 5461;   // 30deg
	gIPMPos.InitAngle_deg = (Ulong)gIPMPos.InitPos * 3600 >> 16;
}

/************************************************************
	设置载波周期寄存器函数
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
	初始位置检测阶段，设置PWM的动作寄存器函数，PWM低电平有效
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

		case 6:									//恢复PWM模块的寄存器设置
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

	case 8:									// A-, B-, C-, 用于转速跟踪, 兼容三相下桥采样
			EPwm1Regs.AQCSFRC.all = 0x06;
			//pPWMForU->AQCSFRC.bit.CSFB = 2;
			EPwm2Regs.AQCSFRC.all = 0x06;
			//pPWMForV->AQCSFRC.bit.CSFB = 2;
			EPwm3Regs.AQCSFRC.all = 0x06;
			//pPWMForW->AQCSFRC.bit.CSFA = 2;
			break;

		default:								//同步机初始位置角检测初始化寄存器
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
* Function Name  : 空载情况下调谐编码器零点补偿角和编码器的接线方式
* Description    : 	，
在旋转过程中识别编码器的接线方式，编码器的零点位置角。
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
            gIPMZero.CurLimit = 3000UL * (Ulong)gMotorInfo.CurrentGet / gMotorInfo.Current;  //额定电流70%左右
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
            
            if(gIMTSet.M > ((long)gIPMZero.CurLimit <<12))      // M 轴电流上升过程处理, 200ms
            {
                gIMTSet.M = (long)gIPMZero.CurLimit << 12;
                gGetParVarable.IdSubStep++;                
            }
			break;
                        
	 	case 3:		
            if(gMainCmd.FreqSet >= 0)
            {
                gPhase.IMPhase += (gMotorExtInfo.Poles * 11);       // 约5RPM的速度转一圈
                gPGData.MoveDircetion = DIR_FORWARD;             //消除设定频率为零的影响
            }
            else
            {
                gPhase.IMPhase -= (gMotorExtInfo.Poles * 11);       // 约5RPM的速度转一圈
                gPGData.MoveDircetion = DIR_BACKWARD;
            }
            m_WaiteCnt++;
			
			m_Dir1 = JudgeABDir();											
            m_Dir2 = JudgeUVWDir();	

            if(m_WaiteCnt > 6000)                                   // 转一圈后判断接线
	        {
                m_WaiteCnt = 0;
                if(m_Dir1 == DIR_ERROR)    
                {
                    gError.ErrorCode.all |= ERROR_ENCODER;
                    gError.ErrorInfo[4].bit.Fault3 = 1;         // abz 信号方向出错
					m_WaiteCnt = 0;
                }      	        
                else if(m_Dir1 * gPGData.MoveDircetion < 0)              //AB方向取反
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
                        gError.ErrorInfo[4].bit.Fault3 = 2;         // abz 信号方向出错
                        m_WaiteCnt = 0;
                    }
                    else if(m_Dir2 * gPGData.MoveDircetion < 0)             //绝对位置方向取反
                    {
                        gPmParEst.UvwDir = !gPmParEst.UvwDir;
    			    }  
                }
                
			    gGetParVarable.IdSubStep++;	
                if((gPGData.PGType == PG_TYPE_RESOLVER)||(gIPMPos.ZSigEnable == 0))
                {
                    gGetParVarable.IdSubStep++;                               // 旋变不需要等待Z信号
                    gIPMPos.RotorPosBak = gIPMPos.RotorPos;             // 记录电机转子位置
					m_WaiteCnt = 0;
					m_WaiteCnt1 = 0;
                }
                gIPMPos.ZResetFlag = C_Z_RESET_POS_NOLIMIT;         // 表示辨识的时候信任Z信号
                m_ZNum = gIPMPos.ZSigNum;
            }
			break;
            
		case 4:
            if(gMainCmd.FreqSet >= 0)
            {
                gPhase.IMPhase += (gMotorExtInfo.Poles * 22);       // 约10RPM的速度转一圈
            }
            else
            {
                gPhase.IMPhase -= (gMotorExtInfo.Poles * 22);       // 约10RPM的速度转一圈
            }
	        m_WaiteCnt++;
	        if(m_ZNum != gIPMPos.ZSigNum)                           // 等待Z信号把电机转子位置角复位为编码器零点位置角
	        {
	            m_WaiteCnt = 0;
				m_WaiteCnt1 = 0;
                gIPMPos.RotorPosBak = gIPMPos.RotorPos;             // 记录电机转子位置
	            gGetParVarable.IdSubStep++;
	        }
	        else if(m_WaiteCnt > 9000)                       // 3圈还没收到Z信号，报故障
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
				 gIPMPos.RotorPosBak = gIPMPos.RotorPos;   // 为角度偏差初始角
		         break;
		    }
		    else
		    {
		        m_WaiteCnt1 = 5;                
		    }  
		     
            if(gMainCmd.FreqSet >= 0)
            {
                gPhase.IMPhase += (gMotorExtInfo.Poles * 6);       // 约5RPM的速度转一圈
				m_Data = (s16)(gIPMPos.RotorPos - gIPMPos.RotorPosBak);     // 累积编码器反馈的位置，最终和给定的位置进行比较，判断编码器是否未接或者线数不对
				m_PosDeta += m_Data;				
                gIPMPos.RotorPosBak = gIPMPos.RotorPos;
            }
            else
            {
                gPhase.IMPhase -= (gMotorExtInfo.Poles * 6);       // 约5RPM的速度转一圈
				m_Data = (s16)(gIPMPos.RotorPosBak - gIPMPos.RotorPos);
				m_PosDeta += m_Data;
                gIPMPos.RotorPosBak = gIPMPos.RotorPos;
            }
            m_DetaPos = gPhase.IMPhase - (s16)gIPMPos.RotorPos;		// 编码器零点位置角=磁极真实角度-编码器反馈角度
    		m_WaiteCnt++;

            if(m_WaiteCnt == 2000)
            {
            	gIPMZero.FirstPos = m_DetaPos;                  //包含了参数辨识之前，功能码设ǖ牧愕阄恢媒嵌?
                m_TotalPosErr = 0;
            }
            else if(m_WaiteCnt > 2000)
            {
            	m_TotalPosErr += (m_DetaPos - gIPMZero.FirstPos);
    	    	if(m_WaiteCnt > (2000 + 8192))
    	    	{
				    m_s32 = m_PosDeta - (s32)gMotorExtInfo.Poles * 61152L;      // 61152 = 10192*6
				    if((abs(m_s32) > 8192L)&&(gIPMPos.TuneDetaPosDisable == 0))            // 8192对应45电角度,由于是新增的功能，增加功能码屏蔽此故障
					{
						gError.ErrorCode.all |= ERROR_ENCODER;
	                    gError.ErrorInfo[4].bit.Fault3 = 7;         // 旋变信号故障,可能未接旋变卡,或者编码器线数设置错误
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
            
		case 6:  //延迟一段时间等待转子位置稳定到达指定角度，记录此时的角度⒏鳴epTotal 赋值
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
		    gGetParVarable.ParEstMstep++;       //切换到下一辨识步骤
		    break;
	}
	
}
/************************************************************
	带载编码器零点位置辨识
* 需要闭环矢量运行；
* 需要功能配合给出加减速过程频率给定
* 目标频率， 加减速时间由用户设置；
* 编码器方向需要用户设置，如不能成功，需要反向试探；
* uvw编码器时，uvw信号的方向不用设置，可以直接辨识得出；
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

    /*执行同步机闭环矢量速度环*/

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
				if(m_Cnt < 5)      // 如果是旋变先等10ms，等测速正常再进行带载辨识,wyk
				{
				    m_Cnt++;
					break;
				}
				else
				{
				    if(m_Cnt < 15)
					{
					    m_Cnt++;
					    m_RTPosAdd += gRotorTrans.RTPos;     // 累积求平均，精度更高，wyk
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
			m_CurLimit = Min(m_CurLimit , 7373UL * (Ulong)gMotorInfo.CurrentGet / gMotorInfo.Current);  //电机额定电流180%左右
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
            gGetParVarable.StatusWord = TUNE_ACC;    //开始加速
             
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
            else if(abs(gPGDir.ABDirCnt) > 36)           // 电机转动一圈后判断绝对位置信号是否正确
            {
                if(m_Dir2 == DIR_ERROR)
                {
                    gError.ErrorCode.all |= ERROR_ENCODER;
                    gError.ErrorInfo[4].bit.Fault3 = 4;         // abz 信号方向出错
                }
                if(m_Dir2 * gMainCmd.FreqSet < 0)                  
                {
                    gPmParEst.UvwDir = !gPmParEst.UvwDir;
			    }    
                m_DirFlag = 1;
            }		
            m_It = Filter64(abs(gIMTSet.T >> 12),m_It);  //加滤波,防止误报
            if((((m_It >= m_CurLimit)&&(abs(gRotorSpeed.SpeedApply) <= 10))||(abs(gRotorSpeed.SpeedApply) >= (1000L<<15/gBasePar.FullFreq01)))&&(gIPMPos.LoadTuneErrorEnable == 1))
			{
				gPGData.PGDir = !gPGData.PGDir;
				EQepRegs->QDECCTL.bit.SWAP = gPGData.PGDir;
				DisableDrive();                         //停止发波
				ResetAsrPar();
			    gGetParVarable.IdSubStep = 5;
			    gGetParVarable.StatusWord = TUNE_DEC;    //开始减速	
			    m_WaiteCnt = 0;
				m_WaiteCnt1 = 0;
				m_TuneNum++;
			    break;		
			}
            
            if(abs(gPGDir.ABDirCnt) > 180 * gMotorExtInfo.Poles)  // 机械上5圈
            {
                gError.ErrorCode.all |= ERROR_ENCODER;
                gError.ErrorInfo[4].bit.Fault3 = 5;         
            }
            if((m_DirFlag == 1) && (m_ZFlag == 1))
            {
        		m_WaiteCnt = 0;
				if(m_TuneNum == 1)                    // 改变方向调谐成功后，清除调谐次数，wyk
				{
				    m_TuneNum = 0;
				}
                gGetParVarable.StatusWord = TUNE_DEC;    //开始减速
			    gGetParVarable.IdSubStep++;
            }
	        break;

		case 3:                                 // 减速阶段
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
    			gGetParVarable.ParEstMstep++; //切换到下一辨识步骤
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
				if(m_WaiteCnt > 1000)   // 等待两秒钟在换方向调谐，辨识的位置更准，wyk
	            {
	                m_WaiteCnt = 0;
	                gGetParVarable.ParEstMstep--; //重新进行初始位置角辨识和带载辨识
					gGetParVarable.IdSubStep = 1; 					
	            }
			}
			else
			{
			    m_WaiteCnt1++;
				if(m_WaiteCnt1 > 5000)   // 速度不为0，大于10秒
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
	同步机反电动势辨识
* 需要电流闭环配合发M轴电流；
* 两点电流分为为: IdSet = 1500, 3000(Q12)；
* 理论上记录d轴电流，约等于线电流，q轴电流很小；
* 理论上记录q轴电压，约等?线)电压，d轴电压很小；
* 简化放出: Uq ≈ w(Ld * Id + Phi_r);

* 加减速的加减速过程频率指令 由辨识程序产生；
* 目标频率，加减速时间由功能产生，如下:
    目标频率:   40% 电机额定转速；
    加减速时间: 机型22以下: 30sec；机型22以上: 50sec；
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
	            //gGetParVarable.StatusWord = TUNE_ACC;       // 功能开始加速
	            gEstBemf.TuneFreqAim = (long)gMotorInfo.FreqPer * 2L / 5L;      // 40% 电机额定频率
	            gEstBemf.FreqRem = 0;                                           // 频率步长余数清零
	            gEstBemf.AccDecTick = (gInvInfo.InvTypeApply <= 20) ? (30L*1000L/2) : (60L*1000L/2);
	                                                                    // 机型22以下: 30sec 加速到额定频率
	                                                                    // 机型22以上: 60sec 加速到额定频率
	            //gEstBemf.TuneFreqSet = 0;
	            //EnableDrive();
	            gGetParVarable.IdSubStep ++;
			}
            break;
            
        case 2:                        // 处于加速过程
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

        case 3:                         // 积分电流和电压----1
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt <= 2048)
            {
                gEstBemf.TotalId1 += gLineCur.CurPer;           // 电机电流为基值
                gEstBemf.TotalVq1 += gOutVolt.VoltApply;        // 电机电压为基值
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

        case 4:                     // 修改电流值
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

        case 5:                         // 积分电流电压2
            gEstBemf.Cnt ++;
            if(gEstBemf.Cnt <= 2048)
            {
                gEstBemf.TotalId2 += gLineCur.CurPer;           // 电机电流为基值
                gEstBemf.TotalVq2 += gOutVolt.VoltApply;        // 电机电压为基值
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

        case 6:                         // 等待减速
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
            if(gEstBemf.Cnt < 500)          // 等待停机停稳
            {
                gEstBemf.Cnt ++;
            }
            else
            {        
                gEstBemf.Cnt = 0;
                gGetParVarable.IdSubStep ++;
            }
            break;

        case 8:                         // 计算反电动势系数，完成辨识           
            temp1 = (Ulong)gEstBemf.TotalVq2 * gEstBemf.TotalId1;
            temp2 = (Ulong)gEstBemf.TotalVq1 * gEstBemf.TotalId2;
            temp3 = (gEstBemf.TotalId2 - gEstBemf.TotalId1) * gEstBemf.TuneFreq;
            fluxRotor = (((Ullong)temp2 - (Ullong)temp1) <<15) /temp3; // Q: 12+12-15 + 15-12 = Q12

            temp3 = fluxRotor * abs(gMotorInfo.FreqPer) >> 12;        // Q15
            gEstBemf.BemfVolt = temp3 * gMotorInfo.Votage * 10L >> 15;              // 0.1 V

            gMainStatus.PrgStatus.all = 0;
            gGetParVarable.IdSubStep = 1;
			gGetParVarable.ParEstMstep ++; //切换到下一辨识步骤

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
函数功能：定子缱璞媸?
*****************************************************************/
/*void RsIdentify(void)
{
	Uint  m_UData1,m_UData2,m_UData3,m_MaxCur;
	Ulong m_ULong1,m_ULong2;
 
    SetADCEndIsr(ADCEndIsrTuneR1);

    m_MaxCur = ((Ulong)gInvInfo.InvCurrent * 3277)>>12;     //80%变频器疃ǖ缌?
	switch(gGetParVarable.IdSubStep)
	{
		case 1:     // 辨识定子电阻时发波的处理, EPWM模块的设置, 参数的初始化
#if (SOFTSERIES == MD380SOFT)
		    if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD_1140; /*1140V定子电阻辩士载频固定1K,2011.5.7 L1082*/
/*              }
            else if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
              {
               gUVCoff.Rs_PRD =SHORT_GND_PERIOD_690; /*690V定子电阻辩士载频固定1.5K，2011.6.20 L1082 */
/*              }
            else
#endif
              {
               gUVCoff.Rs_PRD = TUNE_Rs_PRD;         /*380V定子电阻辩士载频固定2K，2011.7.19  L1082 */     
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
            //EPwm1Regs.ETSEL.bit.INTEN = 0;  //定子电阻辨识时，禁止下溢中断
			EPwm1Regs.TBPRD = gUVCoff.Rs_PRD;
			//EPwm1Regs.CMPB  = EPwm1Regs.TBPRD - gADC.DelayApply;
			EPwm2Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm3Regs.TBPRD = gUVCoff.Rs_PRD;

			EPwm1Regs.CMPA.half.CMPA = gUVCoff.Comper;
			EPwm2Regs.CMPA.half.CMPA = 0;//gUVCoff.Comper;

			//EPwm3Regs.AQCSFRC.bit.CSFA   = 2;
			//EPwm3Regs.AQCSFRC.bit.CSFB   = 2;
			EPwm3Regs.AQCSFRC.all = 0x0A;		//关闭3桥臂
			EPwm3Regs.DBCTL.all   = 0x0000;
			EDIS;
            
			gGetParVarable.IdSubStep = 2;               
			break;

		case 2:         // 需多次辨识时延时之后再开启驱动
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
		 	    m_UData1 = (2048UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;//变频器电流笥?倍以上电机额定电流
			}

			if(0 == gUVCoff.IdRsCnt)            // 第一次辨识可以减小电流，防止过载
			{ 
			    m_UData1 = (512UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current;     
			}

			if(gGetParVarable.IdSubStep == 5)
            {                
                if( 16 < gInvInfo.InvTypeApply )
                {
                    m_UData1 = (3500UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;  //避免检测时报过载
                }
				else
				{
				    m_UData1 = (4096UL * (Ulong)gMotorInfo.CurrentGet)/gMotorInfo.Current ;
				}
            }
            if( gMotorInfo.Current > m_MaxCur )  //电机额定电流大于变频器额定电流时，以变频器额定电流为准
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
                    m_ULong1 =  m_ULong1 * 10;//电压变恳桓鲂∈悖缌?个小数点，所有要乘以10
					//m_ULong1 = 电压差(V)×10×2^14 
					m_UData2 = (gUVCoff.TotalI - gUVCoff.TotalIL)>>7;
					m_ULong2 = ((Ulong)m_UData2 * (Ulong)gMotorInfo.Current)>>10;
					//m_ULong2 = 电流差(A)×10×2^4
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
                        gUVCoff.IdRsDelay = 1000; //第二次辨识之前，延时2s
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
            InitSetPWM();                       //恢复修改的寄存器配置
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
	if(STATUS_RS_TUNE == gMainStatus.RunStep)  //参数辨识时延长定子电阻辨识时间
	{
	    if(gInvInfo.InvTypeApply > 18)   //37Kw以上增加定子电阻辨识时间
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
                gUVCoff.Rs_PRD =SHORT_GND_PERIOD_1140; /*1140V定子电阻辩士载频固定1K,2011.5.7 L1082*/
            }
            else if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
            {
                gUVCoff.Rs_PRD =SHORT_GND_PERIOD_690; /*690V定子电阻辩士载频固定1.5K，2011.6.20 L1082 */
            }
            else
#endif
            {
                gUVCoff.Rs_PRD = TUNE_Rs_PRD;         /*380V定子电阻辩士载频固定2K，2011.7.19  L1082 */     
            }  		

            if( gMotorInfo.Current > gInvInfo.InvCurrent )  //电机额定电流大于变频器额定电流时，以变频器额定电流为准
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
			EPwm1Regs.TBPRD = gUVCoff.Rs_PRD;  //下溢生效
			EPwm2Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm3Regs.TBPRD = gUVCoff.Rs_PRD;
			EPwm1Regs.AQSFRC.all  = 0xC0;
            EPwm2Regs.AQSFRC.all  = 0xC0;
            EPwm3Regs.AQSFRC.all  = 0xC0;
			EPwm1Regs.AQCSFRC.all 	= 0x00;		//下溢生效			
			EPwm1Regs.DBCTL.all 	= 0x0007;
			
			EPwm2Regs.CMPA.half.CMPA= 0;			//周期下溢生效
			EPwm2Regs.AQCSFRC.all 	= 0x0;	
			EPwm2Regs.DBCTL.all 	= 0x0007;      //立即生效
			
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
				if( gMotorInfo.Current > gInvInfo.InvCurrent )  //电机额定电流大于变频器额定电流, 以变频器额定电流为准
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
				if( gMotorInfo.Current > gInvInfo.InvCurrent )  //电机额定电流大于变频器额定电流时，以变频器额定电流为准
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
//				m_UData0 = (gUVCoff.TotalI << 4) / (gUVCoff.TotalIV >> 8);			// 没用

			    if(STATUS_GET_PAR == gMainStatus.RunStep)					// 求死区时间 m_UData7
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
				m_UData1	= gUVCoff.TotalComperL- m_UData7;				// Case3 总时间 - 死区时间
				m_UData2 	= (gUVCoff.TotalVoltL<<3)/gUVCoff.Rs_PRD;	// 时间划归
				m_ULong1 	= ((Ullong)m_UData1 * m_UData2)>>7;				// 除去死区的电压, 2^8	>>11
				
				//m_UData3	= gUVCoff.TotalComper;
				m_UData3	= gUVCoff.TotalComper - m_UData7;
				m_UData4 	= (gUVCoff.TotalVolt<<3)/gUVCoff.Rs_PRD;
				m_ULong2 	= ((Ullong)m_UData3 * m_UData4)>>7;				// >>11
				m_ULong3 	=  m_ULong2 - m_ULong1;//电压变量一个小数点，电流2个小数点，所有要乘以10
				m_ULong4    = m_ULong3 *10;			// m_ULong4 电压之差, 0.01V, 2^8
				//gUVCoff.m_ULong4 = gUVCoff.m_ULong2 *10;
				//m_ULong1 = 电压差(V)×10×2^14 
                if(STATUS_GET_PAR == gMainStatus.RunStep)						// m_UData5 求电流差值
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
				//m_ULong2 = 电流差(A)×10×2^4
				/*while((m_ULong2>>16) != 0)
				{
					m_ULong1 = m_ULong1>>1;
					m_ULong2 = m_ULong2>>1;
				}*/
				m_ULong5 = m_ULong4 / m_ULong6;											// 电压差 / 电流差		2^10
				m_UData2 = (m_ULong5 * (Ullong)1000)>>11;							// 2^0/2	// >>7

				gMotorExtReg.RsPm = m_UData2;											// 电阻实际值
				m_UData3 = ((Ulong)gMotorExtReg.RsPm * (Ulong)gMotorInfo.Current)/gMotorInfo.Votage;	
                gMotorExtPer.Rpm = ((Ulong)m_UData3 * 18597)>>14;		
				gUVCoff.Flag = 0;
				gUVCoff.RsTune = 2;
				DisableDrive();
				ResetADCEndIsr(); 
				InitSetPWM();                       //恢复修改的寄存器配	
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
		            gMainStatus.PrgStatus.all = 0;		//所有控制有效
				}
			}
		    break;
	}	
}

void BeforeRunRsIdentifyICal(void)
{
    s16 m_Data;
    if( gMotorInfo.Current > gInvInfo.InvCurrent )  //电机额定电流大于变频器额定电流时，以变频器额定电流为准
    {
        m_Data = ((long)gInvInfo.InvCurrent *500L) / gMotorInfo.Current;
    }
	else
	{
	    m_Data = 500; 
	}

	gRsIdentifyPID.Deta = (int)(gUVCoff.CurComperCoff - gUVCoff.Temper);

    if((gRsIdentifyPID.Deta > 0)&&(gUVCoff.Flag == 0))		// 给定大于反馈
	{
	    if(gInvInfo.InvTypeApply > 16)							// 增加开关时间
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
	else if(gRsIdentifyPID.Deta < - m_Data)						// 给定比反馈小500
	{
	    if(gUVCoff.Comper > (gDeadBand.DeadBand>>1) + 5)		// 奔浔人狼杏杏嗔
		{
	        if(gInvInfo.InvTypeApply > 16)						// 减小开关时间
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
		gUVCoff.Flag = 1;													// 说明开关时间正在减小
	}
	else if((gRsIdentifyPID.Deta > m_Data)&&(gUVCoff.Flag == 1))		// 给定比反馈大500, 且开关正在减小
	{
	    if(gInvInfo.InvTypeApply > 16)									// 增加开关时间
        {         
           gUVCoff.Comper += 1;         
        }
        else
        {
		   gUVCoff.Comper += 2;
        }
        gUVCoff.Number = 0;
	}
	
	if(gUVCoff.Number == 0)  // 说明开关在增大或者减小, 不进行电压_电流累积
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
	else						// 已进入稳态, 开始累计电压_电流_时间
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
    if(EPwm1Regs.TBSTS.bit.CTRDIR == 1)  // 上升沿
	{
        return;	
	}
#if (SOFTSERIES == MD380SOFT)
	if(GetOverUdcFlag())                    //过压处理
    {
       HardWareOverUDCDeal();				
    }
#endif    
    GetUDCInfo();							//获取母线电压采样数据    
	GetCurrentInfo();						//获取采样电流, 以及温度、母线电压的采样	
	ChangeCurrent();
 	BrakeResControl(); 

	gUVCoff.Temper  = abs(gIUVWQ24.U>>12);	
	gUVCoff.TemperV = abs(gIUVWQ24.V>>12);
	BeforeRunRsIdentifyICal();	
}
/*******************************************************************************
* Function Name  : 辨识定子电阻时候的ADC结束中断处理程序
* Description    : IfPWMPeriodInt()是周期点
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void ADCEndIsrTuneR1(void)
{
	if(EPwm1Regs.TBSTS.bit.CTRDIR == 1)  // 上升沿
	{
        return;	
	}
#if (SOFTSERIES == MD380SOFT)
	if(GetOverUdcFlag())                    //过压处理
    {
       HardWareOverUDCDeal();				
    }
#endif    
    GetUDCInfo();							//获取母线缪共裳?   
	GetCurrentInfo();						//获取采缌? 以及温度、母线电压的采样	
	ChangeCurrent();						//计算各种场合下的电流量
	BrakeResControl(); 
}
*/
/*******************************************************************************
* Function Name  : 辨识初始位置角的时候ADC结束中断硖序
* Description    : IfPWMPeriodInt()是周期点
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADCEndIsrTune_POLSE_POS(void)
{
    EALLOW;             //28035改为EALLOW保护
    ADC_CLEAR_INT_FLAG;
    EDIS;
    EINT;
#if (SOFTSERIES == MD380SOFT)
    if(GetOverUdcFlag())                    //过压处理
    {
       HardWareOverUDCDeal();				
    }
#endif    					
	GetUDCInfo();							//获取母线电压采样数据    
	GetCurrentInfo();						//获取采样电流, 以及温度、母线电压的采样	
	 	 	
	if(EPwm1Regs.TBSTS.bit.CTRDIR == 0)
	{
		if(gIPMInitPos.Step != 0)               //初始位置角辨识
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
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge this interrupt
    EDIS;
}

