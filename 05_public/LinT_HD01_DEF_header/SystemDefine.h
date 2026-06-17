/************************************************************
 28035版本定义系统中使用的宏和常量

************************************************************/
#ifndef MAIN_DEFINE_28035_H
#define MAIN_DEFINE_28035_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(DSP2803X)
//#include "DSP28x_Project.h"
#ifdef TARGET_GS32
#include "DSP_GS32_Device.h"
#else
#include "DSP2803x_Device.h"     // DSP2803x Headerfile Include File
#include "DSP2803x_Examples.h"   // DSP2803x Examples Include File
#endif
#endif

#include "MotorDefine.h"

#define STLEN  1
#define SOFT_INPUT_DETECT           0
#define HARD_WARE_INPUT_DETECT     1

/************************************************************
*************************************************************
    28035版本，�?�能软件版本号的维护
***********************************************************/
#define SOFT_VERSION        75      // 28035ver
// 0.02 update on xxxx
// 0.04 update on 2010�?4�?26 (自测第二版本)
// 0.06 update on 2010�?5�?17 (380驱动部分提交测试版本)
// 0.08 update on 2010, 6, 21 (380驱动第一轮测试完修改后的版本)
// 0.10 update on 2010, 6, 30 (第一轮验证程�?2)
// 0.16 update on 2010�?8�?25 (中试之后)
// 0.18 sizeD验证

// 0.20 update 2010�?11�?15 加入SVC 优化程序
// 0.24 update 2010�?12�?22 修改VF振荡抑制
                         //修改了测试问�?

// 非标记录
// 60001非标，将55kw/220V 改制机型对应�?132kw/380V (通用�?110kw/380V)
// 60002非标，将55kw/220V 改制机型对应�?132kw/380V (通用�?110kw/380V)
// 60003非标，将3.7kw/220V改制机型对应�?7.5kw/380V (通用�?5.5kw/380V) 2010/11/11


/************************************************************
************************************************************/

// // 定义DSP芯片和时�?
//#define    TMS320F2801        1                                   //2801芯片
//#define    TMS320F2802        2                                   //2802芯片
//#define      TMS320F2808      8                                   //2808芯片
#define      TMS320F28035   3                               //28035芯片

//#define        DSP_CLOCK100           100                             //100MHz时钟
//#define      DSP_CLOCK80        80                                  // 80MHz
#define    DSP_CLOCK60          60                           //60MHz时钟

#ifdef       DSP_CLOCK100
    #define     DSP_CLOCK           100                     //100MHz时钟
    #define     PWM_CLK_DIV         0
    #define     PWM_CLOCK           100                     //PWM模块时钟周期
    #define     DBTIME_1140V         50                      //1140V死区时间
    #define     DCTIME_1140V         14                      //1140V死区补偿
#endif
#ifdef      DSP_CLOCK80
    #define     DSP_CLOCK           80                      //80MHz时钟
    #define     PWM_CLK_DIV         0
    #define     PWM_CLOCK           80                      //PWM模块时钟周期
#endif
#ifdef      DSP_CLOCK60
    #define     DSP_CLOCK           60                      //60MHz时钟
    #define     PWM_CLK_DIV         0
    #define     PWM_CLOCK           60                      //PWM模块时钟周期
    #define     DBTIME_1140V         30                     //1140V死区时间
    #define     DCTIME_1140V         8                     //1140V死区补偿
#endif

#define     PWM_CLOCK_HALF      (PWM_CLOCK/2)               //PWM时钟除以2
#define C_TIME_01MS         (DSP_CLOCK * 1000000 / 10000)
#define C_TIME_045MS        (DSP_CLOCK * 1000000 / 2222)
#define C_TIME_05MS         (DSP_CLOCK * 1000000 / 2000)
#define C_TIME_1MS          (DSP_CLOCK * 1000000 / 1000)
#define C_TIME_2MS          (DSP_CLOCK * 1000000 / 500)
#define C_TIME_4MS          (DSP_CLOCK * 1000000 / 250)
#define C_TIME_10MS         (DSP_CLOCK * 1000000 / 100)
#define C_TIME_20MS         (DSP_CLOCK * 1000000 / 50)
#define C_TIME_50MS         (CPU_CLK * 1000000 / 20)


