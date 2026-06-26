/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : f_dspcan.h
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 08/09/2010
* Description        : DSP CAN总线底层驱动库，包含文件

********************************************************************************/
#ifndef		__f_dspcan__
#define		__f_dspcan__

#include "mcan.h"                    // ★ 引入 MCAN 驱动库头文件
// 自动应答邮箱数据定义
//#define		

// CAN波特率宏定义
#define		CAN_BAUD_SUM							7		// CAN波特率总数
#define		CAN_BAUD_1M								6
#define		CAN_BAUD_500K							5
#define		CAN_BAUD_250K							4
#define		CAN_BAUD_125K							3
#define		CAN_BAUD_100K							2
#define		CAN_BAUD_50K							1
#define		CAN_BAUD_20K							0

#define		CAN_INIT_TIME							1		// 初始化进行中
#define		CAN_INIT_SUCC							2		// 初始化成功
#define		CAN_INIT_TIMEOUT						3		// 初始化超时，失败
#define		CAN_INIT_BAUD_ERR						4		// 波特率选择出错

// 收发状态标志
#define		CAN_MBOX_TRAN_SUCC						0		// CAN邮箱操作成功，不表示发送成功
#define		CAN_MBOX_NUM_ERROR						1		// CAN邮箱号出错
#define		CAN_MBOX_BUSY							2		// CAN邮箱忙
#define		CAN_MBOX_EMPTY							3		// CAN接收邮箱空，
#define		CAN_MBOX_REC_SUCC						4		// CAN邮箱接收数据成功
#define		CAN_MBOX_REC_OVER						5		// CAN邮箱接收有数据溢出

typedef struct 
{
	Uint16	BRPREG;
	Uint16	TSEG2REG;
	Uint16 	TSEG1REG;
}CAN_BAUD;



#define		ECANREGS			ECanaRegs					// 选择使用CAN接口		
#define		ECANMBOXES			ECanaMboxes					// 邮箱宏定义
#define		ECANLAMS			ECanaLAMRegs				// 验收滤波寄存器定义

// 内部使用数据类型定义


#define CAN_BASE          CANA_BASE           // MCAN 外设基地址
#define CAN_TX_PIN        GPIO_31_CANA_TX      // TX 引脚复用定义
#define CAN_RX_PIN        GPIO_30_CANA_RX      // RX 引脚复用定义
#define CAN_IRQ_LINE0     INT_CANA0           // 中断线 0
#define CAN_IRQ_LINE1     INT_CANA1           // 中断线 1
#define DSPCAN_CLK        40000               // CAN 时钟 40MHz (KHz)

extern volatile uint32_t mailbox_flag;    // bit 0:1 = transmit / receive
extern volatile uint32_t rec_mailbox_en;  // 接收邮箱使能位图
extern volatile uint32_t tran_mailbox_en; // 发送邮箱使能位图

// GS32 新增 — 用于解析 32-bit CAN ID 的位域结构
struct msg_bits_t {
    Uint16 srcSta:8;    // 源站点ID         bit 0~7
    Uint16 destSta:8;   // 目标站ID         bit 8~15
    Uint16 code:8;      // 命令代码          bit 16~23
    Uint16 aq:1;        // 问答标志          bit 24
    Uint16 framf:4;     // 帧类型标识        bit 25~28
    Uint16 aam:1;       // 自动应答位        bit 29
    Uint16 ame:1;       // 屏蔽使能位        bit 30
    Uint16 ide:1;       // 扩展帧标识        bit 31
};

union msg_bits_u {
    Uint32 all;
    struct msg_bits_t bit;
};
typedef union msg_bits_u msg_pack;

// 发送邮箱软件实例
struct transmit_mailbox_t {
    uint16_t mailbox_id;          // 逻辑邮箱编号 (对应 TI mbox 参数)
    uint32_t send_couter;         // 发送计数 (调试用, TI 无)
    uint32_t RP_ID;               // 远程过程 ID (预留)
    MCAN_TxMessage_t *inst;       // ★ 指向 MCAN 发送消息缓冲
};
typedef struct transmit_mailbox_t transmit_mailbox;// 接收邮箱软件实例
struct receive_mailbox_t {
    uint16_t mailbox_id;          // 逻辑邮箱编号
    uint16_t rec_flag;            // 接收标志
    uint32_t rec_id;              // 接收 ID
    MCAN_RxMessage_t *inst;       // ★ 指向 MCAN 接收消息缓冲
    uint8_t data[8];              // 数据缓冲
};
typedef struct receive_mailbox_t receive_mailbox;
#define TRAN_SIZE  5     // 发送邮箱数量 (对应 TRAN_MBOX_NUM)
#define REC_SIZE   10    // 接收邮箱数量 (对应 REC_MBOX_NUM)
extern receive_mailbox  rec_mail_inst [REC_SIZE];
extern transmit_mailbox tran_mail_inst[TRAN_SIZE];


// CAN数据
typedef struct DSP_CAN_DATA                                  // 数据引用联合体
{
    Uint32 mdl;
	Uint32 mdh;	
}CanDataType;

// DSP  CAN帧完整 数据结构
typedef struct DSP_CAN_TYPE
{
	Uint32  msgid;                                          // ID 29位  11位使用低11位
    CanDataType data;                                       // CAN帧数据
	Uint16 len;                                             // 缓存数据长度
}DspCanDataStru;

extern Uint32 recCanCout;
// 外部函数声明

#ifdef TARGET_GS32
#define ClrCanSendBuf()
#else
#define ClrCanSendBuf() ECANREGS.CANTRR.all	= 0xFFFFFFFF    // 取消正在进行的发送
#endif

extern Uint16 ErroCountReset(void);                         // CAN控制检测复位
extern void DisableDspCAN(void);                            // 禁止CAN控制器
extern Uint16 InitdspECan(Uint16 baud);						// DSP CAN模块初始化
extern void InitTranMbox(Uint16 mbox, DspCanDataStru *datapi);
															// 初始化发送邮箱
extern void InitRecMbox(Uint16 mbox, Uint32 msgid, Uint32 lam);
															// 初始化邮箱为接收邮箱
extern Uint16 eCanDataTran(Uint16 mbox, Uint16 len, DspCanDataStru *data);
															// 指定发送邮箱发送数据
extern Uint16 eCanDataRec(Uint16 mbox, DspCanDataStru *data);		// 指定邮箱数据读取
															

extern Uint16 CanMailBoxEmp(void);                          // 发送箱是否存在空闲












#endif


