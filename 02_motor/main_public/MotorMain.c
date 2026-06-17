/****************************************************************
文件功能：和电机控制相关的程序文件，电机控制模块的主体部分
文件版本：
更新日期：

****************************************************************/
#include "MotorInclude.h"
#include "MotorDataExchange.h"
#include "SystemDefine.h"
#include "MotorDefine.h"
#include "MotorIPMSVC.h"

// // 内部函数声明
void RunCaseDeal05Ms(void);
void RunCaseDeal2Ms(void);
void RunCaseRun05Ms(void);
void RunCaseRun2Ms(void);
void RunCaseStop(void);
void SendDataPrepare(void);	
void PWMPeriodIsr(void);
void PWMZeroIsr(void);
#if (SOFTSERIES == MD500SOFT)
void GetADCAdjustInfo(void);
#endif
extern s32 PmsmFwcAdjMethod(void);
extern void BeforeRunRsIdentify(void);
extern void CalTorqueLimitPar(void);
extern void CalTorqueUp(void);
extern void HVfOscDampDeal();
extern void IPMPosAdjustZIndex(void);
extern void VcAsrControl(void);
extern void VcAsrControl1(void);
extern void ResetParForPmsmSvc(void);
extern void PrepareParForTuneBeforeRun(void);
extern void PrepareAsrPar(void);
extern void PrepareAsrPar1(void);
extern s16 PmsmMaxTorqCtrl(void);
extern s16 PmsmFreqAdjMethod(void);
extern void PMFluxWeak05Ms(void);
extern void ParSend2Ms(void);
extern void ParGet05Ms(void);
extern void ParSend05Ms(void);
extern void OutPutVolt(void);
extern void ParSendTuneBeforeRun(void);
extern void ParGet2Ms(void);
extern void  OutPutPWM1(void);
extern void OutputPhaseLoseAndShortGndDetect(void);
//extern s32 PmsmFwcMixMethod(void);
Uint gAI1,gAI2,gAI3;
#if(AIRCOMPRESSOR == 1)
Uint gAI4,gAI5;
#endif
//VFFILTER1ST_STRUCT gVoltFilt;
extern BEFORE_RUN_PHASE_LOSE_STRUCT gBforeRunPhaseLose;
/************************************************************
    性能模块初始化程序：主循环前初始化性能部分的变量
(所有等于0的变量无需初始化)
************************************************************/
void InitForMotorApp(void)
{
	DisableDrive();
	TurnOffFan();
	ResetADCEndIsr();                                   //设置ADC结束中断的默认处理程序
	//ResetPGType();
    
// 公共变量初始化
	gMainStatus.RunStep = STATUS_LOW_POWER;	//主步骤
	gMainStatus.SubStep = 1;				//辅步骤
	gMainStatus.ParaCalTimes = 0;
	gError.LastErrorCode = gError.ErrorCode.all;
	//gMainStatus.StatusWord.all = 0;
	gCBCProtect.EnableFlag = 1;				//默认启动逐波限流功能
	gADC.ZeroCnt = 0;
	//gADC.DelaySet = 100;
	gADC.DelayApply = 600;
	gFcCal.FcBak = 50;
	gBasePar.FcSetApply = 50;
//	gVoltFilt.InputLast = 0;
//	gVoltFilt.OutLast = 0;
//	gVoltFilt.Total = 0;
//	gVoltFilt.FiltimeVsSamptime = 512;
#if (SOFTSERIES == MD500SOFT)
	gOverLoad.BreakTotal = 0;        //制动电阻过载累积量
	gBrake.FilterTicker = 0; 
	gExcursionInfo.IbExcursionOkFlag = 0;   //上电初始化制动电流零漂检测标志
#endif
    gBrake.DelayClose = 0; 
	        
//	gUVCoff.UDivV = 4096;
    gPWM.gPWMPrd = C_INIT_PRD;//(TBCLK/2/调制频率)=60M/2/5K=6000,   //TBCLK=60MHz,  5KHz  一个PWM周期时间长度为 1S/5K=200US
	gDeadBand.DeadBand = C_MAX_DB;	//初始化死区时间，3.2us  死区时间= DeadBand/TBCLK=(192/60M)S=3.2US
    gMainStatus.StatusWord.all = 0;
    
// 矢量相关变量初始化
    gRotorSpeed.TransRatio = 1000;                  // 测速传动比固定值
    gPGData.QEPIndex = QEP_SELECT_NONE; 
    gPGData.PGType = PG_TYPE_NULL;                  // 初始化为 null，
    gPWM.gZeroLengthPhase = ZERO_VECTOR_NONE; 
	gAsr.KPLowCoff = 10;                            // 初始化Kp系数
	gTemperature.OverTempPoint = 95;                // 初始化过温点
	gEstBemf.BemfVoltDisPlayTotal = 0;        

// 电机类型相关的初始化，默认是异步机电机
    gMotorInfo.MotorType = MOTOR_TYPE_IM;
    //gMotorInfo.LastMotorType = gMotorInfo.MotorType;
    gMotorInfo.LastMotorType = MOTOR_NONE;                // 保证进入主程序后能进行相关初始化
    
    //(*EQepRegs).QEINT.all = 0x0;  //取消QEP的I信号中断
    
    gPGData.PGMode = 0;
    gIPMInitPos.Flag = 0;
	gUVCoff.Flag = 0;
	gUVCoff.RsTune = 0;
    gPmParEst.SynTunePGLoad = 0;
    gIPMInitPos.InitPWMTs = (20 * DSP_CLOCK);	  //20us
    gPWM.PWMModle = MODLE_CPWM;
	gFanCtrl.RunCnt = 5000;
    gFanCtrl.ErrTimes = 0;
#if(AIRCOMPRESSOR == 1)
	gMainCmd.RestarCnt = 0;
#endif
	gMainStatus.StatusWord.bit.OverLoadPreventState = 0;
	FlyingStartInitDeal();

    ParSend2Ms();
    ParSend05Ms();
}

