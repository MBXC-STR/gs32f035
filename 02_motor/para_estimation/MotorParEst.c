/****************************************************************
�ļ�����: �첽����ͬ����������ʶ���������ͺ�������
�ļ��汾�� 
���¸��£� 

*************************************************************/
#include "MotorParaIDinclude.h"
#include "MotorPmsmParEst.h"
#include "MotorEncoder.h"

// // ȫ�ֱ�������
UV_BIAS_COFF_STRUCT		 gUVCoff;
MOTOR_EXTERN_STRUCT		 gMotorExtReg;	            //�����չ��Ϣ�����������ʶ�õ������ݣ�
MOTOR_PARA_EST           gGetParVarable;


// // ͬ�������ر�ʶ˳��
PAR_EST_MAIN_STEP const pmTuneProgNoLoad[IDENTIFY_PROGRESS_LENGTH] = 
{
    IDENTIFY_RS,
    PM_EST_POLSE_POS,
    PM_EST_NO_LOAD,
    PM_EST_BEMF,
    IDENTIFY_END
};

// // ͬ�������ر�ʶ˳��
PAR_EST_MAIN_STEP const pmTuneProgLoad[IDENTIFY_PROGRESS_LENGTH] = 
{
    IDENTIFY_RS,
    PM_EST_POLSE_POS,
    PM_EST_WITH_LOAD,
    IDENTIFY_END,
    IDENTIFY_END
};

// // ͬ��������ʸ�����ر�ʶ˳��
PAR_EST_MAIN_STEP const pmSVC_TuneProgNoLoad[IDENTIFY_PROGRESS_LENGTH] = 
{
    IDENTIFY_RS,
    PM_EST_POLSE_POS,
    PM_EST_BEMF,
    IDENTIFY_END,
    IDENTIFY_END
};
// // ͬ��������ʸ�����ر�ʶ˳��
PAR_EST_MAIN_STEP const pmSVC_TuneProgLoad[IDENTIFY_PROGRESS_LENGTH] = 
{
    IDENTIFY_RS,
    PM_EST_POLSE_POS,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END
};


PAR_EST_MAIN_STEP const noTuneProgress[IDENTIFY_PROGRESS_LENGTH] =      
{
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END
};

PAR_EST_MAIN_STEP const debugTuneProcess[IDENTIFY_PROGRESS_LENGTH] = 
{
    PM_EST_BEMF,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END,
    IDENTIFY_END
};

// // �ڲ���������
void EndOfParIdentify(void);
extern void BeforeRunRsIdentify(void);
extern void ParSendTune(void);
/****************************************************************
    �������ܣ�������ʶ��ѭ���ṹ����
    
*****************************************************************/
void RunCaseGetPar(void)
{    
// ���ϻ��߹���ֹͣ��г��ֹͣ
	if((gError.ErrorCode.all != 0) || (gMainCmd.Command.bit.Start == 0))
	{		
		DisableDrive();
        DINT;                                       // ��ʱֹͣ�жϣ� ���ж���������
        
        gGetParVarable.ParEstContent[gGetParVarable.ParEstMstep] = IDENTIFY_END;
        gGetParVarable.StatusWord = TUNE_FINISH;
	}
    
// ������г
    switch(gGetParVarable.ParEstContent[gGetParVarable.ParEstMstep])
    {
       case IDENTIFY_RS:
	        BeforeRunRsIdentify();
  //         RsIdentify();
            break;
           
       case IDENTIFY_RR_LO:
            break;
            
       case IDENTIFY_LM_IO:
            break;
            
       case PM_EST_POLSE_POS:
			SynTuneInitPos();
            break;
            
	   case PM_EST_NO_LOAD:
			SynTunePGZero_No_Load();           
			break;

        case PM_EST_WITH_LOAD:
            SynTunePGZero_Load();
            break;

        case PM_EST_BEMF:
            SynTuneBemf();
            break;
            
       default: 
            EndOfParIdentify();
            break;            
    }
    
    ParSendTune();    
}

