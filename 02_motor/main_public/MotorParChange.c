/****************************************************************
文件功能：处理从功能部分获得的运行参数
文件版本： 
更新日期： 

****************************************************************/
#include "MotorInclude.h"

extern PMSM_FLUX_WEAK_STRUCT   gPmFluxWeak;
/***********************************************************************
变频器制动额定电流表:
两位小数点，只有以下几个机型确认过
功率   机型   额定电流 	max(10s)(对应1.5V)
18.5    16    9.49A   		32.5
22      17    11.29A		32.5
30      18    15.39A		40.63
37      19    18.98A		52.71
45      20    23.08A		60.94
55      21    28.16A		81.25
75      22    38.43A		114.71 
目前故障设置:
超过额定电流10S报过载ERROR_OVERLOAD_BRAKE  ERR61 故障信息1
超过MAX电流直接报过载ERROR_OVERLOAD_BRAKE  ERR61 故障信息2
硬件采样标定按照最大电流计算，1.5V对应最大电流
没有开通制动管但是有电流报制动管直通ERROR_SHORT_BRAKE ERR62
************************************************************************/
#if (SOFTSERIES == MD500SOFT)
Uint const gInvBreakCurrentTable380T[27] =         // 380V,480V
{
    949,  949,  949,  949, 949,	     //8~12  
    949,  949,  949,  3055,3055,         //13~17
    3250,  4111, 5545, 7393,          //18~21  
    
    997,  384,  384,  384, 384,         //22~26  以下电流数据包含一个小数点
    384,  384,  384,  384, 384,         //27~31
    384,  384,  384                       //32~34
};
Uint const gInvBreakMaxCurrentTable380T[27] =         // 380V,480V
{
    3250,  3250,  3250,  3250, 3250,	     //8~12  
    3250,  3250,  3250,  3250,3250,       //13~17
    4063,  5271,  6094,  8125,      //18~21            
    
    1147, 1147, 1147, 1147, 1147,        //22~26  以下电流数据包含一个小数点
    1147, 1147, 1147, 1147, 1147,        //27~31
    1147 ,1147 ,1147                       //32~34
};
Uint const gInvBreakOverLoadTableFor185And22[10] = 
{
	1,//0,						//	120%  0.5s过载
	2,//0.2,						//	110%  0.2s过载
    100,//10,                 		// 	100%最大电流 10s报过载   
    500,//50,                 		// 	99%最大电流 50s报过载    
    820,//82,                	 	// 	98%最大电流 82s报过载
    1380,//138,                 	// 	97%最大电流 138s报过载
    2450,//245,                   	// 	96%最大电流 245s报过载
    5000,//500,					// 	95%最大电流 500s报过载
    10000,//1000					// 
    10000,//1000					// 
};
Uint const gInvBreakOverLoadTableFor30[10] = 
{
	13,// 1.3,					//	120%  1.3s过载
	50,// 5,						//	110%  5s过载
    100,// 10,                 	//	100%最大电流 10s报过载   
    160,// 16,                 	// 	98% 最大电流 16s报过载    
    230,//23,                 		// 	96% 最大电流 23s报过载
    300,//30,                 		// 	94% 最大电流 30s报过载
    440,//44,                   	// 	92% 最大电流 44s报过载
    580,//58,					// 	90% 最大电流 58s报过载
    2870,//287					// 	85% 最大电流 278s报过载
    5000,//500					//   
};
Uint const gInvBreakOverLoadTableFor37[10] = 
{
	25,// 2.5,					//	120%  2.5s过载
	75,// 7.5,					//	110%  7.5s过载
    100,// 10,                 	// 	100% 最大电流 10s报过载   
    180,// 18,                 	// 	98% 最大电流 18s报过载    
    250,// 25,                 	// 	96% 最大电流 25s报过载
    300,// 30,                 	// 	94% 最大电流 30s报过载
    360,// 36,                   	// 	92% 最大电流 36报过载
	480,// 48,					// 	90% 最大电流 48s报过载
	1000,//	100,					// 	90% 最大电流 48s报过载
	3400,//	340,					// 	90% 最大电流 48s报过载
};

