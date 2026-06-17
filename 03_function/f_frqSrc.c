//======================================================================
//
// 频率源处理。 
//
// Time-stamp: <2008-10-24 15:06:59  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_frqSrc.h"
#include "f_main.h"
#include "f_runSrc.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_ui.h"
#include "f_p2p.h"
#include "f_comm.h"

int32 upDownFrq;
Uint16 upDownFrqInit;
int32 upDownFrqTmp;

int32 uPDownFrqMax;
int32 uPDownFrqMin;
int32 uPDownFrqLower1;
int32 uPDownFrqLower2;

#if F_DEBUG_RAM    // 仅调试功能，在CCS的build option中定义的宏

#define DEBUG_F_MULTI_SET_PLC       0   // 简易PLC，多段速
#define DEBUG_F_JUMP_FRQ            0   // 跳跃频率
#define DEBUG_F_FRQ_SRC_COMPOSE     0   // 频率源叠加

#elif 1

#define DEBUG_F_MULTI_SET_PLC       1
#define DEBUG_F_JUMP_FRQ            1
#define DEBUG_F_FRQ_SRC_COMPOSE     1

#endif


union FRQ_FLAG frqFlag;


#define PLC_END_LOOP_TIME_MAX   250 // 完成一个PLC循环，输出一个_ms的脉冲信号
Uint16 plcStep;         // PLC的阶段，[0, PLC_STEP_MAX]
Uint32 plcTime;         // 当前PLC阶段的运行时间，目前单位为10ms
Uint16 bPlcEndOneLoop;  // plc完成一个循环的标志，该标志保持(PLC_END_LOOP_TIME_MAX)ms
Uint16 plcStepRemOld;   // PLC记忆的step
Uint32 plcTimeRemOld;   // PLC记忆的time

int32 frq;              // 瞬时值, 功能传递给性能的运行频率
int32 frqTmp;           // 计算frq的临时变量
int32 frqFrac;          // 小数部分, Q15
int32 frqTmpFrac;       // 计算frq的临时变量的小数部分, Q15
int32 frqDroop;         // 下垂之后的速度，瞬时值, 功能传递给性能的运行频率
int32 frqCurAimFrac;

int32 frqAim;           // 设定(目标)频率
int32 frqAimTmp;        // 计算frqAim的临时变量, 下一次计算不会使用。跳跃频率之后，点动频率之前。
int32 frqAimTmp0;       // 计算frqAim的临时变量, 下一次计算会使用。频率源设定之后，跳跃频率之前。

int32 frqCurAim;        // 当前目标频率，注意在每次调用AccDecFrqCalc()之前要更新frqCurAim

Uint16 timeBench;       // 加减速时间基准
Uint16 frqSrcChgFlag;   // 频率源改变标志
Uint16 frqSrcNonXAddYFlag; // 非主加辅标志
Uint16 frqSrcChgPidInitFunc;        // PID初值功能码给定标志         
Uint16 frqSrcChgPidInitFrqtmp;      // PID初值为当前频率标志 
Uint16 frqSrcValue;
int32 frqDigitalTmp;    // 数字设定频率临时值


struct FRQ_XY frqXy;       

LOCALF int32 frqDigitalPlc;
LOCALF Uint16 presetFrqOld;    // 预置频率的old值

Uint16 upperFrq;        // 上限频率
Uint16 lowerFrq;        // 下限频率
Uint16 maxFrq;          // 最大频率
Uint16 benchFrq;        // 基准频率
int32 frqAiPu;          // AI,pulse的100%对应频率

Uint16 frqPuQ15;        // 频率转换时，Q15的1PU的频率，即maxFrq+20Hz

Uint16 frqCalcSrc;
Uint16 frqCalcSrcOld;
Uint16 plcStepOld;          // 完成一个PLC循环使用
Uint16 bStopPlc;            // PLC的所有段时间全为0，或者单次运行停机

Uint16 bFrqDigital;         // 数字设定频率标志，不包括UP/DOWN完成之后一段时间的显示处理时间
Uint16 frqKeyUpDownDelta;   // 使用面板UP/DOWN增减频率的delata

LOCALF Uint32 reverseTicker;// 正反转时间计时

Uint16 bAntiReverseRun;     // 处于禁止反转中。当功能码设置为禁止反转，且负频率或者反转运行命令，才为1.

LOCALF JUMP_FRQ jumpFrq1;   // 跳跃频率
LOCALF JUMP_FRQ jumpFrq2;   // 跳跃频率2

// 运行模式
Uint16 runMode; // 运行模式
int32 frqAimOld4Dir;

// 命令捆绑频率源
Uint16 runCmdBind;

LOCALD int32 FrqAimUpDownDeal(void);
LOCALD int32 FrqSrcOperate(void);
LOCALD int32 FrqXyCalc(Uint16 src);
LOCALD int32 FrqPlcSetDeal(void);
LOCALD void JumpFrqDeal(void);
LOCALD void JumpFrqEndCalc(JUMP_FRQ *p);
LOCALD void UpdateLimitFrq(void);
LOCALD void UpdateRunMode(void);


int32 GetFrqX(void);
int32 GetFrqY(void);
int32 GetFrqComp(void);
int32 GetFrq(void);

Uint16 frqPlcType;
//=====================================================================
// 
// 根据频率源(280F的功能码是F0-01)，计算设定频率frqAimTmp0
//
// 由于运行方向、点动命令是在命令源处理的，
// 所以运行方向、点动以及之后的防反转选择、运行方向相反也放在命令源中处理。
//
//=====================================================================
void FrqSrcDeal(void)
{
    static int32 frqAimTmpBak;

// 上下限频率计算
    UpdateLimitFrq();

// 更新运行模式
    UpdateRunMode();

    runFlag.bit.plc = 0;
    runFlag.bit.pid = 0;
    runFlag.bit.torque = 0;

    frqPlcType = 0;
    
    if (RUN_MODE_TORQUE_CTRL == runMode)
    {
        runFlag.bit.torque = 1;
    }

    frqCalcSrc = funcCode.code.frqCalcSrc;
    
    if (frqCalcSrcOld != frqCalcSrc)    // frqCalcSrc改变了
    {
        frqCurAimFrac = 0;  //-= 小数点改变
    }

    if (presetFrqOld != funcCode.code.presetFrq)   // 若预置频率改变了(F0-03,恢复出厂参数,通讯修改等)，更新设定频率
    {
        presetFrqOld = funcCode.code.presetFrq;
    }

// 频率源计算
    if ((!runFlag.bit.jog)          // 点动
// 故障时更新频率源
//        && (!errorCode)             // 故障码
        && (RUN_MODE_TORQUE_CTRL != runMode)    // 没有转矩控制
        )
    {
        frqAimTmp0 = FrqSrcOperate();
    }
    else
    {
        accDecFrqPrcFlag = ACC_DEC_FRQ_NONE;      // 键盘UP/DOWN修改频率标志，清零
    }

    // PLC状态复位
    if (diFunc.f1.bit.resetPLC)
    {
        plcStep = 0;
        plcTime = 0;
    }

    frqCalcSrcOld = frqCalcSrc;     // 更新frqCalcSrcOld
    plcStepOld = plcStep;   // 更新plcStepOld
// 判断是否超过上限频率
    if (frqAimTmp0 > upperFrq)
    {
        frqAimTmp0 = upperFrq;
    }
    else if (frqAimTmp0 < -(int32)upperFrq)
    {
        frqAimTmp0 = -(int32)upperFrq;
    }

#if 0
// 转矩控制时判断是否超过转矩控制最大频率
    if (RUN_MODE_TORQUE_CTRL == runMode)    // 非转矩控制
    {
        // 正向最大频率
        if (frqAimTmp0 >= 0)
        {
            if (frqAimTmp0 > funcCode.code.torqueCtrlFwdMaxFrq)
            {
                frqAimTmp0 = funcCode.code.torqueCtrlFwdMaxFrq;
            }
        }
        // 反向最大频率
        else
        {
            if (frqAimTmp0 < -(int32)funcCode.code.torqueCtrlRevMaxFrq)
            {
                (frqAimTmp0 = -(int32)funcCode.code.torqueCtrlRevMaxFrq);
            }
        }
    }
    
#endif

     // 频率设定起效端子
    if (diSelectFunc.f2.bit.frqOk && (!diFunc.f2.bit.frqOk))
    {    
        frqAimTmp0 = frqAimTmpBak;
    }
    else
    {
        frqAimTmpBak = frqAimTmp0;
    }
        
// 赋值给frqAimTmp
    frqAimTmp = frqAimTmp0;

// 设定频率显示值
    frqAimDisp = ABS_INT32(frqAimTmp);   // 跳跃频率之前，点动之后的值

// 跳跃频率处理，点动频率不受跳跃频率影响。
// 跳跃频率计算之前，就与启动频率比较。
// 否则，若由于跳跃频率的low低于启动频率，不会启动。
// 跳跃频率计算之后，再和下限频率比较，是否启动。
// 否则，若设定频率高于下限频率，会启动，又由于跳跃频率的low低于下限频率，会立即进入停机过程。
    JumpFrqDeal();
}


