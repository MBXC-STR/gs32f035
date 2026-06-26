/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : f_canlink.h
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 08/25/2010
* Description        : CANlink驱动库
					  邮箱31可用作自动应答邮箱
					  邮箱30~16可用作接收邮箱
					  15~0可用作发送邮箱


********************************************************************************/

#ifndef	__f_canlink__
#define	__f_canlink__


// CANlink执行周期
#define     CANlink_DEAL_PERIOD     2                       // CANlink处理周期 ms单位
// 配置相关定义
#define     CANLINK_CFG_LEN        16                       // 支持配置长度
#define     CANLINK_RECDATA_LEN     8                       // 接收点对多数据帧配置长度
#define     CANlINK_SYNC_W_LEN      8                       // 同步写长度

#define     CANLINK_DVC_INFO_LEN    12                      // 设备电子标签长度

// 监测超时倍数
#define     CANLINK_MONI_TIMEOUT    25                      // CANlink监测器超时倍率，1位小数
#define     CANLINK_REQ_CFG_TIME    2000                    // CANlink请求配置时间
#define     MSC_REQ_CFG_TIME        500                     // 主从控制从机请求配置时间

// CANlink运行模式
#define     CANLINK_SAFE_MODE       0                       // CANlink初始安全模式
#define     CANLINK_CFG_MODE        1                       // CANlink配置模式
#define     CANLINK_RUN_MODE        2                       // CANlink运行模式

// 初始化状态
#define     CANLINK_INIT_ENABLE      0                      // CANlink初始化准备中
#define     CANLINK_INIT_SUCC       0xcc                    // 初始化成功
#define     CANL_BUS_STOP           (CANLINK_INIT_SUCC-1)   // CANlink停止


// 帧类型定义
#define		CAN_REMOTE_FRAME		0xd						// 远程帧		1101
#define		CAN_MANAG_FRAME		    0x7						// 管理帧       0111
#define     CAN_COMM_FRAME          0x8                     // 命令控制帧	1000
#define		CAN_CONFIG_FRAME		0xA						// 配置帧		1010
#define		CAN_PLC_DATA_FRAME		0xB						// PLC卡使用数据帧		1011
#define     CANLINK_DATA_FRAME      0xC                     // CANlink数据帧 1100       
#define     CANLINK_BEAT_FRAME      0xE                     // CANlink心跳帧 1110

// 应答/数据传输标识
#define     CANlink_ASK             1                       // CAN问帧帧/点对多
#define     CANlink_ACK             0                       // 应答帧/点对点

// 收发状态标志
#define		CANLINK_RT_SUCC			0						// 数据收/发成功
#define		CANLINK_RT_BUSY			1						// 邮箱忙
#define		CANLINK_RT_TIMEOUT		2						// 超时
#define		CANLINK_R_EMPTY			3						// 未接收到数据
#define		CANLINK_R_OVER			4						// 接收数据有溢出

// ID号屏蔽标识
#define		CANLINK_ID_MASK			0x3f					// ID屏蔽标识

// 邮箱数定义
#define		TRAN_MBOX_NUM			5						// 发送邮箱数
#define		REC_MBOX_NUM			10						// 接收邮箱数
#define          REC_MAX            7                       //限制每2ms周期处理个数

// 功能邮箱号
#define		AAM_MBOX_N				31						// 自动应答邮箱号，自动应答邮箱接收优先级最高
#define		REC_BOX_N				30						// 接收邮箱号
#define		TRAN_BOX_N				15						// 发送邮箱号

// 管理帧编码
#define		CANLINK_STARTUP_NODE	1						// CANlink启动节点
#define		CANLINK_STOP_NODE		2						// 停止节点
#define		CANLINK_ADDR_TEST		3						// 地址检测
#define		CANLINK_SYNC_BROAD		4						// 同步广播
#define		CANLINK_REQ_CFG			5						// 请求配置
#define     CANLINK_CLOSE           6                       // 关闭CANlink

#define     CANlink_SYNC_ENABLE     0xcc                    // 同步广播有效
#define     CANlink_SYNC_DISABLE    0x0                     // 同步广播无效


