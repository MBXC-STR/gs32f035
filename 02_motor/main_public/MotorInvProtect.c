/************************************************************
锟侥硷拷锟斤拷锟斤拷:锟斤拷锟斤拷捅锟狡碉拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
锟侥硷拷锟芥本锟斤拷
锟斤拷锟铰革拷锟铰ｏ拷

*************************************************************/
#include "MotorInvProtectInclude.h"

// // 全锟街憋拷锟斤拷锟斤拷锟斤拷
OVER_LOAD_PROTECT		gOverLoad;
PHASE_LOSE_STRUCT		gPhaseLose;
BEFORE_RUN_PHASE_LOSE_STRUCT gBforeRunPhaseLose;
INPUT_LOSE_STRUCT		gInLose;
#if (SOFT_INPUT_DETECT == STLEN)
SOFT_INPUT_LOSE_STRUCT		gSoftInLose;
#endif
LOAD_LOSE_STRUCT		gLoadLose;
FAN_CTRL_STRUCT			gFanCtrl;
Ulong					gBuffResCnt;	//锟斤拷锟斤拷锟斤拷璞ｏ拷锟斤拷锟斤拷锟�
CBC_PROTECT_STRUCT		gCBCProtect;
struct FAULT_CODE_INFOR_STRUCT_DEF  gError;
extern void FanSpeedCtrl(void);
//锟斤拷频锟斤拷锟酵碉拷锟斤拷墓锟斤拷乇锟�
Uint const gInvOverLoadTable[10] =      /*锟斤拷锟斤拷锟斤拷10锟斤拷锟斤拷锟斤拷锟斤拷9%锟斤拷锟斤拷锟斤拷小值:115%锟斤拷锟斤拷锟斤拷锟街�:196%*/
{
		36000,				//115%锟斤拷频锟斤拷锟斤拷锟斤拷   		1小时锟斤拷锟斤拷
		18000,				//124%锟斤拷频锟斤拷锟斤拷锟斤拷	  	30锟斤拷锟接癸拷锟斤拷
        6000,				//133%锟斤拷频锟斤拷锟斤拷锟斤拷	  	10锟斤拷锟接癸拷锟斤拷
        1800,				//142%锟斤拷频锟斤拷锟斤拷锟斤拷	  	3锟斤拷锟接癸拷锟斤拷
        600,				//151%锟斤拷频锟斤拷锟斤拷锟斤拷   		1锟斤拷锟接癸拷锟斤拷
        200,				//160%锟斤拷频锟斤拷锟斤拷锟斤拷   		20锟斤拷锟斤拷锟�
        120,				//169%锟斤拷频锟斤拷锟斤拷锟斤拷   		12锟斤拷锟斤拷锟�
        20,					//178%锟斤拷频锟斤拷锟斤拷锟斤拷   		6锟斤拷锟斤拷锟�    锟斤拷为178% 2S锟斤拷锟斤拷
        20,					//187%锟斤拷频锟斤拷锟斤拷锟斤拷   		2锟斤拷锟斤拷锟�
        5,					//196%锟斤拷频锟斤拷锟斤拷锟斤拷   		0.5锟斤拷锟斤拷锟�   锟斤拷锟斤拷一锟斤拷锟斤拷锟街�      2011-10-21-chzq
};
#if(SOFTSERIES == MD380SOFT)
Uint const gInvOverLoadTableForP[10] =       /*锟斤拷锟斤拷锟斤拷10锟斤拷锟斤拷锟斤拷锟斤拷4%锟斤拷锟斤拷锟斤拷小值:105%锟斤拷锟斤拷锟斤拷锟街�:141%*/
{
		36000,				//105%锟斤拷频锟斤拷锟斤拷锟斤拷   		1小时锟斤拷锟斤拷
		15000,				//109%锟斤拷频锟斤拷锟斤拷锟斤拷	  	25锟斤拷锟接癸拷锟斤拷
        6000,				//113%锟斤拷频锟斤拷锟斤拷锟斤拷	  	10锟斤拷锟接癸拷锟斤拷
        1800,				//117%锟斤拷频锟斤拷锟斤拷锟斤拷	  	3锟斤拷锟接癸拷锟斤拷
        600,				//121%锟斤拷频锟斤拷锟斤拷锟斤拷   		1锟斤拷锟接癸拷锟斤拷
        300,				//125%锟斤拷频锟斤拷锟斤拷锟斤拷   		30锟斤拷锟斤拷锟�
        100,				//129%锟斤拷频锟斤拷锟斤拷锟斤拷   		10锟斤拷锟斤拷锟�
        30,					//133%锟斤拷频锟斤拷锟斤拷锟斤拷   		3锟斤拷锟斤拷锟�
        10,					//137%锟斤拷频锟斤拷锟斤拷锟斤拷   		1锟斤拷锟斤拷锟�
        5,					//141%锟斤拷频锟斤拷锟斤拷锟斤拷   		0.5锟斤拷锟斤拷锟�     锟斤拷锟斤拷一锟斤拷锟斤拷锟街�   2011-10-21-chzq
};
#else
Uint const gInvOverLoadTableForP[9] =       /*锟斤拷锟斤拷锟斤拷10锟斤拷锟斤拷锟斤拷锟斤拷4%锟斤拷锟斤拷锟斤拷小值:105%锟斤拷锟斤拷锟斤拷锟街�:141%*/
{
		18000,				//110%锟斤拷频锟斤拷锟斤拷锟斤拷   30.0锟斤拷锟斤拷
        6000,				//117%锟斤拷频锟斤拷锟斤拷锟斤拷   10.0锟斤拷锟斤拷
        1800,				//124%锟斤拷频锟斤拷锟斤拷锟斤拷   3.0锟斤拷锟斤拷
        600,				//131%锟斤拷频锟斤拷锟斤拷锟斤拷   1.0锟斤拷锟斤拷
        400,				//138%锟斤拷频锟斤拷锟斤拷锟斤拷   40 锟斤拷
        250,				//145%锟斤拷频锟斤拷锟斤拷锟斤拷   25 锟斤拷
        90,					//152%锟斤拷频锟斤拷锟斤拷锟斤拷   9 锟斤拷
        50,					//159%锟斤拷频锟斤拷锟斤拷锟斤拷   5 锟斤拷
        20					//166%锟斤拷频锟斤拷锟斤拷锟斤拷   2 锟斤拷
};
#endif
//锟斤拷频锟斤拷锟斤拷锟斤拷锟桔硷拷锟斤拷锟斤拷锟斤拷锟斤拷系锟斤拷
Ulong const gInvOverLoadDecTable[12] =
{
        (65536L*60/7),      //0%锟斤拷频锟斤拷锟斤拷锟斤拷    0.7锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/8),		//10%锟斤拷频锟斤拷锟斤拷锟斤拷   0.8锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/9),		//20%锟斤拷频锟斤拷锟斤拷锟斤拷   0.9锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/10),		//30%锟斤拷频锟斤拷锟斤拷锟斤拷   1.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/11),		//40%锟斤拷频锟斤拷锟斤拷锟斤拷   1.1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/13),		//50%锟斤拷频锟斤拷锟斤拷锟斤拷   1.3锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/16),		//60%锟斤拷频锟斤拷锟斤拷锟斤拷   1.6锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/19),		//70%锟斤拷频锟斤拷锟斤拷锟斤拷   1.9锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/24),		//80%锟斤拷频锟斤拷锟斤拷锟斤拷   2.4锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/34),		//90%锟斤拷频锟斤拷锟斤拷锟斤拷   3.4锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
		(65536L*60/56),		//100%锟斤拷频锟斤拷锟斤拷锟斤拷  5.6锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
};
Uint const gInvBreakOverLoadTable[5] = 
{
    1,                      // 100% 10S锟斤拷锟斤拷锟斤拷 10s锟斤拷锟斤拷锟斤拷   110%  2s锟斤拷锟斤拷
    4,                      // 90% 10S锟斤拷锟斤拷锟斤拷 40s锟斤拷锟斤拷锟斤拷    120%  0.5s锟斤拷锟斤拷
    30,                     // 80% 10S锟斤拷锟斤拷锟斤拷 300s锟斤拷锟斤拷锟斤拷
    200,                    // 70% 10S锟斤拷锟斤拷锟斤拷 2000s锟斤拷锟斤拷锟斤拷
    2000,                   // 60% 10S锟斤拷锟斤拷锟斤拷 20000s锟斤拷锟斤拷锟斤拷
};
/*#if (SOFTSERIES == MD500SOFT)
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
*/
#define C_MOTOR_OV_TAB_NUM      14
Uint const gMotorOverLoadBaseTable[C_MOTOR_OV_TAB_NUM] =
{
		480,			//115%锟斤拷锟斤拷锟斤拷锟�  1小时20锟斤拷锟接癸拷锟斤拷
		240,			//125%锟斤拷锟斤拷锟斤拷锟�  40锟斤拷锟接癸拷锟斤拷
		90,				//135%锟斤拷锟斤拷锟斤拷锟�  15锟斤拷锟接癸拷锟斤拷
		36,				//145%锟斤拷锟斤拷锟斤拷锟�  6锟斤拷锟接癸拷锟斤拷
		24,				//155%锟斤拷锟斤拷锟斤拷锟�  4锟斤拷锟接癸拷锟斤拷
		15,				//165%锟斤拷锟斤拷锟斤拷锟�  2.5锟斤拷锟接癸拷锟斤拷
		12,				//175%锟斤拷锟斤拷锟斤拷锟�  2锟斤拷锟接癸拷锟斤拷

		 9,				//185%锟斤拷锟斤拷锟斤拷锟�  1.5锟斤拷锟接癸拷锟斤拷
		 6,				//195%锟斤拷锟斤拷锟斤拷锟�  1锟斤拷锟接癸拷锟斤拷
		 5,				//205%锟斤拷锟斤拷锟斤拷锟�  50S锟斤拷锟斤拷
		 4,				//215%锟斤拷锟斤拷锟斤拷锟�  40S锟斤拷锟斤拷
		 3,				//225%锟斤拷锟斤拷锟斤拷锟�  30S锟斤拷锟斤拷
		 2,				//235%锟斤拷锟斤拷锟斤拷锟�  20S锟斤拷锟斤拷
		 1				//245%锟斤拷锟斤拷锟斤拷锟�  10S锟斤拷锟斤拷
};
Uint gMotorOverLoadTable[C_MOTOR_OV_TAB_NUM] =
{
		48000,				//115%锟斤拷锟斤拷锟斤拷锟�  1小时20锟斤拷锟接癸拷锟斤拷
		24000,				//125%锟斤拷锟斤拷锟斤拷锟�  40锟斤拷锟接癸拷锟斤拷
		9000,				//135%锟斤拷锟斤拷锟斤拷锟�  15锟斤拷锟接癸拷锟斤拷
		3600,				//145%锟斤拷锟斤拷锟斤拷锟�  6锟斤拷锟接癸拷锟斤拷
		2400,				//155%锟斤拷锟斤拷锟斤拷锟�  4锟斤拷锟接癸拷锟斤拷
		1500,				//165%锟斤拷锟斤拷锟斤拷锟�  2.5锟斤拷锟接癸拷锟斤拷
		1200,				//175%锟斤拷锟斤拷锟斤拷锟�  2锟斤拷锟接癸拷锟斤拷

		 900,				//185%锟斤拷锟斤拷锟斤拷锟�  1.5锟斤拷锟接癸拷锟斤拷
		 600,				//195%锟斤拷锟斤拷锟斤拷锟�  1锟斤拷锟接癸拷锟斤拷
		 500,				//205%锟斤拷锟斤拷锟斤拷锟�  50S锟斤拷锟斤拷
		 400,				//215%锟斤拷锟斤拷锟斤拷锟�  40S锟斤拷锟斤拷
		 300,				//225%锟斤拷锟斤拷锟斤拷锟�  30S锟斤拷锟斤拷
		 200,				//235%锟斤拷锟斤拷锟斤拷锟�  20S锟斤拷锟斤拷
		 100				//245%锟斤拷锟斤拷锟斤拷锟�  10S锟斤拷锟斤拷
};

