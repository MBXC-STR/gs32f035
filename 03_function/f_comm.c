//======================================================================
//
// 通讯处理。ModBus协议。
// 
// Time-stamp: <2008-11-23 11:40:25  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_comm.h"
#include "f_p2p.h"
#include "f_ui.h"
#include "f_osc.h"
#include "f_io.h"

#if F_DEBUG_RAM                     // 仅调试功能，在CCS的build option中定义的宏
#define DEBUG_F_MODBUS          0   // 是否使用通讯部分
#elif 1
#define DEBUG_F_MODBUS          1
#endif

enum COMM_STATUS commStatus;    // 串口初始化为等待接收状态
COMM_SEND_DATA commSendData;
COMM_RCV_DATA commRcvData;
Uint16 commErrInfo;
Uint16 commBaudRate;
Uint16 commType;                 // 串口通讯协议类型
Uint16 commProtocol;             // 通讯协议数据格式
Uint32 commTicker;               // *_0.5ms
Uint32 commSendTicker;                  
Uint32 canLinkTicker;            // *_0.5ms
Uint16 commRunCmd;               // 通讯控制运行命令字
Uint16 aoComm[AO_NUMBER+1];      // 通讯AO控制. 0-HDO, 其他为AO
union DO_SCI doComm;             // 通讯DO控制
Uint16 userPwdPass4Comm;         // 1-用户密码通过，仅通讯FP-01使用
Uint16 companyPwdPass4Comm;      // 1-厂家密码通过，FF组使用
Uint16 groupHidePassComm;        // 1-功能组隐藏密码通过，AF组使用
LOCALF Uint16 sendFrame[36];                           // 从机响应帧
LOCALF Uint16 rcvFrame[36];                            // 接收数据数组(主机命令帧)
LOCALF Uint16 commReadData[RTU_READ_DATA_NUM_MAX];     // 读取的数据
LOCALF union SCI_FLAG sciFlag;                         // SCI使用的标志
Uint16 FuncFeData;              // FE组数据
Uint16 FEHigh_group;
Uint16 FELow_grade;
Uint16 indexgroup;
Uint16 indexgrade;
/************************液晶键盘使用****************************/
#if FC_KEY_CONTROL_EN
extern Uint16 LcdKeyFlag;  // 液晶键盘插入标志位
extern Uint16 LcdReadFEflag; //液晶键盘读取FE组
#endif
/*************************************************/
#if DEBUG_F_MODBUS

#if (DSP_CLOCK == 100)
// 100*10^6/4/(_*8)-1
// 实际波特率是 100*10^6/4/(_*8)
#if !DSP_2803X
LOCALF const DSP_BAUD_REGISTER_DATA dspBaudRegData[BAUD_NUM_MAX + 1] =  // LSPCLK = SYSCLKOUT/4
{
    {   3, 0x0028, 0x00af},           // 0, 300bps
    {   6, 0x0014, 0x0057},           // 1, 600bps
    {  12, 0x000a, 0x002b},           // 2, 1200bps
    {  24, 0x0005, 0x0015},           // 3, 2400bps
    {  48, 0x0002, 0x008a},           // 4, 4800bps
    {  96, 0x0001, 0x0044},           // 5, 9600bps
    { 192, 0x0000, 0x00a1},           // 6, 19200bps
    { 384, 0x0000, 0x0050},           // 7, 38400bps
    { 576, 0x0000, 0x0035},           // 8, 57600bps
    {1152, 0x0000, 0x001a},           // 9, 115200bps
    {1280, 0x0000, 0x000E},           // 10,208300bps
    {2560, 0x0000, 0x000b},           // 11,256000bps
    {5120, 0x0000, 0x0005},           // 12,512000bps
};
#endif
#elif (DSP_CLOCK == 60)                               // DSP运行频率60MHz

LOCALF const DSP_BAUD_REGISTER_DATA dspBaudRegData[BAUD_NUM_MAX + 1] =  // LSPCLK = SYSCLKOUT/4
{
#if DSP_2803X
    {   3, 0x0000, 0x1869,  0},           // 0, 300bps
    {   6, 0x0000, 0x0C34,  0},           // 1, 600bps
    {  12, 0x0000, 0x0619,  8},           // 2, 1200bps
    {  24, 0x0000, 0x030c,  4},           // 3, 2400bps
    {  48, 0x0000, 0x0185, 10},           // 4, 4800bps
    {  96, 0x0000, 0x00c2,  5},           // 5, 9600bps
    { 192, 0x0000, 0x0060, 11},           // 6, 19200bps
    { 384, 0x0000, 0x002f, 13},           // 7, 38400bps
    { 576, 0x0000, 0x001f,  9},           // 8, 57600bps
    {1152, 0x0000, 0x000f,  4},           // 9, 115200bps
    {1280, 0x0000, 0x0008,  0},           // 10, 208300bps
    {2560, 0x0000, 0x0006, 15},           // 11, 256000bps
    {5120, 0x0000, 0x0002, 11},           // 12, 512000bps
#else
    {   3, 0x0018, 0x0069},           //  0, 300bps
    {   6, 0x000c, 0x0034},           //  1, 600bps
    {  12, 0x0006, 0x0019},           //  2, 1200bps
    {  24, 0x0003, 0x000c},           //  3, 2400bps
    {  48, 0x0001, 0x0085},           //  4, 4800bps
    {  96, 0x0000, 0x00c2},           //  5, 9600bps
    { 192, 0x0000, 0x0060},           //  6, 19200bps
    { 384, 0x0000, 0x002f},           //  7, 38400bps
    { 576, 0x0000, 0x001f},           //  8, 57600bps
    {1152, 0x0000, 0x000f},           //  9, 115200bps
    {1280, 0x0000, 0x0008},           // 10, 208300bps
    {2560, 0x0000, 0x0006},           // 11, 256000bps
    {5120, 0x0000, 0x0002},           // 12,512000bps
#endif
};
#endif

#if DSP_2803X
const Uint16 commParitys[4] = {0x10, 0x0c, 0x04, 0x00};  //[STOP PARITYENA PARITY] 100 011 010 000
#else
const Uint16 commParitys[4] = {0x87, 0x67, 0x27, 0x07};
#endif

// 某些功能码，通讯不能进行W
#define COMM_NO_W_FC_0  GetCodeIndex(funcCode.code.tuneCmd)              // 调谐
#define COMM_NO_W_FC_1  GetCodeIndex(funcCode.code.menuMode)             // 菜单模式
#define COMM_NO_W_FC_2  GetCodeIndex(funcCode.code.motorFcM2.tuneCmd)    // 调谐
#define COMM_NO_W_FC_5  GetCodeIndex(funcCode.code.funcParaView)         // 功能菜单模式属性
// 某些功能码，通讯不能进行R
#define COMM_NO_R_FC_0  GetCodeIndex(funcCode.code.userPassword)         // 用户密码
// 某些功能码，通讯不能进行RW
#define COMM_NO_RW_FC_0 GetCodeIndex(funcCode.code.userPasswordReadOnly) // 只读用户密码
#define COMM_READ_CURRENT_FC GetCodeIndex(funcCode.group.u0[4])          // 通讯读取电流


const Uint16 COMM_ERR_INDEX[8] = {10, 8, 2, 7, 6, 9, 12, 13};

LOCALD void CommRcvDataDeal(void);
LOCALD void CommSendDataDeal(void);
LOCALD void resetLinSci(void);
LOCALD void closeRTX(void);
LOCALD void setRxConfig(void);
LOCALD void setTxConfig(void);
LOCALD void commErrorDeal(void);
LOCALD void commStatusDeal(void);
LOCALD void CommDataReRcv(Uint16 tmp);
LOCALD void CommDataSend(void);
LOCALD Uint16 CommRead(Uint16, Uint16);
LOCALD Uint16 CommRwFuncCode(Uint16, Uint16, Uint16 rwMode);
LOCALD void inline CommStartSend(void);

#define COMM_READ_FC    0       // 通讯读功能码
#define COMM_WRITE_FC   1       // 通讯写功能码