/************************************************************
主程序不等待循环：用于执行需要快速刷新的程序，要求程序非常简短
************************************************************/
/*void Main0msMotor(void)
{

}*/

/************************************************************
任务:
1. 编码器测速、SVC速度计算；
2. 速度闭环控制；
3. 向下计算gMainCmd.FreqSetApply， 向上传递gMainCmd.FreqToFunc；

4. SVC 不进行编码器测速， FVC和VF会计算编码器速度；

************************************************************/
void Main05msMotor(void)
{
	//gCpuTime.ADCInt = GetTime() - gCpuTime.ADCIntBase;
	//gCpuTime.ADCIntBase = GetTime();

#if (SOFTSERIES == MD500SOFT)
    GetADCAdjustInfo();
#endif

    if(gFluxWeak.Mode == 0)
	{
	    if(gMainCmd.FreqSet > 0)
		{
		    gMainCmd.FreqSetApply = gMainCmd.FreqSet + PmsmFreqAdjMethod();
		}
		else if(gMainCmd.FreqSet < 0)
		{
		    gMainCmd.FreqSetApply = gMainCmd.FreqSet - PmsmFreqAdjMethod();
		}
	}
	else
	{
	    gMainCmd.FreqSetApply = gMainCmd.FreqSet;
	}

    if(SYNC_SVC == gCtrMotorType)
    {   
		if(gRotorSpeed.SvcSpeed != 0)
		{
		    GetFeedRotorSpeed();               //编码器测速，确保间隔均匀
		}
		else
		{
		    gRotorSpeed.SpeedEncoder = 0;     // 避免编码器反馈值异常时，svc启动速度不为0
		}
		gAsr.KPLowCoff = 10;

		gRotorSpeed.SpeedApply = (s16)gPmsmRotorPosEst.SvcRotorSpeed;
      
        if(STATUS_STOP != gMainStatus.RunStep)
		{
            VcAsrControl();
		}
		gMainCmd.FreqSyn = gRotorSpeed.SpeedApply;
        gMainCmd.FreqToFunc = gRotorSpeed.SpeedApply;
        
        gPmsmRotorPosEst.CurFreqSetReal = (((s32)gMainCmd.FreqSyn * (s32)gBasePar.FullFreq01)>>15) * 100;  //记录进入低频运行之前的频率
    }  
    else if((SYNC_FVC == gCtrMotorType)||(gPmParEst.SynTunePGLoad == 1))
    {       
        GetFeedRotorSpeed();               //编码器测速，确保间隔均匀
        gAsr.FreqFeedFilter   = gRotorSpeed.SpeedFeedFilter; 					
		gRotorSpeed.SpeedApply = gRotorSpeed.SpeedEncoder;
        gMainCmd.FreqFeed = gRotorSpeed.SpeedEncoder;
		if(1 == gAsr.Mode)
		{
            VcAsrControl1();
		}
		else
		{
			VcAsrControl();
		}
		gMainCmd.FreqSyn = gRotorSpeed.SpeedApply;
        gMainCmd.FreqToFunc = gRotorSpeed.SpeedApply;	
    }
	else
	{
	    GetFeedRotorSpeed();               //编码器测速，确保间隔均匀
	}

	OutputLoseAdd();             // 输出缺相检测
#if (SOFTSERIES == MD500SOFT)
		InputPhaseLoseDetect();        //输入缺相检测
	#if (SOFT_INPUT_DETECT == STLEN)    
	    SoftInputPhaseLoseUdcCal();
	    SoftInputPhaseLoseDetect();
	#endif
#endif
}
/************************************************************
主程序的单2ms循环：用于执行电机控制程序
思路：数据输入->数据转换->控制算法->公共变量计算->自我保护->控制输出
执行时间：不被中断打断的情况下约120us
************************************************************/
void Main2msMotorA(void)
{

//从控制板获取参数
	ParGet2Ms();
    ParGet05Ms();
//ParameterChange2Ms();
    if(gMainCmd.Command.bit.Start == 0)
    {
        SystemParChg2Ms();
        SystemParChg05Ms();                 // 运行时不转换的参数

        ChangeMotorPar();       //电机参数转换?运行不转换
    }
    RunStateParChg2Ms();
    //RunStateParChg05Ms();

}

