/****************************************************************
�ļ����ܣ������ӹ��ܲ��ֻ�õ����в���
�ļ��汾�� 
�������ڣ� 

****************************************************************/
#include "MotorInclude.h"

extern PMSM_FLUX_WEAK_STRUCT   gPmFluxWeak;
/***********************************************************************
��Ƶ���ƶ��������:
��λС���㣬ֻ�����¼�������ȷ�Ϲ�
����   ����   ����� 	max(10s)(��Ӧ1.5V)
18.5    16    9.49A   		32.5
22      17    11.29A		32.5
30      18    15.39A		40.63
37      19    18.98A		52.71
45      20    23.08A		60.94
55      21    28.16A		81.25
75      22    38.43A		114.71 
Ŀǰ��������:
���������10S������ERROR_OVERLOAD_BRAKE  ERR61 ������Ϣ1
����MAX����ֱ�ӱ�����ERROR_OVERLOAD_BRAKE  ERR61 ������Ϣ2
Ӳ�������궨�������������㣬1.5V��Ӧ������
û�п�ͨ�ƶ��ܵ����е������ƶ���ֱͨERROR_SHORT_BRAKE ERR62
************************************************************************/
#if (SOFTSERIES == MD500SOFT)
Uint const gInvBreakCurrentTable380T[27] =         // 380V,480V
{
    949,  949,  949,  949, 949,	     //8~12  
    949,  949,  949,  3055,3055,         //13~17
    3250,  4111, 5545, 7393,          //18~21  
    
    997,  384,  384,  384, 384,         //22~26  ���µ������ݰ���һ��С����
    384,  384,  384,  384, 384,         //27~31
    384,  384,  384                       //32~34
};
Uint const gInvBreakMaxCurrentTable380T[27] =         // 380V,480V
{
    3250,  3250,  3250,  3250, 3250,	     //8~12  
    3250,  3250,  3250,  3250,3250,       //13~17
    4063,  5271,  6094,  8125,      //18~21            
    
    1147, 1147, 1147, 1147, 1147,        //22~26  ���µ������ݰ���һ��С����
    1147, 1147, 1147, 1147, 1147,        //27~31
    1147 ,1147 ,1147                       //32~34
};
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
// // ͣ������ת��
void SystemParChg2Ms()	
{
	Ulong m_ULong;
    Uint    minType, maxType;
    Uint    *pInvVolt, *pInvCur;
    Uint 	 m_UData,m_Pointer;
#if (SOFTSERIES == MD500SOFT)
	Uint *pInvBreakCur,*pInvBreakMaxCur;
#endif
    
// ��ȡ��Ƶ��ϵͳ����
    //gInvInfo.GPType = gInvInfo.GpTypeSet;
	if(100 > gInvInfo.InvTypeSet)           // 380V
	{
        pInvVolt  = (Uint *)gInvVoltageInfo380T;
        pInvCur = (Uint *)gInvCurrentTable380T;
#if (SOFTSERIES == MD500SOFT)
        pInvBreakCur = (Uint *)gInvBreakCurrentTable380T;
		pInvBreakMaxCur = (Uint *)gInvBreakMaxCurrentTable380T;
#endif
        gInvInfo.InvType  = gInvInfo.InvTypeSet;
		gInvInfo.GPType = gInvInfo.GpTypeSet;
        gInvInfo.InvVoltageType = INV_VOLTAGE_380V;
	}
	else if(200 > gInvInfo.InvTypeSet)      // 220T, û��P�ͻ�
	{
	    pInvVolt  = (Uint *)gInvVoltageInfo220T;//ʹ��380V��Ƶ���Ķ������Ҳ���Ǳ����ƵĻ���
        pInvCur   = (Uint *)gInvCurrentTable220T;
#if (SOFTSERIES == MD500SOFT)
        pInvBreakCur = (Uint *)gInvBreakCurrentTable380T;
		pInvBreakMaxCur = (Uint *)gInvBreakMaxCurrentTable380T;
#endif
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 100;
        gInvInfo.InvVoltageType = INV_VOLTAGE_220V;
        gInvInfo.GPType   = 1;                                      //������P�ͻ�

        //****************************** 380V����Ϊ220V�����¶����ߴ���  
        m_Pointer = gInvInfo.InvType - 7;  
        gInvInfo.InvTypeGaiZhi = gInvTypeTable380To220T[m_Pointer];        
	}
#if (SOFTSERIES == MD380SOFT)
    else if(300 > gInvInfo.InvTypeSet) //220S
    {
        pInvVolt  = (Uint *)gInvVoltageInfo220S;
        pInvCur = (Uint *)gInvCurrentTable220S;
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 200;
        gInvInfo.InvVoltageType = INV_VOLTAGE_220V;  
        gInvInfo.GPType = gInvInfo.GpTypeSet;
    }
#endif
    else if(400 > gInvInfo.InvTypeSet) //480V
    {
#if (SOFTSERIES == MD500SOFT)  //��380V��      
		pInvVolt  = (Uint *)gInvVoltageInfo480T;	//����380Vһ��
        pInvCur = (Uint *)gInvCurrentTable380T;  // ��380V���õ�����
        pInvBreakCur = (Uint *)gInvBreakCurrentTable380T;// ��380V���õ�����
		pInvBreakMaxCur = (Uint *)gInvBreakMaxCurrentTable380T;// ��380V���õ�����
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 300;
        gInvInfo.InvVoltageType = INV_VOLTAGE_380V;//MD500��480V��380V
        gInvInfo.GPType = gInvInfo.GpTypeSet;
#else
		pInvVolt  = (Uint *)gInvVoltageInfo480T;
        pInvCur = (Uint *)gInvCurrentTable380T;  // ��380V���õ�����
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 300;
        gInvInfo.InvVoltageType = INV_VOLTAGE_480V;
        gInvInfo.GPType = gInvInfo.GpTypeSet;
#endif
    }
#if (SOFTSERIES == MD380SOFT)
    else if(500 > gInvInfo.InvTypeSet) //690V                               //690V
    {
        pInvVolt  = (Uint *)gInvVoltageInfo690T;
        pInvCur = (Uint *)gInvCurrentTable690T;
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 400;
        gInvInfo.InvVoltageType = INV_VOLTAGE_690V;
        gInvInfo.GPType = gInvInfo.GpTypeSet;
    }
    else
    {
        pInvVolt  = (Uint *)gInvVoltageInfo1140T;
        pInvCur = (Uint *)gInvCurrentTable1140T;
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 500;
        gInvInfo.InvVoltageType = INV_VOLTAGE_1140V;
        gInvInfo.GPType = gInvInfo.GpTypeSet;
    }    
#endif    
	gInvInfo.InvVolt     = *(pInvVolt + 0);
	gInvInfo.InvLowUdcStad = *(pInvVolt + 1);
	gInvInfo.InvUpUDC    = *(pInvVolt + 2);
	gInvInfo.BaseUdc     = *(pInvVolt + 3);
    minType              = *(pInvVolt + 4);  //��Ƶ�������Ļ�������
    maxType              = *(pInvVolt + 5);  //��Ƶ�������Ļ�������
    gInLose.ForeInvType  = *(pInvVolt + 6);  //��ʼȱ�ౣ������ʼ����
    gUDC.uDCADCoff       = *(pInvVolt + 7); 
    gVFPar.ovPointCoff   = *(pInvVolt + 8);     // ���ڼ����ѹ���Ƶ�

	gInvInfo.InvType = (gInvInfo.InvType > maxType) ? maxType : gInvInfo.InvType;
	if(gInvInfo.InvType <= minType)
    {
        gInvInfo.GPType = 1;	                        //��С�����޷�����GP
        gInvInfo.InvType = minType;
	}
#if (SOFTSERIES == MD500SOFT)
    if(gInvInfo.GPType == 1)   
	{
	    gInvInfo.InvTypeApply = gInvInfo.InvType;
	}
    else if(gInvInfo.InvType < 29)    // 220Kw P������
    {
        gInvInfo.InvTypeApply = gInvInfo.InvType - 1;   //P�ͻ�����������һ��
    }
	else if(gInvInfo.InvType >= 29)   // 250Kw p������
	{
	    gInvInfo.InvTypeApply = gInvInfo.InvType - 2;   //����P�ͻ���������������
	}
    if((gInvInfo.InvType == 34) && (gInvInfo.GPType == 1))                          // 450Gģ����400Gһ������������������ͬ����������450P���б���
    {
        gInvInfo.GPType = 2;
    }
#else
    if(gInvInfo.GPType == 1)   
	{
	    gInvInfo.InvTypeApply = gInvInfo.InvType;
	}
    else 
    {
        gInvInfo.InvTypeApply = gInvInfo.InvType - 1;   //P�ͻ�����������һ��
    }
#endif	
	gInvInfo.InvCurrent  = *(pInvCur + gInvInfo.InvTypeApply - minType);        //��������ʹ�õĶ����
    //gInvInfo.InvCurrentOvload = *(pInvCur + gInvInfo.InvType - minType);
#if (SOFTSERIES == MD500SOFT)
	if((2 == gInvInfo.GPType) && (22 == gInvInfo.InvType))  // MD500 30P��75P������G�ͻ���ͬ
	{
		gInvInfo.InvCurrForP = 1500; //MD500��75P�����Ϊ150A
	}
	else if((2 == gInvInfo.GPType) && (18 == gInvInfo.InvType))
	{
        gInvInfo.InvCurrForP = 6000;
    }
	else
	{
    	gInvInfo.InvCurrForP = *(pInvCur + gInvInfo.InvType - minType);      //P�ͻ�ʹ�õĶ����
    }
    #if (SOFT_INPUT_DETECT == STLEN)
	gSoftInLose.MaxDetaU = 650;
    #endif
	gInvInfo.InvBreakCurrent = *(pInvBreakCur + gInvInfo.InvTypeApply - minType); 
    gInvInfo.InvBreakMaxCurrent = *(pInvBreakMaxCur + gInvInfo.InvTypeApply - minType); 
	gBrake.OverLoadInterval = 2;
	gBrake.minPercent = 90;
	switch(gInvInfo.InvTypeApply)
	{
		case 16:
		case 17:
			gBrake.pBreakOverLoad 	= (Uint *)gInvBreakOverLoadTableFor185And22;
			gBrake.OverLoadInterval	= 1;
			gBrake.minPercent 		= 95;
			gBrake.OverLoadDegradeCoff = 182;
			break;
			
		case 18:
			gBrake.pBreakOverLoad = (Uint *)gInvBreakOverLoadTableFor30;
			gBrake.OverLoadDegradeCoff = 390;
			break;
			
		case 19:
			gBrake.pBreakOverLoad = (Uint *)gInvBreakOverLoadTableFor37;
			gBrake.OverLoadDegradeCoff = 445;
			break;
			
		case 20:
		case 21:
			gBrake.pBreakOverLoad = (Uint *)gInvBreakOverLoadTableFor45And55;
			gBrake.OverLoadDegradeCoff = 182;
			break;
			
		case 22:
			gBrake.pBreakOverLoad = (Uint *)gInvBreakOverLoadTableFor75;
			gBrake.OverLoadDegradeCoff = 287;
			break;
	}
#else
    gInvInfo.InvCurrForP = *(pInvCur + gInvInfo.InvType - minType);      //P�ͻ�ʹ�õĶ����
#endif
    if(gInvInfo.InvVoltageType == INV_VOLTAGE_220V)                             // ����С�����ȷ��
    {   
        gMotorExtInfo.UnitCoff = (gInvInfo.InvType > 18) ? 10 : 1;
    }
    else
    {
        gMotorExtInfo.UnitCoff = (gInvInfo.InvType > 21) ? 10 : 1; 
    }
    
	if((1 != gInvInfo.GPType) && (22 == gInvInfo.InvType))
	{
        // ����Ϊ22��P�ͻ�������Ӧ����1λС���㣬���Ƕ�ȡ�Ĳ���������2λ  
		gInvInfo.InvCurrent = (Ulong)gInvInfo.InvCurrent * 3264 >> 15;
    }
    //gInvInfo.InvCurrForP = *(pInvCur + gInvInfo.InvType - minType);      //P�ͻ�ʹ�õĶ����

//������������������
	m_UData = (gInvInfo.InvTypeApply > 29) ? 29 : gInvInfo.InvTypeApply;
	m_UData = (m_UData < 12) ? 12 : m_UData;
#if (SOFTSERIES == MD380SOFT)
    if(500 <= gInvInfo.InvTypeSet)                //1140V����������������ȷ�� 2011.5.8 L1082
    {
        gDeadBand.DeadBand = DBTIME_1140V*gDeadBand.DeadTimeSet/10;          
        gDeadBand.Comp     = DCTIME_1140V*gDeadBand.DeadTimeSet/10;
    }
    else if(400 <= gInvInfo.InvTypeSet)                       //690V���������Ͳ������̶�
    {
	    gDeadBand.DeadBand = gDeadBandTable[13];         //�����̶�4.8us��������2.5us
	    gDeadBand.Comp     = gDeadCompTable[13];
    }
    else
    {
	    gDeadBand.DeadBand = gDeadBandTable[m_UData - 12];
	    gDeadBand.Comp     = gDeadCompTable[m_UData - 12];
    }
#else
    gDeadBand.DeadBand = gDeadBandTable[m_UData - 12];
	gDeadBand.Comp     = gDeadCompTable[m_UData - 12];
#endif
//	gDeadBand.MaxComp  = gDeadBand.DeadBand>>1; 
	EALLOW;									//��������ʱ��
	EPwm1Regs.DBFED = gDeadBand.DeadBand;
	EPwm1Regs.DBRED = gDeadBand.DeadBand;
	EPwm2Regs.DBFED = gDeadBand.DeadBand;
	EPwm2Regs.DBRED = gDeadBand.DeadBand;
	EPwm3Regs.DBFED = gDeadBand.DeadBand;
	EPwm3Regs.DBRED = gDeadBand.DeadBand;
	EDIS;
        
//����ADC�����ӳ�ʱ��
 /*   #ifdef		DSP_CLOCK100
	gADC.DelayApply = gADC.DelaySet * 10;       // default: 0.5us
    #else
	gADC.DelayApply = gADC.DelaySet * 6;
    #endif
*/
//�������ϵ���͵�ѹϵ��
	//�������̫С�Ĵ���
	m_UData = gInvInfo.InvCurrent>>2;
	DINT;
	gMotorInfo.Current = (gMotorInfo.CurrentGet < m_UData) ? m_UData : gMotorInfo.CurrentGet;
	m_ULong = (((Ulong)gMotorInfo.Current)<<8) / gMotorInfo.CurrentGet;
	gMotorInfo.CurBaseCoff = (m_ULong > 32767) ? 32767 : m_ULong;
	EINT;
    
	//ADֵ32767��Ӧ��Ƶ�������ֵ��2�� ת��Ϊ���������ı�ôֵ��ʾ(Q24)
	//ADת��ֵ�л�Ϊ�����ôֵ��ʾֵ�ķ���Ϊ: (AD/32767 * 2sqrt(2) * Iv/Im) << 24 *8
	// (1/32767 * 2sqrt(2) << 24) * 8 == 11586
	// CPU28035ʱ��32767��Ӧ�ĵ���ֵΪ: 2sqrt(2) *3.3/3.0 * Iv
/*
#ifdef TMS320F2808
    m_Long = ((long)gInvInfo.InvCurrent * 11586L)/gMotorInfo.Current;
#else
	m_Long = ((long)gInvInfo.InvCurrent * 12745L)/gMotorInfo.Current;
#endif
	gCurSamp.Coff = (m_Long * (long)gInvInfo.CurrentCoff) / 1000;

#ifdef TMS320F28035
    gUDC.uDCADCoff = (long)gUDC.uDCADCoff * 3300L / 3000;   // *1.1
#endif
	gUDC.Coff = ((Ulong)gUDC.uDCADCoff * (Ulong)gInvInfo.UDCCoff) / 1000;
*/

//���㲻ͬ�ֱ��ʵ�Ƶ�ʱ�ʾ
    if( 0 == gExtendCmd.bit.FreqUint )      // unit: 1Hz
    {
        gMainCmd.si2puCoeff = 1;
        gMainCmd.pu2siCoeff = 100;
    }        
    else if(1 == gExtendCmd.bit.FreqUint)   // unit: 0.1Hz
    {
        gMainCmd.si2puCoeff = 10;
        gMainCmd.pu2siCoeff = 10;
    }
    else // 2 == frqUnit                        // unit: 0.01Hz
    {
        gMainCmd.si2puCoeff = 100;       // si 2 pu
        gMainCmd.pu2siCoeff = 1;         // pu 2 si
    }    
	gBasePar.FullFreq01 = (Ulong)gBasePar.MaxFreq * (Ulong)gMainCmd.pu2siCoeff + 2000;	//32767��ʾ��Ƶ��ֵ
	gBasePar.FullFreq =   gBasePar.MaxFreq + 20 * gMainCmd.si2puCoeff;	//Ƶ�ʻ�ֵ
	gMotorInfo.FreqPer =  ((Ulong)gMotorInfo.Frequency <<15) / gBasePar.FullFreq;

    gMotorInfo.Motor_HFreq = ((Ulong)gMotorInfo.Frequency * 410) >>10;
    gMotorInfo.Motor_LFreq = ((Ulong)gMotorInfo.Frequency * 205) >>10;

    // ������һЩ�����ļ��㣬��ʡcpuʱ�俪��
    //gUVWPG.UvwPolesRatio = ((Ulong)gMotorExtInfo.Poles << 8) / gUVWPG.UvwPoles;    // Q8 ���
    gRotorTrans.PolesRatio = ((Ulong)gMotorExtInfo.Poles << 8) / gRotorTrans.Poles; // Q8
}