// 中断
#if DSP_2803X
void SCI_RXD_isr(void);
void SCI_TXD_isr(void);
interrupt void Lina_Level0_ISR(void);    // LIN-SCI 中断
interrupt void Lina_Level1_ISR(void);    // LIN-SCI 中断
#else
__interrupt void SCI_RXD_isr(void);       // SCI中断
__interrupt void SCI_TXD_isr(void);       // SCI中断
#endif

extern Uint16 ValidateInvType(void);
// RS485的接收发送切换
#if DSP_2803X
#define RTS (GpioDataRegs.GPBDAT.bit.GPIO39)
#define SET_RTS_R (GpioDataRegs.GPBCLEAR.bit.GPIO39 = 1)
#define SET_RTS_T (GpioDataRegs.GPBSET.bit.GPIO39 = 1)
#else
#define RTS (GpioDataRegs.GPADAT.bit.GPIO27)
#define SET_RTS_R (GpioDataRegs.GPACLEAR.bit.GPIO27 = 1)
#define SET_RTS_T (GpioDataRegs.GPASET.bit.GPIO27 = 1)
#endif
#define RS485_R     0
#define RS485_T     1


// 通讯协议数据操作函数
// 目前仅有MODBUS和PROFIBUS
// 是否考虑将手持操作器的协议也为该做法？
#define PROTOCOL_NUM    1
const protocolDeal protocolFunc[PROTOCOL_NUM] =
{
// MODBUS
    { ModbusRcvDataDeal,       ModbusStartDeal,      
      UpdateModbusCommFormat,  ModbusSendDataDeal,
      ModbusCommErrCheck,
    },
};


//=====================================================================
//
// 通讯处理函数
//
//=====================================================================
void SciDeal(void)
{
    Uint16 commErrFlag;

    // 更新串口配置
    UpdateSciFormat();

    // 密码通过有效时间
    if (commTicker >= (Uint32)30 * TIME_UNIT_MS_PER_SEC * 2)    // 30s之后，访问权失效
    {
        userPwdPass4Comm = 0;
        companyPwdPass4Comm = 0;
        groupHidePassComm = 0;
    }

    // 通讯出错检测
	// MODBUS检测方式为串口停顿时间超过通讯应答超时时间设置
	// PROFIBUS检测方式为CRC校验出错次数达到10次以上(PROFIBUS是否有必要这样处理)
    commErrFlag = protocolFunc[commType].CommErrCheck();

    // 通讯出错
    if (commErrFlag)
    {
        // 通讯出错处理
        commErrorDeal();     // 报故障并置串口为接收
    }
    else
    {
        // 通讯过程处理
        commStatusDeal();
    }
}


//=====================================================================
//
// 通讯接收中断函数
//
//=====================================================================
#if DSP_2803X
void SCI_RXD_isr(void)
#else
__interrupt void SCI_RXD_isr(void)
#endif
{
    Uint16 tmp;

#if DSP_2803X
    tmp = LinaRegs.SCIRD;
#else
    tmp = SciaRegs.SCIRXBUF.all;
#endif
    
    // 数据接收帧头判断
    if (protocolFunc[commType].StartDeal(tmp))
    {
		// 为正常的接收数据  0-无效   1-广播地址    2-本机地址
		if (commRcvData.rcvFlag)
		{
	        // 非帧头通讯数据接收
	        CommDataReRcv(tmp);
		}
    }
    
    commTicker = 0;                     // 有接收数据，重新计时
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;    // Issue PIE ACK
}


//=====================================================================
//
// 通讯发送中断函数
//
// 发送一个字符完成，就进入该中断
//
//=====================================================================
#if DSP_2803X
void SCI_TXD_isr(void)
#else
__interrupt void SCI_TXD_isr(void)
#endif
{
	// 通讯发送数据
    CommDataSend();   
    commTicker = 0;                     // 发送一个字符完成，重新计时                        
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;    // Issue PIE ACK
}


#if DSP_2803X
// 高优先级中断
interrupt void Lina_Level0_ISR(void)
{
	Uint32 LinL0IntVect;  

	LinL0IntVect = LinaRegs.SCIINTVECT0.all;

	// 接收中断
	if(LinL0IntVect == 11)
	{
		SCI_RXD_isr();
	}
	//  发送中断
	else if(LinL0IntVect == 12)
	{
		SCI_TXD_isr();
	}
    // other
    else
    {
        PieCtrlRegs.PIEACK.bit.ACK9 = 1;
    }
}

//Low priority BLIN ISR.  Just a placeholder.
interrupt void Lina_Level1_ISR(void)
{
	PieCtrlRegs.PIEACK.bit.ACK9 = 1; 
}
#endif


//=====================================================================
//
// 通讯接收的数据处理函数
//
//=====================================================================  
LOCALD void CommRcvDataDeal(void)
{
    Uint16 writeErr;
    Uint16 i,j;
    Uint16 rcvFrameMore[12];
	// 不同协议的接收数据信息解析
	// 解析操作命令、地址、数据等信息
    protocolFunc[commType].RcvDataDeal();
    // 清SCI标志
    sciFlag.all = 0;

    // CRC校验
    if (commRcvData.crcRcv != CrcValueByteCalc(rcvFrame, commRcvData.crcSize))  // CRC校验故障判断
    {         
        sciFlag.bit.crcChkErr = 1;                      // 置位：CRCErr、send
		commRcvData.rcvCrcErrCounter++;                 // 记录CRC校验出错次数
    }
     // 广播模式
    else if (!commRcvData.slaveAddr)  
    {
		// 广播写操作
        if ((SCI_CMD_WRITE == commRcvData.commCmd)
          || (SCI_CMD_WRITE_RAM == commRcvData.commCmd)
          || (SCI_CMD_WRITE_MORE == commRcvData.commCmd)
           )
        {
			// 置写标志
            sciFlag.bit.write = 1;
        }
        else
        {   
            sciFlag.bit.cmdErr = 1;                                                 // 命令错误
        }
    }
    else //if (RTUslaveAddress == funcCode.code.commSlaveAddress) // 本机地址判断
    {
		if (SCI_CMD_READ == commRcvData.commCmd)       // 读命令操作
        {
            sciFlag.bit.read = 1;                           // 置位：read、send
        }
        else if ((SCI_CMD_WRITE == commRcvData.commCmd) 
                || (SCI_CMD_WRITE_RAM == commRcvData.commCmd)
                || (SCI_CMD_WRITE_MORE == commRcvData.commCmd))      // 写命令操作
        {
            sciFlag.bit.write = 1;			                        
        }
        else
        {   
            sciFlag.bit.cmdErr = 1;                                                 // 命令错误
        }
    }

    // 写数据处理
    if (sciFlag.bit.write)
    {

#if 0
        // 写EEPROM
        if (SCI_CMD_WRITE == commRcvData.commCmd)
        {
            commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;
        }
#endif
        if(SCI_CMD_WRITE_MORE == commRcvData.commCmd)
        {	
            if(((commRcvData.commData*2) != commRcvData.moreWriteNum)
                ||(commRcvData.commData > RTU_WRITE_DATA_NUM_MAX))
            {
                writeErr = COMM_ERR_ADDR;
            }
            else
            {
                j=0;
                for(i=0;i<commRcvData.moreWriteNum;i++)
                {
                    rcvFrameMore[j] = (rcvFrame[7+i]<<8) + rcvFrame[8+i];
                    i++;
                	j++;
                }
                writeErr = WriteMultiReg(commRcvData.commAddr, (Uint16*) (&rcvFrameMore[0]),commRcvData.commData, 1);
                if(writeErr)
				writeErr = COMM_ERR_PARA;
                sciFlag.bit.write = 1;
            }
        }
        else
        {
            writeErr = CommWrite(commRcvData.commAddr, commRcvData.commData);  
        }
        // 写失败
        if (writeErr)
        {
			// 标示写失败故障
            sciFlag.all |= (0x0001 << COMM_ERR_INDEX[writeErr - 1]);
        }  
    }
}

