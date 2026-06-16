//======================================================================
//
// Time-stamp: <2011-2-21 Lei.Min, 0656>
//
// P2P comm
// 点对点通讯数据处理
//
//======================================================================


#include "f_ui.h"
#include "f_menu.h"
#include "f_frqSrc.h"
#include "f_comm.h"
#include "f_runSrc.h"
#include "f_io.h"
#include "f_error.h"
#include "f_p2p.h"

#if DEBUG_F_MSC_CTRL

MASTER_SALVE_CONTROL_STRUCT MasterSlaveCtrl;
#if DEBUG_F_MSC_CTRL
Uint16 MscSlaveNodeId[MAX_SLAVE_NODE_NUM];             // 主机控制的从机节点列表
Uint16 MscSlaveCfgBuf[MAX_SLAVE_NODE_NUM];             // 需要主机进行配置的从机列表
Uint16 MscSlaveNodeBreakTcnt[MAX_SLAVE_NODE_NUM];      // 主机对从机心跳监测计时
Uint16 MscMasterSendBeatCount;                         // 主机心跳监测计时
extern Uint32 gCanlinkMoniBeat; 
extern Uint16 gCanlinkBeatCount;                       // CANlink监测器心跳超时计时，心跳发送计时
extern Uint16 gCanlinkBeatTime, gCanlinkMoniAddr;      // CANlink心跳时间，监测器地址
extern Uint16 gCanlinkRecDataAddr[CANLINK_RECDATA_LEN];            // CANlink接收点对多数据帧地址

