/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : sci_osc.c
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 05/18/2010
* Description        : MD380示波器SCI后台软件模块C功能文件
*					使用F320F28035芯片SCI接口，使用其它芯片需要修改底层通讯模块
*					数据发送使用满FIFO "4级"模式，
*					数据接收使用 "1级"FIFO中断模式
*                   通讯格式：	数据位 8位  停止位 1位 无校验 无流控
*                               波特率：115’200 57‘600 19‘200
********************************************************************************
* 修改				：
* 版本				：V1.00
* 时间				：
* 说明				：修改增加数据校验，修改发送双缓存操作模式防止交替出错
***************************************************************************************************
* 修改				：
* 版本				：V1.10
* 时间				：
* 说明				：增加数据SCI出错处理，防止波特率不正确造成SCI进入错误状态
*					  修改正波特率计算问题造成19200波特率设置错误
***************************************************************************************************
* 修改				：
* 版本				：V1.20
* 时间				：
* 说明				：增加RS485兼容功能，兼容C2000所有SCI接口
*					  修复使用FIFO大于4字节芯片，当上位机发送过快可能出现的Bug
*					  增加更多编译开关等
***************************************************************************************************/
//***********************如使用的编辑器无法对齐，请将TAB设置为4个字符位置*****************************

#include "f_osc.h"										// 包含头文件
#include "f_comm.h"		
#include "f_ui.h"
#define     DEBUG_F_OSC         1

/************************液晶键盘使用****************************/
#if FC_KEY_CONTROL_EN
extern Uint16	companyPwdPass4Comm;
extern Uint16 userPwdPass4Comm;
Uint16 LcdKeyFlag = 0;  // 液晶键盘插入标志位，初始化为零默认键盘不接入
Uint16 LcdReadFEflag=0; //液晶键盘读取FE组
extern Uint16 keyFunc;
#endif
/*************************************************/ 

#if DEBUG_F_OSC

#if (SCI_INT_LOAD_RAM == 1)
	#if (OSC_TX_INT_EN == 1)
		#pragma CODE_SECTION(sciaTxFifoIsr, "ramfuncs");	// SCI发送中断加载到RAM运行
	#endif
	#pragma CODE_SECTION(sciaRxFifoIsr, "ramfuncs");		// 接收中断加载到RAM运行
#endif

/*****************************************************************/
// 外部函数声明
#if OSC_CON_CHECK
extern Uint16 CrcValueByteCalc(const Uint16 *data, Uint16 length);
#endif
// 功能码控制组声明
#if FC_CODE_CONTROL_EN
Uint16 OscControlWriter(Uint16 addr, Uint16 data);
Uint16 OscControlRead(Uint16 addr, Uint16* result);
#endif

// 监视组宏定义，修改该部分选择示波器数据组
static Uint16 *OscQuqData = (Uint16 *)((&funcCode.group.u0[0]) -1);
															// 定义监视组指针
#define 	GET_OSC_DATA(addr)				OscQuqData[addr]
/*****************************************************************/


static const	Uint16	BautRateCfg[3] = {
									(Uint16)(PERIPHERAL_CLK / 115200 / 8 -1),\
									(Uint16)(PERIPHERAL_CLK / 57600 / 8 -1),\
									(Uint16)(PERIPHERAL_CLK / 19200 / 8 -1)
								};							// 波特率参数

								
static DSP_OSC_CON_DATA	OscCongData;						// 示波器配置参数 
static OSC_DATA_TYPE	OscDataBufA, OscDataBufB;			// 数据缓冲区义，双缓存模式
static uint8		OscConFrameBuf[10];						// 示波器数据接收缓存，命令帧固定8字节
static uint8		BaudCfg = 0;							// 串口重配置标志

static SCI_RT_CON_DATA 	SciTxBufStr, SciRxBufStr;			// 数据收发控制模块

// 内部函数声明
static void InitOsc(void);									// 初始化示波器模块
static void InitSciOSC(uint8);								// 初始化示波器串口												
static uint8 SciDataTX(uint8 *buf, uint8 len);				// 数据发送函数

#if OSC_TX_INT_EN == 1
static interrupt void sciaTxFifoIsr(void);					// 发送中断
#endif
static interrupt void sciaRxFifoIsr(void);					// 接收中断
static uint8 SciDataRx(uint8 *buf, uint8 len, uint8 timeout);		// 数据接收函数
static void SciDataRxDeal(void);							// 命令处理
static void OscDataQcq(void);								// 数据采样
static void OscDataTxDeal(void);							// 数据发送
static uint8 GetOscData(uint8 *databuf);					// 获取示波器数据，完一次全通道采样
static void OscSciIoInit(void);								// SCI接口IO口线初始化							
#if OSC_DATA_CHECK
static void ClearOscBufEnd(void);							// 清空缓存最后4字节
//static uint8 OscDataCk(uint8 *buf, uint8 len);
#endif
/*************************************************/
#if	FC_CODE_APP_EN

#if DEBUG_MD500_SEARIS||DEBUG_MD290_SEARIS||DEBUG_MD380E_SEARIS
static uint8 OscConFrameAPPBuf[69];						// APP电子标签返回数据buff
#define			APP_CONTROL_FRAME_HEAD				OscConFrameAPPBuf[0]
#define			APP_CONTROL_FRAME_FC				OscConFrameAPPBuf[1]
#define			APP_CONTROL_FRAME_DATA_NUM          OscConFrameAPPBuf[2]
#define			APP_CONTROL_FRAME_CRCL				OscConFrameAPPBuf[67]
#define			APP_CONTROL_FRAME_CRCH				OscConFrameAPPBuf[68]
#else
static uint8 OscConFrameAPPBuf[132];						// APP电子标签返回数据buff
#define			APP_CONTROL_FRAME_HEAD				OscConFrameAPPBuf[0]
#define			APP_CONTROL_FRAME_FC				OscConFrameAPPBuf[1]
#define			APP_CONTROL_FRAME_CRCL				OscConFrameAPPBuf[130]
#define			APP_CONTROL_FRAME_CRCH				OscConFrameAPPBuf[131]
#endif

#if DEBUG_MD290_SEARIS
/*const unsigned int APPIDDataBuff[8]={
0x0001,//P0 1 厂商代码
0x000B,//P2 3 产品系列
0x010E,//P4 5 产品型号
0x1D7E,//P6 7 产品版本
0x0000,//P8 9 通信接口版本 v0.0
0x0000,//P10 11 Boot版本   v0.0
0x0015,//P12 13 芯片型号
0xffff//P14 --P15保留
};*/
const unsigned int APPIDDataBuff[8]={
0x0001,//P0 1 厂商代码
0x000B,//P2 3 产品系列
0x0110,//P4 5 产品型号
//0x0B54,//P6 7 产品版本
SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//P6 7 产品版本
0x0000,//P8 9 通信接口版本 v0.0
0x0000,//P10 11 Boot版本   v0.0
0x0015,//P12 13 芯片型号
0xffff//P14 --P15保留
};
#elif DEBUG_MD500_SEARIS
const unsigned int APPIDDataBuff[8]={
0x0001,//P0 1 厂商代码
0x000B,//P2 3 产品系列
0x0111,//P4 5 产品型号
//0x1D7E,//P6 7 产品版本
SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//P6 7 产品版本
0x0000,//P8 9 通信接口版本 v0.0
0x0000,//P10 11 Boot版本   v0.0
0x0015,//P12 13 芯片型号
0xffff//P14 --P15保留
};
#elif DEBUG_MD380E_SEARIS
const unsigned int APPIDDataBuff[8]={
0x0001,//P0 1 厂商代码
0x000B,//P2 3 产品系列
0x010F,//P4 5 产品型号
//0x1D7E,//P6 7 产品版本
SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//P6 7 产品版本
0x0000,//P8 9 通信接口版本 v0.0
0x0000,//P10 11 Boot版本   v0.0
0x0015,//P12 13 芯片型号
0xffff//P14 --P15保留
};
#else
const unsigned int APPIDDataBuff[8]={
0x0001,//P0 1 厂商代码
0x000B,//P2 3 产品系列
0x0107,//P4 5 产品型号
//0x1c86,//P6 7 产品版本
SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//P6 7 产品版本
0x0000,//P8 9 通信接口版本 v0.0
0x0000,//P10 11 Boot版本   v0.0
0x0015,//P12 13 芯片型号
0xffff//P14 --P15保留
};
#endif
void copyAPPtoBuff(void);
#endif

/*******************************************************************************
* 函数名称          : void OscSciFunction(void)
* 入口参数			：con		"OSC_SCI_EN"	示波器功能使能
								其它			示波器停止
* 出口				：OSC_SCI_EN                示波器模式
                      其它                      退出示波器模式
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: 示波器后台主函数，主程序需0.5ms秒间隔调用该函数
* 
********************************************************************************/
void OscSciFunction(void)
{
	static uint8 count = 0;									// 时间计数器
//	static uint8 countRx = 0;								// 接收计时
	static uint8 OscCfg = 0;								// 示波器串口配置标志"0"未配置, "1"配置完成

	if (BaudCfg)											// 修改波特率
	{
		count ++;
		if (count > 2)										// 等待，2间隔修改波特率
		{
			InitSciOSC(OscCongData.baudRate);              // 重新初始化串口	
			BaudCfg = 0;
			count = 0;
		}
		return;
	}
	
	if (0 == OscCfg)										// 初始化标志为”0“
	{
		
		OscDataBufA.frameHead1 = OSC_DATA_FRAME_HEAD_A;		// 初始化缓存
		OscDataBufA.frameHead2 = OSC_DATA_FRAME_HEAD_B;		// 初始化缓存
		OscDataBufB.frameHead1 = OSC_DATA_FRAME_HEAD_A;
		OscDataBufB.frameHead2 = OSC_DATA_FRAME_HEAD_B;
		OscDataBufA.rwPI = 0;
		OscDataBufB.rwPI = 0;
		OscDataBufA.full = 0;                               // 清空缓存
        OscDataBufB.full = 0;

        InitOsc();											// 执行示波器初始化操作
		OscCfg = 1;											// 置位串口初始化标志位
		return;												// 直接返回，减小该模块处理时间
	}

//	countRx ++;
//	if (countRx > 4)										// 第4个间隔接收数据1次
	{
  //		countRx = 0;		
		if (SciDataRx((uint8 *)(&OscConFrameBuf), 8, 0) == TRUE)	// Sci数据接收与处理
		{
			SciDataRxDeal();								// 接收命令处理
			return;
		}
	}
	OscDataQcq();											// 监视信号数据采集	
	OscDataTxDeal();										// 数据发送到PC，循环检测双缓存数据
}	

