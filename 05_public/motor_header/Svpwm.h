/***********************************Inovance***********************************
功能描述（Function Description）: SVPWM模块头文件，定义SVPWM模块中所用到结构体
数据类型和相关宏定义，并对Svpwm.c文件中的结构体变量和函数进行声明
最后修改日期（Date）：2011-04-28
修改日志（History）:（以下记录为第一次转测试后，开始记录）
	作者 		时间 		更改说明
1 	xx 		xxxxx 		xxxxxxx
2 	yy 		yyyyy 		yyyyyyy
************************************Inovance***********************************/
#ifndef SVPWM_INCLUDE_H
#define  SVPWM_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "DataTypeDef.h"
//#include "SubPrgInclude.h"

/* Private typedef -----------------------------------------------------------*/
/*typedef struct PWM_OUT_STRUCT_DEF {
	u16		Phase;			//输出相位
	s16		Ratio;			//调制系数
	u16		PWMPrd;			//当前生效的PWM周期
	u16     SoftPWMFlag;    //随机PWM是否生效的选择
	u16		PWMModle;		//CPWM/DPWM选择
	u16		OverModule;		//过调制选择
	u16		MinPwmCtl;		//是否进行窄脉冲控制选择
	s16		DeadBand;		//死区时间
	u16     ZeroLengthPhase;//记录DPWM调制下哪一相为全脉宽
	s32		U;				//U相PWM输出比较值
	s32		V;				//V相PWM输出比较值
	s32		W;				//W相PWM输出比较值
	
	s32		USet;			//U相PWM输出比较值
	s32		VSet;			//V相PWM输出比较值
	s32		WSet;			//W相PWM输出比较值
}PWM_OUT_STRUCT;	        //作为PWM输出的结构

*/
/* Private define ------------------------------------------------------------*/
//extern PWM_OUT_STRUCT   gPWM;

/* Private macro -------------------------------------------------------------*/
//#define MODLE_CPWM		    0		//连续调制
//#define MODLE_DPWM		    1		//离散调制
#define OVER_MODULE_CANCEL	0		//过调制不生效
#define OVER_MODULE			1		//过调制生效

#define NARROW_PWM_CANCEL	0		//取消窄脉冲控制
#define NARROW_PWM			1		//窄脉冲控制

#define	ZERO_VECTOR_U		0		//DPWM调制时，U相发全脉宽或零脉宽
#define	ZERO_VECTOR_V		1		//DPWM调制时，V相发全脉宽或零脉宽
#define	ZERO_VECTOR_W		2		//DPWM调制时，W相发全脉宽或零脉宽
//#define	ZERO_VECTOR_NONE	100		//没有发全脉宽或零脉宽的相

/* Private function prototypes -----------------------------------------------*/
extern void OutPutPWM(void);
extern void DeadBandComp(void);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

/******************************* END OF FILE***********************************/

