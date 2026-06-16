/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : f_dspcan.c
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 08/09/2010
* Description        : DSP CAN锟斤拷锟竭底诧拷锟斤拷锟斤拷锟斤拷
*                    : 2013/02/27
                                锟睫改底诧拷硬锟斤拷为锟叫断斤拷锟秸凤拷式
* 锟睫革拷                 1012 20130301 V3.02 锟睫改碉拷远锟斤拷锟斤拷锟街★拷锟斤拷卸锟斤拷诠锟斤拷耍锟斤拷锟街癸拷锟皆讹拷锟斤拷锟斤拷锟斤拷锟�
                                
********************************************************************************/
//#include "DSP28x_Project.h"     							// DSP2803x Headerfile Include File	
//#include "main.h"											// 锟斤拷锟斤拷头锟侥硷拷

#include "f_funcCode.h"
#include "f_dspcan.h"
#include "f_canlink.h"



#define DEBUG_F_CAN              0



#if DEBUG_F_CAN

#if (DSP_CLOCK == 100)                                      // CAN锟斤拷锟斤拷锟斤拷使锟斤拷锟斤拷时锟接ｏ拷KHz锟斤拷位
	#define		DSPCAN_CLK		100000                      // 2808   100M
#else
	#define		DSPCAN_CLK		30000                       // 28035   30M
#endif

const	CAN_BAUD	eCanBaud[CAN_BAUD_SUM] = {
									{(DSPCAN_CLK/20/20)-1, 2, 15},	// 20Kbps	
									{(DSPCAN_CLK/20/50)-1, 2, 15},	// 50Kbps		
									{(DSPCAN_CLK/20/100)-1, 2, 15},	// 100Kbps	
									{(DSPCAN_CLK/20/125)-1, 2, 15},	// 125Kbps		3+14+ 2 +1 = 20
									{(DSPCAN_CLK/20/250)-1, 2, 15},	// 250Kbps		3+14+ 2 +1 = 20
									{(DSPCAN_CLK/20/500)-1, 2, 15},	// 500Kbps		2+15+ 2 +1 = 20  85%
								#if (DSPCAN_CLK == 100000)
                                    {(DSPCAN_CLK/20/1000)-1, 2, 15}
                                #else
									{(DSPCAN_CLK/15/1000)-1, 1, 11} //  1Mbps		1+11+ 2 +1 = 15  86.67%
					    		#endif
								};

Uint32 eCanTranEnFlag;// = 0;
Uint32 eCanReEnFlag;// = 0;

/*******************************************************************************
* 锟斤拷锟斤拷锟斤拷锟斤拷          : interrupt void eCanRxIsr(void)
* 锟斤拷诓锟斤拷锟�			: 锟斤拷
* 锟斤拷锟斤拷				锟斤拷
* 锟斤拷锟斤拷	            : 	
* 锟芥本		        : V0.0.1
* 时锟斤拷              : 01/30/2013
* 说锟斤拷				: eCan锟叫断凤拷式锟斤拷锟斤拷锟斤拷锟捷碉拷锟斤拷锟斤拷
********************************************************************************/
Uint32 recCanCout;
#ifdef TARGET_GS32
__interrupt void eCanRxIsr(void)
#else
interrupt void eCanRxIsr(void)
#endif
{
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
    Uint16 i, mbox, dataTy;
    Uint32 *pi;
    DspCanDataStru *data;

    mbox = ECanaRegs.CANGIF0.bit.MIV0;                      // 锟斤拷取锟叫讹拷锟斤拷锟斤拷锟�

    for (i=0; i<REC_MBOX_NUM-1; i++)
    {
        if ( (CanlinkRecBuf.bufFull & (1<<i) ) == 0)        // 锟叫断碉拷前锟斤拷锟斤拷眨锟斤拷锟斤拷锟�
            break;
    }
    recCanCout++;
    data = (DspCanDataStru *)(& (CanlinkRecBuf.buf[i]) );   

    pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);			
    data->msgid = pi[mbox<<2];							    //  锟斤拷ID锟斤拷锟斤拷锟斤拷锟斤拷
    data->data.mdl= pi[(mbox<<2) + 2];	
    data->data.mdh = pi[(mbox<<2) + 3];
    data->len= pi[(mbox<<2) + 1] & 0xf;                     // 锟斤拷取锟斤拷锟斤拷锟斤拷锟捷筹拷锟斤拷

    dataTy = P2bFilte(data->msgid);
    if (dataTy)                                             // 锟秸碉拷锟斤拷远锟斤拷锟斤拷锟�
    {
        if (dataTy == 0xcc)
            CanlinkRecBuf.bufFull |= (1<<i);                // 锟矫斤拷锟秸伙拷锟斤拷 
    }
    else
        CanlinkRecBuf.bufFull |= (1<<i);                    // 锟矫斤拷锟秸伙拷锟斤拷

    ECANREGS.CANRMP.all = 1ul<<mbox;				        // 锟斤拷锟斤拷锟较拷锟斤拷锟侥达拷锟斤拷

    ECANREGS.CANGIF0.all = 0xffffffff;                      // 锟斤拷锟斤拷锟斤拷锟斤拷卸媳锟街�
    ECANREGS.CANGIF1.all = 0xffffffff;    
