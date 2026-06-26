
/*************** (C) COPYRIGHT 2012  Inovance Technology Co., Ltd****************
* File Name          : f_CANlinkEx.c
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 06/15/2012
* Description        : CANlink实现外部功能，用户自定义实现

********************************************************************************/

#include "f_funcCode.h"
#include "f_comm.h"
#include "f_p2p.h"
#include "f_main.h"
#define DEBUG_F_CANLINKEX   1
#if DEBUG_F_CANLINKEX 
Uint16 Canlink_ErrInfo[9] = {0, 0x03, 0x01, 0x04, 0x02, 0x03, 0x04, 0x04, 0x04};
Uint16 CanNodeRunStat;                                      // 节点运行状态

#if DEBUG_MD500_SEARIS
Uint16 gDeviceInformation[12] = {
                                1,                          // 汇川技术
                                11,                         // 变频器产品
                                273,                        // 0x111 MD500E变频器
                                SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//产品版本号
								
                                
                                300,                        // CANlink3.0 V3.00
                                0,
                                0,
                                0
                                };
#elif DEBUG_MD290_SEARIS
Uint16 gDeviceInformation[12] = {
                                1,                          // 汇川技术
                                11,                         // 变频器产品
                                20,                         // MD290变频器
                                //2900,                       // 0xB54 产品版本号
                                SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//产品版本号
                                
                                300,                        // CANlink3.0 V3.00
                                0,
                                0,
                                0
                                };
#elif DEBUG_MD380E_SEARIS
Uint16 gDeviceInformation[12] = {
                                1,                          // 汇川技术
                                11,                         // 变频器产品
                                271,                         // MD380E变频器
                                SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//产品版本号
                                
                                300,                        // CANlink3.0 V3.00
                                0,
                                0,
                                0
                                };
#else
Uint16 gDeviceInformation[12] = {
                                1,                          // 汇川技术
                                11,                         // 变频器产品
                                263,                        // 0X107 MD380变频器
                                //7500,                       // 0X1D4C产品版本号
                                SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//产品版本号
                                
                                300,                        // CANlink3.0 V3.00
                                0,
                                0,
                                0
                                };
#endif

/*******************************************************************************
* 函数名称          :   Uint16 ReadSingleReg(Uint16 addr, Uint16 *data)
* 入口参数			: 	addr   地址
						*data  数据缓存
* 出口				:   CANLINK_ADDR_ERROR		2						// 地址错误
*                       CANLINK_DATA_ERROR		3						// 数据错误
*                       CANLINK_FUN_DIS_ERROR	4						// 功能禁止错误
* 创建	            : 	
* 版本		        :   V0.0.1
* 时间              :   06/15/2012
* 说明				:   读单个寄存器操作函数
********************************************************************************/
Uint16 ReadSingleReg(Uint16 addr, Uint16 *data)
{
    Uint16 oscReturn;
    // 数据读操作
    sciFlag.all = 0;                                        // 清零通讯状态标志
    oscReturn = CommRead(addr, 1);
    oscReturn = Canlink_ErrInfo[oscReturn];
    *data = commReadData[0];
    return oscReturn;    

}

/*******************************************************************************
* 函数名称          :   Uint16 WriteRegFunTest(Uint16 addr, Uint16 data, Uint16 daTest)
* 入口参数			: 	addr   地址
						data   数据
						daTest "0"仅测试地址    "1" 地址数据都测试
* 出口				:   CANLINK_NO_ERROR		0						// 无错误
*                       CANLINK_ADDR_ERROR		2						// 地址错误
*                       CANLINK_DATA_ERROR		3						// 数据错误
*                       CANLINK_FUN_DIS_ERROR	4						// 功能禁止错误
* 创建	            : 	
* 版本		        :   V0.0.1
* 时间              :   06/15/2012
* 说明				:   写寄存器测试函数，地址测试仅用于测试地址,某些配置中判断地址是否有效等
*                                         数据测试后可调用"WriteRegFun"组成完整写操作
********************************************************************************/
Uint16 WriteRegFunTest(Uint16 addr, Uint16 data, Uint16 daTest)
{
    Uint16 errType = 0;
    errType = ConmmWriteAtribte(addr, data); // 写模式
    errType = Canlink_ErrInfo[errType];
    return errType;
}

