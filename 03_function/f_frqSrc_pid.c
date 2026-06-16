//======================================================================
//
// 频率源处理，PID处理-。 
//
// Time-stamp: <2008-03-20 10:10:04  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_frqSrc.h"
#include "f_main.h"
#include "f_runSrc.h"
#include "f_io.h"
#include "f_menu.h"
#include "f_ui.h"

#if F_DEBUG_RAM                         // 仅调试功能，在CCS的build option中定义的宏
#define DEBUG_F_PID             0       // PID
#elif 1

#if !DEBUG_F_POSITION_CTRL
#define DEBUG_F_PID             1
#elif 1
#define DEBUG_F_PID             0
#endif

#endif


#define PID_ERROR_DEAD          20      // PID最小死区，Q15



PID_FUNC pidFunc = PID_FUNC_DEFAULTS;


#if DEBUG_F_PID
LINE_CHANGE_STRUCT pidFuncRef = LINE_CHANGE_STRTUCT_DEFALUTS;

#if 0
// 根据偏差改变PID参数
struct PID_PARA_LINE
{
    LINE_STRUCT pLine;
    LINE_STRUCT iLine;
    LINE_STRUCT dLine;
};
struct PID_PARA_LINE pidParaLine = {LINE_STRTUCT_DEFALUTS, LINE_STRTUCT_DEFALUTS, LINE_STRTUCT_DEFALUTS};
#else
    LINE_STRUCT pidParaLine = LINE_STRTUCT_DEFALUTS;
#endif


#define PID_LPF_FDB 0   // PID反馈滤波
#define PID_LPF_OUT 1   // PID输出滤波
LowPassFilter pidLpf[2] = {LPF_DEFALUTS, LPF_DEFALUTS};


Uint32 pidInitTicker;
enum PID_CALC_FLAG
{
    PID_CALC_FLAG_NO_CALC,          // 0-初值，不计算PID
    PID_CALC_FLAG_CALC,             // 1-计算PID
    PID_CALC_FLAG_PID_INIT_HOLD,    // 2-PID初值
    PID_CALC_FLAG_CALC_TRUE         // 3-真正计算PID
};
enum PID_CALC_FLAG pidCalcFlag;

enum PID_CALC_DEAL
{
    PID_CALC_INIT_U0,     // 0-PID真正计算初值U0
    PID_CALC_TRUE         // 1-真正计算PID
};
enum PID_CALC_DEAL pidCalcDeal;

Uint32 pidFdbLoseTicker;    // PID反馈丢失检测的ticker
int32 pidError;             // PID 当前偏差
Uint16 bPidFdbLose;
int32  deltaError;           // 偏差的微分

void UpdatePidFuncPara(PID_FUNC *p);
void UpdatePidFuncRefFdb(PID_FUNC *p);