#ifdef TARGET_GS32
    
#else
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;                        // Issue PIE ACK
#endif
#ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}

/*******************************************************************************
* 锟斤拷锟斤拷锟斤拷锟斤拷          : void DisableDspCAN(void)
* 锟斤拷诓锟斤拷锟�			: 锟斤拷
* 锟斤拷锟斤拷				锟斤拷
* 锟斤拷锟斤拷	            : 	
* 锟芥本		        : V0.0.1
* 时锟斤拷              : 07/29/2010
* 说锟斤拷				: 锟斤拷止DSP Ecan锟接匡拷
********************************************************************************/
void DisableDspCAN(void)
{
	struct ECAN_REGS ECanaShadow;							// 锟斤拷锟斤拷一锟斤拷影锟接寄达拷锟斤拷锟斤拷某些锟侥达拷锟斤拷只锟斤拷使锟斤拷32位锟斤拷锟斤拷
    ECANREGS.CANTRR.all	= 0xFFFFFFFF;                       // 取锟斤拷锟斤拷锟节斤拷锟叫的凤拷锟斤拷
    EALLOW;
	ECanaShadow.CANMC.all = ECANREGS.CANMC.all;			    // 锟斤拷取CAN锟斤拷锟斤拷锟狡寄达拷锟斤拷
	ECanaShadow.CANMC.bit.CCR = 1;						    // CPU锟斤拷锟斤拷锟睫改诧拷锟斤拷锟绞伙拷全锟斤拷锟斤拷锟轿寄达拷锟斤拷
	ECANREGS.CANMC.all = ECanaShadow.CANMC.all;			    // 锟斤拷写锟斤拷锟狡寄达拷锟斤拷
	EDIS;
}

