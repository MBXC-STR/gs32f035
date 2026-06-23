 /***************************************************************
???????? ???§Ò???????????????ABZ, UVW????????
????·Ú?? 
???????? 
	
****************************************************************/
// ???:
//2808??gpio20-23?????????QEP?????????????????????????spiC???????????? ????????????????gpio??????
//28035??gpio20-23?????????QEP??????gpio???????????????????????????

#include "MotorEncoder.h"
#include "MotorPmsmMain.h"
#include "MotorPmsmParEst.h"
#include "MotorVCInclude.h"
#include "SubPrgInclude.h"

// ?????????? 
volatile struct EQEP_REGS       *EQepRegs;      // QEP ???

//PG_DATA_STRUCT			gPGData;        // ?????????????
IPM_UVW_PG_STRUCT       gUVWPG;         // UVW?????????????
IPM_PG_DIR_STRUCT       gPGDir;         // ????????????????????
//ROTOR_SPEED_STRUCT		gRotorSpeed;    // ?????????????????? 
ROTOR_TRANS_STRUCT		gRotorTrans;    // ???????????¦Ë?¡Â???????
BURR_FILTER_STRUCT		gSpeedFilter;   // ???????????
CUR_LINE_STRUCT_DEF		gSpeedLine;	    // ..

Uint const gUVWAngleTable[6] = {        // UVW ?????????
    //001                //010               //011
    (330*65536L/360), (210*65536L/360),	(270*65536L/360), 
    //100                //101                //110
    (90 *65536L/360), (30 *65536L/360), (150*65536L/360)       
};

// // ???????????
void SpeedSmoothDeal( ROTOR_SPEED_SMOOTH * pSpeedError,s16 iSpeed );
void RotorTransCalVel_old(void);
u16 GetRotorTransPos();
/*************************************************************
    ????????????????? 2808 ??28035?????
*************************************************************/
void InitRtInterface(void)
{
    EALLOW;
#ifdef TMS320F2808                             // 2808 ???spiC??????§à???

	SysCtrlRegs.PCLKCR0.bit.SPICENCLK = 1;      //BIT6 SPI-C
    // ???????????¦Ë??SPI????
    GpioCtrlRegs.GPAPUD.bit.GPIO21  = 0;        // Enable pull-up
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 2;        // SPIC MI 
    GpioCtrlRegs.GPAQSEL2.bit.GPIO21= 3;        // Asynch        
    GpioCtrlRegs.GPAPUD.bit.GPIO22  = 0;        // Enable pull-up
    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 2;        // SPIC CLK 
    GpioCtrlRegs.GPAQSEL2.bit.GPIO22= 3;        // Asynch        
    GpioCtrlRegs.GPAPUD.bit.GPIO23  = 0;        // Enable pull-up
    GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 2;        // SPIC STE
    GpioCtrlRegs.GPAQSEL2.bit.GPIO23= 3;        // Asynch    

    // Initialize SPI FIFO registers
    SpicRegs.SPIFFTX.all=0xE040;
    SpicRegs.SPIFFRX.all=0x405f;                // Receive FIFO reset
    SpicRegs.SPIFFRX.all=0x205f;                // Re-enable transmit FIFO operation
    SpicRegs.SPIFFCT.all=0x0000;

    // Initialize  SPI
    SpicRegs.SPICCR.all =0x000F;                // Reset on, , 16-bit char bits

    #if 1                                       // 12¦ËPG??spi??
    SpicRegs.SPICCR.bit.CLKPOLARITY = 1;        // falling edge receive
    SpicRegs.SPICTL.all =0x000E;                // Enable master mode, SPICLK signal delayed by one half-cycle
                                                // enable talk, and SPI s16 disabled.
                                                // CLOCK PHASE = 1
    #else                                       // 16¦ËPG??spi??
	SpicRegs.SPICCR.bit.CLKPOLARITY = 0;	    //16bit pg
    SpicRegs.SPICTL.all =0x0006;      		    //16bit pg
    #endif
                                       
    // SPI??????    LSPCLK = 100MHz/4
    SpicRegs.SPIBRR = 48;                       // 100/4 * 10^6 / (49)  = 510KHz
    //SpicRegs.SPIBRR = 0x0006;                         // 100/4 * 10^6 / (6+1) = 3.571MHz
    //SpicRegs.SPIBRR = 0x0005;                         // 100/4 * 10^6 / (5+1) = 4.167MHz
    //SpicRegs.SPIBRR = 0x0004;                         // 100/4 * 10^6 / (4+1) = 5.000MHz
    
    SpicRegs.SPICCR.bit.SPISWRESET = 1;     
    SpicRegs.SPIPRI.bit.FREE = 1;          

    // ?????RD???GPIO34 ????
    GpioCtrlRegs.GPBPUD.bit.GPIO34 = 0;         // Enable pull-up       
    GpioCtrlRegs.GPBMUX1.bit.GPIO34= 0;        
    GpioDataRegs.GPBSET.bit.GPIO34 = 1;         // Set sample to High
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;		    // Resolver sample signal config: output
    
#else                                           // 28035 ??????IO?????????

    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0;        // REDVEL\  ->
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;        // SO       <-
    GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0;        // SCLK     ->

    GpioCtrlRegs.GPAQSEL2.bit.GPIO20 = 0x0;              //????Ú…  
    GpioCtrlRegs.GPAQSEL2.bit.GPIO21 = 0x0;              //????Ú…  
    GpioCtrlRegs.GPAQSEL2.bit.GPIO23 = 0x0;              //????Ú…  

    GpioCtrlRegs.GPACTRL.all &= ~(0xFF << 16);
    GpioCtrlRegs.AIOMUX1.bit.AIO10 = 0;         // SAMPLE    AIO-port ->
    GpioCtrlRegs.AIOMUX1.bit.AIO12 = 0;         // RD\       AIO-port ->

    GpioCtrlRegs.GPADIR.bit.GPIO20 = 1;         // 
    GpioCtrlRegs.GPADIR.bit.GPIO21 = 0;         // 
    GpioCtrlRegs.GPADIR.bit.GPIO23 = 1;         //
    GpioCtrlRegs.AIODIR.bit.AIO10 = 1;
    GpioCtrlRegs.AIODIR.bit.AIO12 = 1;

    ROTOR_TRANS_RDVEL = 1;                      // ???????¦Ë??
#endif
    EDIS;
}


