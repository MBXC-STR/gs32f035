//======================================================================
//
// Time-stamp: <2010-06-23  LeiMin, 0656>
//
// MODBUS 协议
//======================================================================

#include "f_comm.h"

#define RTU_MASTER_FRAME_NUM_MAX  8      // 主机命令帧的字符个数
#define DP_RCV_CRC_ERR_NUMBER   5        // CRC校验出错次数，到达该次数后置为接收

// MODBUS协议
#define RTUslaveAddress rcvFrame[0]      // RTU帧的从机地址
#define RTUcmd          rcvFrame[1]      // RTU帧的命令字
#define RTUhighAddr     rcvFrame[2]      // RTU帧的地址高字节
#define RTUlowAddr      rcvFrame[3]      // RTU帧的地址低字节
#define RTUhighData     rcvFrame[4]      // RTU帧的数据高字节
#define RTUlowData      rcvFrame[5]      // RTU帧的数据低字节
#define RTUlowCrc       rcvFrame[6]      // RTU帧的CRC校验低字节
#define RTUhighCrc      rcvFrame[7]      // RTU帧的CRC校验高字节

// DP/CANOPEN协议
//----------------------------------------------------------------------------//
// 帧头 3.5字节时间
// 类型   物理地址  命令            操作地址   数据  控制字   过程数据    校验
// 请求   0xf8     0x47/0x48        xxxx       xxxx  xxxxx    PD1~PD11    CRC16
// 应答   0xf8     0x47/0x48        xxxx       xxxx  xxxxx    PD1~PD11    CRC16
// 长度   1byte    1byte            2byte      2byte 2byte    24byte      2byte
//----------------------------------------------------------------------------//
// 帧头
#define DPStartAddr       rcvFrame[0]    // DP卡接收数据帧头
// 命令区
#define DPCmd             rcvFrame[1]    // DP卡接收数据命令
#define DPCmdHighFuncAddr rcvFrame[2]    // DP卡功能码操作命令地址高位
#define DPCmdLowFuncAddr  rcvFrame[3]    // DP卡操作功能码参数地址低位
#define DPDatHighFuncAddr rcvFrame[4]    // DP卡数据地址高位
#define DPDatLowFuncAddr  rcvFrame[5]    // DP卡数据地址地位
// PZD
#define DPPdCtrlHighWord  rcvFrame[6]    // DP卡PZD控制字高位
#define DPPdCtrlLowWord   rcvFrame[7]    // DP卡PZD控制字低位

// KZW
#define DP_START_ADDR       0xf8          // 帧头
#define DP_DATA_MAX_NUM     34            // 最大长度
#define DP_FUNCCODE_READ    0x64          // 读功能码
#define DP_FUNCCODE_WRTITE  0x65          // 写功能码

#define DP_SPEC_ADDR_START_INDEX         (GetCodeIndex(funcCode.code.u3[16]))       //
#define DP_SPEC_ADDR_END_INDEX           (GetCodeIndex(funcCode.code.u3[25]))       //
#define DP_COMM_FRQ_INDEX                (GetCodeIndex(funcCode.code.u3[16]))  // 对应1000  通讯给定频率
#define DP_COMM_CRTL_WORD_INDEX          (GetCodeIndex(funcCode.code.u3[17]))  // 对应2000  通讯设定控制字
#define DP_COMM_DIGIT_DO_INDEX           (GetCodeIndex(funcCode.code.u3[18]))  // 对应2001  通信控制DO
#define DP_COMM_AO1_OUT_INDEX            (GetCodeIndex(funcCode.code.u3[19]))  // 对应2002  通讯控制AO1
#define DP_COMM_AO2_OUT_INDEX            (GetCodeIndex(funcCode.code.u3[20]))  // 对应2003  通信控制AO2
#define DP_COMM_FPM_OUT_INDEX            (GetCodeIndex(funcCode.code.u3[21]))  // 对应2004  通讯控制FMP
#define DP_COMM_CMD_OUT_INDEX            (GetCodeIndex(funcCode.code.u3[22]))  // 对应CMD   通讯控制字
#define DP_COMM_SPEED_OUT_INDEX          (GetCodeIndex(funcCode.code.u3[23]))  // 对应速度  通讯控制frq

