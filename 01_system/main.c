/****************************************************************
锟侥硷拷说锟斤拷锟斤拷锟斤拷锟斤拷TMS320F280X DSP锟侥碉拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
          锟斤拷锟斤拷锟叫汇川锟斤拷锟斤拷锟缴凤拷锟斤拷锟睫癸拷司
锟侥硷拷锟芥本锟斤拷
锟斤拷锟铰革拷锟铰ｏ拷 888
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
-------------------------锟斤拷锟斤拷锟津部凤拷-----------------------------
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

//	EnableDog();                  // d
	SetInterruptEnable();				// Step 5. enable interrupts:
   	EINT;   							    
   	ERTM;   							    

	m_LoopFlag = 2;
	m_BaseTime = GetTime();
    
  	while(1)							// Step 6. User Application function:
   	{
		m_DetaTime = m_BaseTime - GetTime();		
		if(m_DetaTime >= C_TIME_05MS)
        {            
			m_LoopFlag ++;			
			m_BaseTime -= C_TIME_05MS;
 			KickDog();

            Main05msMotor();
                    
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

            // 锟斤拷锟斤拷cpu忙碌系锟斤拷1
           // gCpuTime.Det05msClk = __IQsat(m_DetaTime, 65535, C_TIME_05MS);
        }

		Main0msFunction();				//锟斤拷锟饺达拷循锟斤拷锟斤拷执锟叫癸拷锟杰诧拷锟街筹拷锟斤拷
//		Main0msMotor();					//锟斤拷锟饺达拷循锟斤拷锟斤拷执锟叫碉拷锟斤拷锟斤拷撇锟斤拷殖锟斤拷锟�
   	}
} 


/***************************************************************
-----------------------锟叫断筹拷锟津部凤拷-----------------------------
****************************************************************/
/***************************************************************
	EPWM锟斤拷锟斤拷锟斤拷锟叫断ｏ拷约30us
****************************************************************/
#ifdef TARGET_GS32
__interrupt void ADC_Over_isr(void)
#else
interrupt void ADC_Over_isr(void)
#endif
{
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
    EALLOW;             //28035为EALLOW
    ADC_CLEAR_INT_FLAG;
    EDIS;
	EINT;
	//gCpuTime.ADCIntBase = GetTime();
	//ADCOverInterrupt();
	gMainCmd.pADCIsr();                         /*薷为指敕绞街碅DC卸铣默ADCEndIsr()*/
//	gCpuTime.ADCInt = gCpuTime.ADCIntBase - GetTime();
	DINT;
    EALLOW;
    ADC_RESET_SEQUENCE;
#ifdef TARGET_GS32
    
#else
   	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;	// Acknowledge this interrupt
#endif
	EDIS;

#ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}
/***************************************************************
	EPWM锟侥癸拷锟斤拷锟叫断ｏ拷锟斤拷硬锟斤拷锟斤拷锟斤拷锟脚号达拷锟斤拷
****************************************************************/
#ifdef TARGET_GS32
__interrupt void EPWM1_TZ_isr(void)
#else
interrupt void EPWM1_TZ_isr(void)
#endif
{
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
	DisableDrive();								//锟斤拷锟饺凤拷锟斤拷锟斤拷锟�
	HardWareErrorDeal();					    //锟斤拷锟斤拷硬锟斤拷锟斤拷锟较ｏ拷锟斤拷锟斤拷锟斤拷锟侥ｏ拷榇︼拷锟�
                // 
#ifdef TARGET_GS32
    
#else
   	PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;	    // Acknowledge this interrupt
#endif
#ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}

/***************************************************************
	CBC锟斤拷锟斤拷锟斤拷TZ锟叫断ｏ拷锟叫讹拷锟斤拷强锟狡关憋拷锟斤拷锟斤拷使锟斤拷锟脚猴拷
****************************************************************/
#ifdef TARGET_GS32
__interrupt void EPWM2_TZ_isr(void)
#else
interrupt void EPWM2_TZ_isr(void)
#endif
{               
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
	DisableDrive();								//锟斤拷锟饺凤拷锟斤拷锟斤拷锟�,锟斤拷锟斤拷锟叫讹拷锟叫匡拷锟斤拷
    gCBCProtect.CBCIntFlag = 1;
#ifdef TARGET_GS32
    
#else  
   	PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;	    // Acknowledge this interrupt
#endif
#ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}

/*----------------------------END---------------------------------
****************************************************************/