/*******************************************************************************
* 函数名称          : void InitOsc(DSP_OSC_CON_DATA *OscCongData)
* 入口参数			：OscCongData		示波器配置参数指针
* 出口				：无
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: 示波器模块初始化，包括SCI与示波器常数初始化
* 
********************************************************************************/

void InitOsc(void)
{
	OscCongData.baudRate = 3;                              // 波特率19200
    OscCongData.interval = 1;                              // 采样间隔
    OscCongData.status = 0;                                // 暂停工作
    OscCongData.runContinue = 1;                           // 连续运行，停机工作
    OscCongData.ch1Addr = 1;
    OscCongData.ch2Addr = 0;
    OscCongData.ch3Addr = 0;
    OscCongData.ch4Addr = 0;
	OscCongData.chSum = 1;

    InitSciOSC(OscCongData.baudRate);                      // 初始化串口
}	


/*******************************************************************************
* 函数名称          : void InitSciOSC(uint8 baudRate)
* 入口参数			：OscCongData		示波器配置参数指针
* 出口				：无
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: 示波器模块初始化，包括SCI与示波器常数初始化
* 
********************************************************************************/
void InitSciOSC(uint8 baudRate)
{
    SciTxBufStr.busy = 0;                                   // 初始化串口收/发控制块
    SciRxBufStr.busy = 0;
//	SciRxBufStr.err  = 0;									// 清空串口出错标志							
	SciRxBufStr.len = 0xff;
#if 	(1 == OSC_SCI_SEL)	
    PieCtrlRegs.PIEIER9.bit.INTx1 = 0;     					// PIE Group 9, INT1	RX接收中断 禁止
	PieCtrlRegs.PIEIER9.bit.INTx2 = 0;     					// PIE Group 9, INT2	TX发送中断
#else
    PieCtrlRegs.PIEIER9.bit.INTx3 = 0;     					// PIE Group 9, INT3	RX接收中断 禁止
	PieCtrlRegs.PIEIER9.bit.INTx4 = 0;     					// PIE Group 9, INT4	TX发送中断
#endif
	
	// 配置SCI-A使用管脚
	OscSciIoInit();


	// 配置串口参数
	SCI_OSC_REGS.SCICCR.all = 0x0007;  						// 1 stop bit,  No loopback
															// No parity,8 char bits,
															// async mode, idle-line protocol
	SCI_OSC_REGS.SCICTL1.all = 0x0002 | 1<<6; 				// enable TX功能 与错误中断
															// Disable RX ERR, SLEEP, TXWAKE
															
   SCI_OSC_REGS.SCILBAUD = BautRateCfg[baudRate-1];	
   SCI_OSC_REGS.SCIHBAUD = BautRateCfg[baudRate-1]>>8;  	// 波特率配置

   SCI_OSC_REGS.SCICTL2.all = 0;							// 接收禁止收
   
   SCI_OSC_REGS.SCIFFTX.all = 0xC000;						// 启用FIFO增强功能，	发送等于零产生中断 减少中断次数

   SCI_OSC_REGS.SCIFFRX.all = 0x0004;						// 配置接收FIFO，4个数据时产生中断
   SCI_OSC_REGS.SCIFFCT.all = 0x00;
   SCI_OSC_REGS.SCIPRI.bit.FREE = 1;						// 调试不停止   

   SCI_OSC_REGS.SCICTL1.bit.SWRESET = 1;     				// Relinquish SCI from Reset	启动串口

	
// SCI相关中断全局寄存器
#if 	(1 == OSC_SCI_SEL)
	PieCtrlRegs.PIEIFR9.bit.INTx1 = 0;						// 清除中断标志    接收
    PieCtrlRegs.PIEIFR9.bit.INTx2 = 0;                      // 发送
#else
	PieCtrlRegs.PIEIFR9.bit.INTx3 = 0;						// 清除中断标志    接收
    PieCtrlRegs.PIEIFR9.bit.INTx4 = 0;                      // 发送
#endif	
	
// 中断向量初始化   
	EALLOW;  												// This is needed to write to EALLOW protected registers
#if 	(1 == OSC_SCI_SEL)
    PieVectTable.SCIRXINTA = &sciaRxFifoIsr;
    #if OSC_TX_INT_EN == 1
	    PieVectTable.SCITXINTA = &sciaTxFifoIsr;
    #endif
#else    
	PieVectTable.SCIRXINTB = &sciaRxFifoIsr;
    #if OSC_TX_INT_EN == 1
	    PieVectTable.SCITXINTB = &sciaTxFifoIsr;
    #endif
#endif
	EDIS;
	
#if 	(1 == OSC_SCI_SEL)	
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;						// 使能中断，
	#if OSC_TX_INT_EN == 1
		PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
	#else
		PieCtrlRegs.PIEIER9.bit.INTx2 = 0;
	#endif
#else	
	PieCtrlRegs.PIEIER9.bit.INTx3 = 1;						// 使能中断，
	#if OSC_TX_INT_EN == 1
		PieCtrlRegs.PIEIER9.bit.INTx4 = 1;
	#else
		PieCtrlRegs.PIEIER9.bit.INTx4 = 0;
	#endif
#endif	

//	OscConFrameBuf[9] = 
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   					// Enable the PIE block，使能PIE模块
	IER |= M_INT9; 											// Enable CPU INT9
}
		
/*******************************************************************************
* 函数名称          : uint8 SciDataTX(uint8 *buf, uint8 len)
* 入口参数			：buf		字节数据起始地址
*					  num		数据长度
* 出口				：TRUE		发送操作成功
					  FALSE		上次发送未完成，发送失败
* 版本		        : V0.0.1
* 时间	            : 05/18/2010
* 说明				: 通过中断方式发送数据
*                     Sci发送数据，主要用于初始化SCI发送模块
********************************************************************************/
uint8 SciDataTX(uint8 *buf, uint8 len)					
{
 //   PieCtrlRegs.PIEIFR9.bit.INTx2 = 0;                      // 清除PIE中发送中断标志
#if RS485_ENABLE == 1
	RS485_RTS_O = RS485_T_O;
#endif
	if (TRUE == SciTxBufStr.busy)							// 发送模块忙
		return	FALSE;
	SCI_OSC_REGS.SCIFFTX.all = 0xC000;						// 复位FIFO,FIFO触发值是"0"
	SCI_OSC_REGS.SCIFFTX.bit.TXFIFOXRESET = 1;				// 启动FIFO
	
	SCI_OSC_REGS.SCITXBUF = *buf++;							// 写4次FIFO
	SCI_OSC_REGS.SCITXBUF = *buf++;
	SCI_OSC_REGS.SCITXBUF = *buf++;
	SCI_OSC_REGS.SCITXBUF = *buf++;
	SciTxBufStr.buf = buf;                                  // 写缓存地址与长度
    SciTxBufStr.len = len - 4;
	SciTxBufStr.busy = TRUE;								// 数据发送中
	
#if OSC_TX_INT_EN == 1	
    SCI_OSC_REGS.SCIFFTX.bit.TXFFINTCLR = 1;  				// Clear SCI Interrupt flag
    SCI_OSC_REGS.SCIFFTX.bit.TXFFIENA = 1;                  // 使能发送FIFO中断
#endif
	
//	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;     					// PIE Group 9, INT2	TX发送中断
	return TRUE;
}


/*	*/
/*******************************************************************************
* 函数名称          : void OscTxFifoTask(void)
* 入口参数			：无
* 出口				：无
* 版本		        : V0.0.1
* 时间	            : 05/31/2010
* 说明				: 非中断模式数据发送模块，需0.5ms 调用一次
*					  主要完成写FIFO数据 
********************************************************************************/
#if OSC_TX_INT_EN == 0
void OscTxFifoTask(void)
{
	uint8 fifo, len;
	uint8 *buf;												// 内部临时变量

	if (SciTxBufStr.busy == 0)								// 未开始发送
		return;

	len = SciTxBufStr.len;
	buf = SciTxBufStr.buf;	
	if (len == 0)
	{
		SciTxBufStr.busy = 0;								// 该次发送完成		
		return;
	}	
	fifo = 4 - SCI_OSC_REGS.SCIFFTX.bit.TXFFST;				// 读取空闲FIFO长度
	if (len < fifo)
	{
		fifo = len;
		len = 0;
	}
	else
	{
		len -= fifo;		
	}	
	while (fifo)
	{
		SCI_OSC_REGS.SCITXBUF = *buf++;						// 写FIFO
		fifo--;
	}
	SciTxBufStr.len = len;
	SciTxBufStr.buf = buf;
}
#endif
	
	
/*******************************************************************************
* 函数名称          : interrupt void sciaTxFifoIsr(void)
* 入口参数			：无
* 出口				：无
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: SCI FIFO模式数据发送中断
* 					  该函数非加速模式状态不允许单步调试
********************************************************************************/
#if OSC_TX_INT_EN == 1
interrupt void sciaTxFifoIsr(void)
{
	uint8 *buf;												// 内部临时变量
	uint8 len;

	buf = SciTxBufStr.buf;
	len = SciTxBufStr.len;
#if SCI_TX_INT_SPEEDUP == 1
	if (len > 3)
    {
    	SCI_OSC_REGS.SCITXBUF = *buf++;						// 写4次FIFO
    	SCI_OSC_REGS.SCITXBUF = *buf++;
    	SCI_OSC_REGS.SCITXBUF = *buf++;
    	SCI_OSC_REGS.SCITXBUF = *buf++;
		SciTxBufStr.buf = buf;								// 回写全局变量
		len -= 4;
		SciTxBufStr.len = len;
    }
    else
    {
		SciTxBufStr.busy = 0;								// 置发送空闲
		SCI_OSC_REGS.SCIFFTX.bit.TXFFIENA = 0;				// 禁止FIFO发送中断
    }
#else	
	if (len > 3)
    {
    	SCI_OSC_REGS.SCITXBUF = *buf++;						// 写4次FIFO
    	SCI_OSC_REGS.SCITXBUF = *buf++;
    	SCI_OSC_REGS.SCITXBUF = *buf++;
    	SCI_OSC_REGS.SCITXBUF = *buf++;
		len -= 4;
    }
    else
    {
        while (len)											// 数据长度小于FIFO深度
        {
			SCI_OSC_REGS.SCITXBUF = *buf++;
			len--;
        }
    }
	if (0 == SCI_OSC_REGS.SCIFFTX.bit.TXFFST)				// FIFO空，数据发送完成
	{
		SciTxBufStr.busy = 0;								// 置发送空闲
		SCI_OSC_REGS.SCIFFTX.bit.TXFFIENA = 0;				// 禁止FIFO发送中断
	}
	else
	{
		SciTxBufStr.buf = buf;								// 回写全局变量
		SciTxBufStr.len = len;	
	}
#endif	
    SCI_OSC_REGS.SCIFFTX.bit.TXFFINTCLR = 1;  				// Clear SCI Interrupt flag	
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;      			// Issue PIE ACK
}
#endif