/*******************************************************************************
* 锟斤拷锟斤拷锟斤拷锟斤拷          : Uint16 InitdspECan(Uint16 baud)
* 锟斤拷诓锟斤拷锟�			: CAN锟接口诧拷锟斤拷锟绞憋拷锟� (eCanBaud锟斤拷锟斤拷)
* 锟斤拷锟斤拷				锟斤拷CAN_INIT_TIME	 锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷
*					  CAN_INIT_SUCC  锟斤拷始锟斤拷锟缴癸拷
*					  CAN_INIT_TIMEOUT 锟斤拷始锟斤拷锟斤拷时
*					  CAN_INIT_BAUD_ERR 锟斤拷锟斤拷锟绞筹拷锟斤拷
* 锟斤拷锟斤拷	            : 	
* 锟芥本		        : V0.0.1
* 时锟斤拷              : 07/29/2010
* 说锟斤拷				: 锟斤拷始锟斤拷DSP Ecan锟接匡拷
********************************************************************************/
#define		IINIT_CAN_TIME				3
Uint16 InitdspECan(Uint16 baud)		// Initialize eCAN-A module
{
	struct ECAN_REGS ECanaShadow;							// 锟斤拷锟斤拷一锟斤拷影锟接寄达拷锟斤拷锟斤拷某些锟侥达拷锟斤拷只锟斤拷使锟斤拷32位锟斤拷锟斤拷
	Uint32 *MsgCtrlPi;										// 锟斤拷始锟斤拷锟斤拷锟斤拷指锟斤拷
	Uint16	i;												// 循锟斤拷锟斤拷锟斤拷
	static	Uint16 con = 0;
	static	Uint16 count = 0;								// 锟斤拷时锟斤拷锟斤拷锟斤拷
	
	if (baud >= CAN_BAUD_SUM)
		return CAN_INIT_BAUD_ERR;							// 锟斤拷锟斤拷锟绞筹拷锟斤拷
	if (count > IINIT_CAN_TIME)								// 锟斤拷始锟斤拷锟斤拷时锟斤拷锟斤拷
    {
    	con = 0;
    	count = 0;
		return  CAN_INIT_TIMEOUT;        
    }

	
	EALLOW;
	if (con == 0)
	{
		GpioCtrlRegs.GPAPUD.bit.GPIO30 = 0;	    // Enable pull-up for GPIO30 (CANRXA)
		GpioCtrlRegs.GPAPUD.bit.GPIO31 = 0;	    // Enable pull-up for GPIO31 (CANTXA)

	/* Set qualification for selected CAN pins to asynch only */
	// Inputs are synchronized to SYSCLKOUT by default.  
	// This will select asynch (no qualification) for the selected pins.

		GpioCtrlRegs.GPAQSEL2.bit.GPIO30 = 3;   // Asynch qual for GPIO30 (CANRXA)   

	/* Configure eCAN-A pins using GPIO regs*/
	// This specifies which of the possible GPIO pins will be eCAN functional pins.

		GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 1;	// Configure GPIO30 for CANTXA operation
		GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 1;	// Configure GPIO31 for CANRXA operation
	
	/* Configure eCAN RX and TX pins for eCAN transmissions using eCAN regs*/  
		ECANREGS.CANTIOC.bit.TXFUNC = 1;
		ECANREGS.CANRIOC.bit.RXFUNC = 1;  

	/* Configure eCAN for HECC mode - (reqd to access mailboxes 16 thru 31) */
										// HECC mode also enables time-stamping feature
		ECanaShadow.CANMC.all = 0;
		ECanaShadow.CANMC.bit.SRES = 1;
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;			// 锟斤拷锟斤拷锟斤拷位CAN模锟斤拷
		
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;			// 锟斤拷取CAN锟斤拷锟斤拷锟狡寄达拷锟斤拷
		ECanaShadow.CANMC.bit.SCB = 1;						// eCAN模式				
		ECanaShadow.CANMC.bit.SUSP = 1;						// 锟斤拷锟借不锟杰碉拷锟斤拷影锟斤拷
//		ECanaShadow.CANMC.bit.DBO = 1;						// 锟斤拷锟斤拷 锟斤拷锟斤拷锟叫� 锟斤拷锟街斤拷锟斤拷前 使锟矫达拷锟侥Ｊ�
		ECanaShadow.CANMC.bit.CCR = 1;						// CPU锟斤拷锟斤拷锟睫改诧拷锟斤拷锟绞伙拷全锟斤拷锟斤拷锟轿寄达拷锟斤拷
		ECanaShadow.CANMC.bit.ABO = 1;						// 锟皆讹拷锟街革拷锟斤拷锟斤拷使锟斤拷
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;			// 锟斤拷写锟斤拷锟狡寄达拷锟斤拷
		
	/* Initialize all bits of 'Master Control Field' to zero */
	// Some bits of MSGCTRL register come up in an unknown state. For proper operation,
	// all bits (including reserved bits) of MSGCTRL must be initialized to zero
		MsgCtrlPi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGCTRL);	// 锟斤拷息锟斤拷锟斤拷锟斤拷指锟斤拷
		for (i=0; i<32; i++)
		{
			MsgCtrlPi[i<<2] = 0x00000000;					// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷息锟斤拷锟狡寄达拷锟斤拷
		}
		MsgCtrlPi = (Uint32 *)(&ECANLAMS.LAM0);				// 息锟斤拷锟斤拷锟斤拷指锟斤拷
		for (i=0; i<32; i++)								// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷渭拇锟斤拷锟�
		{
			MsgCtrlPi[i] = 0x00000000;						// 
		}
		
	/* 
		ECanaMboxes.MBOX0.MSGCTRL.all = 0x00000000;
		..........
		ECanaMboxes.MBOX31.MSGCTRL.all = 0x00000000;
	*/    
	// TAn, RMPn, GIFn bits are all zero upon reset and are cleared again
	//	as a matter of precaution. 
		ECANREGS.CANTRR.all	= 0xFFFFFFFF;					// 锟斤拷位锟斤拷锟斤拷锟斤拷锟斤拷取锟斤拷锟斤拷锟节斤拷锟叫的凤拷锟斤拷
		ECANREGS.CANTA.all	= 0xFFFFFFFF;					// 锟斤拷锟姐发锟斤拷锟斤拷应锟侥达拷锟斤拷/* Clear all TAn bits */      
		ECANREGS.CANRMP.all = 0xFFFFFFFF;					// 锟斤拷锟斤拷锟斤拷息锟斤拷锟斤拷拇锟斤拷锟�/* Clear all RMPn bits */      
		ECANREGS.CANGIF0.all = 0xFFFFFFFF;					// 全锟斤拷锟叫断憋拷志/* Clear all interrupt flag bits */ 
		ECANREGS.CANGIF1.all = 0xFFFFFFFF;
		ECANREGS.CANOPC.all = 0;							// 锟斤拷锟斤拷锟斤拷锟斤拷杀锟斤拷锟斤拷锟�
	/* Configure bit timing parameters for eCANA
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 1 ;            			// CPU锟斤拷锟斤拷锟睫改诧拷锟斤拷锟绞伙拷全锟斤拷锟斤拷锟轿寄达拷锟斤拷
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;
	*/	
		con = 1;											// 锟斤拷一锟阶讹拷锟斤拷锟�
	}
    if (con == 1)
	{
		ECanaShadow.CANES.all = ECANREGS.CANES.all;
		if (ECanaShadow.CANES.bit.CCE == 0 ) 				// Wait for CCE bit to be set..
		{
			count++;
			EDIS;
			return CAN_INIT_TIME;							// 锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷
		}
		else
			con = 2;
	}
	
    if (con == 2)
	{
		ECanaShadow.CANBTC.all = 0;                         // 锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷
		ECanaShadow.CANBTC.bit.BRPREG = eCanBaud[baud].BRPREG;
		ECanaShadow.CANBTC.bit.TSEG2REG = eCanBaud[baud].TSEG2REG;
		ECanaShadow.CANBTC.bit.TSEG1REG = eCanBaud[baud].TSEG1REG; 
		ECanaShadow.CANBTC.bit.SAM = 0;                     // 锟斤拷位锟斤拷锟斤拷
		ECANREGS.CANBTC.all = ECanaShadow.CANBTC.all;
		
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 0 ;            			// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟� Set CCR = 0
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;
		con = 3;
    }
	if (con == 3)
	{
		ECanaShadow.CANES.all = ECANREGS.CANES.all;
		if (ECanaShadow.CANES.bit.CCE != 0 ) 				// Wait for CCE bit to be  cleared..
		{
			count++;
			EDIS;
			return CAN_INIT_TIME;		
		}
	}
    ECANREGS.CANAA.all = 0xffffffff;
