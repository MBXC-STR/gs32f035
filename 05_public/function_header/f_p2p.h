//======================================================================
//
// Time-stamp: <2011-2-21 Lei.Min, 0656>
//
// P2P comm
// 点对点通讯数据处理
//
//======================================================================

#ifndef __F_P2P_H__
#define __F_P2P_H__

#include "f_canlink.h"


#define MSC_RCV_TORQUE_SET      0   // 转矩给定
#define MSC_P2P_RCV_FRQ_SET     1   // 频率给定

#if DEBUG_F_MSC_CTRL

extern CanLinkDataCfgType  gCANlinkDataCFg[CANLINK_CFG_LEN];       // CANlink数据传输配置

extern Uint16 CANlinkRunMode;
extern Uint16 CanInitFlag;

#define MAX_SLAVE_NODE_NUM        5                    // 主从控制从站最大子节点数
// 固定主机从机地址
#define P2P_MSRSTER__ADDR         20       // 固定主机地址
#define P2P_BAUD                  6        // 固定波特率
// 配置负荷分配，需要修改以下宏
#define P2P_STATER_SRC_ADDR       0x730c   // 源首地址
#define P2P_STATER_DES_ADDR       0x730c   // 目标首地址
#define P2P_TRANSINTERVAL_TIME    5        // 触发时间 单位 ms 
#define P2P_REG_LENGHT            1        // 寄存器长度 最大4个
// 主机传递给从机数据存放地址
#define SLAVE_RCV_STATUS          funcCode.code.u3[12]  // 0x730c  主机状态
#define SLAVE_RCV_FRQ             funcCode.code.u3[13]  // 0x730d  主机频率
#define SLAVE_RCV_TORQUE          funcCode.code.u3[14]  // 0x730e  主机转矩
#define SLAVE_RCV_DIR             funcCode.code.u3[15]  // 0x730f  主机相序

typedef struct
{
    Uint16 MSCEnable;               // 主从控制是否有效
    Uint16 MSCEnablePre;            // 前一拍主从控制是否有效    
    Uint16 isMaster;                // 是否为主机
    Uint16 isMasterPre;             // 前一拍是否为主机
    // 主机数据
    Uint16 MasterStatus;            // 主机状态
    Uint16 MasterFrq;               // 主机频率
    Uint16 MasterTorque;            // 主机转矩
    // 从机接收数据
    Uint16 SlaveRcvStatus;          // 通讯接收数据
    Uint16 SlaveRcvFrq;             // 接收数据进行线性处理后
    Uint16 SlaveRcvTorque;          // 通讯发送周期
    // 发送接收出错判断计时
    Uint16 MasterSendPeriod;        // 主从控制发送计时
    Uint16 mscCommErrTime;          // 点对点通讯异常计时     _ms
    Uint16 MasterSendPeriodPre;     // 前一拍主从控制发送周期   *_ms
    Uint16 mscCommErrTimePre;       // 前一拍主从控制心跳时间   *_ms
    Uint16 slaveNodeIds[4];         // 主从控制从机地址列表
    Uint16 slaveNodeNums;           // 主从控制从机节点数
    // 主机地址
    Uint16 masterAddress;           // 主机地址
    Uint16 masterAddressPre;        // 前一拍主机地址
    Uint16 p2pRunDir;               // 主从方向
    // 主从机跟随、故障信息交互
    Uint16 slaveFollowMasterCmd;    // 从机跟随主机命令 0: no  1:yes
    Uint16 slaveErrorSendToMaster;  // 从机故障传主机   0: no  1:yes
    Uint16 masterDispSlaveOut;      // 主机提示从机掉线 0: no  1:yes

} MASTER_SALVE_CONTROL_STRUCT;

// 主从控制
extern MASTER_SALVE_CONTROL_STRUCT MasterSlaveCtrl;
void MasterSlaveDeal(void);

#elif 1
#endif
#endif



