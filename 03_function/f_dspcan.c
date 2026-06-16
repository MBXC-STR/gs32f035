/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : f_dspcan.c
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 08/09/2010
* Description        : DSP CAN总线底层驱动库
*                    : 2013/02/27
                                修改底层硬件为中断接收方式
* 修改                 1012 20130301 V3.02 修改点对多数据帧在中断内过滤，防止点对多数据溢出
                                
********************************************************************************/
//#include "DSP28x_Project.h"     							// DSP2803x Headerfile Include File	
//#include "main.h"											// 包含头文件

#include "f_funcCode.h"
#include "f_dspcan.h"
#include "f_canlink.h"



#define DEBUG_F_CAN              1



#if DEBUG_F_CAN

#if (DSP_CLOCK == 100)                                      // CAN控制器使用主时钟，KHz单位
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
* 函数名称          : interrupt void eCanRxIsr(void)
* 入口参数			: 无
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 01/30/2013
* 说明				: eCan中断方式接收数据到缓存
********************************************************************************/
Uint32 recCanCout;
interrupt void eCanRxIsr(void)
{
    Uint16 i, mbox, dataTy;
    Uint32 *pi;
    DspCanDataStru *data;

    mbox = ECanaRegs.CANGIF0.bit.MIV0;                      // 获取中断邮箱号

    for (i=0; i<REC_MBOX_NUM-1; i++)
    {
        if ( (CanlinkRecBuf.bufFull & (1<<i) ) == 0)        // 判断当前缓存空，跳出
            break;
    }
    recCanCout++;
    data = (DspCanDataStru *)(& (CanlinkRecBuf.buf[i]) );   

    pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);			
    data->msgid = pi[mbox<<2];							    //  读ID，读数据
    data->data.mdl= pi[(mbox<<2) + 2];	
    data->data.mdh = pi[(mbox<<2) + 3];
    data->len= pi[(mbox<<2) + 1] & 0xf;                     // 读取接收数据长度

    dataTy = P2bFilte(data->msgid);
    if (dataTy)                                             // 收到点对多数据
    {
        if (dataTy == 0xcc)
            CanlinkRecBuf.bufFull |= (1<<i);                // 置接收缓存 
    }
    else
        CanlinkRecBuf.bufFull |= (1<<i);                    // 置接收缓存

    ECANREGS.CANRMP.all = 1ul<<mbox;				        // 清除消息挂起寄存器

    ECANREGS.CANGIF0.all = 0xffffffff;                      // 清除所有中断标志
    ECANREGS.CANGIF1.all = 0xffffffff;    

    PieCtrlRegs.PIEACK.bit.ACK9 = 1;                        // Issue PIE ACK
}

/*******************************************************************************
* 函数名称          : void DisableDspCAN(void)
* 入口参数			: 无
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: 禁止DSP Ecan接口
********************************************************************************/
void DisableDspCAN(void)
{
	struct ECAN_REGS ECanaShadow;							// 声明一个影子寄存器，某些寄存器只能使用32位操作
    ECANREGS.CANTRR.all	= 0xFFFFFFFF;                       // 取消正在进行的发送
    EALLOW;
	ECanaShadow.CANMC.all = ECANREGS.CANMC.all;			    // 读取CAN主控制寄存器
	ECanaShadow.CANMC.bit.CCR = 1;						    // CPU请求修改波特率或全局屏蔽寄存器
	ECANREGS.CANMC.all = ECanaShadow.CANMC.all;			    // 回写控制寄存器
	EDIS;
}