/*******************************************************************************
* 函数名称          : uint8 SciDataRx(uint8 *buf, uint8 len uint8 timeout))
* 入口参数			：buf	接收缓存地址
*					  len	数据长度
*					  timeout 超时时间   *调用周期   0 无限等待
* 出口				：TRUE  接收成功
*					  SCI_RT_BUSY	接收操作中
*					  FALSE 超时出错
*					  SCI_RS485_TX_BUSY SCI使用485发送接口忙

* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: SCI FIFO模式数据接收数据
* 					  Sci数据接收与处理
* 					  只能接收4的整数倍长度
********************************************************************************/
uint8 SciDataRx(uint8 *buf, uint8 len, uint8 timeout)									
{
    static uint8 count = 0;									// 超时计数
	static uint8 timeOut = 0;								// 超时时间
//	static uint8 dataLen = 0;								// 长度检测	
	static uint8 countOut = 0;
#if RS485_ENABLE == 1	
	if (SciTxBufStr.busy)
		return SCI_RS485_TX_BUSY;							// 485忙
	if (SCI_OSC_REGS.SCICTL2.bit.TXEMPTY)					// 发送完成进入接收状态
		RS485_RTS_O = RS485_R_O;
	else
		return SCI_RS485_TX_BUSY;
#endif	

	count ++;
	
	if (( count > timeOut) && (timeOut != 0) )				// 超时检测
	{
	    SciRxBufStr.busy = 0;								// 进入空闲状态
		SCI_OSC_REGS.SCICTL1.bit.RXENA = 0;                 // 禁止接收
		SCI_OSC_REGS.SCIFFRX.all = 4;	
		count = 0;											// 清空标志复位FIFO
		return FALSE;
	}
	
	if (TRUE == SciRxBufStr.busy)							// 忙检测
	{
		if (SciRxBufStr.rxflag)								// 收到第一次数据后开始计时
		{
			countOut =0;
			SciRxBufStr.rxflag &= 0x10;						// 清除低位
		}	
		else
		{
			if (++countOut > 2)
			{
				SciRxBufStr.busy = 0;						// 进入空闲状态
				SCI_OSC_REGS.SCICTL1.bit.RXENA = 0;         // 禁止接收
				SCI_OSC_REGS.SCIFFRX.all = 4;
				return FALSE;
			}
		}
		return SCI_RT_BUSY;
    }
	if (0 == SciRxBufStr.len)								// 长度接收完成
	{
		SCI_OSC_REGS.SCIFFRX.all = 4;			
		SciRxBufStr.len = 0xff;
		return TRUE;
	}
	countOut = 0;
//	dataLen = len;											// 数据长度	
	SciRxBufStr.buf = buf;									// 初始化串口发送控制模块
	SciRxBufStr.len = len;
	SciRxBufStr.busy = TRUE;
	SciRxBufStr.rxflag = 0x10;								// 置高位，不开始超时计数
	count = 0;												// 计数与超时初始化
	timeOut = timeout;

	// 数据接收部分
	SCI_OSC_REGS.SCIFFRX.bit.RXFIFORESET = 0;				// 复位FIFO并使能接收
	SCI_OSC_REGS.SCICTL1.bit.RXENA = 1;						//
//	SCI_OSC_REGS.SCICTL1.bit.RXERRINTENA = 1;               // 使能错误中断
    SCI_OSC_REGS.SCIFFRX.all = 1<<14 | 1<<13 | 1<<6 | 1<<5 | 4;	// 接收FIFO是4字节触发
/*    
    SCI_OSC_REGS.SCIFFRX.bit.RXFFINTCLR= 1;                 // 清FIFO中断
    SCI_OSC_REGS.SCIFFRX.bit.RXFFOVRCLR = 1;                // 清除FIFO溢出
    SCI_OSC_REGS.SCIFFRX.bit.RXFIFORESET = 1;               // 使能接收FIFO    
	SCI_OSC_REGS.SCIFFRX.bit.RXFFIENA = 1;                  // 接收FIFO中断使能   
	SCI_OSC_REGS.SCIFFRX.bit.RXFFIL = 4;					// FIFO设置4级
*/	
//	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;     					// PIE Group 9, INT1	RX接收中断使能
	return SCI_RT_BUSY;
}

/*******************************************************************************
* 函数名称          : interrupt void sciaRxFifoIsr(void)
* 入口参数			：无
* 出口				：无
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: SCI FIFO模式数据接收中断
* 
********************************************************************************/
interrupt void sciaRxFifoIsr(void)
{
	uint8 *buf;												// 内部临时变量
	uint8 len;

//	SciRxBufStr.rxflag = 0x01;								// 收到数据写“1”数据接收标志	
	if (SCI_OSC_REGS.SCIRXST.bit.RXERROR)                   // 有接收错误
	{
		BaudCfg = 1;										// 重新配置串口
	}
	else
	{
		SciRxBufStr.rxflag = 0x01;							// 收到数据写“1”数据接收标志		
		buf = SciRxBufStr.buf;
		len = SciRxBufStr.len;	
		
		*buf++ = (uint8)(SCI_OSC_REGS.SCIRXBUF.bit.RXDT);	// 读取数据
		*buf++ = (uint8)(SCI_OSC_REGS.SCIRXBUF.bit.RXDT);
		*buf++ = (uint8)(SCI_OSC_REGS.SCIRXBUF.bit.RXDT);
		*buf++ = (uint8)(SCI_OSC_REGS.SCIRXBUF.bit.RXDT);
		len -= 4;
		if (0 == len)
		{
			SciRxBufStr.busy = 0;							// 置发送空闲
			SCI_OSC_REGS.SCICTL1.bit.RXENA = 0;             // 禁止接收
	        SCI_OSC_REGS.SCIFFRX.all = 1<<14 | 0<<13 | 1<<6 | 0<<5 | 4;
															// 复位FIFO防止使用大FIFO芯片发送出错造成错误中断
	//		PieCtrlRegs.PIEIER9.bit.INTx1 = 0;     			// PIE Group 9, INT1	RX发送中断		
		}
		SciRxBufStr.buf = buf;
		SciRxBufStr.len = len;

		SCI_OSC_REGS.SCIFFRX.all = 1<<14 | 1<<13 | 1<<6 | 1<<5 | 4;
	/*	
		SCI_OSC_REGS.SCIFFRX.bit.RXFFOVRCLR = 1;   			// Clear Overflow flag
		SCI_OSC_REGS.SCIFFRX.bit.RXFFINTCLR = 1;   			// Clear Interrupt flag
	*/
	}
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;       			// Issue PIE ack
}


