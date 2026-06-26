/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : f_canlink.c
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 08/25/2010
* Description        : CANlink驱动库
* 修改               : 2013/01/18   
*                                   心跳超时检测时间修改为心跳时间2.5倍
* 修改               : 2013/02/27 
                                    修改使用中断接收数据方式，2ms最大处理6帧
* 修改                 1012 20130301 V3.02 修改点对多数据帧在中断内过滤，防止点对多数据溢出                                    
********************************************************************************/
//#include "DSP28x_Project.h"     							// DSP2803x Headerfile Include File	
//#include "main.h"											// 包含头文件

#include "f_funcCode.h"
#include "f_dspcan.h"
#include "f_canlink.h"
#include "f_comm.h"
#include "f_plc.h"
#include "f_p2p.h"

#define DEBUG_F_CANLINK              1


#if DEBUG_F_CANLINK

// CANlink状态寄存器

Uint16 CANlinkRunMode;                                      // CANlink运行模式

Uint16 CanInitFlag;                                         // 初始化状态标志

// 同步帧触标志
Uint16 CANlinkSyncFlag;                                     // 同步广播标志 

// 定义CANlink接收缓存
struct CANLINK_REC_BUF	CanlinkRecBuf;						// 接收缓存 由接收邮箱数决定

// CANlink数据传输配置
CanLinkDataCfgType  gCANlinkDataCFg[CANLINK_CFG_LEN]       // CANlink数据传输配置
        = {
        {0x0,    0x0, 0x0, 0x0,   0x0}
/*        
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300}
*/
/*        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
        {0x3000, 0x4, 0x3, 0x100, 0x300},
*/        
                                                        };       // CANlink数据传输配置



Uint16 gSendEn[CANLINK_CFG_LEN];                            // 数据发送使能标识
Uint16 sDaTemp[CANLINK_CFG_LEN*4];                          // 事件触发数据历史
Uint16 gCanlinkRecDataAddr[CANLINK_RECDATA_LEN];            // CANlink接收点对多数据帧地址
Uint16      gtransInterval[CANLINK_CFG_LEN];                // 定时传输计时器
Uint16 gRequCount;                                          // 请求配置计时
Uint32 gCanlinkMoniBeat; 
Uint16 gCanlinkBeatCount;                                   // CANlink监测器心跳超时计时，心跳发送计时
Uint16 gCanlinkBeatTime, gCanlinkMoniAddr;                  // CANlink心跳时间，监测器地址
// 同步写寄存器保存区
struct CANLINK_SYNC_WREG gCanlinkSyncWriteBuf;              // CANlink同步写缓存

// 内部使用函数声明
void CanLlinkDataDeal(CANlinkDataBuf *dataPi);		        // CANlink接收数据处理
// 远程帧
void CanLinkRemoteDeal(CANlinkDataBuf *dataPi);             // CAN使用硬件远程帧
// 命令帧
void CanLinkCommDeal(CANlinkDataBuf *dataPi);		        // 命令帧处理
Uint16 CANlinkReadDeviceInfo(CANlinkDataBuf *dataPi);       // 读取设备电子标签
Uint16 SyncWriteAddrTest(Uint16 addr);                      // 同步写重复判断
void CANlinkSyncWriteReg(Uint16 addr, Uint16 data, Uint16 dWord);         // 同步写寄存器挂起
// 配置帧函数
void CanLinkConfigDeal(CANlinkDataBuf *dataPi);             // 配置帧处理
Uint16 CANlinkDataFrameConfig(CANlinkDataBuf *dataPi);      // 配置数据帧传输
void CANlinkDelConfig(void);                                // 删除配置
Uint16 CANlinkReadConfig(CANlinkDATA *data);                // 读配置信息
void CANlinkP2MConfig(CANlinkDataBuf *dataPi);              // 点对多数据帧接收
// 管理帧函数
void CanLinkManaDeal(CANlinkDataBuf *dataPi);               // CANlink管理帧 
void CANlinkStopUpNode(Uint16);                             // 启动/停止节点
void CANlinkAddrConflictStop(void);                         // 地址冲突停止
void CANlinkSyncWriteRegFun(void);                          // 同步写寄存器执行
void CANlinkSyncBroadFun(void);                             // 同步广播执行
void CANlinkRequestCfg(void);                               // CANlink请求配置
void CANlinkCloseFun(void);                                 // 关闭CANlink
void CANlinkAddrClash(void);                                // 地址冲突检测发送
// 数据帧相关
void CANlinkDataCfgRun(void);                               // CANlink配置运行
void CANlinkDataFrameFun(CANlinkDataBuf *dataPi);           // 数据帧处理
// 心跳帧相关
void CANlinkBeatFrameFun(void);                             // 接收心跳帧处理
void CANlinkBeatRun(void);                                  // 心跳运行
// 外部接口函数声明
void CanlinkOnline(void);                                   // CANlink 被连接在线
void CanlinkMoniTimeout(void);                                  // 监测器超时处理
void CanlinkSafeFun(void);                                   // 设备安全模式处理
void CanlinkAddrErrro(void);                                // 地址冲突处理
void CANlinkDeviceStat(void);                               // 设备状态度处理

void CANlinkReset(void);                                    // 复位CANlink
extern void MscMastBeatFrameFun(CANlinkDataBuf *dataPi);
static Uint16 sBaud = 0, sSourID = 0;						// 波特率与源地址							

/*******************************************************************************
* 函数名称          : void InitCanlinkTran(Uint16	addr)
* 入口参数			: 	本机ID
						自动应答数据
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: 初始化CANlink发送
********************************************************************************/
void InitCanlinkTran(Uint16	addr)
{
	CANlinkMsgID msgId;
	Uint16	i;
    DspCanDataStru intData;                                 // 初始化数据
    
	addr &= CANLINK_ID_MASK;

	// 		    源站     命令		问答	      扩展位()扩展帧
	msgId.all = 0xff | (0xfful<<16) | (1ul<<24)  | (1ul<<31);
//    msgId.bit.srcSta = 0xff;								// 源地址
    msgId.bit.destSta = addr;						        // 本机地址
//    msgId.bit.code = 0xff;									// 命令代码
//    msgId.bit.aq = 1;										// 问答标志
//    msgId.bit.framf = CAN_REMOTE_FRAME;				        // 帧类型标志，远程帧

    intData.msgid = msgId.all;
    
	for (i=0; i<TRAN_MBOX_NUM; i++)
	{
		InitTranMbox(TRAN_BOX_N-i, &intData);		        // 初始化发送邮箱
	}
}

/*******************************************************************************
* 函数名称          : Uint16 CanlinkDataTran(CANlinkDataBuf *dataPi, Uint16 len, Uint16 timeOut)
* 入口参数			: 	*dataPi CANlink数据缓存指针
*                       len     发送数据长度
						timeOut 超时周期
* 出口				：	CANLINK_RT_SUCC		数据发送成功
*						CANLINK_RT_BUSY		邮箱忙
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: CANlink帧发送
********************************************************************************/
Uint32 canTxSucc;
Uint16 CanlinkDataTran(CANlinkDataBuf *dataPi, Uint16 len, Uint16 timeOut)
{
	Uint16 stat, i;
    timeOut = timeOut<<0;
    
	for (i=0; i<TRAN_MBOX_NUM; i++)
	{
		stat = eCanDataTran(TRAN_BOX_N-i, len, (DspCanDataStru *)dataPi);
		if (CAN_MBOX_TRAN_SUCC == stat)
		{
            canTxSucc++;
			return (CANLINK_RT_SUCC);						// 发送成功		
		} 
	}

	return (CANLINK_RT_BUSY);							    // 发送邮箱忙
}