Uint16 dpFrqAim; 
Uint16 saveEepromIndex = 0;
union DP_STATUS dpStatus;                 // DP变频器状态返回字 
Uint16 toDpSpeed;
Uint16 Modbus_Standard_ErrInfo[9] = {0, 0x03, 0x01, 0x04, 0x02, 0x03, 0x04, 0x04, 0x04};
//====================================================================
//
// 数据接收后信息解析
//
//====================================================================
void ModbusRcvDataDeal(void)
{   
    if(commRcvData.dpOrModbus == DP)
    {
    	#if 1
    	commRcvData.commAddr = (DPCmdHighFuncAddr << 8) + DPCmdLowFuncAddr;  //  操作功能参数地址
    	
        commRcvData.commData = (DPDatHighFuncAddr << 8) + DPDatLowFuncAddr;  //  更改功能参数值
       
        if (DPCmd == DP_FUNCCODE_READ)                                       //  读功能码
        {
            commRcvData.commCmd = SCI_CMD_READ;
    		commRcvData.commData = 1;                                        //  固定读一个
        }
        else if(DPCmd == DP_FUNCCODE_WRTITE)                                 // 写功能码且保存EEPROM
        {        
            commRcvData.commCmd = SCI_CMD_WRITE;                             // 写命令操作
            commRcvData.commCmdSaveEeprom = SCI_WRITE_NO_EEPROM;             // 不写EE
            if ((commRcvData.commAddr&0xf000) >= 0xA000)
            {
                commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;       // 写EE
            }
        }
        else
        {
            commRcvData.commCmd = 0;                                         // 无命令
        }
        commRcvData.slaveAddr = DP_START_ADDR;                               // 从机地址(DP卡到)
                                                                        	 // 接收到的CRC校验值
    	commRcvData.crcRcv = (rcvFrame[commRcvData.rcvNumMax - 1] << 8) + rcvFrame[commRcvData.rcvNumMax - 2];  
    	
    	commRcvData.crcSize = commRcvData.rcvNumMax - 2;                     // CRC校验长度
        #endif
    }
    else
    {
        if(RTUcmd == SCI_CMD_WRITE_MORE)
        {
            commRcvData.slaveAddr = RTUslaveAddress;                // 从机地址
            commRcvData.commCmd = RTUcmd;                           // 通讯命令
            commRcvData.commAddr = (RTUhighAddr << 8) + RTUlowAddr; // 操作起始地址
            commRcvData.commData = (RTUhighData << 8) + RTUlowData; // 操作寄存器的数量
            
            commRcvData.moreWriteNum = rcvFrame[6];                 // 字节数
            
        	commRcvData.crcSize = commRcvData.moreWriteNum + 7; // CRC校验长度
        	commRcvData.crcRcv = (rcvFrame[commRcvData.crcSize+1] << 8) + rcvFrame[commRcvData.crcSize];     // CRC校验值
        	commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;  // 存储EEPROM命令
        }
        else
        {
            commRcvData.slaveAddr = RTUslaveAddress;                // 从机地址
            commRcvData.commCmd = RTUcmd;                           // 通讯命令
            commRcvData.commAddr = (RTUhighAddr << 8) + RTUlowAddr; // 操作地址
            commRcvData.commData = (RTUhighData << 8) + RTUlowData; // 操作数据
            commRcvData.crcRcv = (RTUhighCrc << 8) + RTUlowCrc;     // CRC校验值    
        	commRcvData.crcSize = 6;                                // CRC校验长度
        	commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;  // 存储EEPROM命令
        }
    }
}