// 配置帧编码
#define		CANLINK_DEL_CFG			1						// 删除CANlink配置信息
#define		CANLINK_ADD_CFG			2						// 增加设备CANlink配置信息
#define		CANLINK_R_CFG			3						// 读配置
#define		CANLINK_REC_CFG		    0x10					// 数据帧接收配置
#define		CANLINK_DATA_REG		0x80					// 数据帧传输配置 最高位"1"
#define     CANLINK_CFG_ERROR       0x7F                    // 配置帧出错
// 配置异常代码
#define		CANLINK_NO_ERROR		0						// 无错误
#define		CANLINK_CODE_ERROR		1						// 编码出错
#define		CANLINK_INDEX_ERROR		2						// 配置索引出错
#define		CANLINK_CFGDATA_ERROR	3						// 配置信息出错
#define     CANLINK_CFGLEN_ERROR    5                       // 配置信息长度出错

// 命令帧编码
#define		CAN_LINK_DEL			1						// 删除CANlinkK配置信息
#define		CAN_LINK_INC			2						// 增加设备CANlink配置信息
#define		CAN_LINK_R_CFG			3						// 读配置
#define		CAN_LINK_R_REG			4						// 读寄存器
#define		CAN_LINK_W_REG			5						// 写寄存器
#define		CAN_LINK_R_INFO			6						// 读站点设备信息
#define		CAN_LINK_R_WAR			7						// 读告警信息
#define     CAN_LINK_R_32Bit        0x14                    // 读32位寄存器
#define     CAN_LINK_W_32Bit        0x15                    // 写32位寄存器
#define     CAN_LINK_SYNC_W         0x21                    // CANlink同步写寄存器
#define     CAN_LINK_SYNC_W_EN      0x22                    // CANlink同步写生效

#define		CAN_LINK_W_EEP			0x0A					// 写EEPROM
#define     CAN_LINK_ERROR          0xff                    // 命令出错响应
// 命令异常代码
//#define		CANLINK_NO_ERROR		0						// 无错误
#define     CANLINK_FUN_ERROR       1                       // 命令功能不存在
#define		CANLINK_ADDR_ERROR		2						// 地址错误
#define		CANLINK_DATA_ERROR		3						// 数据错误
#define		CANLINK_FUN_DIS_ERROR	4						// 功能禁止错误
#define     CANLINK_DATALEN_ERROR   5                       // 命令数据无效

#define     CANLINK_DATA32_ERROR    9                       // 32位写错误

// 心跳帧代码
#define     CANLINK_MASTER_BEAT     1                       // 监测器心跳
#define     CANLINK_SLAVE_BEAT      2                       // 节点心跳

// PLC卡命令与CANlink无关
#define		CAN_TRAN_TAB_CFG		0x0B					// 发送表配置
#define		CAN_REC_TAB_CFG			0x0C					// 接收表配置
#define		CAN_FUN_U3_CFG			0x0D					// U3自定义功能码配置
#define		CAN_FUN_C0_CFG			0x0E					// C0自定义功能码配置
#define		CAN_READ_PLC_INFO		0x0F					// 读PLC卡设备信息


#define		CAN_LINK_S_WAR			0x10					// 发送告警信息
#define		CAN_LINK_Q_CFG			0x20					// 配置请求命令

#define     SERVO_DRIVER            0

// 功能码定义
#define		CAN_LINK_S_ADDR			(funcCode.code.commSlaveAddress & CANLINK_ID_MASK) // 本站地址
#define    CAN_LINK_BAUD_SEL          (funcCode.code.commBaudRate/1000)    // 波特率设置



// CANlink数据类型声明

struct CANlink_32DATA                                       // CANlink 32位数据引用方式
{
	Uint32 qByte1;
	Uint32 qByte2;	
};

struct CANlink_16DATA                                       // CANlink 16位数据引用方式
{
	Uint16 dByte2;
	Uint16 dByte1;
	Uint16 dByte4;
	Uint16 dByte3;	    
};

struct DSP_CAN_8DATA                                        // CANlink 8位数据引用方式
{
	Uint16 byte4:8;
	Uint16 byte3:8;
	Uint16 byte2:8;
	Uint16 byte1:8;	    
	Uint16 byte8:8;
	Uint16 byte7:8;
	Uint16 byte6:8;
	Uint16 byte5:8;
    
};

// CANlink 的数据结构定义
typedef union CAN_LINK_DATA 
{
    struct CANlink_32DATA qByte;
    struct CANlink_16DATA dByte;
    struct DSP_CAN_8DATA   byte;
} CANlinkDATA;