/*******************************************************************************
* 函数名称          : void InitCanlinkRec(Uint16 addr)
* 入口参数			: 本机ID
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: 初始化CANlink接收邮箱
********************************************************************************/
void InitCanlinkRec(Uint16 addr)
{
	union CANLINK_MsgID msgId;
	Uint32 lam;
	Uint16	i;
    
	addr &= CANLINK_ID_MASK;
	// 			  扩展位				
	msgId.all = (1ul<<31);

// 接收目标为本节点的所有帧
    msgId.bit.destSta = addr;								// 接收目标地址， 节点地址
	lam = (~(0xfful<<8)) & ( ~(7ul<<29));               	// 只能接收扩展帧 "0"滤波匹配
// 			目标地址	  只接收扩展帧		    			// 接收数据不分问、答帧，满足PLC卡数据帧接收
	for (i=0; i<REC_MBOX_NUM-3; i++)
	{
		InitRecMbox((REC_BOX_N-i) | 0x40, msgId.all, lam);	// 初始化接收邮箱
	}
// 接收广播帧
    msgId.bit.destSta = 0;                                  // 广播地址
    msgId.bit.framf = CANLINK_BEAT_FRAME;					// 仅接收广播心跳帧	
    lam = (~(0xful<<25)) & (~(0xfful<<8)) & ( ~(7ul<<29));
    InitRecMbox((REC_BOX_N-i) | 0x40, msgId.all, lam);	    // 接收主机心跳广播
    i++;
	lam = (~(0xfful<<8)) & ( ~(7ul<<29));				    // 
	InitRecMbox((REC_BOX_N-i) | 0x40, msgId.all, lam);	    // 接收命令类广播
    i++;
    
// 接收点对多数据
    msgId.bit.framf = CANLINK_DATA_FRAME;					// 数据帧
    msgId.bit.aq = CANlink_ASK;                             // 点对多数据
	lam = (~(0xful<<25)) & (~(0x1ul<<24)) & ( ~(7ul<<29));	// 只能接收扩展帧 "0"滤波匹配
	InitRecMbox((REC_BOX_N-i), msgId.all, lam);				// 最后接收邮箱允许覆盖	

}

/*******************************************************************************
* 函数名称          : Uint16 CanlinkDataRec(Uint32 msgid, Uint32 *dataPi, Uint16 timeOut)
* 入口参数			: 	消息ID
						发送数据
* 出口				：CANLINK_R_EMPTY	数据邮箱空
*					  CANLINK_R_OVER    接收成功，数据缓存有溢出
*					  CANLINK_RT_SUCC	接收成功
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: CANlink数据接收
********************************************************************************/
/*
Uint16 CanlinkDataRec(struct CANLINK_REC_BUF *dataPi)
{
	Uint16 i, stat=0;
    Uint16 index = 0;                                       // 缓存索引

	dataPi->bufFull = 0;
	for (i=0; i<(REC_MBOX_NUM); i++)
	{
		stat = eCanDataRec(REC_BOX_N-i, (DspCanDataStru *)(&(dataPi->buf[index])) );
		if (CAN_MBOX_EMPTY != stat)//(CAN_MBOX_REC_SUCC == stat) || (CAN_MBOX_REC_OVER == stat) )	// 接收到数据
        {
            dataPi->bufFull |= 1<<index;				    // 接收缓存有效
            index ++;
        }
	}
	if ( 0 == dataPi->bufFull)								// 未收数据 
		return (CANLINK_R_EMPTY);							// 接收邮箱空，返回
	if (CAN_MBOX_REC_OVER == stat)
		return (CANLINK_R_OVER);
	else
		return (CANLINK_RT_SUCC);
}
*/

/*******************************************************************************
* 函数名称          : Uint16 InitCanlink(Uint16 addr, Uint16 baud)
* 入口参数			: 	本机ID
*						波特率选择 
* 出口				：CAN_INIT_TIME	 初始化进行中
*					  CAN_INIT_SUCC  初始化成功
*					  CAN_INIT_TIMEOUT 初始化超时
*					  CAN_INIT_BAUD_ERR 波特率出错
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: 初始化CANlink驱动模块
********************************************************************************/
Uint16 InitCanlink(Uint16 addr, Uint16 baud)
{
	Uint16 stat;
	addr &= CANLINK_ID_MASK;								// 过滤参数
//	baud &= 0x7;
	
	stat = InitdspECan(baud);								// eCAN模块初始化
	if (stat != CAN_INIT_SUCC)
		return (stat);
		
	InitCanlinkTran(addr);							        // 初始化CANlink发送，
	InitCanlinkRec(addr);									// 接收	
	return (CAN_INIT_SUCC);
}

/*******************************************************************************
* 函数名称          : void CANlinkBeatRun(void)
* 入口参数			: 	
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 05/10/2012
* 说明				: CANlink心跳运行，安全模式与心跳时间为零，心跳不执行
********************************************************************************/
void CANlinkBeatRun(void)
{
    CANlinkDataBuf dataBuf;                                 // 数据发送缓存

    if ( (0 == gCanlinkBeatTime)                            // 心跳时间为"0" 安全模式, 不执行心跳
    || (CANLINK_SAFE_MODE == CANlinkRunMode) ) 
    {
        return;        
    }

// 监测器心跳超时检测    
    
    if (++gCanlinkMoniBeat > ((Uint32)gCanlinkBeatTime* CANLINK_MONI_TIMEOUT / CANlink_DEAL_PERIOD /10) )
    {                                                       // 监测器心跳超时检测
        CanlinkMoniTimeout();
        CanlinkSafeFun();                                   // 监测器超时处理，执行安全处理
        CANlinkReset();                                     // 复位CANlink模式
        return;                                             // 超时后不必再处理心跳
    }
// 节点心跳发送
    if (++gCanlinkBeatCount >= (gCanlinkBeatTime/CANlink_DEAL_PERIOD) )
    {                                                       // 节点心跳功能
        CANlinkDeviceStat();                                // 设备状态处理

        dataBuf.msgid.bit.srcSta = CAN_LINK_S_ADDR;         // 地址
        dataBuf.msgid.bit.destSta = gCanlinkMoniAddr;       // 监测器地址
#if DEBUG_F_MSC_CTRL
        if((funcCode.code.MSCEnable) && (!MasterSlaveCtrl.isMaster))
        {
            dataBuf.msgid.bit.srcSta = P2P_MSRSTER__ADDR+funcCode.code.commSlaveAddress;

            if(!MasterSlaveCtrl.slaveErrorSendToMaster) // 从机故障时，是否需主机故障
            {
                CanNodeRunStat &= 0xfffe;
            }
            dataBuf.msgid.bit.destSta = P2P_MSRSTER__ADDR;
        }
#endif

        
        dataBuf.msgid.bit.code = CANLINK_SLAVE_BEAT;        // 节点心跳码    
        dataBuf.msgid.bit.aq = CANlink_ASK;                 // 问
        dataBuf.msgid.bit.framf = CANLINK_BEAT_FRAME;       // 心跳帧
        dataBuf.data.dByte.dByte1 = CanNodeRunStat;         // 设备状态返回
        if (CANLINK_RT_SUCC == CanlinkDataTran(&dataBuf, 2, 1000))// 发送心跳帧
            gCanlinkBeatCount =0;                           // 成功清零定时
        else
            --gCanlinkBeatCount;                            // 失败延时一拍发送
    }
}