//=====================================================================
//
// 数据接收处理后故障整理
//
//=====================================================================
Uint16 SciErrCheck(void)
{
    Uint16 readErr;
	Uint16 operateErr;
	
	// 初值置无故障
	operateErr = COMM_ERR_NONE;
    
    // 通讯读命令
    if (sciFlag.bit.read)               // 通讯参数读取处理
    {
        if(commRcvData.commData > RTU_READ_DATA_NUM_MAX)    // 最大读取12个数据
        {
            sciFlag.bit.paraOver = 1;   //  参数错误故障
        }
        else
        {
            readErr = CommRead(commRcvData.commAddr, commRcvData.commData);
            if (readErr)
            {
				// 数据读取. 如果错误，置故障位，不需要真正的读取
                sciFlag.all |= (0x0001 << COMM_ERR_INDEX[readErr - 1]);                  
            }
        }
    }

	// 故障信息处理
    if (sciFlag.bit.pwdErr)                 // 密码错误：Err01
    {
        operateErr = COMM_ERR_PWD;
    }
    else if (sciFlag.bit.cmdErr)            // 读写命令错误：Err02
    {
        operateErr = COMM_ERR_CMD;
    }
    else if (sciFlag.bit.crcChkErr)         // CRC校验错误: Err03
    {
        operateErr = COMM_ERR_CRC;
    }
    else if (sciFlag.bit.addrOver)          // 功能码无效地址：Err04
    {
        operateErr = COMM_ERR_ADDR;
    }
    else if (sciFlag.bit.paraOver)          // 功能码无效参数：Err05
    {
        operateErr = COMM_ERR_PARA;
    }
    else if (sciFlag.bit.paraReadOnly)      // 参数更改无效：Err06
    {
        operateErr = COMM_ERR_READ_ONLY;
    }
    else if (sciFlag.bit.systemLocked)      // 系统锁定：返回0x0007
    {
        operateErr = COMM_ERR_SYSTEM_LOCKED;
    }
#if 1   // 目前eeprom储存机制下，不会有该错误，但保险起见，还是保留
    else if (sciFlag.bit.saveFunccodeBusy)  // 正在存储参数：返回0x0008
    {
        operateErr = COMM_ERR_SAVE_FUNCCODE_BUSY;
    }
#endif

	return operateErr;
}


//=====================================================================
//
// 通讯发送数据处理函数
//
//=====================================================================
LOCALF void CommSendDataDeal(void)
{
    int16 error;
    Uint16 crcSend;

    // 判断通讯读写操作故障
    error = SciErrCheck();

    // 准备协议发送数据
    protocolFunc[commType].SendDataDeal(error);

    // 准备CRC校验数据
    crcSend = CrcValueByteCalc(sendFrame, commSendData.sendNumMax - 2);
    sendFrame[commSendData.sendNumMax - 2] = crcSend & 0x00ff;    // CRC低位在前
    sendFrame[commSendData.sendNumMax - 1] = crcSend >> 8;
}


//=====================================================================
//
// 通讯写数据函数
// (通讯也可以修改电机参数)
//
//=====================================================================
Uint16 ConmmWriteAtribte(Uint16 addr, Uint16 data)
{
    Uint16 errType = COMM_ERR_NONE;   // 读取功能码错误类型
    Uint16 high = (addr & 0xF000);

// 通讯设定值(通讯频率设定)
    if (COMM_SET_VALUE_ADDR == addr)
    {
        if ((-10000 > (int16)data) || ((int16)data > 10000))
        {
            errType = COMM_ERR_PARA;
        }
    }
// 通讯控制命令处理
    else if (COMM_CMD1_ADDR == addr)                 
    {
        if ((data == 0)||(data > 8))
        {
            commRunCmd = 0;
            errType = COMM_ERR_PARA;
        }
    }
// DO控制
    else if ((COMM_DO_ADDR == addr) 
            ||(COMM_KEYBORD_TEST == addr)) // 按键测试
    {
        //  无属性判断
    }
// HDO控制
    else if ((COMM_HDO_ADDR == addr)  // HDO控制
            ||(COMM_AO1_ADDR == addr) // AO1控制
            ||(COMM_AO2_ADDR == addr) // AO2控制
            )
    {
        if (data > 0x7FFF)
        {
            errType = COMM_ERR_PARA;
        }
    }
    else if ((addr & 0xFF00) == 0x2f00)   // 通信 FE组专用
    {
        Uint16 dataH,dataL,IndexTmp;
        if((addr & 0xFF00) == 0x2F00)
        {
            dataH = data & 0xf000;
            dataL = data & 0x00ff;
            if(dataH == 0xf000)                  // F
            {
                IndexTmp = (data & 0xf00)>>8;    // FX-00
            }
            //else if(dataH == 0x1000)// 0x1fxx  fp
			else if((data & 0xff00) == 0x1F00)  //by modfied yz1990 2014-06-16
            {
                IndexTmp = 16;
            }
            else if(dataH == 0xA000)             // A
            {
                IndexTmp = (data & 0xf00)>>8;    // FX-00
                IndexTmp +=17;
            }
            else if(dataH == 0xB000)             // B
            {
                IndexTmp = (data & 0xf00)>>8;    // FX-00
                IndexTmp +=33;
            }
            else if(dataH == 0xC000)             //c
            {
                IndexTmp = (data & 0xf00)>>8;    // FX-00
                IndexTmp +=49;
            }
            else if(dataH == 0x7000)             //U0
            {
                IndexTmp = (data & 0xf00)>>8;    // FX-00
                IndexTmp += 67;
            }
            else
            {
                errType = COMM_ERR_ADDR;
            }
            
            if(dataL >= funcCodeGradeAll[IndexTmp])
            {
                errType = COMM_ERR_ADDR;
            }
            return errType;
        }
    }
// 写功能码
    else if ((high == 0x0000)      // Fx-RAM
             || (high == 0xF000)   // Fx
             || (high == 0xA000)   // Ax
             || (high == 0x4000)   // Ax-RAM
             || (high == 0xB000)   // Bx
             || (high == 0x5000)   // Bx-RAM
             || (high == 0xC000)   // Cx
             || (high == 0x6000)   // Cx-RAM
             || ((addr & 0xFF00) == 0x1F00)          // FP，1Fxx
             || ((addr & 0xFF00) == 0x7300)   // U3
        ) 
    {
        errType = CommRwFuncodeAtrrib(addr,data,COMM_WRITE_FC);
    }
    // 为统一编址
    else if((addr & 0xFF00) == 0x7000)
    {
        errType = COMM_ERR_READ_ONLY;
    }
// 地址越界
    else
    {
        errType = COMM_ERR_ADDR;
    }
    return errType;  
}
/*******************************************************************************
// 函数名称         : Uint16 CommWrite(Uint16 addr, Uint16 data)
// 入口参数			: addr  data
// 出口				：errType
// 创建	            : zhurf1513	
// 版本		        : V0.71
// 时间             : 2012-06-28
// 说明				: 通讯写操作,经过判断属性可写才写入寄存器。
********************************************************************************/
Uint16 CommWrite(Uint16 addr, Uint16 data)
{
   Uint16 errType = COMM_ERR_NONE;
   errType = ConmmWriteAtribte(addr, data);
   if(errType) // 属性故障，直接退出
   {
       return errType; 
   }
   else
   {
       errType = CommRwFuncCode(addr, data, COMM_WRITE_FC); // 真正写入
   }
   return errType;

}
//
// 通讯读数据函数