#define  READ_RAND_FLASH_WAITE      3                       //Flash中运行的等待时间
#define  READ_PAGE_FLASH_WAITE      3                       //
#define  READ_OTP_WAITE             8                       //OTP中读数据的等待时�?


/************************************************************
与芯片相关的寄存器定�?
************************************************************/
#if (SOFTSERIES == MD500SOFT)
    #define ADC_IU               (AdcaResultRegs.ADCRESULT1<<4)
    #define ADC_IW               (AdcaResultRegs.ADCRESULT2<<4)
    #define ADC_UDC              (AdcaResultRegs.ADCRESULT3<<4)
    #define ADC_IV               (AdcaResultRegs.ADCRESULT4<<4)
    #define ADC_IBREAK           (AdccResultRegs.ADCRESULT5<<4)
    #define ADC_AI1              (AdcaResultRegs.ADCRESULT6<<4)
    #define ADC_AI2              (AdccResultRegs.ADCRESULT7<<4)
    #define ADC_AI3              (AdcaResultRegs.ADCRESULT8<<4)
#if(AIRCOMPRESSOR == 0)
    #define UVW_PG_W             (AdcaResultRegs.ADCRESULT9<<4)          //PG-SinCos/UVW共用端子
    #define UVW_PG_V             (AdccResultRegs.ADCRESULT10<<4)         //PG-SinCos/UVW共用端子
#else
    #define ADC_AI4              (AdcaResultRegs.ADCRESULT9<<4)          //PG-SinCos/UVW共用端子
    #define ADC_AI5              (AdccResultRegs.ADCRESULT10<<4)         //PG-SinCos/UVW共用端子
#endif
    #define UVW_PG_U             (AdccResultRegs.ADCRESULT11<<4)         //PG-SinCos/UVW共用端子
    #define ADC_UU7              (AdccResultRegs.ADCRESULT12<<4)         //PG-SinCos用端�?
    #define PL_VOE_PROTECT       (AdccResultRegs.ADCRESULT13<<4)
    #define ADC_1250             (AdccResultRegs.ADCRESULT14<<4)
    #define ADC_TEMP             (AdccResultRegs.ADCRESULT15<<4)
#else
    #define ADC_GND              (AdccResultRegs.ADCRESULT0<<4)          //28035采样结果右对齐，为了�?2808兼容，左�?4�?
    #define ADC_IU               (AdcaResultRegs.ADCRESULT1<<4)
    #define ADC_IW               (AdcaResultRegs.ADCRESULT2<<4)
    #define ADC_UDC              (AdcaResultRegs.ADCRESULT3<<4)

    #define ADC_TEMP             (AdccResultRegs.ADCRESULT4<<4)
    #define ADC_IV               (AdccResultRegs.ADCRESULT5<<4)          // 这个地方是由于硬件驱动板到DSP版时接口反掉�?
    #define ADC_AI1              (AdccResultRegs.ADCRESULT6<<4)
    #define ADC_VREFLO           (AdcaResultRegs.ADCRESULT6<<4)          //选择ADCINB5与参考地连接，矫正零�?
    #define ADC_AI2              (AdcaResultRegs.ADCRESULT7<<4)

    #define ADC_AI3              (AdcaResultRegs.ADCRESULT8<<4)
    #define ADC_UU4              (AdcaResultRegs.ADCRESULT9<<4)          //PG-SinCos/UVW共用端子
    #define ADC_UU5              (AdccResultRegs.ADCRESULT10<<4)         //PG-SinCos/UVW共用端子
    #define ADC_UU6              (AdccResultRegs.ADCRESULT11<<4)         //PG-SinCos/UVW共用端子
    #define ADC_UU7              (AdccResultRegs.ADCRESULT12<<4)         //PG-SinCos用端�?
    #define PL_VOE_PROTECT       (AdccResultRegs.ADCRESULT13<<4)

    #define UVW_PG_U                (AdccResultRegs.ADCRESULT11<<4)         //PG-SinCos/UVW共用端子
    #define UVW_PG_V                (AdccResultRegs.ADCRESULT10<<4)         //PG-SinCos/UVW共用端子
    #define UVW_PG_W                (AdccResultRegs.ADCRESULT9 <<4)         //PG-SinCos/UVW共用端子