/*************************************************************
	????QEP???
*************************************************************/
void InitSetQEP(void)
{  
	EQepRegs = &EQep1Regs;
	/****************?????????wyk******************/
	EALLOW;
	(*EQepRegs).QEPCTL.all = 0;
	(*EQepRegs).QDECCTL.all = 0;
	(*EQepRegs).QEPCTL.bit.PCRM = 1;
	(*EQepRegs).QEPCTL.bit.QPEN = 1;
	(*EQepRegs).QEPCTL.bit.QCLM = 0;
	(*EQepRegs).QEPCTL.bit.IEL = 1;

	(*EQepRegs).QPOSCTL.all = 0;

    (*EQepRegs).QCAPCTL.all = 0;
	(*EQepRegs).QCAPCTL.bit.CEN = 0;
	(*EQepRegs).QCAPCTL.bit.CCPS = 3;		//CAP????????10*4ns
    (*EQepRegs).QCAPCTL.bit.UPPS = 0;	
	(*EQepRegs).QCAPCTL.bit.CEN = 1;
	(*EQepRegs).QPOSCNT = 0;
	(*EQepRegs).QPOSMAX = 0xFFFFFFFF;

	(*EQepRegs).QEINT.all = 0;
	(*EQepRegs).QCLR.all = 0x0FFF;			//????§Ø???

	(*EQepRegs).QCTMR = 0;
	(*EQepRegs).QEPSTS.bit.COEF = 0;
	(*EQepRegs).QEPSTS.bit.CDEF = 0;

	EDIS;

	ResetSpeedGetReg();
	/****************end***********************/
}

//**************************************************************************
// ???PG??????????????????????????