/*******************************************************************************
* 函数名称          : void CANlinkDataCfgRun(void)
* 入口参数			: 	
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 05/10/2012
* 说明				: CANlink数据帧传输配置运行处理
********************************************************************************/
void CANlinkDataCfgRun(void)
{
    CANlinkDataBuf dataBuf;                                 // 数据发送缓存
//    Uint16 data[4];
    Uint16 i, j;                                            // 内外循环数据
    Uint16 time, daTemp[4];                                 // 定时时间与数据缓存
    
    if (CANLINK_RUN_MODE != CANlinkRunMode)                 // 设备非运行模式，不执行数据帧
    {
        return;
    }
    
    for (i=0; i<CANLINK_CFG_LEN; i++)
    {                                                       // 
        if (0 == gCANlinkDataCFg[i].cfgAddr)                // 判断配置是否为空
            return;

        time = gCANlinkDataCFg[i].transInterval & 0x7fff;   
                                                            // 获取时间，错开定时时间起始时间
        if (0xffff == gCANlinkDataCFg[i].transInterval)
        {
            if (CANlinkSyncFlag == CANlink_SYNC_ENABLE)
                gSendEn[i] = 1;                             // 同步触发条件满足
        }
        else if (0 == (gCANlinkDataCFg[i].transInterval & 0x8000))
        {
            if (++gtransInterval[i]*CANlink_DEAL_PERIOD >= time)
            {
                gSendEn[i] = 1;                             // 时间触发
                gtransInterval[i] = 0;                      // 复位计时器
            }

        }
        else if (0x8000 == (gCANlinkDataCFg[i].transInterval & 0x8000))          
        {                                                   // 条件触发
            if (++gtransInterval[i]*CANlink_DEAL_PERIOD >= time)
            {
//                ReadSingleReg(gCANlinkDataCFg[i].srcRegAddr, &daTemp);
                ReadMultiReg(gCANlinkDataCFg[i].srcRegAddr, daTemp, gCANlinkDataCFg[i].regLen);
                for (j=0; j<gCANlinkDataCFg[i].regLen; j++)
                {
                    if (daTemp[j] != sDaTemp[(i<<2) +j])    // 等待改变，检测是否有数据改变
                    {   
                        gSendEn[i] = 2;                     // 条件&时间触发
                        gtransInterval[i] = 0;              // 复位计时器   
                        break;
                    }
                }
                if (0 == gSendEn[i])
                    gtransInterval[i]--;
            }
        }
        else
        {
            continue;                                       // 该条配置错误
        }
        
        if (gSendEn[i])                                     // 葜、送标志被使能
        {
            ReadMultiReg(gCANlinkDataCFg[i].srcRegAddr, daTemp, gCANlinkDataCFg[i].regLen);
            dataBuf.data.dByte.dByte1 = daTemp[0];            // 数据格式化
            dataBuf.data.dByte.dByte2 = daTemp[1];
            dataBuf.data.dByte.dByte3 = daTemp[2];
            dataBuf.data.dByte.dByte4 = daTemp[3];        
            
            dataBuf.msgid.bit.srcSta = gCANlinkDataCFg[i].disRegAddr;      
                                                            // 寄存器地址低字节
            dataBuf.msgid.bit.code = gCANlinkDataCFg[i].disRegAddr>>8;              
                                                            // 寄存器地址高字节
            if (CAN_LINK_S_ADDR == gCANlinkDataCFg[i].cfgAddr)
            {
                dataBuf.msgid.bit.aq = CANlink_ASK;         // 点对多数据帧
            }
            else
            {
                dataBuf.msgid.bit.aq = CANlink_ACK;         // 点对点
            }
#if DEBUG_F_MSC_CTRL
            if(funcCode.code.MSCEnable && MasterSlaveCtrl.isMaster)
            dataBuf.msgid.bit.aq = CANlink_ASK;             // 点对多数据帧
#endif
            dataBuf.msgid.bit.destSta = gCANlinkDataCFg[i].cfgAddr;           
                                                            // 目标节点地址，或对点多本站地址
            dataBuf.msgid.bit.framf = CANLINK_DATA_FRAME;   // 帧类型

            if (CANLINK_RT_SUCC == CanlinkDataTran(&dataBuf, gCANlinkDataCFg[i].regLen<<1, 1000))
            {                                               // 发送成功清零标志

                if (2 == gSendEn[i])                        // 条件触发成功发送保存数据
                {
                    for (j=0; j<gCANlinkDataCFg[i].regLen; j++)
                        sDaTemp[(i<<2) +j] = daTemp[j];
                }
                    
                gSendEn[i] = 0;                             // 发送标志清零
                gtransInterval[i] = 0;                      // 错开发送冲突
            }
        }
        
    }
}


/*******************************************************************************
* 函数名称          : void CANlinkDataFrameFun(CANlinkDataBuf *dataPi)
* 入口参数			: 	
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 05/10/2012
* 说明				: CANlink数据帧接收操作功能，接收处理CANlink数据帧
********************************************************************************/
void CANlinkDataFrameFun(CANlinkDataBuf *dataPi)
{
    Uint16 addr;//, i;
    Uint16 data[4];
    
    if (CANLINK_RUN_MODE != CANlinkRunMode)                 // 设备非运行模式，不执行数据帧
    {
        return;
    }
    /*                                                      // 过滤已经在中断内完成，该处无需调用
    if (CANlink_ASK == dataPi->msgid.bit.aq)                // 收到点对多数据
    {
        for (i=0; i<CANLINK_RECDATA_LEN; i++)               // 检测是否在接收列表
        {
            if (gCanlinkRecDataAddr[i] == dataPi->msgid.bit.destSta)
            {
                i = 0xcc;
                break;
            }
        }
    }
    else
    {
        i = 0xcc;
    }
    if (i != 0xcc)                                          // 数据点对多数据帧不在配置内
        return;
    */
    addr = dataPi->msgid.bit.code;                          // 合成地址
    addr <<= 8;
    addr += dataPi->msgid.bit.srcSta;
    
    data[0] = dataPi->data.dByte.dByte1;                    // 数据格式化
    data[1] = dataPi->data.dByte.dByte2;
    data[2] = dataPi->data.dByte.dByte3;
    data[3] = dataPi->data.dByte.dByte4;
                                                            // 执行数据帧接收处理
    WriteMultiReg(addr, data, (dataPi->len >> 1), 0);       // 仅保存到RAM

}

/*******************************************************************************
* 函数名称          : void CanlinkFun(void)
* 入口参数			: 
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: CANlink功能模块，使用2ms间隔调用操作
*					  需要功能码   站ID  1~63
*					  波特率设置	     1~2，设置0关闭CAN功能
********************************************************************************/