//=====================================================================
// 
// 1. 根据运行方向,运行方式(点动还是普通运行)，跳跃频率
// 计算设定频率(目标频率frqAim)
// 2. 更性设定频率和运行频率的方向，判断当前是否正在反向(是否在正反转死区内)。
//
// 由于运行方向、点动命令是在命令源处理的，
// 所以该函数应该在命令源中调用。
// 
//=====================================================================
void UpdateFrqAim(void)
{
    int32 frq4Dir;           // 瞬时频率, 判断方向使用

// 赋值给frqAimTmp
    //frqAimTmp = frqAimTmp0;  // FrqSrcDeal为2ms而UpdateFrqAim为0.5毫秒,可能导致以下操作取反多次执行
    //JumpFrqDeal();
    


// 点动。点动方向与runCmd.bit.dir无关。
    if (runFlag.bit.jog)            // 真正有效的点动命令
    {                               // 点动命令取消之后也可能进入该段代码
        if (RUN_CMD_FWD_JOG == runCmd.bit.jog)  // 点动方向，独立于普通运行方向
        {
            frqAimTmp = funcCode.code.jogFrq;
        }
        else if (RUN_CMD_REV_JOG == runCmd.bit.jog) //
        {
            frqAimTmp = -(int16)funcCode.code.jogFrq;
        }

        frqAimDisp = funcCode.code.jogFrq; // 点动时，设定频率显示为点动频率
    }

#if DEBUG_F_PLC_CTRL
    frqAimPLCDisp = (int16)(frqAimTmp*10000/maxFrq);
#endif
    
// 点动频率不能超过上限频率
// 判断是否超过上限频率

    frqFlag.bit.frqSetLimit = 0;
    if (frqAimTmp >= upperFrq)
    {
        frqAimTmp = upperFrq;
        frqFlag.bit.frqSetLimit = 1;
    }
    else if (frqAimTmp <= -(int32)upperFrq)
    {
        frqAimTmp = -(int32)upperFrq;
        frqFlag.bit.frqSetLimit = 1;
    }

// 赋值给frqAim
    frqAim = frqAimTmp;



// 反转禁止
    bAntiReverseRun = 0;
    if ((funcCode.code.antiReverseRun)||(diFunc.f2.bit.forbidRev))
    {
        if ((frqAimTmp < 0) || (frqAim < 0))
        {
            frqAim = 0;      // 反转禁止，输入负频率时，以0频运行。
            frqAimDisp = 0;
            frqFlag.bit.frqSetLimit = 1;
            bAntiReverseRun = 1;
        }
    }

// 运行方向
    if ((FORWARD_DIR != runCmd.bit.dir)
        &&(!runFlag.bit.jog))
    {
        frqAim = -frqAim;
    }
// 判断设定频率的方向
    if (frqAim > 0)
    {
        runFlag.bit.dir = FORWARD_DIR;
    }
    else if (frqAim < 0)
    {
        runFlag.bit.dir = REVERSE_DIR;
    }
    else
    {
        runFlag.bit.dir = runCmd.bit.dir;
    }
// 运行方向相反
    frq4Dir = frq;
    //frq4Dir = frqRun;  // 正反转死区使用反馈的频率
    runFlag.bit.dirFinal = runFlag.bit.dir;
    if (FUNCCODE_runDir_REVERSE == funcCode.code.runDir)
    {
        frqAim = -frqAim;
        frq4Dir = -frq4Dir;

        runFlag.bit.dirFinal = ~runFlag.bit.dirFinal;
    }

// 判断运行频率的方向
    if (frq4Dir > 0)
    {
        runFlag.bit.curDir = FORWARD_DIR;
    }
    else if (frq4Dir < 0)
    {
        runFlag.bit.curDir = REVERSE_DIR;
    }
    else    // 正反转死区
    {
        if (runFlag.bit.run
            && ((int64)frqAimOld4Dir * frqAim < 0)    // 负数与0之间，不能进入
            )
        {
            if (++reverseTicker >= (Uint32)funcCode.code.zeroSpeedDeadTime
                * (Uint16)(TIME_UNIT_ZERO_SPEED_DEAD / RUN_CTRL_PERIOD))
            {
                runFlag.bit.curDir = runFlag.bit.dir;
                
                //if (frqAim)
                    frqAimOld4Dir = frqAim;
            }
        }
        else
        {
            runFlag.bit.curDir = runFlag.bit.dir;

            //if (frqAim)      // 设定频率为0时，不更新frqAimOld4Dir
            {
                frqAimOld4Dir = frqAim;
            }
        }
    }

// 判断是否正在反向
    if ((runFlag.bit.curDir != runFlag.bit.dir) // 正在反向
//        && (funcCode.code.zeroSpeedDeadTime)    // 没有 死区时间，frqCurAim直接赋值为设定频率，不在中间赋值为0.考虑s曲线
        )
    {
        runFlag.bit.dirReversing = 1;
    }
    else        // 没有反向
    {
        runFlag.bit.dirReversing = 0;
        reverseTicker = 0;      // ticker清零
    }
}