// // �����в���ת��
void RunStateParChg2Ms()	
{
    //int     temp;
    Uint    m_UData, tempU;
    Ulong   m_Long;    
    Uint    m_AbsFreq;
    //long    mCurM, mCurT;
     Ulong   tmpAmp;
    Uint    m_FluxWeakDepth;
    Uint    m_LimitOutVolt;
    Uint    m_MaxOutVoltReal;
	gIUVWQ12.U = (int)(gIUVWQ24.U>>12);				
	gIUVWQ12.V = (int)(gIUVWQ24.V>>12);
	gIUVWQ12.W = (int)(gIUVWQ24.W>>12);

    gIMTSetQ12.M = (int)(gIMTSetApply.M >> 12);
    gIMTSetQ12.T = (int)(gIMTSetApply.T >> 12);

	m_AbsFreq = abs(gMainCmd.FreqSyn);
    tempU = ((Ulong)gMotorExtPer.I0 * gMotorInfo.FreqPer) / m_AbsFreq;
    gMotorExtPer.IoVsFreq = (m_AbsFreq < gMotorInfo.FreqPer) ? gMotorExtPer.I0 : tempU;
      
    //�����ߵ���
/*    gIAmpTheta.CurTmpM = abs(gIMTQ12.M);
    gIAmpTheta.CurTmpT = abs(gIMTQ12.T);
    tmpAmp = (long)gIAmpTheta.CurTmpM * gIAmpTheta.CurTmpM;
    tmpAmp += (long)gIAmpTheta.CurTmpT * gIAmpTheta.CurTmpT;
    gIAmpTheta.Amp = (Uint)qsqrt((Ulong)tmpAmp);
	*/
	if(gLineCur.CurCnt >= 1024)
	{
        DINT;
	    gLineCur.ImAvr = (int)(gLineCur.ImTotal / (long)gLineCur.CurCnt);
	    gLineCur.ItAvr = (int)(gLineCur.ItTotal / (long)gLineCur.CurCnt);
		gLineCur.ImTotal = 0;
        gLineCur.ItTotal = 0;
        gLineCur.CurCnt = 0;
		EINT;
	}

	tmpAmp = (long)gLineCur.ImAvr * gLineCur.ImAvr;
    tmpAmp += (long)gLineCur.ItAvr * gLineCur.ItAvr;
    gIAmpTheta.Amp = (Uint)qsqrt((Ulong)tmpAmp);//�����ʵ����Is��Is^2=Id^2+Iq^2
    //...................................�����ߵ���
    if((gMainCmd.Command.bit.StartDC == 1) || 
       (gMainCmd.Command.bit.StopDC == 1))	/****ֱ���ƶ�״̬��ʾ�����Ŵ���****/
    {
        gIAmpTheta.Amp = ((Ulong)gIAmpTheta.Amp * 5792)>>12;//5792)>>12;Ϊ   5792��4096=1.414
    }
	gLineCur.CurPer = Filter(gLineCur.CurPer,gIAmpTheta.Amp,1024);   //�˲����Is
	gLineCur.CurPerFilter += gLineCur.CurPer - (gLineCur.CurPerFilter>>7);	

	// �����Ƶ�������Ϊ��ֵ�ı�ôֵ����
	m_Long = (Ulong)gLineCur.CurPer * gMotorInfo.Current;   //gMotorInfo.CurrentΪ��������ѡ�õĵ�����ֵ(���ܺ�ʵ�ʵ����������) �����ֵ��ʵ����ȡ��Ƶ�����͵Ķ�����͵���Ķ�����е�Сֵ
	gLineCur.CurBaseInv = m_Long/gInvInfo.InvCurrForP;  //P�ͻ�ʹ�õĶ����       ���ر����õ���                                         ������ʵ���ǰ����������ĵ���ֵ����Ϊ������ʱ��ıȽ�ֵһ���Ļ�׼��
                                                                                                //CurBaseInv�����ڵ������Ƶ�ISֵ�����IS�Ǿ���ת���ĵ���ֵ�������Ļ�׼�ͻ��͡�����Ķ��������Сֵ��Ϊһ���Ļ�׼
    // ͬ���� ������������                                                      //CurBaseInv=(�˲����IS)*����Ƶ��������͵��������е���Сֵ��/�����͵Ķ������
    gDeadBand.InvCurFilter = Filter2(gLineCur.CurBaseInv, gDeadBand.InvCurFilter);
	m_UData = abs(gMainCmd.FreqSyn);                                   //������ʵ��ֵ���������Ƶ�?
	gMainCmd.FreqReal = ((Ullong)m_UData * gBasePar.FullFreq01 + (1<<14))>>15;
    gMainCmd.FreqRealFilt += (gMainCmd.FreqReal>>4) - (gMainCmd.FreqRealFilt>>4);
    m_UData = abs(gMainCmd.FreqDesired);
    gMainCmd.FreqDesiredReal = ((Ullong)m_UData * gBasePar.FullFreq01 + (1<<14))>>15;

    gMainCmd.FreqSetReal = ((llong)gMainCmd.FreqSet * (llong)gBasePar.FullFreq01 + (1<<14))>>15;
//    gMainCmd.FreqSetBak = gMainCmd.FreqSet;
    
// Ƿѹ��ɸ��ݹ��������
#if (SOFTSERIES == MD380SOFT)
    gInvInfo.InvLowUDC = gInvInfo.LowUdcCoff;
#else
    gInvInfo.InvLowUDC = Max(gInvInfo.LowUdcCoff,2100);
#endif

   
// ����ĸ�ߵ�ѹ������������ѹ
    if(gFluxWeak.Mode == 2)
    {
    	m_MaxOutVoltReal = ((u32)gUDC.uDCBigFilter * 23170L)>>15;

    	m_FluxWeakDepth = gPWM.OverModuleCoff - 100; //�õ�ѹ����ϵ���������ù�����ϵ����ʵ��������ѹ������
        m_FluxWeakDepth = (long)m_FluxWeakDepth * m_FluxWeakDepth * 9>>5;//100 + (gOutVolt.MaxOutVoltCoff - 100) * 5
    	m_FluxWeakDepth = 100 + m_FluxWeakDepth;

        m_LimitOutVolt = ((u32)m_MaxOutVoltReal * (u32)m_FluxWeakDepth)/100;
        gOutVolt.MaxOutVolt = ((u32)gUDC.uDCBigFilter * 232L * (u32)gPWM.OverModuleCoff)>>15;
    	gOutVolt.MaxOutVoltPer = ((u32)m_MaxOutVoltReal<<12)/(gMotorInfo.Votage*10); /*����������*/
    	gOutVolt.LimitOutVoltPer = ((u32)m_LimitOutVolt<<12)/(gMotorInfo.Votage*10); /*���ŵ���������ѹ*/
//		gOutVolt.LimitOutVoltPer1 = ((u32)m_MaxOutVoltReal<<12)/(gMotorInfo.Votage*10);
    }
    else
    {
	    //��������ѹ��Сʱ���п����������Ҫ�ر���	  
		m_UData = gUDC.uDCBigFilter;//Min(gUDC.uDCBigFilter,gInvInfo.BaseUdc);
		gOutVolt.MaxOutVolt = ((u32)m_UData * 232L * (u32)gPWM.OverModuleCoff)>>15; 
		gOutVolt.LimitOutVolt = gOutVolt.MaxOutVolt - ((u32)gOutVolt.MaxOutVolt * (u32)gPmFluxWeak.FluxWeakDepth)/100;
		gOutVolt.MaxOutVoltPer = ((u32)gOutVolt.MaxOutVolt<<12)/(gMotorInfo.Votage*10); /*����������*/
		gOutVolt.LimitOutVoltPer = ((u32)gOutVolt.LimitOutVolt<<12)/(gMotorInfo.Votage*10); /*���ŵ���������ѹ*/	        
//	    gOutVolt.LimitOutVoltPer1 = (s32)gOutVolt.LimitOutVoltPer * 100l/gPWM.OverModuleCoff;
	}		
	gPmFluxWeak.VoltLpf = Filter16(gUAmpTheta.Amp,gPmFluxWeak.VoltLpf);  //�����ѹ�˲�ֵ Q12
//*************************************************************//LJH1917
    if (gMainCmd.Command.bit.Start == 0)    // ͣ��״̬��ֻ����������10s����ֹ�ջ��ͻ��豸
    {
        gBrake.DelayClose ++;
    }
#if (SOFTSERIES == MD380SOFT)           
    gBrake.Ontimetotal++;
	if (gBrake.Ontimeset == 0)
	{
	    gBrake.Ontimetotal = 0;
	}
	if(gBrake.Ontimetotal > ((Ulong)gBrake.Ontimeset * 500))
	{
		gError.ErrorCode.all |= ERROR_BRAKE_DETECT;
	}
#endif
//*************************************************************//
// VF �����Ŵ���
    gVarAvr.CoffApply = gVFPar.VFOverExc;
   
// ͬ������ز������⴦�� 
	if(MOTOR_TYPE_PM == gMotorInfo.MotorType)
	{
        if(0 == gMainCmd.Command.bit.ControlMode)
        {
            if((1 == gMainCmd.Command.bit.StartDC) || (1 == gMainCmd.Command.bit.StopDC))
            {
                gCtrMotorType = DC_CONTROL;
            }
			else if(gCtrMotorType != RUN_SYNC_TUNE)
			{
			    gCtrMotorType = (CONTROL_MOTOR_TYPE_ENUM)(gMainCmd.Command.bit.ControlMode + 10);
			}
        }
        else
        {
            if((1 == gMainCmd.Command.bit.StartDC) || (1 == gMainCmd.Command.bit.StopDC))
            {
                gCtrMotorType = DC_CONTROL;
            }
        }


    
        // ת�������趨�ı��������λ�ý�
        //gIPMPos.RotorZero = ((Ulong)gIPMPos.RotorZeroGet<<16)/3600;
		if((TUNE_STEP_NOW == PM_EST_WITH_LOAD)&&(gPGData.PGType == PG_TYPE_RESOLVER)) 
		{
		    ;
		}
		else
		{
	        gIPMPos.RotorZero = (Ulong)gIPMPos.RotorZeroGet * 18641L >> 10;
		}
        //gUVWPG.UvwZeroPhase = (Ulong)gUVWPG.UvwZeroPhase_deg * 18641L >> 10;
        gUVWPG.UvwZeroPos = gUVWPG.UvwZeroPos_deg * 18641L >> 10;

        gUVWPG.UvwZIntErr_deg = (long)gUVWPG.UvwZIntErr * 180L >>15;
        gIPMPosCheck.UvwStopErr_deg = (long)gIPMPosCheck.UvwStopErr *180L >>15;
        gIPMPos.AbzErrPos_deg = (long)gIPMPos.AbzErrPos * 180L >>15;
        #if 0
        tempU = gUVWPG.UVWAngle + gUVWPG.UvwZeroPos;
        gUVWPG.UvwRealErr_deg = (int)(gIPMPos.RotorPos - tempU) * 180L >>15;
        #endif
        
        // �ֶ��޸����λ�ý�ʱ��Ҫ��Ӧ, ������Զ�����
        if(gPGData.PGMode == 0 && gMainStatus.ParaCalTimes == 1 && // �Ѿ��ϵ�
            gMainStatus.RunStep != STATUS_GET_PAR &&
            gIPMPos.ZeroPosLast != gIPMPos.RotorZero)
        {
            tempU = gIPMPos.RotorZero - gIPMPos.ZeroPosLast;
            
            SetIPMPos(gIPMPos.RotorPos + tempU);
        }   
        gIPMPos.ZeroPosLast = gIPMPos.RotorZero;
        gPmDecoup.EnableDcp = 0;          
	}

    //�����������������
	if(gCtrMotorType != RUN_SYNC_TUNE)
	{
	    EQepRegs->QDECCTL.bit.SWAP = gPGData.SpeedDir;
	}



// ������ѹУ��ϵ��ʵʱ���㣬�������
#ifdef TMS320F2808
    m_Long = ((long)gInvInfo.InvCurrent * 11586L)/gMotorInfo.Current;
#else
	m_Long = ((long)gInvInfo.InvCurrent * 12745L)/gMotorInfo.Current;
#endif
	//gCurSamp.Coff = (m_Long * (long)gInvInfo.CurrentCoff) / 1000;
	gCurSamp.Coff = (m_Long * (Ulong)gInvInfo.CurrentCoff) * 33 >>15;
#ifdef TMS320F2808
    //gUDC.Coff = (Ulong)gUDC.uDCADCoff * gInvInfo.UDCCoff / 1000;
    gUDC.Coff = (Ulong)gUDC.uDCADCoff * gInvInfo.UDCCoff * 33 >>15;
#else   // TMS320F28035
    gUDC.Coff = (Ulong)gUDC.uDCADCoff * gInvInfo.UDCCoff * 36 >>15;     // *1.1
#endif
    gOvUdc.Limit = gVFPar.ovPoint;
}

