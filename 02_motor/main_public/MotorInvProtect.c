/************************************************************
�ļ�����:����ͱ�Ƶ����������
�ļ��汾�� 
���¸��£� 

*************************************************************/
#include "MotorInvProtectInclude.h"

// // ȫ�ֱ�������
OVER_LOAD_PROTECT		gOverLoad;
PHASE_LOSE_STRUCT		gPhaseLose;
BEFORE_RUN_PHASE_LOSE_STRUCT gBforeRunPhaseLose;
INPUT_LOSE_STRUCT		gInLose;
#if (SOFT_INPUT_DETECT == STLEN)
SOFT_INPUT_LOSE_STRUCT		gSoftInLose;
#endif
LOAD_LOSE_STRUCT		gLoadLose;
FAN_CTRL_STRUCT			gFanCtrl;
Ulong					gBuffResCnt;	//������豣������
CBC_PROTECT_STRUCT		gCBCProtect;
struct FAULT_CODE_INFOR_STRUCT_DEF  gError;
extern void FanSpeedCtrl(void);
//��Ƶ���͵���Ĺ��ر�
Uint const gInvOverLoadTable[10] =      /*������10��������9%������Сֵ:115%�������ֵ:196%*/
{
		36000,				//115%��Ƶ������   		1Сʱ����  
		18000,				//124%��Ƶ������	  	30���ӹ���
        6000,				//133%��Ƶ������	  	10���ӹ���
        1800,				//142%��Ƶ������	  	3���ӹ��� 
        600,				//151%��Ƶ������   		1���ӹ���  
        200,				//160%��Ƶ������   		20�����   
        120,				//169%��Ƶ������   		12�����    
        20,					//178%��Ƶ������   		6�����    ��Ϊ178% 2S����
        20,					//187%��Ƶ������   		2�����    
        5,					//196%��Ƶ������   		0.5�����   ����һ�����ֵ      2011-10-21-chzq
};
#if(SOFTSERIES == MD380SOFT)
Uint const gInvOverLoadTableForP[10] =       /*������10��������4%������Сֵ:105%�������ֵ:141%*/
{
		36000,				//105%��Ƶ������   		1Сʱ����  
		15000,				//109%��Ƶ������	  	25���ӹ���
        6000,				//113%��Ƶ������	  	10���ӹ���
        1800,				//117%��Ƶ������	  	3���ӹ��� 
        600,				//121%��Ƶ������   		1���ӹ���  
        300,				//125%��Ƶ������   		30�����   
        100,				//129%��Ƶ������   		10�����    
        30,					//133%��Ƶ������   		3�����    
        10,					//137%��Ƶ������   		1�����  
        5,					//141%��Ƶ������   		0.5�����     ����һ�����ֵ   2011-10-21-chzq
};
#else
Uint const gInvOverLoadTableForP[9] =       /*������10��������4%������Сֵ:105%�������ֵ:141%*/
{
		18000,				//110%��Ƶ������   30.0����
        6000,				//117%��Ƶ������   10.0����
        1800,				//124%��Ƶ������   3.0����
        600,				//131%��Ƶ������   1.0����
        400,				//138%��Ƶ������   40 ��
        250,				//145%��Ƶ������   25 ��
        90,					//152%��Ƶ������   9 ��
        50,					//159%��Ƶ������   5 ��
        20					//166%��Ƶ������   2 ��
};
#endif
//��Ƶ�������ۼ���������ϵ��
Ulong const gInvOverLoadDecTable[12] =
{
        (65536L*60/7),      //0%��Ƶ������    0.7������������
        (65536L*60/8),		//10%��Ƶ������   0.8������������
        (65536L*60/9),		//20%��Ƶ������   0.9������������
        (65536L*60/10),		//30%��Ƶ������   1.0������������
        (65536L*60/11),		//40%��Ƶ������   1.1������������
        (65536L*60/13),		//50%��Ƶ������   1.3������������
        (65536L*60/16),		//60%��Ƶ������   1.6������������
        (65536L*60/19),		//70%��Ƶ������   1.9������������
        (65536L*60/24),		//80%��Ƶ������   2.4������������
        (65536L*60/34),		//90%��Ƶ������   3.4������������
		(65536L*60/56),		//100%��Ƶ������  5.6������������
};
Uint const gInvBreakOverLoadTable[5] = 
{
    1,                      // 100% 10S������ 10s������   110%  2s����
    4,                      // 90% 10S������ 40s������    120%  0.5s����
    30,                     // 80% 10S������ 300s������
    200,                    // 70% 10S������ 2000s������
    2000,                   // 60% 10S������ 20000s������
};
/*#if (SOFTSERIES == MD500SOFT)
Uint const gInvBreakOverLoadTableFor185And22[10] = 
{
	1,//0,						//	120%  0.5s����
	2,//0.2,						//	110%  0.2s����
    100,//10,                 		// 	100%������ 10s������   
    500,//50,                 		// 	99%������ 50s������    
    820,//82,                	 	// 	98%������ 82s������
    1380,//138,                 	// 	97%������ 138s������
    2450,//245,                   	// 	96%������ 245s������
    5000,//500,					// 	95%������ 500s������
    10000,//1000					// 
    10000,//1000					// 
};
Uint const gInvBreakOverLoadTableFor30[10] = 
{
	13,// 1.3,					//	120%  1.3s����
	50,// 5,						//	110%  5s����
    100,// 10,                 	//	100%������ 10s������   
    160,// 16,                 	// 	98% ������ 16s������    
    230,//23,                 		// 	96% ������ 23s������
    300,//30,                 		// 	94% ������ 30s������
    440,//44,                   	// 	92% ������ 44s������
    580,//58,					// 	90% ������ 58s������
    2870,//287					// 	85% ������ 278s������
    5000,//500					//   
};
Uint const gInvBreakOverLoadTableFor37[10] = 
{
	25,// 2.5,					//	120%  2.5s����
	75,// 7.5,					//	110%  7.5s����
    100,// 10,                 	// 	100% ������ 10s������   
    180,// 18,                 	// 	98% ������ 18s������    
    250,// 25,                 	// 	96% ������ 25s������
    300,// 30,                 	// 	94% ������ 30s������
    360,// 36,                   	// 	92% ������ 36������
	480,// 48,					// 	90% ������ 48s������
	1000,//	100,					// 	90% ������ 48s������
	3400,//	340,					// 	90% ������ 48s������
};

Uint const gInvBreakOverLoadTableFor45And55[10] = 
{
	1,//0,						//	120% ������ 0s������  
	10,// 1,						//	110% ������ 1s������  
    100,// 10,                 	// 	100% ������ 10s������   
    290,// 29,                 	// 	98%  ������ 29s������    
    580,// 58,                 	// 	96%  ������ 58s������
    900,// 90,                 	// 	94%  ������ 90s������
    1500,// 150,            		// 	92%  ������ 150s������
    3000,//300					//
    5000,//500					//
    10000,//1000					//
};
Uint const gInvBreakOverLoadTableFor75[10] = 
{
	5,// 0.5,					//	120% ������ 0.5s����
	30,// 3,						//	110% ������ 3s����
    100,// 10,                 	// 	100%	������ 10s������   
    180,// 18,                 	// 	98% 	������ 18s������    
    270,// 27,                 	// 	96% 	������ 27s������
    390,// 39,                 	// 	94% 	������ 39s������
    530,// 53,                   	// 	92% 	������ 53s������
	1000,// 100,					// 	90% 	������ 100s������
	5000,//500					//
	10000,//1000					//
};
#endif
*/
#define C_MOTOR_OV_TAB_NUM      14
Uint const gMotorOverLoadBaseTable[C_MOTOR_OV_TAB_NUM] =
{
		480,			//115%�������  1Сʱ20���ӹ���
		240,			//125%�������  40���ӹ���
		90,				//135%�������  15���ӹ��� 
		36,				//145%�������  6���ӹ���
		24,				//155%�������  4���ӹ���
		15,				//165%�������  2.5���ӹ���
		12,				//175%�������  2���ӹ���

		 9,				//185%�������  1.5���ӹ���
		 6,				//195%�������  1���ӹ���
		 5,				//205%�������  50S����
		 4,				//215%�������  40S����
		 3,				//225%�������  30S����
		 2,				//235%�������  20S����
		 1				//245%�������  10S����
};
Uint gMotorOverLoadTable[C_MOTOR_OV_TAB_NUM] =
{
		48000,				//115%�������  1Сʱ20���ӹ���
		24000,				//125%�������  40���ӹ���
		9000,				//135%�������  15���ӹ��� 
		3600,				//145%�������  6���ӹ���
		2400,				//155%�������  4���ӹ���
		1500,				//165%�������  2.5���ӹ���
		1200,				//175%�������  2���ӹ���

		 900,				//185%�������  1.5���ӹ���
		 600,				//195%�������  1���ӹ���
		 500,				//205%�������  50S����
		 400,				//215%�������  40S����
		 300,				//225%�������  30S����
		 200,				//235%�������  20S����
		 100				//245%�������  10S����
};

#define C_MOTOR_OV_MAX_CUR      2450
#define C_MOTOR_OV_MIN_CUR      1150
#define C_MOTOR_OV_STEP_CUR     100

//��������ۼ���������ϵ��
Ulong const gMotorOverLoadDecTable[12] =
{
        (65536L*60/30),     //0%�������    3.0������������
        (65536L*60/40),		//10%�������   4.0������������
        (65536L*60/50),		//20%�������   5.0������������
        (65536L*60/60),		//30%�������   6.0������������
        (65536L*60/70),		//40%�������   7.0������������
        (65536L*60/80),		//50%�������   8.0����������?
        (65536L*60/90),		//60%�������   9.0������������
        (65536L*60/100),	//70%�������   10.0������������
        (65536L*60/110),	//80%�������   11.0������������
        (65536L*60/120),	//90%�������   12.0������������
		(65536L*60/130),	//100%�������  13.0������������
};

#if (SOFTSERIES == MD500SOFT)
// //�¶����߱�
Uint const gTempTableBSMXX[37] =
{
        2488,2441,2388,2328,2261,       //0-4
        2188,2108,2023,1933,1839,       //5-9
        1742,1644,1545,1446,1349,       //10-14
        1255,1164,1077,994, 916,        //15-19
        843, 775, 711, 653, 599,        //20-24
        541, 496, 454, 417, 382,        //25-29
        346, 318, 292, 268, 247,        //30-34
        228, 210                         //35-36

};
Uint const gTempTableWAIZHI[37] =
{
        3721,3632,3529,3411,3278,       //0-4
        3132,2973,2804,2626,2443,       //5-9
        2258,2073,1893,1719,1555,       //10-14
        1400,1258,1127,1008, 900,        //15-19
        804, 718, 641, 572, 511,        //20-24
        457, 409, 366, 328, 295,        //25-29
        265, 239, 215, 195, 176,        //30-34
        160, 146                         //35-36
};
//MD500��ͬ���͵Ĺ��µ㲻һ��
Uint const gTempProtectTable[19] =
{
        95,95,110,100,100,         //16-20
        90, 95, 100,95,95,             //21-25
        100, 90, 90,95,90,             //26-30
        93, 97, 103 ,103,             //31-34
};
int const gFanSpeedCtrlTempreture[9]=
{
        -50,         // 132kw 25
        70,          // 160kw 26
        -50,         // 200kw 27
        70,          // 220kw 28
        -50,         // 250kw 29
        70,          // 280kw 30
        -50,         // 315kw 31
        70,          // 355kw 32
        70           // 400kw 33
};