//====================================================================
//
// MODBUS帧头判断
// 参数: tmp-接收帧数据
// 返回: 0-帧头判断过程中
//       1-不需要帧头判断，直接存储接收数据
//
//===================================================================
Uint16 ModbusStartDeal(Uint16 tmp)
{
    if (commTicker >= 4000)                                     // 2s时间
    {
        commRcvData.rcvDataJuageFlag = 0;                       // 长时间没有收完，帧头判断复位
    }
    if((funcCode.code.commProtocolSec == EXTEND_COM_CAR)
        &&(commRcvData.rcvDataJuageFlag)
        )
    {
        return 1;
    }
    
    if ((commTicker > commRcvData.frameSpaceTime))              // 超过3.5个字符时间，新的一帧的开始
    {
        RTUslaveAddress = tmp;          
        // 广播模式    
        if (RTUslaveAddress == 0)       
        {
            commRcvData.rcvNum = 1;
            commRcvData.rcvFlag = 1;                           // 接收到帧的第一个字节，且是广播模式
        }
                                                               // 本机地址
        else if (RTUslaveAddress == funcCode.code.commSlaveAddress) 
        {
            commRcvData.rcvNum = 1;
            commRcvData.rcvFlag = 2;                        
        }
        else if(RTUslaveAddress == DP_START_ADDR)              // DP/CANOPEN
        {
            commRcvData.rcvNum = 1;
            commRcvData.rcvFlag = 2;    
            commRcvData.rcvDataJuageFlag = 1;
        }                                                  
        else                                                   // 其它地址
        {
            commRcvData.rcvFlag = 0;                           // 地址不对应，数据不接收 
        }
        
        return 0;
    }

    return 1;
}


//====================================================================
//
// 更新通讯参数
// 1、接收数据个数
// 2、新帧开始判断时间
// 3、应答延迟时间
//
//====================================================================
void UpdateModbusCommFormat(Uint16 baudRate)
{
    static Uint16 i = 0;
    if(!i)
    {
        commRcvData.rcvNumMax = RTU_MASTER_FRAME_NUM_MAX;      // 初始化接收长度，实际长度在中断判断
        i = 0xcc;
    }
    commRcvData.frameSpaceTime = 385 * 2 / baudRate+ 1;        // 3.5 char time=3.5*(1+8+2)/baud
    commRcvData.delay = (funcCode.code.commProtocolSec == EXTEND_COM_CAR) ? 0 : (funcCode.code.commDelay * 2);
}
//====================================================================//
// name : Uint16 specialAddrShineDeal(Uint16 index,Uint16 data)
// in   : 索引下标 index;数据 data
// out  : 不在表里 0  ,在表里 1
// modf : zhurf 1513
// Time : 2013-06-14
// 说明 : 特殊地址映射表处理(包括所有有效地址，但没实际功能码的地址)
//====================================================================//
Uint16 dataDisp;
Uint16 specialAddrShineDeal(Uint16 index,Uint16 data)
{
    int16 frq,speed;
    if((index >= DP_SPEC_ADDR_START_INDEX)&&(index <= DP_SPEC_ADDR_END_INDEX))
    {
        if(DP_COMM_FRQ_INDEX == index)             // FRQ
        {
            dpFrqAim = data;
        }
        else if(DP_COMM_CRTL_WORD_INDEX == index)  // CMD
        {   
            // 低8位命令，高8位故障码
            commRunCmd = data&0xff;
            if(data >> 8)
            {
                errorOther = ERROR_COMM;
                commErrInfo = COMM_ERROR_EXTENDCAR;
            }
        }
        else if(DP_COMM_DIGIT_DO_INDEX == index)  // DO
        {
            doComm.all = data;
        }
        else if(DP_COMM_AO1_OUT_INDEX == index)  // AO1
        {
            aoComm[1] = data;
        }
        else if(DP_COMM_AO2_OUT_INDEX == index)  // AO2 
        {
            aoComm[2] = data;
        }
        else if(DP_COMM_FPM_OUT_INDEX == index)  // FMP
        {
            aoComm[0] = data;
        }
		#if 0
        else if(DP_COMM_CMD_OUT_INDEX == index)
        {
            // 低8位命令，高8位故障码            // 命令字  
            commRunCmd = data&0xff;
            if(data >> 8)
            {
                errorOther = ERROR_COMM;
                commErrInfo = COMM_ERROR_EXTENDCAR;
            }
        }
		#endif
        else if(DP_COMM_SPEED_OUT_INDEX == index)
        {
            speed = data;                          // 目标转速
            frq = (int32)(int16)speed * polePair * 100 / 60;
            if (ABS_INT16(frq) <= maxFrq)        // 最大频率限制
            {
                dpFrqAim = frq;
            }  
        }
                                                 // 更新RAM,其中16是指U3-16索引
        funcCode.code.u3[index - DP_SPEC_ADDR_START_INDEX + 16] = data;
                                                 
        return 1;
    }
	dataDisp = data;
    return 0;
}
//====================================================================
//
// 反馈DP变频器状态信息
//
//===================================================================
void setDPStatusInfo(void)
{
    dpStatus.all = 0;
    
    // 停机/运行 状态
    dpStatus.bit.run = runFlag.bit.run;     
    // 正转/反转 状态
    dpStatus.bit.fwdRev = runFlag.bit.dir;
    
	// 目标频率到达 
    if ((runFlag.bit.run) 
        && (frq == frqAim))
    {

        dpStatus.bit.frqArrive = 1;
    }

    // 故障
    if (errorCode)
    {
        dpStatus.bit.error = 1; 
    }
    // 故障代码
    dpStatus.bit.errCode = errorCode;
   
}