//=====================================================================
//
// 数字设定频率.
// 频率源为数字设定，即UP/DOWN调节，包括键盘和DI
//
// 备注：
//      PLC和多段指令也会调用本函数
//
//=====================================================================   
LOCALF int32 FrqAimUpDownDeal(void)
{
    int32 frqDeltaDi;                      // 数字设定UP/DOWN频率的增量
    Uint16 up = diFunc.f1.bit.up;
    Uint16 down = diFunc.f1.bit.down;
    int32 delta = 0;
    static Uint16 bFrqDigitalOld;
    static int16 frqDeltaDiRemainder;      // 数字设定UP/DOWN频率的增量余值
    //frqFlag.bit.upDown = 1;
    
// 当DI的UP端子无效、DI的DOWN端子无效、
// 0级菜单下键盘没有单独按下UP、0级菜单下键盘没有单独按下DOWN，才认为没有进行数字设定频率
    if ((up || down || accDecFrqPrcFlag)
        && (MENU0_DISP_STATUS_RUN_STOP == menu0DispStatus)  // 在运行/停机显示时才up/down。即，在故障/调谐显示时不可up/down
        && (!runFlag.bit.dirReversing)  // 未处于正反转切换中
        && ((diSelectFunc.f2.bit.frqOk&&diFunc.f2.bit.frqOk) || (!diSelectFunc.f2.bit.frqOk))    // 频率设定起效端子功能被选择且端子有效 或 频率设定端子未选择
        && ((ERROR_LEVEL_RUN != errorAttribute.bit.level) 
           || ((ERROR_LEVEL_RUN == errorAttribute.bit.level) && (funcCode.code.errorRunFrqSrc == ERR_RUN_FRQ_AIM)))
        )
    {
        bFrqDigital = 1;
    }
    else
    {
        bFrqDigital = 0;
    }

    // 运行状态下，UP/DOWN进行数字设定频率，从当前(的瞬时)频率开始修改。
    if (bFrqDigitalOld != bFrqDigital)      // 数字设定频率标志改变了
    {
        if (bFrqDigital
            && runFlag.bit.common           // 之前没有数字设定频率，且当前电机运行
            && (SWING_NONE == swingStatus)  // 没有进入摆频
            )
        {
            //frqDigitalTmp = ABS_INT32(frq); // 设定频率从当前频率的绝对值开始修改
            if(funcCode.code.updnBenchmark //|| 
               // ((frqFlag.bit.comp) && 
               // (funcCode.code.frqYSrc >= FUNCCODE_frqXySrc_AI1))
              )
            {
                // 以设定频率为基准
            }
            else
            {
                // 处于下限频率限制中
    			if ((ABS_INT32(frqAim) < lowerFrq) && (ABS_INT32(frq) <= lowerFrq))
    			{
                    if (runFlag.bit.curDir^runDirPanelOld)
                    {
                        upDownFrq = uPDownFrqLower2;
                    }
                    else
                    {
                        upDownFrq = uPDownFrqLower1;
                    }
    			}
    			else
                {
        			// 非数字+数字
                    if ((frqFlag.bit.comp)
						&& ((((frqXy.x + frqXy.z) < 0)
                        	&& (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1))
						 || (((frqXy.y + frqXy.z + funcCode.code.presetFrq) < 0)
                        	&& (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1))
							)
                        )
        			{
    					upDownFrq += ABS_INT32(frqAim) - ABS_INT32(frq);
    				}
    				// X(数字)
    		        // Y(数字)
    		        // X(数字) + Y(数字)，也是非叠加
    		        // X(数字) + Y(非数字)
                    else
    				{
    					upDownFrq += ABS_INT32(frq) - ABS_INT32(frqAim);
    				}
                }
			}
        }

        if (!bFrqDigital)
        {
            bFrqDigitalDone4WaitDelay = 1;  // UP/DOWN结束标志
        }

        frqDeltaDiRemainder = 0;  // 数字设定频率标志改变时，余值清零
    }

    if (bFrqDigital)    // 正在UP/DOWN设定频率
    {
        upDownFrqInit = 0;
        frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_ON;
        
        if (ACC_DEC_FRQ_WAIT == accDecFrqPrcFlag)  // 键盘UP/DOWN修改频率
        {
            if (KEY_UP == keyFunc)      // 键盘增加设定频率，即0级菜单下按键UP
            {
                delta = (int32)frqKeyUpDownDelta;
            }
            else //if (KEY_DOWN == keyFunc) // 键盘减小设定频率，即0级菜单下按键DOWN
            {
                delta = -(int32)frqKeyUpDownDelta;
            }

            accDecFrqPrcFlag = ACC_DEC_FRQ_DONE;      // 按键UP/DOWN 已经处理
        }
        else if ((up || down)       // DI端子的UP/DOWN有效
                && (!(up && down) && (!frqFlag.bit.upDown))  // 同时有效，不变化
                )
        {
            int32 diUpDownSlope = funcCode.code.diUpDownSlope;
            
            if (down)
            {
                diUpDownSlope = -diUpDownSlope;
            }
            
            frqDeltaDi = (diUpDownSlope * FRQ_SRC_PERIOD + frqDeltaDiRemainder) / (10000);
            frqDeltaDiRemainder = (diUpDownSlope * FRQ_SRC_PERIOD + frqDeltaDiRemainder) % (10000);
            
            delta = frqDeltaDi;
        }
    }

    frqFlag.bit.upDown = 1;
    upDownFrq += delta;

    if (frqFlag.bit.upDownoperationStatus == UP_DN_OPERATION_OFF)
    {
        upDownFrq = (int16)upDownFrqInit;  
    }
    
    if (upDownFrq > uPDownFrqMax)
    {
        upDownFrq = uPDownFrqMax;
    }
    else if (upDownFrq < uPDownFrqMin)
    {
        upDownFrq = uPDownFrqMin;
    }
    
#if 1
    if ((diFunc.f1.bit.clearUpDownFrq)           // DI端子有效：UP/DOWN设定清零（端子、键盘）
//        || (frqSrcOld != frqSrc)                    // 切绞制噬瓒ǎ蛘咔谢坏絇LC/多段速
        )
    {
        upDownFrq = 0;
        frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
    }
#endif

    bFrqDigitalOld = bFrqDigital;

    // 主辅频始扑悖襓也为数字设定，预置频率(F0-08)不起作用
    if ((frqFlag.bit.comp) && (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1))
    {
        // X(非数字) + Y(数字)
        return upDownFrq;
    }
    else
    {
        // X(数字)
        // Y(数字)
        // X(数字) + Y(数字)，也是非叠加
        // X(数字) + Y(非数字)
        return (upDownFrq + (int32)funcCode.code.presetFrq);
    }
}