struct	CANLINK_MsgID_BITS	                        // 命令方式 数据方式
{
	Uint16	srcSta:8;									// 源站点ID/寄存器地址低字节
	Uint16	destSta:8;									// 目标站ID		                8
	Uint16	code:8;										// 命令代码/寄存器地址低字节	16
	Uint16	aq:1;										// 问答标志/数据传输类型		24
	Uint16	framf:4;									// 帧类型标识	25
//    	Uint16	aam:1;										// 自动应答位	29
//    	Uint16	ame:1;										// 屏蔽使能位	30
//    	Uint16	ide:1;										// 				31
};

// CANLINK消息ID位定义
typedef union CANLINK_MsgID
{
    Uint32 all;
    struct	CANLINK_MsgID_BITS bit;
} CANlinkMsgID;


// CANlinkK接收数据类型
typedef struct CANLINK_DATA_BUF
{
	CANlinkMsgID msgid;                                     // CAN帧ID
    CANlinkDATA data;                                       // CANlink缓存数据
    Uint16 len;
}CANlinkDataBuf;

// CANlink接收缓存数据结构定义
struct CANLINK_REC_BUF
{
	Uint16 bufFull;											// 缓存有效标识位，bit0 "1" buf[0]数据有效
	CANlinkDataBuf buf[REC_MBOX_NUM];
};

// CANlink数据传输配置结构体
typedef struct CANLINK_DATA_CFG                         // 注意数据引起的结构变化
{                                                          
	Uint16 srcRegAddr;                                      // 源寄存器首地址
	Uint16 regLen:8;                                        // 需传输寄存器长度
    Uint16 cfgAddr:8;       							    // 传输目标地址与类型
	Uint16 transInterval;                                   // 传输间隔 以ms为单位，0 表示事件触发
	Uint16 disRegAddr;                                      // 目标寄存器首地址
} CanLinkDataCfgType;


// 同步写寄存器数据类型
struct CANLINK_SYNC_WREG                                // 同步写寄存器数据结构
{                                                          
	Uint16 index;                                           // 索引 "0"表示无效 1~CANlINK_SYNC_W_LEN
	Uint16 dWord;                                           // 32位标识，bit位表示32位高16位
	Uint16 addr[CANlINK_SYNC_W_LEN];                        // 地址数组
    Uint16 data[CANlINK_SYNC_W_LEN];       					// 等待写入数据
};

// 外部使用变量
extern struct CANLINK_REC_BUF	CanlinkRecBuf;				// 接收缓存 由接收邮箱数决定

// 引用外部变量
extern Uint16 CanNodeRunStat;                               // 节点运行状态
extern Uint16 gDeviceInformation[CANLINK_DVC_INFO_LEN];     // 设备电子标签


// 外部使用函数声明
extern void CanlinkFun(void);
extern Uint16 WriteMultiRegTest(Uint16 addr, Uint16 *data, Uint16 len, Uint16 daTest);
extern void WriteMultiRegFun(Uint16 addr, Uint16 *data, Uint16 len, Uint16 eeprom);
extern Uint16 WriteMultiReg(Uint16 addr, Uint16 *data, Uint16 len, Uint16 eeprom);
extern Uint16 ReadMultiReg(Uint16 addr, Uint16 *dataPi, Uint16 len);
/*
extern Uint16 CanControlWriter(Uint16 addr, Uint16 data, Uint16 eeprom);
extern Uint16 WriteMultiReg(Uint16 addr, Uint16 *data, Uint16 len, Uint16 eeprom);
extern Uint16 ReadMultiReg(Uint16 addr, Uint16 *dataPi, Uint16 len);
extern Uint16 ReadSingleReg(Uint16 addr, Uint16 *data);
extern Uint16 WriteRegFunTest(Uint16 addr, Uint16 data, Uint16 daTest);
extern Uint16 WriteRegFun(Uint16 addr, Uint16 data, Uint16 eeprom);
*/
extern Uint16 WriteSingleReg(Uint16 addr, Uint16 data, Uint16 eeprom);
extern Uint16 CanControlRead(Uint16 addr, Uint16* result);
extern Uint16 CanlinkDataTran(CANlinkDataBuf *dataPi, Uint16 len, Uint16 timeOut);
extern Uint16 P2bFilte(Uint32 msgid);

#endif


