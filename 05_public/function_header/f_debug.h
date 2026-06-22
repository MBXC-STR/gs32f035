#ifndef __F_DEBUG_H__
#define __F_DEBUG_H__

//=====================================================================
//MD290，MD380，MD500工程选择宏


#define DEBUG_MD290_SEARIS  0
#define DEBUG_MD500_SEARIS  0
#define DEBUG_MD380_SEARIS  0
#define DEBUG_MD380E_SEARIS 1
//=====================================================================

//=====================================================================
#if defined(DSP2803X)
#define DSP_2803X  1      // 28035
#elif 1
#define DSP_2803X  0
#endif
//=====================================================================
//=====================================================================
//#define FUNC_DEBUG_1140V    0x5a5a
#ifdef FUNC_DEBUG_1140V  // 1140V
#define F_1140V             1   
#elif 1
#define F_1140V             0   
#endif
//=====================================================================

//=====================================================================
#if defined(FUNC_DEBUG_RAM)         // 仅调试功能，在CCS的build option中定义的宏
#define F_DEBUG_RAM             1   // 功能在RAM中调试
#elif 1
#define F_DEBUG_RAM             0   // 不在RAM中调试
#endif
//=====================================================================
#define F_FRQ_UINT_ONLY_ONE     0   // 频率只有一个小数点
#define F_MOTOR_TYPE_MMSM       1

//=====================================================================
// 专机，MD380M/IS380
#if !defined(ZJ_MD380M)
#define DEBUG_F_POSITION_CTRL   0   // 不包含位置控制
#elif 1
#define DEBUG_F_POSITION_CTRL   1   // 包含位置控制
#endif
//=====================================================================

#define DEBUG_F_ABB_MORD_P2P      1

#define DEBUG_F_BACK_TO_OLD_CANOPEN  1
//=====================================================================
// 注意EEPROM的大小
//#define DEBUG_F_MOTOR_FUNCCODE    0   // 不预留功能码给性能调试
#define DEBUG_F_MOTOR_FUNCCODE    1   // 预留功能码给性能调试
//=====================================================================

#define DEBUG_F_PLC_CTRL          0//1    // 可编程卡
#define DEBUG_F_MSC_CTRL          0//1    // 主从控制

//#define DEBUG_F_MSC_CTRL_AUTOADDR          1    // 主从控制
//#if F_DEBUG_RAM && (!defined(_lint))
#if 1
#define LOCALF
#define LOCALD extern
#else
#define LOCALF static
#define LOCALD static
#endif
// LOCALD stands for local Declaration.
// LOCALF stands for local deFinition.


//=====================================================================
// DSP 频率: 60MHz/100MHz. 晶振是20MHz
#if (DSP_CLOCK == 100)
#define TIME_10US       1000UL
#define TIME_50US       5000UL
#define TIME_100US      10000UL // 0.1ms对应的定时器计数值（定时器1 10ns单位）
#define TIME_1MS        100000UL
#elif (DSP_CLOCK == 60)
#define TIME_10US       600UL
#define TIME_50US       3000UL
#define TIME_100US      6000UL  // 0.1ms对应的定时器计数值（定时器1 100/6ns单位）
#define TIME_1MS        60000UL
#endif
//=====================================================================

// dest的bit1 = source的bit2
#define BitAssign(source, bit1, dest, bit2)     \
{                                               \
    if ((source) & (0x1UL << (bit1)))           \
        (dest) |= (0x1UL << (bit2));            \
    else                                        \
        (dest) &= (~(0x1UL << (bit2)));         \
}


//void BitAssign(Uint32 source, Uint32 *dest, int16 bit);

Uint16 qsqrt(Uint32 data);


void UniversalDebug(Uint16 sn);  // 通用测试函数


#if F_DEBUG_RAM
void UniversalDebug32(Uint32 a);
void PowerOnDebug(Uint16 value);

extern Uint16 daValue[];
void InitDa(void);
void UpdateDaBuffer(Uint16 port);
void DaOut(Uint16 port);
#endif


#endif  // #ifndef __F_DEBUG_H__