/*******************************************************************************
// 函数名称         : Uint16 CommReadAtribte(Uint16 addr, Uint16 data)
// 入口参数			: addr  data
// 出口				：errType
// 创建	            : zhurf1513	
// 版本		        : V0.71
// 时间             : 2012-06-28
// 说明				: 通讯读操作属性判断
********************************************************************************/
Uint16 CommReadAtribte(Uint16 addr, Uint16 data)
{
    //int16 i;
    Uint16 high = (addr & 0xF000);
    Uint16 low = (addr & 0x00FF);
    Uint16 errType = COMM_ERR_NONE;
	
// 通讯读取停机或运行显示参数
// 可以连续读取多个参数
    if ((addr & 0xFF00) == 0x1000)      // 停机/运行参数
    {
        if (low + data > COMM_PARA_NUM)
        {
            errType = COMM_ERR_ADDR;
        }
    }
// 读取变频器运行状态
    else if ((COMM_STATUS_ADDR == addr)   // 读取变频器运行状态
           ||(COMM_INV_ERROR == addr)     // 读取故障
           ||(COMM_KEYBORD_TEST == addr)) // 读取按键测试值 
    {
        if (data > 0x01)
        {
            errType = COMM_ERR_PARA;
        }
    }
#if FC_KEY_CONTROL_EN
	//读取LCDJ键盘DI端子状态
	else if(((addr & 0xFF00) == 0x7100)) //  &&(LcdKeyFlag == 1)
	{

	} 
#endif
// 读取功能码
    else if ((high == 0xF000) ||     // Fx, 读取功能码值
             (high == 0xA000) ||     // Ax
             (high == 0xB000) ||     // Bx
             (high == 0xC000) ||     // Cx
             (high == 0x7000) ||     // U0
             ((addr & 0xFF00) == 0x1F00)    // FP
        )
    {
        errType = CommRwFuncodeAtrrib(addr, data, COMM_READ_FC);
    }
// 地址越界
    else    
    {
        errType = COMM_ERR_ADDR;
    }
    return errType;
}
/*******************************************************************************
// 函数名称         : LOCALD Uint16 CommRead(Uint16 addr, Uint16 data)
// 入口参数			: addr  data
// 出口				：errType
// 创建	            : zhurf1513	
// 版本		        : V0.71
// 时间             : 2012-06-28
// 说明				: 通讯读操作，属性判断可读，才读取寄存器。
********************************************************************************/
LOCALD Uint16 CommRead(Uint16 addr, Uint16 data)
{
   Uint16 errType = COMM_ERR_NONE;
   errType = CommReadAtribte(addr, data);
   if(errType) // 属性故障，直接退出
   {
       return errType;
   }
   else
   {
       errType = CommRwFuncCode(addr, data, COMM_READ_FC); // 读取
   }
   
   return errType;
}