/* Disable all Mailboxes  */

	con = 0;
	count = 0;
 	ECANREGS.CANME.all = 0;									// Required before writing the MSGIDs

    EDIS;

// 锟斤拷锟斤拷锟叫断筹拷始锟斤拷
    EALLOW;
	#ifdef TARGET_GS32
	interrupt_disable(INT_CANA0);
	#else
    PieCtrlRegs.PIEIER9.bit.INTx5 = 0;                      // 锟斤拷止锟叫讹拷
    #endif
	ECanaRegs.CANGIM.all = 0;
    ECanaRegs.CANMIM.all = 0;                               // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷
    
    ECanaRegs.CANGIF0.all = 0xffffffff;
    ECanaRegs.CANGIF1.all = 0xffffffff;                     // 锟斤拷锟斤拷卸锟

    ECanaRegs.CANMIL.all = 0;                               // 选锟斤拷EcanA锟叫讹拷0
    ECanaRegs.CANGIM.all = 1;                               // 使锟斤拷锟叫讹拷0

	#ifdef TARGET_GS32
	Interrupt_register(INT_CANA0, &eCanRxIsr);
	#else
	PieVectTable.ECAN0INTA = &eCanRxIsr;                    // CANA 0卸
	#endif
    EDIS;
	#ifdef TARGET_GS32
	interrupt_enable(INT_CANA0);
	#else
    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;                      // 使锟斤拷ECAN1锟叫讹拷
    #endif
    // IER |= M_INT9; 											// Enable CPU INT9
	eCanTranEnFlag = 0;                                     // 锟斤拷锟斤拷锟斤拷锟斤拷始锟斤拷锟斤拷志
	eCanReEnFlag = 0;
	return CAN_INIT_SUCC;									// 锟斤拷始锟斤拷锟缴癸拷 
}	