/*******************************************************************************
* 函数名称          : void SciDataRxDeal(void)
* 入口参数			：无
* 出口				：无
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: 示波器命令帧处理
* 
********************************************************************************/
#if DEBUG_MD500_SEARIS||DEBUG_MD290_SEARIS||DEBUG_MD380E_SEARIS
void SciDataRxDeal(void)
{
	Uint16 err = 0;
#if FC_CODE_CONTROL_EN
	Uint16 dat, rwaddr;
#endif

#if OSC_CON_CHECK
	Uint16  crcResult;
	crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
															// CRC16 校验
	if (crcResult != (Uint16)(( (CONTROL_FRAME_CRCH<<8) & 0xff00) | (CONTROL_FRAME_CRCL & 0x00ff) ))
	{
		return ;
	}
#endif

	if (CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD)			// 帧起始出错
#if	FC_CODE_APP_EN	 
	  if(CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD_APP)	
#endif
	{
		return;
	}


	if (CONTROL_FRAME_FC == FC_STOP_OSC)					// 停止示波器命令
	{
		OscDataBufA.rwPI = 0;
		OscDataBufB.rwPI = 0;
		OscDataBufA.full = 0;                               // 清空缓存
        OscDataBufB.full = 0;
		OscCongData.status = 0;
	}
	else if (OscCongData.status)							// 工作于运行状态
	{
		err = 0x80;											// 命令出错
	}
	else
	{
		switch (CONTROL_FRAME_FC)							// 命令判断
		{
            case FC_COMM_TEST:                              // 通讯测试
                // 无需进行任何操作


                break;

            case FC_CHANNL_SEL:								// 通道设置
				OscCongData.chSum = 0;
				
				OscCongData.ch1Addr = CONTROL_FRAME_P1;
				if (CONTROL_FRAME_P1)	OscCongData.chSum ++;
				OscCongData.ch2Addr = CONTROL_FRAME_P2;
				if (CONTROL_FRAME_P2)	OscCongData.chSum ++;
				OscCongData.ch3Addr = CONTROL_FRAME_P3;
				if (CONTROL_FRAME_P3)	OscCongData.chSum ++;
				OscCongData.ch4Addr = CONTROL_FRAME_P4;
				if (CONTROL_FRAME_P4)	OscCongData.chSum ++;
#if OSC_DATA_CHECK
				if (OscCongData.chSum == 3)
				{
					ClearOscBufEnd();						// 清空4字节				
				}
#endif				
				break;
			
			case FC_PARA_CFG:								// 采样速度选择
				if ((CONTROL_FRAME_P1>0) && (CONTROL_FRAME_P1<4))
				{
					OscCongData.baudRate = CONTROL_FRAME_P1;
					BaudCfg = 1;							// 修改配置						
				}
				if ((CONTROL_FRAME_P2>0) && (CONTROL_FRAME_P2<101))
				{
					OscCongData.interval = CONTROL_FRAME_P2;		
				}
//				OscCongData.runContinue = CONTROL_FRAME_P3;		
		
				break;
			case FC_START_OSC:								// 启动示波器
				OscCongData.status = 1;
				break;
#if FC_CODE_CONTROL_EN
			case FC_READ_FC_DATA:							// 读内部映射功能码操作
/************************液晶键盘使用****************************/
#if FC_KEY_CONTROL_EN
 dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
  if (FF_PASSWORD == dat)   // 厂家密码正确
            {
                companyPwdPass4Comm = 1;
				userPwdPass4Comm = 1;
				commTicker = 0;   //计数清零，30秒后禁止修改FF组
            }
	if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//液晶键盘读FE组
	{
		LcdReadFEflag = 1;
	} 
		
#endif
/****************************************************/
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
															// 合成地址，取得数据
				err = OscControlRead(rwaddr, &dat);		    // 执行操作
				CONTROL_FRAME_P3 = dat&0x00ff;				// 数据转串行  低字节
				CONTROL_FRAME_P4 = dat>>8;					// 高字节

				break;
			case FC_WRITE_FC_DATA:							// 写内部功能码操作
				rwaddr =  ((CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);				
				dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
/************************液晶键盘使用****************************/	
		#if FC_KEY_CONTROL_EN
			if((rwaddr==0x2000)&&(LcdKeyFlag==1))
			{
			    if(dat==1)
				{
				keyFunc =KEY_RUN;
			     break;
				 }
			 	if(dat==6)
				{
			   	 	keyFunc =KEY_STOP;
					break;
				}
            }
			if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//液晶键盘读FE组
			{
				LcdReadFEflag = 1;
			}
	    #endif
/************************液晶键盘使用****************************/				
				err = OscControlWriter(rwaddr, dat);		// 操作功能	
							
				break;
#endif
/************************液晶键盘使用****************************/
#if FC_KEY_CONTROL_EN   	// 串行键盘功能禁能，“1”使能，“0”关闭	
			case FC_VERSION_DATA:							// 读机型、版本号操作							
				 LcdKeyFlag = 1;//等于1表示液晶键盘接入
				 dat = 1;// 1代表液晶键盘中MD380机型号
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_LOWER_DATA:	
			 //	 keydata = CONTROL_FRAME_P2;
               dat= FuncCodeAdd(CONTROL_FRAME_P2);
			 //下限值是否与别的功能码有关联  
            if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.lowerLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
                dat = funcCode.all[dat];
			}
			else
			{
				dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
			}
				
		/*		 if (GetGradeIndex(dat, CONTROL_FRAME_P1) == 10)//f0-10下限值特殊处理
            {
                dat = 50 * decNumber[funcCode.code.frqPoint];
				CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
            }  else*/
			{
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
		    }
				 break;
			case FC_UPPER_DATA:							// 写内部上限值操作							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
				 if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.upperLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
                dat = funcCode.all[dat];
			}
			else
			{
			 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
			} 
				 
				
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_INIT_DATA:
				dat= FuncCodeAdd(CONTROL_FRAME_P2);						// 读内部出厂值操作							
				 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].init;
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_DATA:							// 读内部属性操作							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
			dat =	upsend(GetGradeIndex(dat, CONTROL_FRAME_P1));
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_UDATA:
               dat =dispAttributeU0[CONTROL_FRAME_P1].all;
			   CONTROL_FRAME_P3 = dat&0x00ff;
			   CONTROL_FRAME_P4 = dat>>8;
			break;
#endif

#if	FC_CODE_APP_EN
            case FC_APP_READ_WRITE:
                rwaddr = ( (CONTROL_FRAME_P1<<8) & 0xff00) | (CONTROL_FRAME_P2 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// 跳转到BOOT区
				{
					// 烧录工装有引出boot脚时为1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 0x01;
					else
					{
						EALLOW;//风扇引脚作为启动引脚，这里要恢复成输出
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 0x02;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 0x01;
					err = 0x0;
				#endif
				
				    CONTROL_FRAME_P1 = 0x02;
				    CONTROL_FRAME_P2 = 0x00;

				}
				else if(rwaddr == FC_APP_STATE)					// 读取变频器状态
				{  
				    CONTROL_FRAME_P1 = 0x02;
					CONTROL_FRAME_P3 = 0x01;

				    if(runFlag.bit.run)
				   	{
                        if (FORWARD_DIR == runFlag.bit.dir) // F0-12之前的方向
                       	{
							CONTROL_FRAME_P2 = 0x01;  //正转
							//err = 0x02;
						}
						else
						{
                            CONTROL_FRAME_P2 = 0x02;  //反转
                            //err = 0x02;
						}
					}
					else
					{
                        CONTROL_FRAME_P2 = 0x03;     //停机
					}	
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// 获取APP区电子标签
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#endif
/***********************************************************************************/
			default:
				err = 0x80;									// 命令出错
				break;
		}
		if (err > 0)
		{
			CONTROL_FRAME_FC |= 0x80;						// 检测查是否出错
            CONTROL_FRAME_P2 = 0x80;
            CONTROL_FRAME_P1 = 0x01;
            CONTROL_FRAME_P4 = 0;
			CONTROL_FRAME_P3 = err;							// 出错返回出错代码
		}
        else if (sciFlag.bit.pwdPass)                      // 密码通过：返回 0x8888
        {
            CONTROL_FRAME_P4 = 0x88;
			CONTROL_FRAME_P3 = 0x88;
        }
#if OSC_CON_CHECK											// CRC16校验并加入响应帧
        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
        {
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-1-2);//5个字节
		    //CONTROL_FRAME_CRCL = crcResult;
		    //CONTROL_FRAME_CRCH = crcResult>>8;
		    CONTROL_FRAME_P4 = crcResult;
		    CONTROL_FRAME_CRCL = crcResult>>8;
		}
		else
		{
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);//6个字节
		    CONTROL_FRAME_CRCL = crcResult;
		    CONTROL_FRAME_CRCH = crcResult>>8;
		    //CONTROL_FRAME_P4 = crcResult;
		    //CONTROL_FRAME_CRCL = crcResult>>8;
		}
#endif
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			APP_CONTROL_FRAME_DATA_NUM = 0x40;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 67);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 69);				// 发送72字节响应帧	
		}	
		else
#endif	

        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
       	{
		    SciDataTX(OscConFrameBuf, 8-1);						// 发送7字节响应帧	
		}
		else
		{
		    SciDataTX(OscConFrameBuf, 8);						// 发送8字节响应帧	
		}

#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))
		{
			volatile unsigned char *datatemp;
			//如果没有关闭看门狗  需要关闭
			#if 	FC_CODE_BOOTPIN_EN		// 工装有boot使能脚时使用
			while(1);
			#else
			//   恢复状态
			EALLOW;
			SysCtrlRegs.WDCR = 0x0068; 
			SciaRegs. SCIFFCT.all=0x4000; 
			SysCtrlRegs.CLKCTL.all = 0;
			EDIS;
			
			datatemp = &(SciTxBufStr.len);
			while(*datatemp);
			datatemp = &(SciTxBufStr.busy);
			while(*datatemp);
			while (0 == SciaRegs.SCICTL2.bit.TXEMPTY){}

			asm(" MOV        @SP,#0x0004");
			asm(" LCR        0x3ff7dd");
			asm(" MOV		XAR7,ACC");	
			asm(" LCR *XAR7");
			#endif
		}
#endif	
	}
}
#else   //MD380 
void SciDataRxDeal(void)
{
	Uint16 err = 0;
#if FC_CODE_CONTROL_EN
	Uint16 dat, rwaddr;
#endif

#if OSC_CON_CHECK
	Uint16  crcResult;
	crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
															// CRC16 校验
	if (crcResult != (Uint16)(( (CONTROL_FRAME_CRCH<<8) & 0xff00) | (CONTROL_FRAME_CRCL & 0x00ff) ))
	{
		return ;
	}
#endif

	if (CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD)			// 帧起始出错
#if	FC_CODE_APP_EN	 
	  if(CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD_APP)	
#endif
	{
		return;
	}
	if (CONTROL_FRAME_FC == FC_STOP_OSC)					// 停止示波器命令
	{
		OscDataBufA.rwPI = 0;
		OscDataBufB.rwPI = 0;
		OscDataBufA.full = 0;                               // 清空缓存
        OscDataBufB.full = 0;
		OscCongData.status = 0;
	}
	else if (OscCongData.status)							// 工作于运行状态
	{
		err = 0x80;											// 命令出错
	}
	else
	{
		switch (CONTROL_FRAME_FC)							// 命令判断
		{
            case FC_COMM_TEST:                              // 通讯测试
                // 无需进行任何操作


                break;

            case FC_CHANNL_SEL:								// 通道设置
				OscCongData.chSum = 0;
				
				OscCongData.ch1Addr = CONTROL_FRAME_P1;
				if (CONTROL_FRAME_P1)	OscCongData.chSum ++;
				OscCongData.ch2Addr = CONTROL_FRAME_P2;
				if (CONTROL_FRAME_P2)	OscCongData.chSum ++;
				OscCongData.ch3Addr = CONTROL_FRAME_P3;
				if (CONTROL_FRAME_P3)	OscCongData.chSum ++;
				OscCongData.ch4Addr = CONTROL_FRAME_P4;
				if (CONTROL_FRAME_P4)	OscCongData.chSum ++;
#if OSC_DATA_CHECK
				if (OscCongData.chSum == 3)
				{
					ClearOscBufEnd();						// 清空4字节				
				}
#endif				
				break;
			
			case FC_PARA_CFG:								// 采样速度选择
				if ((CONTROL_FRAME_P1>0) && (CONTROL_FRAME_P1<4))
				{
					OscCongData.baudRate = CONTROL_FRAME_P1;
					BaudCfg = 1;							// 修改配置						
				}
				if ((CONTROL_FRAME_P2>0) && (CONTROL_FRAME_P2<101))
				{
					OscCongData.interval = CONTROL_FRAME_P2;		
				}
//				OscCongData.runContinue = CONTROL_FRAME_P3;		
		
				break;
			case FC_START_OSC:								// 启动示波器
				OscCongData.status = 1;
				break;
#if FC_CODE_CONTROL_EN
			case FC_READ_FC_DATA:							// 读内部映射功能码操作
/************************液晶键盘使用****************************/
#if FC_KEY_CONTROL_EN
 dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
  if (FF_PASSWORD == dat)   // 厂家密码正确
            {
                companyPwdPass4Comm = 1;
				userPwdPass4Comm = 1;
				commTicker = 0;   //计数清零，30秒后禁止修改FF组
            }
	if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//液晶键盘读FE组
	{
		LcdReadFEflag = 1;
	} 
		
			#endif
/****************************************************/
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
															// 合成地址，取得数据
				err = OscControlRead(rwaddr, &dat);		    // 执行操作
				CONTROL_FRAME_P3 = dat&0x00ff;				// 数据转串行  低字节
				CONTROL_FRAME_P4 = dat>>8;					// 高字节

				break;
			case FC_WRITE_FC_DATA:							// 写内部功能码操作
				rwaddr =  ((CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);				
				dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
/************************液晶键盘使用****************************/	
		#if FC_KEY_CONTROL_EN
			if((rwaddr==0x2000)&&(LcdKeyFlag==1))
			{
			    if(dat==1)
				{
				keyFunc =KEY_RUN;
			     break;
				 }
			 	if(dat==6)
				{
			   	 	keyFunc =KEY_STOP;
					break;
				}
            }
			if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//液晶键盘读FE组
			{
				LcdReadFEflag = 1;
			}
	    #endif
/************************液晶键盘使用****************************/				
				err = OscControlWriter(rwaddr, dat);		// 操作功能	
							
				break;
#endif
/************************液晶键盘使用****************************/
#if FC_KEY_CONTROL_EN   	// 串行键盘功能禁能，“1”使能，“0”关闭	
			case FC_VERSION_DATA:							// 读机型、版本号操作							
				 LcdKeyFlag = 1;//等于1表示液晶键盘接入
				 dat = 1;// 1代表液晶键盘中MD380机型号
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_LOWER_DATA:	
			 //	 keydata = CONTROL_FRAME_P2;
               dat= FuncCodeAdd(CONTROL_FRAME_P2);
			 //下限值是否与别的功能码有关联  
            if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.lowerLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
                dat = funcCode.all[dat];
			}
			else
			{
				dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
			}
				
		/*		 if (GetGradeIndex(dat, CONTROL_FRAME_P1) == 10)//f0-10下限值特殊处理
            {
                dat = 50 * decNumber[funcCode.code.frqPoint];
				CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
            }  else*/
			{
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
		    }
				 break;
			case FC_UPPER_DATA:							// 写内部上限值操作							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
				 if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.upperLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
                dat = funcCode.all[dat];
			}
			else
			{
			 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
			} 
				 
				
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_INIT_DATA:
				dat= FuncCodeAdd(CONTROL_FRAME_P2);						// 读内部出厂值操作							
				 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].init;
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_DATA:							// 读内部属性操作							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
			dat =	upsend(GetGradeIndex(dat, CONTROL_FRAME_P1));
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_UDATA:
               dat =dispAttributeU0[CONTROL_FRAME_P1].all;
			   CONTROL_FRAME_P3 = dat&0x00ff;
			   CONTROL_FRAME_P4 = dat>>8;
			break;
#endif
#if	FC_CODE_APP_EN
			case FC_APP_WRITE:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// 跳转到BOOT区
				{
					// 烧录工装有引出boot脚时为1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 1;
					else
					{
						EALLOW;//风扇引脚作为启动引脚，这里要恢复成输出
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 2;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 1;
					err = 0x0;
				#endif
				}
				else
				{
					if(rwaddr == FC_APP_POPEDOM)			// 获取烧录权限
					{
						// ---添加用户程序，判断权限
						CONTROL_FRAME_P3 = 1;				// 1 就绪；0 未就绪
						CONTROL_FRAME_P4 = *(Uint16 *)(0x3FFFBA);
					}
					else
					{
						CONTROL_FRAME_P3 = 0;
						err = 0x02;
					}
				}
				break;
			case FC_APP_READ:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_STATE)					// 读取变频器状态
				{
					if(runFlag.bit.run)
					{
					    CONTROL_FRAME_P3 = 1;					// 1 运行；3 停止
						err = 0x02;
					}
					else
					{
					    CONTROL_FRAME_P3 = 3;					// 1 运行；3 停止
					}
					
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// 获取APP区电子标签
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#endif
/***********************************************************************************/
			default:
				err = 0x80;									// 命令出错
				break;
		}
		if (err > 0)
		{
			CONTROL_FRAME_FC |= 0x80;						// 检测查是否出错
            CONTROL_FRAME_P2 = 0x80;
            CONTROL_FRAME_P1 = 0x01;
            CONTROL_FRAME_P4 = 0;
			CONTROL_FRAME_P3 = err;							// 出错返回出错代码
		}
        else if (sciFlag.bit.pwdPass)                      // 密码通过：返回 0x8888
        {
            CONTROL_FRAME_P4 = 0x88;
			CONTROL_FRAME_P3 = 0x88;
        }

#if OSC_CON_CHECK											// CRC16校验并加入响应帧
		crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
		CONTROL_FRAME_CRCL = crcResult;
		CONTROL_FRAME_CRCH = crcResult>>8;
#endif
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 130);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 132);				// 发送132字节响应帧	
		}
		else