/*******************************************************************************
* 函数名称          : Uint16 InitdspECan(Uint16 baud)
* 入口参数			: CAN接口波特率编号 (eCanBaud索引)
* 出口				：CAN_INIT_TIME	 初始化进行中
*					  CAN_INIT_SUCC  初始化成功
*					  CAN_INIT_TIMEOUT 初始化超时
*					  CAN_INIT_BAUD_ERR 波特率出错
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: 初始化DSP Ecan接口
********************************************************************************/
#define		IINIT_CAN_TIME				3
Uint16 InitdspECan(Uint16 baud)		// Initialize eCAN-A module
{
	struct ECAN_REGS ECanaShadow;							// 声明一个影子寄存器，某些寄存器只能使用32位操作
	Uint32 *MsgCtrlPi;										// 初始化引用指针
	Uint16	i;												// 循环变量
	static	Uint16 con = 0;
	static	Uint16 count = 0;								// 超时计数器
	
	if (baud >= CAN_BAUD_SUM)
		return CAN_INIT_BAUD_ERR;							// 波特率出错
	if (count > IINIT_CAN_TIME)								// 初始化超时出错
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
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;			// 软件复位CAN模块
		
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;			// 读取CAN主控制寄存器
		ECanaShadow.CANMC.bit.SCB = 1;						// eCAN模式				
		ECanaShadow.CANMC.bit.SUSP = 1;						// 外设不受调试影响
//		ECanaShadow.CANMC.bit.DBO = 1;						// 首先 最低有效位 高字节在前 使用大端模式
		ECanaShadow.CANMC.bit.CCR = 1;						// CPU请求修改波特率或全局屏蔽寄存器
		ECanaShadow.CANMC.bit.ABO = 1;						// 自动恢复总线使能
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;			// 回写控制寄存器
		
	/* Initialize all bits of 'Master Control Field' to zero */
	// Some bits of MSGCTRL register come up in an unknown state. For proper operation,
	// all bits (including reserved bits) of MSGCTRL must be initialized to zero
		MsgCtrlPi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGCTRL);	// 消息控制器指针
		for (i=0; i<32; i++)
		{
			MsgCtrlPi[i<<2] = 0x00000000;					// 清零所有消息控制寄存器
		}
		MsgCtrlPi = (Uint32 *)(&ECANLAMS.LAM0);				// 息控制器指针
		for (i=0; i<32; i++)								// 清空所有屏蔽寄存器
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
		ECANREGS.CANTRR.all	= 0xFFFFFFFF;					// 复位发送请求，取消正在进行的发送
		ECANREGS.CANTA.all	= 0xFFFFFFFF;					// 清零发送响应寄存器/* Clear all TAn bits */      
		ECANREGS.CANRMP.all = 0xFFFFFFFF;					// 接收消息挂起寄存器/* Clear all RMPn bits */      
		ECANREGS.CANGIF0.all = 0xFFFFFFFF;					// 全局中断标志/* Clear all interrupt flag bits */ 
		ECANREGS.CANGIF1.all = 0xFFFFFFFF;
		ECANREGS.CANOPC.all = 0;							// 所有邮箱可被覆盖
	/* Configure bit timing parameters for eCANA
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 1 ;            			// CPU请求修改波特率或全局屏蔽寄存器
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;
	*/	
		con = 1;											// 第一阶段完成
	}
    if (con == 1)
	{
		ECanaShadow.CANES.all = ECANREGS.CANES.all;
		if (ECanaShadow.CANES.bit.CCE == 0 ) 				// Wait for CCE bit to be set..
		{
			count++;
			EDIS;
			return CAN_INIT_TIME;							// 初始化进行中
		}
		else
			con = 2;
	}
	
    if (con == 2)
	{
		ECanaShadow.CANBTC.all = 0;                         // 初始化波特率
		ECanaShadow.CANBTC.bit.BRPREG = eCanBaud[baud].BRPREG;
		ECanaShadow.CANBTC.bit.TSEG2REG = eCanBaud[baud].TSEG2REG;
		ECanaShadow.CANBTC.bit.TSEG1REG = eCanBaud[baud].TSEG1REG; 
		ECanaShadow.CANBTC.bit.SAM = 0;                     // 单位采样
		ECANREGS.CANBTC.all = ECanaShadow.CANBTC.all;
		
		ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 0 ;            			// 波特率设置完成 Set CCR = 0
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

// 接收中断初始化
    EALLOW;
    PieCtrlRegs.PIEIER9.bit.INTx5 = 0;                      // 禁止中断
    ECanaRegs.CANGIM.all = 0;
    ECanaRegs.CANMIM.all = 0;                               // 禁用所有邮箱中断
    
    ECanaRegs.CANGIF0.all = 0xffffffff;
    ECanaRegs.CANGIF1.all = 0xffffffff;                     // 清除中断

    ECanaRegs.CANMIL.all = 0;                               // 选择EcanA中断0
    ECanaRegs.CANGIM.all = 1;                               // 使能中断0


    PieVectTable.ECAN0INTA = &eCanRxIsr;                    // CANA 0接收中断入口
    EDIS;
    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;                      // 使能ECAN1中断
    IER |= M_INT9; 											// Enable CPU INT9
	eCanTranEnFlag = 0;                                     // 清空邮箱初始化标志
	eCanReEnFlag = 0;
	return CAN_INIT_SUCC;									// 初始化成功 
}	