Uint32 CanDataDeal;
void CanlinkFun(void)
{
	Uint16 stat;
    Uint16 regCfgTime;
    CANlinkDataBuf rxDataBuf;                               // 改为中断接收方式，变量缓存数据
    Uint16 mboxNum;
	static Uint16 recCanCountTime;

#if DEBUG_F_PLC_CTRL
    static Uint16 timeOutErr = 0;
#endif

#if 0   //屏蔽该函数，否则在断线后计数器不准(变慢)，这样会导致断线检测报ERR-16时间变慢 modfied by yz1990 2014-08-06
	if (ErroCountReset())
        return;
#endif
	
#ifdef TARGET_GS32
        ErroCountReset();
#endif

#if DEBUG_F_PLC_CTRL
    if (funcCode.code.plcEnable)                            // 使用PLC卡
    {
		if ( (sBaud != CAN_BAUD_1M) || (sSourID != INV_PLC_ID) )
		{
			sBaud = CAN_BAUD_1M;
			sSourID = INV_PLC_ID;		
			CanInitFlag = CANLINK_INIT_ENABLE;			
		}
    }
    else
#endif
#if DEBUG_F_MSC_CTRL
    if(MasterSlaveCtrl.MSCEnable)
    {
        if(!funcCode.code.slaveEnable)
        {
            if ( (sBaud != CAN_LINK_BAUD_SEL) 
                || (sSourID != P2P_MSRSTER__ADDR)
                )
            {														// 使用CANlink协议	
                sBaud = CAN_LINK_BAUD_SEL;
                //sBaud = P2P_BAUD;
                sSourID = P2P_MSRSTER__ADDR;
                CanInitFlag = CANLINK_INIT_ENABLE;
            }
        }
        else
        {
            if ( (sBaud != CAN_LINK_BAUD_SEL) 
                || (sSourID != (P2P_MSRSTER__ADDR+funcCode.code.slaveEnable))
                )
            {														// 使用CANlink协议	
                sBaud = CAN_LINK_BAUD_SEL;
                //sBaud = P2P_BAUD;
                sSourID = P2P_MSRSTER__ADDR+funcCode.code.slaveEnable;
                CanInitFlag = CANLINK_INIT_ENABLE;
            }
        }
    }
    else
#endif
    if ( (sBaud != CAN_LINK_BAUD_SEL) || (sSourID != CAN_LINK_S_ADDR) )
    {														// 使用CANlink协议	
        sBaud = CAN_LINK_BAUD_SEL;
        sSourID = CAN_LINK_S_ADDR;
        CanInitFlag = CANLINK_INIT_ENABLE;
    }

	if (CanInitFlag == CANLINK_INIT_ENABLE)					// 初始化CANlink
	{
        if (CAN_INIT_SUCC != InitCanlink(sSourID, sBaud) )
			return ;
		else
        {
#if DEBUG_F_PLC_CTRL
        funcCode.code.u3[10] = 0;
        funcCode.code.u3[11] = 0;
#endif
			CanInitFlag = CANLINK_INIT_SUCC;                // 初始化成功
            CANlinkReset();                                 // 复位CANlink模式
#if DEBUG_F_MSC_CTRL
            if(!MasterSlaveCtrl.MSCEnable)
            CANlinkAddrClash();                             // 地址冲突检测
#endif
        }			
		return;		
	}

    if (CanInitFlag != CANLINK_INIT_SUCC)                   // 初始化不成功，CANlink模块不执行
        return;

    CANlinkSyncFlag = CANlink_SYNC_DISABLE;                 // 清除同步有效标志
    
//	stat = CanlinkDataRec(&CanlinkRecBuf);					// 接收CAN数据，使用中断方式后该函数不再使用

	if (CanMailBoxEmp() )                                   // 存在发送空邮箱	
	{														// 如接收数据成功
        mboxNum = 0;
		for (stat=0; stat<REC_MBOX_NUM; stat++)		        // 处理接收数据
		{
            DINT;                                           // 关            
			if ( CanlinkRecBuf.bufFull & (1u<<stat) )
			{
															// 拷贝数据
                rxDataBuf.msgid.all= CanlinkRecBuf.buf[stat].msgid.all;
                rxDataBuf.len = CanlinkRecBuf.buf[stat].len;
                rxDataBuf.data.qByte.qByte1= CanlinkRecBuf.buf[stat].data.qByte.qByte1;
                rxDataBuf.data.qByte.qByte2= CanlinkRecBuf.buf[stat].data.qByte.qByte2;
                                                            // 调用CANlink数据处理功能

                CanlinkRecBuf.bufFull &=  ~(1u<<stat);      // 清除标志
                
                EINT;                                       // 开中断
                CanDataDeal++;
				CanLlinkDataDeal((CANlinkDataBuf *)(&rxDataBuf));
				
//限制每2ms的周期处理REC_MAX个(分析7个较为合适)，确保2ms执行周期内把数据都处理完，  modfied by yz1990 2014-08-05
//这样会把每周期执行数据分配均匀(由于主站PLC和从站变频器时钟一般不会同步，可能导致一个2ms和下一个2ms数据不一样				
//(如第一个2ms 2条，下一个2ms就是4+6=10条))，所以确保在3个2ms周期内平均把数据处理完就一般不会丢帧了
				if (++mboxNum >= REC_MAX) 
					break;
			}
            else
                EINT;
		}

#if DEBUG_F_PLC_CTRL
        timeOutErr = 0;
        funcCode.code.u3[10] = 0;
#endif
	}
#if DEBUG_F_PLC_CTRL
    else                                                    // 未接到数据
    {

        if (++timeOutErr > 4)                               // 0~4计数，10ms
        {
            if (funcCode.code.u3[10] < 65535)
                funcCode.code.u3[10]++;                     // 超时计数
            timeOutErr = 0;
        }

        // 可编程功能有效时报错
        if ((funcCode.code.plcEnable)
            && (funcCode.code.u3[10] > 0)
            && ((curTime.powerOnTimeSec > 10) // 上电时间超过10秒才判断
            || (curTime.powerOnTimeM > 0))  
            )
        {
            errorOther = ERROR_COMM;
            commErrInfo = COMM_ERROR_PLC;
        }


    }
#endif
    // 离线状态每n秒请求配置
    if (CANLINK_SAFE_MODE == CANlinkRunMode)
    {
        regCfgTime = CANLINK_REQ_CFG_TIME;                         // canlink标准2000ms
#if DEBUG_F_MSC_CTRL 
        if (MasterSlaveCtrl.MSCEnable)
        {
            regCfgTime = 0;                                        // 主从控制不配置
        }
#endif
#if DEBUG_F_PLC_CTRL
        if(funcCode.code.plcEnable)
        {
            regCfgTime = 0;                                         // 运行Ｊ剑辉俜⑴渲?
        }
#endif
        if ((gRequCount++ >= (regCfgTime/CANlink_DEAL_PERIOD))&&(regCfgTime))      // 秒
        {
            CANlinkRequestCfg();
            gRequCount = 0;
        }

    }
    CANlinkBeatRun();                                       // CANlink心跳运行    
    CANlinkDataCfgRun();                                    // 配置运行处理，放在CANlink处理功能最后

#if 1      
    if (++recCanCountTime >=  (1000/2)) // 1s
   	{
        funcCode.group.u3[0] = recCanCout;
#ifdef TARGET_GS32
#else
		funcCode.group.u3[1] = ECanaRegs.CANTEC.bit.TEC; //发送错误计数器
		funcCode.group.u3[2] = ECanaRegs.CANREC.bit.REC; //接收错误计数器
#endif
		recCanCout = 0;
		recCanCountTime = 0;
	}
#endif
}

/*******************************************************************************
* 函数名称          : void CanLlinkDataDeal(struct CANLINK_DATA_BUF *dataPi)
* 入口参数			: 
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: CANlink协议接收数据处理
********************************************************************************/
void CanLlinkDataDeal(CANlinkDataBuf *dataPi)
{
	switch (dataPi->msgid.bit.framf)						// 帧类型处理
	{

        case CAN_REMOTE_FRAME:								// CANlink远程帧软件处理
			CanLinkRemoteDeal(dataPi);
			break;

        case CAN_MANAG_FRAME:								// CANlink网络管理帧
			CanLinkManaDeal(dataPi);
			break;
            
        case CAN_COMM_FRAME:								// 命令帧处理
			CanLinkCommDeal(dataPi);
			break;
            
		case CAN_CONFIG_FRAME:								// 配置帧处理
            CanLinkConfigDeal(dataPi);
			break;
            
		case CAN_PLC_DATA_FRAME:						    // PLC卡数据帧处理
#ifdef TARGET_GS32
#else
			PlcDataFramDeal(dataPi);
#endif
			break;
            
		case CANLINK_DATA_FRAME:						    // CANlink数据帧处理
			CANlinkDataFrameFun(dataPi);
			break;			

		case CANLINK_BEAT_FRAME:						    // CANlink心跳帧处理
			CANlinkBeatFrameFun();                          // 从机接收心跳帧处理
#if DEBUG_F_MSC_CTRL
            if(MasterSlaveCtrl.MSCEnable)
            MscMastBeatFrameFun(dataPi);                    // 主机接收心跳帧处理
#endif
			break;			

		default:

			break;				
			
	}		
}