//=====================================================================
//
// 频率源为PID
//
//=====================================================================
int32 FrqPidSetDeal(void)
{
    static Uint16 pidCalcFlagOld;
    int32 frqPid;
    int32 pidOut = pidFunc.out;
#if 1
    runFlag.bit.pid = 1;

    UpdatePidFuncRefFdb(&pidFunc);  // pid给定、反馈停机时也要运算
    UpdatePidFuncPara(&pidFunc);

    
    if ((FUNCCODE_pidCalcMode_YES == funcCode.code.pidCalcMode) // 停机时运算
            || (runCmd.bit.common))   // 使用运行命令有效
         // || (runFlag.bit.common)) 
    {
        if (PID_CALC_FLAG_NO_CALC == pidCalcFlag)
        {
            pidCalcFlag = PID_CALC_FLAG_CALC;
        }
    }
    else
    {
        pidCalcFlag = PID_CALC_FLAG_NO_CALC;
		pidCalcDeal = PID_CALC_INIT_U0;
    }

// 是否为第1拍启动PID
    if ((PID_CALC_FLAG_CALC == pidCalcFlag)
        && (!pidCalcFlagOld)
        )
    {
        pidCalcFlag = PID_CALC_FLAG_PID_INIT_HOLD;
        
        pidInitTicker = 0;
    }

    // PID初值
    if (PID_CALC_FLAG_PID_INIT_HOLD == pidCalcFlag)
    {
        if (++pidInitTicker >= (int32)funcCode.code.pidInitTime * (TIME_UNIT_PID_INIT / PID_CALC_PERIOD))
        {
            pidCalcFlag = PID_CALC_FLAG_CALC_TRUE;
        }
        
        pidFunc.out = (int32)funcCode.code.pidInit * 0x7fff / 1000;
        pidOut = pidFunc.out;
    }

    // 计算PID偏差
    pidError = pidFunc.ref - pidFunc.fdb;    
    if (funcCode.code.pidDir^diFunc.f2.bit.pidDirRev)   // PID作用方向
    {
        pidError = -pidError;
    }

    // PID计算
    if ((PID_CALC_FLAG_CALC_TRUE == pidCalcFlag)
        && (!diFunc.f1.bit.pidPause)    // PID暂停端子无效
        && (!runFlag.bit.jog)           //+= 运行中点动，暂时不计算。?
        && ((ABS_INT32(pidError) > pidFunc.errorDead) )    // 大于偏差极限才进行PID调节计算
        )
    {
     
        pidFunc.calc(&pidFunc);         // PID计算

        // PID输出滤波
        pidLpf[PID_LPF_OUT].t = (int32)funcCode.code.pidOutLpfTime * (TIME_UNIT_PID_FILTER / PID_CALC_PERIOD); // 滤波
        pidLpf[PID_LPF_OUT].in = pidFunc.out;
        pidLpf[PID_LPF_OUT].calc(&pidLpf[PID_LPF_OUT]);
        pidOut = pidLpf[PID_LPF_OUT].out;
    }

    // 更新PID偏差记录
    pidFunc.error2 = pidFunc.error1;
    pidFunc.error1 = pidFunc.error;
    pidFunc.error = pidError;

    
     if ((!pidCalcFlag)                     // 停机时不运算，停机时清零
        || (frqCalcSrcOld != frqCalcSrc)    // 从其他频率源切换到PID，PID清零
        || ((plcStepOld) && (!plcStep))     // PLC的第0段使用PID，重新进入plc第0段
        || (frqSrcChgFlag)
        )
    {
        pidFunc.deltaPRem = 0;
        pidFunc.deltaIRem = 0;
        pidFunc.deltaDRem = 0;
        pidFunc.deltaRemainder = 0;
		pidLpf[PID_LPF_OUT].remainder = 0;

        if(frqSrcChgFlag)
        {
            if((!frqSrcNonXAddYFlag)) // 主+辅
            {
                pidFunc.out = (int32)frqFlag.bit.frqPidFlag*(1L-2*(funcCode.code.runDir^runCmd.bit.dir))*(frqTmp-frqXy.x1)* 0x7fff / maxFrq;
            }
            else                      // 非 主+辅
            {
                pidFunc.out = (int32)frqFlag.bit.frqPidFlag*(1L-2*(funcCode.code.runDir^runCmd.bit.dir))*frqTmp * 0x7fff / maxFrq;
            }
        }
        else
        {
             pidFunc.out = (int32)funcCode.code.pidInit * 0x7fff / 1000;
        }
        pidFunc.error2 = pidError;
        pidFunc.error1 = pidError;
        pidFunc.error = pidError;
    }
     
    if(pidOut >= 0)
	{
	    frqPid = (pidOut * maxFrq + (1 << 14)) /0x7fff;
	}
	else
	{
		frqPid = (pidOut * maxFrq - (1 << 14)) /0x7fff;
	}

    pidCalcFlagOld = pidCalcFlag;
#endif    
    return frqPid;
}


//=====================================================================
//
// PID计算
//
//=====================================================================
int16 DeltaPDisp,DeltaIDisp,DeltaDDisp;