void Main2msMotorB(void)
{     
    gIPMZero.zFilterCnt++;  
	if(gIPMZero.zFilterCnt >= 4)      
	{
	    gIPMZero.zFilterCnt = 4;
	} 
    switch(gCtrMotorType)
    {
        case SYNC_SVC:	 
            gPhase.OutVoltPhaseCom = 0;
            break;
        case SYNC_VF:
        case ASYNC_VF:
		    gPhase.OutVoltPhaseCom = 0;
            break;

        case SYNC_FVC:
            IPMPosAdjustZIndex();		    
            break;           

        case DC_CONTROL:
            gMainCmd.FreqSyn = 0;
            RunCaseDcBrake();
            gOutVolt.VoltPhaseApply = 0;        // 考虑到同步机，输出电对准转子磁极                                      // 定子磁链就会诟方向上?
            gMainCmd.FreqToFunc = 0;		    
            break;

        case RUN_SYNC_TUNE:  //机参数辨?
		    IPMPosAdjustZIndex();
            // 参数辨识后，电流环参数需要调用辨识得到的pi参数, 得到pi参数前的辨识过程不会使用电流环
            if(gBasePar.FcSetApply > C_DOUBLE_ACR_MAX_FC)
			{
	            gImAcrQ24.KP = (long)gPmParEst.IdKp * gBasePar.FcSetApply * gPmParEst.IsKpK / 1000;         // 辨识时减小Kp,一次发波减小Kp和Ki
	            gItAcrQ24.KP = (long)gPmParEst.IqKp * gBasePar.FcSetApply * gPmParEst.IsKpK / 1000;
	            gImAcrQ24.KI = (s32)gPmParEst.IdKi * gPmParEst.IsKiK / 10;
	            gItAcrQ24.KI = (s32)gPmParEst.IqKi * gPmParEst.IsKiK / 10;
			}
			else
			{
				gImAcrQ24.KP = (long)gPmParEst.IdKp * gBasePar.FcSetApply * gPmParEst.IsKpK / 500;         // 参数辨识时减小Kp和Ki
	            gItAcrQ24.KP = (long)gPmParEst.IqKp * gBasePar.FcSetApply * gPmParEst.IsKpK / 500;
	            gImAcrQ24.KI = (s32)gPmParEst.IdKi * gPmParEst.IsKiK / 10;
	            gItAcrQ24.KI = (s32)gPmParEst.IqKi * gPmParEst.IsKiK / 10;
			}
			gImAcrQ24.KP = Min(gImAcrQ24.KP,8000);  //最大值限制为8000，wyk
            gItAcrQ24.KP = Min(gItAcrQ24.KP,8000);
            break;

        default:
            gMainCmd.FreqSyn = 0;
            gMainCmd.FreqToFunc = 0;
            break;
    }

    //计算载波频率
	CalCarrierWaveFreq();
	FlyingStartFcDeal();
	
	switch(gMainStatus.RunStep)
	{
		case STATUS_RUN:		                    //运行状态，区分VF/FVC/SVC运行
			RunCaseRun2Ms();
			break;

        case STATUS_STOP:
            RunCaseStop();
            break;

        case STATUS_IPM_INIT_POS:                   //同步机初始位置角检测阶段
			RunCaseIpmInitPos();            
            break;
            
		case STATUS_SPEED_CHECK:                    //转速跟踪状态
		    
			break;

		case STATUS_GET_PAR:	                    //参数辨识状态，移到0.5ms时要同时修改参数传递
			RunCaseGetPar();            
			break;

		case STATUS_LOW_POWER:	                    //上电缓冲状态/欠压状态
			RunCaseLowPower();
			break;
            
		case STATUS_SHORT_GND:	                    //上电对地短路判断状态
			RunCaseShortGnd();
			break;

		case STATUS_BEFORE_RUN_DETECT:
		    OutputPhaseLoseAndShortGndDetect();
		    break;

		case STATUS_RS_TUNE:
		    BeforeRunRsIdentify();
			break;

		case STATUS_FLYING_START:
			RunCaseFlyingStart();
            break;
			
		default:
            gMainStatus.RunStep = STATUS_STOP;      // 上电第一排会出现
			break;
	}	
}

void Main2msMotorC(void)
{
    InvCalcPower();     // 功率、转矩计算
//    VfOscIndexCalc();
    
//变频器身检测和保护
	InvDeviceControl();			
}

void Main2msMotorD(void)
{
//电流零漂检测，AD零漂和线性度检测
	GetCurExcursion();				    

//准备需要传送给控制板的数据
    SendDataPrepare(); 
    
//把实时数据传送给控制板
	ParSend2Ms();
    ParSend05Ms();
// End
}