#else //MD380
// //�¶����߱�
Uint const gTempTableP44X[23] =
{
		624,614,603,590,576,561,		//6
		544,525,506,485,464,442,		//6
		419,395,373,350,327,305,		//6
		284,264,244,226,208				//5
};
Uint const gTempTableP8XX[23] =
{
		475,451,426,400,374,348,		//6
		323,299,275,253,232,212,		//6
		193,176,161,146,133,121,		//6
		110,100,91, 83, 76				//5
};
Uint const gTempTableBSMXX[23] =
{
		486,461,435,412,386,361,		//6
		337,313,291,269,248,228,		//6
		209,193,176,161,148,135,		//6
		123,113,103,94, 86				//5		
};
Uint const gTempTableSEMIKON[23] =
{
		558,519,480,451,418,392,		//6
		369,350,331,314,302,288,		//6
		278,269,262,254,247,243,		//6
		237,233,229,226,224				//5
};
Uint const gTempTableWAIZHI[23] =
{
		655,609,563,518,473,430,		//6
		389,350,314,282,251,224,		//6
		199,177,158,140,124,111,		//6
		99, 88, 78, 70, 62				//5		
};
#endif


// //�ļ��ڲ���������
void    SoftWareErrorDeal(void);	
void	TemperatureCheck(void);					//�¶ȼ��
void	OutputPhaseLoseDetect(void);			//���ȱ����
void 	OutputLoseReset(void);		
void	InputPhaseLoseDetect(void);				//����ȱ����

#if (SOFT_INPUT_DETECT == STLEN)
void    SoftInputPhaseLoseUdcCal(void);
void    SoftInputPhaseLoseDetect(void);
#endif
void	ControlFan(void);						//���ȿ���
void	OverLoadProtect(void);					//���ر���
void	CBCLimitCurProtect(void);
void 	SetCBCEnable(void);
void 	SetCBCDisable(void);
void 	BufferResProtect(void);
void BeforeRunOutputPhaseLoseDetect(void)	;
void ResetPhaseLoseDetect(void);	
Uint	OutputPhaseLoseComperCalculate(Uint motorRs,Uint PhaseLoseDetPRD);
void	BeforeRunShortGndDetect(void);
void OutputPhaseLoseAndShortGndDetect(void);
void ShortGnd_PhaseLoseICal(void);
void    BrakeResShortProtect(void);
void  BrakeOverloadProtect(void);
#ifdef TARGET_GS32
__interrupt void ShortGnd_ADC_Over_isr(void);
#else
interrupt void ShortGnd_ADC_Over_isr(void);
#endif
/************************************************************
	��Ƶ����������
************************************************************/
//��������������ɺ���
void MotorOverLoadTimeGenerate(void)
{
	u16 i;
	u32 mMotorOverLoadTime;

	for(i=0;i<14;i++)
	{
		mMotorOverLoadTime = gMotorOverLoadBaseTable[i]*(u32)gComPar.MotorOvLoad;

		if(mMotorOverLoadTime > 48000)//����80����
        {
			mMotorOverLoadTime = 48000;
		}
		else if(mMotorOverLoadTime < 100)//����10S
		{
        	mMotorOverLoadTime = 100;
		}

		gMotorOverLoadTable[i] = mMotorOverLoadTime;
	}
}
void InvDeviceControl(void)			
{
	SoftWareErrorDeal();				//��������
	TemperatureCheck();					//�¶ȼ��
	OutputPhaseLoseDetect();			//���ȱ����
#if (SOFTSERIES == MD380SOFT)
	InputPhaseLoseDetect();				//����ȱ����
#endif
	ControlFan();						//���ȿ���
#if (SOFTSERIES == MD500SOFT)
	CutDownCurForOverLoad();            //���ؽ����
	BrakeOverloadProtect();
#endif
	OverLoadProtect();					//���ر���

    CBCLimitCurPrepare();					//��������������
	CBCLimitCurProtect();				//����������µĹ����ж�

	BufferResProtect();
#if (SOFTSERIES == MD380SOFT)
    BrakeResShortProtect();             //�ƶ������·��������
#endif
    EncodeLineDiscProtect();
}

/*************************************************************
	�������ϴ���
ͣ��״̬Ҳ���ܱ��������ϡ���ѹ����
*************************************************************/
void SoftWareErrorDeal(void)					
{
// ��ʼ�ж���������
#if (SOFTSERIES == MD380SOFT)     
	if((gUDC.uDCFilter > gInvInfo.InvUpUDC) || //��ѹ�ж�,ʹ�ô��˲���ѹ
	    GetOverUdcFlag())
	{
	    DisableDrive(); //ͣ��Ҳ��������ѹ�������û������ѹ����
	    gError.ErrorCode.all |= ERROR_OVER_UDC;
        if((gUDC.uDCFilter > gInvInfo.InvUpUDC) && (!GetOverUdcFlag())
            &&(gMainStatus.ErrFlag.bit.OvCurFlag == 0))
        {
            gError.ErrorInfo[0].bit.Fault2 = 2;//������ѹ
        }
        else
        {
            gError.ErrorInfo[0].bit.Fault2 = 1; //Ӳ����ѹ
        }
	}
#else
	if((gUDC.uDCFilter > gInvInfo.InvUpUDC)||(ADC_UDC > 55125))    //55125��Ӧ860V
	{
	    DisableDrive(); //ͣ��Ҳ��������ѹ�������û������ѹ����
	    gError.ErrorCode.all |= ERROR_OVER_UDC;
        if(gUDC.uDCFilter > gInvInfo.InvUpUDC)
		{
            gError.ErrorInfo[0].bit.Fault2 = 1; //������ѹ
		}
		else
		{
		    gError.ErrorInfo[0].bit.Fault2 = 2;
		}
#if(AIRCOMPRESSOR == 1)
		if((gMainCmd.RestarCnt <= 3)&&(gOverLoad.OverLoadPreventEnable == 1))
		{
		    gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
		}
#endif
	}
#endif
	else if(gUDC.uDCBigFilter < gInvInfo.InvLowUDC) //Ƿѹ�ж�,ʹ�ô��˲���ѹ
	{
	    DisableDrive();
        if(STATUS_GET_PAR == gMainStatus.RunStep)   //���������ʶ������Ƿѹ���¼Ĵ��������޸ĺ�û�лָ�
        {
            EndOfParIdentify();
        }         
        gMainStatus.RunStep = STATUS_LOW_POWER;
        gMainStatus.SubStep = 0;
		DisConnectRelay();	
		gError.ErrorCode.all |= ERROR_LOW_UDC;
        gMainStatus.StatusWord.bit.LowUDC = 0;
#if(AIRCOMPRESSOR == 1)
		if((gMainCmd.RestarCnt <= 3)&&(gMainCmd.Command.bit.Start == 1)&&(gOverLoad.OverLoadPreventEnable == 1))   //���й����б�Ƿѹ���ϴ���־λ
		{
		    gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
		}
#endif
	}

	if(//(gMainStatus.RunStep == STATUS_LOW_POWER) ||                         // Ƿѹ
	   (gError.ErrorCode.all & ERROR_SHORT_EARTH) == ERROR_SHORT_EARTH)
	{
		gUDC.uDCBigFilter = gUDC.uDCFilter;
		return;										//Ƿѹ״̬�²��ж���������
	}
	if((STATUS_STOP == gMainStatus.RunStep) &&
        (gError.LastErrorCode != gError.ErrorCode.all) && 
        (gError.ErrorCode.all != 0))
	{
	    gError.LastErrorCode = gError.ErrorCode.all;
        if(0 != gError.ErrorCode.all)
        {
			gPhase.IMPhase += 0x4000L;				//���ϸ�λ�����нǶȷ����仯
        }
    }
   
// ��ʼ���ϸ�λ
	if(gSubCommand.bit.ErrorOK == 1) 			
	{
		gError.ErrorCode.all = 0;
        gError.ErrorInfo[0].all = 0;
        gError.ErrorInfo[1].all = 0;
        gError.ErrorInfo[2].all = 0;
        gError.ErrorInfo[3].all = 0;
        gError.ErrorInfo[4].all = 0;
        gBrake.ErrCode = 0;
        gLineCur.ErrorShow = 0;
        //CBC�жϸ�λ
        EALLOW;
        EPwm2Regs.TZCLR.bit.CBC = 1;
        EPwm2Regs.TZCLR.bit.INT = 1;
        EDIS;
		if(gMainStatus.ErrFlag.bit.OvCurFlag == 1)
		{
			gMainStatus.ErrFlag.bit.OvCurFlag = 0;
			//gPhase.IMPhase += 16384;
			EALLOW;
			EPwm2Regs.TZCLR.bit.OST = 1;
			EPwm3Regs.TZCLR.bit.OST = 1;
			EPwm1Regs.TZCLR.bit.OST = 1;
			EPwm1Regs.TZCLR.bit.INT = 1;
			EDIS;
		}
	}


}

//	MD500���¶ȼ�⺯��
#if (SOFTSERIES == MD500SOFT)
void TemperatureCheck(void)
{
	Uint    * m_pTable,*m_ErrTempTable;
	long    m_TempAD,m_IndexLow,m_IndexHigh,m_Index,m_ErrTemp;
	Uint    m_LimtCnt;
    int    mType,m_TempMax,m_TempMin;

    mType = gInvInfo.InvTypeApply;

    if((gInvInfo.InvTypeSet>100) && (gInvInfo.InvTypeSet<200))    //380V����Ϊ220V�����¶����ߴ���
    {
        mType = gInvInfo.InvTypeGaiZhi; 
    }
    m_ErrTempTable = (Uint *)gTempProtectTable;
    
// �����¶ȵ��ȷ�� 18.5KW-110KW��Ҫ���
    if((gInvInfo.InvTypeApply > 15)&&(gInvInfo.InvTypeApply <= 34))
    {
        m_ErrTemp = *(m_ErrTempTable + gInvInfo.InvTypeApply - 16);
    }
    else
    {
        m_ErrTemp = 95;
    }
	gTemperature.OverTempPoint = m_ErrTemp;
	
// �¶����ߵ�ѡ��
// MD500�Ĺ��ʷ�Χ�ڣ��Ž����¶ȼ��㣬������´���	
	if((mType <= 34) && (mType >= 16))    // [16, 25]
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
		else 
		{
			m_pTable = (Uint *)gTempTableWAIZHI;
		}
		
		// Ӳ���������Լ�������·������
	    m_TempAD = gTemperature.TempAD>>4;//�¶�AD��߲���Ϊ12λ
	
		// ��ʼ��ѯ�¶ȱ�
		m_IndexLow  = 0;
		m_IndexHigh = 36;
		m_Index = 18;//�¶ȱ�����ַ����м�ֵ
	    m_TempMax = 124;//����¶�
	    m_TempMin = -20;//����¶�
		if(m_TempAD >= m_pTable[m_IndexLow])
		{	
			mType = m_TempMin * 16;
		}
		else if(m_TempAD <= m_pTable[m_IndexHigh])
		{
			mType = m_TempMax * 16;//Q4
		}
		else
		{
			m_LimtCnt = 0;
			while(m_LimtCnt < 8)//8Ϊ���ַ���ѯ�����Ҫ����
			{
				m_LimtCnt++;					//������ѭ��
				if(m_TempAD == m_pTable[m_Index])
				{
					mType = (m_Index<<6) + m_TempMin * 16;
					break;
				}
				else if(m_IndexLow+1 == m_IndexHigh)
				{
					mType = (m_IndexLow<<6) + m_TempMin * 16 
								+ (((long)m_pTable[m_IndexLow] - m_TempAD)<<6)
					                /((long)m_pTable[m_IndexLow] - (long)m_pTable[m_IndexHigh]);
					break;
				}
				
				if(m_TempAD > m_pTable[m_Index])
				{
					m_IndexHigh = m_Index;
				}
				else
				{
					m_IndexLow = m_Index;
				}
				m_Index = (m_IndexLow + m_IndexHigh)>>1;
			}
		}
		if(abs(mType - gTemperature.TempBak) >= 8)			//�¶ȱ仯����0.5�ȲŸ�ֵ
		{
			gTemperature.TempBak = mType;
			gTemperature.Temp = mType>>4;
		}
	}
	else
	{
		gTemperature.Temp = 150;
		gTemperature.TempBak = 2400;
	}
	    //gTemperature.Temp = gTestDataReceive.TestData28;