//=====================================================================
//
// 频率源为PLC
//
//=====================================================================
LOCALF int32 FrqPlcSetDeal(void)
{
#if DEBUG_F_MULTI_SET_PLC
    static Uint16 plcEndLoopDelayTicker;
    static Uint16 plcTimeTicker;
    static Uint16 plcFlag;
    static Uint16 plcStepEnable;
    static Uint16 bPlcEndOneLoopEnable = 1;
    Uint32 plcTimeMax = 0;
    Uint16 loop = 0;
    Uint16 plcStep1;            // PLC的阶段，[0, PLC_STEP_MAX-1]
    int32 frqPlc;

    runFlag.bit.plc = 1;
    bStopPlc = 0;               // 停止PLC标志

#define PLC_TIME_TICKER     5   // 5个PLC处理周期，plcTime增加1，防止plcTime溢出
    // 转速跟踪、启动直流制动时间不计入PLC时间，启动频率保持时间计入PLC时间。    
    if (((RUN_STATUS_START == runStatus) && (START_RUN_STATUS_HOLD_START_FRQ == startRunStatus))
        || (RUN_STATUS_NORMAL == runStatus)
        )
    {
        if ((!frqPlcType)  // 本次2ms循环未进行PLC运行处理
            && (++plcTimeTicker >= PLC_TIME_TICKER))   // 防止plcTimeMax溢出
        {
            plcTimeTicker = 0;
            plcTime++;
        }
        
        plcFlag = 1;    // 上电之后使用过PLC
    }
    else if (!runFlag.bit.run)  // 正在(单次运行)停机，不更新plcStep。
    {
        
        // 不掉电记忆。停机记忆暂不考虑
        if (((FUNCCODE_plcPowerOffRemMode_REM != (funcCode.code.plcPowerOffRemMode%10))
             || (plcFlag))  // 上电之后使用过PLC
             && (FUNCCODE_plcStopRemMode_REM != (funcCode.code.plcPowerOffRemMode/10)) // 非停机记忆
            && (!runCmd.bit.pause)
            )
        {
		    plcStep = 0;    // 从step0重新开始搜索
		    plcTime = 0;
        }       
        else
        {
            if (plcStep >= PLC_STEP_MAX)
            {
                plcStepRemOld = 0;
                plcTimeRemOld = 0;
            }
            plcStep = plcStepRemOld;    // 从plcStepRemOld重新开始搜索
            plcTime = plcTimeRemOld;    // 恢复plcTime
        }       
    }

    if (!plcStep)
    {
        bPlcEndOneLoopEnable = 1;
    }
    
    if (plcStep < PLC_STEP_MAX)
    {
        plcTimeMax = (Uint32)funcCode.code.plcAttribute[plcStep].runTime
            * ((TIME_UNIT_PLC / PLC_TIME_TICKER) / FRQ_SRC_PERIOD);
        if (FUNCCODE_plcTimeUnit_H == funcCode.code.plcTimeUnit) // PLC运行时间单位
            plcTimeMax *= TIME_UNIT_SEC_PER_HOUR;
    }
#undef PLC_TIME_TICKER

    if (plcTime >= plcTimeMax)
    {
        Uint16 bEndSearch = 0;
        
        plcTime = 0;
        plcTimeTicker = 0;

        for (;;)    // 搜索下一个运行时间不为0的plcStep
        {
            if (++plcStep >= PLC_STEP_MAX)
            {
                plcStep = PLC_STEP_MAX;     // 防止plcStep一直增加
                // plcRunMode改变时，也能处理
                if (FUNCCODE_plcRunMode_REPEAT == funcCode.code.plcRunMode) // 一直循环
                {
                    plcStep = 0;
                }

                if (FUNCCODE_plcRunMode_ONCE_RUN == funcCode.code.plcRunMode) 
                {
                    plcStep = plcStepEnable;
					bEndSearch = 1;
                }

                if ((plcStepOld < PLC_STEP_MAX) && (runFlag.bit.common) && (!runFlag.bit.jog))  // 完成一个PLC循环
                {
                    plcEndLoopDelayTicker = 0;
                    if (bPlcEndOneLoopEnable)
                    {
                        bPlcEndOneLoop = 1;
                        bPlcEndOneLoopEnable = 0;
                    }
                }
            }

            if (plcStep < PLC_STEP_MAX) // plcStep在有效范围内
            {
                if (funcCode.code.plcAttribute[plcStep].runTime)    // plc运行时间不为0，则停止搜索退出循环，设定频率为当前plcStep频率
                {
                    bEndSearch = 1;
                }
            }
            else
            {
                bEndSearch = 1;  // PLC运行不是一直循环，且plcStep >= PLC_STEP_MAX, 则退出循环
            }
            
            if (++loop > PLC_STEP_MAX)      // 全部plc阶段的运行时间都为0，不再循环，不启动PLC
            {
                bStopPlc = 1;
                bEndSearch = 1;
            }

            if (bEndSearch)
            {
                break;
            }
        }
    }

    if (plcStep >= PLC_STEP_MAX)    // 单次运行结束停机，或者单次运行结束保持终值
    {        
        if (FUNCCODE_plcRunMode_ONCE_STOP == funcCode.code.plcRunMode) // 单次运行结束停机
        {
            bStopPlc = 1;

            if (!runFlag.bit.common)    // PLC单次运行结束停机，加减速时间使用最后一段加减速时间不为0的那一段PLC的加减速时间
            {
                plcStep = 0;
            }
        }
        // 单次运行结束保持终值。保持为最后一段的frqAimTmp0
    }

    plcStep1 = plcStep;
    plcStepEnable = plcStep;
    if (plcStep1 >= PLC_STEP_MAX)
    {
        plcStep1 = PLC_STEP_MAX - 1;
    }

// PLC第_段加减速时间选择
    accDecTimeSrcPlc = funcCode.code.plcAttribute[plcStep1].accDecTimeSet;

// 处理frqAimTmp0
    frqPlc = UpdateMultiSetFrq(plcStep1);

// 完成一个PLC循环，输出一个_ms的脉冲信号
// 频率源选择，运行时不可修改。所以该_ms的处理放在这里没有问题。
// 也要考虑PLC单次运行停机/保持终值
    if (bPlcEndOneLoop)
    {
        if (++plcEndLoopDelayTicker >= PLC_END_LOOP_TIME_MAX / FRQ_SRC_PERIOD)
        {
            //plcEndLoopDelayTicker = 0;
            bPlcEndOneLoop = 0;
        }
    }
    
    frqPlcType = 1;

    return frqPlc;
#else
    return 0;
#endif
}


//=====================================================================
//
// 更新多段指令、PLC的频率
//
// 输入：
//      step -- 多段指令step
// 要求: 0 <= step <= PLC_STEP_MAX-1
//
//=====================================================================
LOCALF int32 UpdateMultiSetFrq(Uint16 step)
{
#if DEBUG_F_MULTI_SET_PLC
    static Uint16 stepOld;
    int32 frqPlc;
    int16 i;
    
    if (!step)               // plcStep == 0
    {
        switch (funcCode.code.plcFrq0Src) // F8-00 多段速0给定方式
        {
            case FUNCCODE_plcFrq0Src_FC:
                frqPlc = (int32)(int16)funcCode.code.plcFrq[0] * maxFrq / 1000;
                break;

            case FUNCCODE_plcFrq0Src_AI1:
            case FUNCCODE_plcFrq0Src_AI2:
            case FUNCCODE_plcFrq0Src_AI3:
                i = funcCode.code.plcFrq0Src - FUNCCODE_plcFrq0Src_AI1;
                frqPlc = ((int32)aiDeal[i].set * maxFrq + (1 << 14)) >> 15;
                break;
                
            case FUNCCODE_plcFrq0Src_PULSE:
                frqPlc = ((int32)pulseInSet * maxFrq + (1 << 14)) >> 15;
                break;
                
            case FUNCCODE_plcFrq0Src_PID:
                frqPlc = FrqPidSetDeal();
                break;
                
            case FUNCCODE_plcFrq0Src_PRESET_FRQ:
                if (stepOld)    // stepOld != 0
                {
                    frqDigitalTmp = frqDigitalPlc;  // 恢复frqDigitalTmp
                }
                
                frqPlc = FrqAimUpDownDeal();
                break;
                
            default:
                break;
        }
    }
    else    // plcStep != 0
    {
        if (!stepOld)    // stepOld == 0
        {
            frqDigitalPlc = frqDigitalTmp;  // 保存frqDigitalTmp
        }

        frqPlc = (int32)(int16)funcCode.code.plcFrq[step] * maxFrq / 1000;
    }

    stepOld = step;

    return frqPlc;
#endif
}