/*************************************************************
	为功能模块准备需要的所有参数
*************************************************************/
void SendDataPrepare(void)		
{
    Uint tempU;
    s16   mAiCounter;
	long  m_Long;
    Ulong mTotal1;
    Ulong mTotal2;
    Ulong mTotal3;
#if(AIRCOMPRESSOR == 1)
    Ulong mTotal4;
	Ulong mTotal5;
#endif
    Uint   mRatio;
	///////////////////////////////////////////////停机时候显示电流为0处理
	if((gMainStatus.RunStep == STATUS_LOW_POWER) ||
	   (gMainStatus.RunStep == STATUS_STOP) ||
	   (gLineCur.CurBaseInv < (4096/50)&& 
	   (gMotorInfo.MotorType != MOTOR_TYPE_PM))  ||          //检测电流小于变频器额定电流2%，显示0
          //检测电流小于变频器额定电流2%，显示0
	   (1 == gMainStatus.StatusWord.bit.OutOff ))	
	{
		gLineCur.CurPerShow = 0;
        gLineCur.CurTorque  = 0;
        gLineCur.Temp       = 0;
	}
	else
	{
		gLineCur.CurPerShow = gLineCur.CurPerFilter >> 7;
		if(gCtrMotorType == ASYNC_VF)
		{
        	gLineCur.Temp = Filter32(gIMTQ12.T, gLineCur.Temp);				
		}
		else
		{
        	gLineCur.Temp = Filter32(gIMTSetQ12.T, gLineCur.Temp);
		}
        //if(((long)gRotorSpeed.SpeedApply * gIMTSetQ12.T) < 0)
		if(((long)gRotorSpeed.SpeedApply * gLineCur.Temp) < 0)       // 之前的做法在过零时会存在突变，wyk
        {
        	gLineCur.CurTorque = -(s16)abs(gLineCur.Temp);
        }
        else
        {
        	gLineCur.CurTorque = (s16)abs(gLineCur.Temp);
        }
	}
    
	//同步机角度转换
	tempU = (Uint)((s16)gRotorTrans.RTPos + gRotorTrans.PosComp);
    gRotorTrans.RtRealPos = ((Ulong)tempU * 3600L + 10) >> 16;
	if(gMotorInfo.MotorType == MOTOR_TYPE_PM)
    {   
	    if(SYNC_SVC == gCtrMotorType)
		{
		    gIPMPos.RealPos = ((Ulong)((Uint)gPhase.IMPhase) * 3600L + 10) >> 16;
		}
		else
		{
	        gIPMPos.RealPos = ((Ulong)gIPMPos.RotorPos * 3600L + 10) >> 16;
		}
    }
    
    // ai 采样处理
    DINT;
    mTotal1 = gAI.ai1Total;
    mTotal2 = gAI.ai2Total;
    mTotal3 = gAI.ai3Total;
#if(AIRCOMPRESSOR == 1)
    mTotal4 = gAI.ai4Total;
	mTotal5 = gAI.ai5Total;
#endif
    mAiCounter = gAI.aiCounter;
    
    gAI.ai1Total = 0;
    gAI.ai2Total = 0;
    gAI.ai3Total = 0;
#if(AIRCOMPRESSOR == 1)
    gAI.ai4Total = 0;
	gAI.ai5Total = 0;
#endif
    gAI.aiCounter = 0;
    EINT;
    
    gAI.gAI1 = mTotal1 / mAiCounter;
    gAI.gAI2 = mTotal2 / mAiCounter;
    gAI.gAI3 = mTotal3 / mAiCounter;
#if(AIRCOMPRESSOR == 1)
    gAI.gAI4 = mTotal4 / mAiCounter;
	gAI.gAI5 = mTotal5 / mAiCounter;
#endif
	gAI1 = gAI.gAI1;
    gAI2 = gAI.gAI2;
	gAI3 = gAI.gAI3;
#if(AIRCOMPRESSOR == 1)
    gAI4 = gAI.gAI4;
	gAI5 = gAI.gAI5;
#endif
    // 计算实际输出电压
    if(gMainCmd.Command.bit.ControlMode == IDC_SVC_CTL)		// SVC使用电压重构的电压计算
    {
		m_Long = (((long)gPmsmRotorPosEst.Ud * (long)gPmsmRotorPosEst.Ud) + 				// 电压重构出的值，不用进行限幅
          ((long)gPmsmRotorPosEst.Uq * (long)gPmsmRotorPosEst.Uq));
		gOutVolt.VoltApplySVC = (Uint)qsqrt(m_Long);
		mRatio = gOutVolt.VoltApplySVC;
    }
	else
	{
	    mRatio = gOutVolt.VoltApply;                              // FVC和VF使用计算的输出电压计算
	}
    mRatio = __IQsat(mRatio, 8192, 0);                              // 有过调制
	//gOutVolt.VoltDisplay = VFFilter1st(mRatio, &gVoltFilt);
	//gOutVolt.VoltDisplay = Min(gOutVolt.VoltDisplay,4915);       //输出电压最大限制在1.2倍
	gOutVolt.VoltDisplay = Filter(gOutVolt.VoltDisplay,mRatio,32);		// 1s滤波
    gOutVolt.VoltDisplay = Min(gOutVolt.VoltDisplay,4915);      //输出电压最大限制在1.2倍
}