void PidFuncCalc(PID_FUNC *p)
{
    static Uint16 pidChgOld;
    int32 Kp, Ki, Kd;
    int32 deltaP, deltaI, deltaD;   // P,I,D单独部分的delt

// Kp,Ki,Kd赋值
    Kp = p->Kp;
    Ki = p->Ki;
    Kd = p->Kd;
#if 1   
// PID参数切换
    if (FUNCCODE_pidParaChgCondition_DI  == funcCode.code.pidParaChgCondition)   // DI端子
    { 
        if (diFunc.f2.bit.pidChg)   // DI-PID参数切换 端子有效
        {
            Kp = p->Kp2;
            Kd = p->Kd2;
            Ki = p->Ki2;
        }

        if(diFunc.f2.bit.pidChg != pidChgOld)
        {
            int32 tmp1,tmp2;

            tmp1 = (Kp * (pidError - p->error));
            tmp2 = (Kd * (pidError - p->error2 + 3 * (p->error - p->error1)));
                
            if(p->Qi >= p->Qp)
            {
                tmp1 = tmp1<<(p->Qi - p->Qp);
            }
            else
            {
                tmp1 = tmp1>>(p->Qp - p->Qi);
            }

            if(p->Qi >= p->Qd)
            {
                tmp2 = tmp2<<(p->Qd - p->Qi);
            }
            else
            {
                tmp2 = tmp2>>(p->Qd - p->Qi);
            }
            
            Ki = (p->delta<<p->Qi - tmp1 - tmp2 )/((pidError + p->error) >> 1);
        }
        
        pidChgOld = diFunc.f2.bit.pidChg;
    }
    else if ((FUNCCODE_pidParaChgCondition_PID_ERROR == funcCode.code.pidParaChgCondition)   // 根据偏差自动切换
            ||(FUNCCODE_pidParaChgCondition_FRQ  == funcCode.code.pidParaChgCondition))
    {
        if(FUNCCODE_pidParaChgCondition_FRQ  == funcCode.code.pidParaChgCondition)
        {
            pidParaLine.x1 = 0;
            pidParaLine.x2 = maxFrq;
            pidParaLine.x = ABS_INT32(frq); // p->error不会超过0xffff
        }
        else
        {
            pidParaLine.x1 = p->errorSmall;
            pidParaLine.x2 = p->errorBig;
            pidParaLine.x = ABS_INT32(pidError); // p->error不会超过0xffff
        }
        
        pidParaLine.y1 = p->Kp;
        pidParaLine.y2 = p->Kp2;
        //pidParaLine.x = ABS_INT32(pidError); // p->error不会超过0xffff
        pidParaLine.calc(&pidParaLine);
        Kp = pidParaLine.y;

        pidParaLine.y1 = p->Ki;
        pidParaLine.y2 = p->Ki2;
        //pidParaLine.x = ABS_INT32(pidError); // p->error不会超过0xffff
        pidParaLine.calc(&pidParaLine);
        Ki = pidParaLine.y;

        pidParaLine.y1 = p->Kd;
        pidParaLine.y2 = p->Kd2;
        //pidParaLine.x = ABS_INT32(pidError); // p->error不会超过0xffff
        pidParaLine.calc(&pidParaLine);
        Kd = pidParaLine.y;
    }

// 计算deltaP
    p->deltaPRem += (int64)Kp * (pidError - p->error);     // Qp + 15
    deltaP = p->deltaPRem >> p->Qp;                     // Q15
    p->deltaPRem = p->deltaPRem - (deltaP << p->Qp);    // 保留余值，下次计算使用
    
// 计算deltaI
    if (( (((p->out >= p->outMax) && (pidError > 0)) 
        || ((p->out < p->outMin) && (pidError < 0))
        || (frqFlag.bit.frqSetLimit && frqFlag.bit.frqPidFlag))             // 已经达到最大值积分I不再增加
         && (funcCode.code.pidIAttribute/10))     // PID积分输出到有限值时停止积分
        || ((diFunc.f2.bit.pidPauseI) && (funcCode.code.pidIAttribute%10))) // PID积分暂停
    {
        deltaI = 0;
        p->deltaIRem = 0;
    }
    else
    {
        p->deltaIRem += (int64)Ki * pidError;           // Qi + 15
        deltaI = p->deltaIRem >> p->Qi;   // Q15
        p->deltaIRem = p->deltaIRem - (deltaI << p->Qi);
    }

    deltaError = pidError - 2*p->error + p->error1;
    p->deltaDRem += (int64)Kd * (deltaError);
    deltaD = p->deltaDRem >> p->Qd;
    p->deltaDRem = p->deltaDRem - (deltaD << p->Qd);
    
// 微分限幅
    if((deltaD > 0) && (deltaD > p->pidDLimit))
    {
        deltaD = p->pidDLimit;
        p->deltaDRem = 0;  
    }
    else if ((deltaD < 0) && (deltaD < (-p->pidDLimit)))
    {
        deltaD = (-p->pidDLimit);
        p->deltaDRem = 0;
    }

    if(pidCalcDeal == PID_CALC_INIT_U0)
   	{  // U0 = Kp*e(0)+Ki*e(0)
        p->out += (((int64)Kp * pidError) >> p->Qp) + (((int64)Ki * pidError) >> p->Qi);
		pidCalcDeal = PID_CALC_TRUE;
	}
// 计算delta
    if(pidCalcDeal == PID_CALC_TRUE)
 	{
        p->delta = deltaP + deltaI + deltaD; // Q15
    }
#if 0
    p->delta += p->deltaRemainder;      // 上一次的delta限幅余值
    if (p->delta > p->deltaMax)
    {
        p->deltaRemainder = p->delta - p->deltaMax;  // 保留delta限幅余值，下次计算使用
        p->delta = p->deltaMax;
    }
    else if (p->delta < p->deltaMin)
    {
        p->deltaRemainder = p->delta - p->deltaMin;
        p->delta = p->deltaMin;
    }
    else
    {
        p->deltaRemainder = 0;
    }
#endif

// 计算out, out限幅
    if (frqFlag.bit.frqSetLimit && frqFlag.bit.frqPidFlag)   // 频率PID且频率限幅
    {
        if (((frqAimTmp > 0) && (p->delta < 0))
            || ((frqAimTmp < 0) && (p->delta > 0))
            )
        {
            p->out += p->delta;
        }
        else
        {
            p->deltaRemainder = 0;   // 清两次输出偏差限幅余值
            p->deltaPRem = 0;            // 清比例余值
            p->deltaDRem = 0;            // 清微分限幅余值
        }
    }
    else
    {
        p->out += p->delta;
    }

    if (p->out > p->outMax)
    {
        p->out = p->outMax;
        p->deltaRemainder = 0;   // 清两次输出偏差限幅余值
        p->deltaPRem = 0;            // 清比例余值
        p->deltaDRem = 0;            // 清微分限幅余值
    }
    else if (p->out < p->outMin)
    {
        p->out = p->outMin;
        p->deltaRemainder = 0;   // 清两次输出偏差限幅余值
        p->deltaPRem = 0;            // 清比例余值
        p->deltaDRem = 0;            // 清微分限幅余值
    }

    DeltaPDisp = deltaP;
    DeltaIDisp = deltaI;
    DeltaDDisp = deltaD;
#endif
}