// ��ʼ���¶��жϺͱ�������
	gTemperature.ErrCnt++;
	if(gTemperature.Temp < m_ErrTemp)
	{	
		gTemperature.ErrCnt = 0;
	}

	if(gTemperature.ErrCnt >= 5)
	{
		gTemperature.ErrCnt = 0;
		gError.ErrorCode.all |= ERROR_INV_TEMPERTURE;		//���ȱ���
	}
}
#else
/************************************************************
MD380�¶ȼ��,˵������:

1������С�ڵ���10       (2.2Kw ��?
	�¶�����:	     1		TABLE_P44X
				2		TABLE_P8XX
				3		TABLE_SEMIKON
				4		TABLE_BSMXX

2�������ڣ�11��18֮�� (11Kw �� 30Kw)
	�¶�����:	     1		TABLE_BSMXX
				2		TABLE_P44X
				3~4		TABLE_SEMIKON

3�������ڣ�19��26֮�䣩 (37Kw �� 200Kw,���Ұ���37, ������200)
	�¶Ȳ������ã�		TABLE_WAIZHI

4�����ʹ��ڵ���27		TABLE_BSMXX

5�����ʹ��ڵ���27ʱ���¶Ȳ�����·��ͬ����Ҫ����3.3V��3V��ת����

6�����ʹ��ڵ���19��85�ȱ�������������95�ȱ�����

�������з�ʽ��ÿ4����һ�����ݣ���ʼ��ַ����Ϊ12�ȣ�����ֵΪAD����ֵ
AD����ֵ��AD_RESULT>>6
************************************************************/
void TemperatureCheck(void)
{
	Uint    * m_pTable;
	Uint    m_TempAD,m_IndexLow,m_IndexHigh,m_Index,m_ErrTemp;
	Uint    m_LimtCnt;
    Uint    mType;

	//...׼���¶ȱ�,690V���¶Ȳ����ͱ�����380V���ʹ���27��һ��
	if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
    {
       mType = 28;
    }
    else  if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
    {
       
       mType = 20;  /*1140V ���µ���¶������趨Ϊ�����¶����� 2011.7.26 L1082*/
    }
    else
    {
        mType = gInvInfo.InvTypeApply;
    }

    if((gInvInfo.InvTypeSet>100) && (gInvInfo.InvTypeSet<200))    //380V����Ϊ220V�����¶����ߴ���
    {
        mType = gInvInfo.InvTypeGaiZhi; 
    }

// �����¶ȵ��ȷ��
      m_ErrTemp = 95;

// �¶����ߵ�ѡ��
	if(mType <= 10)
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP8XX;
		}
		else if(gInvInfo.TempType == 3)
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
		else    // gInvInfo.TempType == 4
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
	}
	else if(mType <= 18)    // [11, 18]
	{
		if(gInvInfo.TempType == 1)
		{
			m_pTable = (Uint *)gTempTableBSMXX;
		}
		else if(gInvInfo.TempType == 2)
		{
			m_pTable = (Uint *)gTempTableP44X;
		}
		else if(gInvInfo.TempType == 3)
		{
			m_pTable = (Uint *)gTempTableSEMIKON;
		}
		else 
		{
			m_pTable = (Uint *)gTempTableWAIZHI;
		}
	}
    else if(mType <= 26)    // [19, 26]
    {
        if(gInvInfo.TempType == 1)
        {
            m_pTable = (Uint *)gTempTableWAIZHI;
        }
        else    
        {
            m_pTable = (Uint *)gTempTableBSMXX;                
        }

        m_ErrTemp = 85;
    }
    else        // mType >= 27
    {
        m_pTable = (Uint *)gTempTableBSMXX;
    }
    gTemperature.OverTempPoint = m_ErrTemp;

// Ӳ���������Լ�������·������
    m_TempAD = (gTemperature.TempAD>>6);	

#ifdef  TMS320F2808
    if(mType >= 27) // 3V��3.3v ������·��ת��
    {
        m_TempAD = ((long)gTemperature.TempAD * 465)>>15; 
    }
#endif

// ��ʼ��ѯ�¶ȱ�
	m_IndexLow  = 0;
	m_IndexHigh = 22;
	m_Index = 11;
	if(m_TempAD >= m_pTable[m_IndexLow])
	{	
		mType = 12 * 16;
	}
	else if(m_TempAD <= m_pTable[m_IndexHigh])
	{
		mType = 100 * 16;
	}
	else
	{
		m_LimtCnt = 0;
		while(m_LimtCnt < 7)
		{
			m_LimtCnt++;					//������ѭ��
			if(m_TempAD == m_pTable[m_Index])
			{
				mType = (m_Index<<6) + (12 * 16);
				break;
			}
			else if(m_IndexLow+1 == m_IndexHigh)
			{
				mType = (m_IndexLow<<6) + (12 * 16) 
							+ ((m_pTable[m_IndexLow] - m_TempAD)<<6)
				                /(m_pTable[m_IndexLow] - m_pTable[m_IndexHigh]);
				break;
			}
			
			if(m_TempAD > m_pTable[m_Index])
			{
				m_IndexHigh = m_Index;
			}
			else
			{
				m_IndexLow = m_Index;
			}
			m_Index = (m_IndexLow + m_IndexHigh)>>1;
		}
	}
	if(mType - gTemperature.TempBak >= 8)			//�¶ȱ仯����0.5�ȲŸ�ֵ
	{
		gTemperature.TempBak = mType;
		gTemperature.Temp = mType>>4;
	}

// ��ʼ���¶��жϺͱ�������
	gTemperature.ErrCnt++;
	if(gTemperature.Temp < m_ErrTemp)
	{	
		gTemperature.ErrCnt = 0;
	}

	if(gTemperature.ErrCnt >= 5)
	{
		gTemperature.ErrCnt = 0;
		gError.ErrorCode.all |= ERROR_INV_TEMPERTURE;		//���ȱ���
	}

}
#endif
/************************************************************
	���ȱ����
************************************************************/
void OutputPhaseLoseDetect(void)			
{
	Uint m_U,m_V,m_W;
	Uint m_Max,m_Min;

	/*gPhaseLose.TotalU += abs(gIUVWQ24.U >> 12);
	gPhaseLose.TotalV += abs(gIUVWQ24.V >> 12);
	gPhaseLose.TotalW += abs(gIUVWQ24.W >> 12);

    //gPhaseLose.Time   += gPWM.gPWMPrdApply;
    //gPhaseLose.Time += 2000L * DSP_CLOCK;
	gPhaseLose.Time += gMainCmd.FreqReal;
	*/
    //gPhaseLose.Cnt ++;
    
	if((gSubCommand.bit.OutputLost == 0) ||	
	   (gMainCmd.FreqReal < 80) ||					//0.8Hz���²����
	   (gMainCmd.FreqReal > 50000)||                //500Hz����Ҳ�����
	   (gMainCmd.Command.bit.StartDC == 1) ||		//ֱ���ƶ�״̬�²����
	   (gMainCmd.Command.bit.StopDC  == 1) ||
	   ((gMainStatus.RunStep != STATUS_RUN) && 		//�������л����ٶ������׶�
	    (gMainStatus.RunStep != STATUS_SPEED_CHECK)))
	{
		OutputLoseReset();
		return;
	}    

	if((gPhaseLose.Time < 4000000)||(gPhaseLose.Cnt<400))    // 20������������������Ҫ200ms���ж����ȱ�� 
	{		
		return;
	}

	m_U = gPhaseLose.TotalU/gPhaseLose.Cnt;
	m_V = gPhaseLose.TotalV/gPhaseLose.Cnt;
	m_W = gPhaseLose.TotalW/gPhaseLose.Cnt;

	m_Max = (m_U > m_V) ? m_U : m_V;
	m_Min = (m_U < m_V) ? m_U : m_V;
	m_Max = (m_Max > m_W) ? m_Max : m_W;
	m_Min = (m_Min < m_W) ? m_Min : m_W;
	m_Min = Max(m_Min ,1);

	if((m_Max > 500) && (m_Max/m_Min > 10))
	{
		gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;
        gError.ErrorInfo[1].bit.Fault4 = 1;
#if(AIRCOMPRESSOR == 1)
		if((gMainCmd.RestarCnt <= 3)&&(gOverLoad.OverLoadPreventEnable == 1))
		{
		    gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
		}
#endif
	}
	//gPhaseLose.errMaxCur = m_Max;
    //gPhaseLose.errMinCur = m_Min;
	OutputLoseReset();
}
void OutputLoseAdd(void)		//���ȱ�����ۼӵ�������
{
	gPhaseLose.TotalU += abs(gIUVWQ24.U >> 12);
	gPhaseLose.TotalV += abs(gIUVWQ24.V >> 12);
	gPhaseLose.TotalW += abs(gIUVWQ24.W >> 12);

    gPhaseLose.Time += gMainCmd.FreqReal;
	gPhaseLose.Cnt ++;
}

void OutputLoseReset(void)		//���ȱ���⸴λ�Ĵ�������
{
	gPhaseLose.Cnt = 0;
	gPhaseLose.TotalU = 0;
	gPhaseLose.TotalV = 0;
	gPhaseLose.TotalW = 0;
	gPhaseLose.Time   = 0;
}