//**************************************************************************
void ReInitForPG(void)
{
    switch(gPGData.PGType)
    {
        case PG_TYPE_UVW:
            gPGData.PGMode = 0;
            EALLOW;
            GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 1;    //eQEP1A
            GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 1;    //eQEP1B
            GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 1;    //eQEP1I
            GpioCtrlRegs.GPAQSEL2.bit.GPIO20 = 0x2;              //????Ú…  
   			GpioCtrlRegs.GPAQSEL2.bit.GPIO21 = 0x2;              //????Ú…  
            GpioCtrlRegs.GPAQSEL2.bit.GPIO23 = 0x2;              //????Ú…  
//			GpioCtrlRegs.GPACTRL.bit.QUALPRD2 = 0x01; //PULS_IN??????5*5*20ns = 300ns????????????????
            GpioCtrlRegs.GPACTRL.all &= ~(0xFF << 16);
			GpioCtrlRegs.GPACTRL.all |= (0x01 << 16);
        #ifdef TMS320F2808
            GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 2;    //eQEP2A
            GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 2;    //eQEP2B
            GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 2;    //eQEP2I
        #endif   
        #ifdef TMS320F28035
            GpioCtrlRegs.AIOMUX1.bit.AIO6  = 2;     // ad uvw???
            GpioCtrlRegs.AIOMUX1.bit.AIO10 = 2;
            GpioCtrlRegs.AIOMUX1.bit.AIO12 = 2;
        #endif
            EDIS;            
            break;
            
        case PG_TYPE_SPECIAL_UVW:
            gPGData.PGMode = 0;
            break;
            
        case PG_TYPE_RESOLVER:      // ????2808??28035????????
            gPGData.PGMode = 1;
            EALLOW;
            #ifdef TARGET_GS32
            SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_EQEP1);
            #else
            SysCtrlRegs.PCLKCR1.bit.EQEP1ENCLK = 0;//?????QEP???
            #endif
        #ifdef TMS320F2808
            #ifdef TARGET_GS32
            SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_EQEP2);
            #else
            SysCtrlRegs.PCLKCR1.bit.EQEP2ENCLK = 0;
            #endif
        #endif
            EDIS;
            
            InitRtInterface();      // ???????????????
            break;
            
        case PG_TYPE_SC:
            gPGData.PGMode = 0;
            break;
            
        default:        // PG_TYPE_ABZ
            gPGData.PGMode = 0;
            EALLOW;
            GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 1;    //eQEP1A
            GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 1;    //eQEP1B
            GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 1;    //eQEP1I
            GpioCtrlRegs.GPAQSEL2.bit.GPIO20 = 0x2;              //????Ú…  
   			GpioCtrlRegs.GPAQSEL2.bit.GPIO21 = 0x2;              //????Ú…  
            GpioCtrlRegs.GPAQSEL2.bit.GPIO23 = 0x2;              //????Ú…  
//			GpioCtrlRegs.GPACTRL.bit.QUALPRD2 = 0x01; //PULS_IN??????5*5*20ns = 300ns????????????????
            GpioCtrlRegs.GPACTRL.all &= ~(0xFF << 16);
			GpioCtrlRegs.GPACTRL.all |= (0x01 << 16);
            #ifdef TMS320F2808
            GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 2;    //eQEP2A
            GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 2;    //eQEP2B
            GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 2;    //eQEP2I
        #endif            
            EDIS;
            break;
    }

    // ????????????????????????????????qep
    if(gPGData.PGMode == 0)
    {
        gPGData.QEPIndex = QEP_SELECT_NONE;
    }
    else        // ????????????????????qep????¡¤
    {
        gPGData.QEPIndex = (QEP_INDEX_ENUM_STRUCT)gExtendCmd.bit.QepIndex;
    }
}

/************************************************************
    ????????§Ö?????(??????)???????AB???????
************************************************************/
s16 JudgeABDir(void)
{
	s16 m_Deta;

	m_Deta = gIPMPos.RotorPos - gPGDir.ABAngleBak;
	if(m_Deta > 1820)                               // 1820 = 10??
	{
		gPGDir.ABDirCnt++;
		gPGDir.ABAngleBak = gIPMPos.RotorPos;
	}
	else if(m_Deta < -1820)
	{
		gPGDir.ABDirCnt--;
		gPGDir.ABAngleBak = gIPMPos.RotorPos;
	}

	if(gPGDir.ABDirCnt > 2)    		         return (DIR_FORWARD);
	else if(gPGDir.ABDirCnt < -2)           return (DIR_BACKWARD);
	else							         return (DIR_ERROR);
}