/*******************************************************************************
* 函数名称          : void CanLinkRemoteDeal(struct CANLINK_DATA_BUF *dataPi)
* 入口参数			: 数据缓存指针
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 10/18/2010
* 说明				: CANlink远程帧软件处理
********************************************************************************/
void CanLinkRemoteDeal(CANlinkDataBuf *dataPi)
{
    Uint16 i;
#if DEBUG_F_PLC_CTRL
    if(funcCode.code.plcEnable)
    {
    	if (dataPi->msgid.bit.aq == CANlink_ACK)                // 收到应答帧不处理
            return;
    //	dataPi->msgid.bit.framf = CAN_REMOTE_FRAME;			    //
        dataPi->msgid.bit.destSta = sSourID;					// 本机地址
    //	dataPi->msgid.bit.srcSta = 0xff	;						// 
        dataPi->msgid.bit.aq = CANlink_ACK;						// 远程帧使用该位响应
    //	dataPi->msgid.bit.code = 0xff;							//
    //	dataPi->mdl.all= CanLinkChara[0];
    //	dataPi->mdh.all= CanLinkChara[1];
        
        funcCode.code.u3[10] = 0;
        funcCode.code.u3[11] = 0;
        
	    CanlinkDataTran(dataPi, 8, 1000);			            // 发送远程帧响应
        return;
    }
#endif
	if ( (dataPi->msgid.bit.aq == CANlink_ACK)              // 收到应答帧不处理
	|| (CANLINK_RUN_MODE == CANlinkRunMode) )               // 运行模式不响应远程帧
        return;
    if ( (4 != dataPi->len)                                  // 数据长度不对，监测器地址不对
        || (0 == dataPi->data.dByte.dByte2)
        || (dataPi->data.dByte.dByte1 > 20000)              // 心跳时间大于20'000ms
    )
        return;

#if DEBUG_F_MSC_CTRL
    if (MasterSlaveCtrl.MSCEnable)
        return;                                             // 主机不响应远程帧
#endif
    
    
    gCanlinkMoniAddr = dataPi->data.dByte.dByte2;           // 监测器地址
    gCanlinkBeatTime = dataPi->data.dByte.dByte1;           // 心跳时间
    gCanlinkMoniBeat = 0;
    gCanlinkBeatCount = 0;                                  // CANlink监测器心跳超时计时，心跳发送计时


    gCanlinkSyncWriteBuf.index = 0;                         // 同步写设置为无效
    gCanlinkSyncWriteBuf.dWord = 0;                         // 32位写清除
    
    for (i=0; i<CANLINK_CFG_LEN; i++)                       // 配置定时清零
    {  
        gtransInterval[i] = 0;                              // 清零计数与使能
        gSendEn[i] = 0;
    }
    
    CANlinkRunMode = CANLINK_CFG_MODE;                      // 进入配置模式
    CanlinkOnline();                                        // CANlink被连接
    
    dataPi->msgid.bit.destSta = gCanlinkMoniAddr;           //监测器地址
 	dataPi->msgid.bit.srcSta = sSourID	;					// 
    dataPi->msgid.bit.aq = CANlink_ACK;						// 远程帧使用该位响应
	dataPi->msgid.bit.code = CANLINK_CFG_LEN;				// 支持配置数
	
	dataPi->data.dByte.dByte1= gDeviceInformation[0];       // 获取主电子标签
    dataPi->data.dByte.dByte2= gDeviceInformation[1];
    dataPi->data.dByte.dByte3= gDeviceInformation[2];
    dataPi->data.dByte.dByte4= gDeviceInformation[3];
        
	CanlinkDataTran(dataPi, 8, 1000);			            // 发送远程帧响应

}

/*******************************************************************************
* 函数名称          : void CanLinkManaDeal(CANlinkDataBuf *dataPi)
* 入口参数			: 数据缓存指针
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: CANlink协议管理帧处理
********************************************************************************/
void CanLinkManaDeal(CANlinkDataBuf *dataPi)
{
#if DEBUG_F_MSC_CTRL
    if (funcCode.code.MSCEnable)  // 主从不响应任何远程远程管理帧，否需要则冲突之后需重新改参数
        return;                                             
#endif 
	if ( (dataPi->msgid.bit.aq == CANlink_ACK)                
	   && (CANLINK_ADDR_TEST == dataPi->msgid.bit.code))    // 地址冲突响应
    {
		CANlinkAddrConflictStop();
        return;            
    }
    if (dataPi->msgid.bit.aq == CANlink_ACK)                // 收到应答帧不处理
        return;
    
    if ( (CANLINK_SAFE_MODE == CANlinkRunMode)             // 安全模式不响应管理帧
    && (CANLINK_ADDR_TEST != dataPi->msgid.bit.code))       // 地址检测例外
        return;

#if 0//DEBUG_F_MSC_CTRL
    if ((MasterSlaveCtrl.isMaster)                          // 主机不响应远程帧
        && (CANLINK_ADDR_TEST != dataPi->msgid.bit.code)    // 地址检测例外
        && (CANLINK_REQ_CFG != dataPi->msgid.bit.code)      // 请求配置除外
        )
        return;                                             
#endif
    
	switch (dataPi->msgid.bit.code)							// 管理编码
	{
		case CANLINK_STARTUP_NODE:							// 启动节点
            CANlinkStopUpNode(1);
			break;
            
		case CANLINK_STOP_NODE:								// 停止节点
			CANlinkStopUpNode(0);
			break;
            
		case CANLINK_ADDR_TEST:								// 地址冲突检测，直接响应，无需任何处理

			break;
            
		case CANLINK_SYNC_BROAD:                            // 收到同步广播
		    CANlinkSyncBroadFun();                          // 同步广播处理
           break;
            
		case CANLINK_REQ_CFG:                               // 请求配置
            break;            
		case CANLINK_CLOSE:                                 // 关闭CANlink
            CANlinkCloseFun();
            break;            
		default:
            return;                                         // 代码无效不响应
            
	}
    if (0 == dataPi->msgid.bit.destSta)                     // CANlink广播帧不应答
        return; 
    
	dataPi->msgid.bit.destSta = dataPi->msgid.bit.srcSta;	// 写目标地址
	dataPi->msgid.bit.srcSta = sSourID;						// 写源地址
	dataPi->msgid.bit.aq = CANlink_ACK;						// 答标志
	
    CanlinkDataTran(dataPi, 0, 1000);			            // 发送响应帧，无数据区
    
}

/*******************************************************************************
* 函数名称          : void CANlinkBeatFrameFun(void)
* 入口参数			: 
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/11/2012
* 说明				: CANlink心跳接收处理
********************************************************************************/
void CANlinkBeatFrameFun(void)
{
    gCanlinkMoniBeat = 0;                                   // 监测心跳超时计数清零
}



