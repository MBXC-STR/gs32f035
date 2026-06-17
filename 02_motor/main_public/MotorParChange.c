/****************************************************************
锟侥硷拷锟斤拷锟杰ｏ拷锟斤拷锟斤拷锟接癸拷锟杰诧拷锟街伙拷玫锟斤拷锟斤拷胁锟斤拷锟�
锟侥硷拷锟芥本锟斤拷
锟斤拷锟斤拷锟斤拷锟节ｏ拷

****************************************************************/
#include "MotorInclude.h"

extern PMSM_FLUX_WEAK_STRUCT   gPmFluxWeak;
/***********************************************************************
锟斤拷频锟斤拷锟狡讹拷锟筋定锟斤拷锟斤拷锟斤拷:
锟斤拷位小锟斤拷锟姐，只锟斤拷锟斤拷锟铰硷拷锟斤拷锟斤拷锟斤拷确锟较癸拷
锟斤拷锟斤拷   锟斤拷锟斤拷   锟筋定锟斤拷锟斤拷 	max(10s)(锟斤拷应1.5V)
18.5    16    9.49A   		32.5
22      17    11.29A		32.5
30      18    15.39A		40.63
37      19    18.98A		52.71
45      20    23.08A		60.94
55      21    28.16A		81.25
75      22    38.43A		114.71 
目前锟斤拷锟斤拷锟斤拷锟斤拷:
锟斤拷锟斤拷锟筋定锟斤拷锟斤拷10S锟斤拷锟斤拷锟斤拷ERROR_OVERLOAD_BRAKE  ERR61 锟斤拷锟斤拷锟斤拷息1
锟斤拷锟斤拷MAX锟斤拷锟斤拷直锟接憋拷锟斤拷锟斤拷ERROR_OVERLOAD_BRAKE  ERR61 锟斤拷锟斤拷锟斤拷息2
硬锟斤拷锟斤拷锟斤拷锟疥定锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟姐，1.5V锟斤拷应锟斤拷锟斤拷锟斤拷
没锟叫匡拷通锟狡讹拷锟杰碉拷锟斤拷锟叫碉拷锟斤拷锟斤拷锟狡讹拷锟斤拷直通ERROR_SHORT_BRAKE ERR62
************************************************************************/
#if (SOFTSERIES == MD500SOFT)
Uint const gInvBreakCurrentTable380T[27] =         // 380V,480V
{
    949,  949,  949,  949, 949,	     //8~12  
    949,  949,  949,  3055,3055,         //13~17
    3250,  4111, 5545, 7393,          //18~21  
    
    997,  384,  384,  384, 384,         //22~26  锟斤拷锟铰碉拷锟斤拷锟斤拷锟捷帮拷锟斤拷一锟斤拷小锟斤拷锟斤拷
    384,  384,  384,  384, 384,         //27~31
    384,  384,  384                       //32~34
};
Uint const gInvBreakMaxCurrentTable380T[27] =         // 380V,480V
{
    3250,  3250,  3250,  3250, 3250,	     //8~12  
    3250,  3250,  3250,  3250,3250,       //13~17
    4063,  5271,  6094,  8125,      //18~21            
    
    1147, 1147, 1147, 1147, 1147,        //22~26  锟斤拷锟铰碉拷锟斤拷锟斤拷锟捷帮拷锟斤拷一锟斤拷小锟斤拷锟斤拷
    1147, 1147, 1147, 1147, 1147,        //27~31
    1147 ,1147 ,1147                       //32~34
};
Uint const gInvBreakOverLoadTableFor185And22[10] = 
{
	1,//0,						//	120%  0.5s锟斤拷锟斤拷
	2,//0.2,						//	110%  0.2s锟斤拷锟斤拷
    100,//10,                 		// 	100%锟斤拷锟斤拷锟斤拷 10s锟斤拷锟斤拷锟斤拷
    500,//50,                 		// 	99%锟斤拷锟斤拷锟斤拷 50s锟斤拷锟斤拷锟斤拷
    820,//82,                	 	// 	98%锟斤拷锟斤拷锟斤拷 82s锟斤拷锟斤拷锟斤拷
    1380,//138,                 	// 	97%锟斤拷锟斤拷锟斤拷 138s锟斤拷锟斤拷锟斤拷
    2450,//245,                   	// 	96%锟斤拷锟斤拷锟斤拷 245s锟斤拷锟斤拷锟斤拷
    5000,//500,					// 	95%锟斤拷锟斤拷锟斤拷 500s锟斤拷锟斤拷锟斤拷
    10000,//1000					// 
    10000,//1000					// 
};
Uint const gInvBreakOverLoadTableFor30[10] = 
{
	13,// 1.3,					//	120%  1.3s锟斤拷锟斤拷
	50,// 5,						//	110%  5s锟斤拷锟斤拷
    100,// 10,                 	//	100%锟斤拷锟斤拷锟斤拷 10s锟斤拷锟斤拷锟斤拷
    160,// 16,                 	// 	98% 锟斤拷锟斤拷锟斤拷 16s锟斤拷锟斤拷锟斤拷
    230,//23,                 		// 	96% 锟斤拷锟斤拷锟斤拷 23s锟斤拷锟斤拷锟斤拷
    300,//30,                 		// 	94% 锟斤拷锟斤拷锟斤拷 30s锟斤拷锟斤拷锟斤拷
    440,//44,                   	// 	92% 锟斤拷锟斤拷锟斤拷 44s锟斤拷锟斤拷锟斤拷
    580,//58,					// 	90% 锟斤拷锟斤拷锟斤拷 58s锟斤拷锟斤拷锟斤拷
    2870,//287					// 	85% 锟斤拷锟斤拷锟斤拷 278s锟斤拷锟斤拷锟斤拷
    5000,//500					//   
};
Uint const gInvBreakOverLoadTableFor37[10] = 
{
	25,// 2.5,					//	120%  2.5s锟斤拷锟斤拷
	75,// 7.5,					//	110%  7.5s锟斤拷锟斤拷
    100,// 10,                 	// 	100% 锟斤拷锟斤拷锟斤拷 10s锟斤拷锟斤拷锟斤拷
    180,// 18,                 	// 	98% 锟斤拷锟斤拷锟斤拷 18s锟斤拷锟斤拷锟斤拷
    250,// 25,                 	// 	96% 锟斤拷锟斤拷锟斤拷 25s锟斤拷锟斤拷锟斤拷
    300,// 30,                 	// 	94% 锟斤拷锟斤拷锟斤拷 30s锟斤拷锟斤拷锟斤拷
    360,// 36,                   	// 	92% 锟斤拷锟斤拷锟斤拷 36锟斤拷锟斤拷锟斤拷
	480,// 48,					// 	90% 锟斤拷锟斤拷锟斤拷 48s锟斤拷锟斤拷锟斤拷
	1000,//	100,					// 	90% 锟斤拷锟斤拷锟斤拷 48s锟斤拷锟斤拷锟斤拷
	3400,//	340,					// 	90% 锟斤拷锟斤拷锟斤拷 48s锟斤拷锟斤拷锟斤拷
};