/************************************************************
    区分驱动方式，主要完成为05ms速度环控制准备好参数；
    
************************************************************/
void RunCaseRun2Ms(void)
{
    long  Imtset_M;

    //EnableDrive();
    if((gMainCmd.Command.bit.Start == 0)||( gBrake.ErrCode != 0))            // 结束运行
    {
        gMainStatus.RunStep = STATUS_STOP;
		ResetADCEndIsr();
        RunCaseStop();
#if(AIRCOMPRESSOR == 1)
		if(gMainStatus.StatusWord.bit.OverLoadPreventState != 2)  //正常停机重启次数清零
		{
		    if(gMainCmd.RestarCnt <= 3)
			{
		        gMainCmd.RestarCnt = 0;
			}
		}
#endif
        return;
    }

    gMainStatus.StatusWord.bit.StartStop = 1;         
    
    switch(gCtrMotorType)
    {            
        case ASYNC_VF:  //异步机和同步机VF控制,暂辈磺?
        case SYNC_VF:  
            //gIPMInitPos.Flag = 0;               // VF运行后下次再运行矢量需要辨识初始位置
            VFWsTorqueBoostComm();				//转差补偿靥嵘脖淞考扑恪?
            VFWSCompControl();					//转差补偿处理(调整F)
            VFAutoTorqueBoost();        
            #if 1                           // 减速限制功能频率增加给定，这样导致错误
            if(speed_DEC &&(abs(gMainCmd.FreqSet) > abs(gVFPar.tpLst)))
            {
              gMainCmd.FreqSet = gVFPar.tpLst;
            }
            gVFPar.tpLst = gMainCmd.FreqSet;
            #endif
    		VfOverCurDeal();				        //过流抑制处理(调整F)
    		//VfOverUdcDeal();					    //过压抑制处理(调整F)
    		VfFreqDeal();                           // gVFPar.FreqApply

            gMainCmd.FreqToFunc = gVFPar.FreqApply;
            gMainCmd.FreqSetApply = gVFPar.FreqApply;

            VFSpeedControl();
            CalTorqueUp(); 
            
            HVfOscDampDeal();             // HVf 振荡抑制， 产生电压相位
            gOutVolt.VoltPhaseApply = gHVfOscDamp.VoltPhase;            
            gOutVolt.Volt = gHVfOscDamp.VoltAmp;
            
            VFOverMagneticControl();               
            break;
            
        case SYNC_SVC:	
            CalTorqueLimitPar();
            //CalUdcLimitIT();                   //矢量控制的过压抑制功能// 计算转矩上限和转矩控制
            PrepareAsrPar();
            PrepPmsmCsrPrar();
            if(gFluxWeak.Mode == 1)
        	{
        		Imtset_M = PmsmFwcAdjMethod();				
        	}
        	/*else if(gFluxWeak.Mode == 2)
        	{
        		PMFluxWeak05Ms();//Imtset_M = PmsmFwcAdjMethod();     //PmsmFwcMixMethod();
        	}*/
            else
            {
                Imtset_M = PmsmMaxTorqCtrl();

            }

			if(gFluxWeak.Mode == 2)
		    {
		        PMFluxWeak05Ms();
		    }

            PmsmSvcCalImForLowSpeed(); 
            Imtset_M += (long)gPmsmRotorPosEst.SvcIdSetForLowSpeed<<12;
            gIMTSet.M = Imtset_M;
				//get coff 
			FlyingStartParaAdjust();
            break;    
            
        case SYNC_FVC:        			   
            CalTorqueLimitPar();
            //CalUdcLimitIT();                   //矢量控制的过压抑制功能// 计算转矩上限和转矩控制
            if(1 == gAsr.Mode)           
            {
	            PrepareAsrPar1();
			}
			else
			{
				PrepareAsrPar();
			}
	        PrepPmsmCsrPrar();		
            if(gFluxWeak.Mode == 1)
        	{
        		gIMTSet.M = PmsmFwcAdjMethod();				
        	}
        /*	else if(gFluxWeak.Mode == 2)
        	{
        		gIMTSet.M = PmsmFwcAdjMethod();	//PmsmFwcMixMethod();
        	}*/
			else
			{
			    gIMTSet.M = PmsmMaxTorqCtrl();
			}

			if(gFluxWeak.Mode == 2)
		    {
		        PMFluxWeak05Ms();
		    }
            break;

        case DC_CONTROL:
            ;
            break;
                        
        default:            
            break;         
    }
}