/***************************************************************
����������򣺴���0.5Msѭ���У����ܴ��ݵĲ�������ɲ���ת�������в���׼���ȹ���
1. update gCtrMotorType;
2. measure speed ;
*************************************************************/
void SystemParChg05Ms()
{    
// ������͸���
    if(gMotorInfo.LastMotorType != gMotorInfo.MotorType)
    {
        gMotorInfo.LastMotorType = gMotorInfo.MotorType;

        gPGData.PGType = PG_TYPE_NULL;     // ��������޸ĺ����³�ʼ��������
    }
    
// PG�����͸���
    if(gPGData.PGType != (PG_TYPE_ENUM_STRUCT)gPGData.PGTypeGetFromFun)
    {
        gPGData.PGType = (PG_TYPE_ENUM_STRUCT)gPGData.PGTypeGetFromFun;
        ReInitForPG();

        gIPMInitPos.Flag = 0;
    }
    
// QEP���ٵĴ��� -- ѡ��QEP
	if(gPGData.QEPIndex != (QEP_INDEX_ENUM_STRUCT)gExtendCmd.bit.QepIndex)
	{
        gPGData.QEPIndex = (QEP_INDEX_ENUM_STRUCT)gExtendCmd.bit.QepIndex;
                
        if(gPGData.QEPIndex == QEP_SELECT_1) // ����PG������
        {
            EQepRegs = (struct EQEP_REGS *)&EQep1Regs;
            EALLOW;
            #ifdef TARGET_GS32
            interrupt_register(INT_EQEP1, &PG_Zero_isr);
            #else
            PieVectTable.EQEP1_INT = &PG_Zero_isr;
            #endif
            #ifdef TARGET_GS32
            interrupt_enable(INT_EQEP1);
            #else
            PieCtrlRegs.PIEIER5.bit.INTx1 = 1;
            #endif
            SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 1;
            EDIS;
        }
        #ifdef TMS320F2808                      // 28035 ֻ��һ��QEP
        if(gPGData.QEPIndex == QEP_SELECT_2)
        {
            EQepRegs = (struct EQEP_REGS *)&EQep2Regs;
            EALLOW;
            #ifdef TARGET_GS32
            interrupt_register(INT_EQEP2, &PG_Zero_isr);
            #else
            PieVectTable.EQEP2_INT = &PG_Zero_isr;
            #endif
            #ifdef TARGET_GS32
            interrupt_enable(INT_EQEP2);
            #else
            PieCtrlRegs.PIEIER5.bit.INTx2 = 1;
            #endif
            SysCtrlRegs.PCLKCR1.bit.EQEP2ENCLK = 1;
            EDIS;
        }
        #endif        
        InitSetQEP();        
    }    

    if(MOTOR_TYPE_PM != gMotorInfo.MotorType ||     // �첽��
        gPGData.PGMode != 0)                        // ����λ�ñ����� -- ����
    {
        EALLOW;
        (*EQepRegs).QEINT.all = 0x0;  //ȡ��QEP��I�ź��ж�
        EDIS;
    }
    else
    {
        EALLOW;
        (*EQepRegs).QEINT.all = 0x0400; //����λ�ñ������Ƿ���Ҫ���ж�?
        EDIS;
    }
}