/*******************************************************************************
* 函数名称          :   Uint16 WriteRegFun(Uint16 addr, Uint16 data, Uint16 eeprom)
* 入口参数			: 	addr   地址
						*data  数据缓存
						eeprom "1" 掉电保存
* 出口				:   CANLINK_NO_ERROR		0						// 无错误
*                       CANLINK_ADDR_ERROR		2						// 地址错误
*                       CANLINK_DATA_ERROR		3						// 数据错误
*                       CANLINK_FUN_DIS_ERROR	4						// 功能禁止错误
* 创建	            : 	
* 版本		        :   V0.0.1
* 时间              :   06/15/2012
* 说明				:   写寄存器函数，直接写不进行任何判断
********************************************************************************/
Uint16 WriteRegFun(Uint16 addr, Uint16 data, Uint16 eeprom)
{
    Uint16 oscReturn;
    extern Uint16 CommRwFuncCode(Uint16, Uint16, Uint16);
    
	// 数据写操作
    if (eeprom)
        commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;
    else
        commRcvData.commCmdSaveEeprom = SCI_WRITE_NO_EEPROM;    // 返回仅保存RAM
    sciFlag.all = 0;                                        // 清零通讯状态标志

    oscReturn = CommRwFuncCode(addr, data, 1); // COMM_WRITE_FC
    oscReturn = Canlink_ErrInfo[oscReturn];
    //-------------------------//
    commRcvData.commCmdSaveEeprom = SCI_WRITE_NO_EEPROM;    // 返回仅保存RAM

    return oscReturn;

}

/*******************************************************************************
* 函数名称          :   Uint16 WriteSingleReg(Uint16 addr, Uint16 data, Uint16 eeprom)
* 入口参数			: 	addr   地址
						*data  数据缓存
						eeprom "1" 掉电保存
* 出口				:   CANLINK_NO_ERROR		0						// 无错误
*                       CANLINK_ADDR_ERROR		2						// 地址错误
*                       CANLINK_DATA_ERROR		3						// 数据错误
*                       CANLINK_FUN_DIS_ERROR	4						// 功能禁止错误
* 创建	            : 	
* 版本		        :   V0.0.1
* 时间              :   06/15/2012
* 说明				:   写寄存器测试函数
********************************************************************************/
Uint16 WriteSingleReg(Uint16 addr, Uint16 data, Uint16 eeprom)
{
    Uint16 reStat;
    reStat= WriteRegFunTest(addr, data, 1);
    if (reStat)
        return reStat;
    WriteRegFun(addr, data, eeprom);    
    return reStat;
}

/*******************************************************************************
* 函数名称          :   Uint16 WriteMultiReg(Uint16 addr, Uint16 *data, Uint16 len, Uint16 eeprom)
* 入口参数			: 	addr   地址
						*data  数据缓存
						eeprom "1" 掉电保存
* 出口				:   CANLINK_NO_ERROR		0						// 无错误
*                       CANLINK_ADDR_ERROR		2						// 地址错误
*                       CANLINK_DATA_ERROR		3						// 数据错误
*                       CANLINK_FUN_DIS_ERROR	4						// 功能禁止错误
* 创建	            : 	
* 版本		        :   V0.0.1
* 时间              :   06/15/2012
* 说明				:   写寄存器函数，直接写不进行任何判断
*                       特性判断由 WriteRegFunTest 完成，两函数合成完整写操作
********************************************************************************/
Uint16 WriteMultiReg(Uint16 addr, Uint16 *data, Uint16 len, Uint16 eeprom)
{
    Uint16 i, reStat = 0;
    for (i=0; i<len; i++)
    {
        reStat = WriteRegFunTest(addr+i, data[i], 1);
        if (reStat)                                         // 写寄存器测试
        {
            return reStat;
        }
    }
    for (i=0; i<len; i++)                                   // 真实写寄存器
    {
        WriteRegFun(addr+i, data[i], eeprom);
    }
    return reStat;                                          // 返回执行状态
}