/*******************************************************************************
* 函数名称          : void ErroCountReset(void)
* 入口参数			: 无
* 出口				：0  CAN控制正常状态
*                     1  CAN控制器错误复位中
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 05/16/2012
* 说明				: 手动复位CAN总线错误计数器
********************************************************************************/
Uint16 ErroCountReset(void)
{
    static Uint16 stat = 0;
    struct ECAN_REGS ECanaShadow;							// 声明一个影子寄存器，某些寄存器只能使用32位操作
    Uint32 canIf;
    
    if (0 == stat)
    {
        canIf = ECANREGS.CANGIF0.all;                       // 读取中断标志寄存器
        if (canIf & (7ul<<8) )                              // 错误计数>96，进入消极错误，总线关闭
        {
            EALLOW;
            ECANREGS.CANGIF0.all = canIf;                   // 清除中断标识
            ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
            ECanaShadow.CANMC.bit.CCR = 1 ;
            ECANREGS.CANMC.all = ECanaShadow.CANMC.all;
            EDIS;
            stat = 1;                                       // 进入下一状态
        }
    }
    else
    {
        EALLOW;
        ECanaShadow.CANMC.all = ECANREGS.CANMC.all;
		ECanaShadow.CANMC.bit.CCR = 0 ;            			// Set CCR = 0  清除错误计数器
		ECANREGS.CANMC.all = ECanaShadow.CANMC.all;
        EDIS;
        stat = 0;
    }
    return (stat);
}


/*******************************************************************************
* 函数名称          : void InitTranMbox(Uint16 mbox)
* 入口参数			: mbox      邮箱编号 0~31， 
*					  *datapi   邮箱初始化数据结构
*                     msgid     bit0~bit28  29位帧ID
*                               bit31       扩展帧标识
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: 初化CAN发送邮箱，可初始化为自动应答邮箱
********************************************************************************/
void InitTranMbox(Uint16 mbox, DspCanDataStru *datapi)//Uint32 msgid, Uint32 *dataPi)
{
	Uint16 id;
	Uint32 ECanaShadow, *msgIdPi;	                        //指针赋值消息ID地址

	id = mbox & 0x1f;
	eCanTranEnFlag |= 1ul <<mbox;							// 邮箱初始化发送标志

	msgIdPi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	msgIdPi[id<<2] = datapi->msgid; 					    // 写消息标志，确定是否为自动应答邮箱
	msgIdPi[(id<<2) +1] = 8;
	
	ECanaShadow = ECANREGS.CANMD.all;
	ECanaShadow &= ~(1ul<<id);
	ECANREGS.CANMD.all = ECanaShadow;						// 清零设置为发送邮箱

	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANME.all = ECanaShadow;						// 使能对应邮箱

	msgIdPi[(id<<2) + 2] = datapi->data.mdl;			    // 写自动应答信息到
	msgIdPi[(id<<2) + 3] = datapi->data.mdl;	
	
}