/************************************************************
    ????????§Ö?????(??????)???????UVW???????
************************************************************/
s16 JudgeUVWDir(void)
{
    s16 m_Deta;
    
    GetUvwPhase(); 
    m_Deta = (s16)gUVWPG.UVWAngle - (s16)gPGDir.UVWAngleBak;
    
    if(m_Deta > 1820)   // 10???10/360  *65536(Q16???)
    {
        if(gPGDir.UVWDirCnt < 32767)
        {
            gPGDir.UVWDirCnt ++;
        }
        gPGDir.UVWAngleBak = gUVWPG.UVWAngle;
    }
    else if(m_Deta < -1820)
    {
        if(gPGDir.UVWDirCnt > -32767)
        {
            gPGDir.UVWDirCnt --;
        }
        gPGDir.UVWAngleBak = gUVWPG.UVWAngle;
    }

	if(gPGDir.UVWDirCnt > 2)    	        return (DIR_FORWARD);
	else if(gPGDir.UVWDirCnt < -2)         return (DIR_BACKWARD);
	else							        return (DIR_ERROR);
}

/*******************************************************************************
	????:        UVW_GetUVWState(void)
	????:        UVW?????????UVW????¦Ë?? ---- ??AD?§Ø??????
	             ???UVW????????UVW?????????1~6???,???????????
	?§Ø????:    ?????§Ø??????????????????????????????
	????:	 
	???:        gUVWPG.UVWAngle
	
	??????:      PG_Zero_isr(void)
*******************************************************************************/
void GetUvwPhase()
{
    u16  m_UVWAngle;
    if((MOTOR_TYPE_PM != gMotorInfo.MotorType) ||
        (gPGData.PGType != PG_TYPE_UVW)   ||
        (gMainCmd.Command.bit.ControlMode != IDC_FVC_CTL))
    {
        return;
    }
        
// ????AD???????UVW????????
#if(AIRCOMPRESSOR == 0)
    gUVWPG.UVWData   = Get_UVW_PG_U() + Get_UVW_PG_V() + Get_UVW_PG_W();
#else
    gUVWPG.UVWData   = 0;
#endif

    if((gUVWPG.UVWData > 0) && (gUVWPG.UVWData < 7))
    {
        m_UVWAngle =  gUVWAngleTable[gUVWPG.UVWData - 1];
    }
    else if((gMainCmd.Command.bit.Start)&&(gUVWPG.ErrorEnable == 1))
    {
        gError.ErrorCode.all |= ERROR_ENCODER;
        if(gUVWPG.UVWData == 7) gError.ErrorInfo[4].bit.Fault3 = 11;     // uvw????????
        else                    gError.ErrorInfo[4].bit.Fault3 = 12;     // uvw????????????????pg?????
    }

    if(gUVWPG.UvwDir == 0)
    {
        gUVWPG.UVWAngle = m_UVWAngle;
    }
    else
    {
        gUVWPG.UVWAngle = 65536U - m_UVWAngle;
    }
}