/*******************************************************************************
* 锟斤拷锟斤拷锟斤拷锟斤拷          : void ErroCountReset(void)
* 锟斤拷诓锟斤拷锟�			: 锟斤拷
* 锟斤拷锟斤拷				锟斤拷0  CAN锟斤拷锟斤拷锟斤拷锟斤拷状态
*                     1  CAN锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷位锟斤拷
* 锟斤拷锟斤拷	            : 	
* 锟芥本		        : V0.0.1
* 时锟斤拷              : 05/16/2012
* 说锟斤拷				: 锟街讹拷锟斤拷位CAN锟斤拷锟竭达拷锟斤拷锟斤拷锟斤拷锟�
********************************************************************************/
Uint16 ErroCountReset(void)
{
    static Uint16 stat = 0;
    struct ECAN_REGS ECanaShadow;							// 锟斤拷锟斤拷一锟斤拷影锟接寄达拷锟斤拷锟斤拷某些锟侥达拷锟斤拷只锟斤拷使锟斤拷32位锟斤拷锟斤拷
    Uint32 canIf;
    
    if (0 == stat)
    {
        canIf = ECANREGS.CANGIF0.all;                       // 锟斤拷取锟叫断憋拷志锟侥达拷锟斤拷
        if (canIf & (7ul<<8) )                              // 锟斤拷锟斤拷锟斤拷锟�>96锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟竭关憋拷
        {
            EALLOW;
            ECANREGS.CANGIF0.all = canIf;                   // 锟斤拷锟斤拷卸媳锟绞�
            ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
            ECanaShadow.CANMC.bit.CCR = 1 ;
            ECANREGS.CANMC.all = ECanaShadow.CANMC.all;
            EDIS;
            stat = 1;                                       // 锟斤拷锟斤拷锟斤拷一状态
        }
    }
    else
    {
        EALLOW;
        ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 0 ;            			// Set CCR = 0  锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;
        EDIS;
        stat = 0;
    }
    return (stat);
}


/*******************************************************************************
* 锟斤拷锟斤拷锟斤拷锟斤拷          : void InitTranMbox(Uint16 mbox)
* 锟斤拷诓锟斤拷锟�			: mbox      锟斤拷锟斤拷锟斤拷 0~31锟斤拷 
*					  *datapi   锟斤拷锟斤拷锟绞硷拷锟斤拷锟斤拷萁峁�
*                     msgid     bit0~bit28  29位帧ID
*                               bit31       锟斤拷展帧锟斤拷识
* 锟斤拷锟斤拷				锟斤拷锟斤拷
* 锟斤拷锟斤拷	            : 	
* 锟芥本		        : V0.0.1
* 时锟斤拷              : 07/29/2010
* 说锟斤拷				: 锟斤拷锟斤拷CAN锟斤拷锟斤拷锟斤拷锟戒，锟缴筹拷始锟斤拷为锟皆讹拷应锟斤拷锟斤拷锟斤拷
********************************************************************************/
void InitTranMbox(Uint16 mbox, DspCanDataStru *datapi)//Uint32 msgid, Uint32 *dataPi)
{
	Uint16 id;
	Uint32 ECanaShadow, *msgIdPi;	                        //指锟诫赋值锟斤拷息ID锟斤拷址

	id = mbox & 0x1f;
	eCanTranEnFlag |= 1ul <<mbox;							// 锟斤拷锟斤拷锟绞硷拷锟斤拷锟斤拷捅锟街�

	msgIdPi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	msgIdPi[id<<2] = datapi->msgid; 					    // 写锟斤拷息锟斤拷志锟斤拷确锟斤拷锟角凤拷为锟皆讹拷应锟斤拷锟斤拷锟斤拷
	msgIdPi[(id<<2) +1] = 8;
	
	ECanaShadow = ECANREGS.CANMD.all;
	ECanaShadow &= ~(1ul<<id);
	ECANREGS.CANMD.all = ECanaShadow;						// 锟斤拷锟斤拷锟斤拷锟斤拷为锟斤拷锟斤拷锟斤拷锟斤拷

	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANME.all = ECanaShadow;						// 使锟杰讹拷应锟斤拷锟斤拷

	msgIdPi[(id<<2) + 2] = datapi->data.mdl;			    // 写锟皆讹拷应锟斤拷锟斤拷息锟斤拷
	msgIdPi[(id<<2) + 3] = datapi->data.mdl;	
	
}