/************************************************************
	������ʶ��Ҫ�Ĳ����ĳ�ʼ��
	
************************************************************/
void PrepareParForTune(void)
{
    s16 m_index;
    PAR_EST_MAIN_STEP *m_PIdentifyFlow;
    
    //���в�����ʶ�����з��صı�������ҪԤ�ȸ�ֵ������ᵼ�¶�Ӧ�Ĺ��������    
	gMotorExtReg.R1     = gMotorExtInfo.R1;         // IM motor
    gMotorExtReg.R2     = gMotorExtInfo.R2;
    gMotorExtReg.L0     = gMotorExtInfo.L0;
    gMotorExtReg.LM     = gMotorExtInfo.LM;
    gMotorExtReg.I0     = gMotorExtInfo.I0;

    gMotorExtReg.RsPm   = gMotorExtInfo.RsPm;       // PM motor
    gMotorExtReg.LD     = gMotorExtInfo.LD;         
    gMotorExtReg.LQ     = gMotorExtInfo.LQ;
    gEstBemf.BemfVolt   = gMotorExtInfo.BemfVolt;               // PM ת�Ӵ��� %

    gPGData.PGDir               = gPGData.SpeedDir;
    gPGData.PGErrorFlag         = 0;
    gPmParEst.CoderPos_deg      = gIPMPos.RotorZeroGet;
    gPmParEst.UvwDir            = gUVWPG.UvwDir;
    gPmParEst.UvwZeroAng_deg       = gUVWPG.UvwZeroPos_deg;

    gPmParEst.IdKp = gVCPar.AcrImKp;
    gPmParEst.IdKi = gVCPar.AcrImKi;
    gPmParEst.IqKp = gVCPar.AcrItKp;
    gPmParEst.IqKi = gVCPar.AcrItKi;
    
    gGetParVarable.ParEstMstep = 0;
    gGetParVarable.StatusWord = TUNE_INITIAL;
    gGetParVarable.IdSubStep = 1;                               // �ӹ��̲���
    gUVCoff.IdRsCnt = 0;
    gUVCoff.IdRsDelay = 0;    

    gIPMZero.DetectCnt = 0;                 // must be initiated
    gGetParVarable.QtEstDelay = 0;
            
    switch(gGetParVarable.TuneType)
    {
        case TUNE_PM_COMP_LOAD:
			if(gMainCmd.Command.bit.ControlMode == IDC_SVC_CTL)
			{
				m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)pmSVC_TuneProgLoad;
			}
			else
			{
				m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)pmTuneProgLoad;
			}
			break;
            
        case TUNE_PM_COMP_NO_LOAD:
			if(gMainCmd.Command.bit.ControlMode == IDC_SVC_CTL)
			{
				m_PIdentifyFlow = (PAR_EST_MAIN_STEP *) pmSVC_TuneProgNoLoad;
			}
			else
			{
				m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)pmTuneProgNoLoad;
			}
            break;

        case TUNE_PM_PARA_temp:         //rt debug
            m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)debugTuneProcess;
			break;
            
		default:            
             m_PIdentifyFlow = (PAR_EST_MAIN_STEP *)noTuneProgress;
            break;
    }
    
    for(m_index=0; m_index < IDENTIFY_PROGRESS_LENGTH; m_index++)
    {
        gGetParVarable.ParEstContent[m_index] = *(m_PIdentifyFlow + m_index);
    }
    
    
}
void PrepareParForTuneBeforeRun(void)
{
   // PAR_EST_MAIN_STEP *m_PIdentifyFlow;
    
    //���в�����ʶ�����з��صı�������ҪԤ�ȸ�ֵ������ᵼ�¶�Ӧ�Ĺ�������� 
	gMotorExtReg.R1     = gMotorExtInfo.R1;         // IM motor
    gMotorExtReg.R2     = gMotorExtInfo.R2;
    gMotorExtReg.L0     = gMotorExtInfo.L0;
    gMotorExtReg.LM     = gMotorExtInfo.LM;
    gMotorExtReg.I0     = gMotorExtInfo.I0;   
    gEstBemf.BemfVolt   = gMotorExtInfo.BemfVolt;               // PM ת�Ӵ��� %
    gPGData.PGDir               = gPGData.SpeedDir;
    gPGData.PGErrorFlag         = 0;
    gPmParEst.CoderPos_deg      = gIPMPos.RotorZeroGet;
    gPmParEst.UvwDir            = gUVWPG.UvwDir;
    gPmParEst.UvwZeroAng_deg       = gUVWPG.UvwZeroPos_deg;
    
    gGetParVarable.StatusWord = TUNE_INITIAL;

            
}
/*******************************************************************
    ��������: ����������ʶ���ָ����ⲿģ����޸ģ�׼������?
    �ӳ��Ƴ��������˳�������ʶ�󷴸�����
********************************************************************/
void EndOfParIdentify(void)
{ 
    if(gGetParVarable.QtEstDelay == 0)
    {
       // gGetParVarable.ParEstMstep = 0;
        gMainStatus.PrgStatus.all = 0;
        if(TUNE_FINISH != gGetParVarable.StatusWord)
        {
            gGetParVarable.StatusWord = TUNE_SUCCESS;
        }
    	DisableDrive();
        
        EALLOW;  						                //�����û��������?
        #ifdef TARGET_GS32
        Interrupt_register(INT_ADCA1, &ADC_Over_isr);
        #else
        PIE_VECTTABLE_ADCINT = &ADC_Over_isr;		    //ADC�����ж�--INT1
        #endif
        #ifdef TARGET_GS32
        Interrupt_register(INT_EPWM1_TZ, &EPWM1_TZ_isr);		//�����ж�
#else
        PieVectTable.EPWM1_TZINT = &EPWM1_TZ_isr;		//�����ж�--INT2
        #endif
        //PieVectTable.EPWM1_INT 	= &EPWM1_zero_isr;		//�����ж�--INT3
        #ifdef TARGET_GS32
        Interrupt_disable(INT_EPWM2);
        #else
        PieCtrlRegs.PIEIER3.bit.INTx2 = 0;              //�ر�EPWM2�ж�
        #endif
        EDIS;
        
    	InitSetPWM();
       	InitSetAdc();
        SetInterruptEnable();	                        // �����ʶ��Ŀ��;�˳����ж��п����ǹرյģ����ڴ˴�?
       	EINT;   							    
       	ERTM;
    	//gMainStatus.RunStep = STATUS_STOP;
    	//gGetParVarable.IdSubStep = 1;
    }
    else if(gGetParVarable.QtEstDelay >= 5)
    {
        gMainStatus.RunStep = STATUS_STOP;
    	gGetParVarable.IdSubStep = 1;
		gGetParVarable.ParEstMstep = 0;
    }

    gGetParVarable.QtEstDelay ++;

	if((gEstBemf.BemfVolt > gMotorInfo.Votage*12)||(gEstBemf.BemfVolt < gMotorInfo.Votage*4))
	{
	    gError.ErrorCode.all |= ERROR_COEFF;        //380V�Ļ�����456��152V֮��ͱ�����?
	}
}