//====================================================================
//
// 准备发送数据
// 参数: err-通讯命令处理出错信息,为0表示操作成功
// 返回: 1、发送数据长度
//       2、发送数据信息
//
// 转化为标准的MODBUS错误描述
// 01,命令码错误
// 02,地址错误
// 03,数据错误
// 04,命令无法处理
//====================================================================
void ModbusSendDataDeal(Uint16 err)
{
//    Uint16 PZD2ReturnData;
    Uint16 cycleReadDataStartIndex;                     // 循环数据开始地址
	Uint16 indexRead,indexWrite, group, grade;
    Uint16 pzdSaveData;
	Uint16 pzdDataSaveCmd = 0; 
	int16 i;
    if(commRcvData.dpOrModbus == DP)
    {
        #if 1
        commSendData.sendNumMax = DP_DATA_MAX_NUM;
    	// 发送数据
        sendFrame[0] = DP_START_ADDR;                   // 发送帧头
        sendFrame[1] = DPCmd;                           // 发送命令
    		
    	if (!(DPCmd))                                   // 没有操作命令
    	{
            sendFrame[2] = 0x0;                         // 
    	    sendFrame[3] = 0x0;                         // 操作地址
    	    sendFrame[4] = 0x0;                         // 数据高位
            sendFrame[5] = 0x0;                         // 数据低位
    	}
        else if (err)                                   // 处理功能码操作命令存在故障
        {
            sendFrame[1] = DPCmd + 0x80;                // 操作命令 +　0x80
            sendFrame[2] = Modbus_Standard_ErrInfo[err]>>8;
            sendFrame[3] = Modbus_Standard_ErrInfo[err]&0x00ff;
    	    //sendFrame[2] = err>>8;                      // 地址
    	    //sendFrame[3] = err&0x00ff;                  // 
            sendFrame[4] = 0x0;                         // 数据保留
            sendFrame[5] = 0x0;                         // 数据保留
        }
        else if (sciFlag.bit.read)                      // 通讯参数读取处理
        {
            sendFrame[2] = DPCmdHighFuncAddr;           // 操作功能码地址高位
    	    sendFrame[3] = DPCmdLowFuncAddr;            // 操作功能码地址低位           
            sendFrame[4] = commReadData[0] >> 8;        // 返回读取值
            sendFrame[5] = commReadData[0] & 0x00ff;
        }
        else if (sciFlag.bit.write)
        {
            sendFrame[2] = rcvFrame[2];                // 操作功能码地址高位
    	    sendFrame[3] = rcvFrame[3];                // 操作功能码地址低位
    	    sendFrame[4] = rcvFrame[4];                // 数据高位
            sendFrame[5] = rcvFrame[5];                // 数据低位  
        }
 

    	// PD1、PD2接收
        if (!sciFlag.bit.crcChkErr)                    // CRC校验成功
        {
            commRcvData.rcvCrcErrCounter = 0;           // 重新开始判断是否CRC校验出错
        }
        setDPStatusInfo();                              // 状态字 PD1、PD2发送
    	if (motorFc.motorCtrlMode == FUNCCODE_motorCtrlMode_FVC)
    	{
            toDpSpeed = frqFdb;                    // 闭环矢量时反馈实际转速
    	}
        else
        {
            toDpSpeed = frqRunDisp;                // 开环矢量时反馈同步转速
        }
    	sendFrame[6] = rcvFrame[6];                     // 控制字
    	sendFrame[7] = rcvFrame[7];
                                              
    	pzdDataSaveCmd = (DPPdCtrlHighWord << 8) + DPPdCtrlLowWord; // PZD快捷数据保存命令标志
        // PROFIBUS/CANOPEN协议地址为FE-00~FE-11 FE-20~FE-31
		cycleReadDataStartIndex = 8;                    // 起始字节
    	                                                
    	for (i = 0; i < 12; i++)                        // 周期性数据输入输出(PZD1~PZD12)
    	{   
            if(pzdDataSaveCmd&(1<<i))                 // 有效数据上传       
    	    {
                // PZD输出地址(主站->变频器)
        		group = funcCode.group.fe[i] / 100;
                grade = funcCode.group.fe[i] % 100;
        		indexWrite = GetGradeIndex(group, grade);

                // PZD输出 CRC校验成功且不为F0-00
                if ((!sciFlag.bit.crcChkErr) && (indexWrite))
                {
                    pzdSaveData = (rcvFrame[cycleReadDataStartIndex] << 8) + rcvFrame[cycleReadDataStartIndex + 1];

                    if(1 == specialAddrShineDeal(indexWrite,pzdSaveData)) // 特殊地址处理
                    {
                        // do nothing 
                    }
                    else if (ModifyFunccodeUpDown(indexWrite, &pzdSaveData, 0) == COMM_ERR_NONE)
                    {
                        ModifyFunccodeEnter(indexWrite, pzdSaveData);
                    }
                }

    	    }
            // 读取单元全部上传
            // PZD输入地址(变频器->主站)
    	    group = funcCode.group.fe[i + 20] / 100;
            grade = funcCode.group.fe[i + 20] % 100;
    		indexRead = GetGradeIndex(group, grade);
            if (!indexRead)
            {
                sendFrame[cycleReadDataStartIndex++] = 0;
                sendFrame[cycleReadDataStartIndex++] = 0;
            }
            else
            {
        		// PZD输入
        		sendFrame[cycleReadDataStartIndex++] = (funcCode.all[indexRead] >> 8);
            	sendFrame[cycleReadDataStartIndex++] = (funcCode.all[indexRead] & 0x00ff);  
            }

    	} 
        #endif
        
    }
    else
    {
        sendFrame[0] = rcvFrame[0];   // 回复接收帧头前2位
        sendFrame[1] = rcvFrame[1];   // 回复接收帧头前2位
        commSendData.sendNumMax = 8;  // 发送数据长度

        if (err)
        {
            // 标准的MODBUS协议
            if (commProtocol)
            {
                sendFrame[1] = rcvFrame[1]+0x80;   // 回复接收帧头前2位    
                sendFrame[2] = Modbus_Standard_ErrInfo[err];
                commSendData.sendNumMax = 5;      // 发送数据长度
            }
            // 非标准的MODBUS协议
            else
            {
                sendFrame[2] = 0x80;
        	    sendFrame[3] = 0x01;
                sendFrame[4] = 0x00;
                sendFrame[5] = err;
            }
        }
        else if (sciFlag.bit.pwdPass)           // 密码通过：返回0x8888
        {
            sendFrame[2] = RTUhighAddr;
            sendFrame[3] = RTUlowAddr;
#if DEBUG_MD500_SEARIS||DEBUG_MD290_SEARIS
            sendFrame[4] = RTUhighData;//0x88;
            sendFrame[5] = RTUlowData;//0x88;
#else
            sendFrame[4] = 0x88;
            sendFrame[5] = 0x88;
#endif
    	}
        else if (sciFlag.bit.write)             // 写数据操作处理。若有错误，则报错，不会真正写
        {
            sendFrame[2] = RTUhighAddr;
            sendFrame[3] = RTUlowAddr;
            sendFrame[4] = RTUhighData;
            sendFrame[5] = RTUlowData;
    	}
        else if (sciFlag.bit.read)              // 通讯参数读取处理，真正需要读取
        {
            Uint16 sendNum;
            Uint16 readDataStartIndex;
            sendNum = commRcvData.commData << 1;
            
            // 标准的MODEBUS通讯协议
            if (commProtocol)
            {
                sendFrame[2] = sendNum;                 // 接收到的是功能码个数*2
                commSendData.sendNumMax = sendNum + 5;  // 最大发送字符个数
                readDataStartIndex = 3;
            }
            // 非标准MODBUS通讯协议
            else if (commProtocol == 0)
            {
                sendFrame[2] = sendNum >> 8;     // 功能码个数高位
                sendFrame[3] = sendNum & 0x00ff; // 功能码个数低位
                commSendData.sendNumMax = sendNum + 6;    // 最大发送字符个数
                readDataStartIndex = 4;
            }

            // 读取的数据值
            for (i = commRcvData.commData - 1; i >= 0; i--)
            {
                sendFrame[(i << 1) + readDataStartIndex] = commReadData[i] >> 8;
                sendFrame[(i << 1) + readDataStartIndex + 1] = commReadData[i] & 0x00ff;
            }
        }
    }
}