/************************************************************
    启动机运行前的数据初始化处理，为电机运行准备初始参数

************************************************************/
void PrepareParForRun(void)
{
    // 设置控制模式和电机类型的组合，用于控逻辑分类。运行中不运行修改，尤其是不能与参数辨识状态冲突
    gCtrMotorType = (CONTROL_MOTOR_TYPE_ENUM)(gMainCmd.Command.bit.ControlMode + 10);
	gPmParEst.SynTunePGLoad =0;

// 公共变量初始化
    gMainStatus.StatusWord.bit.StartStop = 0;
    gMainStatus.StatusWord.bit.SpeedSearchOver = 0;

	gMainStatus.PrgStatus.all = 0;
	gMainStatus.PrgStatus.bit.ACRDisable = 1;    
    gGetParVarable.StatusWord = TUNE_INITIAL;
    gVarAvr.UDCFilter = gInvInfo.BaseUdc;
	gMainCmd.FreqSyn = 0;
	gMainCmd.FreqReal = 0;
	gOutVolt.Volt = 0;
	gOutVolt.VoltApply = 0;
	gRatio = 0;
	gCurSamp.U = 0;
	gCurSamp.V = 0;
	gCurSamp.W = 0;
	gCurSamp.UErr = 600L<<12;
	gCurSamp.VErr = 600L<<12;
    gIUVWQ24.U = 0;
    gIUVWQ24.V = 0;
    gIUVWQ24.W = 0;
	gIUVWQ12.U = 0;
	gIUVWQ12.V = 0;
	gIUVWQ12.W = 0;
	gLineCur.CurPerShow = 0;
    gLineCur.CurTorque  = 0;
	gLineCur.CurBaseInv = 0;
	gLineCur.CurPer = 0;
	gLineCur.CurPerFilter = 0;
	gLineCur.ImTotal = 0;
    gLineCur.ItTotal = 0;
    gLineCur.CurCnt = 0;
	gIMTQ12.M = 0;
	gIMTQ12.T = 0;
    gIMTQ24.M = 0;
    gIMTQ24.T = 0;
	gDCBrake.Time = 0;
    gPWM.gZeroLengthPhase = ZERO_VECTOR_NONE;
    gIAmpTheta.ThetaFilter = gIAmpTheta.Theta;
	gMainStatus.StatusWord.bit.RunStart = 0;
#if (SOFTSERIES == MD500SOFT)
	gBrake.IBreakTotal = 0;
	gBrake.OnCounter = 0;
	gBrake.TotalCounter	= 0;
#endif
// Vf 相关都初始化
	if(gMainCmd.Command.bit.ControlMode == IDC_VF_CTL)
	{
	    VfVarInitiate();
	}
    
    if(gMainCmd.Command.bit.ControlMode != IDC_VF_CTL)
    {
  	    ResetParForVC();  //VF调用该函数有问题，因为它改变了输出电压和频率
  	    ResetParForPmsmSvc();
//		gMainStatus.StatusWord.bit.OverLoadPreventState = 0;
    }
    if(gOverLoad.OverLoadPreventEnable == 1)    //10s后标志位清零，等待重启
	{
	    static Uint m_WaitTime = 0;
		if(gMainStatus.StatusWord.bit.OverLoadPreventState == 2)
		{
			m_WaitTime ++;
			if(m_WaitTime > 5000)    //10s
			{
			    gMainStatus.StatusWord.bit.OverLoadPreventState = 0;
			    m_WaitTime = 0;
#if(AIRCOMPRESSOR == 1)
				gMainCmd.RestarCnt ++;
#endif
			}
		}
		else
		{
		    m_WaitTime = 0;
		}
	}
}

/************************************************************
	切换到停机状态(公用子函数)
************************************************************/
void TurnToStopStatus(void)
{
	DisableDrive();
	gMainStatus.RunStep = STATUS_STOP;
	gMainStatus.SubStep = 1;
}