extern void CANlinkDelConfig(void);
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
* 说明				: canlink主机监控从机心跳帧处理
********************************************************************************/
void MscMastBeatFrameFun(CANlinkDataBuf *dataPi)
{
    Uint16 i, nodeId,slaveStatus;

    if (!MasterSlaveCtrl.isMaster)                    // 非主机,直接退出 // 主从控制无效效
        return;
    if(dataPi->msgid.bit.srcSta == P2P_MSRSTER__ADDR) // 过滤掉主机心跳，在设置主机-->从机过程，不记录节点
        return;
        
    nodeId = dataPi->msgid.bit.srcSta;                // 发送心跳帧从机ID

    slaveStatus = dataPi->data.dByte.dByte1;           // 发送心跳帧从机状态
    if((slaveStatus&0x01)) // 从机故障并且从机故障主机故障生效
    {
        errorOther = ERROR_P2P_SLAVE;
        //commErrInfo = COMM_ERROR_P2P_SLAVE_ERROR;       // U0-45=22 
    }
    
    if (nodeId)
    {
        for (i = 0; i < MAX_SLAVE_NODE_NUM; i++)        // 遍历查询该从机
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
            if (!MscSlaveNodeId[i])                   // 空闲缓存
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
                if(MasterSlaveCtrl.masterDispSlaveOut) // 从机掉站
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
* 说明				: 主从控制主机参数配置
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

/*******************************************************************************
* 函数名称          : void MscClrNodeSum(void)
* 入口参数			: 无
* 出口				：无
* 创建	            :
* 版本		        : V0.0.1
* 时间              : 2012-11-21
* 说明				: 清除节点数
********************************************************************************/
void MscClrNodeSum(void)
{
	static Uint16 i = 0;
    for(i = 0; i < MAX_SLAVE_NODE_NUM; i++)		                // 删除节点记录
    {
        MscSlaveNodeId[i] = 0;
    } 
    MasterSlaveCtrl.slaveNodeNums = 0;
}

 
#endif

/*******************************************************************************
* 函数名称          : void LimitData(int16 *data, int16 max, int16 min)
* 入口参数			: 	*data 需要上下限限制的变量
*                       max     变量上限值
						min     变量下限值
* 出口				：
* 创建	            :
********************************************************************************/
void LimitData(int16 *data, int16 max, int16 min)
{
    if (*data > max)
    {
        *data = max;
    }
    else if (*data < min)
    {
        *data = min;
    }

}


//=====================================================================
//
// 函数: 主从控制处理
// 描述: 更新主从刂剖�
//          1. 更新主机发送数据
//          2. 处理从机接收数据
// 
//=====================================================================
void MasterSlaveDeal(void)
{
    //static Uint16 p2pLooperTicker = 0; //
    Uint16 digit[5];
    
    
    MasterSlaveCtrl.MSCEnable = funcCode.code.MSCEnable;             // 主从控制是否有效
    
    if((CanInitFlag == CANL_BUS_STOP)&&(funcCode.code.MSCEnable))   // canlink总线停止
    {
        CanInitFlag = CANLINK_INIT_ENABLE;                          // 非主从控制时，可能出现地址冲突
                                                                    // 再次激活
        return;
    }
    else if (CanInitFlag != CANLINK_INIT_SUCC)                      // canlink未初始化成功
    {
        return;
    }
    else if(!funcCode.code.MSCEnable)                               // 主从无效
    {
        MscClrNodeSum();                                            // 清理节点信息
        dspSubCmd.bit.svcSlaveFlag = 0;                             
        return;
    }
    
    MasterSlaveCtrl.isMaster = (!funcCode.code.slaveEnable)&MasterSlaveCtrl.MSCEnable;  // 是否为主机
    MasterSlaveCtrl.mscCommErrTime = funcCode.code.MSCOverTime*100;  // 主从控制心跳时间 _*ms
    MasterSlaveCtrl.MasterSendPeriod = funcCode.code.MasterSendPeriod + 1;   // _ms
    MasterSlaveCtrl.masterAddress = funcCode.code.commSlaveAddress; // 主机地址
    
    GetNumberDigit1(digit, funcCode.code.salveCmdFollow);
    MasterSlaveCtrl.slaveFollowMasterCmd   = digit[0];              // 跟随命令与否
    MasterSlaveCtrl.slaveErrorSendToMaster = digit[1];              // 从机故障信息传输与否
    MasterSlaveCtrl.masterDispSlaveOut     = digit[2];              // 主机显示从机掉线与否
    // 为从机
    if (!MasterSlaveCtrl.isMaster)                                  
    {
        if(MasterSlaveCtrl.isMasterPre != MasterSlaveCtrl.isMaster)
        {
            MscClrNodeSum();                                        // 清理节点信息
            MasterSlaveCtrl.isMasterPre = MasterSlaveCtrl.isMaster; // 备份
            return;    
        }
        CANlinkDelConfig();                                         // 删除发送配置
        gCanlinkRecDataAddr[0] = P2P_MSRSTER__ADDR;                 // 配置接收传输
        gCanlinkBeatTime = MasterSlaveCtrl.mscCommErrTime;          // 配置心跳
        CANlinkRunMode = CANLINK_RUN_MODE;                          // RUN模式
    }
    // 为主机
    else 
    {
        if(MasterSlaveCtrl.isMasterPre != MasterSlaveCtrl.isMaster)
        {
            MscClrNodeSum();                                        // 清理节点信息
            MasterSlaveCtrl.isMasterPre = MasterSlaveCtrl.isMaster; // 备份
            return;  
        }
        CANlinkDelConfig();                                         // 删除接收配置
        MSCMasterCfg();                                             // 配置发送传输
        CANlinkRunMode = CANLINK_RUN_MODE;                          // RUN模式
        CheckSlaveNodeBeatStatus();                                 // 检查节点的个数，用于监控
        if((!MasterSlaveCtrl.slaveNodeNums)&&(runFlag.bit.run))     // 无从站运行报警
        {
            errorOther = ERROR_COMM;
            commErrInfo = COMM_ERROR_P2P_SLAVE_ERROR; 
        }
    }

    MasterSlaveCtrl.isMasterPre         = MasterSlaveCtrl.isMaster; // 备份
//    MasterSlaveCtrl.mscCommErrTimePre   = MasterSlaveCtrl.mscCommErrTime;
//    MasterSlaveCtrl.MasterSendPeriodPre = MasterSlaveCtrl.MasterSendPeriod;
//    MasterSlaveCtrl.masterAddressPre    = MasterSlaveCtrl.masterAddress;
    MasterSlaveCtrl.MSCEnablePre        = MasterSlaveCtrl.MSCEnable;
//    p2pLooperTicker = 0xA5A5;                                        // 上电锁定

    
    if (MasterSlaveCtrl.MSCEnable)                                   // 主从控制有效
    {
      
        if (MasterSlaveCtrl.isMaster)                                // 主机传送数据
        {
            dspSubCmd.bit.svcSlaveFlag = 0; // 主机清零
            
            //MasterSlaveCtrl.MasterFrq = frqRun * 10000 / maxFrq;     // -100.00%~100.00%
            switch(funcCode.code.slaveRevDataSel)
            {
                // 发送 转矩 + 运行频率
                case 0:
                    MasterSlaveCtrl.MasterFrq = frqDroop * 10000 / maxFrq;//下垂后的频率，防止主机下垂控制，视窗误入。
                    break;
                // 发送转矩 + 目标频率
                case 1:
                    MasterSlaveCtrl.MasterFrq = frqCurAim * 10000 / maxFrq;
                    break;
                default:
                    break;
            }
            MasterSlaveCtrl.MasterStatus = runFlag.bit.run;
            MasterSlaveCtrl.MasterTorque = (int32)((int16)itDispP2pSlave) * 10000 / 2000;  // -100.00%~100.00%
            MasterSlaveCtrl.MasterSendPeriod = funcCode.code.MasterSendPeriod + 1;   // _ms
            SLAVE_RCV_STATUS = MasterSlaveCtrl.MasterStatus;
            SLAVE_RCV_FRQ = MasterSlaveCtrl.MasterFrq;
            SLAVE_RCV_TORQUE = MasterSlaveCtrl.MasterTorque;
            SLAVE_RCV_DIR = funcCode.code.runDir;
            
        }
        else                                                         // 从机接收数据
        {  
            dspSubCmd.bit.svcSlaveFlag = 1; // 从机立牌
            MasterSlaveCtrl.SlaveRcvStatus = SLAVE_RCV_STATUS;
            MasterSlaveCtrl.SlaveRcvFrq = SLAVE_RCV_FRQ;       
            MasterSlaveCtrl.SlaveRcvTorque = SLAVE_RCV_TORQUE;  
            MasterSlaveCtrl.p2pRunDir = SLAVE_RCV_DIR;

            MasterSlaveCtrl.SlaveRcvTorque = ((int32)(int16)funcCode.code.slaveRevGain*(int16)MasterSlaveCtrl.SlaveRcvTorque / 100) + ((int16)funcCode.code.slaveRevOffset);
            LimitData((int16*)&MasterSlaveCtrl.SlaveRcvTorque, (int16)10000,(int16)-10000);

            MasterSlaveCtrl.SlaveRcvFrq = ((int32)(int16)funcCode.code.slaveRevGain*(int16)MasterSlaveCtrl.SlaveRcvFrq / 100) + ((int16)funcCode.code.slaveRevOffset);
            LimitData((int16*)&MasterSlaveCtrl.SlaveRcvFrq, (int16)10000, (int16)-10000);

      }
    }
    else
    {
        dspSubCmd.bit.svcSlaveFlag = 0; // 主从无效
    }
}


#else
void MasterSlaveDeal(void);
#endif