//====================================================================
//
// 通讯出错判断
// 返回: 0、通讯正常
//       1、通讯出错
//
//====================================================================
Uint16 ModbusCommErrCheck(void)
{
    if ((funcCode.code.commProtocolSec == MODBUS)
        &&(funcCode.code.commOverTime) 
        && (commTicker >= (Uint32)funcCode.code.commOverTime * 2 * TIME_UNIT_sciCommOverTime)
        )
    {
        // MODBUS通讯超时检测计时
        commTicker = 0;  
        errorOther = ERROR_COMM;
        commErrInfo = COMM_ERROR_MODBUS;
        return 1;
    }
    else if((funcCode.code.commProtocolSec == EXTEND_COM_CAR)
            && ((curTime.powerOnTimeSec > 10)|| (curTime.powerOnTimeM > 0)) // 上电时间超过10秒才判断
            && (funcCode.code.commExtendOverTime)
            && (commTicker >= (Uint32)funcCode.code.commExtendOverTime * 2 * TIME_UNIT_sciCommOverTime)
            )
    {
        commTicker = 0;  
        errorOther = ERROR_COMM;
        commErrInfo = COMM_ERROR_PROFIBUS;
        return 1;
    }
    
    if ((commRcvData.rcvCrcErrCounter > DP_RCV_CRC_ERR_NUMBER) && (funcCode.code.commProtocolSec == EXTEND_COM_CAR))
    {
       // PROFIBUS CRC校验出错计数
       commRcvData.rcvCrcErrCounter = 0;           // 重新开始判断是否CRC校验出错
       return 1;
    }

    return 0;

}