/*******************************************************************************
* 函数名称          : void InitReMbox(Uint16 mbox, union CANMSGID_REG msgid, union CANLAM_REG lam)
* 入口参数			: mbox 邮箱编号 0~31，bit7 “1” 接收远程帧 “0”普通帧	    bit6 "1"覆盖保护(不允许覆盖)
*					  msgid	消息标识ID
*					  lam	接收屏蔽寄存器，写"0"参与验收过滤位
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: 初化CAN接收邮箱
********************************************************************************/
void InitRecMbox(Uint16 mbox, Uint32 msgid, Uint32 lam)
{
	Uint16 id;
	Uint32 ECanaShadow,  *pi;								// = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	
	id = mbox & 0x1f;
	eCanReEnFlag |= 1ul << id;
	
	pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);
	pi[id<<2] = msgid | 1ul<<30;  							// 消息标识寄存器，使用过滤功能
	if ((mbox & 0x80) == 0x80)								// 发送远程帧传输初始化
		pi[(id<<2) +1] = 1<<4 | 8;							// 消息控制寄存器
	else
		pi[(id<<2) +1] = 8;
		
	ECanaShadow = ECANREGS.CANOPC.all;
	if ( (mbox & 0x40) == 0x40 )							// 使能覆盖保护检查，主初始化中已经将所有邮箱禁止覆盖保护
		ECanaShadow |= 1ul<<id;
	else
		ECanaShadow &= ~(1ul<<id);
	ECANREGS.CANOPC.all = ECanaShadow;
		
	ECanaShadow = ECANREGS.CANMD.all;						// 置“1”设置为接收邮箱
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANMD.all = ECanaShadow;						// 
	
	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow |= 1ul<<id;
	ECANREGS.CANME.all = ECanaShadow;						// 使能对应邮箱
	
	pi = (Uint32 *)(&ECANLAMS.LAM0);						// 配置接收屏蔽寄存器
	pi[id] = lam;

    EALLOW;
    ECanaShadow = ECANREGS.CANMIM.all;
	ECanaShadow |= 1ul<<id;
    ECANREGS.CANMIM.all = ECanaShadow;                      // 使能接收中断
    EDIS;
}


/*******************************************************************************
* 函数名称          : Uint16 eCanDataTran(Uint16 mbox, Uint16 len, Uint32 msgid, Uint32 *dataPi)
* 入口参数			: mbox      邮箱编号 0~31，
*                     len       发送数据长度字节数
*					  msgid     消息标识ID			只包含有效ID位
*                     dataPi    数据缓冲区
* 出口				：CAN_MBOX_NUM_ERROR		邮箱号出错，该邮箱未被初始化为发送邮箱
*					  CAN_MBOX_BUSY				邮箱忙
*					  CAN_MBOX_TRAN_SUCC		发送成功
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 08/25/2010
* 说明				: 指定邮箱发送数据，邮箱必须被初始化为发送邮箱
********************************************************************************/
Uint16 eCanDataTran(Uint16 mbox, Uint16 len, DspCanDataStru *data)
{
	Uint32 ECanaShadow, *pi;
    Uint32 msgid;
    

	if ( (eCanTranEnFlag & (1ul << mbox)) != (1ul << mbox) )
	{
		return (CAN_MBOX_NUM_ERROR);						// CAN邮箱号出错，邮箱未初始化
	}
	
	if (ECANREGS.CANTRS.all & (1ul << mbox))				// 检查上次发送是否完成，发送请求标志置位
	{
		return (CAN_MBOX_BUSY);								// CAN邮箱忙
	}

    msgid = data->msgid;
	mbox &= 0x1f;
   
	ECANREGS.CANTA.all = 1ul << mbox;						// 清空发送响应标志
	
	pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);				// 获取操作寄存器地址，写ID，写数据
	
	msgid &= ~(0x7ul<<29);									// 清除高三位
	msgid |= pi[mbox<<2] & (0x7ul << 29);					// 不修改ID配置位
	
	ECanaShadow = ECANREGS.CANME.all;
	ECanaShadow &= ~(1ul<<mbox);
	ECANREGS.CANME.all = ECanaShadow;						// 禁止对应邮箱
	
	pi[mbox<<2] = msgid;									// 重写ID
	pi[(mbox<<2) + 1] = len & 0xf;
	pi[(mbox<<2) + 2] = data->data.mdl;			            // 写数据
	pi[(mbox<<2) + 3] = data->data.mdh;
	
	ECanaShadow |= 1ul<<mbox;
	ECANREGS.CANME.all = ECanaShadow;						// 使能对应邮箱	

	ECANREGS.CANTRS.all = 1ul << mbox;						// 使能发送
	return (CAN_MBOX_TRAN_SUCC);
}