/*************************************************************
	ͬ��������첽��������任
	
*************************************************************/
void ChangeMotorPar(void)
{
	Uint m_UData,m_BaseL;
	Ulong m_Ulong;
	//Uint m_AbsFreq;
    
	//��л�ֵΪ���迹��?2*pi*���Ƶ��
	m_BaseL = ((Ulong)gMotorInfo.Votage * 3678)/gMotorInfo.Current;
	m_BaseL = ((Ulong)m_BaseL * 5000)/gBasePar.FullFreq01;
    

    m_UData = ((Ulong)gMotorExtInfo.RsPm * (Ulong)gMotorInfo.Current)/gMotorInfo.Votage;	
    gMotorExtPer.Rpm = ((Ulong)m_UData * 18597)>>14;
    
    m_Ulong = (((Ulong)gMotorExtInfo.LD <<15) + m_BaseL) >>1;
    gMotorExtPer.LD = m_Ulong / m_BaseL / 10;                   // ͬ����d����Q13
    m_Ulong = (((Ulong)gMotorExtInfo.LQ <<15) + m_BaseL) >>1;
    gMotorExtPer.LQ = m_Ulong / m_BaseL / 10;                   // ͬ����q����Q13

    // ����ͬ����ת�Ӵ���
    m_Ulong = ((long)gMotorExtInfo.BemfVolt <<12) / (gMotorInfo.Votage *10);      // Q12
    //gMotorExtPer.EMF = (u16)m_Ulong * 10;
    gMotorExtPer.FluxRotor = (m_Ulong << 15) / gMotorInfo.FreqPer;                   // Q12\

    //gMotorExtPer.ItRated = 4096L;
    //gPowerTrq.rpItRated = (1000L<<12) / gMotorExtPer.ItRated;
    gPowerTrq.rpItRated = 1000;


	//....������������
	m_Ulong = (((Ullong)gMotorInfo.Frequency * (Ullong)gMainCmd.pu2siCoeff * 19661L)>>15);
	gMotorExtInfo.Poles = (m_Ulong + (gMotorExtInfo.Rpm>>1)) / gMotorExtInfo.Rpm;

    //0.01HzΪ��λ�Ķת����
    //m_Ulong = ((Ulong)gMotorExtInfo.Rpm * gMotorExtInfo.Poles * 100L)/60;
    m_Ulong = gMotorExtInfo.Rpm * gMotorExtInfo.Poles * 6830L >> 12;
    gMotorExtInfo.RatedComp = (Ulong)gMotorInfo.Frequency * gMainCmd.pu2siCoeff - m_Ulong;
                              
    //��ô���Ķת����       
    gMotorExtPer.RatedComp = ((long)gMotorExtInfo.RatedComp << 15)/gBasePar.FullFreq01; 
}

