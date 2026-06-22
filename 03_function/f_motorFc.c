//======================================================================
//
// 电机参数的更新
// 包括：
// 电机本身的参数,
// 控制参数
//
// Time-stamp: <2007-12-20 14:01:46  Shisheng.Zhi, 0354>
//
//======================================================================


#include "f_main.h"
#include "f_runSrc.h"
#include "f_frqSrc.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_invPara.h"


#if F_DEBUG_RAM
#define DEBUG_F_MANY_MOTOR     0    // 多个电机
#elif 1
#define DEBUG_F_MANY_MOTOR     1
#endif


Uint16 polePair;


#if DEBUG_F_MANY_MOTOR


//=====================================================================
//
// 更新第1/2电机，
// 更新与电机相关的电机参数、控制参数。
// 更新相关的参数
//
//=====================================================================

//Uint16 polePair;        // 极对数

extern const int16 decNumber[];
void UpdateMotorPara(void)
{
    Uint16 tmp; 
    
    // 电机序号
    if (diSelectFunc.f2.bit.motorSnDi)  
    {
        motorSn = (enum MOTOR_SN)diFunc.f2.bit.motorSnDi;   // 端子确定
    }
    else
    {
        motorSn = funcCode.code.motorSn;                    // 功能码确定
    }
    


    // 更新电机参数
    if (MOTOR_SN_1 == motorSn)  // 第1电机
    {
        memcpy(&motorFc.motorPara, &funcCode.code.motorParaM1, sizeof(struct MOTOR_PARA_STRUCT) * 2);  // 电机参数
        memcpy(&motorFc.pgPara,    &funcCode.code.pgParaM1, sizeof(struct PG_PARA_STRUCT) * 2);        // PG卡参数
        memcpy(&motorFc.vcPara, &funcCode.code.vcParaM1, sizeof(struct VC_PARA) * 2);                  // 矢量控制参数
        motorFc.motorCtrlMode = funcCode.code.motorCtrlMode;             // 电机控制方式
        motorFc.accDecTimeMotor = 0;                                     // 电机加减速时间(第1电机的加减速时间由端子改变)
        motorFc.torqueBoost = funcCode.code.torqueBoost;                 // 转矩提升               
//      motorFc.antiVibrateGainMode = funcCode.code.antiVibrateGainMode; // 抑制振荡增益模式
        motorFc.antiVibrateGain = funcCode.code.antiVibrateGain;         // 抑制振荡增益
    }
    else    // 第2电机
    {
		 memcpy(&motorFc, &funcCode.all[GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.motorType)], sizeof(struct MOTOR_FC));
    }

    // 强制VF运行
    if (errorCode == ERROR_ENCODER)   // 为编码器故障
    {
        motorFc.motorCtrlMode = FUNCCODE_motorCtrlMode_VF;
    }
#if F_MOTOR_TYPE_MMSM
    // 强制为闭环矢量
    else if (motorFc.motorPara.elem.motorType == MOTOR_TYPE_PMSM)  // 为同步机
    {
        //motorFc.motorCtrlMode = FUNCCODE_motorCtrlMode_FVC;
    }
#endif

#if 1
    // 计算极对数
    tmp = motorFc.motorPara.elem.ratingSpeed;
    polePair = (60UL * motorFc.motorPara.elem.ratingFrq / decNumber[funcCode.code.frqPoint] + tmp / 2) / tmp;
#endif    
}

#elif 1

void UpdateMotorPara(void)
{
    motorFc.motorCtrlMode = funcCode.code.motorCtrlMode;
}

#endif