/*******************************************************************************
* 锟斤拷锟斤拷锟斤拷锟斤拷          : void InitReMbox(Uint16 mbox, union CANMSGID_REG msgid, union CANLAM_REG lam)
* 锟斤拷诓锟斤拷锟�			: mbox 锟斤拷锟斤拷锟斤拷 0~31锟斤拷bit7 锟斤拷1锟斤拷 锟斤拷锟斤拷远锟斤拷帧 锟斤拷0锟斤拷锟斤拷通帧	    bit6 "1"锟斤拷锟角憋拷锟斤拷(锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷)
*					  msgid	锟斤拷息锟斤拷识ID
*					  lam	锟斤拷锟斤拷锟斤拷锟轿寄达拷锟斤拷锟斤拷写"0"锟斤拷锟斤拷锟斤拷锟秸癸拷锟斤拷位
* 锟斤拷锟斤拷				锟斤拷锟斤拷
* 锟斤拷锟斤拷	            : 	
* 锟芥本		        : V0.0.1
* 时锟斤拷              : 07/29/2010
* 说锟斤拷				: 锟斤拷锟斤拷CAN锟斤拷锟斤拷锟斤拷锟斤拷
********************************************************************************/
void InitRecMbox(Uint16 mbox, Uint32 msgid, Uint32 lam)
{
	Uint16 id;
	Uint32 ECanaShadow,  *pi;								// = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	
	id = mbox & 0x1f;
	eCanReEnFlag |= 1ul << id;
	
	pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	pi[id<<2] = msgid | 1ul<<30;  							// 锟斤拷息锟斤拷识锟侥达拷锟斤拷锟斤拷使锟矫癸拷锟剿癸拷锟斤拷
	if ((mbox & 0x80) == 0x80)								// 锟斤拷锟斤拷远锟斤拷帧锟斤拷锟斤拷锟绞硷拷锟�
		pi[(id<<2) +1] = 1<<4 | 8;							// 锟斤拷息锟斤拷锟狡寄达拷锟斤拷
	else
		pi[(id<<2) +1] = 8;
		
	ECanaShadow = ECANREGS.CANOPC.all;
	if ( (mbox & 0x40) == 0x40 )							// 使锟杰革拷锟角憋拷锟斤拷锟斤拷椋拷锟斤拷锟绞硷拷锟斤拷锟斤拷丫锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷止锟斤拷锟角憋拷锟斤拷
		ECanaShadow |= 1ul<<id;
	else
		ECanaShadow &= ~(1ul<<id);
	ECANREGS.CANOPC.all = ECanaShadow;
		
	ECanaShadow = ECANREGS.CANMD.all;						// 锟矫★拷1锟斤拷锟斤拷锟斤拷为锟斤拷锟斤拷锟斤拷锟斤拷
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANMD.all = ECanaShadow;						// 
	
	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANME.all = ECanaShadow;						// 使锟杰讹拷应锟斤拷锟斤拷
	
	pi = (Uint32 *)(&ECANLAMS.LAM0);						// 锟斤拷锟矫斤拷锟斤拷锟斤拷锟轿寄达拷锟斤拷
	pi[id] = lam;

    EALLOW;
    ECanaShadow = ECANREGS.CANMIM.all;
	ECanaShadow |= 1ul<<id;
    ECANREGS.CANMIM.all = ECanaShadow;                      // 使锟杰斤拷锟斤拷锟叫讹拷
    EDIS;
}