/*******************************************************************************
* 函数名称          : void CanLinkConfigDeal(CANlinkDataBuf *dataPi)
* 入口参数			: 数据缓存指针
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/11/2012
* 说明				: CANlink协议配置帧处理
********************************************************************************/
void CanLinkConfigDeal(CANlinkDataBuf *dataPi)
{
	Uint16  len= 0;
    Uint16  err = CANLINK_NO_ERROR;

	if ( (dataPi->msgid.bit.aq == CANlink_ACK)              // 收到应答帧不处理
	|| ((0 == dataPi->msgid.bit.destSta) && (CANLINK_DEL_CFG != dataPi->msgid.bit.code))
	    )                                                   // 仅删除配置允许广播
        return;
    
    if (CANLINK_CFG_MODE != CANlinkRunMode)                 // 非配置模式不响应配置帧
        return;

    if (dataPi->msgid.bit.code >= CANLINK_DATA_REG)         // 数据传输配置
    {
        err = CANlinkDataFrameConfig(dataPi);
    }
    else
    {
    	switch (dataPi->msgid.bit.code)					    // 配置编码
    	{
    		case CANLINK_DEL_CFG:						    // 删除配置
                CANlinkDelConfig();
    			break;

    		case CANLINK_R_CFG:							    // 读配置
                err =  CANlinkReadConfig(&dataPi->data);
                len = 8;
    			break;
    		case CANLINK_REC_CFG:						    // 数据帧接收配置
                CANlinkP2MConfig(dataPi);
    			break;

    		default:
                err = CANLINK_CODE_ERROR;
    			break;				
    			
    	}
    }
    if (0 == dataPi->msgid.bit.destSta)                     // 广播不响应
        return;
    
	dataPi->msgid.bit.destSta = dataPi->msgid.bit.srcSta;	// 写目标地址
	dataPi->msgid.bit.srcSta = sSourID;						// 写源地址
	dataPi->msgid.bit.aq = CANlink_ACK;						// 答标志
	if (err)
    {
        dataPi->msgid.bit.code = CANLINK_CFG_ERROR;         // 配置出错
        dataPi->data.dByte.dByte1 = err;                    // 出错返回
        len = 2;
    }
    CanlinkDataTran(dataPi, len, 1000);                     // 配置帧响应
}

/*******************************************************************************
* 函数名称          : void CanLinkCommDeal(CANlinkDataBuf *dataPi)
* 入口参数			: 数据缓存指针
* 出口				：
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: CANlink协议命令帧处理
********************************************************************************/
void CanLinkCommDeal(CANlinkDataBuf *dataPi)
{
	Uint16 err = 0, len = 4;
	#if SERVO_DRIVER
			Uint16 qByte[2];
            static Uint16 dWord[2];                         // 用于同步写32位
            static Uint16 dWordEn;
	#endif
    
	if (dataPi->msgid.bit.aq == CANlink_ACK)                // 收到应答帧不处理
        return;
    if ( (dataPi->len >= 4) || (dataPi->len == 8)           // 数长度判断
        || ( (dataPi->msgid.bit.code == CAN_LINK_SYNC_W_EN) && (dataPi->len == 0))
    )
    {
    	switch (dataPi->msgid.bit.code)						// 命令编码
    	{
    		case CAN_LINK_R_REG:							// 读寄存器
                if (0 == dataPi->msgid.bit.destSta)         // 广播帧读取操作不执行
                    return;
    			err = ReadMultiReg(dataPi->data.dByte.dByte2, (Uint16*) (&dataPi->data.dByte.dByte1), 1 );
                if ((0 == err) & (dataPi->len == 8))        // 是否读第2个寄存器
                {
                    err = ReadMultiReg(dataPi->data.dByte.dByte4, (Uint16*) (&dataPi->data.dByte.dByte3), 1);
                    if (0 == err)
                        len = 8;
                }                
    			break;
    		case CAN_LINK_W_REG:							// 写寄存器
    		    err = WriteMultiRegTest(dataPi->data.dByte.dByte2, (Uint16*)(&dataPi->data.dByte.dByte1), 1, 1);
                if (err)
                    break;
                if (dataPi->len == 8)                       // 是否写第2个寄存器
                {
                    err = WriteMultiRegTest(dataPi->data.dByte.dByte4, (Uint16*)(&dataPi->data.dByte.dByte3), 1, 1);
                    if (err)
                        break;
                    WriteMultiRegFun(dataPi->data.dByte.dByte4, (Uint16*)(&dataPi->data.dByte.dByte3), 1, 1);
                }
    			WriteMultiRegFun(dataPi->data.dByte.dByte2, (Uint16*)(&dataPi->data.dByte.dByte1), 1, 1);
                len = 0;
    			break;
    		case CAN_LINK_R_INFO:						    // 读站点设备信息
                if (0 == dataPi->msgid.bit.destSta)         // 广播帧读取操作不执行
                    return;
                err = CANlinkReadDeviceInfo(dataPi);
                len = 8;
    			break;
/*                
		case CAN_LINK_R_WAR:							// 读告警信息

    			break;
*/
    // PLC卡CAN相关命令

    		case CAN_TRAN_TAB_CFG:								// PLC卡发送表配置
#ifdef TARGET_GS32
#else
    			err = InvTranTabCfg(dataPi);
#endif
    //			len = 4;
    			break;
    		case CAN_REC_TAB_CFG:								// PLC卡接收表配置
#ifdef TARGET_GS32
#else
    			err = InvRecTabCfg(dataPi);
#endif
    //			len = 4;
    			break;
    /*
    		case CAN_LINK_R_32Bit:								// 读32位寄存器

    			break;
    		case CAN_LINK_W_32Bit:								// 写32位寄存器

    			break;
    */
            case CAN_LINK_SYNC_W:                           // 同步写寄存器
		    err = WriteMultiRegTest(dataPi->data.dByte.dByte2, (Uint16*)(&dataPi->data.dByte.dByte1), 1, 1);
            if (err == 0)                                   // 无错误16位标准同步写
            {
                CANlinkSyncWriteReg(dataPi->data.dByte.dByte2, dataPi->data.dByte.dByte1, 0);
            }
#if SERVO_DRIVER
            else if (err == CANLINK_DATA32_ERROR)           // 32位写错误
            {
                if (dWordEn == 0)                           // 未记录32位写
                {
                    dWord[0] = dataPi->data.dByte.dByte2;   // 地址
                    dWord[1] = dataPi->data.dByte.dByte1;   // 保存数据
                    dWordEn = 1;                            // 记录半次32位操作
                    err = 0;
                }
                else
                {
                    if ((dataPi->data.dByte.dByte2 - dWord[0]) == 1 )
                    {
                        CANlinkSyncWriteReg(dWord[0], dWord[1], 1);
                                                            // 32位高在前，置标识
                        CANlinkSyncWriteReg(dataPi->data.dByte.dByte2, dataPi->data.dByte.dByte1, 0);
                        err = 0;
                        dWordEn = 0;
                    } 
                    else if ((dWord[0] - dataPi->data.dByte.dByte2) == 1)
                    {
                        CANlinkSyncWriteReg(dataPi->data.dByte.dByte2, dataPi->data.dByte.dByte1, 1);
                                                            // 32位高在前，置标识
                        CANlinkSyncWriteReg(dWord[0], dWord[1], 0);
                        err = 0;
                        dWordEn = 0;
                    }
                    else
                    {
                        err = CANLINK_ADDR_ERROR;           // 地址错误
                        dWordEn = 0;
                    }
                }
            }
#endif            
            else
                break;

            len = 0;
            
            break;

            case CAN_LINK_SYNC_W_EN:                        // 同步写寄存器生效
                CANlinkSyncWriteRegFun();
                break; 
                
    		default:
    			err = CANLINK_FUN_ERROR;					// 读写命令无效
    			break;				
    			
    	}
    }    
    else                                                    // CAN数据格式错误
        err = CANLINK_DATALEN_ERROR;
    
    if (0 == dataPi->msgid.bit.destSta)                     // CANlink广播帧不响应
        return; 
    
	dataPi->msgid.bit.destSta = dataPi->msgid.bit.srcSta;	// 写目标地址
	dataPi->msgid.bit.srcSta = sSourID;						// 写源地址
	dataPi->msgid.bit.aq = CANlink_ACK;						// 答标志

    if (err)
    {
        dataPi->msgid.bit.code = CAN_LINK_ERROR;            // 操作出错，命令码返回0xFF
        dataPi->data.dByte.dByte1 = err;
        len = 2;
    }
	
	CanlinkDataTran(dataPi, len, 1000);			            // 发送响应帧
}