#define C_MOTOR_OV_MAX_CUR      2450
#define C_MOTOR_OV_MIN_CUR      1150
#define C_MOTOR_OV_STEP_CUR     100

//锟斤拷锟斤拷锟斤拷锟斤拷奂锟斤拷锟斤拷锟斤拷锟斤拷锟较碉拷锟�
Ulong const gMotorOverLoadDecTable[12] =
{
        (65536L*60/30),     //0%锟斤拷锟斤拷锟斤拷锟�    3.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/40),		//10%锟斤拷锟斤拷锟斤拷锟�   4.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/50),		//20%锟斤拷锟斤拷锟斤拷锟�   5.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/60),		//30%锟斤拷锟斤拷锟斤拷锟�   6.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/70),		//40%锟斤拷锟斤拷锟斤拷锟�   7.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/80),		//50%锟斤拷锟斤拷锟斤拷锟�   8.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷?
        (65536L*60/90),		//60%锟斤拷锟斤拷锟斤拷锟�   9.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/100),	//70%锟斤拷锟斤拷锟斤拷锟�   10.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/110),	//80%锟斤拷锟斤拷锟斤拷锟�   11.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        (65536L*60/120),	//90%锟斤拷锟斤拷锟斤拷锟�   12.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
		(65536L*60/130),	//100%锟斤拷锟斤拷锟斤拷锟�  13.0锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
};

#if (SOFTSERIES == MD500SOFT)
// //锟铰讹拷锟斤拷锟竭憋拷
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
//MD500锟斤拷同锟斤拷锟酵的癸拷锟铰点不一锟斤拷
Uint const gTempProtectTable[19] =
{
        95,95,110,100,100,         //16-20
        90, 95, 100,95,95,             //21-25
        100, 90, 90,95,90,             //26-30
        93, 97, 103 ,103,             //31-34
};
s16 const gFanSpeedCtrlTempreture[9]=
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
// //锟铰讹拷锟斤拷锟竭憋拷
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


// //锟侥硷拷锟节诧拷锟斤拷锟斤拷锟斤拷锟斤拷
void    SoftWareErrorDeal(void);	
void	TemperatureCheck(void);					//锟铰度硷拷锟�
void	OutputPhaseLoseDetect(void);			//锟斤拷锟饺憋拷锟斤拷锟�
void 	OutputLoseReset(void);		
void	InputPhaseLoseDetect(void);				//锟斤拷锟斤拷缺锟斤拷锟斤拷

#if (SOFT_INPUT_DETECT == STLEN)
void    SoftInputPhaseLoseUdcCal(void);
void    SoftInputPhaseLoseDetect(void);
#endif
void	ControlFan(void);						//锟斤拷锟饺匡拷锟斤拷
void	OverLoadProtect(void);					//锟斤拷锟截憋拷锟斤拷
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
	锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
************************************************************/
//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷珊锟斤拷锟�
void MotorOverLoadTimeGenerate(void)
{
	u16 i;
	u32 mMotorOverLoadTime;

	for(i=0;i<14;i++)
	{
		mMotorOverLoadTime = gMotorOverLoadBaseTable[i]*(u32)gComPar.MotorOvLoad;

		if(mMotorOverLoadTime > 48000)//锟斤拷锟斤拷80锟斤拷锟斤拷
        {
			mMotorOverLoadTime = 48000;
		}
		else if(mMotorOverLoadTime < 100)//锟斤拷锟斤拷10S
		{
        	mMotorOverLoadTime = 100;
		}

		gMotorOverLoadTable[i] = mMotorOverLoadTime;
	}
}
void InvDeviceControl(void)			
{
	SoftWareErrorDeal();				//锟斤拷锟斤拷锟斤拷锟斤拷
	TemperatureCheck();					//锟铰度硷拷锟�
	OutputPhaseLoseDetect();			//锟斤拷锟饺憋拷锟斤拷锟�
#if (SOFTSERIES == MD380SOFT)
	InputPhaseLoseDetect();				//锟斤拷锟斤拷缺锟斤拷锟斤拷
#endif
	ControlFan();						//锟斤拷锟饺匡拷锟斤拷
#if (SOFTSERIES == MD500SOFT)
	CutDownCurForOverLoad();            //锟斤拷锟截斤拷锟筋处锟斤拷
	BrakeOverloadProtect();
#endif
	OverLoadProtect();					//锟斤拷锟截憋拷锟斤拷

    CBCLimitCurPrepare();					//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	CBCLimitCurProtect();				//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷碌墓锟斤拷锟斤拷卸锟�

	BufferResProtect();
#if (SOFTSERIES == MD380SOFT)
    BrakeResShortProtect();             //锟狡讹拷锟斤拷锟斤拷锟铰凤拷锟斤拷锟斤拷锟斤拷锟�
#endif
    EncodeLineDiscProtect();
}