void OutputPhaseLoseAndShortGndDetect(void)			
{
	Uint Data1,Data2;
	if((gError.ErrorCode.all != 0) || 
	((gMainCmd.Command.bit.Start == 0)&&(gMainStatus.RunStep != STATUS_SHORT_GND)))
	{
		gMainStatus.SubStep = 1;
		gBforeRunPhaseLose.CheckOverFlag = 1;
		ResetPhaseLoseDetect();
		return;
	}
	
	switch(gMainStatus.SubStep)
	{
		case 1:
			gBforeRunPhaseLose.Counter	= 0;
			gBforeRunPhaseLose.OverFlag	= 0;
		//	gMainStatus.PrgStatus.bit.PWMDisable = 1; // ��ֹ�����жϷ���
			DisableDrive();
			// ����ǰ�Եض�·��⼰����ǰ���ȱ���������ֵ�ļ���
			// Ĭ��Ϊ2048(Q12��ʽ)��������С��80%��Ƶ���������С��2����������
			Data1 = ((Ulong)gInvInfo.InvCurrent*3277)/gMotorInfo.Current;			// 80%��Ƶ�������, Q12
			Data2 = ((Ulong)gInvInfo.InvCurrent<<12)/gMotorInfo.Current;			// ��Ƶ�������, Q12
			gBforeRunPhaseLose.CurComperCoff = (Data1 < 2048) ? Data1 : 2048;		// min(50%��ֵ����, 80% ��Ƶ�������)
            gBforeRunPhaseLose.CurComperCoffLimit = (Data2 < 3277) ? Data2 : 3277;	// min(80%��ֵ����, ��Ƶ�������)
			//gBforeRunPhaseLose.CurComperCoff = (gBforeRunPhaseLose.CurComperCoff<Data2)?gBforeRunPhaseLose.CurComperCoff:Data2;
			EALLOW;
			#ifdef TARGET_GS32
			interrupt_register(INT_ADC1, &ShortGnd_ADC_Over_isr);
			#else
			PieVectTable.ADCINT1	= &ShortGnd_ADC_Over_isr; // ����ǰ�Եض�·�����ȱ����AD�ж�  
			#endif
			EPwm1Regs.TBPRD = 600;	// ADжΪ10us
			EPwm2Regs.TBPRD = 600;
			EPwm3Regs.TBPRD = 600;
			EPwm1Regs.DBCTL.all	= 0x0;
			EPwm2Regs.DBCTL.all	= 0x0;
			EPwm3Regs.DBCTL.all	= 0x0;
			EPwm1Regs.AQCSFRC.all 	= 0x0A;
			EPwm2Regs.AQCSFRC.all 	= 0x0A;			
			EPwm3Regs.AQCSFRC.all 	= 0x0A;
			if((gBforeRunPhaseLose.OverFlag !=1)&&
			(gShortGnd.ocFlag == 0))
			{
				EnableDrive();
			}
			if(gExtendCmd.bit.ShortGndBeforeRun != 1)	// ����ǰ�����Եض�·, ֱ�Ӽ�����ȱ��, PWM�Ĵ�������
			{
				EPwm1Regs.AQCSFRC.all 	= 0x09;		// A+, B-
				EPwm2Regs.AQCSFRC.all 	= 0x06;			
				EPwm3Regs.AQCSFRC.all 	= 0x0A;

			}
			else // ����ǰ�Եض�·���, ��־λ����, PWM�Ĵ�������
			{
				gBforeRunPhaseLose.ShortGndDetectFlag = 1;
				EPwm1Regs.AQCSFRC.all 	= 0x09;		// A+
				EPwm2Regs.AQCSFRC.all 	= 0x0A;			
				EPwm3Regs.AQCSFRC.all 	= 0x0A;
			}
			EDIS;
			gMainStatus.SubStep++;
			break;
			
		case 2:
		case 3:
			if((gExtendCmd.bit.ShortGndBeforeRun == 1)&&
			(gMainStatus.SubStep == 2))
			{
				BeforeRunShortGndDetect();	// ����ǰ�Եض�·����ж�, δ��ɼ��ǰ, ѭ��ִ��
				gMainStatus.StatusWord.bit.ShortGndOver = 1;
			}
			if((gError.ErrorCode.all == 0)&&
			(gSubCommand.bit.OutputLostBeforeRun == 1))
			{
				if((gBforeRunPhaseLose.OverFlag !=1)&&
				(gShortGnd.ocFlag == 0)&&
				(gMainStatus.ErrFlag.bit.OvCurFlag == 0))
				{
					EnableDrive();
				}
				gBforeRunPhaseLose.Counter++;
				BeforeRunOutputPhaseLoseDetect(); //	����ǰ���ȱ�����ж�
			}
			else
			{
				gBforeRunPhaseLose.CheckOverFlag = 1;
				ResetPhaseLoseDetect();	
			}
			break;
		
		case 4:
		default:
			gBforeRunPhaseLose.CheckOverFlag = 1;
			ResetPhaseLoseDetect();	
			break;
	}
 
}
void ResetPhaseLoseDetect(void)	
{
	DisableDrive();
	gMainStatus.SubStep = 1;
	gBforeRunPhaseLose.maxIU = 0;
	gBforeRunPhaseLose.maxIV = 0;
	gBforeRunPhaseLose.maxIW = 0;
	gBforeRunPhaseLose.Counter = 0;
	gBforeRunPhaseLose.UWPhaseLoseFlag = 0;
	gBforeRunPhaseLose.ShortGndDetectFlag = 0;
	gShortGnd.ocFlag = 0;
//	gMainStatus.PrgStatus.bit.PWMDisable = 0;	// �ָ������жϷ���
	gMainStatus.RunStep = STATUS_STOP;
	EALLOW;
	#ifdef TARGET_GS32
    interrupt_register(INT_ADC1, &ADC_Over_isr);
    #else
	PieVectTable.ADCINT1     = &ADC_Over_isr;
	#endif
	EDIS;
	InitSetPWM();                       //�ָ��޸ĵļĴ�������
}