//=====================================================================
//
// 上下限频率，最大频率更新
// 上限频率源选择为AI时，100.0%杂δ苈肷瓒?
//
//=====================================================================
LOCALF void UpdateLimitFrq(void)
{
    int32 tmp;
    Uint16 i;

// 最大频率
    maxFrq = funcCode.code.maxFrq;
    frqPuQ15 = maxFrq + 20 * decNumber[funcCode.code.frqPoint];

// 上限频率
    tmp = funcCode.code.upperFrq;
    switch (funcCode.code.upperFrqSrc)
    {
        case FUNCCODE_upperFrqSrc_FC:  // 数值设定
            break;

        case FUNCCODE_upperFrqSrc_AI1: // AI1
        case FUNCCODE_upperFrqSrc_AI2: // AI2
        case FUNCCODE_upperFrqSrc_AI3: // AI3
            i = funcCode.code.upperFrqSrc - FUNCCODE_upperFrqSrc_AI1;
            tmp = ((int32)aiDeal[i].set * maxFrq + (1<<14)) >> 15;
            tmp += funcCode.code.upperFrqOffset;    // 上限频率偏置
            break;
            
        case FUNCCODE_upperFrqSrc_PULSE: // PULSE脉冲设定(DI5)
            tmp = ((int32)pulseInSet * maxFrq + (1<<14)) >> 15;
            tmp += funcCode.code.upperFrqOffset;    // 上限频率偏置
            break;
            
        case FUNCCODE_upperFrqSrc_COMM:
            // funcCode.code.frqComm不能超过32767，目前 [-10000, +10000]
            tmp = ((int32)(int16)funcCode.code.frqComm * maxFrq + 5000) / 10000;   // 四舍五入
            break;

        default:
            break;
    }

    

    if (tmp < 0L)        // AI，PulseIn可能为负数
        tmp = 0;
    if (tmp > (int32)maxFrq)
        tmp = maxFrq;
    
    upperFrq = tmp;

// 下限频率
    lowerFrq = funcCode.code.lowerFrq;

// 上限频率不能低于下限频率
    if (upperFrq < lowerFrq)
    {
        upperFrq = lowerFrq;
    }
}


//=====================================================================
//
// 函数  : 跳跃频率处理
// 参数  : 
//
//=====================================================================
LOCALF void JumpFrqDeal(void)
{
#if DEBUG_F_JUMP_FRQ
    int32 tmp = ABS_INT32(frqAimTmp);
    int32 tmp1 = ABS_INT32(frqTmp);

    jumpFrq1.frq = funcCode.code.jumpFrq1;
    jumpFrq1.range = funcCode.code.jumpFrqRange;
    JumpFrqEndCalc(&jumpFrq1);

    jumpFrq2.frq = funcCode.code.jumpFrq2;
    jumpFrq2.range = funcCode.code.jumpFrqRange;
    JumpFrqEndCalc(&jumpFrq2);

    if ((jumpFrq2.low <= jumpFrq1.low) && (jumpFrq1.low <= jumpFrq2.high))
    {
        jumpFrq1.low = jumpFrq2.low;
    }
    if ((jumpFrq1.low <= jumpFrq2.low) && (jumpFrq2.low <= jumpFrq1.high))
    {   
        jumpFrq2.low = jumpFrq1.low;
    }
    if ((jumpFrq2.low <= jumpFrq1.high) && (jumpFrq1.high <= jumpFrq2.high))
    {   
        jumpFrq1.high = jumpFrq2.high;
    }
    if ((jumpFrq1.low <= jumpFrq2.high) && (jumpFrq2.high <= jumpFrq1.high))
    {   
        jumpFrq2.high = jumpFrq1.high;
    }

    if ((jumpFrq1.low < tmp) && (tmp < jumpFrq1.high))  // 在跳跃频率区间
    {
        // 设定频率跳跃
        if ((tmp < ABS_INT32(frq)) && (tmp < ABS_INT32(frqAim))) // 要考虑：低于下限频率，以下限频率运行。
        {
            tmp = jumpFrq1.high;
        }
        else
        {
            tmp = jumpFrq1.low;
        }
        
    }
    if ((jumpFrq2.low < tmp) && (tmp < jumpFrq2.high))  // 在跳跃频率区间
    {
        // 设定频率跳跃
        if ((tmp < ABS_INT32(frq)) && (tmp < ABS_INT32(frqAim)))
        {
            tmp = jumpFrq2.high;
        }
        else
        {
            tmp = jumpFrq2.low;
        }
    }

    // 设定频率跳跃
    if (tmp > (int32)upperFrq)
    {
        tmp = (int32)upperFrq;
    }
    frqAimTmp = (frqAimTmp >= 0) ? (tmp) : (-tmp);
    
    
    if (funcCode.code.jumpFrqMode)
    {
        if ((jumpFrq1.low < tmp1) && (tmp1 < jumpFrq1.high))  // 在跳跃频率区间
        {
            if((ABS_INT16(frqAimTmp) >= ABS_INT16(frqTmp))
                    && (runFlag.bit.run)
                    && (runCmd.bit.common)
                )
            {
                tmp1 = jumpFrq1.high;
            }
            else
            {
                tmp1 = jumpFrq1.low;
            }        
        }
        if ((jumpFrq2.low < tmp1) && (tmp1 < jumpFrq2.high))  // 在跳跃频率区间
        {
            if((ABS_INT16(frqAimTmp) >= ABS_INT16(frqTmp))
                && (runFlag.bit.run)
                && (runCmd.bit.common)
            )
            {
                tmp1 = jumpFrq2.high;
            }
            else
            {
                tmp1 = jumpFrq2.low;
            }
        }

        if (tmp1 > (int32)upperFrq)
        {
            tmp1 = (int32)upperFrq;
        }

        frqTmp = (frqTmp >= 0) ? (tmp1) : (-tmp1);
    }
#endif
}

//=====================================================================
//
// 跳跃频率的两端点计算
//
// 输入：p->frq   -- 跳跃频率
//       p->range -- 跳跃频率幅度
// 输出：p->low   -- 跳跃频率范围的low
//       p->high  -- 跳跃频率范围的high
//
//=====================================================================
#define JUMP_FRQ_RANGE_ALL  0   // 跳跃频率的high, low之间的差值为jumpFrqRange的2倍。
#define JUMP_FRQ_RANGE_HALF 1   // 跳跃频率的high, low之间的差值为jumpFrqRange。
#define JUMP_FRQ_RANGE  JUMP_FRQ_RANGE_ALL  // 跳跃频率的high, low之间的差值为jumpFrqRange的2倍。
//#define JUMP_FRQ_RANGE  JUMP_FRQ_RANGE_HALF // 跳跃频率的high, low之间的差值为jumpFrqRange的1倍。
LOCALF void JumpFrqEndCalc(JUMP_FRQ *p)
{
#if DEBUG_F_JUMP_FRQ
    int32 low;

#if 0       // 跳跃频率设为0时，不起作用。与之前保持一致
    if (0 == p->frq)    // 跳跃频率设为0时，不起作用。
    {
        p->low = 0;
        p->high = 0;
        return;
    }
#endif

    low = (int32)p->frq - ((int32)p->range >> JUMP_FRQ_RANGE);
    if (low < 0)
        low = 0;
    p->low = low;

    p->high = (int32)p->frq + ((int32)p->range >> JUMP_FRQ_RANGE);
#endif
}
#undef JUMP_FRQ_RANGE_ALL
#undef JUMP_FRQ_RANGE_HALF
#undef JUMP_FRQ_RANGE



