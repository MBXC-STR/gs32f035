/****************************************************************
文件说明：基于TMS320F280X DSP的电机控制软件
          深圳市汇川技术股份有限公司
文件版本： 
最新更新： 888
****************************************************************/
#include "MotorInclude.h"

extern void ADCOverInterrupt(void);
extern void PWMZeroInterrupt(void);
extern void HardWareErrorDeal(void);

extern void Main2msMotorA(void);
extern void Main2msMotorB(void);
extern void Main2msMotorC(void);
extern void Main2msMotorD(void);

extern void Main05msMotor(void);     
//extern void Main0msMotor(void);

//extern void Main2msFunction(void);
//extern void Main05msFunction(void);     
extern void Main05msFunctionA(void);
extern void Main05msFunctionB(void);
extern void Main05msFunctionC(void);
extern void Main05msFunctionD(void);
extern void Main0msFunction(void);
/***************************************************************
-------------------------主程序部分-----------------------------
****************************************************************/
void main(void)
{					
	Ulong m_BaseTime,m_DetaTime;
	Uint  m_LoopFlag;
   
   	InitSysCtrl();						// Step 1. Initialize System Control
   
   	InitInterrupt();					// Step 2. Initialize Interrupt service program:
   
   	InitPeripherals(); 					// Step 3. Initialize all the Device Peripherals:
   
   	InitForMotorApp();					// Step 4. User specific code
   	InitForFunctionApp();

	EnableDog();                  // d
	SetInterruptEnable();				// Step 5. enable interrupts:
   	EINT;   							    
   	ERTM;   							    

	m_LoopFlag = 2;
	m_BaseTime = GetTime();
    
  	while(1)							// Step 6. User Application function:
   	{
		m_DetaTime = m_BaseTime - GetTime();		
		if(m_DetaTime >= C_TIME_05MS)	//判断0.5MS周期
        {            
			m_LoopFlag ++;			
			m_BaseTime -= C_TIME_05MS;
 			KickDog();

            Main05msMotor();                        // 驱动0.5ms程序          
                    
			if((m_LoopFlag & 0x03) == 0)            // prA
			{         
                Main05msFunctionA();
                Main05msFunctionB();					
			}
            else if((m_LoopFlag & 0x03) == 1)       // prB
            {                                
                //Main05msFunctionB();
                Main05msFunctionC();
                //Main05msFunctionD();	
            }
			else if((m_LoopFlag & 0x03) == 2)       // prC
			{    

                Main05msFunctionD();
                Main2msMotorA(); 

			}
            else if((m_LoopFlag & 0x03) == 3)       // prD
            {             
                Main2msMotorB(); 
                Main2msMotorC(); 
				Main2msMotorD(); 	
            }

            // 计算cpu忙碌系数1
           // gCpuTime.Det05msClk = __IQsat(m_DetaTime, 65535, C_TIME_05MS);
        }

		Main0msFunction();				//不等待循环－执行功能部分程序
//		Main0msMotor();					//不等待循环－执行电机控制部分程序
   	}
} 


/***************************************************************
-----------------------中断程序部分-----------------------------
****************************************************************/
/***************************************************************
	EPWM的周期中断，约30us
****************************************************************/
__interrupt void ADC_Over_isr(void)
{
    EALLOW;             //28035改为EALLOW保护
    ADC_CLEAR_INT_FLAG;
    EDIS;
	EINT;
	//gCpuTime.ADCIntBase = GetTime();
	//ADCOverInterrupt();
	gMainCmd.pADCIsr();                         /*修改为函数指针方式执行ADC结束中断程序，默认ADCEndIsr()*/					
//	gCpuTime.ADCInt = gCpuTime.ADCIntBase - GetTime();
	DINT;
    EALLOW;
    ADC_RESET_SEQUENCE;
   	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;	// Acknowledge this interrupt
    EDIS;
}
/***************************************************************
	EPWM的过流中断，对硬件过流信号处理
****************************************************************/
__interrupt void EPWM1_TZ_isr(void)
{
	DisableDrive();								//首先封锁输出
	HardWareErrorDeal();					    //处理硬件故障－电机控制模块处理
                // 
   	PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;	    // Acknowledge this interrupt
}

/***************************************************************
	CBC触发的TZ中断，中断中强制关闭驱动使能信号
****************************************************************/
__interrupt void EPWM2_TZ_isr(void)
{               
	DisableDrive();								//首先封锁输出,周期中断中开启
    gCBCProtect.CBCIntFlag = 1;  
   	PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;	    // Acknowledge this interrupt
}

/*----------------------------END---------------------------------
****************************************************************/