Uint const gInvBreakOverLoadTableFor45And55[10] = 
{
	1,//0,						//	120% 锟斤拷锟斤拷锟斤拷 0s锟斤拷锟斤拷锟斤拷
	10,// 1,						//	110% 锟斤拷锟斤拷锟斤拷 1s锟斤拷锟斤拷锟斤拷
    100,// 10,                 	// 	100% 锟斤拷锟斤拷锟斤拷 10s锟斤拷锟斤拷锟斤拷
    290,// 29,                 	// 	98%  锟斤拷锟斤拷锟斤拷 29s锟斤拷锟斤拷锟斤拷
    580,// 58,                 	// 	96%  锟斤拷锟斤拷锟斤拷 58s锟斤拷锟斤拷锟斤拷
    900,// 90,                 	// 	94%  锟斤拷锟斤拷锟斤拷 90s锟斤拷锟斤拷锟斤拷
    1500,// 150,            		// 	92%  锟斤拷锟斤拷锟斤拷 150s锟斤拷锟斤拷锟斤拷
    3000,//300					//
    5000,//500					//
    10000,//1000					//
};
Uint const gInvBreakOverLoadTableFor75[10] = 
{
	5,// 0.5,					//	120% 锟斤拷锟斤拷锟斤拷 0.5s锟斤拷锟斤拷
	30,// 3,						//	110% 锟斤拷锟斤拷锟斤拷 3s锟斤拷锟斤拷
    100,// 10,                 	// 	100%	锟斤拷锟斤拷锟斤拷 10s锟斤拷锟斤拷锟斤拷
    180,// 18,                 	// 	98% 	锟斤拷锟斤拷锟斤拷 18s锟斤拷锟斤拷锟斤拷
    270,// 27,                 	// 	96% 	锟斤拷锟斤拷锟斤拷 27s锟斤拷锟斤拷锟斤拷
    390,// 39,                 	// 	94% 	锟斤拷锟斤拷锟斤拷 39s锟斤拷锟斤拷锟斤拷
    530,// 53,                   	// 	92% 	锟斤拷锟斤拷锟斤拷 53s锟斤拷锟斤拷锟斤拷
	1000,// 100,					// 	90% 	锟斤拷锟斤拷锟斤拷 100s锟斤拷锟斤拷锟斤拷
	5000,//500					//
	10000,//1000					//
};
#endif
// // 停锟斤拷锟斤拷锟斤拷转锟斤拷
void SystemParChg2Ms()	
{
	Ulong m_ULong;
    Uint    minType, maxType;
    Uint    *pInvVolt, *pInvCur;
    Uint 	 m_UData,m_Pointer;
#if (SOFTSERIES == MD500SOFT)
	Uint *pInvBreakCur,*pInvBreakMaxCur;
#endif
    
// 锟斤拷取锟斤拷频锟斤拷系统锟斤拷锟斤拷
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
	else if(200 > gInvInfo.InvTypeSet)      // 220T, 没锟斤拷P锟酵伙拷
	{
	    pInvVolt  = (Uint *)gInvVoltageInfo220T;//使锟斤拷380V锟斤拷频锟斤拷锟侥额定锟斤拷锟斤拷锟斤拷也锟斤拷锟角憋拷锟斤拷锟狡的伙拷锟斤拷
        pInvCur   = (Uint *)gInvCurrentTable220T;
#if (SOFTSERIES == MD500SOFT)
        pInvBreakCur = (Uint *)gInvBreakCurrentTable380T;
		pInvBreakMaxCur = (Uint *)gInvBreakMaxCurrentTable380T;
#endif
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 100;
        gInvInfo.InvVoltageType = INV_VOLTAGE_220V;
        gInvInfo.GPType   = 1;                                      //锟斤拷锟斤拷锟斤拷P锟酵伙拷

        //****************************** 380V锟斤拷锟斤拷为220V锟斤拷锟斤拷锟铰讹拷锟斤拷锟竭达拷锟斤拷
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
#if (SOFTSERIES == MD500SOFT)  //锟斤拷380V锟斤拷
		pInvVolt  = (Uint *)gInvVoltageInfo480T;	//锟斤拷锟斤拷380V一锟斤拷
        pInvCur = (Uint *)gInvCurrentTable380T;  // 锟斤拷380V锟斤拷锟矫碉拷锟斤拷锟斤拷
        pInvBreakCur = (Uint *)gInvBreakCurrentTable380T;// 锟斤拷380V锟斤拷锟矫碉拷锟斤拷锟斤拷
		pInvBreakMaxCur = (Uint *)gInvBreakMaxCurrentTable380T;// 锟斤拷380V锟斤拷锟矫碉拷锟斤拷锟斤拷
        gInvInfo.InvType  = gInvInfo.InvTypeSet - 300;
        gInvInfo.InvVoltageType = INV_VOLTAGE_380V;//MD500锟斤拷480V锟斤拷380V
        gInvInfo.GPType = gInvInfo.GpTypeSet;
#else
		pInvVolt  = (Uint *)gInvVoltageInfo480T;
        pInvCur = (Uint *)gInvCurrentTable380T;  // 锟斤拷380V锟斤拷锟矫碉拷锟斤拷锟斤拷
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
    minType              = *(pInvVolt + 4);  //锟斤拷频锟斤拷锟斤拷锟斤拷锟侥伙拷锟斤拷锟斤拷锟斤拷
    maxType              = *(pInvVolt + 5);  //锟斤拷频锟斤拷锟斤拷锟斤拷锟侥伙拷锟斤拷锟斤拷锟斤拷
    gInLose.ForeInvType  = *(pInvVolt + 6);  //锟斤拷始缺锟洁保锟斤拷锟斤拷锟斤拷始锟斤拷锟斤拷
    gUDC.uDCADCoff       = *(pInvVolt + 7); 
    gVFPar.ovPointCoff   = *(pInvVolt + 8);     // 锟斤拷锟节硷拷锟斤拷锟窖癸拷锟斤拷频锟�

	gInvInfo.InvType = (gInvInfo.InvType > maxType) ? maxType : gInvInfo.InvType;
	if(gInvInfo.InvType <= minType)
    {
        gInvInfo.GPType = 1;	                        //锟斤拷小锟斤拷锟斤拷锟睫凤拷锟斤拷锟斤拷GP
        gInvInfo.InvType = minType;
	}
#if (SOFTSERIES == MD500SOFT)
    if(gInvInfo.GPType == 1)   
	{
	    gInvInfo.InvTypeApply = gInvInfo.InvType;
	}
    else if(gInvInfo.InvType < 29)    // 220Kw P锟斤拷锟斤拷锟斤拷
    {
        gInvInfo.InvTypeApply = gInvInfo.InvType - 1;   //P锟酵伙拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷一锟斤拷
    }
	else if(gInvInfo.InvType >= 29)   // 250Kw p锟斤拷锟斤拷锟斤拷
	{
	    gInvInfo.InvTypeApply = gInvInfo.InvType - 2;   //锟斤拷锟斤拷P锟酵伙拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	}
    if((gInvInfo.InvType == 34) && (gInvInfo.GPType == 1))                          // 450G模锟斤拷锟斤拷400G一锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷同锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷450P锟斤拷锟叫憋拷锟斤拷
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
        gInvInfo.InvTypeApply = gInvInfo.InvType - 1;   //P锟酵伙拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷一锟斤拷
    }
#endif	
	gInvInfo.InvCurrent  = *(pInvCur + gInvInfo.InvTypeApply - minType);        //锟斤拷锟斤拷锟斤拷锟斤拷使锟矫的额定锟斤拷锟斤拷
    //gInvInfo.InvCurrentOvload = *(pInvCur + gInvInfo.InvType - minType);
#if (SOFTSERIES == MD500SOFT)
	if((2 == gInvInfo.GPType) && (22 == gInvInfo.InvType))  // MD500 30P锟斤拷75P锟斤拷锟斤拷锟斤拷G锟酵伙拷锟斤拷同
	{
		gInvInfo.InvCurrForP = 1500; //MD500锟斤拷75P锟筋定锟斤拷锟斤拷为150A
	}
	else if((2 == gInvInfo.GPType) && (18 == gInvInfo.InvType))
	{
        gInvInfo.InvCurrForP = 6000;
    }
	else
	{
    	gInvInfo.InvCurrForP = *(pInvCur + gInvInfo.InvType - minType);      //P锟酵伙拷使锟矫的额定锟斤拷锟斤拷
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
    gInvInfo.InvCurrForP = *(pInvCur + gInvInfo.InvType - minType);      //P锟酵伙拷使锟矫的额定锟斤拷锟斤拷
#endif
    if(gInvInfo.InvVoltageType == INV_VOLTAGE_220V)                             // 锟斤拷锟斤拷小锟斤拷锟斤拷锟饺凤拷锟�
    {   
        gMotorExtInfo.UnitCoff = (gInvInfo.InvType > 18) ? 10 : 1;
    }
    else
    {
        gMotorExtInfo.UnitCoff = (gInvInfo.InvType > 21) ? 10 : 1; 
    }
    
	if((1 != gInvInfo.GPType) && (22 == gInvInfo.InvType))
	{
        // 锟斤拷锟斤拷为22锟斤拷P锟酵伙拷锟斤拷锟斤拷锟斤拷应锟斤拷锟斤拷1位小锟斤拷锟姐，锟斤拷锟角讹拷取锟侥诧拷锟斤拷锟斤拷锟斤拷锟斤拷2位
		gInvInfo.InvCurrent = (Ulong)gInvInfo.InvCurrent * 3264 >> 15;
    }
    //gInvInfo.InvCurrForP = *(pInvCur + gInvInfo.InvType - minType);      //P锟酵伙拷使锟矫的额定锟斤拷锟斤拷

//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	m_UData = (gInvInfo.InvTypeApply > 29) ? 29 : gInvInfo.InvTypeApply;
	m_UData = (m_UData < 12) ? 12 : m_UData;
#if (SOFTSERIES == MD380SOFT)
    if(500 <= gInvInfo.InvTypeSet)                //1140V锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷确锟斤拷 2011.5.8 L1082
    {
        gDeadBand.DeadBand = DBTIME_1140V*gDeadBand.DeadTimeSet/10;          
        gDeadBand.Comp     = DCTIME_1140V*gDeadBand.DeadTimeSet/10;
    }
    else if(400 <= gInvInfo.InvTypeSet)                       //690V锟斤拷锟斤拷锟斤拷锟斤拷锟酵诧拷锟斤拷锟斤拷锟教讹拷
    {
	    gDeadBand.DeadBand = gDeadBandTable[13];         //锟斤拷锟斤拷锟教讹拷4.8us锟斤拷锟斤拷锟斤拷锟斤拷2.5us
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
	EALLOW;									//锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷
	EPwm1Regs.DBFED = gDeadBand.DeadBand;
	EPwm1Regs.DBRED = gDeadBand.DeadBand;
	EPwm2Regs.DBFED = gDeadBand.DeadBand;
	EPwm2Regs.DBRED = gDeadBand.DeadBand;
	EPwm3Regs.DBFED = gDeadBand.DeadBand;
	EPwm3Regs.DBRED = gDeadBand.DeadBand;
	EDIS;
        
//锟斤拷锟斤拷ADC锟斤拷锟斤拷锟接筹拷时锟斤拷
 /*   #ifdef		DSP_CLOCK100
	gADC.DelayApply = gADC.DelaySet * 10;       // default: 0.5us
    #else
	gADC.DelayApply = gADC.DelaySet * 6;
    #endif
*/
//锟斤拷锟斤拷锟斤拷锟较碉拷锟斤拷偷锟窖瓜碉拷锟�
	//锟斤拷锟斤拷锟斤拷锟教★拷拇锟斤拷锟�
	m_UData = gInvInfo.InvCurrent>>2;
	DINT;
	gMotorInfo.Current = (gMotorInfo.CurrentGet < m_UData) ? m_UData : gMotorInfo.CurrentGet;
	m_ULong = (((Ulong)gMotorInfo.Current)<<8) / gMotorInfo.CurrentGet;
	gMotorInfo.CurBaseCoff = (m_ULong > 32767) ? 32767 : m_ULong;
	EINT;
    
	//AD值32767锟斤拷应锟斤拷频锟斤拷锟筋定锟斤拷锟斤拷值锟斤拷2锟斤拷 转锟斤拷为锟斤拷锟斤拷疃拷锟斤拷锟斤拷谋锟矫粗碉拷锟绞�(Q24)
	//AD转锟斤拷值锟叫伙拷为锟斤拷锟斤拷锟矫粗碉拷锟绞局碉拷姆锟斤拷锟轿�: (AD/32767 * 2sqrt(2) * Iv/Im) << 24 *8
	// (1/32767 * 2sqrt(2) << 24) * 8 == 11586
	// CPU28035时锟斤拷32767锟斤拷应锟侥碉拷锟斤拷值为: 2sqrt(2) *3.3/3.0 * Iv
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

//锟斤拷锟姐不同锟街憋拷锟绞碉拷频锟绞憋拷示
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
	gBasePar.FullFreq01 = (Ulong)gBasePar.MaxFreq * (Ulong)gMainCmd.pu2siCoeff + 2000;	//32767锟斤拷示锟斤拷频锟斤拷值
	gBasePar.FullFreq =   gBasePar.MaxFreq + 20 * gMainCmd.si2puCoeff;	//频锟绞伙拷值
	gMotorInfo.FreqPer =  ((Ulong)gMotorInfo.Frequency <<15) / gBasePar.FullFreq;

    gMotorInfo.Motor_HFreq = ((Ulong)gMotorInfo.Frequency * 410) >>10;
    gMotorInfo.Motor_LFreq = ((Ulong)gMotorInfo.Frequency * 205) >>10;

    // 锟斤拷锟斤拷锟斤拷一些锟斤拷锟斤拷锟侥硷拷锟姐，锟斤拷省cpu时锟戒开锟斤拷
    //gUVWPG.UvwPolesRatio = ((Ulong)gMotorExtInfo.Poles << 8) / gUVWPG.UvwPoles;    // Q8 锟斤拷锟�
    gRotorTrans.PolesRatio = ((Ulong)gMotorExtInfo.Poles << 8) / gRotorTrans.Poles; // Q8
}

// // 锟斤拷锟斤拷锟叫诧拷锟斤拷转锟斤拷
void RunStateParChg2Ms()	
{
    //s16     temp;
    Uint    m_UData, tempU;
    Ulong   m_Long;    
    Uint    m_AbsFreq;
    //long    mCurM, mCurT;
     Ulong   tmpAmp;
    Uint    m_FluxWeakDepth;
    Uint    m_LimitOutVolt;
    Uint    m_MaxOutVoltReal;
	gIUVWQ12.U = (s16)(gIUVWQ24.U>>12);				
	gIUVWQ12.V = (s16)(gIUVWQ24.V>>12);
	gIUVWQ12.W = (s16)(gIUVWQ24.W>>12);

    gIMTSetQ12.M = (s16)(gIMTSetApply.M >> 12);
    gIMTSetQ12.T = (s16)(gIMTSetApply.T >> 12);

	m_AbsFreq = abs(gMainCmd.FreqSyn);
    tempU = ((Ulong)gMotorExtPer.I0 * gMotorInfo.FreqPer) / m_AbsFreq;
    gMotorExtPer.IoVsFreq = (m_AbsFreq < gMotorInfo.FreqPer) ? gMotorExtPer.I0 : tempU;
      
    //锟斤拷锟斤拷锟竭碉拷锟斤拷
/*    gIAmpTheta.CurTmpM = abs(gIMTQ12.M);
    gIAmpTheta.CurTmpT = abs(gIMTQ12.T);
    tmpAmp = (long)gIAmpTheta.CurTmpM * gIAmpTheta.CurTmpM;
    tmpAmp += (long)gIAmpTheta.CurTmpT * gIAmpTheta.CurTmpT;
    gIAmpTheta.Amp = (Uint)qsqrt((Ulong)tmpAmp);
	*/
	if(gLineCur.CurCnt >= 1024)
	{
        DINT;
	    gLineCur.ImAvr = (s16)(gLineCur.ImTotal / (long)gLineCur.CurCnt);
	    gLineCur.ItAvr = (s16)(gLineCur.ItTotal / (long)gLineCur.CurCnt);
		gLineCur.ImTotal = 0;
        gLineCur.ItTotal = 0;
        gLineCur.CurCnt = 0;
		EINT;
	}

	tmpAmp = (long)gLineCur.ImAvr * gLineCur.ImAvr;
    tmpAmp += (long)gLineCur.ItAvr * gLineCur.ItAvr;
    gIAmpTheta.Amp = (Uint)qsqrt((Ulong)tmpAmp);//锟斤拷锟斤拷锟绞碉拷锟斤拷锟絀s锟斤拷Is^2=Id^2+Iq^2
    //...................................锟斤拷锟斤拷锟竭碉拷锟斤拷
    if((gMainCmd.Command.bit.StartDC == 1) || 
       (gMainCmd.Command.bit.StopDC == 1))	/****直锟斤拷锟狡讹拷状态锟斤拷示锟斤拷锟斤拷锟脚达拷锟斤拷****/
    {
        gIAmpTheta.Amp = ((Ulong)gIAmpTheta.Amp * 5792)>>12;//5792)>>12;为   5792锟斤拷4096=1.414
    }
	gLineCur.CurPer = Filter(gLineCur.CurPer,gIAmpTheta.Amp,1024);   //锟剿诧拷锟斤拷锟絀s
	gLineCur.CurPerFilter += gLineCur.CurPer - (gLineCur.CurPerFilter>>7);	

	// 锟斤拷锟斤拷锟狡碉拷锟斤拷疃拷锟斤拷锟轿拷锟街碉拷谋锟矫粗碉拷锟斤拷锟�
	m_Long = (Ulong)gLineCur.CurPer * gMotorInfo.Current;   //gMotorInfo.Current为锟斤拷锟斤拷锟斤拷锟斤拷选锟矫的碉拷锟斤拷锟斤拷值(锟斤拷锟杰猴拷实锟绞碉拷锟斤拷锟斤拷锟斤拷锟斤拷锟�) 锟斤拷锟斤拷锟街碉拷锟绞碉拷锟斤拷锟饺★拷锟狡碉拷锟斤拷锟斤拷偷亩疃拷锟斤拷锟斤拷偷锟斤拷锟侥额定锟斤拷锟斤拷锟叫碉拷小值
	gLineCur.CurBaseInv = m_Long/gInvInfo.InvCurrForP;  //P锟酵伙拷使锟矫的额定锟斤拷锟斤拷       锟斤拷锟截憋拷锟斤拷锟矫碉拷锟斤拷                                         锟斤拷锟斤拷锟斤拷实锟斤拷锟角帮拷锟斤拷锟斤拷锟斤拷锟斤拷锟侥碉拷锟斤拷值锟斤拷锟斤拷为锟斤拷锟斤拷锟斤拷时锟斤拷谋冉锟街狄伙拷锟斤拷幕锟阶硷拷锟�
                                                                                                //CurBaseInv锟斤拷锟斤拷锟节碉拷锟斤拷锟斤拷锟狡碉拷IS值锟斤拷锟斤拷锟絀S锟角撅拷锟斤拷转锟斤拷锟侥碉拷锟斤拷值锟斤拷锟斤拷锟斤拷锟侥伙拷准锟酵伙拷锟酵★拷锟斤拷锟斤拷亩疃拷锟斤拷锟斤拷锟斤拷锟叫≈碉拷锟轿伙拷锟斤拷幕锟阶�
    // 同锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷                                                      //CurBaseInv=(锟剿诧拷锟斤拷锟絀S)*锟斤拷锟斤拷频锟斤拷锟筋定锟斤拷锟斤拷锟酵碉拷锟斤拷疃拷锟斤拷锟斤拷械锟斤拷锟叫≈碉拷锟�/锟斤拷锟斤拷锟酵的额定锟斤拷锟斤拷锟斤拷
    gDeadBand.InvCurFilter = Filter2(gLineCur.CurBaseInv, gDeadBand.InvCurFilter);
	m_UData = abs(gMainCmd.FreqSyn);                                   //锟斤拷锟斤拷锟斤拷实锟斤拷值锟斤拷锟斤拷锟斤拷锟斤拷锟狡碉拷?
	gMainCmd.FreqReal = ((Ullong)m_UData * gBasePar.FullFreq01 + (1<<14))>>15;
    gMainCmd.FreqRealFilt += (gMainCmd.FreqReal>>4) - (gMainCmd.FreqRealFilt>>4);
    m_UData = abs(gMainCmd.FreqDesired);
    gMainCmd.FreqDesiredReal = ((Ullong)m_UData * gBasePar.FullFreq01 + (1<<14))>>15;

    gMainCmd.FreqSetReal = ((llong)gMainCmd.FreqSet * (llong)gBasePar.FullFreq01 + (1<<14))>>15;
//    gMainCmd.FreqSetBak = gMainCmd.FreqSet;
    
// 欠压锟斤拷筛锟斤拷莨锟斤拷锟斤拷锟斤拷锟斤拷
#if (SOFTSERIES == MD380SOFT)
    gInvInfo.InvLowUDC = gInvInfo.LowUdcCoff;
#else
    gInvInfo.InvLowUDC = Max(gInvInfo.LowUdcCoff,2100);
#endif

   
// 锟斤拷锟斤拷母锟竭碉拷压锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷压
    if(gFluxWeak.Mode == 2)
    {
    	m_MaxOutVoltReal = ((u32)gUDC.uDCBigFilter * 23170L)>>15;

    	m_FluxWeakDepth = gPWM.OverModuleCoff - 100; //锟矫碉拷压锟斤拷锟斤拷系锟斤拷锟斤拷锟斤拷锟斤拷锟矫癸拷锟斤拷锟斤拷系锟斤拷锟斤拷实锟斤拷锟斤拷锟斤拷锟斤拷压锟斤拷锟斤拷锟斤拷
        m_FluxWeakDepth = (long)m_FluxWeakDepth * m_FluxWeakDepth * 9>>5;//100 + (gOutVolt.MaxOutVoltCoff - 100) * 5
    	m_FluxWeakDepth = 100 + m_FluxWeakDepth;

        m_LimitOutVolt = ((u32)m_MaxOutVoltReal * (u32)m_FluxWeakDepth)/100;
        gOutVolt.MaxOutVolt = ((u32)gUDC.uDCBigFilter * 232L * (u32)gPWM.OverModuleCoff)>>15;
    	gOutVolt.MaxOutVoltPer = ((u32)m_MaxOutVoltReal<<12)/(gMotorInfo.Votage*10); /*锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷*/
    	gOutVolt.LimitOutVoltPer = ((u32)m_LimitOutVolt<<12)/(gMotorInfo.Votage*10); /*锟斤拷锟脚碉拷锟斤拷锟斤拷锟斤拷锟斤拷压*/
//		gOutVolt.LimitOutVoltPer1 = ((u32)m_MaxOutVoltReal<<12)/(gMotorInfo.Votage*10);
    }
    else
    {
	    //锟斤拷锟斤拷锟斤拷疃拷锟窖癸拷锟叫∈憋拷锟斤拷锌锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷要锟截憋拷锟斤拷
		m_UData = gUDC.uDCBigFilter;//Min(gUDC.uDCBigFilter,gInvInfo.BaseUdc);
		gOutVolt.MaxOutVolt = ((u32)m_UData * 232L * (u32)gPWM.OverModuleCoff)>>15; 
		gOutVolt.LimitOutVolt = gOutVolt.MaxOutVolt - ((u32)gOutVolt.MaxOutVolt * (u32)gPmFluxWeak.FluxWeakDepth)/100;
		gOutVolt.MaxOutVoltPer = ((u32)gOutVolt.MaxOutVolt<<12)/(gMotorInfo.Votage*10); /*锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷*/
		gOutVolt.LimitOutVoltPer = ((u32)gOutVolt.LimitOutVolt<<12)/(gMotorInfo.Votage*10); /*锟斤拷锟脚碉拷锟斤拷锟斤拷锟斤拷锟斤拷压*/
//	    gOutVolt.LimitOutVoltPer1 = (s32)gOutVolt.LimitOutVoltPer * 100l/gPWM.OverModuleCoff;
	}		
	gPmFluxWeak.VoltLpf = Filter16(gUAmpTheta.Amp,gPmFluxWeak.VoltLpf);  //锟斤拷锟斤拷锟窖癸拷瞬锟街� Q12
//*************************************************************//LJH1917
    if (gMainCmd.Command.bit.Start == 0)    // 停锟斤拷状态锟斤拷只锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷10s锟斤拷锟斤拷止锟秸伙拷锟酵伙拷锟借备
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
// VF 锟斤拷锟斤拷锟脚达拷锟斤拷
    gVarAvr.CoffApply = gVFPar.VFOverExc;
   
// 同锟斤拷锟斤拷锟斤拷夭锟斤拷锟斤拷锟斤拷獯︼拷锟�
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


    
        // 转锟斤拷锟斤拷锟斤拷锟借定锟侥憋拷锟斤拷锟斤拷锟斤拷锟轿伙拷媒锟�
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
        gUVWPG.UvwRealErr_deg = (s16)(gIPMPos.RotorPos - tempU) * 180L >>15;
        #endif
        
        // 锟街讹拷锟睫革拷锟斤拷锟轿伙拷媒锟绞憋拷锟揭拷锟接�, 锟斤拷锟斤拷锟斤拷远锟斤拷锟斤拷锟�
        if(gPGData.PGMode == 0 && gMainStatus.ParaCalTimes == 1 && // 锟窖撅拷锟较碉拷
            gMainStatus.RunStep != STATUS_GET_PAR &&
            gIPMPos.ZeroPosLast != gIPMPos.RotorZero)
        {
            tempU = gIPMPos.RotorZero - gIPMPos.ZeroPosLast;
            
            SetIPMPos(gIPMPos.RotorPos + tempU);
        }   
        gIPMPos.ZeroPosLast = gIPMPos.RotorZero;
        gPmDecoup.EnableDcp = 0;          
	}

    //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
	if(gCtrMotorType != RUN_SYNC_TUNE)
	{
	    EQepRegs->QDECCTL.bit.SWAP = gPGData.SpeedDir;
	}



// 锟斤拷锟斤拷锟斤拷压校锟斤拷系锟斤拷实时锟斤拷锟姐，锟斤拷锟斤拷锟斤拷锟�
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
锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷颍捍锟斤拷锟�0.5Ms循锟斤拷锟叫ｏ拷锟斤拷锟杰达拷锟捷的诧拷锟斤拷锟斤拷锟斤拷刹锟斤拷锟阶拷锟斤拷锟斤拷锟斤拷胁锟斤拷锟阶硷拷锟斤拷裙锟斤拷锟�
1. update gCtrMotorType;
2. measure speed ;
*************************************************************/
void SystemParChg05Ms()
{    
// 锟斤拷锟斤拷锟斤拷透锟斤拷锟�
    if(gMotorInfo.LastMotorType != gMotorInfo.MotorType)
    {
        gMotorInfo.LastMotorType = gMotorInfo.MotorType;

        gPGData.PGType = PG_TYPE_NULL;     // 锟斤拷锟斤拷锟斤拷锟斤拷薷暮锟斤拷锟斤拷鲁锟绞硷拷锟斤拷锟斤拷锟斤拷锟�
    }
    
// PG锟斤拷锟斤拷锟酵革拷锟斤拷
    if(gPGData.PGType != (PG_TYPE_ENUM_STRUCT)gPGData.PGTypeGetFromFun)
    {
        gPGData.PGType = (PG_TYPE_ENUM_STRUCT)gPGData.PGTypeGetFromFun;
        ReInitForPG();

        gIPMInitPos.Flag = 0;
    }
    
// QEP锟斤拷锟劫的达拷锟斤拷 -- 选锟斤拷QEP
	if(gPGData.QEPIndex != (QEP_INDEX_ENUM_STRUCT)gExtendCmd.bit.QepIndex)
	{
        gPGData.QEPIndex = (QEP_INDEX_ENUM_STRUCT)gExtendCmd.bit.QepIndex;
                
        if(gPGData.QEPIndex == QEP_SELECT_1) // 锟斤拷锟斤拷PG锟斤拷锟斤拷锟斤拷
        {
            EQepRegs = (struct EQEP_REGS *)&EQep1Regs;
            EALLOW;
            #ifdef TARGET_GS32
            Interrupt_register(INT_EQEP1, &PG_Zero_isr);
            Interrupt_enable(INT_EQEP1);
            SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EQEP1);
            #else
            PieVectTable.EQEP1_INT = &PG_Zero_isr;
            PieCtrlRegs.PIEIER5.bit.INTx1 = 1;
            SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 1;
            #endif
            EDIS;
        }
        #ifdef TMS320F2808                      // 28035 只锟斤拷一锟斤拷QEP
        if(gPGData.QEPIndex == QEP_SELECT_2)
        {
            EQepRegs = (struct EQEP_REGS *)&EQep2Regs;
            EALLOW;
            #ifdef TARGET_GS32
            Interrupt_register(INT_EQEP2, &PG_Zero_isr);
            #else
            PieVectTable.EQEP2_INT = &PG_Zero_isr;
            #endif
            #ifdef TARGET_GS32
            Interrupt_enable(INT_EQEP2);
            SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EQEP2);
            #else
            PieCtrlRegs.PIEIER5.bit.INTx2 = 1;
            SysCtrlRegs.PCLKCR1.bit.EQEP2ENCLK = 1;
            #endif
            
            EDIS;
        }
        #endif        
        InitSetQEP();        
    }    

    if(MOTOR_TYPE_PM != gMotorInfo.MotorType ||     // 锟届步锟斤拷
        gPGData.PGMode != 0)                        // 锟斤拷锟斤拷位锟矫憋拷锟斤拷锟斤拷 -- 锟斤拷锟斤拷
    {
        EALLOW;
        (*EQepRegs).QEINT.all = 0x0;  //取锟斤拷QEP锟斤拷I锟脚猴拷锟叫讹拷
        EDIS;
    }
    else
    {
        EALLOW;
        (*EQepRegs).QEINT.all = 0x0400; //锟斤拷锟斤拷位锟矫憋拷锟斤拷锟斤拷锟角凤拷锟斤拷要锟斤拷锟叫讹拷?
        EDIS;
    }
}

/*************************************************************
	同锟斤拷锟斤拷锟斤拷锟斤拷觳斤拷锟斤拷锟斤拷锟斤拷锟戒换
	
*************************************************************/
void ChangeMotorPar(void)
{
	Uint m_UData,m_BaseL;
	Ulong m_Ulong;
	//Uint m_AbsFreq;
    
	//锟斤拷谢锟街滴拷锟斤拷杩癸拷锟�?2*pi*锟斤拷锟狡碉拷锟�
	m_BaseL = ((Ulong)gMotorInfo.Votage * 3678)/gMotorInfo.Current;
	m_BaseL = ((Ulong)m_BaseL * 5000)/gBasePar.FullFreq01;
    

    m_UData = ((Ulong)gMotorExtInfo.RsPm * (Ulong)gMotorInfo.Current)/gMotorInfo.Votage;	
    gMotorExtPer.Rpm = ((Ulong)m_UData * 18597)>>14;
    
    m_Ulong = (((Ulong)gMotorExtInfo.LD <<15) + m_BaseL) >>1;
    gMotorExtPer.LD = m_Ulong / m_BaseL / 10;                   // 同锟斤拷锟斤拷d锟斤拷锟斤拷Q13
    m_Ulong = (((Ulong)gMotorExtInfo.LQ <<15) + m_BaseL) >>1;
    gMotorExtPer.LQ = m_Ulong / m_BaseL / 10;                   // 同锟斤拷锟斤拷q锟斤拷锟斤拷Q13

    // 锟斤拷锟斤拷同锟斤拷锟斤拷转锟接达拷锟斤拷
    m_Ulong = ((long)gMotorExtInfo.BemfVolt <<12) / (gMotorInfo.Votage *10);      // Q12
    //gMotorExtPer.EMF = (u16)m_Ulong * 10;
    gMotorExtPer.FluxRotor = (m_Ulong << 15) / gMotorInfo.FreqPer;                   // Q12\

    //gMotorExtPer.ItRated = 4096L;
    //gPowerTrq.rpItRated = (1000L<<12) / gMotorExtPer.ItRated;
    gPowerTrq.rpItRated = 1000;


	//....锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	m_Ulong = (((Ullong)gMotorInfo.Frequency * (Ullong)gMainCmd.pu2siCoeff * 19661L)>>15);
	gMotorExtInfo.Poles = (m_Ulong + (gMotorExtInfo.Rpm>>1)) / gMotorExtInfo.Rpm;

    //0.01Hz为锟斤拷位锟侥额定转锟斤拷锟斤拷
    //m_Ulong = ((Ulong)gMotorExtInfo.Rpm * gMotorExtInfo.Poles * 100L)/60;
    m_Ulong = gMotorExtInfo.Rpm * gMotorExtInfo.Poles * 6830L >> 12;
    gMotorExtInfo.RatedComp = (Ulong)gMotorInfo.Frequency * gMainCmd.pu2siCoeff - m_Ulong;
                              
    //锟斤拷么锟斤拷锟侥额定转锟斤拷锟斤拷
    gMotorExtPer.RatedComp = ((long)gMotorExtInfo.RatedComp << 15)/gBasePar.FullFreq01; 
}

// 锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷省锟阶拷丶锟斤拷锟�
void InvCalcPower(void)
{
    long m_PowerN,m_s32;
	static Uint m_CntTime = 0;
   
    gPowerTrq.angleFilter = Filter64(gIAmpTheta.PowerAngle, gPowerTrq.angleFilter);     //锟剿诧拷锟接大，达拷16锟斤拷为64
    gPowerTrq.angleFilter = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.angleFilter : 0; 
    gPowerTrq.anglePF = abs(gPowerTrq.angleFilter);
	gRotorSpeed.SpeedBigFilter = Filter(gRotorSpeed.SpeedBigFilter,gRotorSpeed.SpeedApply,1024);
    
	//gPowerTrq.Cur_Ft4 = Filter32(gLineCur.CurPer, gPowerTrq.Cur_Ft4);
    //gPowerTrq.Volt_Ft4 = Filter64(gOutVolt.VoltApply,gPowerTrq.Volt_Ft4);  // 锟斤拷锟斤拷锟斤拷锟窖癸拷锟斤拷锟斤拷瞬锟斤拷锟绞癸拷锟绞撅拷墓锟斤拷矢锟狡斤拷锟�
    gPowerTrq.Cur_Ft4 = Filter(gPowerTrq.Cur_Ft4,gLineCur.CurPer,1024);
	gPowerTrq.Volt_Ft4 = Filter(gPowerTrq.Volt_Ft4,gOutVolt.VoltApply,1024);	
	gPowerTrq.InvPowerPU = (1732L * (long)gPowerTrq.Volt_Ft4 /1000L) * gPowerTrq.Cur_Ft4 >> 12;
    gPowerTrq.InvPowerPU = (long)gPowerTrq.InvPowerPU * qsin(16384 - gPowerTrq.anglePF) >> 15;

	m_PowerN = ((long)gMotorInfo.Current * gMotorInfo.Votage) / 1000;
    //m_PowerN = ((long)gMotorInfo.Current * gMotorInfo.Votage) >> 10;
	if(gInvInfo.InvVoltageType == INV_VOLTAGE_220V)   //220v锟斤拷锟斤拷37Kw锟叫伙拷小锟斤拷锟斤拷
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
    //gPowerTrq.TrqOutHoAndFo_pu = (gIMTSetApply.T>>12) * (long)gMotorInfo.Current /(long)gInvInfo.InvCurrForP * 1000L >> 12;	//锟斤拷锟斤拷锟斤拷锟接匡拷锟狡碉拷转锟截碉拷锟斤拷
    gPowerTrq.TrqOutHoAndFo_pu = (gIMTSetApply.T>>12) *  1000L >> 12;
    gPowerTrq.TrqOut_pu = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.TrqOut_pu : 0;       // 停锟斤拷状态锟斤拷锟解处锟斤拷
    gPowerTrq.TrqOutHoAndFo_pu = (gMainStatus.StatusWord.bit.StartStop) ? gPowerTrq.TrqOutHoAndFo_pu : 0;       // 停锟斤拷状态锟斤拷锟解处锟斤拷
			
	if(gPowerTrq.InvPower_si < (gMotorInfo.Power>>2))          //锟斤拷锟斤拷锟斤拷诙疃拷锟斤拷锟�1/4锟脚硷拷锟斤拷
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
	else if(gPowerTrq.InvPower_si > ((gMotorInfo.Power * 5)>>4)&&((gIMTSet.M>>12)>-500L))  //锟斤拷锟节额定锟斤拷锟斤拷1/3锟斤拷没锟叫猴拷锟斤拷锟斤拷锟斤拷锟�
	{			
        m_s32 = ((s32)gPowerTrq.Volt_Ft4 * gMotorInfo.Votage * 10L) >> 12;   
        m_s32 = m_s32 * qsin(16384 - gPowerTrq.anglePF) >> 15;
		gEstBemf.BemfVoltOnline = abs(m_s32 * gMotorInfo.FreqPer / gRotorSpeed.SpeedBigFilter);
	    gEstBemf.BemfVoltFilter = Filter32(gEstBemf.BemfVoltOnline , gEstBemf.BemfVoltFilter);
        gEstBemf.BemfVoltFilter = __IQsat(gEstBemf.BemfVoltFilter,4000,2500);	
	}
    m_CntTime++;
	if(m_CntTime > 4096)   // 每8s锟斤拷锟斤拷一锟斤拷
    {
        m_CntTime = 0;				
        gEstBemf.BemfVoltDisPlay = gEstBemf.BemfVoltDisPlayTotal>>12; 	         
        if(gEstBemf.BemfVoltDisPlay >= gEstBemf.BemfVoltDisPlayLast + 20)
		{
		    if(gEstBemf.BemfVoltDisPlay >= gEstBemf.BemfVoltDisPlayLast + 50)    //锟秸匡拷始锟斤拷锟斤拷锟饺较匡拷
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
       &&(INV_VOLTAGE_380V == gInvInfo.InvVoltageType))   //svc时锟斤拷锟节磁筹拷锟斤拷锟斤拷
	{
		m_s32 = ((s32)gEstBemf.BemfVoltDisPlayLast<<10)/gMotorInfo.Votage;
	    m_s32 = (m_s32 * (s32)gBasePar.FullFreq)/((s32)gMotorInfo.Frequency * 10);
		gPmsmRotorPosEst.InvOfKf = 1048576L/m_s32; 
	}
}