void BeforeRunShortGndDetect(void)
{
	DisableDrive();
//	gBforeRunPhaseLose.ShortGndDetectFlag = 0;
	if((gShortGnd.ocFlag != 0)||
	(gBforeRunPhaseLose.OverFlag == 1)||
//	(gBforeRunPhaseLose.maxIU > (30 * 32))||
	//(gUDC.uDC > gShortGnd.BaseUDC + 650)|| // ��ʱɾ��ĸ�ߵ�ѹ�ж�����
	(gCBCProtect.CBCIntFlag == 1))
	{
		EALLOW;
		EPwm1Regs.AQCSFRC.all 	= 0x0A;
		EPwm2Regs.AQCSFRC.all 	= 0x0A;			
		EPwm3Regs.AQCSFRC.all 	= 0x0A;
		EDIS;
		gError.ErrorCode.all |= ERROR_SHORT_EARTH;
		gBforeRunPhaseLose.CheckOverFlag = 1;
		if(abs(gShortGnd.ShortCur) > (30 * 32)||
		(gBforeRunPhaseLose.OverFlag == 1)) 
		{
			gError.ErrorInfo[2].bit.Fault4 =3; //�������� 
		}
		/*if(gUDC.uDC > gShortGnd.BaseUDC + 650)  ��ʱɾ��ĸ�ߵ�ѹ�ж�����
		{
			gError.ErrorInfo[2].bit.Fault4 = 4; //������ѹ  
		}*/
		if(1 == gShortGnd.ocFlag) 
		{
			gError.ErrorInfo[2].bit.Fault4 = 1; //Ӳ������,Ӳ�����ȼ���
		}
		if(2 == gShortGnd.ocFlag) 
		{
			gError.ErrorInfo[2].bit.Fault4 = 2; //Ӳ����ѹ
		}
		gLineCur.ErrorShow = gBforeRunPhaseLose.maxIU;
		gMainStatus.SubStep = 4;					// ȷ���Եض�·��, ֱ�ӽ�����Ⲣ���и�λ
	}
	else											// û�жԵض�·, �����ȱ�������üĴ���, ������һ�����
	{
		gMainStatus.SubStep++;
		gBforeRunPhaseLose.maxIU 	= 0;
		gBforeRunPhaseLose.maxIV	= 0;
		gBforeRunPhaseLose.maxIW	= 0;
		EALLOW;
		EPwm1Regs.AQCSFRC.all 	= 0x09;
		EPwm2Regs.AQCSFRC.all 	= 0x06;			
		EPwm3Regs.AQCSFRC.all 	= 0x0A;
		EDIS;
	}
	gBforeRunPhaseLose.ShortGndDetectFlag = 0;
}
void BeforeRunOutputPhaseLoseDetect(void)	
{
	if((gBforeRunPhaseLose.OverFlag == 1)&&	// 	��Ƶ������������������������ֵ����Ϊδȱ��
	(gBforeRunPhaseLose.maxIU > gBforeRunPhaseLose.CurComperCoff))
	{
		if(gBforeRunPhaseLose.UWPhaseLoseFlag == 0)
		{
			gBforeRunPhaseLose.maxIU = 0;
			gBforeRunPhaseLose.maxIV = 0;
			gBforeRunPhaseLose.maxIW = 0;
			gBforeRunPhaseLose.Counter = 0;
			gBforeRunPhaseLose.UWPhaseLoseFlag = 1;
			gBforeRunPhaseLose.OverFlag =0;
			if((gBforeRunPhaseLose.OverFlag !=1)&&
			(gShortGnd.ocFlag == 0)&&
			(gMainStatus.ErrFlag.bit.OvCurFlag == 0))
			{
				EnableDrive();
			}
			EALLOW;
			EPwm1Regs.AQCSFRC.all 	= 0x09;
			EPwm2Regs.AQCSFRC.all 	= 0x0A;			
			EPwm3Regs.AQCSFRC.all 	= 0x06;
			EDIS;
		}
		else
		{
			gMainStatus.SubStep++;
			gBforeRunPhaseLose.CheckOverFlag = 1;
			ResetPhaseLoseDetect();
		}
	}
	else
	{
		if(gBforeRunPhaseLose.Counter >= 6)
		{
			DisableDrive();
			gMainStatus.SubStep++;
			if(gMainStatus.ErrFlag.bit.OvCurFlag == 0)
			{
				gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;//�����ȱ��
				gError.ErrorInfo[1].bit.Fault4 = 3;
				gBforeRunPhaseLose.Counter = 0;
			}
		}
	}
}
#ifdef TARGET_GS32
__interrupt void ShortGnd_ADC_Over_isr(void)
#else
interrupt void ShortGnd_ADC_Over_isr(void)
#endif
{
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
	EALLOW;             //28035��ΪEALLOW����
	ADC_CLEAR_INT_FLAG;
	EDIS;
	EINT;

	ShortGnd_PhaseLoseICal();
	DINT;
	EALLOW;
	ADC_RESET_SEQUENCE;
#ifdef TARGET_GS32
    
#else
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;	// Acknowledge this interrupt
#endif
	EDIS;
#ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}

void ShortGnd_PhaseLoseICal(void)
{
//	gCpuTime.ADCIntBase = GetTime();
    s32 OverCur;
    				
	GetCurrentInfo();
	if(gBforeRunPhaseLose.ShortGndDetectFlag == 1)   // �Եض�·���
    {
        OverCur = (gIUVWQ24.U >> 12) + (gIUVWQ24.V >> 12);
        OverCur = OverCur + (gIUVWQ24.W >> 12);
        gBforeRunPhaseLose.CurTotal = abs((s16)OverCur);					// ���������֮��, Q12, pu
        gBforeRunPhaseLose.maxIU    = abs(gIUVWQ24.U >> 12);
    }
    else		// ���ȱ����
    {
		gBforeRunPhaseLose.maxIU	= Max(gBforeRunPhaseLose.maxIU,abs(gIUVWQ24.U >> 12));		// U��������
        gBforeRunPhaseLose.CurTotal = gBforeRunPhaseLose.maxIU;
	}
	if((gBforeRunPhaseLose.maxIU > gBforeRunPhaseLose.CurComperCoff)		// ���ڶԵض�·���, U���������50%, ���������֮�ʹ���50%, ��Ϊ�Եض�·
	 &&(gBforeRunPhaseLose.CurTotal > gBforeRunPhaseLose.CurComperCoff))	// �������ȱ����, U������������50%, 
	{
		DisableDrive();
		EALLOW;
		EPwm1Regs.AQCSFRC.all 	= 0x0A;
		EDIS;
		gBforeRunPhaseLose.OverFlag = 1;
	}
	else if((gBforeRunPhaseLose.maxIU > gBforeRunPhaseLose.CurComperCoffLimit)
	&&(gBforeRunPhaseLose.CurTotal < (gBforeRunPhaseLose.CurComperCoffLimit>>1))
	&&(gBforeRunPhaseLose.ShortGndDetectFlag == 1))             // �Եض�·���, ����⵽U���������80%���������С��40%ʱ���ٷ������
	{
	    DisableDrive();
		EALLOW;
		EPwm1Regs.AQCSFRC.all 	= 0x0A;
		EDIS;
	}
//	gCpuTime.ADCInt = gCpuTime.ADCIntBase - GetTime();
}
/************************************************************
	����ȱ����
	�źŵĸߵ͵�ƽ��ָ�������ϵ��źţ����ǵ�DSP���źţ���DSP���źŵ�ƽ�෴
ȱ����ԭ��1: MD380��
    �̵��������ź���ȱ���źŸ��ϣ� ������ʱ��һֱΪ�ͣ�
    ��PLһֱΪ�ߣ���̵���û�����ϣ�
    ��PLΪ��������ȱ�ࣻ    

ȱ����ԭ��2��MD500Ӳ���޸ĺ�zzb1812��
//��Ҫ����0.5ms��ִ��
// ����ȱ��720Hz ,1sӦ����720�����ڣ�ʵ����ƫ�500Hz
// ����450�����ھ���Ϊȱ�࣬450Hz
// ȱ1��ʱ�źŲ������������6ms���ҵĵ͵�ƽ
// ȱ3��ʱΪ������720Hz�ź�

// �������300Hz��һ��Ƚ�׼ȷ����300������
// �����ߵ�ƽΪ����������
18.5kw ���ϲ���ȱ���·
************************************************************/
#if (SOFTSERIES == MD500SOFT)
//#if (HARD_WARE_INPUT_DETECT == STLEN)
void InputPhaseLoseReset()
{
	gInLose.LowLevelCnt = 0;
	gInLose.PLFreqCnt    = 0;
	gInLose.HighLevelCnt = 0;
	gInLose.PLFreqTotal  = 0;
	gInLose.LongLowLevelCnt = 0;
}
void InputPhaseLoseDetect(void)			
{
	if((gMainCmd.Command.bit.Start == 0)              //ͣ��״̬\����״̬\18.5KW���²��������ȱ��
		 ||(gInvInfo.InvTypeApply < gInLose.ForeInvType)
		 ||(gError.ErrorCode.all != 0))
	{
		gInLose.HighLevelCnt = 0;
		gInLose.LongLowLevelCnt = 0;
		gInLose.LowLevelCnt  = 0;
		gInLose.PLFreq       = 0;
		gInLose.PLFreqCnt    = 0;
		gInLose.PLFreqTotal = 0;
		return;
	}

	if(PL_INPUT_HIGH)
	{
		Uint m_Data;
				
		if(gInLose.HighLevelCnt && gInLose.LowLevelCnt)
		{					
			m_Data = (gInLose.HighLevelCnt + gInLose.LowLevelCnt);//0.5ms��ִ��
			gInLose.PLFreqTotal += m_Data;	//�ۻ�
			if (gInLose.LowLevelCnt > 8)//4ms���ϵĵ͵�ƽ����
			{
				gInLose.LongLowLevelCnt ++;
			}
			gInLose.HighLevelCnt = 0;
			gInLose.LowLevelCnt  = 0;
			gInLose.PLFreqCnt++;			
		}
		else if((gInLose.HighLevelCnt > 250)&&(gSubCommand.bit.RelayEnalbe == 1))//����0.5s�Ӹߵ�ƽ������Ϊ����������Ϲ���
		{
			gError.ErrorCode.all |= ERROR_RESISTANCE_CONTACK;		
			InputPhaseLoseReset();
			return;
		}
		gInLose.HighLevelCnt++;
	}
	else if(gInLose.HighLevelCnt)
	{
		gInLose.LowLevelCnt++;
		if (gInLose.LowLevelCnt > 6000)//0.5msִ��3s�������ֹ�������ۻ�
		{
			InputPhaseLoseReset();
		}
	}
	
	if(gInLose.PLFreqTotal > 2000) //����1s
	{	
		if(gInLose.PLFreqCnt > 450 || gInLose.LongLowLevelCnt > 20) 					            
		{
#if (HARD_WARE_INPUT_DETECT == STLEN)
			if (gSubCommand.bit.InputLost >= 1)
			{
				gError.ErrorCode.all |= ERROR_INPUT_LACK_PHASE;	
    #if(AIRCOMPRESSOR == 1)
				if((gMainCmd.RestarCnt <= 3)&&(gOverLoad.OverLoadPreventEnable == 1))
				{
				    gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
				}	
    #endif
			}
#endif
		}
		// �������300Hz��һ��Ƚ�׼ȷ����150������
		else if(gInLose.PLFreqCnt > 200) 
		{
			if (gSubCommand.bit.RelayEnalbe == 1)
			{
				gError.ErrorCode.all |= ERROR_RESISTANCE_CONTACK;	
			}
		}	
		InputPhaseLoseReset();
	}
}
	#if (SOFT_INPUT_DETECT == STLEN)
/**********************************************
��������ȱ����
**********************************************/
void SoftInputPhaseLoseUdcCal(void)
{
    Uint    temp;
    Uint    m_Udc;
 
    gSoftInLose.MaxUdcFlag++;
    if(gSoftInLose.MaxUdcFlag > 30)
    {
        m_Udc           = (gSoftInLose.MaxUdc_5s + gSoftInLose.MinUdc_5s)>>1;
        gSoftInLose.uDCAverage =  Filter32(m_Udc,gSoftInLose.uDCAverage);
        gSoftInLose.MaxUdcFlag = 30;
    }
    gSoftInLose.A_uDC       = gUDC.uDC;
    gSoftInLose.uDCFilter   = gSoftInLose.uDCAverage;
    
    gSoftInLose.Count_5s++;
    if(gSoftInLose.Count_5s >= 10000)										// 5s��ʱ
    {
        if(abs(gSoftInLose.MaxUdc_5s - gSoftInLose.MinUdc_5s) >= 300)
        {
            gSoftInLose.Count_5s    = 0;
            temp = gSoftInLose.MaxUdc_5s;
            gSoftInLose.MaxUdc_5s   = gSoftInLose.MinUdc_5s;
            gSoftInLose.MinUdc_5s   = temp;
            gSoftInLose.MaxUdcFlag  = 0;
        }
        else
        {
            gSoftInLose.Count_5s = 10001;
        }
    } 

    gSoftInLose.Count_100ms++;

	if(gSoftInLose.Count_100ms >= 200)
	{
		gSoftInLose.MaxUdc_100ms    = gSoftInLose.MaxUdcTemper;
		gSoftInLose.MinUdc_100ms    = gSoftInLose.MinUdcTemper;
		gSoftInLose.MaxUdcTemper    = gSoftInLose.MinUdc_100ms;
		gSoftInLose.MinUdcTemper    = gSoftInLose.MaxUdc_100ms;
		gSoftInLose.Count_100ms     = 0;
	}
}
void SoftInputPhaseLoseDetect(void)
{														 
    u32  u_dvalve,deta_t;
    u32  deta_t1,deta_t2;
    static s16  m_DetaCount = 0;
    Uint    m_InLoseTime;
        
    if((gMainStatus.RunStep != STATUS_RUN) || 		//�������н׶β����
       (gSubCommand.bit.InputLost != 1)||
       (gSoftInLose.MaxUdc_100ms - gSoftInLose.MinUdc_100ms <= gSoftInLose.MaxDetaU)||
       (gPowerTrq.InvPower_si <= ((gSoftInLose.HalfInvPowerPU*200)>>10)))     //||         //��Ƶ������ʵ�50%=2048*1.732
	{
		gSoftInLose.Flag = 0;
        gSoftInLose.time1 = 0; 
        gSoftInLose.time2 = 0;
        gSoftInLose.time3 = 0;
        gSoftInLose.time4 = 0;
	
	}

    else
    {
        gSoftInLose.uDC = gSoftInLose.A_uDC; 
        u_dvalve = gSoftInLose.uDCFilter;
        if(	gSoftInLose.uDC <= u_dvalve )
        {
            if(gSoftInLose.Flag == 0)
            {	//	��һ�ε��ڱȽϵ�ѹ
               gSoftInLose.time1 = GetTime();
               gSoftInLose.Flag = 1;
            }
            else if(gSoftInLose.Flag ==2) 
            {	//	�ڶ��ε��ڱȽϵ�ѹ
               gSoftInLose.time3 = GetTime();
               gSoftInLose.Flag = 3;
            }       
        }
        else if(gSoftInLose.uDC  > u_dvalve)
        {
            if(gSoftInLose.Flag ==1)
            {	//	��һ�ε͵�ѹ�ָ�
               gSoftInLose.Flag = 2;  
               gSoftInLose.time2 = GetTime();            
            }	//	�ڶ��ε͵�ѹ�ָ�
            else if(gSoftInLose.Flag ==3)
            {
                gSoftInLose.time4 = GetTime();   
                gSoftInLose.Flag = 0;  
    			//	����г����Ƶ��
                deta_t1 = ((gSoftInLose.time1 - gSoftInLose.time3)&0xFFFFFFFF)>>1;
                deta_t2 = ((gSoftInLose.time2 - gSoftInLose.time4)&0xFFFFFFFF)>>1;
                deta_t = ((deta_t1 + deta_t2)/DSP_CLOCK);
                deta_t = Max(deta_t ,3000);
                deta_t = Min(deta_t ,15000);
                
                gSoftInLose.wait_r += deta_t;
    			//	��������ʮ����г��Ƶ�ʵ�ƽ��ֵ
                m_DetaCount ++;
                if(m_DetaCount < 10)
                {
                    return;
                }
                deta_t = gSoftInLose.wait_r / 10;              
                m_DetaCount = 0;
                gSoftInLose.wait_r = 0;

              
                
                if((DOWNSTATE <= deta_t)&&(deta_t <= UPSTATE))
                {
                    if(gPowerTrq.InvPower_si <= (gSoftInLose.HalfInvPowerPU>>1))
                    {
                        gSoftInLose.InLoseAddTime = 1;     //  5 MIN 
                    }
                    else if(gPowerTrq.InvPower_si <= gSoftInLose.HalfInvPowerPU)
                    {
                        gSoftInLose.InLoseAddTime = 150;   //  2S
                    }
                    else
                    {
                        gSoftInLose.InLoseAddTime = 750;   //  400 MS
                    }
                    gSoftInLose.Flose = gSoftInLose.Flose+gSoftInLose.InLoseAddTime;
                	//	����10�ζ��ڷ�Χ�ڣ���ȱ��
                    if(gSoftInLose.Flose >= 1500)
                    {
                        gError.ErrorCode.all |= ERROR_INPUT_LACK_PHASE;
                        gError.ErrorInfo[1].bit.Fault3 = 2;                                       
                       
                        gSoftInLose.time1 = 0;
                        gSoftInLose.time2 = 0;
                        gSoftInLose.time3 = 0;
                        gSoftInLose.time4 = 0; 
                    }
                }
                else
                {    
                    gSoftInLose.Flose = 0;
                    gSoftInLose.time1 = 0;
                    gSoftInLose.time2 = 0;
                    gSoftInLose.time3 = 0;
                    gSoftInLose.time4 = 0; 
                }
            }
        } 
    }
    
    if(gSoftInLose.FloseLast == gSoftInLose.Flose)
    {
    // ���������жϱ�־��ȣ�����  20S ����
        if(gSoftInLose.FloseSameTime ++ > 800)           
        {
            if(gSoftInLose.Flose >= 30)
            {
                gSoftInLose.Flose = gSoftInLose.Flose - 30;
            }
			else
			{
				gSoftInLose.Flose = 0;
			}
        }
    }
    else
    {
        gSoftInLose.FloseLast = gSoftInLose.Flose;
        gSoftInLose.FloseSameTime =0;
    }

    if(gMainStatus.StatusWord.bit.LowUDC == 0)
    {
        gSoftInLose.MinUdc_5s = gSoftInLose.MaxUdc_5s;
    } 
    if((gError.ErrorCode.all & ERROR_OVER_UDC) == ERROR_OVER_UDC)
    {
        gSoftInLose.MaxUdc_5s = gSoftInLose.MinUdc_5s; 
    }
 }
	#endif
#else
//MD380
void InputPhaseLoseDetect(void)			
{
	if(((STATUS_RUN!=gMainStatus.RunStep)&&(STATUS_SPEED_CHECK!=gMainStatus.RunStep)  //���л�ת�ٸ���״̬�ż��
	    &&(STATUS_GET_PAR!=gMainStatus.RunStep))||
	   (gInvInfo.InvTypeApply < gInLose.ForeInvType))
	{
		gInLose.Cnt = 0;
		gInLose.UpCnt = 0;
		gInLose.ErrCnt = 0;
		gInLose.CntRes = 0;
		gInLose.UpCntRes = 0;
		return;
	}
    
	if(PL_INPUT_HIGH)           // PL�Ǹߵ�ƽ		
	{
		gInLose.UpCnt ++;
		gInLose.UpCntRes ++;
	}
    
	if(gInLose.UpCntRes != 0)	// ����500ms��PL�͵�ƽ�ж�Ϊ�̵�������
	{
		gInLose.CntRes++;	
		if(gInLose.CntRes >= 250)       // 500ms
		{
			if((gSubCommand.bit.RelayEnalbe == 1) &&          //F9-12-ʮλ
				(gInLose.UpCntRes >= 249) && (gInvInfo.InvTypeApply >= 19))
			{
			    gError.ErrorCode.all |= ERROR_RESISTANCE_CONTACK;
			}
			gInLose.CntRes = 0;
			gInLose.UpCntRes = 0;
		}
	}

	gInLose.Cnt++;	
	if(gInLose.Cnt < 500)       //  ȱ����1secһ��ѭ�� 
    {
        return;
    }

	if((gSubCommand.bit.InputLost == 1) &&              //F9-12-��λ
	   (gInLose.UpCnt > 5) && (gInLose.UpCnt < 485))    // 1sec��PL���ڵ͵�ƽ���ȱ�෽�?
	{
		gInLose.ErrCnt++;
		if(gInLose.ErrCnt >= 2)
		{
			gError.ErrorCode.all |= ERROR_INPUT_LACK_PHASE;
			gInLose.ErrCnt = 0;
		}
	}
	else
	{
		gInLose.ErrCnt = 0;
	}
	gInLose.Cnt = 0;
	gInLose.UpCnt = 0;
}

#endif

/************************************************************
	������?
1��	�ϵ绺��״̬������1�����ڷ��Ȳ����У�
2��	����״̬���������У�
3��	ֱ���ƶ��ȴ��ڼ䣬��������
4��	ģ���¶ȵ���40�ȣ�����ֹͣ���¶ȸ���42�ȷ������У�40��42��֮�䲻�仯��
5)	��������������10��Źر�
************************************************************/
#if (SOFTSERIES == MD500SOFT)
void ControlFan(void)						
{
	if(gMainStatus.RunStep == STATUS_LOW_POWER)
	{
		TurnOffFan();
		gFanCtrl.EnableCnt = 0;
		return;
	}

	gFanCtrl.FanDetaTime = ((gFanCtrl.FanTime - GetTime()) & 0xFFFFFFUL)/DSP_CLOCK;
    gFanCtrl.FanTime = GetTime();    

    if(gFanCtrl.FanDetaTime > 2200)     //ִ�м������2.2ms ��Ϊʱ���޷���֤
    {
        gFanCtrl.ErrTimes++;
        if(gFanCtrl.ErrTimes > 10)
        {
            gFanCtrl.ErrTimes = 20;
        }
    }


	gFanCtrl.EnableCnt++;
	if(gFanCtrl.EnableCnt < 500)
	{
		TurnOffFan();
		return;
	}
	gFanCtrl.EnableCnt = 500;

    
	if((gMainCmd.Command.bit.Start == 1) ||
	   (gTemperature.Temp > 42) ||
	   (gSubCommand.bit.FanNoStop == 1))
	{
		gFanCtrl.RunCnt = 0;
        if(gFanCtrl.ErrTimes > 10)
        {
            TurnOnFan();
        }
        else
        {
            FanSpeedCtrl();
        }
	}
	else 
	{
        if(gTemperature.Temp < 40)
        {
            gFanCtrl.RunCnt++;
        }
		
		if(gFanCtrl.RunCnt >= 5000)
		{
			gFanCtrl.RunCnt = 5000;
            gFanCtrl.ErrTimes = 0;
			TurnOffFan();
		}
        else
        {
            if(gFanCtrl.ErrTimes > 10)
            {
                TurnOnFan();
            }
            else
            {
                FanSpeedCtrl();
            }
        }
	}
}
void FanSpeedCtrl(void)
{
    static u16 m_FanFlag = 0;
    static u16 m_FanStatus = 0;
    int m_AdjSpeedTemp;
    if((gInvInfo.InvTypeApply >= 25) && (gInvInfo.InvTypeApply <= 34))
    {
        m_AdjSpeedTemp = gFanSpeedCtrlTempreture[gInvInfo.InvTypeApply - 25];
        if(gTemperature.Temp >= m_AdjSpeedTemp)
        {
            TurnOnFan();
            m_FanStatus = 1;
        }
        else if(gTemperature.Temp < m_AdjSpeedTemp - 20)
        {
            if((m_FanFlag == 0) || (gTemperature.Temp <= -20))
            {
                TurnOnFan();
                m_FanFlag = 1;
            }
            else
            {
                TurnOffFan();
                m_FanFlag = 0;
            }
            m_FanStatus = 0;
        }
        else
        {
            if(m_FanStatus == 1)
            {
                TurnOnFan();
            }
            else
            {
                if(m_FanFlag == 0)
                {
                    TurnOnFan();
                    m_FanFlag = 1;
                }
                else
                {
                    TurnOffFan();
                    m_FanFlag = 0;
                }
            }
        }
    }
    else
    {
        TurnOnFan();
    }
}
#else
void ControlFan(void)						
{
	if(gMainStatus.RunStep == STATUS_LOW_POWER)
	{
		TurnOffFan();
		gFanCtrl.EnableCnt = 0;
		return;
	}

	gFanCtrl.EnableCnt++;
	if(gFanCtrl.EnableCnt < 500)
	{
		TurnOffFan();
		return;
	}
	gFanCtrl.EnableCnt = 500;

	if((gMainCmd.Command.bit.Start == 1) ||
	   (gTemperature.Temp > 42) ||
	   (gSubCommand.bit.FanNoStop == 1))
	{
		gFanCtrl.RunCnt = 0;
		TurnOnFan();
	}
	else if(gTemperature.Temp < 40)
	{
		gFanCtrl.RunCnt++;
		if(gFanCtrl.RunCnt >= 5000)
		{
			gFanCtrl.RunCnt = 5000;
			TurnOffFan();
		}
	}
}
#endif
/************************************************************
	��Ƶ���͵�����ر���
************************************************************/
void OverLoadProtect(void)				
{
	Ulong m_LDeta = 0;
	Uint m_Cur,m_Data,m_Index,m_CurBaseInv,m_MotorOvLoad;
	Uint m_TorCurBottom,m_TorCurUpper,m_TorCurStep,m_TorCurData;
    Uint *m_TorCurLine;
	Uint m_Limit;
    long m_OverLoadMargin,m_MaxCurLimit,m_Long;
	
#if (SOFTSERIES == MD500SOFT)	
    m_CurBaseInv = gLineCur.CurBaseInvOverload;
#else
    m_CurBaseInv = gLineCur.CurBaseInv;
#endif

/*	if( 28 <= gInvInfo.InvTypeApply )
    {
        //m_CurBaseInv = (s32)gLineCur.CurBaseInv * (s32)gInvInfo.InvCurrent / gInvInfo.InvOlCurrent;
        m_CurBaseInv = (long)gLineCur.CurBaseInv * (long)gInvInfo.InvCurrent / gInvInfo.InvCurrForP;
    }
*/    
	gOverLoad.FilterInvCur = Filter16(m_CurBaseInv,gOverLoad.FilterInvCur);
	gOverLoad.FilterMotorCur = Filter16(gLineCur.CurPer,gOverLoad.FilterMotorCur);
	gOverLoad.FilterRealFreq = Filter16(gMainCmd.FreqReal,gOverLoad.FilterRealFreq);

	if(gMainStatus.RunStep == STATUS_LOW_POWER)
	{
		gOverLoad.InvTotal.all = 0;
		gOverLoad.MotorTotal.all = 0;
#if (SOFTSERIES == MD500SOFT)	
		gOverLoad.BreakTotal = 0;
#endif
		gOverLoad.Cnt = 0;
		gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
		return;
	}

	/************************��ѹ�����ߺͶ�ת����**********************************/
    if(gOverLoad.OverLoadPreventEnable != 0)
	{
	    if((gOverLoad.InvTotal.half.MSW >= 30000L)&&(gMainStatus.RunStep == STATUS_RUN))
		{		    
		    if((gMainCmd.FreqReal < gMainCmd.XiuMianFreq - 200)&&(gMainCmd.XiuMianFreq > 200))  //����Ƶ�ʼ�2Hz
			{
				gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
			}
			else
			{
				if(gMainStatus.StatusWord.bit.OverLoadPreventState != 2)
				{
				    gMainStatus.StatusWord.bit.OverLoadPreventState = 1;
				}
			}
		}
	/*	else
		{
			if(gOverLoad.OverLoadPreventEnable == 1)
			{
			    gMainStatus.StatusWord.bit.OverLoadPreventState = 0;
			}
		}*/
	}
	/**************************end************************************************/
	gOverLoad.Cnt++;
	if(gOverLoad.Cnt < 5)		
	{
        return;		    //ÿ10ms���һ�?
	}
	gOverLoad.Cnt = 0;

	////////////////////////////////////////////////////////////////
	//ѡ���������

    if(1 == gInvInfo.GPType)        //G�ͻ��������ߣ���ת�ظ��ػ���
    {
        m_TorCurLine    = (Uint *)gInvOverLoadTable;
        m_TorCurBottom  = 1150;     //115%��Ƶ������
        m_TorCurUpper   = 1960;     //���ֵ196%����    // �����������ֵ,2011-10-21-chzq
        m_TorCurStep    = 90;
        m_TorCurData    = 5;        //>=196%,0.5s����
    }
#if(SOFTSERIES == MD380SOFT)
    else                            //P�ͻ��������ߣ������ˮ���ฺ�ػ���
    {       
        m_TorCurLine    = (Uint *)gInvOverLoadTableForP;
        m_TorCurBottom  = 1050;
        m_TorCurUpper   = 1410;     //���ֵ141%����    // �����������ֵ,2011-10-21-chzq
        m_TorCurStep    = 40;
        m_TorCurData    = 5;        //>=141%,0.5s����
    }
#else
    else                            //P�ͻ��������ߣ������ˮ���ฺ�ػ���
    {       
        m_TorCurLine    = (Uint *)gInvOverLoadTableForP;
        m_TorCurBottom  = 1100;
        m_TorCurUpper   = 1660;     //���ֵ166%����   
        m_TorCurStep    = 70;
        m_TorCurData    = 20;        //>=166%,2.0s����
    }
#endif

    m_MaxCurLimit = 5000;       // 500% ��֤��������
	////////////////////////////////////////////////////////////////
	//��ʼ�жϱ�Ƶ���Ĺ���
	m_Cur = ((Ulong)gOverLoad.FilterInvCur * 1000L) >> 12;

	if(m_Cur < m_TorCurBottom)
    {
		if(gOverLoad.InvTotal.half.MSW < 10)
		{
			gOverLoad.InvTotal.all = 0;
		}
        else if(gMainStatus.RunStep == STATUS_STOP)
		{
			gOverLoad.InvTotal.all -= gInvOverLoadDecTable[0];  
		}
		else if(m_Cur < 1000)       /*С�ڱ�Ƶ������������յ�ǰ������С���������ۼ���*/
		{
			gOverLoad.InvTotal.all -= gInvOverLoadDecTable[m_Cur/100 + 1];
		}
	}
	else
	{
		if(gOverLoad.FilterRealFreq < 500)		
		{
			m_Data = gOverLoad.FilterRealFreq * 13 + 26214;
			m_Cur  = (((Ulong)m_Cur)<<15)/m_Data;
            if(gOverLoad.OverLoadPreventEnable != 0)
            {
                if(gOverLoad.InvTotal.half.MSW > 10800)     //������30%����Ƶ������������170%
                {
                    m_MaxCurLimit = (1700UL * (Ulong)m_Data)>>15;
                }
            }
		}
		if(m_Cur >= m_TorCurUpper)
		{
			m_Data = m_TorCurData;
		}
		else
		{
			m_Index = (m_Cur - m_TorCurBottom)/m_TorCurStep;
			m_Data = *(m_TorCurLine + m_Index) -
			         (((long)(*(m_TorCurLine + m_Index)) - (*(m_TorCurLine + m_Index + 1))) * 
					  (long)(m_Cur - m_TorCurBottom - m_Index * m_TorCurStep))/m_TorCurStep;
		}
		m_LDeta = ((Ulong)3600<<16)/(Uint)m_Data;
		gOverLoad.InvTotal.all += m_LDeta;
	    	
		if(gOverLoad.InvTotal.half.MSW >= 36000)
		{
			gOverLoad.InvTotal.half.MSW = 36000;
			//AddOneError(ERROR_INV_OVER_LOAD,1);

			gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
			gError.ErrorCode.all |= ERROR_INV_OVER_LAOD;
            gError.ErrorInfo[1].bit.Fault1 = 2;
		}
	}
	//gOverLoad.InvTotal.half.MSW = gTestDataReceive.TestData28;
	gOverLoad.TotalPercent = (Ulong)gOverLoad.InvTotal.half.MSW * 100L/36000L;
    /*****************�����ش���***********************************/
	if((gOverLoad.OverLoadPreventEnable != 0)&&(gMainStatus.StatusWord.bit.OverLoadPreventState != 2))  //   �����߹��Ϻ����Ƶ���
	{
	    m_OverLoadMargin = 30000L - (long)gOverLoad.InvTotal.half.MSW;
	    if(m_OverLoadMargin < 720)
	    {
	        m_Limit = 1800;
	        if(m_OverLoadMargin < 400)
	        {
	            m_Limit = 1700;
	        }
	        m_Long = 1350 + (m_OverLoadMargin<<1);
	        if(m_MaxCurLimit > m_Long)
	        {
	            m_MaxCurLimit = m_Long;
	        }
	        m_MaxCurLimit = __IQsat(m_MaxCurLimit,m_Limit,950);
	    }
	    //gLineCur.OverLoadMargin = m_OverLoadMargin; //������
	    gLineCur.MaxCurLimit = m_MaxCurLimit;
	    gLineCur.MaxCurLimit =  __IQsat(gLineCur.MaxCurLimit,1800,250);
	}
	else
	{
	    gLineCur.MaxCurLimit = 1800;
	}
    /****************************end******************************/
    //��Ƶ������Ԥ��������
	if(((gOverLoad.InvTotal.all + m_LDeta * 1000UL)>>16) > 36000)
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 1;
	}
	else
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
	}


	MotorOverLoadTimeGenerate();//��������������ɺ���
	////////////////////////////////////////////////////////////////
	//��ʼ�жϵ���Ĺ���
	//if(gMainCmd.SubCmd.bit.MotorOvLoadEnable == 0)
	if(gSubCommand.bit.MotorOvLoad == 0)
	{
		gOverLoad.MotorTotal.all = 0;
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
		return;
	}

	m_Cur = ((Ulong)gOverLoad.FilterMotorCur * 1000L)>>12;
	//m_Cur = ((Ulong)m_Cur * 100L)/100;//gComPar.MotorOvLoad;          // ���ݹ��ر���ϵ�����������������
                                                            	//Ȼ���øñ���������ѯ���ر�������
	m_LDeta = (Ulong)m_Cur * (Ulong)gMotorInfo.CurBaseCoff;
	if(m_LDeta >= (C_MOTOR_OV_MAX_CUR * 256L))
	{
		m_Cur = C_MOTOR_OV_MAX_CUR;
	}
	else
	{
		m_Cur = m_LDeta>>8;
	}
    

	m_MotorOvLoad = __IQsat(gComPar.MotorOvLoad,150,100);


	if(m_Cur < (u16)((u32)m_MotorOvLoad*C_MOTOR_OV_MIN_CUR/100UL))
	{
		if(gOverLoad.MotorTotal.half.MSW < 10)
		{
			gOverLoad.MotorTotal.all = 0;
		}
        else if(gMainStatus.RunStep == STATUS_STOP)
		{
			gOverLoad.MotorTotal.all -= gMotorOverLoadDecTable[0];  
		}
		else if(m_Cur < m_MotorOvLoad*10)                   /*С��100%��������յ��������������*/
		{
			gOverLoad.MotorTotal.all -= gMotorOverLoadDecTable[m_Cur/m_MotorOvLoad + 1];  
		}
	}
	else
	{
		if(m_Cur >= C_MOTOR_OV_MAX_CUR)
		{
			m_Data = gMotorOverLoadTable[C_MOTOR_OV_TAB_NUM - 1];
		}
		else
		{
			m_Index = (m_Cur - (u16)((u32)m_MotorOvLoad*C_MOTOR_OV_MIN_CUR/100UL))/C_MOTOR_OV_STEP_CUR;
			m_Data = gMotorOverLoadTable[m_Index] -
			         ((long)(gMotorOverLoadTable[m_Index] - gMotorOverLoadTable[m_Index+1]) * 
					  (long)(m_Cur - (u16)((u32)m_MotorOvLoad*C_MOTOR_OV_MIN_CUR/100UL) - m_Index * C_MOTOR_OV_STEP_CUR))/C_MOTOR_OV_STEP_CUR;
		}
		m_LDeta = ((Ulong)3600<<16)/(Uint)m_Data;
		gOverLoad.MotorTotal.all += m_LDeta;

		if(gOverLoad.MotorTotal.half.MSW > 36000)
		{
			gOverLoad.MotorTotal.half.MSW = 36000;
			//AddOneError(ERROR_MOTOR_OVER_LOAD,1);
			gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
			gError.ErrorCode.all |= ERROR_MOTOR_OVER_LOAD;
		}		
	}
    //�������Ԥ������?  
	//if(gOverLoad.MotorTotal.half.MSW > gBasePar.PerMotorOvLoad * 360)
	if(gOverLoad.MotorTotal.half.MSW > gComPar.PerMotorOvLoad * 360)
	{
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 1;
	}
	else
	{
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
	}
}