#define STATIC_KP_VALUE 100   // 作用于积分、微分的比例项固定为10.0 
Uint16 minTmp;
Uint16 lowerTmp;
Uint16 frqxyxTmp;

Uint16 minFlag;
//=====================================================================
//
// PID参数更新
//
// 保证:
//      2^9  < p->Kp <= 2^14， 1000 - 100.0% - 1 - 1*2^Qp
//      2^11 < p->Ki <= 2^15,
//
//=====================================================================
LOCALF void UpdatePidFuncPara(PID_FUNC *p)
{
    Uint32 Kp = (((Uint32)funcCode.code.pidKp << 16) / 1000) << 8; // Q24，此时要保证不大于2^29，即pidKp<=2^5*1000
    Uint32 Ki = (((((((Uint32)funcCode.code.pidKp * PID_CALC_PERIOD) << 14) / 100) << 6) / 100) << 6)
            / funcCode.code.pidTi; // Q26，此时要保证小于2^30
//    Uint32 Kd;
    Uint16 Qp, Qi, Qd;
    int32 outMax;
    int32 outMin;
#if 1
// Kp，Qp
    Qp = 24;
    while (Kp >= 1L << 15)
    {
        Kp >>= 1;
        Qp--;
    }

    if (p->Qp > Qp)             // 如果新的Qp变小了，则之前的余值清零
    {
        p->deltaPRem = 0;
    }
    p->Qp = Qp;
    p->Kp = Kp;

// Ki，Qi
    Qi = 26;
    while (Ki >= 1L << 16)
    {
        Ki >>= 1;
        Qi--;
    }

    if (p->Qi > Qi)
    {
        p->deltaIRem = 0;
    }
    p->Qi = Qi;
    p->Ki = Ki;

// Kd, Qd
    Qd = 12;
    p->Kd = (((((Uint32)funcCode.code.pidKp * funcCode.code.pidTd) << 6) / 5) << 3) / (25*PID_CALC_PERIOD); // Q12
    p->Qd = Qd;

    // errorDead，限制最小死区为 PID_ERROR_DEAD
    p->errorDead = ((Uint32)funcCode.code.pidErrMin << 15) / 1000;// + PID_ERROR_DEAD; // Q15

    // 1. 当反馈为AI时，499.5mv-500.4999mv之间都会显示为500mv。
    // 为了在显示500mv时，PID结算结果不抖动/变化，作了如此处理。
    // 2. 设置ref = 50%，pidErrMin = 20%，要在AI反馈为300mv/700mv时，PID输出没有变化，做了如此处理。
    if (funcCode.code.pidFdbSrc <= FUNCCODE_pidFdbSrc_AI1_SUB_AI2)
    {
        p->errorDead += PID_ERROR_DEAD;  // 仅仅在AI时才有PID死区
    }

#if 0
    if (frqFlag.bit.frqPidFlag)     // 频率PID
    {

    // PID输出的上限
    // PID正作用，反馈低于给定，频率到上限频率；再反馈高于给定。立即从上限频率向下运行。
        outMax = 0x7FFF;            // 100.0%
        if (upperFrq < maxFrq)      // 上限频率
        {
            outMax = ((int32)upperFrq << 15) / maxFrq;
        }
        
        // PID为辅助频率且上限频率超过辅助频率范围
        if ((funcCode.code.frqYSrc == FUNCCODE_frqXySrc_PID)
           && (upperFrq > frqAiPu))   
        {
            outMax = ((int32)frqAiPu << 15) / maxFrq;
        }
        
        p->outMax = outMax;

    // PID输出的下限
    // PID正作用，反馈高于给定，频率到下限频率；再反馈低于给定。立即从下限频率向上运行。
        outMin = lowerFrq;
        if ((0 == funcCode.code.reverseCutOffFrq)// 禁止反转或截止频率为0，PID下限为下限频率
			||(funcCode.code.antiReverseRun == 1)
		   ) //反转截止频率为0或者禁止反转 即((FA-08=0&&F8-13=0)||(FA-08=0&&F8-13=1)||(FA-08!=0&&F8-13=1)) 
        {
            //;
            // 主+辅 & PID为辅   by modfied yz1990 2014-09-25
            if ((funcCode.code.frqYSrc == FUNCCODE_frqXySrc_PID)         
                && (frqFlag.bit.comp)
                )
           	{
           	    if((frqXy.x1 >= 0)&&(frqXy.x1 <= ((int32)lowerFrq)))
           	   	{
                    outMin = (int32)lowerFrq - frqXy.x1;
					//outMin = 200;
					minTmp = outMin;
					lowerTmp = lowerFrq;
					frqxyxTmp = frqXy.x1;
					minFlag = 1;
				}
				else
				{
                    outMin = 0;
					minTmp = outMin;
					lowerTmp = lowerFrq;
					frqxyxTmp = frqXy.x1;

					minFlag = 2;

				}
			}
        }
        // 截止频率不为0，PID下限: 下限频率和反转截止频率的max，再取负。
        else
        {
            //if (outMin < funcCode.code.reverseCutOffFrq)
            {
                outMin = funcCode.code.reverseCutOffFrq;
            }
#if 0
            // PID为辅助频率且上限频率超过辅助频率范围
            if ((funcCode.code.frqYSrc == FUNCCODE_frqXySrc_PID)
                && (outMin > frqAiPu))
            {
                outMin = frqAiPu;
            }
#endif
            outMin = -outMin;
        }
#if 0
		// 主+辅 & PID为辅  PID反向截止频率FA-08不生效，和安川一样的处理 by modfied yz1990 2014-07-24
        if ((funcCode.code.frqYSrc == FUNCCODE_frqXySrc_PID)         
             && frqFlag.bit.comp
            )
        {
             outMin = frqAiPu;
			 outMin = -outMin;
        }	
#endif		
        p->outMin = (outMin << 15) / maxFrq;

    }
    else
    {
        p->outMax = 0x7FFF;
        p->outMin = 0;           // 非频率PID,PID下限为0
    }

#endif
// 从新规划纯PID/ 主 + PID 时PID的上下限(其它组合和纯PID一样，不一定合理，
// 只能使用纯PID/主+PID(已能满足应用),说明书只介绍此两种)  by modfied yz1990 2015-05-25

    if (frqFlag.bit.frqPidFlag)     // 频率PID
    {

    // PID输出的上限
    // PID正作用，反馈低于给定，频率到上限频率；再反馈高于给定。立即从上限频率向下运行。
        outMax = 0x7FFF;            // 100.0%
        if (upperFrq < maxFrq)      // 上限频率
        {
            outMax = ((int32)upperFrq << 15) / maxFrq;
        }
        
        // PID为辅助频率且上限频率超过辅助频率范围
        if ((funcCode.code.frqYSrc == FUNCCODE_frqXySrc_PID)
           && (upperFrq > frqAiPu))   
        {
            outMax = ((int32)frqAiPu << 15) / maxFrq;
        }
        
        p->outMax = outMax;

    // PID输出的下限
    // PID正作用，反馈高于给定，频率到下限频率；再反馈低于给定。立即从下限频率向上运行。
        outMin = lowerFrq;
        if ((0 == funcCode.code.reverseCutOffFrq)// 禁止反转或截止频率为0，PID下限为下限频率
			||(funcCode.code.antiReverseRun == 1)
		   ) //反转截止频率为0或者禁止反转 即((FA-08=0&&F8-13=0)||(FA-08=0&&F8-13=1)||(FA-08!=0&&F8-13=1)) 
        {
            //;
            // 主+辅 & PID为辅   by modfied yz1990 2014-09-25
            if ((funcCode.code.frqYSrc == FUNCCODE_frqXySrc_PID)         
                && (frqFlag.bit.comp)
                )
           	{   
                outMin = -(frqXy.x1 - lowerFrq);//
			}
        }
        // 截止频率不为0，此时，下限频率一般为0，否则PID下限:>=lowerFrq,<=-lowerFrq;当lowerFrq=0时，PID下限 
        else //反转截止频率不为0且不禁止反转(FA-08!=0&&F8-13=0) 
        {
            //if (outMin < funcCode.code.reverseCutOffFrq)
            {
                outMin = -(int32)funcCode.code.reverseCutOffFrq;
            }

			// 主+辅 & PID为辅   by modfied yz1990 2015-05-25
			if ((funcCode.code.frqYSrc == FUNCCODE_frqXySrc_PID)         
                && (frqFlag.bit.comp)
                )
           	{   
                outMin = -(frqXy.x1 + funcCode.code.reverseCutOffFrq);//此时FA-08!=0

				if(lowerFrq > funcCode.code.reverseCutOffFrq)
               	{
                    outMin = -(frqXy.x1 + lowerFrq);//此时FA-08!=0
				}
			}

#if 0
            // PID为辅助频率且下限频率超过辅助频率范围
            if ((funcCode.code.frqYSrc == FUNCCODE_frqXySrc_PID)
                && (outMin > frqAiPu))
            {
                outMin = frqAiPu;
            }
#endif
            //outMin = -outMin;
        }
        p->outMin = (outMin << 15) / maxFrq;

    }
    else
    {
        p->outMax = 0x7FFF;
        p->outMin = 0;           // 非频率PID,PID下限为0
    }


// PID的输出最小值
//    p->outMin = -(int32)funcCode.code.reverseCutOffFrq * 0x7FFF / maxFrq; // 反转截止频率

// 微分限幅
    p->pidDLimit =  (Uint32)funcCode.code.pidDLimit * 53687>>14;  //0x7FFF / 10000;

#if 0
// PID输出偏差的最大值，最小值
    p->deltaMax = (Uint32)funcCode.code.pidOutDeltaMax * 53687>>14;  //0x7FFF / 10000;
    p->deltaMin = -(int32)((Uint32)funcCode.code.pidOutDeltaMin * 53687>>14);  //0x7FFF / 10000;
#endif

    // Kp2
    if (Qp >= 16)
    {
        p->Kp2 = (((Uint32)funcCode.code.pidKp2 << 16) / 1000) << (Qp - 16);
    }
    else
    {
         p->Kp2 = (((Uint32)funcCode.code.pidKp2 << Qp) / 1000);
    }

    // Ki2
    if (Qi >= 20)
    {
        //p->Ki2 = (((((((Uint32)funcCode.code.pidKp2 * PID_CALC_PERIOD) << 14) / 100) << 6) / 100) << (Qi - 20)) / funcCode.code.pidTi2; // Q26，此时要保证小于2^30
        p->Ki2 = (((((((Uint32)funcCode.code.pidKp2 * PID_CALC_PERIOD) << 14) *327>>15) << 6) *327>>15) << (Qi - 20)) / funcCode.code.pidTi2; // Q26，此时要保证小于2^30

	}
    else if(Qi >= 14)
    {
        //p->Ki2 = ((((((Uint32)funcCode.code.pidKp2 * PID_CALC_PERIOD) << 14) / 100) << (Qi - 14)) / 100) / funcCode.code.pidTi2; // Q26，此时要保证小于2^30
        p->Ki2 = ((((((Uint32)funcCode.code.pidKp2 * PID_CALC_PERIOD) << 14) *327>>15) << (Qi - 14)) *327>>15) / funcCode.code.pidTi2; // Q26，此时要保证小于2^30

	}
    else
    {
       p->Ki2 = ((((Uint32)funcCode.code.pidKp2 * PID_CALC_PERIOD) << Qi) / 10000) / funcCode.code.pidTi2; // Q26，此时要保证小于2^30 
    }

    // Kd2
    p->Kd2 = (((((Uint32)funcCode.code.pidKp2 * funcCode.code.pidTd2) << 6) / 5) << 3) / (25*PID_CALC_PERIOD); // Q12

    // PID参数切换偏差
	//p->errorSmall = (int32)funcCode.code.pidParaChgDelta1 * 0x7FFF / 1000;
    //p->errorBig   = (int32)funcCode.code.pidParaChgDelta2 * 0x7FFF / 1000;
    p->errorSmall = (int32)funcCode.code.pidParaChgDelta1 * 2097 >>6;
    p->errorBig   = (int32)funcCode.code.pidParaChgDelta2 * 2097 >>6;
#endif
}