//=====================================================================
//
// 更新运行模式
// 转矩控制，速度控制，位置控制
//
//=====================================================================
void UpdateRunMode(void)
{
    static Uint16 runModeOld;

#if (DEBUG_F_POSITION_CTRL)
    if ((FUNCCODE_posCtrl_POSITION_CTRL == funcCode.code.posCtrl)   // 位置控制
        || ((FUNCCODE_posCtrl_SWITCH_TO_PC == funcCode.code.posCtrl)    // 速度/转矩控制<->位置控制
            && (diFunc.f3.bit.posCtrl))
        || ((FUNCCODE_posCtrl_SWITCH_FROM_PC == funcCode.code.posCtrl)  // 位置控制<->速度/转矩控制
            && (!diFunc.f3.bit.posCtrl))
        )
    {
        if (FUNCCODE_motorCtrlMode_FVC == motorFc.motorCtrlMode)
        {
            runMode = RUN_MODE_POSITION_CTRL;   // 位置控制
        }
        else if (runFlag.bit.run)
        {
            errorOther = 98;    // 当前不是FVC，则报错98
        }
    }
    else
#endif
    {
        // 速度控制/转矩控制切换
        runMode = (funcCode.code.torqueCtrl)^ diFunc.f2.bit.SpdTorqSwitch;
        if (FUNCCODE_motorCtrlMode_VF == motorFc.motorCtrlMode)     // VF运行
        {
            runMode = RUN_MODE_SPEED_CTRL;                  // 速度控制
        }

        // 转矩控制禁止
        runMode = runMode & (~diFunc.f1.bit.forbidTorqueCtrl);

        // 转矩控制
        if (RUN_MODE_TORQUE_CTRL == runMode)
        {
            #if DEBUG_F_MSC_CTRL
            int32 tmp;
            tmp = (int32)(int16)MasterSlaveCtrl.SlaveRcvFrq  * maxFrq / 10000;
            
            if((MasterSlaveCtrl.MSCEnable)          // 主从分配有效
                &&(!MasterSlaveCtrl.isMaster)        // 从机有效
//                && (!funcCode.code.slaveRevDataSel)  // 转矩有效
                )
            {
                if(MasterSlaveCtrl.p2pRunDir) // 主机F0-09 = 1，从机需要取反，否则会出现速度不一致
                {
                    frqAimTmp0 = -tmp;
                }
                else
                {
                    frqAimTmp0 = tmp;
                }
            }
            else
            #endif
            
            if (upperTorque >= 0)
            {
                frqAimTmp0 = funcCode.code.torqueCtrlFwdMaxFrq;   // 转矩控制正向最大频率
            }
            else
            {
                    frqAimTmp0 = -(int32)funcCode.code.torqueCtrlRevMaxFrq;   // 转矩控制反向最大频率
            }
            #if 0//DEBUG_F_MSC_CTRL
            
            // 负荷分配从机频率限幅
            if ((MasterSlaveCtrl.MSCEnable)          // 主从分配有效
                &&(!MasterSlaveCtrl.isMaster)        // 从机有效
                && (!funcCode.code.slaveRevDataSel)  // 转矩有效
                && (funcCode.code.slaveTCFrqPosiDev) // 从机防飞车系数有效
                )
            {
                // 从机频率限幅
                Uint16 masterFrq = (Uint32)(ABS_INT16((int16)MasterSlaveCtrl.SlaveRcvFrq ) + funcCode.code.slaveTCFrqPosiDev) * maxFrq / 10000;

                if (ABS_INT32(frq) > masterFrq)
                {
                    if (frqAimTmp0 >= 0)
                    {
                        frqAimTmp0 = masterFrq;
                    }
                    else
                    {
                        frqAimTmp0 = -(int32)masterFrq;
                    }
                }                
            }
            #endif

        }
        // 转矩控制 --> 非转矩控制
        else if (RUN_MODE_TORQUE_CTRL == runModeOld)    
        {
            frqTmp = frqRun;
        }
    }
 
    runModeOld = runMode;
}



int32 FrqSrcOperate(void)
{
    int32 frq;
    Uint16 digit[5];
    GetNumberDigit1(digit, funcCode.code.frqRunCmdBind);
    runCmdBind = digit[runSrc];
    frqFlag.bit.frqPidFlag = 0;     // 清使用频率PID标志

    // 命令源绑定的频率源
    if (runCmdBind > 0)
    {
        frqAiPu = maxFrq;
        frqFlag.bit.comp = 0;
        frq = FrqXyCalc(runCmdBind);
        frqXDisp = 0; // 显示
        frqYDisp = 0;
    }
    // 无绑定
    else
    {
        frq = GetFrq();

        frqXDisp = (frqXy.x); // 显示
        frqYDisp = (frqXy.y);
    }
    return frq;
}



#define UP_DOWN_LIMIT_X 0
#define UP_DOWN_LIMIT_Y 1
#define UP_DOWN_LIMIT_NO_COMP 0
#define UP_DOWN_LIMIT_COMP 1
int32 frqMax;
int32 frqMin;
void getUpDownLimit(Uint16 comp, Uint16 type)
{
    int32 noUpDownFrq;
    int32 min;
    int32 upDownFrqPu;
    int32 minFrq;
    minFrq = frqMin;
    // 为组合(数字+非数字 /  非数字+数字)
    if (comp == UP_DOWN_LIMIT_COMP)
    {
        //非数字+数字
        if (type == UP_DOWN_LIMIT_Y)  // Y为数字
        {
            // X(非数字) + Y(数字)
            noUpDownFrq = frqXy.x + frqXy.z;
        }
        // 数字+非数字
        else if (type == UP_DOWN_LIMIT_X)  // X为数字
        {
                noUpDownFrq = frqXy.y + frqXy.z + (int32)funcCode.code.presetFrq;
            //if (frqXy.y >= 0)
            {
                min = 0 + frqXy.y + frqXy.z;
                if (min > minFrq)     // 取大值
                {
                    minFrq = min;
                }
            }
            #if 0
            else
            {
                max = 0 + frqXy.y - frqXy.z;
                if (max < frqMax)     // 取大值
                {
                    frqMax = max;
                }
            }
            #endif    
            
        }
    }
    else
    {
            noUpDownFrq = frqXy.z + (int32)funcCode.code.presetFrq;
            minFrq = frqXy.z;
    }

    // 最小值限幅
    if (minFrq > frqMax)
    {
        minFrq = frqMax;
    }
    
    uPDownFrqMax = frqMax - noUpDownFrq;
    uPDownFrqMin = minFrq - noUpDownFrq;
    uPDownFrqLower1 = lowerFrq - noUpDownFrq;
    uPDownFrqLower2 = (int16)(-lowerFrq) - noUpDownFrq;

     // 叠加时计算辅助频率Y(数字)的范围
    if ((comp == UP_DOWN_LIMIT_COMP) && (type == UP_DOWN_LIMIT_Y)) 
    {
        // 叠加时辅助频率源Y的范围
        if (funcCode.code.frqYRangeBase)
        {
            upDownFrqPu  = ((Uint32)ABS_INT32(frqXy.x)*funcCode.code.frqYRange) / 100;
        }
        else
        {
            upDownFrqPu = ((Uint32)maxFrq*funcCode.code.frqYRange) / 100;
        }

        if (uPDownFrqMax > upDownFrqPu)
        {
            uPDownFrqMax = upDownFrqPu;
        }

        if (uPDownFrqMin < (-upDownFrqPu))
        {
            uPDownFrqMin = -upDownFrqPu;
        }
    }
}

void ResetUpDownFrq(void)
{
    frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
    upDownFrq = 0;
    uPDownFrqMax = 0;
    uPDownFrqMin = 0;
    uPDownFrqLower1= 0;
    uPDownFrqLower2= 0;

}