/*******************************************************************************
* 函数名称          : Uint16 CANlinkReadDeviceInfo(CANlinkDataBuf *dataPi)
* 入口参数			: *dataPi CANlink数据结构
* 出口				：执行状态
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/27/2012
* 说明				: CANlink读取设备信息处理
********************************************************************************/
Uint16 CANlinkReadDeviceInfo(CANlinkDataBuf *dataPi)
{
    Uint16 index;
    index = dataPi->data.dByte.dByte1 - 1;
    
    if (index >= (CANLINK_DVC_INFO_LEN>>2) )
    {                                                       // 设备信息对像索引超过
        return CANLINK_ADDR_ERROR;                          // 地址出错
    }
    index <<= 2;
    dataPi->data.dByte.dByte1 = gDeviceInformation[index + 0];
    dataPi->data.dByte.dByte2 = gDeviceInformation[index + 1];
    dataPi->data.dByte.dByte3 = gDeviceInformation[index + 2];
    dataPi->data.dByte.dByte4 = gDeviceInformation[index + 3];
                                                            // 获取电子标签对象组
    return CANLINK_NO_ERROR;
}


/*******************************************************************************
* 函数名称          : Uint16 CANlinkDataFrameConfig(CANlinkDataBuf *dataPi)
* 入口参数			: *dataPi CANlink数据结构
* 出口				：执行状态
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/13/2012
* 说明				: CANlink数据帧传输配置命令处理
********************************************************************************/
Uint16 CANlinkDataFrameConfig(CANlinkDataBuf *dataPi)
{
    Uint16 index, err = CANLINK_NO_ERROR;
    Uint16 data[4];
    Uint16 i;
    
    index = dataPi->msgid.bit.code & 0x7f;
    if (index < CANLINK_CFG_LEN)                            // 小于支持的最大索引值
    {
        if ( (dataPi->data.byte.byte2 <= 4)                // 长度<=4、
            && (dataPi->data.byte.byte1 <= 245)            // 配置类型<=245
            && (dataPi->data.dByte.dByte4 > 0)              // 定时时间必须大于0
            && ( (dataPi->data.dByte.dByte4 <= 30000)      // 定时触发
            || ( (0x8000 == (dataPi->data.dByte.dByte4 & 0x8000)) && (dataPi->data.dByte.dByte4 <= (0x8000 + 30000)) )
                                                            // 条件触发
            || (dataPi->data.dByte.dByte4 <= 0xffff) )      // 同步触发
            && (8 == dataPi->len)                           // 数据长度必须是8
            )                                               
        {
            if ( ReadMultiReg(dataPi->data.dByte.dByte2, data, dataPi->data.byte.byte2) )
            {
                err = CANLINK_CFGDATA_ERROR;
            }
            else                                            // 初始化为与初始值不同
            {
                for (i=0; i<4; i++)
                    sDaTemp[(index<<2) + i] = data[i]-1;
            } 
/*            
            for (i=0; i<dataPi->data.byte.byte2; i++)       // 检查是否可被读取
            {
                if (ReadSingleReg(dataPi->data.dByte.dByte2+i, &data) )
                {
                    err = CANLINK_CFGDATA_ERROR;
                    break;
                }
                else if (0 == i)                            // 初始化为与初始值不同
                {
                    sDaTemp[i] = data-1;
                }                    
            }
*/            
            if (CANLINK_NO_ERROR == err)
            {
                gCANlinkDataCFg[index].cfgAddr = dataPi->data.byte.byte1;
                gCANlinkDataCFg[index].regLen = dataPi->data.byte.byte2;
                gCANlinkDataCFg[index].srcRegAddr = dataPi->data.dByte.dByte2;
                gCANlinkDataCFg[index].disRegAddr = dataPi->data.dByte.dByte3;
                gCANlinkDataCFg[index].transInterval = dataPi->data.dByte.dByte4;
                                                            // 以CANlink处理周期初始化
            }
		}
        else
        {
            if (8 == dataPi->len)                           // 非长度出错
                err = CANLINK_CFGDATA_ERROR;
            else
                err = CANLINK_CFGLEN_ERROR;
        }

    }
    else
    {
        err = CANLINK_INDEX_ERROR;                          // 超过配置索引范围
    }
    return err;
}

/*******************************************************************************
* 函数名称          : void CANlinkDelConfig(void)
* 入口参数			: 
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/13/2012
* 说明				: CANlink删除配置
********************************************************************************/
void CANlinkDelConfig(void)
{
    Uint16 i;
    for (i=0; i<CANLINK_CFG_LEN; i++)
    {
        gCANlinkDataCFg[i].cfgAddr = 0;                     // 配置地址/类型"0"表示配置为空

    }
    for (i=0; i<CANLINK_RECDATA_LEN; i++)
    {
        gCanlinkRecDataAddr[i] = 0;                         // 点对多接收地址"0"表示配置为空

    }
}

/*******************************************************************************
* 函数名称          : Uint16 CANlinkReadConfig(CANlinkDATA *data)
* 入口参数			: 
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/11/2012
* 说明				: CANlink读取配置信息
********************************************************************************/
Uint16 CANlinkReadConfig(CANlinkDATA *data)
{
    Uint16 err, index;
    index = data->dByte.dByte1;
    
    if (data->dByte.dByte1 < CANLINK_CFG_LEN)               // 在正常范围内
    {
        data->byte.byte1 = gCANlinkDataCFg[index].cfgAddr;
        data->byte.byte2 = gCANlinkDataCFg[index].regLen;        
        data->dByte.dByte2 = gCANlinkDataCFg[index].srcRegAddr;
        data->dByte.dByte3 = gCANlinkDataCFg[index].disRegAddr;
        data->dByte.dByte4 = gCANlinkDataCFg[index].transInterval;
        err = CANLINK_NO_ERROR;
    }
    else
    {
        err = CANLINK_INDEX_ERROR;                          // 配置索引出错
    }  
    return err;
}


/*******************************************************************************
* 函数名称          : Uint16 CANlinkP2MConfig(CANlinkDATA *data)
* 入口参数			: 帧数据结构
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/11/2012
* 说明				: CANlink点对多数据帧接收配置
********************************************************************************/
void CANlinkP2MConfig(CANlinkDataBuf *dataPi)
{
    #if CANLINK_RECDATA_LEN >0 
    gCanlinkRecDataAddr[0] = dataPi->data.byte.byte1;
    #endif
    #if CANLINK_RECDATA_LEN >1 
    gCanlinkRecDataAddr[1] = dataPi->data.byte.byte2;
    #endif
    #if CANLINK_RECDATA_LEN >2 
    gCanlinkRecDataAddr[2] = dataPi->data.byte.byte3;
    #endif
    #if CANLINK_RECDATA_LEN >3
    gCanlinkRecDataAddr[3] = dataPi->data.byte.byte4;
    #endif
    #if CANLINK_RECDATA_LEN >4 
    gCanlinkRecDataAddr[4] = dataPi->data.byte.byte5;
    #endif
    #if CANLINK_RECDATA_LEN >5 
    gCanlinkRecDataAddr[5] = dataPi->data.byte.byte6;
    #endif
    #if CANLINK_RECDATA_LEN >6
    gCanlinkRecDataAddr[6] = dataPi->data.byte.byte7;
    #endif
    #if CANLINK_RECDATA_LEN >7 
    gCanlinkRecDataAddr[7] = dataPi->data.byte.byte8;
    #endif
}


/*******************************************************************************
* 函数名称          : void CANlinkStartUpNode(void)
*                     void CANlinkStopNode(void)
* 入口参数			: "0"   停止节点
*                     "1"   启动节点
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/11/2012
* 说明				: CANlink管理帧启动/停止设备
********************************************************************************/
void CANlinkStopUpNode(Uint16 con)
{
    Uint16 i;
    if (con)
        CANlinkRunMode = CANLINK_RUN_MODE;                  // 启动节点
    else
        CANlinkRunMode = CANLINK_CFG_MODE;                  // 停止   
        
    for (i=0; i<CANLINK_CFG_LEN; i++)                       // 清零计数器
    {  
        gtransInterval[i] = 0;                              // 清零计数与使能
        gSendEn[i] = 0;
    }    
}