/*******************************************************************
    停机状态的处理
********************************************************************/
void RunCaseStop(void)
{
//停机封锁输出
    ResetADCEndIsr();
	DisableDrive();	    
	PrepareParForRun();
  
#if(SOFTSERIES == MD380SOFT)    
    if( gBrake.ErrCode == 0) 
#endif
	{
	    gMainCmd.FreqToFunc = 0; 
	}
    if(gMainCmd.Command.bit.Start == 0)
	{
	    gBforeRunPhaseLose.CheckOverFlag = 0;
		gBforeRunPhaseLose.CheckFirstFlag = 0;
	}
//等待零漂检测完成
#if (SOFTSERIES == MD500SOFT)
	if(gMainStatus.StatusWord.bit.RunEnable != 1)
#else
	if((gMainStatus.StatusWord.bit.RunEnable != 1)||( gBrake.ErrCode != 0))
#endif
	{
		return;
	}

//判断是否需要对地短路检测
	if((1 == gExtendCmd.bit.ShortGnd) && (gMainStatus.StatusWord.bit.ShortGndOver == 0))
	{
		gMainStatus.RunStep = STATUS_SHORT_GND;
		gMainStatus.SubStep = 1;        // 重新进行对地短路检测
		if(gBforeRunPhaseLose.CheckFirstFlag == 0)
		{
			EALLOW;
			EPwm1Regs.AQCSFRC.all 	= 0x0A;
			EPwm2Regs.AQCSFRC.all 	= 0x0A;			
			EPwm3Regs.AQCSFRC.all 	= 0x0A;
			EDIS;
			gBforeRunPhaseLose.CheckFirstFlag = 1;
		}
		return;
	}
	else
	{
		gMainStatus.StatusWord.bit.ShortGndOver = 1;
	}

//判断是否需要起动电机
	if(gMainCmd.Command.bit.Start == 1)	
	{
	    // 参数辨识
	    if(TUNE_NULL != gGetParVarable.TuneType)
	    {
		    gMainStatus.RunStep = STATUS_GET_PAR;
            PrepareParForTune();
		    return;			
	    }

        if(((gSubCommand.bit.OutputLostBeforeRun == 1)||
		(gExtendCmd.bit.ShortGndBeforeRun == 1))&&
		(gBforeRunPhaseLose.CheckOverFlag == 0))
		{
			gMainStatus.RunStep = STATUS_BEFORE_RUN_DETECT;
			gMainStatus.SubStep = 1;
			if(gBforeRunPhaseLose.CheckFirstFlag == 0)
			{
				EALLOW;
				EPwm1Regs.AQCSFRC.all 	= 0x0A;
				EPwm2Regs.AQCSFRC.all 	= 0x0A;			
				EPwm3Regs.AQCSFRC.all 	= 0x0A;
				EDIS;
				gBforeRunPhaseLose.CheckFirstFlag = 1;
			}
			return;	
		}
    	if(gError.ErrorCode.all != 0 )
    	{
    		return;
    	}
		if(((gExtendCmd.bit.SpeedSearch == 1) && (SYNC_SVC == gCtrMotorType)) && (gFlyingStart.Flag == 0))
		{     // 0－转速跟踪无效，1－转速跟踪模式1，2－转速跟踪模式2
		    gMainStatus.RunStep = STATUS_FLYING_START;   // 转速跟踪起动状态
		    gFlyingStart.Step = 0;
		    gMainStatus.SubStep = 1;
			gFlyingStart.PhaseLoseCnt = 0;
		    return;
		}

		if((gUVCoff.BeforeRunTuneEnable != 0)&&(gUVCoff.RsTune==0)&&(SYNC_SVC == gCtrMotorType))
		{
		    gMainStatus.RunStep = STATUS_RS_TUNE;   // 进入定子电阻辨识
			gGetParVarable.IdSubStep = 1;
			return;
		}
        //同步机识别磁极初始位置角阶段
#define FVC_INIT_TEST_CONDITION  ((0 == gRotorSpeed.SpeedEncoder) && (gPGData.PGType == PG_TYPE_ABZ))

    	if(((gIPMInitPos.Flag == 0) &&(1 != gIPMInitPos.InitTestFlag) &&
            (FVC_INIT_TEST_CONDITION || (SYNC_SVC == gCtrMotorType))&&(SYNC_VF != gCtrMotorType))||
			((gIPMInitPos.Flag == 2)&&(SYNC_SVC == gCtrMotorType)))
		{
			gMainStatus.RunStep = STATUS_IPM_INIT_POS;
			gMainStatus.SubStep = 1;
            gIPMInitPos.Step = 0;
            gIPMInitPos.OkFlag = 1;
            return;
		}

        
		if((gUVCoff.RsTune == 2)&&(SYNC_SVC == gCtrMotorType))   // 将辨识的定子电，DQ轴电感和电流环PI参数传给功能
		{		    
			PrepareParForTuneBeforeRun();
			ParSendTuneBeforeRun();
			gGetParVarable.StatusWord = TUNE_SUCCESS;
            gUVCoff.RsTune = 1;
		}

		if(SYNC_SVC == gCtrMotorType)
		{
		    gMainStatus.StatusWord.bit.RunStart = 1;   //传给功能的速度指令
		}

        if((SYNC_SVC == gCtrMotorType)
            &&(0 == gIPMInitPos.InitTestFlag))
        {
            gIPMInitPos.Flag = 0;  
        }

		if((SYNC_SVC == gCtrMotorType)&&(gUVCoff.BeforeRunTuneEnable == 2))
		{
            gUVCoff.RsTune   = 0;
		}

		FlyingStartInitDeal();
		
        // else ...STATUS_RUN
		gMainStatus.RunStep = STATUS_RUN;
		gMainStatus.PrgStatus.all = 0;            
		gMainStatus.SubStep = 1; 
		             
        gBrake.DelayClose = 0;    //LJH1917
		EnableDrive();

        RunCaseRun2Ms();        // 优化启动时间，在该拍就能发波
	}
}

/*************************************************************
	计算调制系数：从输出电压计算调制系数
*************************************************************/
void CalRatioFromVot(void)
{
	Uint	m_Ratio;
    
    // 计算调制系数
    if( 1 == gMainStatus.StatusWord.bit.OutOff )   //输出掉载
    {
	    gOutVolt.VoltApply = 287;                       
    } 
	
	m_Ratio = ((Ulong)gOutVolt.VoltApply * (Ulong)gMotorInfo.Votage)/gInvInfo.InvVolt;	
    //gOutVolt.VoltDisplay = (m_Ratio > 4096) ? 4096 : m_Ratio;
	m_Ratio = ((Ulong)m_Ratio<<2) * gInvInfo.BaseUdc / gUDC.uDC >> 2;  // 电压低时最多小1
	gRatio = (m_Ratio > 8192) ? 8192 : m_Ratio;
}