/*************************************************************
	锟斤拷锟斤拷锟斤拷锟较达拷锟斤拷
停锟斤拷状态也锟斤拷锟杰憋拷锟斤拷锟斤拷锟斤拷锟较★拷锟斤拷压锟斤拷锟斤拷
*************************************************************/
void SoftWareErrorDeal(void)					
{
// 锟斤拷始锟叫讹拷锟斤拷锟斤拷锟斤拷锟斤拷
#if (SOFTSERIES == MD380SOFT)     
	if((gUDC.uDCFilter > gInvInfo.InvUpUDC) || //锟斤拷压锟叫讹拷,使锟矫达拷锟剿诧拷锟斤拷压
	    GetOverUdcFlag())
	{
	    DisableDrive(); //停锟斤拷也锟斤拷锟斤拷锟斤拷锟斤拷压锟斤拷锟斤拷锟斤拷锟矫伙拷锟斤拷锟斤拷锟窖癸拷锟斤拷锟�
	    gError.ErrorCode.all |= ERROR_OVER_UDC;
        if((gUDC.uDCFilter > gInvInfo.InvUpUDC) && (!GetOverUdcFlag())
            &&(gMainStatus.ErrFlag.bit.OvCurFlag == 0))
        {
            gError.ErrorInfo[0].bit.Fault2 = 2;//锟斤拷锟斤拷锟斤拷压
        }
        else
        {
            gError.ErrorInfo[0].bit.Fault2 = 1; //硬锟斤拷锟斤拷压
        }
	}
#else
	if((gUDC.uDCFilter > gInvInfo.InvUpUDC)||(ADC_UDC > 55125))    //55125锟斤拷应860V
	{
	    DisableDrive(); //停锟斤拷也锟斤拷锟斤拷锟斤拷锟斤拷压锟斤拷锟斤拷锟斤拷锟矫伙拷锟斤拷锟斤拷锟窖癸拷锟斤拷锟�
	    gError.ErrorCode.all |= ERROR_OVER_UDC;
        if(gUDC.uDCFilter > gInvInfo.InvUpUDC)
		{
            gError.ErrorInfo[0].bit.Fault2 = 1; //锟斤拷锟斤拷锟斤拷压
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
	else if(gUDC.uDCBigFilter < gInvInfo.InvLowUDC) //欠压锟叫讹拷,使锟矫达拷锟剿诧拷锟斤拷压
	{
	    DisableDrive();
        if(STATUS_GET_PAR == gMainStatus.RunStep)   //锟斤拷锟斤拷锟斤拷锟斤拷锟绞讹拷锟斤拷锟斤拷锟角费癸拷锟斤拷录拇锟斤拷锟斤拷锟斤拷锟斤拷薷暮锟矫伙拷谢指锟�
        {
            EndOfParIdentify();
        }         
        gMainStatus.RunStep = STATUS_LOW_POWER;
        gMainStatus.SubStep = 0;
		DisConnectRelay();	
		gError.ErrorCode.all |= ERROR_LOW_UDC;
        gMainStatus.StatusWord.bit.LowUDC = 0;
#if(AIRCOMPRESSOR == 1)
		if((gMainCmd.RestarCnt <= 3)&&(gMainCmd.Command.bit.Start == 1)&&(gOverLoad.OverLoadPreventEnable == 1))   //锟斤拷锟叫癸拷锟斤拷锟叫憋拷欠压锟斤拷锟较达拷锟斤拷志位
		{
		    gMainStatus.StatusWord.bit.OverLoadPreventState = 2;
		}
#endif
	}

	if(//(gMainStatus.RunStep == STATUS_LOW_POWER) ||                         // 欠压
	   (gError.ErrorCode.all & ERROR_SHORT_EARTH) == ERROR_SHORT_EARTH)
	{
		gUDC.uDCBigFilter = gUDC.uDCFilter;
		return;										//欠压状态锟铰诧拷锟叫讹拷锟斤拷锟斤拷锟斤拷锟斤拷
	}
	if((STATUS_STOP == gMainStatus.RunStep) &&
        (gError.LastErrorCode != gError.ErrorCode.all) && 
        (gError.ErrorCode.all != 0))
	{
	    gError.LastErrorCode = gError.ErrorCode.all;
        if(0 != gError.ErrorCode.all)
        {
			gPhase.IMPhase += 0x4000L;				//锟斤拷锟较革拷位锟斤拷锟斤拷锟叫角度凤拷锟斤拷锟戒化
        }
    }
   
// 锟斤拷始锟斤拷锟较革拷位
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
        //CBC锟叫断革拷位
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

//	MD500锟斤拷锟铰度硷拷夂拷锟�
#if (SOFTSERIES == MD500SOFT)
void TemperatureCheck(void)
{
	Uint    * m_pTable,*m_ErrTempTable;
	long    m_TempAD,m_IndexLow,m_IndexHigh,m_Index,m_ErrTemp;
	Uint    m_LimtCnt;
    s16    mType,m_TempMax,m_TempMin;

    mType = gInvInfo.InvTypeApply;

    if((gInvInfo.InvTypeSet>100) && (gInvInfo.InvTypeSet<200))    //380V锟斤拷锟斤拷为220V锟斤拷锟斤拷锟铰讹拷锟斤拷锟竭达拷锟斤拷
    {
        mType = gInvInfo.InvTypeGaiZhi; 
    }
    m_ErrTempTable = (Uint *)gTempProtectTable;
    
// 锟斤拷锟斤拷锟铰度碉拷锟饺凤拷锟� 18.5KW-110KW锟斤拷要锟斤拷锟�
    if((gInvInfo.InvTypeApply > 15)&&(gInvInfo.InvTypeApply <= 34))
    {
        m_ErrTemp = *(m_ErrTempTable + gInvInfo.InvTypeApply - 16);
    }
    else
    {
        m_ErrTemp = 95;
    }
	gTemperature.OverTempPoint = m_ErrTemp;
	
// 锟铰讹拷锟斤拷锟竭碉拷选锟斤拷
// MD500锟侥癸拷锟绞凤拷围锟节ｏ拷锟脚斤拷锟斤拷锟铰度硷拷锟姐，锟斤拷锟斤拷锟斤拷麓锟斤拷锟�
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
		
		// 硬锟斤拷锟斤拷锟斤拷锟斤拷锟皆硷拷锟斤拷锟斤拷锟斤拷路锟斤拷锟斤拷锟斤拷
	    m_TempAD = gTemperature.TempAD>>4;//锟铰讹拷AD锟斤拷卟锟斤拷锟轿�12位
	
		// 锟斤拷始锟斤拷询锟铰度憋拷
		m_IndexLow  = 0;
		m_IndexHigh = 36;
		m_Index = 18;//锟铰度憋拷锟斤拷锟斤拷址锟斤拷锟斤拷屑锟街�
	    m_TempMax = 124;//锟斤拷锟斤拷露锟�
	    m_TempMin = -20;//锟斤拷锟斤拷露锟�
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
			while(m_LimtCnt < 8)//8为锟斤拷锟街凤拷锟斤拷询锟斤拷锟斤拷锟揭拷锟斤拷锟�
			{
				m_LimtCnt++;					//锟斤拷锟斤拷锟斤拷循锟斤拷
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
		if(abs(mType - gTemperature.TempBak) >= 8)			//锟铰度变化锟斤拷锟斤拷0.5锟饺才革拷值
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
// 锟斤拷始锟斤拷锟铰讹拷锟叫断和憋拷锟斤拷锟斤拷锟斤拷
	gTemperature.ErrCnt++;
	if(gTemperature.Temp < m_ErrTemp)
	{	
		gTemperature.ErrCnt = 0;
	}

	if(gTemperature.ErrCnt >= 5)
	{
		gTemperature.ErrCnt = 0;
		gError.ErrorCode.all |= ERROR_INV_TEMPERTURE;		//锟斤拷锟饺憋拷锟斤拷
	}
}
#else
/************************************************************
MD380锟铰度硷拷锟�,说锟斤拷锟斤拷锟斤拷:

1锟斤拷锟斤拷锟斤拷小锟节碉拷锟斤拷10       (2.2Kw 锟斤拷?
	锟铰讹拷锟斤拷锟斤拷:	     1		TABLE_P44X
				2		TABLE_P8XX
				3		TABLE_SEMIKON
				4		TABLE_BSMXX

2锟斤拷锟斤拷锟斤拷锟节ｏ拷11锟斤拷18之锟斤拷 (11Kw 锟斤拷 30Kw)
	锟铰讹拷锟斤拷锟斤拷:	     1		TABLE_BSMXX
				2		TABLE_P44X
				3~4		TABLE_SEMIKON

3锟斤拷锟斤拷锟斤拷锟节ｏ拷19锟斤拷26之锟戒） (37Kw 锟斤拷 200Kw,锟斤拷锟揭帮拷锟斤拷37, 锟斤拷锟斤拷锟斤拷200)
	锟铰度诧拷锟斤拷锟斤拷锟矫ｏ拷		TABLE_WAIZHI

4锟斤拷锟斤拷锟酵达拷锟节碉拷锟斤拷27		TABLE_BSMXX

5锟斤拷锟斤拷锟酵达拷锟节碉拷锟斤拷27时锟斤拷锟铰度诧拷锟斤拷锟斤拷路锟斤拷同锟斤拷锟斤拷要锟斤拷锟斤拷3.3V锟斤拷3V锟斤拷转锟斤拷锟斤拷

6锟斤拷锟斤拷锟酵达拷锟节碉拷锟斤拷19锟斤拷85锟饺憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷95锟饺憋拷锟斤拷锟斤拷

锟斤拷锟斤拷锟斤拷锟叫凤拷式锟斤拷每4锟斤拷锟斤拷一锟斤拷锟斤拷锟捷ｏ拷锟斤拷始锟斤拷址锟斤拷锟斤拷为12锟饺ｏ拷锟斤拷锟斤拷值为AD锟斤拷锟斤拷值
AD锟斤拷锟斤拷值锟斤拷AD_RESULT>>6
************************************************************/
void TemperatureCheck(void)
{
	Uint    * m_pTable;
	Uint    m_TempAD,m_IndexLow,m_IndexHigh,m_Index,m_ErrTemp;
	Uint    m_LimtCnt;
    Uint    mType;

	//...准锟斤拷锟铰度憋拷,690V锟斤拷锟铰度诧拷锟斤拷锟酵憋拷锟斤拷锟斤拷380V锟斤拷锟酵达拷锟斤拷27锟斤拷一锟斤拷
	if(INV_VOLTAGE_690V == gInvInfo.InvVoltageType)
    {
       mType = 28;
    }
    else  if(INV_VOLTAGE_1140V == gInvInfo.InvVoltageType)
    {
       
       mType = 20;  /*1140V 锟斤拷锟铰碉拷锟斤拷露锟斤拷锟斤拷锟斤拷瓒ㄎ拷锟斤拷锟斤拷露锟斤拷锟斤拷锟� 2011.7.26 L1082*/
    }
    else
    {
        mType = gInvInfo.InvTypeApply;
    }

    if((gInvInfo.InvTypeSet>100) && (gInvInfo.InvTypeSet<200))    //380V锟斤拷锟斤拷为220V锟斤拷锟斤拷锟铰讹拷锟斤拷锟竭达拷锟斤拷
    {
        mType = gInvInfo.InvTypeGaiZhi; 
    }

// 锟斤拷锟斤拷锟铰度碉拷锟饺凤拷锟�
      m_ErrTemp = 95;

// 锟铰讹拷锟斤拷锟竭碉拷选锟斤拷
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

// 硬锟斤拷锟斤拷锟斤拷锟斤拷锟皆硷拷锟斤拷锟斤拷锟斤拷路锟斤拷锟斤拷锟斤拷
    m_TempAD = (gTemperature.TempAD>>6);	

#ifdef  TMS320F2808
    if(mType >= 27) // 3V锟斤拷3.3v 锟斤拷锟斤拷锟斤拷路锟斤拷转锟斤拷
    {
        m_TempAD = ((long)gTemperature.TempAD * 465)>>15; 
    }
#endif

// 锟斤拷始锟斤拷询锟铰度憋拷
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
			m_LimtCnt++;					//锟斤拷锟斤拷锟斤拷循锟斤拷
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
	if((u16)(mType - gTemperature.TempBak) >= 8)			//锟铰度变化锟斤拷锟斤拷0.5锟饺才革拷值
	{
		gTemperature.TempBak = mType;
		gTemperature.Temp = mType>>4;
	}

// 锟斤拷始锟斤拷锟铰讹拷锟叫断和憋拷锟斤拷锟斤拷锟斤拷
	gTemperature.ErrCnt++;
	if(gTemperature.Temp < m_ErrTemp)
	{	
		gTemperature.ErrCnt = 0;
	}

	if(gTemperature.ErrCnt >= 5)
	{
		gTemperature.ErrCnt = 0;
		gError.ErrorCode.all |= ERROR_INV_TEMPERTURE;		//锟斤拷锟饺憋拷锟斤拷
	}

}
#endif
/************************************************************
	锟斤拷锟饺憋拷锟斤拷锟�
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
	   (gMainCmd.FreqReal < 80) ||					//0.8Hz锟斤拷锟铰诧拷锟斤拷锟�
	   (gMainCmd.FreqReal > 50000)||                //500Hz锟斤拷锟斤拷也锟斤拷锟斤拷锟�
	   (gMainCmd.Command.bit.StartDC == 1) ||		//直锟斤拷锟狡讹拷状态锟铰诧拷锟斤拷锟�
	   (gMainCmd.Command.bit.StopDC  == 1) ||
	   ((gMainStatus.RunStep != STATUS_RUN) && 		//锟斤拷锟斤拷锟斤拷锟叫伙拷锟斤拷锟劫讹拷锟斤拷锟斤拷锟阶讹拷
	    (gMainStatus.RunStep != STATUS_SPEED_CHECK)))
	{
		OutputLoseReset();
		return;
	}    

	if((gPhaseLose.Time < 4000000)||(gPhaseLose.Cnt<400))    // 20锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷要200ms锟斤拷锟叫讹拷锟斤拷锟饺憋拷锟�
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
void OutputLoseAdd(void)		//锟斤拷锟饺憋拷锟斤拷锟斤拷奂拥锟斤拷锟斤拷锟斤拷锟�
{
	gPhaseLose.TotalU += abs(gIUVWQ24.U >> 12);
	gPhaseLose.TotalV += abs(gIUVWQ24.V >> 12);
	gPhaseLose.TotalW += abs(gIUVWQ24.W >> 12);

    gPhaseLose.Time += gMainCmd.FreqReal;
	gPhaseLose.Cnt ++;
}

void OutputLoseReset(void)		//锟斤拷锟饺憋拷锟斤拷飧次伙拷拇锟斤拷锟斤拷锟斤拷锟�
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
		//	gMainStatus.PrgStatus.bit.PWMDisable = 1; // 锟斤拷止锟斤拷锟斤拷锟叫断凤拷锟斤拷
			DisableDrive();
			// 锟斤拷锟斤拷前锟皆地讹拷路锟斤拷饧帮拷锟斤拷锟角帮拷锟斤拷缺锟斤拷锟斤拷锟斤拷锟斤拷锟街碉拷募锟斤拷锟�
			// 默锟斤拷为2048(Q12锟斤拷式)锟斤拷锟斤拷锟斤拷锟斤拷小锟斤拷80%锟斤拷频锟斤拷锟筋定锟斤拷锟斤拷锟斤拷小锟斤拷2锟斤拷锟斤拷锟斤拷疃拷锟斤拷锟�
			Data1 = ((Ulong)gInvInfo.InvCurrent*3277)/gMotorInfo.Current;			// 80%锟斤拷频锟斤拷锟筋定锟斤拷锟斤拷, Q12
			Data2 = ((Ulong)gInvInfo.InvCurrent<<12)/gMotorInfo.Current;			// 锟斤拷频锟斤拷锟筋定锟斤拷锟斤拷, Q12
			gBforeRunPhaseLose.CurComperCoff = (Data1 < 2048) ? Data1 : 2048;		// min(50%锟斤拷值锟斤拷锟斤拷, 80% 锟斤拷频锟斤拷锟筋定锟斤拷锟斤拷)
            gBforeRunPhaseLose.CurComperCoffLimit = (Data2 < 3277) ? Data2 : 3277;	// min(80%锟斤拷值锟斤拷锟斤拷, 锟斤拷频锟斤拷锟筋定锟斤拷锟斤拷)
			//gBforeRunPhaseLose.CurComperCoff = (gBforeRunPhaseLose.CurComperCoff<Data2)?gBforeRunPhaseLose.CurComperCoff:Data2;
			EALLOW;
			#ifdef TARGET_GS32
			Interrupt_register(INT_ADCA1, &ShortGnd_ADC_Over_isr);
			#else
			PieVectTable.ADCINT1	= &ShortGnd_ADC_Over_isr; // 锟斤拷锟斤拷前锟皆地讹拷路锟斤拷锟斤拷锟饺憋拷锟斤拷锟紸D锟叫讹拷
			#endif
			EPwm1Regs.TBPRD = 600;	// AD卸为10us
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
			if(gExtendCmd.bit.ShortGndBeforeRun != 1)	// 锟斤拷锟斤拷前锟斤拷锟斤拷锟皆地讹拷路, 直锟接硷拷锟斤拷锟斤拷缺锟斤拷, PWM锟侥达拷锟斤拷锟斤拷锟斤拷
			{
				EPwm1Regs.AQCSFRC.all 	= 0x09;		// A+, B-
				EPwm2Regs.AQCSFRC.all 	= 0x06;			
				EPwm3Regs.AQCSFRC.all 	= 0x0A;

			}
			else // 锟斤拷锟斤拷前锟皆地讹拷路锟斤拷锟�, 锟斤拷志位锟斤拷锟斤拷, PWM锟侥达拷锟斤拷锟斤拷锟斤拷
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
				BeforeRunShortGndDetect();	// 锟斤拷锟斤拷前锟皆地讹拷路锟斤拷锟斤拷卸锟�, 未锟斤拷杉锟斤拷前, 循锟斤拷执锟斤拷
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
				BeforeRunOutputPhaseLoseDetect(); //	锟斤拷锟斤拷前锟斤拷锟饺憋拷锟斤拷锟斤拷卸锟�
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
//	gMainStatus.PrgStatus.bit.PWMDisable = 0;	// 锟街革拷锟斤拷锟斤拷锟叫断凤拷锟斤拷
	gMainStatus.RunStep = STATUS_STOP;
	EALLOW;
	#ifdef TARGET_GS32
    Interrupt_register(INT_ADCA1, &ADC_Over_isr);
    #else
	PieVectTable.ADCINT1     = &ADC_Over_isr;
	#endif
	EDIS;
	InitSetPWM();                       //锟街革拷锟睫改的寄达拷锟斤拷锟斤拷锟斤拷
}

void BeforeRunShortGndDetect(void)
{
	DisableDrive();
//	gBforeRunPhaseLose.ShortGndDetectFlag = 0;
	if((gShortGnd.ocFlag != 0)||
	(gBforeRunPhaseLose.OverFlag == 1)||
//	(gBforeRunPhaseLose.maxIU > (30 * 32))||
	//(gUDC.uDC > gShortGnd.BaseUDC + 650)|| // 锟斤拷时删锟斤拷母锟竭碉拷压锟叫讹拷锟斤拷锟斤拷
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
			gError.ErrorInfo[2].bit.Fault4 =3; //锟斤拷锟斤拷锟斤拷锟斤拷
		}
		/*if(gUDC.uDC > gShortGnd.BaseUDC + 650)  锟斤拷时删锟斤拷母锟竭碉拷压锟叫讹拷锟斤拷锟斤拷
		{
			gError.ErrorInfo[2].bit.Fault4 = 4; //锟斤拷锟斤拷锟斤拷压
		}*/
		if(1 == gShortGnd.ocFlag) 
		{
			gError.ErrorInfo[2].bit.Fault4 = 1; //硬锟斤拷锟斤拷锟斤拷,硬锟斤拷锟斤拷锟饺硷拷锟斤拷
		}
		if(2 == gShortGnd.ocFlag) 
		{
			gError.ErrorInfo[2].bit.Fault4 = 2; //硬锟斤拷锟斤拷压
		}
		gLineCur.ErrorShow = gBforeRunPhaseLose.maxIU;
		gMainStatus.SubStep = 4;					// 确锟斤拷锟皆地讹拷路锟斤拷, 直锟接斤拷锟斤拷锟斤拷獠拷锟斤拷懈锟轿�
	}
	else											// 没锟叫对地讹拷路, 锟斤拷锟斤拷锟饺憋拷锟斤拷锟斤拷锟斤拷眉拇锟斤拷锟�, 锟斤拷锟斤拷锟斤拷一锟斤拷锟斤拷锟�
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
	if((gBforeRunPhaseLose.OverFlag == 1)&&	// 	锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷值锟斤拷锟斤拷为未缺锟斤拷
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
				gError.ErrorCode.all |= ERROR_OUTPUT_LACK_PHASE;//锟斤拷锟斤拷锟饺憋拷锟�
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
	EALLOW;             //28035锟斤拷为EALLOW锟斤拷锟斤拷
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
	if(gBforeRunPhaseLose.ShortGndDetectFlag == 1)   // 锟皆地讹拷路锟斤拷锟�
    {
        OverCur = (gIUVWQ24.U >> 12) + (gIUVWQ24.V >> 12);
        OverCur = OverCur + (gIUVWQ24.W >> 12);
        gBforeRunPhaseLose.CurTotal = abs((s16)OverCur);					// 锟斤拷锟斤拷锟斤拷锟斤拷锟街拷锟�, Q12, pu
        gBforeRunPhaseLose.maxIU    = abs(gIUVWQ24.U >> 12);
    }
    else		// 锟斤拷锟饺憋拷锟斤拷锟�
    {
		gBforeRunPhaseLose.maxIU	= Max(gBforeRunPhaseLose.maxIU,abs(gIUVWQ24.U >> 12));		// U锟斤拷锟斤拷锟斤拷锟斤拷
        gBforeRunPhaseLose.CurTotal = gBforeRunPhaseLose.maxIU;
	}
	if((gBforeRunPhaseLose.maxIU > gBforeRunPhaseLose.CurComperCoff)		// 锟斤拷锟节对地讹拷路锟斤拷锟�, U锟斤拷锟斤拷锟斤拷锟斤拷锟�50%, 锟斤拷锟斤拷锟斤拷锟斤拷锟街拷痛锟斤拷锟�50%, 锟斤拷为锟皆地讹拷路
	 &&(gBforeRunPhaseLose.CurTotal > gBforeRunPhaseLose.CurComperCoff))	// 锟斤拷锟斤拷锟斤拷锟饺憋拷锟斤拷锟�, U锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷50%,
	{
		DisableDrive();
		EALLOW;
		EPwm1Regs.AQCSFRC.all 	= 0x0A;
		EDIS;
		gBforeRunPhaseLose.OverFlag = 1;
	}
	else if((gBforeRunPhaseLose.maxIU > gBforeRunPhaseLose.CurComperCoffLimit)
	&&(gBforeRunPhaseLose.CurTotal < (gBforeRunPhaseLose.CurComperCoffLimit>>1))
	&&(gBforeRunPhaseLose.ShortGndDetectFlag == 1))             // 锟皆地讹拷路锟斤拷锟�, 锟斤拷锟斤拷獾経锟斤拷锟斤拷锟斤拷锟斤拷锟�80%锟斤拷锟斤拷锟斤拷锟斤拷锟叫★拷锟�40%时锟斤拷锟劫凤拷锟斤拷锟斤拷锟�
	{
	    DisableDrive();
		EALLOW;
		EPwm1Regs.AQCSFRC.all 	= 0x0A;
		EDIS;
	}
//	gCpuTime.ADCInt = gCpuTime.ADCIntBase - GetTime();
}
/************************************************************
	锟斤拷锟斤拷缺锟斤拷锟斤拷
	锟脚号的高低碉拷平锟斤拷指锟斤拷锟斤拷锟斤拷锟较碉拷锟脚号ｏ拷锟斤拷锟角碉拷DSP锟斤拷锟脚号ｏ拷锟斤拷DSP锟斤拷锟脚号碉拷平锟洁反
缺锟斤拷锟斤拷原锟斤拷1: MD380锟斤拷
    锟教碉拷锟斤拷锟斤拷锟斤拷锟脚猴拷锟斤拷缺锟斤拷锟脚号革拷锟较ｏ拷 锟斤拷锟斤拷锟斤拷时锟斤拷一直为锟酵ｏ拷
    锟斤拷PL一直为锟竭ｏ拷锟斤拷痰锟斤拷锟矫伙拷锟斤拷锟斤拷希锟�
    锟斤拷PL为锟斤拷锟斤拷锟斤拷锟斤拷缺锟洁；

缺锟斤拷锟斤拷原锟斤拷2锟斤拷MD500硬锟斤拷锟睫改猴拷zzb1812锟斤拷
//锟斤拷要锟斤拷锟斤拷0.5ms锟斤拷执锟斤拷
// 锟斤拷锟斤拷缺锟斤拷720Hz ,1s应锟斤拷锟斤拷720锟斤拷锟斤拷锟节ｏ拷实锟斤拷锟斤拷偏锟筋到500Hz
// 锟斤拷锟斤拷450锟斤拷锟斤拷锟节撅拷锟斤拷为缺锟洁，450Hz
// 缺1锟斤拷时锟脚号诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�6ms锟斤拷锟揭的低碉拷平
// 缺3锟斤拷时为锟斤拷锟斤拷锟斤拷720Hz锟脚猴拷

// 锟斤拷锟斤拷锟斤拷锟�300Hz锟斤拷一锟斤拷冉锟阶既凤拷锟斤拷锟�300锟斤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷锟竭碉拷平为锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
18.5kw 锟斤拷锟较诧拷锟斤拷缺锟斤拷锟铰�
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
	if((gMainCmd.Command.bit.Start == 0)              //停锟斤拷状态\锟斤拷锟斤拷状态\18.5KW锟斤拷锟铰诧拷锟斤拷锟斤拷锟斤拷锟饺憋拷锟�
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
			m_Data = (gInLose.HighLevelCnt + gInLose.LowLevelCnt);//0.5ms锟斤拷执锟斤拷
			gInLose.PLFreqTotal += m_Data;	//锟桔伙拷
			if (gInLose.LowLevelCnt > 8)//4ms锟斤拷锟较的低碉拷平锟斤拷锟斤拷
			{
				gInLose.LongLowLevelCnt ++;
			}
			gInLose.HighLevelCnt = 0;
			gInLose.LowLevelCnt  = 0;
			gInLose.PLFreqCnt++;			
		}
		else if((gInLose.HighLevelCnt > 250)&&(gSubCommand.bit.RelayEnalbe == 1))//锟斤拷锟斤拷0.5s锟接高碉拷平锟斤拷锟斤拷锟斤拷为锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷瞎锟斤拷锟�
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
		if (gInLose.LowLevelCnt > 6000)//0.5ms执锟斤拷3s锟斤拷锟斤拷锟斤拷锟街癸拷锟斤拷锟斤拷锟斤拷刍锟�
		{
			InputPhaseLoseReset();
		}
	}
	
	if(gInLose.PLFreqTotal > 2000) //锟斤拷锟斤拷1s
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
		// 锟斤拷锟斤拷锟斤拷锟�300Hz锟斤拷一锟斤拷冉锟阶既凤拷锟斤拷锟�150锟斤拷锟斤拷锟斤拷
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
锟斤拷锟斤拷锟斤拷锟斤拷缺锟斤拷锟斤拷
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
    if(gSoftInLose.Count_5s >= 10000)										// 5s锟斤拷时
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
        
    if((gMainStatus.RunStep != STATUS_RUN) || 		//锟斤拷锟斤拷锟斤拷锟叫阶段诧拷锟斤拷锟�
       (gSubCommand.bit.InputLost != 1)||
       (gSoftInLose.MaxUdc_100ms - gSoftInLose.MinUdc_100ms <= gSoftInLose.MaxDetaU)||
       (gPowerTrq.InvPower_si <= ((gSoftInLose.HalfInvPowerPU*200)>>10)))     //||         //锟斤拷频锟斤拷锟筋定锟斤拷锟绞碉拷50%=2048*1.732
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
            {	//	锟斤拷一锟轿碉拷锟节比较碉拷压
               gSoftInLose.time1 = GetTime();
               gSoftInLose.Flag = 1;
            }
            else if(gSoftInLose.Flag ==2) 
            {	//	锟节讹拷锟轿碉拷锟节比较碉拷压
               gSoftInLose.time3 = GetTime();
               gSoftInLose.Flag = 3;
            }       
        }
        else if(gSoftInLose.uDC  > u_dvalve)
        {
            if(gSoftInLose.Flag ==1)
            {	//	锟斤拷一锟轿低碉拷压锟街革拷
               gSoftInLose.Flag = 2;  
               gSoftInLose.time2 = GetTime();            
            }	//	锟节讹拷锟轿低碉拷压锟街革拷
            else if(gSoftInLose.Flag ==3)
            {
                gSoftInLose.time4 = GetTime();   
                gSoftInLose.Flag = 0;  
    			//	锟斤拷锟斤拷谐锟斤拷锟斤拷频锟斤拷
                deta_t1 = ((gSoftInLose.time1 - gSoftInLose.time3)&0xFFFFFFFF)>>1;
                deta_t2 = ((gSoftInLose.time2 - gSoftInLose.time4)&0xFFFFFFFF)>>1;
                deta_t = ((deta_t1 + deta_t2)/DSP_CLOCK);
                deta_t = Max(deta_t ,3000);
                deta_t = Min(deta_t ,15000);
                
                gSoftInLose.wait_r += deta_t;
    			//	锟斤拷锟斤拷锟斤拷锟斤拷十锟斤拷锟斤拷谐锟斤拷频锟绞碉拷平锟斤拷值
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
                	//	锟斤拷锟斤拷10锟轿讹拷锟节凤拷围锟节ｏ拷锟斤拷缺锟斤拷
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
    // 锟斤拷锟斤拷锟斤拷锟斤拷锟叫断憋拷志锟斤拷龋锟斤拷锟斤拷锟�  20S 锟斤拷锟斤拷
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
	if(((STATUS_RUN!=gMainStatus.RunStep)&&(STATUS_SPEED_CHECK!=gMainStatus.RunStep)  //锟斤拷锟叫伙拷转锟劫革拷锟斤拷状态锟脚硷拷锟�
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
    
	if(PL_INPUT_HIGH)           // PL锟角高碉拷平
	{
		gInLose.UpCnt ++;
		gInLose.UpCntRes ++;
	}
    
	if(gInLose.UpCntRes != 0)	// 锟斤拷锟斤拷500ms锟斤拷PL锟酵碉拷平锟叫讹拷为锟教碉拷锟斤拷锟斤拷锟斤拷
	{
		gInLose.CntRes++;	
		if(gInLose.CntRes >= 250)       // 500ms
		{
			if((gSubCommand.bit.RelayEnalbe == 1) &&          //F9-12-十位
				(gInLose.UpCntRes >= 249) && (gInvInfo.InvTypeApply >= 19))
			{
			    gError.ErrorCode.all |= ERROR_RESISTANCE_CONTACK;
			}
			gInLose.CntRes = 0;
			gInLose.UpCntRes = 0;
		}
	}

	gInLose.Cnt++;	
	if(gInLose.Cnt < 500)       //  缺锟斤拷锟斤拷1sec一锟斤拷循锟斤拷
    {
        return;
    }

	if((gSubCommand.bit.InputLost == 1) &&              //F9-12-锟斤拷位
	   (gInLose.UpCnt > 5) && (gInLose.UpCnt < 485))    // 1sec锟斤拷PL锟斤拷锟节低碉拷平锟斤拷锟饺憋拷喾斤拷?
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
	锟斤拷锟斤拷锟斤拷?
1锟斤拷	锟较电缓锟斤拷状态锟斤拷锟斤拷锟斤拷1锟斤拷锟斤拷锟节凤拷锟饺诧拷锟斤拷锟叫ｏ拷
2锟斤拷	锟斤拷锟斤拷状态锟斤拷锟斤拷锟斤拷锟斤拷锟叫ｏ拷
3锟斤拷	直锟斤拷锟狡讹拷锟饺达拷锟节间，锟斤拷锟斤拷锟斤拷锟斤拷
4锟斤拷	模锟斤拷锟铰度碉拷锟斤拷40锟饺ｏ拷锟斤拷锟斤拷停止锟斤拷锟铰度革拷锟斤拷42锟饺凤拷锟斤拷锟斤拷锟叫ｏ拷40锟斤拷42锟斤拷之锟戒不锟戒化锟斤拷
5)	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷10锟斤拷殴乇锟�
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

    if(gFanCtrl.FanDetaTime > 2200)     //执锟叫硷拷锟斤拷锟斤拷锟�2.2ms 锟斤拷为时锟斤拷锟睫凤拷锟斤拷证
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
    s16 m_AdjSpeedTemp;
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
	锟斤拷频锟斤拷锟酵碉拷锟斤拷锟斤拷乇锟斤拷锟�
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

	/************************锟斤拷压锟斤拷锟斤拷锟竭和讹拷转锟斤拷锟斤拷**********************************/
    if(gOverLoad.OverLoadPreventEnable != 0)
	{
	    if((gOverLoad.InvTotal.half.MSW >= 30000L)&&(gMainStatus.RunStep == STATUS_RUN))
		{		    
		    if((gMainCmd.FreqReal < gMainCmd.XiuMianFreq - 200)&&(gMainCmd.XiuMianFreq > 200))  //锟斤拷锟斤拷频锟绞硷拷2Hz
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
        return;		    //每10ms锟斤拷锟揭伙拷?
	}
	gOverLoad.Cnt = 0;

	////////////////////////////////////////////////////////////////
	//选锟斤拷锟斤拷锟斤拷锟斤拷锟�

    if(1 == gInvInfo.GPType)        //G锟酵伙拷锟斤拷锟斤拷锟斤拷锟竭ｏ拷锟斤拷转锟截革拷锟截伙拷锟斤拷
    {
        m_TorCurLine    = (Uint *)gInvOverLoadTable;
        m_TorCurBottom  = 1150;     //115%锟斤拷频锟斤拷锟斤拷锟斤拷
        m_TorCurUpper   = 1960;     //锟斤拷锟街�196%锟斤拷锟斤拷    // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟街�,2011-10-21-chzq
        m_TorCurStep    = 90;
        m_TorCurData    = 5;        //>=196%,0.5s锟斤拷锟斤拷
    }
#if(SOFTSERIES == MD380SOFT)
    else                            //P锟酵伙拷锟斤拷锟斤拷锟斤拷锟竭ｏ拷锟斤拷锟斤拷锟剿拷锟斤拷喔猴拷鼗锟斤拷锟�
    {       
        m_TorCurLine    = (Uint *)gInvOverLoadTableForP;
        m_TorCurBottom  = 1050;
        m_TorCurUpper   = 1410;     //锟斤拷锟街�141%锟斤拷锟斤拷    // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟街�,2011-10-21-chzq
        m_TorCurStep    = 40;
        m_TorCurData    = 5;        //>=141%,0.5s锟斤拷锟斤拷
    }
#else
    else                            //P锟酵伙拷锟斤拷锟斤拷锟斤拷锟竭ｏ拷锟斤拷锟斤拷锟剿拷锟斤拷喔猴拷鼗锟斤拷锟�
    {       
        m_TorCurLine    = (Uint *)gInvOverLoadTableForP;
        m_TorCurBottom  = 1100;
        m_TorCurUpper   = 1660;     //锟斤拷锟街�166%锟斤拷锟斤拷
        m_TorCurStep    = 70;
        m_TorCurData    = 20;        //>=166%,2.0s锟斤拷锟斤拷
    }
#endif

    m_MaxCurLimit = 5000;       // 500% 锟斤拷证锟斤拷锟斤拷锟斤拷锟斤拷
	////////////////////////////////////////////////////////////////
	//锟斤拷始锟叫断憋拷频锟斤拷锟侥癸拷锟斤拷
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
		else if(m_Cur < 1000)       /*小锟节憋拷频锟斤拷锟筋定锟斤拷锟斤拷锟斤拷锟斤拷锟秸碉拷前锟斤拷锟斤拷锟斤拷小锟斤拷锟斤拷锟斤拷锟斤拷锟桔硷拷锟斤拷*/
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
                if(gOverLoad.InvTotal.half.MSW > 10800)     //锟斤拷锟斤拷锟斤拷30%锟斤拷锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷170%
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
    /*****************锟斤拷锟斤拷锟截达拷锟斤拷***********************************/
	if((gOverLoad.OverLoadPreventEnable != 0)&&(gMainStatus.StatusWord.bit.OverLoadPreventState != 2))  //   锟斤拷锟斤拷锟竭癸拷锟较猴拷锟斤拷锟狡碉拷锟斤拷
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
	    //gLineCur.OverLoadMargin = m_OverLoadMargin; //锟斤拷锟斤拷锟斤拷
	    gLineCur.MaxCurLimit = m_MaxCurLimit;
	    gLineCur.MaxCurLimit =  __IQsat(gLineCur.MaxCurLimit,1800,250);
	}
	else
	{
	    gLineCur.MaxCurLimit = 1800;
	}
    /****************************end******************************/
    //锟斤拷频锟斤拷锟斤拷锟斤拷预锟斤拷锟斤拷锟斤拷锟斤拷
	if(((gOverLoad.InvTotal.all + m_LDeta * 1000UL)>>16) > 36000)
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 1;
	}
	else
	{
		gMainStatus.StatusWord.bit.PerOvLoadInv = 0;
	}


	MotorOverLoadTimeGenerate();//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷珊锟斤拷锟�
	////////////////////////////////////////////////////////////////
	//锟斤拷始锟叫断碉拷锟斤拷墓锟斤拷锟�
	//if(gMainCmd.SubCmd.bit.MotorOvLoadEnable == 0)
	if(gSubCommand.bit.MotorOvLoad == 0)
	{
		gOverLoad.MotorTotal.all = 0;
		gMainStatus.StatusWord.bit.PerOvLoadMotor = 0;
		return;
	}

	m_Cur = ((Ulong)gOverLoad.FilterMotorCur * 1000L)>>12;
	//m_Cur = ((Ulong)m_Cur * 100L)/100;//gComPar.MotorOvLoad;          // 锟斤拷锟捷癸拷锟截憋拷锟斤拷系锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
                                                            	//然锟斤拷锟矫该憋拷锟斤拷锟斤拷锟斤拷锟斤拷询锟斤拷锟截憋拷锟斤拷锟斤拷锟斤拷
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
		else if(m_Cur < m_MotorOvLoad*10)                   /*小锟斤拷100%锟筋定锟斤拷锟斤拷锟斤拷锟秸碉拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�*/
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
    //锟斤拷锟斤拷锟斤拷锟皆わ拷锟斤拷锟斤拷锟�?
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
	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷状态锟铰的癸拷锟截憋拷锟斤拷
//锟斤拷锟杰筹拷锟斤拷锟斤拷锟斤拷锟斤拷时锟戒超锟斤拷5000ms锟斤拷锟斤拷锟斤拷锟斤拷应2500锟斤拷2ms
锟斤拷锟斤拷铣锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷150锟斤拷锟斤拷锟斤拷锟斤拷10s锟斤拷锟斤拷锟斤拷锟藉，之前影锟斤拷锟斤拷锟�
************************************************************/
void CBCLimitCurProtect(void)
{
	s16     m_CurU,m_CurV,m_CurW,m_Coff;
	s16	    m_Max,m_Add,m_Sub;
    Uint    m_Limit;
    
	if((gSubCommand.bit.CBCEnable == 1) && (gMainStatus.RunStep != STATUS_GET_PAR) && (gMainStatus.RunStep != STATUS_IPM_INIT_POS) && (gMainStatus.RunStep != STATUS_FLYING_START))   //DQ锟斤拷锟叫和筹拷始位锟矫憋拷识时锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	{
		if(gCBCProtect.EnableFlag == 0) SetCBCEnable();	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	}
	else
	{
		if(gCBCProtect.EnableFlag == 1)  SetCBCDisable();//取锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	}
		
	//锟斤拷始锟街憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷值锟侥伙拷锟斤拷
	m_Coff = (((long)gMotorInfo.Current)<<10) / gInvInfo.InvCurrent;
	m_CurU = (s16)(((long)gIUVWQ12.U * (long)m_Coff)>>10);
	m_CurU = abs(m_CurU);
	m_CurV = (s16)(((long)gIUVWQ12.V * (long)m_Coff)>>10);
	m_CurV = abs(m_CurV);
	m_CurW = (s16)(((long)gIUVWQ12.W * (long)m_Coff)>>10);
	m_CurW = abs(m_CurW);
    
	//锟斤拷始锟叫讹拷锟角凤拷锟斤拷一锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷5锟斤拷
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

	if((gCBCProtect.CntU > 2500) || 		//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷, 锟轿猴拷一锟斤拷锟斤拷锟�5000ms
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
            m_Limit = 150;  //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
        }
        
        if(0 == gCBCProtect.CbcFlag)              // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟藉发锟斤拷
        {
            gCBCProtect.NoCbcTimes++;
            if(5000 <= gCBCProtect.NoCbcTimes)   //锟斤拷锟斤拷10锟斤拷没锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷之前锟斤拷影锟斤拷锟斤拷锟斤拷锟斤拷
            {
                gCBCProtect.NoCbcTimes = 5000;
                gCBCProtect.CbcTimes = 0;
            }
        }
        else                                     // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟藉发锟斤拷
        {
            gCBCProtect.NoCbcTimes = 0;
            gCBCProtect.CbcFlag = 0;
        }

        if(m_Limit <= gCBCProtect.CbcTimes) 
        {
            gError.ErrorCode.all |= ERROR_TRIP_ZONE;
            gCBCProtect.CbcTimes = m_Limit - 1;      //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟睫凤拷锟斤拷位
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

    	if(gCBCProtect.Flag.bit.CBC_U == 1)   //锟斤拷锟斤拷U锟斤拷锟斤拷锟斤拷幕锟斤拷锟�
    	{
    		gCBCProtect.TotalU += m_Add;
    	}
    	else //if(m_CurU < 8688)					//小锟斤拷1.5锟斤拷锟斤拷频锟斤拷锟斤拷值锟斤拷锟斤拷锟斤拷锟桔硷拷值锟捷硷拷 : 4096*1.5*sqrt(2)
    	{
    		gCBCProtect.TotalU -= m_Sub;
    	}

    	if(gCBCProtect.Flag.bit.CBC_V == 1) 	//锟斤拷锟斤拷V锟斤拷锟斤拷锟斤拷幕锟斤拷锟�
    	{
    		gCBCProtect.TotalV += m_Add;
    	}
    	else //if(m_CurV < 8688)					//小锟斤拷1.5锟斤拷锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷锟桔硷拷值锟捷硷拷
    	{
    		gCBCProtect.TotalV -= m_Sub;
    	}

    	if(gCBCProtect.Flag.bit.CBC_W == 1) 	//锟斤拷锟斤拷W锟斤拷锟斤拷锟斤拷幕锟斤拷锟�
    	{
    		gCBCProtect.TotalW += m_Add;
    	}
    	else //if(m_CurW < 8688)					//锟斤拷1.5锟斤拷锟斤拷频锟斤拷锟斤拷锟斤拷锟斤拷锟桔硷拷值锟捷硷拷
    	{
    		gCBCProtect.TotalW -= m_Sub;
    	}

    	gCBCProtect.Flag.all = 0;

    	//锟斤拷锟斤拷锟斤拷锟斤拷值锟睫凤拷
    	gCBCProtect.TotalU = __IQsat(gCBCProtect.TotalU, m_Limit, 0);
    	gCBCProtect.TotalV = __IQsat(gCBCProtect.TotalV, m_Limit, 0);
    	gCBCProtect.TotalW = __IQsat(gCBCProtect.TotalW, m_Limit, 0);

    	m_Max = (gCBCProtect.TotalU > gCBCProtect.TotalV) ? gCBCProtect.TotalU : gCBCProtect.TotalV;
    	m_Max = (m_Max > gCBCProtect.TotalW) ? m_Max : gCBCProtect.TotalW;
        if(m_Max >= m_Limit)      //锟斤拷锟斤拷锟斤拷锟斤拷40锟脚癸拷锟斤拷
        {
            gError.ErrorCode.all |= ERROR_TRIP_ZONE;
        }          
    }
}

/*************************************************************
	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
*************************************************************/
void SetCBCEnable(void)
{
	gCBCProtect.EnableFlag = 1;
    EALLOW;
    
#ifdef TMS320F2808	
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_ENABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_ENABLE;		// EPWM1锟斤拷锟斤拷锟斤拷锟斤拷
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
	锟截憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
*************************************************************/
void SetCBCDisable(void)
{
	gCBCProtect.EnableFlag = 0;

	EALLOW;
    EPwm2Regs.TZEINT.all = 0x0000;//锟斤拷止TZ2锟叫断ｏ拷锟斤拷锟斤拷锟斤拷锟叫讹拷
#ifdef TMS320F2808
    EPwm1Regs.TZSEL.bit.CBC2 = TZ_DISABLE;
	EPwm1Regs.TZSEL.bit.CBC3 = TZ_DISABLE;		// EPWM1锟斤拷锟斤拷锟斤拷锟斤拷
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
	锟斤拷锟斤拷锟斤拷璞ｏ拷锟斤拷锟斤拷锟�
	
锟斤拷锟斤拷锟侥斤拷锟斤拷欠压状态锟斤拷锟斤拷为锟角伙拷锟斤拷锟斤拷锟斤拷锟斤拷
*************************************************************/
void BufferResProtect(void)
{
	if(gBuffResCnt >= 150000L)			//锟斤拷锟斤拷锟斤拷璞ｏ拷锟斤拷锟斤拷锟�
	{
		gError.ErrorCode.all |= ERROR_RESISTER_HOT;
	}
	gBuffResCnt--;	
	gBuffResCnt = __IQsat(gBuffResCnt,200000L,0);					
}

#if (SOFTSERIES == MD500SOFT)	

/*************************************************************
锟狡讹拷IGBT直通锟叫断ｏ拷锟斤拷锟叫讹拷锟斤拷执锟叫ｏ拷每锟截诧拷锟斤拷锟斤拷执锟斤拷一锟斤拷
没锟叫匡拷通锟斤拷志时锟叫讹拷
没锟叫匡拷通时锟斤拷锟斤拷锟斤拷锟节额定锟斤拷锟斤拷锟斤拷37.5%锟斤拷直通
*************************************************************/
void  BrakeIGBTProtect(void)
{
	Uint m_Mincurrent;
	if(((gBrake.Flag & 0x02) == 0x02) || (gExcursionInfo.IbOkFlag == 0))
	{
		gBrake.IgbtShotTicker = 0;
		return;
	}
	// 锟截讹拷时锟斤拷锟斤拷欠锟斤拷械锟斤拷锟�
	m_Mincurrent = (gInvInfo.InvBreakCurrent * 3) >> 3;
	if (gBrake.IBreak > m_Mincurrent)
	{
		gBrake.IgbtShotTicker += gPWM.gPWMPrdApply;
	}	
	else
	{
		gBrake.IgbtShotTicker = 0;
	}
	// 锟斤拷锟斤拷2s锟斤拷锟斤拷
	if (gBrake.IgbtShotTicker >= 60000000L) // 2s = 2 * 60 000 000/2 
	{	
		gBrake.IgbtShotTicker = 60000000L;
		gError.ErrorCode.all |= ERROR_IGBT_SHORT_BRAKE;	
        gError.ErrorInfo[4].bit.Fault1 = 2;	
	}
	/*锟狡讹拷锟杰癸拷锟斤拷锟叫讹拷,锟斤拷之前锟斤拷锟狡讹拷锟杰癸拷锟截憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟狡碉拷锟矫猴拷锟斤拷锟斤拷*/
	if (gBrake.IBreak >= (gInvInfo.InvBreakMaxCurrent * 2))
	{
		gBrake.FilterTicker ++;			
	}
	//锟斤拷锟�3锟斤拷锟斤拷锟较憋拷锟斤拷锟斤拷
	if (gBrake.FilterTicker >= 3)
	{	
		gBrake.FilterTicker		= 0;
		gError.ErrorCode.all |= ERROR_OVER_CURRENT;
		gError.ErrorInfo[0].bit.Fault1 = 2;//锟狡讹拷锟斤拷锟斤拷锟斤拷锟�
		gLineCur.ErrorShow		= gLineCur.CurPer;
	}
}

/*************************************************************
1锟斤拷锟狡讹拷锟斤拷锟斤拷选取太小
2锟斤拷锟狡讹拷锟斤拷锟斤拷锟较达拷时锟斤拷时锟斤拷铣锟�

*************************************************************/
void  BrakeOverloadProtect(void)
{
    u16 m_CurrCoff;
	Ulong	IBreakrsm,Duty;
	s16	m_Index,m_IndexData;

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
	/*锟狡讹拷锟杰癸拷锟斤拷锟叫讹拷*/
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
    			m_CurrCoff = 1;//锟桔加碉拷锟劫讹拷锟斤拷1000锟斤拷锟斤拷100ms锟斤拷锟斤拷锟斤拷
            }
		}
		else if (m_Index >= 90)    
		{
            m_Index = ((s16)m_Index - (s16)gBrake.minPercent)/gBrake.OverLoadInterval;
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
		gOverLoad.BreakTotal += (2000000L/m_CurrCoff);//锟桔加碉拷锟劫讹拷锟斤拷10锟斤拷锟斤拷10s锟斤拷锟斤拷锟斤拷
	}	
	else if (gOverLoad.BreakTotal > gBrake.OverLoadDegradeCoff)  // 锟斤拷锟斤拷锟斤拷小时锟斤拷锟截硷拷锟斤拷值锟斤拷小
	{
		gOverLoad.BreakTotal -= gBrake.OverLoadDegradeCoff; //锟斤拷小时锟斤拷90s
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
	锟狡讹拷锟斤拷锟斤拷锟斤拷路锟斤拷锟斤拷锟斤拷锟斤拷
	
锟斤拷锟斤拷锟�?
1锟斤拷sizeD\SizeE,锟斤拷7.5KW-30KW锟脚斤拷锟斤拷锟狡讹拷锟斤拷锟斤拷锟铰凤拷锟斤拷锟斤拷锟斤拷
2锟斤拷停锟斤拷状态锟斤拷锟狡讹拷锟斤拷元没锟叫达拷也锟斤拷锟�
锟叫讹拷原锟斤拷:
1)锟斤拷锟斤拷为SizeD锟斤拷SizeE
2)锟斤拷一锟轿斤拷锟斤拷锟斤拷锟斤拷卸希锟街伙拷锟铰硷拷锟斤拷锟斤拷锟斤拷系锟斤拷锟斤拷洗锟斤拷锟斤拷希锟斤拷锟绞憋拷囟锟斤拷贫锟斤拷锟斤拷锟酵憋拷锟斤拷锟酵ｏ拷锟阶刺�
3)10ms锟斤拷锟斤拷TZ锟脚猴拷锟斤拷然为锟酵碉拷平锟斤拷锟斤拷锟斤拷为锟斤拷锟狡讹拷锟斤拷锟斤拷锟铰凤拷锟斤拷锟斤拷锟轿拷锟斤拷锟斤拷锟斤拷锟�
4)锟狡讹拷锟斤拷锟斤拷锟铰凤拷锟斤拷喜锟斤拷锟斤拷锟斤拷侄锟斤拷锟轿伙拷锟街伙拷艿锟斤拷绺次�

*************************************************************/
void  BrakeResShortProtect(void)
{
    if((19 > gInvInfo.InvTypeApply)&&(12 < gInvInfo.InvTypeApply))
    {
        gBrake.BreResShotDetFlag = 1;            //7.5KW-30KW锟斤拷锟斤拷要锟斤拷锟�
    }
    else
    {
        gBrake.BreResShotDetFlag = 0;
    } 

    if((gBrake.BreResShotDetFlag == 1)&&(gBrake.ErrCode == ERROR_OVER_CURRENT))//只锟叫碉拷一锟斤拷为锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷要锟斤拷锟斤拷锟狡讹拷锟斤拷锟斤拷锟铰凤拷锟斤拷锟斤拷
    {                                                          //锟斤拷锟斤拷一锟轿硷拷录为锟斤拷压锟斤拷锟较ｏ拷锟斤拷直锟接憋拷锟斤拷压
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
                gError.ErrorCode.all |= ERROR_SHORT_BRAKE; //60 锟斤拷锟斤拷锟街讹拷锟斤拷位
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