// 获取经过主辅运算之后的频率
int32 GetFrq(void)
{
    int32 frq;
//    static s16 upperFrqBak;
//    static s16 lowerFrqBak;

    frqMax = upperFrq;         // 最大，默认为 上限频率
    frqMin = -(int32)upperFrq; // 最小，默认为 -上限频率

    
#if 0
    if ((upperFrqBak != upperFrqBak)
        || (lowerFrqBak != lowerFrq)
        )
    {
        ResetUpDownFrq();
    }
    
	upperFrqBak = upperFrq;
	lowerFrqBak = lowerFrq;
#endif

    frqFlag.bit.comp = 0;           // 先清零
    frqFlag.bit.upDown = 0;
    frqFlag.bit.x = 0;
    frqFlag.bit.y = 0;
    frqXy.x = 0;
    frqXy.y = 0;
    frqXy.z = 0;                    // 若没有叠加，该值仍然为0
    
    //switch ((funcCode.code.frqCalcSrc >> 0) & 0x000F)
    switch (funcCode.code.frqCalcSrc%10)
    {
        case FUNCCODE_frqCalcSrc_X: // 主频率源X
            frqFlag.bit.x = 1;  // 主频率源X有效
            
            break;

        case FUNCCODE_frqCalcSrc_COMPOSE: // 主辅运算结果
            frqFlag.bit.x = 1;  // 主频率源X有效
            frqFlag.bit.y = 1;  // 辅助频率源Y有效
            
            break;

        case FUNCCODE_frqCalcSrc_X_OR_Y: // 主 <--> 辅
            if (!diFunc.f1.bit.frqSrcSwitch)
            {
                frqFlag.bit.x = 1;  // 主频率源X有效
            }
            else
            {
                frqFlag.bit.y = 1;  // 辅助频率源Y有效
            }
            
            break;

        case FUNCCODE_frqCalcSrc_X_OR_COMPOSE: // 主 <--> 主辅运算结果
            if (!diFunc.f1.bit.frqSrcSwitch)
            {
                frqFlag.bit.x = 1;  // 主频率源X有效
            }
            else
            {
                frqFlag.bit.x = 1;  // 主频率源X有效
                frqFlag.bit.y = 1;  // 辅助频率源Y有效
            }
            
            break;

        case FUNCCODE_frqCalcSrc_Y_OR_COMPOSE: // 辅 <--> 主辅运算结果
            if (!diFunc.f1.bit.frqSrcSwitch)
            {
                frqFlag.bit.y = 1;  // 辅助频率源Y有效
            }
            else
            {
                frqFlag.bit.x = 1;  // 主频率源X有效
                frqFlag.bit.y = 1;  // 辅助频率源Y有效
            }
            break;

        default:
            break;
    }   

    if (frqFlag.bit.x && frqFlag.bit.y)
    {
        frqXy.z = funcCode.code.frqYOffsetFc; // 辅助频率偏置
        
        //if (((funcCode.code.frqCalcSrc >> 4) & 0x000F) == 0)
        if ((funcCode.code.frqCalcSrc/10) == 0)
        {
            if ((funcCode.code.frqXSrc >= FUNCCODE_frqXySrc_AI1) ||
                (funcCode.code.frqYSrc >= FUNCCODE_frqXySrc_AI1))
            {
                frqFlag.bit.comp = 1;                 // 置comp标志
            }
        }
    }

    // 频率源是否发生改变
    // 频率源是否发生改变
    if (frqSrcValue != (frqFlag.bit.x + (frqFlag.bit.y << 1)))
    {
        frqSrcChgFlag = 1;
        if(frqFlag.bit.x^frqFlag.bit.y)      // 不叠加 x y={1 ,0} or {0, 1}
        {        
            frqSrcNonXAddYFlag = 1;          // 非主加辅
        }
    }
    else
    {
        frqSrcChgFlag = 0;
        frqSrcNonXAddYFlag = 0;          // 非主加辅
    }
    frqSrcValue = frqFlag.bit.x + (frqFlag.bit.y << 1);

    // X+Y为组合频率
    if (frqFlag.bit.comp)
    {
        // 数字+非数字
        if (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1)
        {
            GetFrqY();  // 计算辅助频率Y
            getUpDownLimit(UP_DOWN_LIMIT_COMP, UP_DOWN_LIMIT_X);
            GetFrqX();  // 计算主频率X
        }
        // 非数字+数字
        else if (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1)
        {
            GetFrqX();  // 计算主频率X
            getUpDownLimit(UP_DOWN_LIMIT_COMP, UP_DOWN_LIMIT_Y); 
            GetFrqY();  // 计算辅助频率Y
        }
        // 非数字+非数字
        else
        {
            GetFrqX();  // 计算主频率X
            GetFrqY();  // 计算辅助频率Y
        }
    }
    // 主
    else 
    {
        // 主
        if (frqFlag.bit.x)
        {
            if (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1)
            {
                getUpDownLimit(UP_DOWN_LIMIT_NO_COMP, UP_DOWN_LIMIT_X); 
            }
            GetFrqX();  // 计算主频率X
        }
        
        // 辅
        if (frqFlag.bit.y)
        {
            // 辅助频率Y的范围为X
            if (funcCode.code.frqYRangeBase)
            {
                GetFrqX();
            }
            
            if (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1)
            {
                getUpDownLimit(UP_DOWN_LIMIT_NO_COMP, UP_DOWN_LIMIT_Y); 
            }
            
            GetFrqY();  // 计算辅助频率Y
        }   
    }
    
    if (frqFlag.bit.x && frqFlag.bit.y)
    {
        frq = GetFrqComp();
    }
    else if (frqFlag.bit.x)
    {
        frq = frqXy.x;
    }
    else if (frqFlag.bit.y)
    {
        frq = frqXy.y;
    }

    if (frq > frqMax)
    {
        frq = frqMax;
    }
    else if (frq < frqMin)
    {
        frq = frqMin;
    }

    // 防止主辅频率切换时一个为数字一个不为数字
    // 切换为非数字时frqFlag.bit.upDown无法置1,此时upDownFrq会被清除
    if (frqFlag.bit.upDown)
    {
        if(!((funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1) || 
            (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1)
            ))
        {
            frqFlag.bit.fcPosLimit = 0;
            frqFlag.bit.fcNegLimit = 0;
            frqFlag.bit.upDownoperationStatus = UP_DN_OPERATION_OFF;
            upDownFrqTmp = 0;
            upDownFrq = 0;
            
        }
    }
    return frq;
}