/*************************************************************
    周期中断：完成模拟量采样、电流计算、VC电流环控制等操作

注意:该函数在参数辨识中也有使用运男薷模枰觳槭欠裼跋炜赵乇媸?
*************************************************************/
void ADCEndIsr()
{  
    if((gCBCProtect.CBCIntFlag == 1)&&(gMainCmd.Command.bit.Start == 1)
        &&(0 == EPwm1Regs.TBSTS.bit.CTRDIR)&&(gError.ErrorCode.all == 0))        //周期中断
    {
	    EnableDrive();								//运行状态且逐波限流后才能再次开启
	    gCBCProtect.CBCIntFlag = 0;
        //clear the Flag of CBC and Int
        EALLOW;      
        EPwm2Regs.TZCLR.bit.CBC = 1;  //放在此处清0，确保CBC中断在清之前只进一次
        EPwm2Regs.TZCLR.bit.INT = 1; 
        EDIS;
    }
	if(gBasePar.FcSetApply > C_DOUBLE_ACR_MAX_FC)
	{
	    /*重新赋值与载频相关的系数*/
	    if(EPwm1Regs.TBSTS.bit.CTRDIR == 1)
		{
		    PWMZeroIsr();		 	
			OutPutVolt();
		}
		else
		{
		    if(gPmsmRotorPosEst.FcCoff == 200)
			{
			    PWMZeroIsr();
		        PWMPeriodIsr();
				return;
			}
		    PWMPeriodIsr();
#if (SOFTSERIES == MD500SOFT)
			BrakeIGBTProtect();
#endif
            BrakeResControl();                      //制动电阻控制
		}
		gPmsmRotorPosEst.FcCoff = 400;
		gMainCmd.StepCoeff      = 2;
		
	}
	else if((gMainCmd.StepCoeff == 2)&&(EPwm1Regs.TBSTS.bit.CTRDIR == 0)  //从单次发波变为两次发波的过度阶段
	     &&(gBasePar.FcSetApply <= C_DOUBLE_ACR_MAX_FC))
	{        
        PWMPeriodIsr();
		gPmsmRotorPosEst.FcCoff = 200;
		gMainCmd.StepCoeff      = 1;
		
	}
    else
    {
        gPmsmRotorPosEst.FcCoff = 200;
	    gMainCmd.StepCoeff      = 1; 
		PWMZeroIsr();
		PWMPeriodIsr();
		if(EPwm1Regs.TBSTS.bit.CTRDIR == 0)
		{
#if (SOFTSERIES == MD500SOFT)
			BrakeIGBTProtect();
#endif
            BrakeResControl();                      //制动电阻控制
		}			
    }  
           
}
/******************************************************************************
* Function Name  : 周期中断函数为PWMPeriodIsr
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void PWMPeriodIsr(void)
{
    switch(gCtrMotorType)
    {            
        case ASYNC_VF:                                      //异步机和同步机VF控制,暂时不区分?
        case SYNC_VF:
             
            break;
            
        case SYNC_SVC:
            
           	if(STATUS_RUN != gMainStatus.RunStep) // 停机后不能执行磁极位置辨识程序，会导致下次启动时的磁极位置改变
                break;

            PmsmSvcCtrl();                                  /*同步机开环矢量控制速度和位置估算*/

            gPhase.IMPhaseApply = gPmsmRotorPosEst.SvcRotorPos;           
            gPhase.IMPhase = gPmsmRotorPosEst.SvcRotorPos>>16;

            VCCsrControl();
#ifdef  SYNC_SVC_TEST_CODE
            gPmsmRotorPosEst.PosError = (gPmsmRotorPosEst.SvcRotorPos>>16) - (s16)gIPMPos.RotorPos;
            gPmsmRotorPosEst.SpeedError = gPmsmRotorPosEst.SvcRotorSpeed - gRotorSpeed.SpeedEncoder;
#endif
            
            break;
            
        case SYNC_FVC:
             PmDecoupleDeal();          
        	 VCCsrControl();
            break;
            
        case RUN_SYNC_TUNE:                          
            PmDecoupleDeal();
            VCCsrControl();
            break;
            
        default:            
            break;         
    }  	     
    gPhase.OutPhase = gPhase.IMPhase + gOutVolt.VoltPhaseApply + gPhase.OutVoltPhaseCom; //增加发波补偿，主要在深度弱磁时有意义，wyk
    CalRatioFromVot(); // 计算调制系数
    SoftPWMProcess();//随机PWM处理，获取载波周期和角度步长
    OutPutPWM1();   //增加过调制
	OutPutVolt();

	if(gPGData.PGType == PG_TYPE_RESOLVER)  // 旋变信号采样完成，等待下次中断读取
	{
		RT_SAMPLE_END;        
	    #ifdef TMS320F28035
	    ROTOR_TRANS_RD = 1;
	    ROTOR_TRANS_SCLK   = 1;
	    #endif
	} 
}

/******************************************************************************
* Function Name  : 下溢中断函数为PWMZeroIsr
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void PWMZeroIsr(void)
{		
#if (SOFTSERIES == MD380SOFT) 
    if(GetOverUdcFlag())                    //过压处理
	{
	   HardWareOverUDCDeal();				
	}
#endif
		
    GetUDCInfo();							//获取母线电压采样数据
	GetCurrentInfo();						//获取采样电流, 以及温度、母线电压的采样
	CalOutputPhase();                       //计算输出相位	，就是电机的转子电角度
	ChangeCurrent();						//计算各种场合下的电流量
}
/*******************************************************************************
* Function Name  : 复位ADC采样结束中断的执行程序为ADCEndIsr
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ResetADCEndIsr(void)
{
    gMainCmd.pADCIsr = ADCEndIsr;                    //设置ADC结束中断的默认处理程序
}

/*******************************************************************************
* Function Name  : 设置ADC结束中断执行程序
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SetADCEndIsr(void (*p_mADCIsr)())
{
    gMainCmd.pADCIsr = p_mADCIsr;                    //设置ADC结束中断的默认处理程序
}