Uint16 comIndex;
/*******************************************************************************
// 函数名称         :LOCALD Uint16 CommRwFuncodeAtrrib(Uint16 addr, Uint16 data, Uint16 rwMode)
// 入口参数			: addr  data rwmode
// 出口				：errType
// 创建	            : zhurf1513	
// 版本		        : V0.71
// 时间             : 2012-06-28
// 说明				: 通讯读写功能码（不包括特殊地址）操作，属性判断可读写，才读写寄存器。
********************************************************************************/
LOCALD Uint16 CommRwFuncodeAtrrib(Uint16 addr, Uint16 data, Uint16 rwMode)
{
    Uint16 index;
       Uint16 group;
       Uint16 grade;
       Uint16 gradeAdd;
       //int16 i;
       Uint16 tmp;
       //Uint16 highH;
       Uint16 funcCodeGradeComm[FUNCCODE_GROUP_NUM];
       Uint16 errType = COMM_ERR_NONE;
       // 获得功能码组和索引号
       CommGetFuncCodeGroupGrade(addr,&group, &grade);

   // 更新通讯情况下，对每一group，用户可以操作的grade个数
       UpdataFuncCodeGrade(funcCodeGradeComm);

   // 判断group, grade是否合理
       if (COMM_READ_FC == rwMode)     // 通讯读功能码
       {
           if(!data) // 读取 0 个数据
           {
              errType = COMM_ERR_PARA;
              return errType;
           }
           gradeAdd = data - 1;
       }
       else        // 通讯写功能码
       {
           gradeAdd = 0;
       }

       if (grade + gradeAdd >= funcCodeGradeComm[group]) // 超过界限
       {
           errType = COMM_ERR_ADDR;
           return errType;
       }

    index = GetGradeIndex(group, grade);    // 计算功能码序号
    
#if FC_KEY_CONTROL_EN 
   if(LcdKeyFlag !=1)   //等于1时表示液晶键盘接入
#endif
   {
    if ((COMM_NO_RW_FC_0 == index) ||       // 某些功能码，通讯不能进行RW
        //(COMM_NO_RW_FC_1 == index) ||
        ((COMM_WRITE_FC == rwMode) &&       // 某些功苈耄ㄑ恫荒芙蠾
         (//(COMM_NO_W_FC_0 == index) ||
          (COMM_NO_W_FC_1 == index) ||
          //(COMM_NO_W_FC_2 == index) ||
             //(COMM_NO_W_FC_3 == index) ||
             //(COMM_NO_W_FC_4 == index) ||
             (COMM_NO_W_FC_5 == index)
          )
         ) ||
        ((COMM_READ_FC == rwMode) &&
         ((COMM_NO_R_FC_0 == index)         // 某些功能码，通讯不能进行R
          )
         ) 
           )
       {
           errType = COMM_ERR_ADDR;            // 无效地址
           return errType;
       }
    }

   #if DEBUG_MD290_SEARIS   // 屏蔽功能码，通讯不能进行RW
       if (((index >= GetCodeIndex(funcCode.code.motorParaM1.elem.rsvdF11[0]))
           &&(index <= GetCodeIndex(funcCode.code.pgParaM1.elem.fvcPgLoseTime))
           )
           ||((index >= GetCodeIndex(funcCode.code.errorAction[4]))
           &&(index <= GetCodeIndex(funcCode.code.errorShow[1]))
           )
           ||((index >= GetCodeIndex(funcCode.code.osChkValue))
           &&(index <= GetCodeIndex(funcCode.code.devChkTime))
           )
           ||((index >= GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.rsvdF11[0]))
           &&(index <= GetCodeIndex(funcCode.code.motorFcM2.pgPara.elem.fvcPgLoseTime))
           )
           ||((index >= GetCodeIndex(funcCode.code.motorFcM2.vcPara.vcSpdLoopKp1))
           &&(index <= GetCodeIndex(funcCode.code.motorFcM2.vcPara.spdLoopI))
           )
           ||(index ==  GetCodeIndex(funcCode.code.svcMode))
          )
       {
           errType = COMM_ERR_ADDR;            // 无效地址
           return errType;
       }
   #endif

       // 密码校验
       if (COMM_WRITE_FC == rwMode) // 写
       {
           // U3组
           if (group == FUNCCODE_GROUP_U3)
           {
               // 写入U3 PLC卡监视参数组信息
               funcCode.all[index] = data;
               return errType;
           }
           else if (GetCodeIndex(funcCode.code.factoryPassword) == index) // FF-00/0F-00, 厂家密码
           {
               if (COMPANY_PASSWORD == data)   // 厂家密码正确
               {
                   companyPwdPass4Comm = 1;
               }
               else
               {
                   errType = COMM_ERR_PARA;        // 无效参数
               }

               return errType;
           }
           else if (GetCodeIndex(funcCode.code.userPassword) == index) // 用户密码
           {
              #if FC_KEY_CONTROL_EN
             if(LcdKeyFlag !=1)//等于1表示液晶键盘接入
             #endif
             {
               if (data == funcCode.code.userPassword)
               {
                   sciFlag.bit.pwdPass = 1;
                   userPwdPass4Comm = 1;
               }
               else
               {
                errType = COMM_ERR_PWD;
            }
            
            return errType;
        }
      }
    }

    if ((group == FC_GROUP_FACTORY) && (!companyPwdPass4Comm)) // FF组
    {
        errType = COMM_ERR_SYSTEM_LOCKED;   // 系统(厂家功能码)锁定     
        return errType;
       }

       if ((group == FUNCCODE_GROUP_AF) && (!groupHidePassComm))  // AF组
       {
           errType = COMM_ERR_SYSTEM_LOCKED;   // 系统(厂家功能码)锁定
           return errType;
       }

       if (COMM_READ_FC == rwMode)     // 通讯读功能码
       {
           return errType;     // 后面都是通讯写功能码的处理
       }

       tmp = ModifyFunccodeUpDown(index, &data, 0);
       if (COMM_ERR_PARA == tmp)
       {
           errType = COMM_ERR_PARA;
       }
       else if (COMM_ERR_READ_ONLY == tmp)
       {
           errType = COMM_ERR_READ_ONLY;
       }
       else
       {
           if (GetCodeIndex(funcCode.code.tuneCmd) == index)
           {
               errType = ValidateTuneCmd(data, MOTOR_SN_1);
               return errType;
           }
           else if (GetCodeIndex(funcCode.code.paraInitMode) == index) // 功能码初始化
           {
               if (!userPwdPass4Comm)
               {
                   errType = COMM_ERR_SYSTEM_LOCKED;
               }
               else if (FUNCCODE_RW_MODE_NO_OPERATION != funcCodeRwMode) // 正在操作功能码
               {
                   errType = COMM_ERR_SAVE_FUNCCODE_BUSY;
               }
               else if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA == data) // 恢复出厂参数(不包含电机参数)
               {
                   funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA;
               }
               else if (FUNCCODE_paraInitMode_SAVE_USER_PARA == data)    // 保存用户参数
               {
                   funcCodeRwModeTmp = FUNCCODE_paraInitMode_SAVE_USER_PARA;
               }
               else if (FUNCCODE_paraInitMode_RESTORE_USER_PARA == data) // 恢复用户保存的参数
               {

                   if ((funcCode.code.saveUserParaFlag1 == USER_PARA_SAVE_FLAG1)
                       && (funcCode.code.saveUserParaFlag2 == USER_PARA_SAVE_FLAG2))
                   {
                       funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_USER_PARA;
                   }
                   else
                   {
                       errType = COMM_ERR_PARA;
                   }
               }
               else if (FUNCCODE_paraInitMode_CLEAR_RECORD == data) // 清除记录信息
               {
                   ClearRecordDeal();
               }

               else if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL == data) // 恢复出厂参数(包含电机参数)
               {
                   //funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL;
               }
               return errType;
           }
           if (GetCodeIndex(funcCode.code.inverterType) == index) // 修改变频器机型属性判断
           {
               // 机型不能超过范围
               errType = ValidateInvType();
        }
       // 仅DI5可以定义为DI_FUNC_APTP_ZERO
           else if ((GetCodeIndex(funcCode.code.diFunc[0]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[1]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[2]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[3]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[5]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[6]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[7]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[8]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[9]) == index)
           )
           {
            if (data == DI_FUNC_APTP_ZERO)
            {
                   errType = COMM_ERR_PARA;
               }
           }

       }

       return errType;
}
/*******************************************************************************
// 函数名称         : Uint16 CommRwFuncCode(Uint16 addr, Uint16 data, Uint16 rwMode)
// 入口参数			: 地址addr  数据data  读写模式rwMode
// 出口				：错误类型errType 0 表示无错误
// 创建	            : zhurf1513	
// 版本		        : V0.71
// 时间             : 2012-06-28
// 说明				: 读写功能码，直接读写，没有经过任何属性的判断
********************************************************************************/
Uint16 CommRwFuncCode(Uint16 addr, Uint16 data, Uint16 rwMode)
{
    Uint16 highH;
    Uint16 errType = 0;
    Uint16 group = 0,grade = 0;
    int16 i;
    Uint16 Index;
    static  Uint16 canOpen = 0;
    Uint16 low = (addr & 0x00FF);
    highH = (addr & 0xF000);
    // 获取功能参数组和索引
    CommGetFuncCodeGroupGrade(addr,&group,&grade);
    // 获取下标
    Index = GetGradeIndex(group, grade); 
    if(COMM_WRITE_FC==rwMode)
    {
    // 按键测试
        if (COMM_KEYBORD_TEST == addr)
        {
            if (data == 1)
            {
                // 开始按键判断测试
                keyBordTestFlag = 1;
                keyBordValue = 0;
            }
        }
    // 通讯设定值(通讯频率设定)
        else if (COMM_SET_VALUE_ADDR == addr)
        {
            funcCode.code.frqComm = data;
            // 兼容F0-28任意设置，1000地址可以写
            dpFrqAim = (int32)(int16)funcCode.code.frqComm * maxFrq / 10000;
        }
    // 通讯控制命令处理
        else if (COMM_CMD1_ADDR == addr)                 
        {
            if (data == 8)
            {
                if (canOpen == 0)
                {
                    commRunCmd = 1;
                    canOpen = 1;
                }
                else
                {
                    commRunCmd = 2;
                    canOpen = 0;                
                }
            }else if ((1 <= data) && (data <= 7)) // 仅0001-0007命令
            {
                commRunCmd = data;
            } 

        }
    // DO控制
        else if (COMM_DO_ADDR == addr)            
        {

            doComm.all = data;
            
        }
    // HDO控制
        else if (COMM_HDO_ADDR == addr)            
        {

            aoComm[0] = data;
        }
    // AO1控制
        else if (COMM_AO1_ADDR == addr)            
        {
            aoComm[1] = data;
        }
    // AO2控制
        else if (COMM_AO2_ADDR == addr)            
        {
            aoComm[2] = data;
        }
		else if ((Index == GetCodeIndex(funcCode.code.tuneCmd))       // F1-12不能写EEPROM
		|| (Index == GetCodeIndex(funcCode.code.motorFcM2.tuneCmd))   // A2-37不能写EEPROM
		|| (Index == GetCodeIndex(funcCode.code.paraInitMode))        // F5-01不能写EEPROM
		|| (Index == GetCodeIndex(funcCode.code.factoryPassword))     // FF-00不能写EEPROM
		|| (Index == GetCodeIndex(funcCode.code.userPassword))        // F5-03不能写EEPROM
		|| (highH == 0x7300)                                           // U3组不能写EEPROM
		)
		{
			errType = COMM_ERR_NONE;
		} 
        else if (Index == GetCodeIndex(funcCode.code.checkDigitalTube))  //F7-00不保存EEPROM
        {
            funcCode.all[Index] = data;   //RAM
        }

    // 写功能码控制
        else if  ((highH == 0x0000)      // Fx-RAM
                 || (highH == 0xF000)   // Fx
                 || (highH == 0xA000)   // Ax
                 || (highH == 0x4000)   // Ax-RAM
                 || (highH == 0xB000)   // Bx
                 || (highH == 0x5000)   // Bx-RAM
                 || (highH == 0xC000)   // Cx
                 || (highH == 0x6000)   // Cx-RAM
                 || ((addr & 0xFF00) == 0x1F00)   // FP，1Fxx
                 || ((addr & 0xFF00) == 0x7300)   // U3
                 ||((addr & 0xFF00) == 0x2F00)    // 通信专用 FE
            )// 通讯写功能码
        {
            Uint16 dataH,dataL,IndexTmp;
            if((addr & 0xFF00) == 0x2F00)
            {
                dataH = data & 0xf000;
                dataL = data & 0x00ff;
                if(dataH == 0xf000)                  // F
                {
                    IndexTmp = (data & 0xf00)>>8;    // FX-00
                }
               // else if(dataH == 0x1000)// 0x1fxx  fp
			    else if((data & 0xff00) == 0x1F00)  //by modfied yz1990 2014-06-16
                {
                    IndexTmp = 16;
                }
                else if(dataH == 0xA000)             // A
                {
                    IndexTmp = (data & 0xf00)>>8;    // FX-00
                    IndexTmp +=17;
                }
                else if(dataH == 0xB000)             // B
                {
                    IndexTmp = (data & 0xf00)>>8;    // FX-00
                    IndexTmp +=33;
                }
                else if(dataH == 0xC000)             //c
                {
                    IndexTmp = (data & 0xf00)>>8;    // FX-00
                    IndexTmp +=49;
                }
                else if(dataH == 0x7000)             //UX-00
                {
                    IndexTmp = (data & 0xf00)>>8;    // FX-00
                    IndexTmp += 67;
                }
                else
                {
                    errType = COMM_ERR_ADDR;
                    return errType;
                }

                if(dataL >= funcCodeGradeAll[IndexTmp])
                {
                    errType = COMM_ERR_ADDR;
                }
                else
                {
                    data = IndexTmp*100+dataL;
                    ModifyFunccodeEnter(Index, data); 
                }
            }
            else if (COMM_ERR_PARA == ModifyFunccodeEnter(Index, data))
            {
                errType = COMM_ERR_PARA;
            }
            else
            {
                if ((highH >= 0xA000)                                            // 为保存EEPROM地址
                    && (commRcvData.commCmdSaveEeprom == SCI_WRITE_WITH_EEPROM)      // 有保存EEPROM命令
    			)
                {
                    if (FUNCCODE_RW_MODE_NO_OPERATION == funcCodeRwMode)
                    {
                        SaveOneFuncCode(Index);
                    }
                    else                            // 正在操作EEPROM，不响应
                    {
                        errType = COMM_ERR_SAVE_FUNCCODE_BUSY;
                    }
                }
            }
        }     

    }
    // 通讯读功能码
    else 
    {
    // 通讯读取停机或运行显示参数
    // 可以连续读取多个参数
        if ((addr & 0xFF00) == 0x1000)      // 停机/运行参数
        {
            for (i = 0; i < data; i++)
            {
                commReadData[i] = funcCode.group.u0[commDispIndex[i + low]];
                
                // 为通讯读取电流
                if ((i + low) == DISP_OUT_CURRENT)
                {
                    // 通讯读取电流分辨率为0.1A
                    if ((funcCode.code.commReadCurrentPoint)
                        ||(funcCode.code.commProtocolSec == EXTEND_COM_CAR)  // DP || CANOPEN 固化1个小数点 
                        )
                    {
                        if (invPara.type <= invPara.pointLimit) // 小功率2小数点
                        commReadData[i] = commReadData[i] / 10;
                    }
                }
            }
        }
    // 读取变频器运行状态
        else if (COMM_STATUS_ADDR == addr)         
        {

#if FC_KEY_CONTROL_EN
           	 if(tuneCmd&&(LcdKeyFlag ==1))
    		{
    		   if (runFlag.bit.run)
    		   {
    		       commReadData[0] = 0x0005;
    		   }		   
    		   else
    		   {
    		   	   commReadData[0] = 0x0004;
    		   }
    		} 
    		else  
#endif
             if (runFlag.bit.run)
            {
                if (FORWARD_DIR == runFlag.bit.dir) // F0-12之前的方向
                {
                    commReadData[0] = 0x0001;
                }
                else
                {
                    commReadData[0] = 0x0002;
                }
            }
            else
            {
                commReadData[0] = 0x0003;
            }
            
        }
    // 读取故障
        else if (COMM_INV_ERROR == addr)     
        {
            commReadData[0] = errorCode;

        }
    // 读取按键测试值
        else if (COMM_KEYBORD_TEST == addr)    
        {
            if (keyBordValue == 0x01FF)
            {
                commReadData[0] = 1;
            }
            else
            {
                commReadData[0] = 0;
            }

        }
#if FC_KEY_CONTROL_EN
    	//读取LCDJ键盘DI端子状态
    	else if(((addr & 0xFF00) == 0x7100)) //  &&(LcdKeyFlag == 1)
    	{
          
             switch(low)
             {
    	         case 0:
    			     commReadData[0] =(diStatus.a.all>>16) & 0x0000FFFF;   
    	             break;
    			 case 1:
    			      commReadData[0] = diStatus.a.all & 0x0000FFFF;  
    	             break;
    			 case 2:
    			      commReadData[0] = 0; 
    	             break;
    			 case 3:
    			      commReadData[0] = doStatus.a.all & 0x0000FFFF;  
    	             break;
    			case 4:
    			     commReadData[0] = (diFunc.f1.all>>16) & 0x0000FFFF;  
    	             break;
    			case 5:
    			     commReadData[0] = diFunc.f1.all & 0x0000FFFF;  
    	             break;
    			case 6:
    			     commReadData[0] = (diFunc.f2.all>>16) & 0x0000FFFF;   
    	             break;
    			 case 7:
    			      commReadData[0] = diFunc.f2.all & 0x0000FFFF;   
    	             break;
    			default:break;
             } 
            
    	} 
#endif
    // 读取功能码
        else if ((highH == 0xF000) ||     // Fx, 读取功能码值
                 (highH == 0xA000) ||     // Ax
                 (highH == 0xB000) ||     // Bx
                 (highH == 0xC000) ||     // Cx
                 (highH == 0x7000) ||     // U0
                 ((addr & 0xFF00) == 0x1F00)    // FP
            )
        {
            for (i = 0; i < data; i++, Index++)
            {
                commReadData[i] = funcCode.all[Index];  // U0组也可以

                // 为通讯读取电流
                if (COMM_READ_CURRENT_FC == Index)
                {
                    // 通讯读取电流分辨率为0.1A
                    if ((funcCode.code.commReadCurrentPoint)
                        ||(funcCode.code.commProtocolSec == EXTEND_COM_CAR)  // DP || CANOPEN 固化1个小数点 
                        )
                    {   
                        if (invPara.type <= invPara.pointLimit) // 小功率2小数点
                        commReadData[i] = commReadData[i] / 10;
                    }
                }
            }
//读取FE组，可实现在FE组连续读取FE组中映射的不连续地址 by modfied yz1990 2014-07-15
			if((addr & 0xFF00) == 0xFE00)
			{ 		
                for (i = 0; i < data; i++)
                {				
                    FEHigh_group = (funcCode.group.fe[(addr & 0xFF)+i] / 100); //去FE组映射地址高位，注意这里FE组映射地址都为十进制 如FE-02=uF3-04 则这里FE02中值为304，不是0xF304;A1-05,1805,目的和funcCodeGradeSum[FUNCCODE_GROUP_NUM]组号对应
			        FELow_grade  = (funcCode.group.fe[(addr & 0xFF)+i] % 100);
           
                    commReadData[i] = funcCode.all[GetGradeIndex(FEHigh_group, FELow_grade)];  // U0组也可以

                   // 为通讯读取电流				   
				   if (COMM_READ_CURRENT_FC == GetGradeIndex(FEHigh_group, FELow_grade))
				   {
                       // 通讯读取电流分辨率为0.1A
                       if ((funcCode.code.commReadCurrentPoint)
                          ||(funcCode.code.commProtocolSec == EXTEND_COM_CAR)  // DP || CANOPEN 固化1个小数点 
                          )
                       {   
                           if (invPara.type <= invPara.pointLimit) // 小功率2小数点
                           commReadData[i] = commReadData[i] / 10;
                       }
                   }
               
             	}
			}
        }
    }	
        return errType;
        
}