/************************************************************
	����������״̬�µĹ��ر���
//���ܳ���������ʱ�䳬��5000ms��������Ӧ2500��2ms
����ϳ������������������150��������10s�������壬֮ǰӰ�����
************************************************************/
void CBCLimitCurProtect(void)
{
	int     m_CurU,m_CurV,m_CurW,m_Coff;
	int	    m_Max,m_Add,m_Sub;
    Uint    m_Limit;
    
	if((gSubCommand.bit.CBCEnable == 1) && (gMainStatus.RunStep != STATUS_GET_PAR) && (gMainStatus.RunStep != STATUS_IPM_INIT_POS) && (gMainStatus.RunStep != STATUS_FLYING_START))   //DQ���кͳ�ʼλ�ñ�ʶʱ������������
	{
		if(gCBCProtect.EnableFlag == 0) SetCBCEnable();	//��������������
	}
	else
	{
		if(gCBCProtect.EnableFlag == 1)  SetCBCDisable();//ȡ������������
	}
		
	//��ʼ�ֱ���������������ֵ�Ļ���
	m_Coff = (((long)gMotorInfo.Current)<<10) / gInvInfo.InvCurrent;
	m_CurU = (int)(((long)gIUVWQ12.U * (long)m_Coff)>>10);
	m_CurU = abs(m_CurU);
	m_CurV = (int)(((long)gIUVWQ12.V * (long)m_Coff)>>10);
	m_CurV = abs(m_CurV);
	m_CurW = (int)(((long)gIUVWQ12.W * (long)m_Coff)>>10);
	m_CurW = abs(m_CurW);
    
	//��ʼ�ж��Ƿ���һ��������������5��
	if(m_CurU > 9267)	gCBCProtect.CntU++;             // 9267 = 4096 * 1.414 * 1.6
	else				gCBCProtect.CntU = gCBCProtect.CntU>>1;
	gCBCProtect.CntU = (gCBCProtect.CntU > 3000)?3000:gCBCProtect.CntU;

	if(m_CurV > 9267)	gCBCProtect.CntV++;
	else				gCBCProtect.CntV = gCBCProtect.CntV>>1;
	gCBCProtect.CntV = (gCBCProtect.CntV > 3000)?3000:gCBCProtect.CntV;

	if(m_CurW > 9267)	gCBCProtect.CntW++;
	else				gCBCProtect.CntW = gCBCProtect.CntW>>1;
	gCBCProtect.CntW = (gCBCProtect.CntW > 3000)?3000:gCBCProtect.CntW;
    
	if(gMainCmd.FreqReal > 20)
	{
		gCBCProtect.CntU = 0;
		gCBCProtect.CntV = 0;
		gCBCProtect.CntW = 0;
	}

	if((gCBCProtect.CntU > 2500) || 		//��������������, �κ�һ�����5000ms
	   (gCBCProtect.CntV > 2500) || 
	   (gCBCProtect.CntW > 2500) )
	{
		gError.ErrorCode.all |= ERROR_INV_OVER_LAOD;
        gError.ErrorInfo[1].bit.Fault1 = 1;
        gLineCur.ErrorShow = gLineCur.CurPer; //zzb1812
	}
    
	if(gCBCProtect.EnableFlag == 0)
	{
		gCBCProtect.TotalU = 0;
		gCBCProtect.TotalV = 0;
		gCBCProtect.TotalW = 0;
		return;
	}
#if (SOFTSERIES == MD380SOFT)
    if((INV_VOLTAGE_690V == gInvInfo.InvVoltageType)||(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)) 
    {
        if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
        {
            m_Limit = 10; 
        } 
        else
        {
            m_Limit = 150;  //������������������
        }
        
        if(0 == gCBCProtect.CbcFlag)              // �����������巢��
        {
            gCBCProtect.NoCbcTimes++;
            if(5000 <= gCBCProtect.NoCbcTimes)   //����10��û��������������֮ǰ��Ӱ��������
            {
                gCBCProtect.NoCbcTimes = 5000;
                gCBCProtect.CbcTimes = 0;
            }
        }
        else                                     // �����������巢��
        {
            gCBCProtect.NoCbcTimes = 0;
            gCBCProtect.CbcFlag = 0;
        }

        if(m_Limit <= gCBCProtect.CbcTimes) 
        {
            gError.ErrorCode.all |= ERROR_TRIP_ZONE;
            gCBCProtect.CbcTimes = m_Limit - 1;      //�������������޷���λ
        } 
    }
    else
#endif
    {
        m_Limit = 500;     
    	m_Add = 2;
    	m_Sub = 1;
    	if(gMainStatus.RunStep == STATUS_STOP)
        {
            m_Sub = 2;
        }

    	if(gCBCProtect.Flag.bit.CBC_U == 1)   //����U������Ļ���
    	{
    		gCBCProtect.TotalU += m_Add;
    	}
    	else //if(m_CurU < 8688)					//С��1.5����Ƶ����ֵ�������ۼ�ֵ�ݼ� : 4096*1.5*sqrt(2)
    	{
    		gCBCProtect.TotalU -= m_Sub;
    	}

    	if(gCBCProtect.Flag.bit.CBC_V == 1) 	//����V������Ļ���
    	{
    		gCBCProtect.TotalV += m_Add;
    	}
    	else //if(m_CurV < 8688)					//С��1.5����Ƶ���������ۼ�ֵ�ݼ�
    	{
    		gCBCProtect.TotalV -= m_Sub;
    	}

    	if(gCBCProtect.Flag.bit.CBC_W == 1) 	//����W������Ļ���
    	{
    		gCBCProtect.TotalW += m_Add;
    	}
    	else //if(m_CurW < 8688)					//��1.5����Ƶ���������ۼ�ֵ�ݼ�
    	{
    		gCBCProtect.TotalW -= m_Sub;
    	}

    	gCBCProtect.Flag.all = 0;

    	//��������ֵ�޷�
    	gCBCProtect.TotalU = __IQsat(gCBCProtect.TotalU, m_Limit, 0);
    	gCBCProtect.TotalV = __IQsat(gCBCProtect.TotalV, m_Limit, 0);
    	gCBCProtect.TotalW = __IQsat(gCBCProtect.TotalW, m_Limit, 0);

    	m_Max = (gCBCProtect.TotalU > gCBCProtect.TotalV) ? gCBCProtect.TotalU : gCBCProtect.TotalV;
    	m_Max = (m_Max > gCBCProtect.TotalW) ? m_Max : gCBCProtect.TotalW;
        if(m_Max >= m_Limit)      //��������40�Ź���
        {
            gError.ErrorCode.all |= ERROR_TRIP_ZONE;
        }          
    }
}