/*******************************************************************************
* 函数名称          :   Uint16 WriteRegFunTest(Uint16 addr, Uint16 data, Uint16 daTest)
* 入口参数			: 	addr   地址
						data   数据
						daTest "0"仅测试地址    "1" 地址数据都测试
* 出口				:   CANLINK_NO_ERROR		0						// 无错误
*                       CANLINK_ADDR_ERROR		2						// 地址错误
*                       CANLINK_DATA_ERROR		3						// 数据错误
*                       CANLINK_FUN_DIS_ERROR	4						// 功能禁止错误
* 创建	            : 	
* 版本		        :   V0.0.1
* 时间              :   06/15/2012
* 说明				:   写寄存器测试函数，用于测试地址，某些配置中判断地址是否有效等
*                                         数据测试后可调用"WriteMultiReg"组成完整写操作
********************************************************************************/
Uint16 WriteMultiRegTest(Uint16 addr, Uint16 *data, Uint16 len, Uint16 daTest)
{
    Uint16 i, err = 0;
    for (i=0; i<len; i++)
    {
        err = WriteRegFunTest(addr, data[i], daTest);
        if (err)
            break;            
    }
    return err;
}

/*******************************************************************************
* 函数名称          :   void WriteMultiRegFun(Uint16 addr, Uint16 *data, Uint16 len, Uint16 eeprom)
* 入口参数			: 	addr   地址
						*data  数据缓存
						eeprom "1" 掉电保存
* 出口				:   无
* 创建	            : 	
* 版本		        :   V0.0.1
* 时间              :   06/15/2012
* 说明				:   写寄存器函数，唤腥魏闻卸?
*                       特性判断由 WriteRegFunTest 完成，两函数合成完整写操作
********************************************************************************/
void WriteMultiRegFun(Uint16 addr, Uint16 *data, Uint16 len, Uint16 eeprom)
{
    Uint16 i;
    for (i=0; i<len; i++)
    {
        WriteRegFun(addr, data[i], eeprom);
    }
}

/*******************************************************************************
* 函数名称          : Uint16 ReadMultiReg(Uint16 addr, Uint16 *data, Uint16 len)
* 入口参数			: addr  写操作起始地址
*                     data  数据缓存
*                     len   写长度
* 出口				：CANLINK_NO_ERROR		    0						// 无错误
*                     CANLINK_ADDR_ERROR		2						// 地址错误
*                     CANLINK_DATA_ERROR		3						// 数据错误
*                     CANLINK_FUN_DIS_ERROR 	4						// 功能禁止错误
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 05/10/2012
* 说明				: CANlink数据帧发投嗉拇嫫鞫敛僮?********************************************************************************/
Uint16 ReadMultiReg(Uint16 addr, Uint16 *dataPi, Uint16 len)
{
    Uint16 i, reStat;
    for (i=0; i<len; i++)
    {
        reStat = ReadSingleReg(addr++, dataPi++);          // 读单寄存器操作
        if (reStat)                                         // 写寄存器测试
        {
            return reStat;
        }
    }
    return reStat;                                          // 返回执行状态
}

/*******************************************************************************
* 函数名称          : void CanlinkOnline(void)
* 入口参数			: 
* 出口				:
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 08/03/2012
* 说明				: CANlink总线连接，总线已被连接
********************************************************************************/
void CanlinkOnline(void)
{

}