/*******************************************************************************
// 函数名称         : Uint16 CommGetFuncCodeIndex(Uint16 addr, Uint16 data)
// 入口参数			: 地址addr  数据data  
// 出口				：错误类型errType 0 表示无错误
// 创建	            : zhurf1513	
// 版本		        : V0.71
// 时间             : 2012-06-28
// 说明				: 获取通讯功能码的下标
********************************************************************************/
void CommGetFuncCodeGroupGrade(Uint16 addr, Uint16 *group, Uint16 *grade)
{
    Uint16 highH;
    //Uint16 Index;
    highH = (addr & 0xF000);
    // 获取group, grade
    *group = (addr >> 8) & 0x0F;
    *grade = addr & 0xFF;

    // 为FE组功能参数
#if DEBUG_MD500_SEARIS||DEBUG_MD290_SEARIS
    if ((((addr & 0xFF00) == 0xFE00)||((addr & 0xFF00) == 0x0E00))
        && (LcdReadFEflag!=1)
        )
#else
    if (((addr & 0xFF00) == 0xFE00)
        && (LcdReadFEflag!=1)
        )
#endif
    {
        *group = funcCode.group.fe[*grade] / 100;
        *grade = funcCode.group.fe[*grade] % 100;
    }

    LcdReadFEflag = 0;
    if((addr & 0xFF00) == 0x2F00)
    {
        *group = FUNCCODE_GROUP_FE;
    }
    
    if ((0xA000 == highH) || (0x4000 == highH))       // Ax
    {
        *group += FUNCCODE_GROUP_A0;
    }
    else if ((0xB000 == highH) || (0x5000 == highH))  // Bx
    {
        *group += FUNCCODE_GROUP_B0;
    }
    else if ((0xC000 == highH) || (0x6000 == highH))  // Cx
    {
        *group += FUNCCODE_GROUP_C0;
    }
    else if ((addr & 0xFF00) == 0x1F00)  // FP
    {
        *group = FUNCCODE_GROUP_FP;
    }
    else if (0x7000 == highH)            // U0
    {
        *group += FUNCCODE_GROUP_U0;
    }
////  获取下标结束
}