#endif	
		SciDataTX(OscConFrameBuf, 8);						// 发送8字节响应帧	
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))
		{
			volatile unsigned char *datatemp;
			//如果没有关闭看门狗  需要关闭
			#if 	FC_CODE_BOOTPIN_EN		// 工装有boot使能脚时使用
			while(1);
			#else
			//   恢复状态
			EALLOW;
			SysCtrlRegs.WDCR = 0x0068; 
			SciaRegs. SCIFFCT.all=0x4000; 
			SysCtrlRegs.CLKCTL.all = 0;
			EDIS;
			
			datatemp = &(SciTxBufStr.len);
			while(*datatemp);
			datatemp = &(SciTxBufStr.busy);
			while(*datatemp);
			while (0 == SciaRegs.SCICTL2.bit.TXEMPTY){}

			asm(" MOV        @SP,#0x0004");
			asm(" LCR        0x3ff7dd");
			asm(" MOV		XAR7,ACC");	
			asm(" LCR *XAR7");
			#endif
		}
#endif	
	}

}
#endif

#if 0
void SciDataRxDeal(void)
{
	Uint16 err = 0;
#if FC_CODE_CONTROL_EN
	Uint16 dat, rwaddr;
#endif

#if OSC_CON_CHECK
	Uint16  crcResult;
	crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
															// CRC16 校验
	if (crcResult != (Uint16)(( (CONTROL_FRAME_CRCH<<8) & 0xff00) | (CONTROL_FRAME_CRCL & 0x00ff) ))
	{
		return ;
	}
#endif

	if (CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD)			// 帧起始出错
#if	FC_CODE_APP_EN	 
	  if(CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD_APP)	
#endif
	{
		return;
	}


	if (CONTROL_FRAME_FC == FC_STOP_OSC)					// 停止示波器命令
	{
		OscDataBufA.rwPI = 0;
		OscDataBufB.rwPI = 0;
		OscDataBufA.full = 0;                               // 清空缓存
        OscDataBufB.full = 0;
		OscCongData.status = 0;
	}
	else if (OscCongData.status)							// 工作于运行状态
	{
		err = 0x80;											// 命令出错
	}
	else
	{
		switch (CONTROL_FRAME_FC)							// 命令判断
		{
            case FC_COMM_TEST:                              // 通讯测试
                // 无需进行任何操作


                break;

            case FC_CHANNL_SEL:								// 通道设置
				OscCongData.chSum = 0;
				
				OscCongData.ch1Addr = CONTROL_FRAME_P1;
				if (CONTROL_FRAME_P1)	OscCongData.chSum ++;
				OscCongData.ch2Addr = CONTROL_FRAME_P2;
				if (CONTROL_FRAME_P2)	OscCongData.chSum ++;
				OscCongData.ch3Addr = CONTROL_FRAME_P3;
				if (CONTROL_FRAME_P3)	OscCongData.chSum ++;
				OscCongData.ch4Addr = CONTROL_FRAME_P4;
				if (CONTROL_FRAME_P4)	OscCongData.chSum ++;
#if OSC_DATA_CHECK
				if (OscCongData.chSum == 3)
				{
					ClearOscBufEnd();						// 清空4字节				
				}
#endif				
				break;
			
			case FC_PARA_CFG:								// 采样速度选择
				if ((CONTROL_FRAME_P1>0) && (CONTROL_FRAME_P1<4))
				{
					OscCongData.baudRate = CONTROL_FRAME_P1;
					BaudCfg = 1;							// 修改配置						
				}
				if ((CONTROL_FRAME_P2>0) && (CONTROL_FRAME_P2<101))
				{
					OscCongData.interval = CONTROL_FRAME_P2;		
				}
//				OscCongData.runContinue = CONTROL_FRAME_P3;		
		
				break;
			case FC_START_OSC:								// 启动示波器
				OscCongData.status = 1;
				break;
#if FC_CODE_CONTROL_EN
			case FC_READ_FC_DATA:							// 读内部映射功能码操作
/************************液晶键盘使用****************************/
#if FC_KEY_CONTROL_EN
 dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
  if (FF_PASSWORD == dat)   // 厂家密码正确
            {
                companyPwdPass4Comm = 1;
				userPwdPass4Comm = 1;
				commTicker = 0;   //计数清零，30秒后禁止修改FF组
            }
	if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//液晶键盘读FE组
	{
		LcdReadFEflag = 1;
	} 
		
			#endif
/****************************************************/
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
															// 合成地址，取得数据
				err = OscControlRead(rwaddr, &dat);		    // 执行操作
				CONTROL_FRAME_P3 = dat&0x00ff;				// 数据转串行  低字节
				CONTROL_FRAME_P4 = dat>>8;					// 高字节

				break;
			case FC_WRITE_FC_DATA:							// 写内部功能码操作
				rwaddr =  ((CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);				
				dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
/************************液晶键盘使用****************************/	
		#if FC_KEY_CONTROL_EN
			if((rwaddr==0x2000)&&(LcdKeyFlag==1))
			{
			    if(dat==1)
				{
				keyFunc =KEY_RUN;
			     break;
				 }
			 	if(dat==6)
				{
			   	 	keyFunc =KEY_STOP;
					break;
				}
            }
			if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//液晶键盘读FE组
			{
				LcdReadFEflag = 1;
			}
	    #endif
/************************液晶键盘使用****************************/				
				err = OscControlWriter(rwaddr, dat);		// 操作功能	
							
				break;
#endif
/************************液晶键盘使用****************************/
#if FC_KEY_CONTROL_EN   	// 串行键盘功能禁能，“1”使能，“0”关闭	
			case FC_VERSION_DATA:							// 读机型、版本号操作							
				 LcdKeyFlag = 1;//等于1表示液晶键盘接入
				 dat = 1;// 1代表液晶键盘中MD380机型号
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_LOWER_DATA:	
			 //	 keydata = CONTROL_FRAME_P2;
               dat= FuncCodeAdd(CONTROL_FRAME_P2);
			 //下限值是否与别的功能码有关联  
            if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.lowerLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
                dat = funcCode.all[dat];
			}
			else
			{
				dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
			}
				
		/*		 if (GetGradeIndex(dat, CONTROL_FRAME_P1) == 10)//f0-10下限值特殊处理
            {
                dat = 50 * decNumber[funcCode.code.frqPoint];
				CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
            }  else*/
			{
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
		    }
				 break;
			case FC_UPPER_DATA:							// 写内部上值操作							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
				 if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.upperLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
                dat = funcCode.all[dat];
			}
			else
			{
			 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
			} 
				 
				
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_INIT_DATA:
				dat= FuncCodeAdd(CONTROL_FRAME_P2);						// 读内部出厂值操作							
				 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].init;
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_DATA:							// 读内部属性操作							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
			dat =	upsend(GetGradeIndex(dat, CONTROL_FRAME_P1));
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_UDATA:
               dat =dispAttributeU0[CONTROL_FRAME_P1].all;
			   CONTROL_FRAME_P3 = dat&0x00ff;
			   CONTROL_FRAME_P4 = dat>>8;
			break;
#endif
#if	FC_CODE_APP_EN

#if DEBUG_MD500_SEARIS
            case FC_APP_READ_WRITE:
                rwaddr = ( (CONTROL_FRAME_P1<<8) & 0xff00) | (CONTROL_FRAME_P2 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// 跳转到BOOT区
				{
					// 烧录工装有引出boot脚时为1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 0x01;
					else
					{
						EALLOW;//风扇引脚作为启动引脚，这里要恢复成输出
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 0x02;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 0x01;
					err = 0x0;
				#endif
				
				    CONTROL_FRAME_P1 = 0x02;
				    CONTROL_FRAME_P2 = 0x00;

				}
				else if(rwaddr == FC_APP_STATE)					// 读取变频器状态
				{  
				    CONTROL_FRAME_P1 = 0x02;
					CONTROL_FRAME_P3 = 0x01;

				    if(runFlag.bit.run)
				   	{
                        if (FORWARD_DIR == runFlag.bit.dir) // F0-12之前的方向
                       	{
							CONTROL_FRAME_P2 = 0x01;  //正转
							//err = 0x02;
						}
						else
						{
                            CONTROL_FRAME_P2 = 0x02;  //反转
                            //err = 0x02;
						}
					}
					else
					{
                        CONTROL_FRAME_P2 = 0x03;     //停机
					}	
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// 获取APP区电子标签
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#else //MD380

			case FC_APP_WRITE:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// 跳转到BOOT区
				{
					// 烧录工装有引出boot脚时为1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 1;
					else
					{
						EALLOW;//风扇引脚作为启动引脚，这里要恢复成输出
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 2;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 1;
					err = 0x0;
				#endif
				}
				else
				{
					if(rwaddr == FC_APP_POPEDOM)			// 获取烧录权限
					{
						// ---添加用户程序，判断权限
						CONTROL_FRAME_P3 = 1;				// 1 就绪；0 未就绪
						CONTROL_FRAME_P4 = *(Uint16 *)(0x3FFFBA);
					}
					else
					{
						CONTROL_FRAME_P3 = 0;
						err = 0x02;
					}
				}
				break;
			case FC_APP_READ:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_STATE)					// 读取变频器状态
				{
					if(runFlag.bit.run)
					{
					    CONTROL_FRAME_P3 = 1;					// 1 运行；3 停止
						err = 0x02;
					}
					else
					{
					    CONTROL_FRAME_P3 = 3;					// 1 运行；3 停止
					}
					
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// 获取APP区电子标签
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#endif

#endif
/***********************************************************************************/
			default:
				err = 0x80;									// 命令出错
				break;
		}
		if (err > 0)
		{
			CONTROL_FRAME_FC |= 0x80;						// 检测查是否出错
            CONTROL_FRAME_P2 = 0x80;
            CONTROL_FRAME_P1 = 0x01;
            CONTROL_FRAME_P4 = 0;
			CONTROL_FRAME_P3 = err;							// 出错返回出错代码
		}
        else if (sciFlag.bit.pwdPass)                      // 密码通过：返回 0x8888
        {
            CONTROL_FRAME_P4 = 0x88;
			CONTROL_FRAME_P3 = 0x88;
        }

#if OSC_CON_CHECK											// CRC16校验并加入响应帧
#if DEBUG_MD500_SEARIS
        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
        {
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-1-2);//5个字节
		    //CONTROL_FRAME_CRCL = crcResult;
		    //CONTROL_FRAME_CRCH = crcResult>>8;
		    CONTROL_FRAME_P4 = crcResult;
		    CONTROL_FRAME_CRCL = crcResult>>8;
		}
		else
		{
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);//6个字节
		    CONTROL_FRAME_CRCL = crcResult;
		    CONTROL_FRAME_CRCH = crcResult>>8;
		    //CONTROL_FRAME_P4 = crcResult;
		    //CONTROL_FRAME_CRCL = crcResult>>8;
		}
		
#else
		crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);//6个字节
		CONTROL_FRAME_CRCL = crcResult;
		CONTROL_FRAME_CRCH = crcResult>>8;
#endif
#endif
#if	FC_CODE_APP_EN
#if DEBUG_MD500_SEARIS
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			APP_CONTROL_FRAME_DATA_NUM = 0x40;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 67);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 69);				// 发送72字节响应帧	
		}