/*******************************************************************************
* 锟斤拷锟斤拷锟斤拷锟斤拷          : Uint16 eCanDataTran(Uint16 mbox, Uint16 len, Uint32 msgid, Uint32 *dataPi)
* 锟斤拷诓锟斤拷锟�			: mbox      锟斤拷锟斤拷锟斤拷 0~31锟斤拷
*                     len       锟斤拷锟斤拷锟斤拷锟捷筹拷锟斤拷锟街斤拷锟斤拷
*					  msgid     锟斤拷息锟斤拷识ID			只锟斤拷锟斤拷锟斤拷效ID位
*                     dataPi    锟斤拷锟捷伙拷锟斤拷锟斤拷
* 锟斤拷锟斤拷				锟斤拷CAN_MBOX_NUM_ERROR		锟斤拷锟斤拷懦锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟轿达拷锟斤拷锟绞硷拷锟轿拷锟斤拷锟斤拷锟斤拷锟�
*					  CAN_MBOX_BUSY				锟斤拷锟斤拷忙
*					  CAN_MBOX_TRAN_SUCC		锟斤拷锟酵成癸拷
* 锟斤拷锟斤拷	            : 	
* 锟芥本		        : V0.0.1
* 时锟斤拷              : 08/25/2010
* 说锟斤拷				: 指锟斤拷锟斤拷锟戒发锟斤拷锟斤拷锟捷ｏ拷锟斤拷锟斤拷锟斤拷氡伙拷锟绞硷拷锟轿拷锟斤拷锟斤拷锟斤拷锟�
********************************************************************************/
Uint16 eCanDataTran(Uint16 mbox, Uint16 len, DspCanDataStru *data)
{
	Uint32 ECanaShadow, *pi;
    Uint32 msgid;
    

	if ( (eCanTranEnFlag & (1ul << mbox)) != (1ul << mbox) )
	{
		return (CAN_MBOX_NUM_ERROR);						// CAN锟斤拷锟斤拷懦锟斤拷锟斤拷锟斤拷锟斤拷锟轿达拷锟绞硷拷锟�
	}
	
	if (ECANREGS.CANTRS.all & (1ul << mbox))				// 锟斤拷锟斤拷洗畏锟斤拷锟斤拷欠锟斤拷锟缴ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟街撅拷锟轿�
	{
		return (CAN_MBOX_BUSY);								// CAN锟斤拷锟斤拷忙
	}

    msgid = data->msgid;
	mbox &= 0x1f;
   
	ECANREGS.CANTA.all = 1ul << mbox;						// 锟斤拷辗锟斤拷锟斤拷锟接︼拷锟街�
	
	pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);				// 锟斤拷取锟斤拷锟斤拷锟侥达拷锟斤拷锟斤拷址锟斤拷写ID锟斤拷写锟斤拷锟斤拷
	
	msgid &= ~(0x7ul<<29);									// 锟斤拷锟斤拷锟斤拷锟轿�
	msgid |= pi[mbox<<2] & (0x7ul << 29);					// 锟斤拷锟睫革拷ID锟斤拷锟斤拷位
	
	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow &= ~(1ul<<mbox);
	ECANREGS.CANME.all = ECanaShadow;						// 锟斤拷止锟斤拷应锟斤拷锟斤拷
	
	pi[mbox<<2] = msgid;									// 锟斤拷写ID
	pi[(mbox<<2) + 1] = len & 0xf;
	pi[(mbox<<2) + 2] = data->data.mdl;			            // 写锟斤拷锟斤拷
	pi[(mbox<<2) + 3] = data->data.mdh;
	
	ECanaShadow |= 1ul<<mbox;
	ECANREGS.CANME.all = ECanaShadow;						// 使锟杰讹拷应锟斤拷锟斤拷	

	ECANREGS.CANTRS.all = 1ul << mbox;						// 使锟杰凤拷锟斤拷
	return (CAN_MBOX_TRAN_SUCC);
}