//=====================================================================
//
// 通讯中断发送触发函数
//
//=====================================================================
LOCALF void inline CommStartSend(void)
{
#if DSP_2803X
    LinaRegs.SCITD = sendFrame[0];     // 发送第一个数据
#else
    SciaRegs.SCITXBUF = sendFrame[0];     // 发送第一个数据
#endif
}


//=====================================================================
//
// 通讯数据接收配置
//
//=====================================================================
#if DSP_2803X
void resetLinSci(void)
{
    LinaRegs.SCIGCR0.bit.RESET = 0;
    LinaRegs.SCIGCR0.bit.RESET = 1;
    LinaRegs.SCIGCR1.bit.SWnRST = 0; 
}


void closeRTX(void)
{
    LinaRegs.SCIGCR1.bit.RXENA = 0;
    LinaRegs.SCIGCR1.bit.TXENA = 0;
}


void setRxConfig(void)
{
    LinaRegs.SCIGCR1.bit.RXENA = 1;
    LinaRegs.SCIGCR1.bit.TXENA = 0;
}


void setTxConfig(void)
{ 
    LinaRegs.SCIGCR1.bit.TXENA = 1;
    LinaRegs.SCIGCR1.bit.RXENA = 0;
}
#endif


//====================================================================
//
// 通讯出错处理
//
//===================================================================
void commErrorDeal(void)
{
    errorOther = ERROR_COMM;
    commErrInfo = COMM_ERROR_MODBUS;
    commStatus = SCI_RECEIVE_DATA;        // 置为接收状态
    SET_RTS_R;  // RTS = RS485_R;         // RTS置为接收
    
    #if DSP_2803X
    EALLOW;        
    setRxConfig();
    EDIS;
    #else
    SciaRegs.SCICTL1.all = 0x0021;  // 接收
    SciaRegs.SCICTL2.all = 0x00C2;  // 开启接收中断
    #endif
}


//====================================================================
//
// 通讯状态处理
//
//===================================================================
void commStatusDeal(void)
{
    switch (commStatus)
    {
        // 接收数据
        case SCI_RECEIVE_DATA:
            SET_RTS_R;             // RTS = RS485_R;   // RTS置为接收,防止状态不对
            break;
			
        case SCI_RECEIVE_OK:
            CommRcvDataDeal();
            // 返回数据
            if ((commRcvData.slaveAddr)              // 非广播
                && ((!sciFlag.bit.crcChkErr)         // CRC校验成功或为PROFIBUS协议
                || (funcCode.code.commProtocolSec == EXTEND_COM_CAR))
                )
            {
                CommSendDataDeal();                 // 发送数据准备
                commStatus = SCI_SEND_DATA_PREPARE; // 接收处理完成，准备发送
            }
            else                                    // 广播，DSP响应之后不发送，继续接收
            {
                commStatus = SCI_RECEIVE_DATA;
                SET_RTS_R;  // RTS = RS485_R;       // RTS置为接收
                #if DSP_2803X
                EALLOW;
                setRxConfig();
                EDIS;
                #else
                SciaRegs.SCICTL1.all = 0x0021;  // 接收
                SciaRegs.SCICTL2.all = 0x00C2;  // 开启接收中断
                #endif
                break;
            }

        // 发送数据准备    
        case SCI_SEND_DATA_PREPARE:
            if ((commTicker >= commRcvData.delay)               // 应答延迟
                && (commTicker > commRcvData.frameSpaceTime))   // MODBUS为3.5个字符时间
            {
                SET_RTS_T;  // RTS = RS485_T;                          // RTS置为发送
                #if DSP_2803X
                EALLOW;            
                setTxConfig();
                EDIS;
                #else                    
                SciaRegs.SCICTL1.all = 0x0022;          // 发送
                SciaRegs.SCICTL2.all = 0x00C1;          // 开启发送中断
                #endif
                commStatus = SCI_SEND_DATA;
                commSendData.sendNum = 1;               // 当前发送数据个数置为1
                CommStartSend();                        // 开始发送
                commSendTicker = 0;
            }
            break;

        // 发送数据
        case SCI_SEND_DATA:
            commSendTicker++;
            SET_RTS_T;  // RTS = RS485_T;     // RTS置为发送,防止状态不对
            // 长时间数据发送不成功(1.5倍)
            if (commSendTicker >= ((Uint32)825L*commSendData.sendNumMax/(commBaudRate*10) + 1))   // *2ms
            {
                commStatus = SCI_SEND_OK;
            }
            else
            {
                break;
            }
            
        // 发送数据OK
        case SCI_SEND_OK:
            #if DSP_2803X
			if (LinaRegs.SCIFLR.bit.TXEMPTY)
            #else
            if (SciaRegs.SCICTL2.bit.TXEMPTY)   // Transmitter empty flag, 真正发送完毕
            #endif
            {
                commStatus = SCI_RECEIVE_DATA;  // 发送完毕后，置为接收状态
                SET_RTS_R;  // RTS = RS485_R;                  // RTS置为接收
                #if DSP_2803X
                EALLOW;
                setRxConfig();
                EDIS;
                #else
                SciaRegs.SCICTL1.all = 0x0021;  // 接收
                SciaRegs.SCICTL2.all = 0x00C2;  // 开启接收中断
                #endif
            }
            break;

        default:
            break;
    }
}


//====================================================================
//
// 通讯数据接收处理
//
//===================================================================
void CommDataReRcv(Uint16 tmp)
{
    if (commRcvData.rcvNum < commRcvData.rcvNumMax)  // 接收一帧数据还没有完成
    {
        rcvFrame[commRcvData.rcvNum++] = tmp;
    }

    if(funcCode.code.commProtocolSec == EXTEND_COM_CAR)    //   判断是DP模式下的专用协议还是标准MODBUS
    {
        if((rcvFrame[1] ==SCI_CMD_READ)
            ||(rcvFrame[1] ==SCI_CMD_WRITE)
            ||(rcvFrame[1] ==SCI_CMD_WRITE_MORE))
        {
            commRcvData.dpOrModbus = MD; //  2:modbus
            commRcvData.rcvNumMax = 8;
            // 01 10 f0 08 00 02 04
            if((rcvFrame[1] ==SCI_CMD_WRITE_MORE)&&(commRcvData.rcvNum>=8))
            {
                commRcvData.rcvNumMax = rcvFrame[6] + 9;
            }
        }
        else
        {
            #if 1
            commRcvData.dpOrModbus = DP; // 1:dp 
            commRcvData.rcvNumMax = 34;
            #endif
        }
    }
    else
    {
        commRcvData.dpOrModbus = MD;
        commRcvData.rcvNumMax = 8;
        // 01 10 f0 08 00 02 04
        if((rcvFrame[1] ==SCI_CMD_WRITE_MORE)&&(commRcvData.rcvNum>=8))
        {
            commRcvData.rcvNumMax = rcvFrame[6] + 9;
        }
        
    }
    if (commRcvData.rcvNum >= commRcvData.rcvNumMax) // 接收一帧数据完成
    {
        // PROFIBUS置帧头判断标志
        commRcvData.rcvDataJuageFlag = 0;
        
        if (2 == commRcvData.rcvFlag)                // 本机地址才返回(DSP切换成发送状态)
        {
            #if DSP_2803X
            EALLOW;
            closeRTX();                  // 关闭发送接收
            EDIS;
            #else
            SciaRegs.SCICTL1.all = 0x0004;      // 关闭发送接收，sleep
            SciaRegs.SCICTL2.all = 0x00C0;      // 关闭接收发送中断
            #endif
        }
        
        commStatus = SCI_RECEIVE_OK;            // 标志接收数据完成
        commRcvData.rcvFlag = 0;
    }
}


//====================================================================
//
// 通讯数据发送处理
//
//===================================================================
void CommDataSend(void)
{
	 // 发送一帧数据没有完成
    if (commSendData.sendNum< commSendData.sendNumMax)          
    {
#if DSP_2803X
        LinaRegs.SCITD = sendFrame[commSendData.sendNum++];
#else
        SciaRegs.SCITXBUF = sendFrame[commSendData.sendNum++];
#endif
    }
	// 发送一帧数据全部完成
    else if (commSendData.sendNum >= commSendData.sendNumMax)     
    {
		// 标志发送数据完成
        commStatus = SCI_SEND_OK;      
    }
}