#else   //MD380
		if((CONTROL_FRAME_FC == FC_APP_READ) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 130);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 132);				// 发送132字节响应帧	
		}
#endif		
		else

#endif	
#if DEBUG_MD500_SEARIS
        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
       	{
		    SciDataTX(OscConFrameBuf, 8-1);						// 发送7字节响应帧	
		}
		else
		{
		    SciDataTX(OscConFrameBuf, 8);						// 发送8字节响应帧	
		}
#else
		SciDataTX(OscConFrameBuf, 8);						// 发送8字节响应帧	
#endif
#if	FC_CODE_APP_EN

#if DEBUG_MD500_SEARIS
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))

#else
		if((CONTROL_FRAME_FC == FC_APP_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))
#endif
		{
			volatile unsigned char *datatemp;
			//如果没有关闭看门狗  需要关闭
			#if 	FC_CODE_BOOTPIN_EN		// 工装有boot使能脚时使用
			while(1);
			#else
			//   恢复状态
			EALLOW;
			SysCtrlRegs.WDCR = 0x0068; 
			SciaRegs. SCIFFCT.all=0x4000; 
			SysCtrlRegs.CLKCTL.all = 0;
			EDIS;
			
			datatemp = &(SciTxBufStr.len);
			while(*datatemp);
			datatemp = &(SciTxBufStr.busy);
			while(*datatemp);
			while (0 == SciaRegs.SCICTL2.bit.TXEMPTY){}

			asm(" MOV        @SP,#0x0004");
			asm(" LCR        0x3ff7dd");
			asm(" MOV		XAR7,ACC");	
			asm(" LCR *XAR7");
			#endif
		}
#endif	
	}
}
#endif

