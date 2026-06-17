#ifndef __F_MAIN_H__
#define __F_MAIN_H__


#include "f_funcCode.h"
#include "f_error.h"
#include "f_common.h"
#include "f_canlink.h"

#if 1
#if DEBUG_MD500_SEARIS
#define PRODUCT_VERSION      50000       // 产品版本号
#elif DEBUG_MD290_SEARIS
#define PRODUCT_VERSION      29000       // 产品版本号
#else	
#define PRODUCT_VERSION      38000       // 产品版本号
#if F_1140V
#define PRODUCT_VERSION      38000+2     // 690_1140V产品版本号
#endif
#endif
#if DEBUG_MD500_SEARIS
// 定义通用软件版本信息
#define SOFTWARE_VERSION           1        // 通用软件版本
#define SOFTWARE_VERSION_TEMP      57         // 通用软件版本升级过程

// 定义非标软件版本信息
#define NON_STANDARD_VERSION       0         // 非标号
#define NON_STANDARD_VERSION_TEMP  0         // 非标的升级过程

// 定义临时版本软件信息
#define TEMP_SOFTWARE_NO           0        // 临时软件号
#define TEMP_SOFTWARE_VERSION      0         // 临时软件升级过程

#define SOFT_TYPE                  0         // 软件类型 0:通用  1:非标
#define IS_TEMP_SOFTWARE           0         // 是否为临时软件
#elif DEBUG_MD290_SEARIS
// 定义通用软件版本信息
#define SOFTWARE_VERSION           29        // 通用软件版本
#define SOFTWARE_VERSION_TEMP      0         // 通用软件版本升级过程

// 定义非标软件版本信息
#define NON_STANDARD_VERSION       0         // 非标号
#define NON_STANDARD_VERSION_TEMP  0         // 非标的升级过程

// 定义临时版本软件信息
#define TEMP_SOFTWARE_NO           0        // 临时软件号
#define TEMP_SOFTWARE_VERSION      0         // 临时软件升级过程

#define SOFT_TYPE                  0         // 软件类型 0:通用  1:非标
#define IS_TEMP_SOFTWARE           0         // 是否为临时软件
#else
// 定义通用软件版本信息
#define SOFTWARE_VERSION           1        // 通用软件版本
#define SOFTWARE_VERSION_TEMP      0         // 通用软件版本升级过程

// 定义非标软件版本信息
#define NON_STANDARD_VERSION       1         // 非标号
#define NON_STANDARD_VERSION_TEMP  0         // 非标的升级过程

// 定义临时版本软件信息
#define TEMP_SOFTWARE_NO           500        // 临时软件号
#define TEMP_SOFTWARE_VERSION      99         // 临时软件升级过程

#define SOFT_TYPE                  0         // 软件类型 0:通用  1:非标
#define IS_TEMP_SOFTWARE           0         // 是否为临时软件

#if F_1140V
// 定义通用软件版本信息
#define SOFTWARE_VERSION           13        // 通用软件版本
#define SOFTWARE_VERSION_TEMP      0         // 通用软件版本升级过程

// 定义非标软件版本信息
#define NON_STANDARD_VERSION       0         // 非标号
#define NON_STANDARD_VERSION_TEMP  0         // 非标的升级过程

// 定义临时版本软件信息
#define TEMP_SOFTWARE_NO           0        // 临时软件号
#define TEMP_SOFTWARE_VERSION      0         // 临时软件升级过程

#define SOFT_TYPE                  0         // 软件类型 0:通用  1:非标
#define IS_TEMP_SOFTWARE           0         // 是否为临时软件
#endif

#endif

#endif
#define ABS_INT16(a) (((a) >= 0) ? (a) : (-(a)))
#define ABS_INT32(a) (((a) >= 0L) ? (a) : (-(a)))
#define ABS_INT64(a) (((a) >= (int64)0) ? (a) : (-(a)))

#define  GetMax( x, y ) ( ((x) > (y)) ? (x) : (y) )
#define  GetMin( x, y ) ( ((x) < (y)) ? (x) : (y) )

#define DoNothing()                 // 空处理

//#define  GetTime() 	(CpuTimer1.RegsAddr->TIM.all)

#define MAX_UINT32  0xFFFFFFFF      // 32bit的无符号最大值
#define MAX_UINT16  0xFFFF          // 16bit的无符号最大值

// F2808的板子
#define OpenDrive()     (GpioDataRegs.GPACLEAR.bit.GPIO6 = 1)       // 开启PWM
#define CloseDrive()    (GpioDataRegs.GPASET  .bit.GPIO6 = 1)       // 关闭PWM