// ��Ƶ��������ʡ�ת�ؼ���
void InvCalcPower(void)
{
    long m_PowerN,m_s32;
	static Uint m_CntTime = 0;
   
    gPowerTrq.angleFilter = Filter64(gIAmpTheta.PowerAngle, gPowerTrq.angleFilter);     //�˲��Ӵ󣬴�16��Ϊ64
    gPowerTrq.angleFilter = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.angleFilter : 0; 
    gPowerTrq.anglePF = abs(gPowerTrq.angleFilter);
	gRotorSpeed.SpeedBigFilter = Filter(gRotorSpeed.SpeedBigFilter,gRotorSpeed.SpeedApply,1024);
    
	//gPowerTrq.Cur_Ft4 = Filter32(gLineCur.CurPer, gPowerTrq.Cur_Ft4);
    //gPowerTrq.Volt_Ft4 = Filter64(gOutVolt.VoltApply,gPowerTrq.Volt_Ft4);  // �������ѹ�����˲���ʹ��ʾ�Ĺ��ʸ�ƽ��
    gPowerTrq.Cur_Ft4 = Filter(gPowerTrq.Cur_Ft4,gLineCur.CurPer,1024);
	gPowerTrq.Volt_Ft4 = Filter(gPowerTrq.Volt_Ft4,gOutVolt.VoltApply,1024);	
	gPowerTrq.InvPowerPU = (1732L * (long)gPowerTrq.Volt_Ft4 /1000L) * gPowerTrq.Cur_Ft4 >> 12;
    gPowerTrq.InvPowerPU = (long)gPowerTrq.InvPowerPU * qsin(16384 - gPowerTrq.anglePF) >> 15;

	m_PowerN = ((long)gMotorInfo.Current * gMotorInfo.Votage) / 1000;
    //m_PowerN = ((long)gMotorInfo.Current * gMotorInfo.Votage) >> 10;
	if(gInvInfo.InvVoltageType == INV_VOLTAGE_220V)   //220v����37Kw�л�С����
	{
	    m_PowerN = (gInvInfo.InvType >= 19) ? m_PowerN : (m_PowerN*409)>>12;
	}
	else
	{
        m_PowerN = (gInvInfo.InvType >= 22) ? m_PowerN : (m_PowerN*409)>>12;   // 0.01->0.1
    }
	gPowerTrq.InvPower_si= ((long)gPowerTrq.InvPowerPU * m_PowerN) >> 12;     	

    gPowerTrq.InvPower_si = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.InvPower_si : 0;
	//gPowerTrq.InvPower_E  = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.InvPower_E  : 0;
//    gPowerTrq.InvPower_si =  __IQsat(gPowerTrq.InvPower_si,(s16)gMotorInfo.Power,-((s16)gMotorInfo.Power));    
    
    gPowerTrq.TrqOut_pu = (long)gLineCur.CurTorque * gPowerTrq.rpItRated >> 12;
    //gPowerTrq.TrqOutHoAndFo_pu = (gIMTSetApply.T>>12) * (long)gMotorInfo.Current /(long)gInvInfo.InvCurrForP * 1000L >> 12;	//�������ӿ��Ƶ�ת�ص���
    gPowerTrq.TrqOutHoAndFo_pu = (gIMTSetApply.T>>12) *  1000L >> 12;
    gPowerTrq.TrqOut_pu = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.TrqOut_pu : 0;       // ͣ��״̬���⴦��
    gPowerTrq.TrqOutHoAndFo_pu = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.TrqOutHoAndFo_pu : 0;       // ͣ��״̬���⴦�� 
			
	if(gPowerTrq.InvPower_si < (gMotorInfo.Power>>2))          //������ڶ����1/4�ż���
    {
        if(gInvInfo.InvType >= 22)
		{
		    gEstBemf.BemfVoltFilter = (Uint)((Ulong)gMotorInfo.Power*6060UL/gMotorInfo.Current);
		}
		else
		{
            gEstBemf.BemfVoltFilter = (Uint)((Ulong)gMotorInfo.Power*60600UL/gMotorInfo.Current);
		}
		gEstBemf.BemfVoltFilter = __IQsat(gEstBemf.BemfVoltFilter,3600,3000);
    }
	else if(gPowerTrq.InvPower_si > ((gMotorInfo.Power * 5)>>4)&&((gIMTSet.M>>12)>-500L))  //���ڶ����1/3��û�к��������
	{			
        m_s32 = ((s32)gPowerTrq.Volt_Ft4 * gMotorInfo.Votage * 10L) >> 12;   
        m_s32 = m_s32 * qsin(16384 - gPowerTrq.anglePF) >> 15;
		gEstBemf.BemfVoltOnline = abs(m_s32 * gMotorInfo.FreqPer / gRotorSpeed.SpeedBigFilter);
	    gEstBemf.BemfVoltFilter = Filter32(gEstBemf.BemfVoltOnline , gEstBemf.BemfVoltFilter);
        gEstBemf.BemfVoltFilter = __IQsat(gEstBemf.BemfVoltFilter,4000,2500);	
	}
    m_CntTime++;
	if(m_CntTime > 4096)   // ÿ8s����һ��
    {
        m_CntTime = 0;				
        gEstBemf.BemfVoltDisPlay = gEstBemf.BemfVoltDisPlayTotal>>12; 	         
        if(gEstBemf.BemfVoltDisPlay >= gEstBemf.BemfVoltDisPlayLast + 20)
		{
		    if(gEstBemf.BemfVoltDisPlay >= gEstBemf.BemfVoltDisPlayLast + 50)    //�տ�ʼ�����ȽϿ�
			{
			    gEstBemf.BemfVoltDisPlayLast += 50;
			}
			else
			{
		        gEstBemf.BemfVoltDisPlayLast += 20;
			}
		}
		else if(gEstBemf.BemfVoltDisPlay <= gEstBemf.BemfVoltDisPlayLast - 20)
		{
		    if(gEstBemf.BemfVoltDisPlay <= gEstBemf.BemfVoltDisPlayLast - 50)
			{
			    gEstBemf.BemfVoltDisPlayLast -= 50;
			}
			else
			{
		        gEstBemf.BemfVoltDisPlayLast -= 20;
			}
		}			  	
		gEstBemf.BemfVoltDisPlayTotal = 0;
	}
	else
	{
	    gEstBemf.BemfVoltDisPlayTotal += gEstBemf.BemfVoltFilter;
	}	
	if((gMainCmd.Command.bit.ControlMode == IDC_SVC_CTL)&&(gUVCoff.OnlineTuneBemfVoltEnable == 1)
       &&(INV_VOLTAGE_380V == gInvInfo.InvVoltageType))   //svcʱ���ڴų�����
	{
		m_s32 = ((s32)gEstBemf.BemfVoltDisPlayLast<<10)/gMotorInfo.Votage;
	    m_s32 = (m_s32 * (s32)gBasePar.FullFreq)/((s32)gMotorInfo.Frequency * 10);
		gPmsmRotorPosEst.InvOfKf = 1048576L/m_s32; 
	}
}