/************************************************************
    ???????????¦Ë?????
* ?????????????¦Ë??????
* ?????????¦Ë????????????????Z???
************************************************************/
u16 GetRotorTransPos()
{    
	Uint  mData,mBit;
	static Uint m_RtErrTimes = 0;
	//Uint  mWatie;

    mData = 0;
	DINT;
	ROTOR_TRANS_RD=0;		//begin to transmit data  	  
// 1st Fall-edge
	ROTOR_TRANS_SCLK  = 0;	//Set SCLK
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
	mBit   = (ROTOR_TRANS_SO) << 15;     //MSB-bit15	    
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
    NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 2nd Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 14;     // bit14
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
    NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 3rd Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 13;     // bit13
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 4th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 12;     // bit12
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 5th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 11;     // bit11
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 6th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 10;     // bit10
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
    NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 7th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 9;     // bit9
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
    NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 8th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 8;     // bit8
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
// 9th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 7;     // bit7
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 10th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 6;     // bit6
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 11th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 5;     // bit5
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 12th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 4;     // bit4
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 12th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 3;     // bit3
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 12th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 2;     // bit2
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 12th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 1;     // bit1
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 12th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
	mBit   = (ROTOR_TRANS_SO) << 0;     // bit0
	mData  = mData | mBit;	
	ROTOR_TRANS_SCLK = 1;
//    asm(" RPT #5 || NOP ");
	NOP;
	NOP;
	NOP;
	NOP;
	NOP;
// 12th Fall-edge
    ROTOR_TRANS_SCLK  = 0;
    //asm(" RPT #5 || NOP ");
    EINT;

	gRotorTrans.RtorBuffer = (Uint)mData;
    gRotorTrans.RtError = gRotorTrans.RtorBuffer & 0x0006;
    if((gRotorTrans.RtError == 0)&&(IDC_FVC_CTL == gMainCmd.Command.bit.ControlMode)&&(gPGData.DiscDetectDelayTime != 0))
    {
        m_RtErrTimes++;
        if(m_RtErrTimes > 50)
        {
            m_RtErrTimes = 500;
            gError.ErrorCode.all |= ERROR_ENCODER;
            gError.ErrorInfo[4].bit.Fault3 = 13;          
        }
    }
    else
    {
        m_RtErrTimes = 0;
    }
    
    gRotorTrans.RtorBuffer = gRotorTrans.RtorBuffer & 0xFFF0;
	 if(gCtrMotorType != RUN_SYNC_TUNE)
	{
		if( gPGData.SpeedDir )
	    {
			gRotorTrans.RtorBuffer = 0xFFFF - gRotorTrans.RtorBuffer;
		}
	}	 
	else
	{
	    if( gPGData.PGDir )
	    {
			gRotorTrans.RtorBuffer = 0xFFFF - gRotorTrans.RtorBuffer;
		}
	} 


    gRotorTrans.AbsRotPos = gRotorTrans.RtorBuffer >> 4;    // ??????????????????§Ö¦Ë????

    return(gRotorTrans.RtorBuffer);
}
/*************************************************************
	???UVW????????UVW??????¦Ë??
*************************************************************/
u16 IPMCalAbsPos(void)
{
	if(gPGData.PGType == PG_TYPE_UVW)
	{
		GetUvwPhase();
        return (gUVWPG.UVWAngle + gIPMPos.RotorZero);
	}// UVW??????
	else if(gPGData.PGType == PG_TYPE_SPECIAL_UVW)
	{
        gUVWPG.UVWAngle = gUVWAngleTable[gUVWPG.UVWExtData - 1];
        return (gUVWPG.UVWAngle + gIPMPos.RotorZero);
	}// ?????UVW??????   
    else
    {
        return (gIPMPos.RotorPos);
    }// ?????§µ????????????????????????????
}

/***************************************************************************************
1?? ????????????????????????????:FVC??ABZ??????(??????OC??????) 
2??	???CF-20??????????????????¦Ë?ms????§³??????2ms?????????????????
3??	?0????????????????????10s
	z1812
***************************************************************************************/

void EncodeLineDiscProtect(void)
{
	if((PG_TYPE_ABZ == gPGData.PGType)&&(IDC_FVC_CTL == gMainCmd.Command.bit.ControlMode))
	{		
		if(Encode_DisConnect)
		{
			gPGData.Cnt ++;
			if((gPGData.DiscDetectDelayTime != 0) && (gPGData.Cnt >= 10))   // 20ms,????????????????
			{
				gError.ErrorCode.all |= ERROR_ENCODER;
		        gError.ErrorInfo[4].bit.Fault3 = 13;		//?????????????
		        gPGData.Cnt = 0;
			}
            
			if(gPGData.Cnt > 32766)
			{
				gPGData.Cnt = 32766;	
			}
		}
		else
		{
			gPGData.Cnt = 0;			
		}			
	}
}