#define Nop()   asm(" nop")     // nop指令


// 程序中一些模块的调用周期
#define FUNC_DEAL_PERIOD        2       // 功能程序处理周期，_ms
#define RUN_CTRL_PERIOD         2       // 命令源程序 RunSrcDeal() 处理周期，_ms
//#define RUN_CTRL_PERIOD         0.5     // 命令源程序 RunSrcDeal() 处理周期，_ms
#define TORQUE_CTRL_PERIOD      2       // 转矩控制的周期
#define COMM_DEAL_PERIOD        2       // 通讯处理的周期
#define ERROR_DEAL_PERIOD       2       // 故障处理errorDeal()处理周期，_ms
#define FRQ_SRC_PERIOD          2       // 频率源程序frqSrc()处理周期，_ms
#define PULSE_IN_CALC_PERIOD    2       // 脉冲输入的计算周期
#define AI_CALC_PERIOD          2       // AI的计算周期
#define DI_CALC_PERIOD          2       // DI的计算周期
#define DO_CALC_PERIOD          2       // DO的计算周期
#define PID_CALC_PERIOD         2       // PID的计算周期
#define SERVO_CALC_PERIOD       RUN_CTRL_PERIOD       // 位置环的周期
#define VF_CALC_PERIOD          2       // VF分离的计算周期
#define CORE_FUNC_PERIOD        2       // 性能到功能数据更新周期
#define DISP_SCAN_PERIOD        2       // 键盘扫描周期
#define RUN_TIME_CAL_PERIOD     2       // 运行时间统计周期

enum POWER_ON_STATUS
{
    POWER_ON_WAIT,              // 等待上电准备OK。
    POWER_ON_CORE_OK,           // (性能)上电准备OK。母线电压建立完毕，上电对地短路检测完毕
    POWER_ON_FUNC_WAIT_OT       // 功能等待时间超时。功能的等待时间超过_时间，性能上电准备还没有完毕。
};
extern enum POWER_ON_STATUS powerOnStatus;




extern Uint16 limitedByOtherCodeIndex[];
extern Uint16 limitedByOtherCodeIndexNum;




#define FRQ_REF_I   0
#define FRQ_FDB_I   1
#define PC_REF_I    2
#define PC_FDB_I    3
extern Uint16 driveCoeff[2][4];


#define DECIMAL     0
#define HEX         1
Uint16 GetNumberDigit(Uint16 digit[5], Uint16 number, Uint16 mode);
void GetNumberDigit1(Uint16 digit[5], Uint16 number);
void GetNumberDigit2(Uint16 digit[5], Uint16 number);

extern struct MOTOR_FC motorFc;
extern enum MOTOR_SN motorSn;
extern Uint16 polePair;

extern Uint16 torqueCurrent;

extern Uint16 memoryValue;

#define AIAO_CHK_CHECK    0     // 检测是否需要进行AIAO校正初始化
#define AIAO_CHK_START    1     // 开始AIAO校正值读取
#define AIAO_CHK_END      2     // 结束AIAO校正
#define AIAO_CHK_WORD    0xA5   // AIAO校正标志字
#define AIAO_CHK_READ_NULL   0
#define AIAO_CHK_READ_OK     1
extern Uint16 aiaoChckReadOK;          // 读取的AIAO校正出厂值正常
extern Uint16 softVersionViewType;         // 显示类型
extern Uint16 softVersionVierPoint;        // 显示小数点
extern Uint16 coreSoftVersionViewType;         // 显示类型
extern Uint16 coreSoftVersionVierPoint;        // 显示小数点
void coreSoftVersionDeal(void);
extern Uint16 coreSoftWareVesion;                          // 高8位:通用软件，低8位:升级过程
extern Uint16 coreNonStandardVesion;                       // 高10位:非标号，低6位:非标升级过程
extern Uint16 coreTmpsoftNoAndVersion;                     // 高10位:临时软件号，低6位:临时升级过程
extern Uint16 coreSoftType;                                // 高8位:通用或非标，低8位:是否临时版本

#define VIEW_TYPE_STANDAR        0  // 显示v开头
#define VIEW_TYPE_NON_STANDAR    1  // 显示数字开头
#define VIEW_TYPE_TEMP           2  // 显示F开头



void InitForFunctionApp(void);          // 功能码初始化

void Main0msFunction(void);             // 功能处理函数, 0ms
void Main05msFunction(void);            // 功能处理函数, 0.5ms
void Main2msFunction(void);             // 功能处理函数, 2ms

extern int32 curABZPulseNum;
extern Uint16 motorRs;   // 电机运转圈数


#endif // __F_MAIN_H__