/*******************************************************************************
* 锟斤拷锟斤拷锟斤拷锟斤拷          : Uint16 eCanDataRec(Uint16 mbox, Uint32 *dataPi)
* 锟斤拷诓锟斤拷锟�			: mbox      锟斤拷锟斤拷锟斤拷 0~31锟斤拷
*					  *	data    锟斤拷锟秸伙拷锟斤拷
* 锟斤拷锟斤拷				锟斤拷CAN_MBOX_NUM_ERROR		锟斤拷锟斤拷懦锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟轿达拷锟斤拷锟绞硷拷锟轿拷锟斤拷锟斤拷锟斤拷锟�
*					  CAN_MBOX_EMPTY			锟斤拷锟斤拷锟斤拷锟斤拷锟�
*					  CAN_MBOX_REC_SUCC			锟斤拷锟斤拷锟斤拷锟捷成癸拷
*					  CAN_MBOX_REC_OVER			锟斤拷锟斤拷锟斤拷锟斤拷锟叫革拷锟斤拷
* 锟斤拷锟斤拷	            : 	
* 锟芥本		        : V0.0.1
* 时锟斤拷              : 08/25/2010
* 说锟斤拷				: 锟斤拷锟斤拷锟斤拷锟捷斤拷锟秸伙拷锟斤拷锟斤拷
********************************************************************************/
/*
Uint16 eCanDataRec(Uint16 mbox, DspCanDataStru *data)
{
	Uint32 *pi;
	
	mbox &= 0x1f;
//	if ( (eCanReEnFlag & (1ul << mbox)) != (1ul << mbox))
//	{
//		return (CAN_MBOX_NUM_ERROR);						// CAN锟斤拷锟斤拷懦锟斤拷锟斤拷锟斤拷锟斤拷锟轿达拷锟绞硷拷锟�
//	}
	if (ECANREGS.CANRMP.all & (1ul << mbox) )				// 锟斤拷锟斤拷欠锟斤拷薪锟斤拷锟斤拷锟较拷锟斤拷锟�
	{
		pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);			
		data->msgid = pi[mbox<<2];							//  锟斤拷ID锟斤拷锟斤拷锟斤拷锟斤拷
		data->data.mdl= pi[(mbox<<2) + 2];	
		data->data.mdh = pi[(mbox<<2) + 3];
        data->len= pi[(mbox<<2) + 1] & 0xf;                 // 锟斤拷取锟斤拷锟斤拷锟斤拷锟捷筹拷锟斤拷

//		ECanaShadow = 1ul<<mbox;
		
		if (ECANREGS.CANRML.all & (1ul << mbox))			// 锟斤拷锟斤拷锟斤拷锟斤拷欠癖桓锟斤拷枪锟�
		{
			ECANREGS.CANRMP.all = 1ul<<mbox;				// 锟斤拷锟斤拷锟较拷锟斤拷锟侥达拷锟斤拷
			return (CAN_MBOX_REC_OVER);
		}	
		else
		{
			ECANREGS.CANRMP.all = 1ul<<mbox;				// 锟斤拷锟斤拷锟较拷锟斤拷锟侥达拷锟斤拷
			return (CAN_MBOX_REC_SUCC);		
		}
	}
	else
	{
		return (CAN_MBOX_EMPTY);							// CAN锟斤拷锟斤拷眨锟斤拷蘅啥锟饺★拷锟斤拷锟�		
	}
}
*/

/*******************************************************************************
* 锟斤拷锟斤拷锟斤拷锟斤拷          : Uint16 CanMailBoxEmp()
* 锟斤拷诓锟斤拷锟�			: 
* 锟斤拷锟斤拷				: 1  锟叫匡拷锟斤拷
*                   : 0  锟睫匡拷锟斤拷
* 锟斤拷锟斤拷	            : 	
* 锟芥本		        : V0.0.1
* 时锟斤拷              : 08/25/2010
* 说锟斤拷				: CAN 锟斤拷锟斤拷锟斤拷锟斤拷占锟斤拷
********************************************************************************/
#define TX_MAILBOX_MAST (( (1ul << TRAN_MBOX_NUM) - 1) << TRAN_BOX_N)
Uint16 CanMailBoxEmp(void)
{
    if ( (ECANREGS.CANTRS.all & TX_MAILBOX_MAST) == TX_MAILBOX_MAST)
        return 0;                                           // 锟斤拷锟斤拷锟斤拷锟斤拷忙
    else
        return 1;

}



#elif 1



#endif