/*************************************************************
	��������������
*************************************************************/
void SetCBCEnable(void)
{
	gCBCProtect.EnableFlag = 1;
    EALLOW;
    
#ifdef TMS320F2808	
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_ENABLE;		// EPWM1��������
	EPwm1Regs.TZSEL.bit.CBC4 = TZ_ENABLE;
        
	EPwm2Regs.TZSEL.bit.CBC2 = TZ_ENABLE;       // EPWM2
    EPwm2Regs.TZSEL.bit.CBC3 = TZ_ENABLE;
    EPwm2Regs.TZSEL.bit.CBC4 = TZ_ENABLE;

    EPwm3Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm3Regs.TZSEL.bit.CBC3 = TZ_ENABLE;
	EPwm3Regs.TZSEL.bit.CBC4 = TZ_ENABLE;       // EPWM3
#else
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm2Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
    EPwm3Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
#endif

	EDIS;
}
/*************************************************************
	�ر�����������
*************************************************************/
void SetCBCDisable(void)
{
	gCBCProtect.EnableFlag = 0;

	EALLOW;
    EPwm2Regs.TZEINT.all = 0x0000;//��ֹTZ2�жϣ��������ж�
#ifdef TMS320F2808
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_DISABLE;		// EPWM1��������
	EPwm1Regs.TZSEL.bit.CBC4 = TZ_DISABLE;
        
	EPwm2Regs.TZSEL.bit.CBC2 = TZ_DISABLE;       // EPWM2
    EPwm2Regs.TZSEL.bit.CBC3 = TZ_DISABLE;
    EPwm2Regs.TZSEL.bit.CBC4 = TZ_DISABLE;

    EPwm3Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm3Regs.TZSEL.bit.CBC3 = TZ_DISABLE;
	EPwm3Regs.TZSEL.bit.CBC4 = TZ_DISABLE;       // EPWM3
#else
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm2Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
    EPwm3Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
#endif

	EDIS;
}
/*************************************************************
	������豣������
	
�����Ľ���Ƿѹ״̬����Ϊ�ǻ���������
*************************************************************/
void BufferResProtect(void)
{
	if(gBuffResCnt >= 150000L)			//������豣������
	{
		gError.ErrorCode.all |= ERROR_RESISTER_HOT;
	}
	gBuffResCnt--;	
	gBuffResCnt = __IQsat(gBuffResCnt,200000L,0);					
}