Uint const gInvBreakOverLoadTableFor45And55[10] = 
{
	1,//0,						//	120% 最大电流 0s报过载  
	10,// 1,						//	110% 最大电流 1s报过载  
    100,// 10,                 	// 	100% 最大电流 10s报过载   
    290,// 29,                 	// 	98%  最大电流 29s报过载    
    580,// 58,                 	// 	96%  最大电流 58s报过载
    900,// 90,                 	// 	94%  最大电流 90s报过载
    1500,// 150,            		// 	92%  最大电流 150s报过载
    3000,//300					//
    5000,//500					//
    10000,//1000					//
};
Uint const gInvBreakOverLoadTableFor75[10] = 
{
	5,// 0.5,					//	120% 最大电流 0.5s过载
	30,// 3,						//	110% 最大电流 3s过载
    100,// 10,                 	// 	100%	最大电流 10s报过载   
    180,// 18,                 	// 	98% 	最大电流 18s报过载    
    270,// 27,                 	// 	96% 	最大电流 27s报过载
    390,// 39,                 	// 	94% 	最大电流 39s报过载
    530,// 53,                   	// 	92% 	最大电流 53s报过载
	1000,// 100,					// 	90% 	最大电流 100s报过载
	5000,//500					//
	10000,//1000					//
};
#endif
// // 停机参数转换
void SystemParChg2Ms()	
{
	Ulong m_ULong;
    Uint    minType, maxType;
    Uint    *pInvVolt, *pInvCur;
    Uint 	 m_UData,m_Pointer;
#if (SOFTSERIES == MD500SOFT)
	Uint *pInvBreakCur,*pInvBreakMaxCur;
#endif
    
// 获取变频器系统参数
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
	else if(200 > gInvInfo.InvTypeSet)      // 220T, 没有P型机
	{
	    pInvVolt  = (Uint *)gInvVoltageInfo220T;//使用380V变频器的额定电流，也就是被改制的机型
        pInvCur   = (Uint *)gInvCurrentTable220T;
#if (SOFTSERIES == MD500SOFT)
        pInvBreakCur = (Uint *)gInvBreakCurrentTable380T;
		pInvBreakMaxCur = (Uint *)gInvBreakMaxCurrentTable380T;
#endif
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 100;
        gInvInfo.InvVoltageType = INV_VOLTAGE_220V;
        gInvInfo.GPType   = 1;                                      //不允许P型机

        //****************************** 380V改制为220V机型温度曲线处理  
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
#if (SOFTSERIES == MD500SOFT)  //用380V的      
		pInvVolt  = (Uint *)gInvVoltageInfo480T;	//表与380V一样
        pInvCur = (Uint *)gInvCurrentTable380T;  // 与380V共用电流表
        pInvBreakCur = (Uint *)gInvBreakCurrentTable380T;// 与380V共用电流表
		pInvBreakMaxCur = (Uint *)gInvBreakMaxCurrentTable380T;// 与380V共用电流表
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 300;
        gInvInfo.InvVoltageType = INV_VOLTAGE_380V;//MD500的480V当380V
        gInvInfo.GPType = gInvInfo.GpTypeSet;
#else
		pInvVolt  = (Uint *)gInvVoltageInfo480T;
        pInvCur = (Uint *)gInvCurrentTable380T;  // 与380V共用电流表
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
    minType              = *(pInvVolt + 4);  //变频器允许的机型下限
    maxType              = *(pInvVolt + 5);  //变频器允许的机型上限
    gInLose.ForeInvType  = *(pInvVolt + 6);  //开始缺相保护的起始机型
    gUDC.uDCADCoff       = *(pInvVolt + 7); 
    gVFPar.ovPointCoff   = *(pInvVolt + 8);     // 用于计算过压抑制点

	gInvInfo.InvType = (gInvInfo.InvType > maxType) ? maxType : gInvInfo.InvType;
	if(gInvInfo.InvType <= minType)
    {
        gInvInfo.GPType = 1;	                        //最小机型无法区分GP
        gInvInfo.InvType = minType;
	}
#if (SOFTSERIES == MD500SOFT)
    if(gInvInfo.GPType == 1)   
	{
	    gInvInfo.InvTypeApply = gInvInfo.InvType;
	}
    else if(gInvInfo.InvType < 29)    // 220Kw P及以下
    {
        gInvInfo.InvTypeApply = gInvInfo.InvType - 1;   //P型机采样电流低一挡
    }
	else if(gInvInfo.InvType >= 29)   // 250Kw p及以上
	{
	    gInvInfo.InvTypeApply = gInvInfo.InvType - 2;   //大功率P型机采样电流低两挡
	}
    if((gInvInfo.InvType == 34) && (gInvInfo.GPType == 1))                          // 450G模块与400G一样，电流采样电流不同，保护按照450P进行保护
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
        gInvInfo.InvTypeApply = gInvInfo.InvType - 1;   //P型机采样电流低一挡
    }
#endif	
	gInvInfo.InvCurrent  = *(pInvCur + gInvInfo.InvTypeApply - minType);        //电流采样使用的额定电流
    //gInvInfo.InvCurrentOvload = *(pInvCur + gInvInfo.InvType - minType);
#if (SOFTSERIES == MD500SOFT)
	if((2 == gInvInfo.GPType) && (22 == gInvInfo.InvType))  // MD500 30P与75P电流与G型机不同
	{
		gInvInfo.InvCurrForP = 1500; //MD500的75P额定电流为150A
	}
	else if((2 == gInvInfo.GPType) && (18 == gInvInfo.InvType))
	{
        gInvInfo.InvCurrForP = 6000;
    }
	else
	{
    	gInvInfo.InvCurrForP = *(pInvCur + gInvInfo.InvType - minType);      //P型机使用的额定电流
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
    gInvInfo.InvCurrForP = *(pInvCur + gInvInfo.InvType - minType);      //P型机使用的额定电流
#endif
    if(gInvInfo.InvVoltageType == INV_VOLTAGE_220V)                             // 电流小数点的确定
    {   
        gMotorExtInfo.UnitCoff = (gInvInfo.InvType > 18) ? 10 : 1;
    }
    else
    {
        gMotorExtInfo.UnitCoff = (gInvInfo.InvType > 21) ? 10 : 1; 
    }
    
	if((1 != gInvInfo.GPType) && (22 == gInvInfo.InvType))
	{
        // 机型为22的P型机，电流应该是1位小数点，但是读取的采样电流是2位  
		gInvInfo.InvCurrent = (Ulong)gInvInfo.InvCurrent * 3264 >> 15;
    }
    //gInvInfo.InvCurrForP = *(pInvCur + gInvInfo.InvType - minType);      //P型机使用的额定电流

//死区和死区补偿参数
	m_UData = (gInvInfo.InvTypeApply > 29) ? 29 : gInvInfo.InvTypeApply;
	m_UData = (m_UData < 12) ? 12 : m_UData;
#if (SOFTSERIES == MD380SOFT)
    if(500 <= gInvInfo.InvTypeSet)                //1140V死区和死区补偿量确定 2011.5.8 L1082
    {
        gDeadBand.DeadBand = DBTIME_1140V*gDeadBand.DeadTimeSet/10;          
        gDeadBand.Comp     = DCTIME_1140V*gDeadBand.DeadTimeSet/10;
    }
    else if(400 <= gInvInfo.InvTypeSet)                       //690V死区补偿和补偿量固定
    {
	    gDeadBand.DeadBand = gDeadBandTable[13];         //死区固定4.8us，补偿量2.5us
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
	EALLOW;									//设置死区时间
	EPwm1Regs.DBFED = gDeadBand.DeadBand;
	EPwm1Regs.DBRED = gDeadBand.DeadBand;
	EPwm2Regs.DBFED = gDeadBand.DeadBand;
	EPwm2Regs.DBRED = gDeadBand.DeadBand;
	EPwm3Regs.DBFED = gDeadBand.DeadBand;
	EPwm3Regs.DBRED = gDeadBand.DeadBand;
	EDIS;
        
//启动ADC采样延迟时间
 /*   #ifdef		DSP_CLOCK100
	gADC.DelayApply = gADC.DelaySet * 10;       // default: 0.5us
    #else
	gADC.DelayApply = gADC.DelaySet * 6;
    #endif
*/
//计算电流系数和电压系数
	//电机电流太小的处理
	m_UData = gInvInfo.InvCurrent>>2;
	DINT;
	gMotorInfo.Current = (gMotorInfo.CurrentGet < m_UData) ? m_UData : gMotorInfo.CurrentGet;
	m_ULong = (((Ulong)gMotorInfo.Current)<<8) / gMotorInfo.CurrentGet;
	gMotorInfo.CurBaseCoff = (m_ULong > 32767) ? 32767 : m_ULong;
	EINT;
    
	//AD值32767对应变频器额定电流值的2倍 转换为电机额定电流的标么值表示(Q24)
	//AD转换值切换为电机标么值表示值的方法为: (AD/32767 * 2sqrt(2) * Iv/Im) << 24 *8
	// (1/32767 * 2sqrt(2) << 24) * 8 == 11586
	// CPU28035时，32767对应的电流值为: 2sqrt(2) *3.3/3.0 * Iv
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

//计算不同分辨率的频率表示
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
	gBasePar.FullFreq01 = (Ulong)gBasePar.MaxFreq * (Ulong)gMainCmd.pu2siCoeff + 2000;	//32767表示的频率值
	gBasePar.FullFreq =   gBasePar.MaxFreq + 20 * gMainCmd.si2puCoeff;	//频率基值
	gMotorInfo.FreqPer =  ((Ulong)gMotorInfo.Frequency <<15) / gBasePar.FullFreq;

    gMotorInfo.Motor_HFreq = ((Ulong)gMotorInfo.Frequency * 410) >>10;
    gMotorInfo.Motor_LFreq = ((Ulong)gMotorInfo.Frequency * 205) >>10;

    // 编码器一些常量的计算，节省cpu时间开销
    //gUVWPG.UvwPolesRatio = ((Ulong)gMotorExtInfo.Poles << 8) / gUVWPG.UvwPoles;    // Q8 电机
    gRotorTrans.PolesRatio = ((Ulong)gMotorExtInfo.Poles << 8) / gRotorTrans.Poles; // Q8
}

// // 运行中参数转换
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
      
    //计算线电流
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
    gIAmpTheta.Amp = (Uint)qsqrt((Ulong)tmpAmp);//这个其实就是Is，Is^2=Id^2+Iq^2
    //...................................计算线电流
    if((gMainCmd.Command.bit.StartDC == 1) || 
       (gMainCmd.Command.bit.StopDC == 1))	/****直流制动状态表示电流放大处理****/
    {
        gIAmpTheta.Amp = ((Ulong)gIAmpTheta.Amp * 5792)>>12;//5792)>>12;为   5792、4096=1.414
    }
	gLineCur.CurPer = Filter(gLineCur.CurPer,gIAmpTheta.Amp,1024);   //滤波后的Is
	gLineCur.CurPerFilter += gLineCur.CurPer - (gLineCur.CurPerFilter>>7);	

	// 计算变频器额定电流为基值的标么值电流
	m_Long = (Ulong)gLineCur.CurPer * gMotorInfo.Current;   //gMotorInfo.Current为驱动程序选用的电流基值(可能和实际电机电流不等) ，这个值其实就是取变频器机型的额定电流和电机的额定电流中的小值
	gLineCur.CurBaseInv = m_Long/gInvInfo.InvCurrForP;  //P型机使用的额定电流       过载保护用电流                                         这里其实就是把用于限流的电流值调整为和限流时候的比较值一样的基准，
                                                                                                //CurBaseInv是用于电流限制的IS值，这个IS是经过转换的电流值，把它的基准和机型、电机的额定电流中最小值定为一样的基准
    // 同步机 用于死区补偿                                                      //CurBaseInv=(滤波后的IS)*（变频器额定电流和电机额定电流中的最小值）/（机型的额定电流）
    gDeadBand.InvCurFilter = Filter2(gLineCur.CurBaseInv, gDeadBand.InvCurFilter);
	m_UData = abs(gMainCmd.FreqSyn);                                   //计算用实际值表镜脑诵衅德?
	gMainCmd.FreqReal = ((Ullong)m_UData * gBasePar.FullFreq01 + (1<<14))>>15;
    gMainCmd.FreqRealFilt += (gMainCmd.FreqReal>>4) - (gMainCmd.FreqRealFilt>>4);
    m_UData = abs(gMainCmd.FreqDesired);
    gMainCmd.FreqDesiredReal = ((Ullong)m_UData * gBasePar.FullFreq01 + (1<<14))>>15;

    gMainCmd.FreqSetReal = ((llong)gMainCmd.FreqSet * (llong)gBasePar.FullFreq01 + (1<<14))>>15;
//    gMainCmd.FreqSetBak = gMainCmd.FreqSet;
    
// 欠压点可根据功能码调整
#if (SOFTSERIES == MD380SOFT)
    gInvInfo.InvLowUDC = gInvInfo.LowUdcCoff;
#else
    gInvInfo.InvLowUDC = Max(gInvInfo.LowUdcCoff,2100);
#endif

   
// 根据母线电压计算最大输出电压
    if(gFluxWeak.Mode == 2)
    {
    	m_MaxOutVoltReal = ((u32)gUDC.uDCBigFilter * 23170L)>>15;

    	m_FluxWeakDepth = gPWM.OverModuleCoff - 100; //该电压提升系数根据设置过调制系数与实际提升电压反算获得
        m_FluxWeakDepth = (long)m_FluxWeakDepth * m_FluxWeakDepth * 9>>5;//100 + (gOutVolt.MaxOutVoltCoff - 100) * 5
    	m_FluxWeakDepth = 100 + m_FluxWeakDepth;

        m_LimitOutVolt = ((u32)m_MaxOutVoltReal * (u32)m_FluxWeakDepth)/100;
        gOutVolt.MaxOutVolt = ((u32)gUDC.uDCBigFilter * 232L * (u32)gPWM.OverModuleCoff)>>15;
    	gOutVolt.MaxOutVoltPer = ((u32)m_MaxOutVoltReal<<12)/(gMotorInfo.Votage*10); /*电流环上限*/
    	gOutVolt.LimitOutVoltPer = ((u32)m_LimitOutVolt<<12)/(gMotorInfo.Votage*10); /*励磁电流调整电压*/
//		gOutVolt.LimitOutVoltPer1 = ((u32)m_MaxOutVoltReal<<12)/(gMotorInfo.Votage*10);
    }
    else
    {
	    //当电机额定电压很小时，有可能溢出，需要特别处理	  
		m_UData = gUDC.uDCBigFilter;//Min(gUDC.uDCBigFilter,gInvInfo.BaseUdc);
		gOutVolt.MaxOutVolt = ((u32)m_UData * 232L * (u32)gPWM.OverModuleCoff)>>15; 
		gOutVolt.LimitOutVolt = gOutVolt.MaxOutVolt - ((u32)gOutVolt.MaxOutVolt * (u32)gPmFluxWeak.FluxWeakDepth)/100;
		gOutVolt.MaxOutVoltPer = ((u32)gOutVolt.MaxOutVolt<<12)/(gMotorInfo.Votage*10); /*电流环上限*/
		gOutVolt.LimitOutVoltPer = ((u32)gOutVolt.LimitOutVolt<<12)/(gMotorInfo.Votage*10); /*励磁电流调整电压*/	        
//	    gOutVolt.LimitOutVoltPer1 = (s32)gOutVolt.LimitOutVoltPer * 100l/gPWM.OverModuleCoff;
	}		
	gPmFluxWeak.VoltLpf = Filter16(gUAmpTheta.Amp,gPmFluxWeak.VoltLpf);  //输出电压滤波值 Q12
//*************************************************************//LJH1917
    if (gMainCmd.Command.bit.Start == 0)    // 停机状态下只允许不超过10s，防止烧坏客户设备
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
// VF 过励磁处理
    gVarAvr.CoffApply = gVFPar.VFOverExc;
   
// 同步机相关参数特殊处理 
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


    
        // 转换功能设定的编码器零点位置角
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
        
        // 手动修改零点位置角时需要响应, 旋变会自动计算
        if(gPGData.PGMode == 0 && gMainStatus.ParaCalTimes == 1 && // 已经上电
            gMainStatus.RunStep != STATUS_GET_PAR &&
            gIPMPos.ZeroPosLast != gIPMPos.RotorZero)
        {
            tempU = gIPMPos.RotorZero - gIPMPos.ZeroPosLast;
            
            SetIPMPos(gIPMPos.RotorPos + tempU);
        }   
        gIPMPos.ZeroPosLast = gIPMPos.RotorZero;
        gPmDecoup.EnableDcp = 0;          
	}

    //编码器反馈方向矫正
	if(gCtrMotorType != RUN_SYNC_TUNE)
	{
	    EQepRegs->QDECCTL.bit.SWAP = gPGData.SpeedDir;
	}



// 电流电压校正系数实时计算，方便调试
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
参数计算程序：处理0.5Ms循环中，功能传递的参数，完成参数转换、运行参数准备等工作
1. update gCtrMotorType;
2. measure speed ;
*************************************************************/
void SystemParChg05Ms()
{    
// 电机类型更改
    if(gMotorInfo.LastMotorType != gMotorInfo.MotorType)
    {
        gMotorInfo.LastMotorType = gMotorInfo.MotorType;

        gPGData.PGType = PG_TYPE_NULL;     // 电机类型修改后，重新初始化编码器
    }
    
// PG卡类型更改
    if(gPGData.PGType != (PG_TYPE_ENUM_STRUCT)gPGData.PGTypeGetFromFun)
    {
        gPGData.PGType = (PG_TYPE_ENUM_STRUCT)gPGData.PGTypeGetFromFun;
        ReInitForPG();

        gIPMInitPos.Flag = 0;
    }
    
// QEP测速的处理 -- 选择QEP
	if(gPGData.QEPIndex != (QEP_INDEX_ENUM_STRUCT)gExtendCmd.bit.QepIndex)
	{
        gPGData.QEPIndex = (QEP_INDEX_ENUM_STRUCT)gExtendCmd.bit.QepIndex;
                
        if(gPGData.QEPIndex == QEP_SELECT_1) // 本地PG卡测速
        {
            EQepRegs = (struct EQEP_REGS *)&EQep1Regs;
            EALLOW;
            PieVectTable.EQEP1_INT = &PG_Zero_isr;
            PieCtrlRegs.PIEIER5.bit.INTx1 = 1;
            SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 1;
            EDIS;
        }
        #ifdef TMS320F2808                      // 28035 只有一个QEP
        if(gPGData.QEPIndex == QEP_SELECT_2)
        {
            EQepRegs = (struct EQEP_REGS *)&EQep2Regs;
            EALLOW;
            PieVectTable.EQEP2_INT = &PG_Zero_isr;
            PieCtrlRegs.PIEIER5.bit.INTx2 = 1;
            SysCtrlRegs.PCLKCR1.bit.EQEP2ENCLK = 1;
            EDIS;
        }
        #endif        
        InitSetQEP();        
    }    

    if(MOTOR_TYPE_PM != gMotorInfo.MotorType ||     // 异步机
        gPGData.PGMode != 0)                        // 绝对位置编码器 -- 旋变
    {
        EALLOW;
        (*EQepRegs).QEINT.all = 0x0;  //取消QEP的I信号中断
        EDIS;
    }
    else
    {
        EALLOW;
        (*EQepRegs).QEINT.all = 0x0400; //绝对位置编码器是否需要该中断?
        EDIS;
    }
}

/*************************************************************
	同步电机、异步电机参数变换
	
*************************************************************/
void ChangeMotorPar(void)
{
	Uint m_UData,m_BaseL;
	Ulong m_Ulong;
	//Uint m_AbsFreq;
    
	//电感基值为：阻抗基?2*pi*最大频率
	m_BaseL = ((Ulong)gMotorInfo.Votage * 3678)/gMotorInfo.Current;
	m_BaseL = ((Ulong)m_BaseL * 5000)/gBasePar.FullFreq01;
    

    m_UData = ((Ulong)gMotorExtInfo.RsPm * (Ulong)gMotorInfo.Current)/gMotorInfo.Votage;	
    gMotorExtPer.Rpm = ((Ulong)m_UData * 18597)>>14;
    
    m_Ulong = (((Ulong)gMotorExtInfo.LD <<15) + m_BaseL) >>1;
    gMotorExtPer.LD = m_Ulong / m_BaseL / 10;                   // 同步机d轴电感Q13
    m_Ulong = (((Ulong)gMotorExtInfo.LQ <<15) + m_BaseL) >>1;
    gMotorExtPer.LQ = m_Ulong / m_BaseL / 10;                   // 同步机q轴电感Q13

    // 计算同步机转子磁链
    m_Ulong = ((long)gMotorExtInfo.BemfVolt <<12) / (gMotorInfo.Votage *10);      // Q12
    //gMotorExtPer.EMF = (u16)m_Ulong * 10;
    gMotorExtPer.FluxRotor = (m_Ulong << 15) / gMotorInfo.FreqPer;                   // Q12\

    //gMotorExtPer.ItRated = 4096L;
    //gPowerTrq.rpItRated = (1000L<<12) / gMotorExtPer.ItRated;
    gPowerTrq.rpItRated = 1000;


	//....计算电机极对数
	m_Ulong = (((Ullong)gMotorInfo.Frequency * (Ullong)gMainCmd.pu2siCoeff * 19661L)>>15);
	gMotorExtInfo.Poles = (m_Ulong + (gMotorExtInfo.Rpm>>1)) / gMotorExtInfo.Rpm;

    //0.01Hz为单位的额定转差率
    //m_Ulong = ((Ulong)gMotorExtInfo.Rpm * gMotorExtInfo.Poles * 100L)/60;
    m_Ulong = gMotorExtInfo.Rpm * gMotorExtInfo.Poles * 6830L >> 12;
    gMotorExtInfo.RatedComp = (Ulong)gMotorInfo.Frequency * gMainCmd.pu2siCoeff - m_Ulong;
                              
    //标么化的额定转差率       
    gMotorExtPer.RatedComp = ((long)gMotorExtInfo.RatedComp << 15)/gBasePar.FullFreq01; 
}

// 变频器输出功率、转矩计算
void InvCalcPower(void)
{
    long m_PowerN,m_s32;
	static Uint m_CntTime = 0;
   
    gPowerTrq.angleFilter = Filter64(gIAmpTheta.PowerAngle, gPowerTrq.angleFilter);     //滤波加大，从16改为64
    gPowerTrq.angleFilter = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.angleFilter : 0; 
    gPowerTrq.anglePF = abs(gPowerTrq.angleFilter);
	gRotorSpeed.SpeedBigFilter = Filter(gRotorSpeed.SpeedBigFilter,gRotorSpeed.SpeedApply,1024);
    
	//gPowerTrq.Cur_Ft4 = Filter32(gLineCur.CurPer, gPowerTrq.Cur_Ft4);
    //gPowerTrq.Volt_Ft4 = Filter64(gOutVolt.VoltApply,gPowerTrq.Volt_Ft4);  // 对输出电压进行滤波，使显示的功率更平稳
    gPowerTrq.Cur_Ft4 = Filter(gPowerTrq.Cur_Ft4,gLineCur.CurPer,1024);
	gPowerTrq.Volt_Ft4 = Filter(gPowerTrq.Volt_Ft4,gOutVolt.VoltApply,1024);	
	gPowerTrq.InvPowerPU = (1732L * (long)gPowerTrq.Volt_Ft4 /1000L) * gPowerTrq.Cur_Ft4 >> 12;
    gPowerTrq.InvPowerPU = (long)gPowerTrq.InvPowerPU * qsin(16384 - gPowerTrq.anglePF) >> 15;

	m_PowerN = ((long)gMotorInfo.Current * gMotorInfo.Votage) / 1000;
    //m_PowerN = ((long)gMotorInfo.Current * gMotorInfo.Votage) >> 10;
	if(gInvInfo.InvVoltageType == INV_VOLTAGE_220V)   //220v机型37Kw切换小数点
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
    //gPowerTrq.TrqOutHoAndFo_pu = (gIMTSetApply.T>>12) * (long)gMotorInfo.Current /(long)gInvInfo.InvCurrForP * 1000L >> 12;	//用于主从控制的转矩电流
    gPowerTrq.TrqOutHoAndFo_pu = (gIMTSetApply.T>>12) *  1000L >> 12;
    gPowerTrq.TrqOut_pu = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.TrqOut_pu : 0;       // 停机状态特殊处理
    gPowerTrq.TrqOutHoAndFo_pu = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.TrqOutHoAndFo_pu : 0;       // 停机状态特殊处理 
			
	if(gPowerTrq.InvPower_si < (gMotorInfo.Power>>2))          //必须大于额定功率1/4才计算
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
	else if(gPowerTrq.InvPower_si > ((gMotorInfo.Power * 5)>>4)&&((gIMTSet.M>>12)>-500L))  //大于额定功率1/3且没有很深的弱磁
	{			
        m_s32 = ((s32)gPowerTrq.Volt_Ft4 * gMotorInfo.Votage * 10L) >> 12;   
        m_s32 = m_s32 * qsin(16384 - gPowerTrq.anglePF) >> 15;
		gEstBemf.BemfVoltOnline = abs(m_s32 * gMotorInfo.FreqPer / gRotorSpeed.SpeedBigFilter);
	    gEstBemf.BemfVoltFilter = Filter32(gEstBemf.BemfVoltOnline , gEstBemf.BemfVoltFilter);
        gEstBemf.BemfVoltFilter = __IQsat(gEstBemf.BemfVoltFilter,4000,2500);	
	}
    m_CntTime++;
	if(m_CntTime > 4096)   // 每8s调节一次
    {
        m_CntTime = 0;				
        gEstBemf.BemfVoltDisPlay = gEstBemf.BemfVoltDisPlayTotal>>12; 	         
        if(gEstBemf.BemfVoltDisPlay >= gEstBemf.BemfVoltDisPlayLast + 20)
		{
		    if(gEstBemf.BemfVoltDisPlay >= gEstBemf.BemfVoltDisPlayLast + 50)    //刚开始调整比较快
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
       &&(INV_VOLTAGE_380V == gInvInfo.InvVoltageType))   //svc时用于磁场估算
	{
		m_s32 = ((s32)gEstBemf.BemfVoltDisPlayLast<<10)/gMotorInfo.Votage;
	    m_s32 = (m_s32 * (s32)gBasePar.FullFreq)/((s32)gMotorInfo.Frequency * 10);
		gPmsmRotorPosEst.InvOfKf = 1048576L/m_s32; 
	}
}