//=====================================================================
//
// 通讯初始化
//
//=====================================================================
void InitSetScia(void)
{
    // 应该放在前面
    commStatus = SCI_RECEIVE_DATA;              // 接收状态   
    SET_RTS_R;  // RTS = RS485_R;               // RTS置为接收
#if DSP_2803X
    EALLOW;
    // reset
	resetLinSci();

	LinaRegs.SCIGCR1.bit.SLEEP = 0;
    LinaRegs.SCIFLR.bit.TXWAKE = 0;  
    LinaRegs.SCIFLR.bit.TXEMPTY = 1;
    LinaRegs.SCIFLR.bit.TXRDY = 1;
	// 配置为数据接收
    setRxConfig(); 

	LinaRegs.SCIGCR1.bit.TIMINGMODE = 1; //Asynchronous Timing
	LinaRegs.SCIGCR1.bit.CLK_MASTER = 1; //Enable SCI Clock
	LinaRegs.SCIGCR1.bit.CONT = 1;		 //Continue on Suspend
	if (funcCode.code.commProtocolSec == EXTEND_COM_CAR)          // 无校验(8-N-1)
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[3];
    else
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[funcCode.code.commParity];
    LinaRegs.SCISETINT.bit.SETRXINT = 1;
    LinaRegs.SCISETINT.bit.SETTXINT = 1;
    LinaRegs.SCIFORMAT.bit.CHAR = 0x7;
    LinaRegs.SCIGCR1.bit.SWnRST = 1; 
       
EDIS;
#else
    SciaRegs.SCICTL1.all = 0x0001;              // SCI软件复位，低有效
    SciaRegs.SCICTL2.all = 0x00C2;
    SciaRegs.SCICCR.all = 0x0087;               // 2 stop bit, No loopback, No parity,8 bits,async mode,idle-line
    SciaRegs.SCIPRI.bit.FREE = 1;
    SciaRegs.SCICTL1.all = 0x0021;              // Relinquish SCI from Reset
#endif
}


//=====================================================================
//
// SCI通讯参数修改函数
//
//=====================================================================
void UpdateSciFormat(void)
{
    Uint16 digit[5];
    Uint16 tmp;
    static Uint16 commParityBak = 0;
	// 获得通讯波特率
    GetNumberDigit1(digit, funcCode.code.commBaudRate);
    //commType = MODBUS;     // 通讯类型
    commType = 0;     // 通讯类型 MD DP CANOPEN 共用
    // 为PROFIBUS
    if (funcCode.code.commProtocolSec == EXTEND_COM_CAR)          // CANOPEN与Modbus兼容
    {
		tmp = digit[1] + 9;  // 波特率 
    }
    // 为MODBUS
    else 
    {
		tmp = digit[commType]; // 波特率
    }
    
    // 获得通讯数据传送格式   
    GetNumberDigit1(digit, funcCode.code.commProtocol);
    commProtocol = digit[commType];
    
	// 更新通讯配置文件
    protocolFunc[commType].UpdateCommFormat(dspBaudRegData[tmp].baudRate);

    commBaudRate = dspBaudRegData[tmp].baudRate;
    
 // 出现接收故障时处理    
#if DSP_2803X
	if ( LinaRegs.SCIFLR.bit.BRKDT
        || LinaRegs.SCIFLR.bit.PE 
        || LinaRegs.SCIFLR.bit.OE
        || LinaRegs.SCIFLR.bit.FE)
#else
    if (SciaRegs.SCIRXST.bit.RXERROR)      
#endif
    {
        InitSetScia();   // 初始化SCI寄存器
    }
#if DSP_2803X

EALLOW;
    if (funcCode.code.commProtocolSec == EXTEND_COM_CAR)          // 无校验(8-N-1)
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[3];
    else
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[funcCode.code.commParity];
    
    LinaRegs.BRSR.bit.SCI_LIN_PSH = dspBaudRegData[tmp].high;
    LinaRegs.BRSR.bit.SCI_LIN_PSL = dspBaudRegData[tmp].low;
    LinaRegs.BRSR.bit.M = dspBaudRegData[tmp].M;
EDIS;
    if(commParityBak != funcCode.code.commParity ) // 数据格式发生改变需要初始化，否则 8-N-1 切换到8-E/O-1将出错
    {
        InitSetScia();   // 初始化SCI寄存器
    }
    commParityBak = funcCode.code.commParity;
#else
    if (funcCode.code.commProtocolSec == EXTEND_COM_CAR)
        SciaRegs.SCICCR.all = commParitys[3];               // 无校验(8-N-1)
    else
        SciaRegs.SCICCR.all = commParitys[funcCode.code.commParity];
    
    SciaRegs.SCIHBAUD = dspBaudRegData[tmp].high;
    SciaRegs.SCILBAUD = dspBaudRegData[tmp].low;
    SciaRegs.SCICTL1.bit.SWRESET = 1;
#endif

}


#if 1//F_DEBUG_RAM    // 仅调试功能，在CCS的build option中定义的宏
void InitSciaGpio(void)
{
#if DSP_2803X  // 2803x平台
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO14 = 0;		// Enable pull-up for GPIO14 (LIN TX)
    GpioCtrlRegs.GPAPUD.bit.GPIO15 = 0;		// Enable pull-up for GPIO15 (LIN RX)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 3;  // Asynch input GPIO15 (LINRXA)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 0x01;  // No qualification for all group A GPIO 0-15
    GpioCtrlRegs.GPACTRL.bit.QUALPRD1 = 0x03;
    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 2;   // Configure GPIO14 for LIN TX operation (2-Enable,0-Disable)
    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 2;   // Configure GPIO15 for LIN RX operation (2-Enable,0-Disable)
    GpioCtrlRegs.GPBPUD.bit.GPIO39 = 0;    
    GpioCtrlRegs.GPBMUX1.bit.GPIO39 = 0;        // Configure GPIO39, RTS
    GpioCtrlRegs.GPBDIR.bit.GPIO39 = 1;         // output
    GpioDataRegs.GPBDAT.bit.GPIO39 = RS485_R;   // Receive
    EDIS;
// 通讯控制使用中断，初始化
    EALLOW;
    PieVectTable.LIN0INTA = &Lina_Level0_ISR;
    PieVectTable.LIN1INTA = &Lina_Level1_ISR;
    EDIS;
	IER |= M_INT9;   	                 // Enable interrupts:
	PieCtrlRegs.PIEIER9.bit.INTx3=1;     // PIE Group 9, INT3
	PieCtrlRegs.PIEIER9.bit.INTx4=1;     // PIE Group 9, INT4	
#else
    EALLOW;    
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // Enable pull-up for GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;    // Enable pull-up for GPIO29 (SCITXDA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // Asynch input GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // Configure GPIO28 for SCIRXDA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // Configure GPIO29 for SCITXDA operation
    GpioCtrlRegs.GPAPUD.bit.GPIO27 = 0;    
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;        // Configure GPIO27, RTS
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;         // output
    GpioDataRegs.GPADAT.bit.GPIO27 = RS485_R;   // Receive
    EDIS;
    // 通讯控制使用中断，初始化
    EALLOW;
    PieVectTable.SCIRXINTA = SCI_RXD_isr;
	PieVectTable.SCITXINTA = SCI_TXD_isr;
    EDIS;
	IER |= M_INT9;   	            //  Enable interrupts:
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
 #endif   
	
}
#endif  // #if F_DEBUG_RAM

#else   // #if DEBUG_F_MODBUS
void InitSetScia(void){}
void InitSciaGpio(void){}
void SciDeal(void){}
void UpdateSciFormat(void){}
interrupt void SCI_RXD_isr(void){}
interrupt void SCI_TXD_isr(void){}
Uint16 CommWrite(Uint16 addr, Uint16 data){}
Uint16 CommRead(Uint16 addr, Uint16 data){}
#endif