#endif    
//#define INVUPUDC_380V 52547 //对应820V/1022.7*65536
#define INVUPUDC_380V 55174 //对应820V*1.05/1022.7*65536 =861V

// // 与芯片相关的寄存器定义
//#define PIE_VECTTABLE_ADCINT    PieVectTable.ADCINT1                    //ADC中断向量
//#define ADC_CLEAR_INT_FLAG      AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1    //清除ADC模块的中断标志
//#define ADC_RESET_SEQUENCE      AdcRegs.SOCPRICTL.bit.RRPOINTER = 0x20  //reset the sequence
//#define ADC_START_CONVERSION    AdcRegs.ADCSOCFRC1.all = 0xFFFF         //软件启动AD
//#define ADC_END_CONVERSIN       AdcRegs.ADCINTFLG.bit.ADCINT1           //AD转换完成标志

#define  ADC_VOLTAGE_08         ( 8L*65535/33)  // AD输入0.8V对应的采样�??   28035AD范围�?0-3.3V
#define  ADC_VOLTAGE_10         (10L*65535/33)  // AD输入1.0V对应的采样�??
#define  ADC_VOLTAGE_20         (20L*65535/33)  // AD输入2.0V对应的采样�??
#define  ADC_VOLTAGE_25         (25L*65535/33)  // AD输入2.5V对应的采样�??

#define PL_INPUT_HIGH           (PL_VOE_PROTECT < ADC_VOLTAGE_08)       //输入缺相信号电平判断

#define PHASE_LOSE_CNT           20                             // 输出缺相累计计算的电流周期数
#define Encode_DisConnect       ((AdcResult.ADCRESULT9<<4) < ADC_VOLTAGE_20)             //编码器断线检�?

#define DisConnectRelay()       GpioDataRegs.GPASET.bit.GPIO11   = 1
#define ConnectRelay()          GpioDataRegs.GPACLEAR.bit.GPIO11 = 1    //上电缓冲继电器控�?

#define EnableDrive()           GpioDataRegs.GPACLEAR.bit.GPIO7  = 1
#define DisableDrive()      GpioDataRegs.GPASET.bit.GPIO7  = 1      //PWM输出允许控制
#define GetOverUdcFlag()      GpioDataRegs.AIODAT.bit.AIO2 == 0       // 28035通过io口做硬件过压

// //旋转变压器用到的宏定�?,IO的功能详见AD2S1205数据手册
#define RT_SAMPLE_START         (GpioDataRegs.AIOCLEAR.bit.AIO10 = 1)      // 旋变信号�?始采�?
#define RT_SAMPLE_END           (GpioDataRegs.AIOSET.bit.AIO10 = 1)        // 旋变信号采样完成

#define ROTOR_TRANS_RDVEL       GpioDataRegs.GPADAT.bit.GPIO20
#define ROTOR_TRANS_SO          GpioDataRegs.GPADAT.bit.GPIO21
#define ROTOR_TRANS_RD          GpioDataRegs.GPADAT.bit.GPIO23
#define ROTOR_TRANS_SCLK        GpioDataRegs.AIODAT.bit.AIO12
#define CURRENT_ADJUST      1  // 运行过程中AD零漂矫正，非MD500项目可以不处�?
#ifdef __cplusplus
}
#endif /* extern "C" */
#endif  // end of definition