/*******************************************************************************
* 函数名称          : void CanlinkMoniTimeout(void)
* 入口参数			: 
* 出口				:
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 05/10/2012
* 说明				: CANlink监测器超时处理，CANlink总线掉线
********************************************************************************/
void CanlinkMoniTimeout(void)
{
    // 通讯检测时间不为0
	// 主机超时，直接故障。add 1513 12-11-19
    //if (funcCode.code.canOverTime)
    {
        errorOther = ERROR_COMM;
        commErrInfo = COMM_ERROR_CANLINK; 
    }
}

/*******************************************************************************
* 函数名称          : void CanlinkSafeFun(void)
* 入口参数			: 
* 出口				:
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 05/10/2012
* 说明				: CANlink设备安全处理，当CANlink设备进入安全模式触发该函数
********************************************************************************/
void CanlinkSafeFun(void)
{
    
    WriteSingleReg(0x2000, 6, 0);                           // 写通讯命令停机
    
}

/*******************************************************************************
* 函数名称          : void CanlinkAddrErrro(void)
* 入口参数			: 
* 出口				:
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 05/10/2012
* 说明				: CANlink地址冲突处理
********************************************************************************/
void CanlinkAddrErrro(void)
{
    errorOther = ERROR_COMM;
    commErrInfo = COMM_ERROR_CANLINK_ADDR;     
}

/*******************************************************************************
* 函数名称          : void CANlinkDeviceStat(void)
* 入口参数			: 
* 出口				:
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 05/10/2012
* 说明				: CANlink设备状态栏处理
********************************************************************************/
void CANlinkDeviceStat(void)
{
    CanNodeRunStat = 0;
    if (errorCode)       // 故障 BIT0:1,没故障BIT0:0
    {
        CanNodeRunStat |= 0x01;
    }
    if (runFlag.bit.run) // 运行 BIT1:1,停机  BIT1:0
    {
        CanNodeRunStat |= 0x01<<1;
    }
}
#endif
#if 0 //DEBUG_F_MSC_CTRL
Uint16 MscSlaveNodeId[MAX_SLAVE_NODE_NUM];             // 主机控制的从机节点列表
Uint16 MscSlaveCfgBuf[MAX_SLAVE_NODE_NUM];             // 需要主机进行配置的从机列表
Uint16 MscSlaveNodeBreakTcnt[MAX_SLAVE_NODE_NUM];      // 主机对从机心跳监测计时
Uint16 MscMasterSendBeatCount;                         // 主机心跳监测计时
extern Uint32 gCanlinkMoniBeat; 
extern Uint16 gCanlinkBeatCount;                       // CANlink监测器心跳超时计时，心跳发送计时
extern Uint16 gCanlinkBeatTime, gCanlinkMoniAddr;      // CANlink心跳时间，监测器地址

#if 0
/*******************************************************************************
* 函数名称          : void sendRemotFrame(CANlinkDataBuf *dataPi)
* 入口参数			: *dataPi
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 2012-11-21
* 说明				: 发远程帧，
********************************************************************************/

Uint16 sendRemotFrame(CANlinkDataBuf *dataPi,Uint16 hurtTime, Uint16 nodeId)
{
 	dataPi->msgid.bit.srcSta = 0xff	;					    // 保留
    dataPi->msgid.bit.destSta = nodeId;                     // 目标地址
 	dataPi->msgid.bit.code = 0xff;                          // 保留
    dataPi->msgid.bit.aq = CANlink_ASK;						// 远程帧使用该位响应
	dataPi->msgid.bit.framf = CAN_REMOTE_FRAME;				// 远程帧
	dataPi->data.dByte.dByte2 = CAN_LINK_S_ADDR;            // 主机地址
    dataPi->data.dByte.dByte1 = hurtTime;                   // 心跳时间

	return CanlinkDataTran(dataPi, 4, 1000);			    // 发送远程帧响应
}