#if 0
void SciDataRxDeal(void)
{
	Uint16 err = 0;
#if FC_CODE_CONTROL_EN
	Uint16 dat, rwaddr;
#endif

#if OSC_CON_CHECK
	Uint16  crcResult;
	crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
															// CRC16 校验
	if (crcResult != (Uint16)(( (CONTROL_FRAME_CRCH<<8) & 0xff00) | (CONTROL_FRAME_CRCL & 0x00ff) ))
	{
		return ;
	}
#endif

	if (CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD)			// 帧起始出错
#if	FC_CODE_APP_EN	 
	  if(CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD_APP)	
#endif
	{
		return;
	}


	if (CONTROL_FRAME_FC == FC_STOP_OSC)					// 停止示波器命令
	{
		OscDataBufA.rwPI = 0;
		OscDataBufB.rwPI = 0;
		OscDataBufA.full = 0;                               // 清空缓存
        OscDataBufB.full = 0;
		OscCongData.status = 0;
	}
	else if (OscCongData.status)							// 工作于运行状态
	{
		err = 0x80;											// 命令出错
	}
	else
	{
		switch (CONTROL_FRAME_FC)							// 命令判断
		{
            case FC_COMM_TEST:                              // 通讯测试
                // 无需进行任何操作


                break;

            case FC_CHANNL_SEL:								// 通道设置
				OscCongData.chSum = 0;
				
				OscCongData.ch1Addr = CONTROL_FRAME_P1;
				if (CONTROL_FRAME_P1)	OscCongData.chSum ++;
				OscCongData.ch2Addr = CONTROL_FRAME_P2;
				if (CONTROL_FRAME_P2)	OscCongData.chSum ++;
				OscCongData.ch3Addr = CONTROL_FRAME_P3;
				if (CONTROL_FRAME_P3)	OscCongData.chSum ++;
				OscCongData.ch4Addr = CONTROL_FRAME_P4;
				if (CONTROL_FRAME_P4)	OscCongData.chSum ++;
#if OSC_DATA_CHECK
				if (OscCongData.chSum == 3)
				{
					ClearOscBufEnd();						// 清空4字节				
				}
#endif				
				break;
			
			case FC_PARA_CFG:								// 采样速度选择
				if ((CONTROL_FRAME_P1>0) && (CONTROL_FRAME_P1<4))
				{
					OscCongData.baudRate = CONTROL_FRAME_P1;
					BaudCfg = 1;							// 修改配置						
				}
				if ((CONTROL_FRAME_P2>0) && (CONTROL_FRAME_P2<101))
				{
					OscCongData.interval = CONTROL_FRAME_P2;		
				}
//				OscCongData.runContinue = CONTROL_FRAME_P3;		
		
				break;
			case FC_START_OSC:								// 启动示波器
				OscCongData.status = 1;
				break;
#if FC_CODE_CONTROL_EN
			case FC_READ_FC_DATA:							// 读内部映射功能码操作
/************************液晶键盘使用****************************/
#if FC_KEY_CONTROL_EN
 dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
  if (FF_PASSWORD == dat)   // 厂家密码正确
            {
                companyPwdPass4Comm = 1;
				userPwdPass4Comm = 1;
				commTicker = 0;   //计数清零，30秒后禁止修改FF组
            }
	if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//液晶键盘读FE组
	{
		LcdReadFEflag = 1;
	} 
		
			#endif
/****************************************************/
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
															// 合成地址，取得数据
				err = OscControlRead(rwaddr, &dat);		    // 执行操作
				CONTROL_FRAME_P3 = dat&0x00ff;				// 数据转串行  低字节
				CONTROL_FRAME_P4 = dat>>8;					// 高字节

				break;
			case FC_WRITE_FC_DATA:							// 写内部功能码操作
				rwaddr =  ((CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);				
				dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
/************************液晶键盘使用****************************/	
		#if FC_KEY_CONTROL_EN
			if((rwaddr==0x2000)&&(LcdKeyFlag==1))
			{
			    if(dat==1)
				{
				keyFunc =KEY_RUN;
			     break;
				 }
			 	if(dat==6)
				{
			   	 	keyFunc =KEY_STOP;
					break;
				}
            }
			if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//液晶键盘读FE组
			{
				LcdReadFEflag = 1;
			}
	    #endif
/************************液晶键盘使用****************************/				
				err = OscControlWriter(rwaddr, dat);		// 操作功能	
							
				break;
#endif
/************************液晶键盘使用****************************/
#if FC_KEY_CONTROL_EN   	// 串行键盘功能禁能，“1”使能，“0”关闭	
			case FC_VERSION_DATA:							// 读机型、版本号操作							
				 LcdKeyFlag = 1;//等于1表示液晶键盘接入
				 dat = 1;// 1代表液晶键盘中MD380机型号
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_LOWER_DATA:	
			 //	 keydata = CONTROL_FRAME_P2;
               dat= FuncCodeAdd(CONTROL_FRAME_P2);
			 //下限值是否与别的功能码有关联  
            if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.lowerLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
                dat = funcCode.all[dat];
			}
			else
			{
				dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
			}
				
		/*		 if (GetGradeIndex(dat, CONTROL_FRAME_P1) == 10)//f0-10下限值特殊处理
            {
                dat = 50 * decNumber[funcCode.code.frqPoint];
				CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
            }  else*/
			{
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
		    }
				 break;
			case FC_UPPER_DATA:							// 写内部上限值操作							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
				 if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.upperLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
                dat = funcCode.all[dat];
			}
			else
			{
			 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
			} 
				 
				
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_INIT_DATA:
				dat= FuncCodeAdd(CONTROL_FRAME_P2);						// 读内部出厂值操作							
				 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].init;
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_DATA:							// 读内部属性操作							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
			dat =	upsend(GetGradeIndex(dat, CONTROL_FRAME_P1));
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_UDATA:
               dat =dispAttributeU0[CONTROL_FRAME_P1].all;
			   CONTROL_FRAME_P3 = dat&0x00ff;
			   CONTROL_FRAME_P4 = dat>>8;
			break;
#endif

#if	FC_CODE_APP_EN
#if DEBUG_MD500_SEARIS
            case FC_APP_READ_WRITE:
                rwaddr = ( (CONTROL_FRAME_P1<<8) & 0xff00) | (CONTROL_FRAME_P2 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// 跳转到BOOT区
				{
					// 烧录工装有引出boot脚时为1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 0x01;
					else
					{
						EALLOW;//风扇引脚作为启动引脚，这里要恢复成输出
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 0x02;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 0x01;
					err = 0x0;
				#endif
				
				    CONTROL_FRAME_P1 = 0x02;
				    CONTROL_FRAME_P2 = 0x00;

				}
				else if(rwaddr == FC_APP_STATE)					// 读取变频器状态
				{  
				    CONTROL_FRAME_P1 = 0x02;
					CONTROL_FRAME_P3 = 0x01;

				    if(runFlag.bit.run)
				   	{
                        if (FORWARD_DIR == runFlag.bit.dir) // F0-12之前的方向
                       	{
							CONTROL_FRAME_P2 = 0x01;  //正转
							//err = 0x02;
						}
						else
						{
                            CONTROL_FRAME_P2 = 0x02;  //反转
                            //err = 0x02;
						}
					}
					else
					{
                        CONTROL_FRAME_P2 = 0x03;     //停机
					}	
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// 获取APP区电子标签
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#else //MD380

			case FC_APP_WRITE:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// 跳转到BOOT区
				{
					// 烧录工装有引出boot脚时为1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 1;
					else
					{
						EALLOW;//风扇引脚作为启动引脚，这里要恢复成输出
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 2;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 1;
					err = 0x0;
				#endif
				}
				else
				{
					if(rwaddr == FC_APP_POPEDOM)			// 获取烧录权限
					{
						// ---添加用户程序，判断权限
						CONTROL_FRAME_P3 = 1;				// 1 就绪；0 未就绪
						CONTROL_FRAME_P4 = *(Uint16 *)(0x3FFFBA);
					}
					else
					{
						CONTROL_FRAME_P3 = 0;
						err = 0x02;
					}
				}
				break;
			case FC_APP_READ:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_STATE)					// 读取变频器状态
				{
					if(runFlag.bit.run)
					{
					    CONTROL_FRAME_P3 = 1;					// 1 运行；3 停止
						err = 0x02;
					}
					else
					{
					    CONTROL_FRAME_P3 = 3;					// 1 运行；3 停止
					}
					
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// 获取APP区电子标签
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#endif

#endif
/***********************************************************************************/
			default:
				err = 0x80;									// 命令出错
				break;
		}
		if (err > 0)
		{
			CONTROL_FRAME_FC |= 0x80;						// 检测查是否出错
            CONTROL_FRAME_P2 = 0x80;
            CONTROL_FRAME_P1 = 0x01;
            CONTROL_FRAME_P4 = 0;
			CONTROL_FRAME_P3 = err;							// 出错返回出错代码
		}
        else if (sciFlag.bit.pwdPass)                      // 密码通过：返回 0x8888
        {
            CONTROL_FRAME_P4 = 0x88;
			CONTROL_FRAME_P3 = 0x88;
        }
#if DEBUG_MD500_SEARIS//yz-------------------------
#if OSC_CON_CHECK											// CRC16校验并加入响应帧
        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
        {
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-1-2);//5个字节
		    //CONTROL_FRAME_CRCL = crcResult;
		    //CONTROL_FRAME_CRCH = crcResult>>8;
		    CONTROL_FRAME_P4 = crcResult;
		    CONTROL_FRAME_CRCL = crcResult>>8;
		}
		else
		{
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);//6个字节
		    CONTROL_FRAME_CRCL = crcResult;
		    CONTROL_FRAME_CRCH = crcResult>>8;
		    //CONTROL_FRAME_P4 = crcResult;
		    //CONTROL_FRAME_CRCL = crcResult>>8;
		}
#endif
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			APP_CONTROL_FRAME_DATA_NUM = 0x40;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 67);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 69);				// 发送72字节响应帧	
		}	
		else
#endif	

        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
       	{
		    SciDataTX(OscConFrameBuf, 8-1);						// 发送7字节响应帧	
		}
		else
		{
		    SciDataTX(OscConFrameBuf, 8);						// 发送8字节响应帧	
		}

#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))
		{
			volatile unsigned char *datatemp;
			//如果没有关闭看门狗  需要关闭
			#if 	FC_CODE_BOOTPIN_EN		// 工装有boot使能脚时使用
			while(1);
			#else
			//   恢复状态
			EALLOW;
			SysCtrlRegs.WDCR = 0x0068; 
			SciaRegs. SCIFFCT.all=0x4000; 
			SysCtrlRegs.CLKCTL.all = 0;
			EDIS;
			
			datatemp = &(SciTxBufStr.len);
			while(*datatemp);
			datatemp = &(SciTxBufStr.busy);
			while(*datatemp);
			while (0 == SciaRegs.SCICTL2.bit.TXEMPTY){}

			asm(" MOV        @SP,#0x0004");
			asm(" LCR        0x3ff7dd");
			asm(" MOV		XAR7,ACC");	
			asm(" LCR *XAR7");
			#endif
		}
#endif	

//-----------------------------------------yz
#else //MD380
///////////yz
#if OSC_CON_CHECK											// CRC16校验并加入响应帧
		crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
		CONTROL_FRAME_CRCL = crcResult;
		CONTROL_FRAME_CRCH = crcResult>>8;
#endif
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 130);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 132);				// 发送132字节响应帧	
		}
		else
#endif	
		SciDataTX(OscConFrameBuf, 8);						// 发送8字节响应帧	
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))
		{
			volatile unsigned char *datatemp;
			//如果没有关闭看门狗  需要关闭
			#if 	FC_CODE_BOOTPIN_EN		// 工装有boot使能脚时使用
			while(1);
			#else
			//   恢复状态
			EALLOW;
			SysCtrlRegs.WDCR = 0x0068; 
			SciaRegs. SCIFFCT.all=0x4000; 
			SysCtrlRegs.CLKCTL.all = 0;
			EDIS;
			
			datatemp = &(SciTxBufStr.len);
			while(*datatemp);
			datatemp = &(SciTxBufStr.busy);
			while(*datatemp);
			while (0 == SciaRegs.SCICTL2.bit.TXEMPTY){}

			asm(" MOV        @SP,#0x0004");
			asm(" LCR        0x3ff7dd");
			asm(" MOV		XAR7,ACC");	
			asm(" LCR *XAR7");
			#endif
		}
#endif	

#endif
	}
//yz/////////////
}
#endif
#if	FC_CODE_APP_EN
void copyAPPtoBuff(void)
{
	uint8 i;
	uint16 *ptr;
	ptr = (uint16 *)APPIDDataBuff;
	
#if DEBUG_MD500_SEARIS||DEBUG_MD290_SEARIS||DEBUG_MD380E_SEARIS
    for (i=0;i<64;i++)
	{
		if(i<16)
		{
			OscConFrameAPPBuf[3+i++] = *ptr>>8;
			OscConFrameAPPBuf[3+i]   = *ptr++;
		}
		else
		{
			OscConFrameAPPBuf[3+i++] = 0xff;
			OscConFrameAPPBuf[3+i] = 0xff;
		}
		
	}
#else
	for (i=0;i<128;i++)
	{
		if(i<16)
		{
			OscConFrameAPPBuf[2+i++] = *ptr;
			OscConFrameAPPBuf[2+i] = (*ptr++)>>8;
		}
		else
		{
			OscConFrameAPPBuf[2+i++] = 0xff;
			OscConFrameAPPBuf[2+i] = 0xff;
		}
		
	}
#endif
}
#endif	
/*******************************************************************************
* 函数名称          : void OscDataQcq(void)
* 入口参数			：无
* 出口				：无
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: 按设定的采样间隔将数据写入采集双缓存
* 
********************************************************************************/
void OscDataQcq(void)
{
	static uint8 count = 0;
	static uint8 bufSel = 0;
//	static uint8 check = 0;
	uint8 *dataBuf;
	
	if (OscCongData.status != 1)							// 停止状态
	{
		bufSel = 0;											// 重置
		count = 0;
		OscDataBufA.rwPI = 0;								// 清空缓存
		OscDataBufA.full = 0;	
		OscDataBufB.rwPI = 0;
		OscDataBufB.full = 0;	
		return;
	}
	count ++;
	if (count == OscCongData.interval)						// 计数等于采样间隔
	{
		if (bufSel == 0)									// 操作缓存A
		{
#if OSC_DATA_CHECK
			if (OscDataBufA.rwPI == 0)
				OscDataBufA.check = OSC_DATA_FRAME_HEAD_A ^ OSC_DATA_FRAME_HEAD_B;			// 头校验
#endif			
			dataBuf = (uint8 *)(&(OscDataBufA.oscDataBuf[OscDataBufA.rwPI]) );			
			OscDataBufA.check ^= GetOscData(dataBuf);		// 获取数据取得校验
			OscDataBufA.rwPI += OscCongData.chSum << 1;		// 修改缓存指针
			if (OscDataBufA.rwPI > (OSC_BUF_DATA_LEN - (OscCongData.chSum << 1) ) )	
			{												// 缓存满
				OscDataBufA.rwPI = 0;
				OscDataBufA.full = 1;
				bufSel = 1;
			}	
		}
		else												// 操作缓存B
		{
#if OSC_DATA_CHECK			
			if (OscDataBufB.rwPI == 0)
				OscDataBufB.check = OSC_DATA_FRAME_HEAD_A ^ OSC_DATA_FRAME_HEAD_B;			// 头校验
#endif			
			dataBuf = (uint8 *)(&OscDataBufB.oscDataBuf[OscDataBufB.rwPI]);			
			OscDataBufB.check ^= GetOscData(dataBuf);		// 获取数据取得校验
			OscDataBufB.rwPI += OscCongData.chSum << 1;		// 
			if (OscDataBufB.rwPI > (OSC_BUF_DATA_LEN - (OscCongData.chSum << 1) ) )	
			{												// 缓存满
				OscDataBufB.rwPI = 0;
				OscDataBufB.full = 1;
				bufSel = 0;	
			}		
		}
		count = 0;
	}
}
/*******************************************************************************
* 函数名称          : void OscDataTxDeal(void)
* 入口参数			：无
* 出口				：无
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: 示波器数据发送处理，检测当前状态发送示波器数据
* 
********************************************************************************/
void OscDataTxDeal(void)
{
	static uint8  frame = 0;
	static uint8  bufSel = 0;
	if (OscCongData.status != 1)							// 停止状态
	{
		bufSel = 0;											// 重置
		return;
	}
	
	if (bufSel == 0)
	{
		if (OscDataBufA.full)								// 缓存A满
		{
			OscDataBufA.frameNum = frame;
#if OSC_DATA_CHECK
			OscDataBufA.check ^= frame ;
#endif
//			OscDataCk( (uint8 *)(&OscDataBufA), 67);
			if (SciDataTX((uint8 *)(&OscDataBufA), 68))
			{
				OscDataBufA.full = 0;						// 发送缓存A
				frame++;
				bufSel = 1;			
			}		
		}
		return;
	}
	if (OscDataBufB.full)
	{
		OscDataBufB.frameNum = frame;
#if OSC_DATA_CHECK
			OscDataBufB.check ^= frame;
#endif		
//		OscDataCk( (uint8 *)(&OscDataBufB), 67);

		if (SciDataTX((uint8 *)(&OscDataBufB), 68))
		{
			OscDataBufB.full = 0;
			frame++;
			bufSel = 0;		
		}
	}
}