#if (SOFTSERIES == MD500SOFT)	

/*************************************************************
�ƶ�IGBTֱͨ�жϣ����ж���ִ�У�ÿ�ز�����ִ��һ��
û�п�ͨ��־ʱ�ж�
û�п�ͨʱ�������ڶ������37.5%��ֱͨ
*************************************************************/
void  BrakeIGBTProtect(void)
{
	Uint m_Mincurrent;
	if(((gBrake.Flag & 0x02) == 0x02) || (gExcursionInfo.IbOkFlag == 0))
	{
		gBrake.IgbtShotTicker = 0;
		return;
	}
	// �ض�ʱ����Ƿ��е���
	m_Mincurrent = (gInvInfo.InvBreakCurrent * 3) >> 3;
	if (gBrake.IBreak > m_Mincurrent)
	{
		gBrake.IgbtShotTicker += gPWM.gPWMPrdApply;
	}	
	else
	{
		gBrake.IgbtShotTicker = 0;
	}
	// ����2s����
	if (gBrake.IgbtShotTicker >= 60000000L) // 2s = 2 * 60 000 000/2 
	{	
		gBrake.IgbtShotTicker = 60000000L;
		gError.ErrorCode.all |= ERROR_IGBT_SHORT_BRAKE;	
        gError.ErrorInfo[4].bit.Fault1 = 2;	
	}
	/*�ƶ��ܹ����ж�,��֮ǰ���ƶ��ܹ��ر����������Ƶ��ú�����*/
	if (gBrake.IBreak >= (gInvInfo.InvBreakMaxCurrent * 2))
	{
		gBrake.FilterTicker ++;			
	}
	//���3�����ϱ�����
	if (gBrake.FilterTicker >= 3)
	{	
		gBrake.FilterTicker		= 0;
		gError.ErrorCode.all |= ERROR_OVER_CURRENT;
		gError.ErrorInfo[0].bit.Fault1 = 2;//�ƶ��������
		gLineCur.ErrorShow		= gLineCur.CurPer;
	}
}

/*************************************************************
1���ƶ�����ѡȡ̫С
2���ƶ������ϴ�ʱ��ʱ��ϳ�

*************************************************************/
void  BrakeOverloadProtect(void)
{
    u16 m_CurrCoff;
	Ulong	IBreakrsm,Duty;
	int	m_Index,m_IndexData;

	if(((gBrake.OnCounter == 0)&&(gBrake.TotalCounter > 1))||(gBrake.TotalCounter == 0))
	{
		gBrake.IBreakrsm    = 0;
        gBrake.TotalCounter	= 0;
	}	
	else if(gBrake.TotalCounter > 1)
	{
		DINT;		
		Duty = ((Ulong)gBrake.OnCounter<<20)/gBrake.TotalCounter;
		gBrake.OnCounter 		= 0;
		gBrake.TotalCounter		= 0;
		EINT;
		IBreakrsm = gBrake.IBreakTotal;
		Duty = qsqrt(Duty);
		gBrake.IBreakrsm = IBreakrsm*Duty>>10;
	}
	/*�ƶ��ܹ����ж�*/
	if (gBrake.IBreakrsm > gInvInfo.InvBreakCurrent)
	{	
        m_Index = (Ulong)gBrake.IBreakrsm * 100UL/gInvInfo.InvBreakMaxCurrent;
		if (m_Index >= 100)     
		{
			m_IndexData= (m_Index - 100)/10;
            m_IndexData= __IQsat(m_IndexData,1,0);
            if(m_Index < 120)
            {
    			m_CurrCoff = *(gBrake.pBreakOverLoad+(1 - m_IndexData));
            }
            else
            {
    			m_CurrCoff = 1;//�ۼӵ��ٶ���1000����100ms������
            }
		}
		else if (m_Index >= 90)    
		{
            m_Index = ((int)m_Index - (int)gBrake.minPercent)/gBrake.OverLoadInterval;
            m_Index = __IQsat(m_Index,4,-1);
			
			m_Index = (6 - m_Index);
            
			m_CurrCoff = *(gBrake.pBreakOverLoad + m_Index);			
		}
		else
		{
			m_Index = (m_Index - 70)/5;
            m_Index = __IQsat(m_Index,3,1);
            
			m_CurrCoff = *(gBrake.pBreakOverLoad+(10 - m_Index));
		}
		gOverLoad.BreakTotal += (2000000L/m_CurrCoff);//�ۼӵ��ٶ���10����10s������
	}	
	else if (gOverLoad.BreakTotal > gBrake.OverLoadDegradeCoff)  // ������Сʱ���ؼ���ֵ��С
	{
		gOverLoad.BreakTotal -= gBrake.OverLoadDegradeCoff; //��Сʱ��90s
	}
	else
	{
		gBrake.FilterTicker = 0;
	}
	if (gOverLoad.BreakTotal >= 100000000L) 
	{
		gOverLoad.BreakTotal = 100000000L;
		gError.ErrorCode.all |= ERROR_OVERLOAD_BRAKE;
        gError.ErrorInfo[3].bit.Fault4 = 1;
        gLineCur.ErrorShow = gLineCur.CurPer; 
	}
    
}
#else
/*************************************************************
	�ƶ�������·��������
	
�����?
1��sizeD\SizeE,��7.5KW-30KW�Ž����ƶ������·�������
2��ͣ��״̬���ƶ���Ԫû�д�Ҳ���
�ж�ԭ��:
1)����ΪSizeD��SizeE
2)��һ�ν�������жϣ�ֻ��¼�������ϵ����ϴ����ϣ���ʱ�ض��ƶ�����ͬʱ����ͣ��״̬
3)10ms����TZ�ź���ȻΪ�͵�ƽ������Ϊ���ƶ������·������Ϊ��������
4)�ƶ������·���ϲ������ֶ���λ��ֻ�ܵ��縴λ

*************************************************************/
void  BrakeResShortProtect(void)
{
    if((19 > gInvInfo.InvTypeApply)&&(12 < gInvInfo.InvTypeApply))
    {
        gBrake.BreResShotDetFlag = 1;            //7.5KW-30KW����Ҫ���
    }
    else
    {
        gBrake.BreResShotDetFlag = 0;
    } 

    if((gBrake.BreResShotDetFlag == 1)&&(gBrake.ErrCode == ERROR_OVER_CURRENT))//ֻ�е�һ��Ϊ��������ʱ��Ҫ�����ƶ������·�����
    {                                                          //����һ�μ�¼Ϊ��ѹ���ϣ���ֱ�ӱ���ѹ 
        gBrake.WaitTime++;
        if(gBrake.WaitTime < 6)  //10ms
        {
            EALLOW;
        	EPwm1Regs.TZCLR.bit.OST = 1;
        	EDIS;
        }
        else
        {                                              
            if(EPwm1Regs.TZFLG.bit.OST == 1)
            {
                gError.ErrorCode.all |= ERROR_SHORT_BRAKE; //60 �����ֶ���λ     
                //gError.ErrorInfo[0].bit.Fault1 = 1;
            }
            else
            {
                gError.ErrorCode.all |= ERROR_OVER_CURRENT ;       
                gError.ErrorInfo[0].bit.Fault1 = 1;
            }
            gBrake.WaitTime = 0;
            gBrake.ErrCode  = 0;
        }
    }
    else
    {
        gBrake.WaitTime = 0;
    }
}
#endif