/*******************************************************************************
* 函数名称          : void revieceCfgFrame(CANlinkDataBuf *dataPi)
* 入口参数			: *dataPi
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 2012-11-21
* 说明				: 收配置帧，
********************************************************************************/
Uint16 revieceCfgFrame(CANlinkDataBuf *dataPi,Uint16 cfgNum, Uint16 nodeId)
{
 	dataPi->msgid.bit.srcSta = CAN_LINK_S_ADDR;			    // 配置器地址
    dataPi->msgid.bit.destSta = nodeId;                     // 目标地址
 	dataPi->msgid.bit.code = cfgNum|0x10;                   // 第一条配置 0x11
    dataPi->msgid.bit.aq = CANlink_ASK;						// 远程帧使用该位响应
	dataPi->msgid.bit.framf = CAN_CONFIG_FRAME;				// 配置帧
	dataPi->data.byte.byte1 = CAN_LINK_S_ADDR;              // 本节点地址

	dataPi->data.byte.byte2 = 0;
	dataPi->data.byte.byte3 = 0;
	dataPi->data.byte.byte4 = 0;
    
	dataPi->data.byte.byte5 = 0;
	dataPi->data.byte.byte6 = 0;
	dataPi->data.byte.byte7 = 0;
	dataPi->data.byte.byte8 = 0;
	return CanlinkDataTran(dataPi, 8, 1000);			            // 发送远程帧响应
}
#endif
/*******************************************************************************
* 函数名称          : Uint16 BroadFrame(CANlinkDataBuf *dataPi, Uint16 framf, Uint16 mode, Uint16 aq)
* 入口参数			: *dataPi, framf, mode, aq
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 2012-11-21
* 说明				: canlink广播帧发送
********************************************************************************/
Uint16 BroadFrame(CANlinkDataBuf *dataPi, Uint16 framf, Uint16 mode, Uint16 aq)
{
    dataPi->msgid.bit.srcSta = P2P_MSRSTER__ADDR;			// 配置器地址
    dataPi->msgid.bit.destSta = 0;                          // 目标地址
 	dataPi->msgid.bit.code = mode;                          // 第一条配置 0x11
    dataPi->msgid.bit.aq = aq;						        // 远程帧使用该位响应
	dataPi->msgid.bit.framf = framf;				        
	return CanlinkDataTran(dataPi, 0, 1000);			           
}



/*******************************************************************************
* 函数名称          : void MscMastBeatFrameFun(CANlinkDataBuf *dataPi)
* 入口参数			: *dataPi
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 2012-11-21
* 说明				: canlink主机监控从机心跳〈?
********************************************************************************/
void MscMastBeatFrameFun(CANlinkDataBuf *dataPi)
{
    Uint16 i, nodeId,slaveStatus;
    if (!MasterSlaveCtrl.isMaster)                    // 非主机,直接退出
        return;

    nodeId = dataPi->msgid.bit.srcSta;                // 发送心跳帧从机ID
    // 从机报警后，主机选择是否需要停机 u0-45 = 22
    slaveStatus = dataPi->data.dByte.dByte1;           // 发送心跳帧从机状态    

    if((slaveStatus&0x01)&&((funcCode.code.salveCmdFollow%10)>=10)) // 从机故障并且从机故障主机故障生效
    {
        errorOther = ERROR_COMM;
        commErrInfo = COMM_ERROR_P2P_SLAVE_ERROR;  // U0-45=22 
    }
    
    if (nodeId)
    {
        for (i = 0; i < MAX_SLAVE_NODE_NUM; i++)      // 遍历查询该从机
        {
            if (MscSlaveNodeId[i] == nodeId)
            {
                MscSlaveNodeBreakTcnt[i] = 0;         // 检测心跳时间清0
                return;
            }
        }
        // 节点不存在表里，寻找空闲位置存放
        for(i = 0; i < MAX_SLAVE_NODE_NUM; i++)
        {
            if (!MscSlaveNodeId[i]) // 空闲缓存
            {
                MscSlaveNodeId[i] = nodeId;
				MscSlaveNodeBreakTcnt[i] = 0;
                return;
            }
        }
    }
    
}