// 主辅运算
int32 GetFrqComp(void)
{
#if DEBUG_F_FRQ_SRC_COMPOSE
    int32 frq;
    int32 x;
    int32 y;   
    int32 frqYCompMax;   // 辅助频率Y叠加时上限

    // 叠加时辅助频率源Y的范围
    if (funcCode.code.frqYRangeBase)
    {
        frqYCompMax  = ((Uint32)ABS_INT32(frqXy.x)*funcCode.code.frqYRange) / 100;
    }
    else
    {
        frqYCompMax = ((Uint32)maxFrq*funcCode.code.frqYRange) / 100;
    }

    // Y频率限幅
    if ((frqXy.y > frqYCompMax)
       && ((funcCode.code.frqCalcSrc/10) != FUNCCODE_frqCalcSrc_ADD)
       )
    {
        frqXy.y = frqYCompMax;
    }

    
    //switch ((funcCode.code.frqCalcSrc >> 4) & 0x000F)
    switch (funcCode.code.frqCalcSrc/10)
    {
        case FUNCCODE_frqCalcSrc_ADD:   // 主 + 辅
            x = frqXy.x;

            // X+Y时，当辅助频率源为数字给定时，预置频率（F0-08）不起作用，UP/DOWN起作用
            if ((funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1) &&
                (funcCode.code.frqYSrc < FUNCCODE_frqXySrc_AI1))
            {
                frqXy.y = 0;
            }
            y = frqXy.y;
            
            frq = x + y;
            //frqXy.z = zFc;

#if 0
            if (frq > upperFrq)
            {
                frq = upperFrq;
                frqXy.y = upperFrq - x - z;
            }
            else if (frq < -(int32)upperFrq)
            {
                frq = -(int32)upperFrq;
                frqXy.y = -(int32)upperFrq - x - z;
            }
#endif
            break;

        case FUNCCODE_frqCalcSrc_SUBTRATION: // 主 - 辅
            x = frqXy.x;
            y = frqXy.y; 
            
            frq = x - y;
            //frqXy.z = zFc;
            break;

        case FUNCCODE_frqCalcSrc_MAX: // MAX(主, 辅)
            x = frqXy.x;
            y = frqXy.y; 

            //frq = GetMax(x, y);
            
            if (ABS_INT32(x) >= ABS_INT32(y))
            {
                frq = x;
            }
            else
            {
                frq = y;
            }
            
            //frqXy.z = zFc;
            break;

        case FUNCCODE_frqCalcSrc_MIN: // MIN(主, 辅)
            x = frqXy.x;
            y = frqXy.y; 
            
            if (ABS_INT32(x) < ABS_INT32(y))
            {
                frq = x;
            }
            else
            {
                frq = y;
            }

            //frqXy.z = zFc;
            break;

        default:
            break;
    }
    
    frq += frqXy.z;   // 辅助频率偏置
    
#if 0
    if (frq > upperFrq)
    {
        frq = upperFrq;
    }
    else if (frq < -(int32)upperFrq)
    {
        frq = -(int32)upperFrq;
    }
#endif

    return frq;
#endif
}



//===========================================================
// 获取主频率X
//===========================================================
int32 GetFrqX(void)
{
    int32 frq;
    
    frqAiPu = maxFrq;
    //frqFlag.bit.x = 1;

    // 主频率源X与预置频率切换
    if (diFunc.f2.bit.frqXSrc2Preset) 
    {
        frq = funcCode.code.presetFrq;
    }
    else
    {
        frq = FrqXyCalc(funcCode.code.frqXSrc);
    }
    frqXy.x1 = frq;     // 赋值
    frqXy.x = frq;     // 赋值
    
    return frq;
}


// 获取辅频率Y
int32 GetFrqY(void)
{
#if DEBUG_F_FRQ_SRC_COMPOSE
    int32 frq;
    
    // 为叠加
    if (frqFlag.bit.comp)    
    {
        // 叠加时Y范围为X
        if (FUNCCODE_frqYRangeBase_FRQ_X == funcCode.code.frqYRangeBase)
        {
            // X(数字) + Y(非数字)时以预置频率作为Y的量程
            if ((funcCode.code.frqYSrc >= FUNCCODE_frqXySrc_AI1) 
                && (funcCode.code.frqXSrc < FUNCCODE_frqXySrc_AI1)
                )
            {
                frqAiPu = funcCode.code.presetFrq;
            }
            else
            {
                frqAiPu = ABS_INT32(frqXy.x);
            }   
        }
        // 叠加时Y范围为最大频率
        else
        {
            frqAiPu = maxFrq;
        }

        frqAiPu = ((Uint32)frqAiPu * funcCode.code.frqYRange) / 100;
    }
    // 相对于最大频率
    else    
    {
        frqAiPu = maxFrq;
    }

    // 辅频率源Y与预置频率切换
    if (diFunc.f2.bit.frqYSrc2Preset) 
    {
        frq = funcCode.code.presetFrq;
    }
    else
    {
        frq = FrqXyCalc(funcCode.code.frqYSrc);
    }        

    // 叠加时辅助频率Y限幅
    if (frqFlag.bit.comp)
    {
        if (frq > frqAiPu)
        {
            frq = frqAiPu;
        }
        else if (frq < -frqAiPu)
        {
            frq = - frqAiPu;
        }
    }
    
    frqXy.y = frq;     // 赋值
    
    return frq;
#endif
}


//=====================================================================
//
//
//=====================================================================
LOCALF int32 FrqXyCalc(Uint16 src)
{
    int32 tmp = 0;
    
    switch (src)         // 非点动，才进入频率源处理
    {
        case FUNCCODE_frqXySrc_FC:              // 功能码设定，掉电不记忆
        case FUNCCODE_frqXySrc_FC_P_OFF_REM:    // 功能码设定，掉电记忆
            tmp = FrqAimUpDownDeal();
            break;

        case FUNCCODE_frqXySrc_AI1:             // AI1
        case FUNCCODE_frqXySrc_AI2:             // AI2
        case FUNCCODE_frqXySrc_AI3:             // AI3
                tmp = src - FUNCCODE_frqXySrc_AI1;
            //tmp = ((int32)aiDeal[tmp].set * frqAiPu + (1 << 14)) >> 15;
            tmp = ((int32)aiDeal[tmp].set * frqAiPu ) / 0x7FFF;
            break;

        case FUNCCODE_frqXySrc_PULSE:           // PULSE脉冲设定(DI5)
            tmp = ((int32)pulseInSet * frqAiPu + (1 << 14)) >> 15;
            break;

        case FUNCCODE_frqXySrc_MULTI_SET:       // 多段指令
            tmp = UpdateMultiSetFrq(diFunc.f1.bit.multiSet);
            break;

        case FUNCCODE_frqXySrc_PLC:             // PLC
            tmp = FrqPlcSetDeal();
            break;

        case FUNCCODE_frqXySrc_PID:             // PID
            frqFlag.bit.frqPidFlag = 1;
            tmp = FrqPidSetDeal();
            break;

        case FUNCCODE_frqXySrc_COMM:            // 通讯设定
            // funcCode.code.frqComm不能超过32767，目前 [-10000, +10000]

#if DEBUG_F_PLC_CTRL
            if (funcCode.code.plcEnable)  // PLC功能有效
            {
                tmp = (int32)(int16)funcCode.code.plcFrqSet * maxFrq / 10000;
            }
            else
#endif
#if DEBUG_F_MSC_CTRL
            if ((MasterSlaveCtrl.MSCEnable)     // 主从控制有效
                && (!MasterSlaveCtrl.isMaster)  // 为从机
//                && (funcCode.code.slaveRevDataSel == MSC_P2P_RCV_FRQ_SET)  // 从机数据作为频率给定
                )
            { 
                tmp = ((int32)(int16)MasterSlaveCtrl.SlaveRcvFrq* maxFrq) / 10000;
                if(MasterSlaveCtrl.p2pRunDir) // 主机F0-09 = 1，从机需要取反，否则会出现速度不一致
                {
                    tmp = -tmp;
                }
            }
            else
#endif
            if ((funcCode.code.commProtocolSec == EXTEND_COM_CAR)   // PROFIBUS-DP频率给定
                )
            {
                tmp = (int32)(int16)dpFrqAim;                    // 直接给定频率
                #if 0
                // 转速换算
                if(funcCode.code.commSetSpeedType) //  下达为转速
                tmp = (int32)(int16)dpFrqAim * polePair * 100 / 60;
                #endif
            }
            else
            {
                tmp = (int32)(int16)funcCode.code.frqComm * maxFrq / 10000;
            }
            break;

        default:
            break;
    }

    return tmp;
}