#if OSC_DATA_CHECK
	#define OSC_GET_CHECK		check ^= *databuf;	databuf++
#else
	#define OSC_GET_CHECK		databuf++
#endif

/*******************************************************************************
* 函数名称          : uint8 GetOscData(Uint16 *databuf)
* 入口参数			：databuf		缓存指针
* 出口				：采样数据校验值
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: 采样示波器数据
* 
********************************************************************************/
uint8 GetOscData(uint8 *databuf)
{
	uint8 check = 0;;
	if (OscCongData.ch1Addr)								// 通道1采样
	{
		*databuf = GET_OSC_DATA(OscCongData.ch1Addr);// & 0xff;
		OSC_GET_CHECK;
		*databuf = GET_OSC_DATA(OscCongData.ch1Addr) >> 8;
		OSC_GET_CHECK;
	}
	if (OscCongData.ch2Addr)
	{
		*databuf = GET_OSC_DATA(OscCongData.ch2Addr);// & 0xff;
		OSC_GET_CHECK;
		*databuf = GET_OSC_DATA(OscCongData.ch2Addr) >> 8;
		OSC_GET_CHECK;
	}
	if (OscCongData.ch3Addr)
	{
		*databuf = GET_OSC_DATA(OscCongData.ch3Addr);// & 0xff;
		OSC_GET_CHECK;
		*databuf = GET_OSC_DATA(OscCongData.ch3Addr) >> 8;
		OSC_GET_CHECK;
	}
	if (OscCongData.ch4Addr)
	{
		*databuf = GET_OSC_DATA(OscCongData.ch4Addr);// & 0xff;
		OSC_GET_CHECK;
		*databuf = GET_OSC_DATA(OscCongData.ch4Addr) >> 8;
		OSC_GET_CHECK;
	}
	return(check);
}												

/*******************************************************************************
* 函数名称          : void ClearOscBufEnd(void)
* 入口参数			：无
* 出口				：无
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: 当通道总数为3时清空缓存最后4字节
* 
********************************************************************************/
#if OSC_DATA_CHECK
void ClearOscBufEnd(void)
{
	OscDataBufA.oscDataBuf[OSC_BUF_DATA_LEN-1] = 0;
	OscDataBufA.oscDataBuf[OSC_BUF_DATA_LEN-2] = 0;
	OscDataBufA.oscDataBuf[OSC_BUF_DATA_LEN-3] = 0;
	OscDataBufA.oscDataBuf[OSC_BUF_DATA_LEN-4] = 0;
	OscDataBufB.oscDataBuf[OSC_BUF_DATA_LEN-1] = 0;
	OscDataBufB.oscDataBuf[OSC_BUF_DATA_LEN-2] = 0;
	OscDataBufB.oscDataBuf[OSC_BUF_DATA_LEN-3] = 0;
	OscDataBufB.oscDataBuf[OSC_BUF_DATA_LEN-4] = 0;
}
#endif


/*******************************************************************************
* 函数名称          : uint8  OscDataCk(uint8 *buf, uint8 len)
* 入口参数			：buf				数据起始地址
*					  len				长度
* 出口				：校验结果
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 05/18/2010
* 说明				: 数据校验功能，返回所有数据异或操作结果
* 
*******************************************************************************
#if OSC_DATA_CHECK
uint8  OscDataCk(uint8 *buf, uint8 len)
{
	uint8 outData = *buf++;
	while (--len)
	{
		outData ^= *buf++;
	}
	return outData;											// 返回输出结果
}
#endif

*/
/*******************************************************************************
* 函数名称          : void OscSciIoInit(void)
* 入口参数			：无
* 出口				：
* 创建	            : Yanyi	
* 版本		        : V0.0.1
* 时间              : 06/9/2010
* 说明				: 示波器后台SCI使用IO口初始化
* 					  现为28035 SCIA初始化代码，根据实际芯片使用IO口线修改
*******************************************************************************/
void OscSciIoInit(void)
{
	EALLOW;
    
	SysCtrlRegs.PCLKCR0.bit.SCIAENCLK = 1;     				// SCI-A 时钟使能    
	GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    					// Enable pull-up for GPIO28 (SCIRXDA)
	GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;	   					// Enable pull-up for GPIO29 (SCITXDA)
	GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  					// Asynch input GPIO28 (SCIRXDA)
	GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   					// Configure GPIO28 for SCIRXDA operation
	GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   					// Configure GPIO29 for SCITXDA operation

#if RS485_ENABLE == 1	

    GpioCtrlRegs.GPAPUD.bit.GPIO27 = 0;    
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;        			// Configure GPIO27, RTS
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;         			// output
    GpioDataRegs.GPADAT.bit.GPIO27 = RS485_R_O;   			// Receive

#endif
	
	
	EDIS;
}


//===================================================================
// 函数名称: 后台写参数
// 参数    ：addr  地址
//			 dat   写数据
// return  ：执行状态(为0执行成功)
// 创建    : Yanyi	
//===================================================================
Uint16 OscControlWriter(Uint16 addr, Uint16 data)
{
    Uint16 oscReturn;
	// 数据写操作
	commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;  // 默认写EEPROM
    sciFlag.all = 0;                                        // 清零通讯状态标志
    oscReturn = CommWrite(addr, data);
    commRcvData.commCmdSaveEeprom = SCI_WRITE_NO_EEPROM;
    return oscReturn;
}


//===================================================================
// 函数名称: 后台读参数
// 入口参数：addr   地址   
//			 dat	读取数据指针
// return	 ：执行状态(为0执行成功)
// 创建	   : Yanyi	
//===================================================================
Uint16 OscControlRead(Uint16 addr, Uint16* result)
{
    
    Uint16 oscReturn = 0;
    // 数据读操作
    sciFlag.all = 0;                                        // 清零通讯状态标志
    oscReturn = CommRead(addr, 1);
    *result = commReadData[0];
    return oscReturn;
}



#else


void OscSciFunction(void)
{}



#endif
#if FC_KEY_CONTROL_EN
//===================================================================
// 函数名称: 功能码地址处理函数
// 入口参数：group   通信接收到的组名  
// 出口	 ：返回当前功能码的组地址
// 创建	   : lichaochao	
//备注：目前液晶键盘不读取BC两组以节省380空间
//===================================================================
uint16 FuncCodeAdd(uint8 group)
{
    uint16 keydata,keyhighH,keygroup;

        keydata = group;
        keyhighH = (keydata & 0xF0);
        keygroup = keydata&0x0F;

        if ((0x00 == keyhighH) || (0xF0 == keyhighH))         // Fx
        {
            keygroup = group&0x0f;
        }
        else
        if ((0xA0 == keyhighH) || (0x40 == keyhighH))       // Ax
        {
            keygroup += FUNCCODE_GROUP_A0;
        }
     if ((keydata & 0xFF) == 0x1F)  // FP
        {
            keygroup = FUNCCODE_GROUP_FP;
        }
        return keygroup;
}



//===================================================================
// 函数名称: 发送液晶键盘显示处理
// 入口参数：curFcindex   当前功能码所在index   
// 出口	 ：返回当前功能码的属性值
// 创建	   : lichaochao	
//备注： 因FR组参数不能通信读取，以下功能码小数点在MD380中处理  
//===================================================================



uint16 upsend(uint16 curFcindex)
{

    union FUNC_ATTRIBUTE attribute;    
    attribute = funcCodeAttribute[curFcindex].attribute;
#if F_FRQ_UINT_ONLY_ONE
    // 最近一次故障频率
    if (GetCodeIndex(funcCode.code.errorScene3.elem.errorFrq) == curFcindex)
    {
        attribute.bit.point = funcCode.code.errorFrqUnit & 0x000F;
    }
    // 第二次故障频率
    else if (GetCodeIndex(funcCode.code.errorScene2.elem.errorFrq) == curFcindex)
    {
        attribute.bit.point = (funcCode.code.errorFrqUnit >> 4) & 0x000F;
    }
    // 第一次故障频率
    else if (GetCodeIndex(funcCode.code.errorScene1.elem.errorFrq) == curFcindex)
    {
        attribute.bit.point = (funcCode.code.errorFrqUnit >> 8 ) & 0x000F;
    }
#endif
return attribute.all;
}
#endif
//end