/*******************************************************************************
* 函数名称          : void CheckSlaveNodeBeatStatus(void)
* 入口参数			: 无
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 2012-11-21
* 说明				: 主从控制主机发送心跳帧、检测节点心跳是否正常
********************************************************************************/
void CheckSlaveNodeBeatStatus()
{
    Uint16 i ,j;
    CANlinkDataBuf dataBuf; 

    if (!MasterSlaveCtrl.isMaster)                    // 非主机,直接退出
    {
        MasterSlaveCtrl.slaveNodeNums = 0;
        return;
    }
    
    if (++MscMasterSendBeatCount >= ((Uint32)MasterSlaveCtrl.mscCommErrTime/CANlink_DEAL_PERIOD) )
    {                                                 // 主机发送心跳帧
        if (CANLINK_RT_SUCC == BroadFrame(&dataBuf,CANLINK_BEAT_FRAME,CANLINK_MASTER_BEAT,CANlink_ASK))
            MscMasterSendBeatCount = 0;               // 成功清零计时
        else
            --MscMasterSendBeatCount;                 // 失败延时一拍发送
    }    
    
    for (i = 0,j = 0; i < MAX_SLAVE_NODE_NUM; i++)    // 遍历判断节点心跳是否正常
    {
        
        if (MscSlaveNodeId[i] > 0)                    // 有效节点
        {
            MscSlaveNodeBreakTcnt[i]++;               // 心跳检测计时
            
            // 心跳时间超过2倍心跳发送时间按时,删除该节点
//            if (MscSlaveNodeBreakTcnt[i] > ((Uint32)2*MasterSlaveCtrl.mscCommErrTime/ CANlink_DEAL_PERIOD))
            if(++MscSlaveNodeBreakTcnt[i]  > ((Uint32)3 * MasterSlaveCtrl.mscCommErrTime / CANlink_DEAL_PERIOD) )
            {
                // 从机从在线到下线，主机就会报警 U0-45=21
                // 目的:防止从机脱机之后不可控，会导致不可控制的现象。如果从机不可控，主机也要停止排查原因再次运行。
                if(funcCode.code.salveCmdFollow/100) // 从机脱线,主机是否要报警停车
                {
                    errorOther = ERROR_COMM;
                    commErrInfo = COMM_ERROR_P2P_SLAVE_OUT; 
                }
                MscSlaveNodeId[i] = 0;
                MscSlaveNodeBreakTcnt[i] = 0;
            }
            else
            {
                j++;                                  // 有效节点数
            }
        }
    }

    MasterSlaveCtrl.slaveNodeNums = j;                // 主机控制的节点数
}




/*******************************************************************************
* 函数名称          : void MSCMasterCfg(void)
* 入口参数			: 无
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 2012-11-21
* 说明				: 主从控制主机参数配置,2ms调用

********************************************************************************/
void MSCMasterCfg(void)
{
    gCANlinkDataCFg[0].cfgAddr = P2P_MSRSTER__ADDR;
    gCANlinkDataCFg[0].srcRegAddr = P2P_STATER_SRC_ADDR;
    gCANlinkDataCFg[0].disRegAddr = P2P_STATER_DES_ADDR;
    gCANlinkDataCFg[0].regLen = 4;
    gCANlinkDataCFg[0].transInterval = MasterSlaveCtrl.MasterSendPeriod;
    //CANlinkRunMode = CANLINK_RUN_MODE;    // RUN模式
	gCanlinkBeatTime = 0;
	gCanlinkMoniAddr = 0;
    gCanlinkMoniBeat = 0;
    gCanlinkBeatCount = 0;
}
 
#endif