//=====================================================================
//
// PID给定、反馈更新
//
// 输出：p->ref, Q15, 100.0% -- 2^15
//       p->fdb, Q15, 100.0% -- 2^15
//
//=====================================================================
LOCALF void UpdatePidFuncRefFdb(PID_FUNC *p)
{
    int32 aimValue;
    int32 fdb;
    int16 i;
#if 1
// PID给定
    switch (funcCode.code.pidSetSrc) // PID给定源
    {
        // 功能码设定
        case FUNCCODE_pidSetSrc_FC:
            aimValue = (int32)(int16)funcCode.code.pidSet * 2097 >> 6;  // 0x7fff/1000 (Q15)
            break;

        // AI1\AI2\AI3
        // AI设定本身为Q15
        case FUNCCODE_pidSetSrc_AI1:
        case FUNCCODE_pidSetSrc_AI2:
        case FUNCCODE_pidSetSrc_AI3:
            i = funcCode.code.pidSetSrc - FUNCCODE_pidSetSrc_AI1;
            aimValue = aiDeal[i].set;
            break;

        // 脉冲输入
        // 本身为Q15
        case FUNCCODE_pidSetSrc_PULSE:
            aimValue = pulseInSet;
            break;

        // 通讯
        case FUNCCODE_pidSetSrc_COMM:
            aimValue = (int32)(int16)funcCode.code.frqComm * 13421 >> 12;  // 0x7FFF/10000
            break;

        // 多段速设定
        case FUNCCODE_pidSetSrc_MULTI_SET:
            aimValue = UpdateMultiSetFrq(diFunc.f1.bit.multiSet);
            aimValue = aimValue * 0x7fff / (int32)maxFrq;               // PID的100%对应最大频率
            break;
            
        default:
            break;
    }


#if 0  // 给定可以小于0
    // 给定不能小于0
    if (aimValue < 0)
    {
        aimValue = 0;
    }
#endif

// PID给定变化时间
    pidFuncRef.aimValue = aimValue;
    pidFuncRef.tickerAll = (Uint32)funcCode.code.pidSetChangeTime * (TIME_UNIT_PID_SET_CHANGE / PID_CALC_PERIOD);
    pidFuncRef.maxValue = 0x7fff;
    pidFuncRef.curValue = p->ref;
    pidFuncRef.calc(&pidFuncRef);
    p->ref = pidFuncRef.curValue;

    // PID反馈源
    switch (funcCode.code.pidFdbSrc) // PID反馈值
    {
        // AI1\AI2\AI3
        case FUNCCODE_pidFdbSrc_AI1:
        case FUNCCODE_pidFdbSrc_AI2:
        case FUNCCODE_pidFdbSrc_AI3:
            fdb = aiDeal[funcCode.code.pidFdbSrc - FUNCCODE_pidFdbSrc_AI1].set;
            break;

        // AI1-AI2
        case FUNCCODE_pidFdbSrc_AI1_SUB_AI2:    // 即使 |aiDeal[0].set - aiDeal[1].set| > 1也没有问题。
            fdb = (int32)aiDeal[0].set - aiDeal[1].set;
            break;

        // 脉冲输入
        case FUNCCODE_pidFdbSrc_PULSE:
            fdb = pulseInSet;
            break;

        // 通讯
        case FUNCCODE_pidFdbSrc_COMM:
            fdb = (int32)(int16)funcCode.code.frqComm * 13421 >> 12;  // 0x7FFF/10000
            break;

        // AI1+AI2
        case FUNCCODE_pidFdbSrc_AI1_ADD_AI2:
            fdb = (int32)aiDeal[0].set + aiDeal[1].set;
            break;

        // MAX(AI1,AI2)
        case FUNCCODE_pidFdbSrc_MAX_AI:
            fdb = GetMax(ABS_INT32(aiDeal[0].set), ABS_INT32(aiDeal[1].set));
            break;

        // MIN(AI1,AI2)
        case FUNCCODE_pidFdbSrc_MIN_AI:
            fdb = GetMin(ABS_INT32(aiDeal[0].set), ABS_INT32(aiDeal[1].set));
            break;

        default:
            break;
    }

#if 0   // 反馈可以小于0   
    // 反馈不能小于0
    if (fdb < 0)
    {
        fdb = 0;
    }
#endif    
    // 反馈不能大于100%
    if (fdb > 0x7fff)
    {
        fdb = 0x7fff;
    }

    // PID反馈丢失检测
    if (PID_CALC_FLAG_CALC_TRUE == pidCalcFlag)
    {
        // 反馈丢失检测值不为0且反馈小于检测值且处于运行状态
        if ((funcCode.code.pidFdbLoseDetect) && (runFlag.bit.run) && (fdb < (int32)funcCode.code.pidFdbLoseDetect * 0x7fff / 1000))
        {
            if (pidFdbLoseTicker < (int32)funcCode.code.pidFdbLoseDetectTime * (TIME_UNIT_PID_FDB_LOSE / PID_CALC_PERIOD))
            {
                pidFdbLoseTicker++;
            }
            else
            {
                bPidFdbLose = 1;

                // 运行时PID反馈丢失
                errorOther = ERROR_FDB_LOSE;
            }
        }
        else
        {
            bPidFdbLose = 0;
            pidFdbLoseTicker = 0;
        }
    }
    else
    {
        pidFdbLoseTicker = 0;
    }
        

// PID反馈滤波
    pidLpf[PID_LPF_FDB].t = (int32)funcCode.code.pidFdbLpfTime * (TIME_UNIT_PID_FILTER / PID_CALC_PERIOD); // 滤波
    pidLpf[PID_LPF_FDB].in = fdb;
    pidLpf[PID_LPF_FDB].calc(&pidLpf[PID_LPF_FDB]);
    p->fdb = pidLpf[PID_LPF_FDB].out;
#endif
}

#elif 1

int32 FrqPidSetDeal(void){}
void PidFuncCalc(PID_FUNC *p){}

#endif