/*******************************************************************************
* 函数名称          : void CANlinkAddrConflictStop(void)
* 入口参数			: 
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/11/2012
* 说明				: CANlink停止一切活动停止
********************************************************************************/
void CANlinkAddrConflictStop(void)
{
    CanInitFlag = CANL_BUS_STOP;                            // CANlink总线停止，需修改参数激活生新初始化
    CANlinkReset();                                         // 复位CANlink模式
    DisableDspCAN();                                        // 禁止 CANlink使用的CAN控制器
    CanlinkAddrErrro();                                     // 地址错误触发外部事件
}

/*******************************************************************************
* 函数名称          : void CANlinkSyncBroadFun(void)
* 入口参数			: 
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/27/2012
* 说明				: CANlink接收到同步广播处理
********************************************************************************/
void CANlinkSyncBroadFun(void)
{
    gCanlinkMoniBeat = 0;                                   // 监测心跳超时计数清零
    CANlinkSyncFlag = CANlink_SYNC_ENABLE;                  // 同步广播有效，标识
}

/*******************************************************************************
* 函数名称          :void CANlinkRequestCfg(void)
* 入口参数			: 
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/27/2012
* 说明				: CANlink请求配置广播帧
********************************************************************************/
void CANlinkRequestCfg(void)
{
    CANlinkDataBuf data;

    data.msgid.bit.destSta = 0;                             // 广播地址
 	data.msgid.bit.srcSta = sSourID	;					    // 源地址
    data.msgid.bit.aq = CANlink_ASK;						// 问
	data.msgid.bit.code = CANLINK_REQ_CFG;				    // 请求配置帧
    data.msgid.bit.framf = CAN_MANAG_FRAME;

    CanlinkDataTran(&data, 0, 1000);			            // 发送帧
}

/*******************************************************************************
* 函数名称          : void CANlinkAddrClash(void)
* 入口参数			: 
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/27/2012
* 说明				: CANlink地址冲突检测
********************************************************************************/
void CANlinkAddrClash(void)
{
    CANlinkDataBuf data;

    data.msgid.bit.destSta = sSourID;                       // 本机地址
 	data.msgid.bit.srcSta = sSourID	;					    // 源地址
    data.msgid.bit.aq = CANlink_ASK;						// 问
	data.msgid.bit.code = CANLINK_ADDR_TEST;				// 地址冲突检测
    data.msgid.bit.framf = CAN_MANAG_FRAME;

    CanlinkDataTran(&data, 0, 1000);			            // 发送帧

}


/*******************************************************************************
* 函数名称          : void CANlinkCloseFun(void)
* 入口参数			: 
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/27/2012
* 说明				: CANlink关闭，使其回到初始化模式，安全状态
********************************************************************************/
void CANlinkCloseFun(void)
{
    CANlinkReset();                                         // 复位CANlink模式
    CanlinkSafeFun();                                       // 执行安全处理
}

/*******************************************************************************
* 函数名称          : void CANlinkSyncWriteRegFun(void)
* 入口参数			: 
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/11/2012
* 说明				: CANlink同步写寄存器操作，执行该命令将所有挂起同步写执行
********************************************************************************/
void CANlinkSyncWriteRegFun(void)
{
    Uint16 i; 

    for (i=0; i<gCanlinkSyncWriteBuf.index; i++)            // 挂起操作执行写
    {
        if (gCanlinkSyncWriteBuf.dWord & (1<<i) )
        {
            WriteMultiReg(gCanlinkSyncWriteBuf.addr[i], (Uint16*) (&gCanlinkSyncWriteBuf.data[i]), 2, 0);
            i++;                                            // 写两个16位
        }
        else
            WriteMultiReg(gCanlinkSyncWriteBuf.addr[i], (Uint16*) (&gCanlinkSyncWriteBuf.data[i]), 1, 0);
    }
    gCanlinkSyncWriteBuf.index = 0;                         // 设置为无效
    gCanlinkSyncWriteBuf.dWord = 0;                         // 32位写清除
}


/*******************************************************************************
* 函数名称          : void CANlinkSyncWriteReg(Uint16 addr, Uint16 data)
* 入口参数			: 
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/11/2012
* 说明				: CANlink同步写寄存器操作，将同步写操作记忆，超过命令数将覆盖最后一次
********************************************************************************/
void CANlinkSyncWriteReg(Uint16 addr, Uint16 data, Uint16 dWord)
{
    if (SyncWriteAddrTest(addr) == 0)                       // 地址存在重复
        return;
    
    if (gCanlinkSyncWriteBuf.index < CANlINK_SYNC_W_LEN)    // 小于最大值, ++
    {
        gCanlinkSyncWriteBuf.index++;
    }
    if (dWord)
        gCanlinkSyncWriteBuf.dWord |= 1 << (gCanlinkSyncWriteBuf.index -1);
                                                            // 32位写标识
        
    gCanlinkSyncWriteBuf.addr[gCanlinkSyncWriteBuf.index-1] = addr;
    gCanlinkSyncWriteBuf.data[gCanlinkSyncWriteBuf.index-1] = data;    
}

/*******************************************************************************
* 函数名称          : Uint16 SyncWriteAddrTest(Uint16 addr)
* 入口参数			: dWord 32位同步写标识
* 出口				：1 地址有效
*                     0 地址无效
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 06/11/2013
* 说明				: 同步写操作缓存地址判断
********************************************************************************/
Uint16 SyncWriteAddrTest(Uint16 addr)
{
    Uint16 i;

    for (i=0; i<gCanlinkSyncWriteBuf.index; i++)            // 存在地址重复不操作
    {
        if (addr == gCanlinkSyncWriteBuf.addr[i])
            return 0;
    }
    return 1;
}


/*******************************************************************************
* 函数名称          : void CANlinkResetReg(void)
* 入口参数			: 
* 出口				：无
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 08/02/2012
* 说明				: 复位CANlink
********************************************************************************/
void CANlinkReset(void)
{
    gRequCount = 0;                                         // 复位请求配置计数
    gCanlinkMoniBeat = 0;                                   // 清除心跳及超时检测计数器返回
    gCanlinkBeatCount = 0;
    ClrCanSendBuf();                                        // 清空发送缓存
#if DEBUG_F_MSC_CTRL
    if(!MasterSlaveCtrl.MSCEnable)
    CANlinkRunMode = CANLINK_SAFE_MODE;                     // 复位运行模式
#endif 
}


/*******************************************************************************
* 函数名称          : Uint16 P2bFilte(CANlinkMsgID *msgid)
* 入口参数			: 
* 出口				: 0xcc  
* 创建	            : 	
* 版本		        : V0.0.1
* 时间              : 08/25/2010
* 说明				: 中断内过滤点对多数据帧
********************************************************************************/
Uint16 P2bFilte(Uint32 msgid)
{
    Uint16 i;
    CANlinkMsgID id;

    id.all = msgid;

    #define CANLINK_DATA_ASK    ((1ul + (CANLINK_DATA_FRAME<<1))<<24)
    
    if ((msgid & (0x1ful <<24) ) == CANLINK_DATA_ASK)           // 收到点对多数据
    {
        for (i=0; i<CANLINK_RECDATA_LEN; i++)                   // 检测是否在接收列表
        {
            if (gCanlinkRecDataAddr[i] == id.bit.destSta)
            {
                i = 0xcc;
                break;
            }
        }    
    }
    else
         i = 0;
    
    return i;
}



//***********************以下函数非CANlink协议内容，由用于自定义************






#elif 1

void CanlinkFun(void){}

#endif