/*******************************************************************************
* 函数名称          : Uint16 eCanDataRec(Uint16 mbox, Uint32 *dataPi)
* 入口参数			: mbox      邮箱编号 0~31，
*					  *	data    接收缓存
* 出口				：CAN_MBOX_NUM_ERROR		邮箱号出错，该邮箱未被初始化为发送邮箱
*					  CAN_MBOX_EMPTY			接收邮箱空
*					  CAN_MBOX_REC_SUCC			接收数据成功
*					  CAN_MBOX_REC_OVER			接收数据有覆盖
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 08/25/2010
* 说明				: 接收数据接收缓存区
********************************************************************************/
/*
Uint16 eCanDataRec(Uint16 mbox, DspCanDataStru *data)
{
	Uint32 *pi;
	
	mbox &= 0x1f;
//	if ( (eCanReEnFlag & (1ul << mbox)) != (1ul << mbox))
//	{
//		return (CAN_MBOX_NUM_ERROR);						// CAN邮箱号出错，邮箱未初始化
//	}
	if (ECANREGS.CANRMP.all & (1ul << mbox) )				// 检查是否有接收消息挂起
	{
		pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);			
		data->msgid = pi[mbox<<2];							//  读ID，读数据
		data->data.mdl= pi[(mbox<<2) + 2];	
		data->data.mdh = pi[(mbox<<2) + 3];
        data->len= pi[(mbox<<2) + 1] & 0xf;                 // 读取接收数据长度

//		ECanaShadow = 1ul<<mbox;
		
		if (ECANREGS.CANRML.all & (1ul << mbox))			// 检查邮箱是否被覆盖过
		{
			ECANREGS.CANRMP.all = 1ul<<mbox;				// 清除消息挂起寄存器
			return (CAN_MBOX_REC_OVER);
		}	
		else
		{
			ECANREGS.CANRMP.all = 1ul<<mbox;				// 清除消息挂起寄存器
			return (CAN_MBOX_REC_SUCC);		
		}
	}
	else
	{
		return (CAN_MBOX_EMPTY);							// CAN邮箱空，无可读取数据		
	}
}
*/

/*******************************************************************************
* 函数名称          : Uint16 CanMailBoxEmp()
* 入口参数			: 
* 出口				: 1  有空闲
*                   : 0  无空闲
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 08/25/2010
* 说明				: CAN 发送邮箱空检测
********************************************************************************/
#define TX_MAILBOX_MAST (( (1ul << TRAN_MBOX_NUM) - 1) << TRAN_BOX_N)
Uint16 CanMailBoxEmp(void)
{
    if ( (ECANREGS.CANTRS.all & TX_MAILBOX_MAST) == TX_MAILBOX_MAST)
        return 0;                                           // 所有邮箱忙
    else
        return 1;

}



#elif 1



#endif




