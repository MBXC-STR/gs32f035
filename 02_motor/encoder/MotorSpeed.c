/***********************************Inovance***********************************
功能描述（Function Description）:编码器反馈文件，包括位置反馈和速度反馈的处理
    1) M法测速
    2) T法测速
    3) 旋转变压器测速
最后修改日期（Date）：
修改日志（History）:（以下记录为第一次转测试后，开始记录）
	作者 		时间 		更改说明
1 	xx 		xxxxx 		xxxxxxx
2 	yy 		yyyyy 		yyyyyyy
************************************Inovance***********************************/

/* Includes ------------------------------------------------------------------*/
#include "MotorInclude.h"
#include "MotorEncoder.h"

/* Private variables ---------------------------------------------------------*/
PG_DATA_STRUCT			gPGData;
ROTOR_SPEED_STRUCT		gRotorSpeed;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
//void    ResetPGType(void);
void    ResetSpeedGetReg(void);
void    GetFeedRotorSpeed(void);
void    GetFeedSpeedQEP(void);
s32     CalFeedSpeed(s16 DetaPos, u32 DetaTime);
void    QEPGetRealPos(void);              
void    ReadRTPos(void);
void    CalRTSpeed(void);
extern  u16 GetRotorTransPos();
/****************************************************************
	通用滤波程序(Long格式)
	LastOne     上一拍滤波后的数据
	Input       本拍采样数据
	FilterTime  滤波时间（ms单位）
算法：OutPut = LastOne + (Input - LastOne) * Coff
      Coff是必须小于32的值。
*****************************************************************/
s32 FilterL(s32 LastOne, s32 Input, s32 Coff)
{
	s32   m_Deta,m_Add;
	
	m_Deta = Input - LastOne;
	m_Add = ((s64)m_Deta * (s64)Coff)>>15;
	if((m_Add == 0) && (m_Deta != 0))
	{
		m_Add = (m_Deta > 0)?1:-1;
	}
	return (LastOne + m_Add);	
}

/*******************************************************************************
* Function Name  : 读取QEP捕获计数器锁存值
* Description    :  编写人于兆凯（1918）
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
u16 GetQepTimLat(void)
{
	return((*EQepRegs).QCTMRLAT);
}

/*******************************************************************************
* Function Name  : 读取QEP捕获计数器周期锁存值
* Description    :  编写人于兆凯（1918）
* Input          : 
* Output         : None
* Return         : None
*******************************************************************************/
u16 GetQepTimPrdLat(void)
{
	return((*EQepRegs).QCPRDLAT);
}
/*******************************************************************************
* Function Name  : 初始化测速用的寄存器
* Description    : 
* Input          : SetpTime 以主定制器为计数的测速间隔，如2ms间隔，系统时钟100MHz，设置为200000
                   MaxQepCnt QEP计数器最大值，32位计数器为(1UL<<32),16位计数器为(1UL<<16)
* Output         : None
* Return         : None
*******************************************************************************/
void ResetSpeedGetReg(void)
{
    gRotorSpeed.Pos              = GetQepCnt();
    gRotorSpeed.Time             = GetTime();
    gRotorSpeed.DetaPos          = 0;
	gRotorSpeed.CapTimeLast      = 0;			//对捕获单元前一次锁存值进行初始化
    gRotorSpeed.DetaTimeAdd      = C_TIME_4MS;
    gRotorSpeed.SpeedFeed        = 0;
    gRotorSpeed.PrescaFlag       = 0;
    gRotorSpeed.ChangeFlag       = 0;
}

/*******************************************************************************
* Function Name  : 编码器速度反馈函数，区分不同编码器类型
* Description    : 
* Input          : 
* Output         : 
* Return         : None
*******************************************************************************/
void GetFeedRotorSpeed(void)
{
    s32 m_tempLong,m_Data,m_SpeedFeed;

    if(gPGData.PGType == PG_TYPE_RESOLVER)
    {	    
	    // 模拟一个Z信号， 用于带载辨识；产生条件是: 连续两拍负角度，当前拍正角度, 同时满足4ms 间隔时间
	    if(gMainStatus.RunStep == STATUS_GET_PAR)
	    {        
	        if(((s16)gRotorTrans.RtorBuffer > 0) &&
	            ((s16)gRotorTrans.SimuZBack <= 0) &&
	            ((s16)gRotorTrans.SimuZBack2 <= 0) &&
	            (gIPMZero.zFilterCnt == 4))
	        {
				gIPMPos.ZSigNumSet++;
			    gIPMZero.zFilterCnt = 0;
	        }
	        gRotorTrans.SimuZBack2 = gRotorTrans.SimuZBack;
	        gRotorTrans.SimuZBack = gRotorTrans.RtorBuffer;
	    } 
	       
	    if(gVCPar.VCSpeedFilter <= 10)    // 旋变滤波时间比较长
		{
		    m_Data =  163840L / (20 + 3*gVCPar.VCSpeedFilter);  
		}
		else
		{
		    m_Data =  163840L / (40 + gVCPar.VCSpeedFilter);
		}	
		gAsr.KPLowCoff = 10;	                                     	
    }
    else
    {    	
    	GetFeedSpeedQEP();                                /*ABZ测速程序*/
        m_Data =  163840L / (10 + 3*gVCPar.VCSpeedFilter);
		gAsr.KPLowCoff = 15;
    } 
	gRotorSpeed.SpeedFeedFilter = FilterL(gRotorSpeed.SpeedFeedFilter,gRotorSpeed.SpeedFeed,m_Data);  
    m_SpeedFeed = gRotorSpeed.SpeedFeedFilter>>9;
	gRotorSpeed.SpeedFeedQ12 = (s16)__IQsat(m_SpeedFeed,32767,-32767);
	m_tempLong = (long)gRotorSpeed.SpeedFeedQ12 * gRotorSpeed.TransRatio;
    m_SpeedFeed= (llong)m_tempLong * 16777 >> 24;
	gRotorSpeed.SpeedEncoder = (s16)__IQsat(m_SpeedFeed,32767,-32767); 	  
}
/*******************************************************************************
* Function Name  : 捕获方式测速，修正了整数脉冲数的准确时间
* Description    : 仍然存在的问题:
                    1)无法捕获4个脉冲的准确时间
                    2)低速下，测速周期内无法捕捉到脉冲，如何计算速度
                    3)测速时刻和速度闭环、生效时刻有时间差，需要考虑速度变化
* Input          : 
* Output         : gRotorSpeed.MSpeed
* Return         : None
*******************************************************************************/
void GetFeedSpeedQEP(void)
{
    s32 m_Speed;
    u32 m_QEPCnt;              //当前位置
    u32 m_Time;                //当前时间
    u16 m_CaptureTime;         //捕获单元计数器的锁存值
	u16 m_CapturePeriod;       //捕获单元锁存值
	u16 m_Cnt;	

    m_Cnt = 0;
    do
    {
        DINT;
        m_QEPCnt        = GetQepCnt();
        m_Time          = GetTime();
        m_CaptureTime   = GetQepTimLat(); 
        m_CapturePeriod = GetQepTimPrdLat();
        EINT;
        if((m_Cnt++) > 4)   break;                          /*避免死循环*/
    }while((0 == m_CaptureTime) || (m_CaptureTime == m_CapturePeriod));

    gRotorSpeed.DetaPos   = (s16)(m_QEPCnt - gRotorSpeed.Pos); 
    gRotorSpeed.DetaTime  = ((gRotorSpeed.Time - m_Time) & 0xFFFFFFUL); 
    gRotorSpeed.DetaTime += gRotorSpeed.DetaTimeAdd;

    /*测速计算*/
    if(gRotorSpeed.DetaPos != 0) 
    {
        if(gRotorSpeed.DetaTimeAdd == 0)
        {
	        gRotorSpeed.DetaTime = gRotorSpeed.DetaTime + ((u32)gRotorSpeed.CapTimeLast<<3) - ((u32)m_CaptureTime<<3);
        }
        gRotorSpeed.DetaTimeAdd = 0;
        gRotorSpeed.Pos         = m_QEPCnt;
        gRotorSpeed.Time        = m_Time;
        gRotorSpeed.CapTimeLast = m_CaptureTime;            //保存上次的捕获值
        gRotorSpeed.PulseNum    = gPGData.PulseNum;
        m_Speed = CalFeedSpeed(gRotorSpeed.DetaPos,gRotorSpeed.DetaTime);/*调用速度计算函数计算速度*/
    }
    else if(gRotorSpeed.DetaTime >= C_TIME_4MS)		        //低速的处理                                                                  
    {
        gRotorSpeed.DetaTimeAdd     = C_TIME_4MS;
        gRotorSpeed.Pos             = m_QEPCnt;
        gRotorSpeed.Time            = m_Time;
        gRotorSpeed.CapTimeLast     = 0;
        m_Speed = 0;
    }
    else
    {
        m_Speed = gRotorSpeed.SpeedFeedFilter;   // 如果本拍没有检测到脉冲，沿用上一拍的速度
    } 
    gRotorSpeed.SpeedFeed = m_Speed;
}

/*******************************************************************************
* Function Name  : 通过位置偏差和时间偏差计算速度
* Description    : 使用4个全局变量:
                    gMotorExtInfo.Poles    电机极对数
                    gPGData.PulseNum    编码器线数
                    gBasePar.FullFreq   32768对应的德?0.1Hz单位)
                    gSysClock           系统时钟，(1MHz单位)
* Input          : DetaTime 时间差内计量的DetaPos QEP脉冲数(4倍频后的值)
* Output         : 频率，标幺值s32格式，会超过32767，应贸绦蚴侗?
* Return         : None
*******************************************************************************/
s32 CalFeedSpeed(s16 DetaPos, u32 DetaTime)
{
    s16 m_DetaPos;
    s32 m_Speed;
    u32 m_DetaTime;
    u32 m_Long,m_Long1,m_Long2;
    u64 m_Llong;

    m_DetaPos  = abs(DetaPos);
    m_DetaTime = DetaTime;
    
	m_Long1 = (u32)gMotorExtInfo.Poles * (u32)m_DetaPos;
	m_Long2 = (25000000UL * (u32)DSP_CLOCK)/m_DetaTime;
	m_Llong = ((u64)m_Long1 * (u64)m_Long2);
    if(m_Llong > ((u64)1 << (63 - 24)))
    {
        m_Llong = ((u64)1 << 63);
    }
    else
    {
        m_Llong = m_Llong<<24;                              /*反馈速度修改为Q24格式*/
    }
	m_Long  = gRotorSpeed.PulseNum * (u32)gBasePar.FullFreq01;

	m_Speed = (s32)(m_Llong/m_Long);
    m_Speed = Min(m_Speed,(1UL<<24));
	if(DetaPos < 0)
	{
		m_Speed = -m_Speed;
	}
    return m_Speed;

}

/*******************************************************************************
* Function Name  : 计算编码器的相对位置
* Description    : 同步机控制下gPGData.RefPos和gPGData.RotorPos在上电、编码器零点位置角
                   调谐结束后会重置
                   同步机控制下gPGData.RotorPos在基准Z信号到达后会修正
* Input          : None 
* Output         : None
* Return         : None
*******************************************************************************/
void QEPGetRealPos(void)              
{
    u32 m_QepCnt;
    s32 m_s32;
	s16	m_DetaQep,m_Add;
    static s16 m_Remain = 0;
    static s16 m_RemainAbs = 0;	

	m_QepCnt             = GetQepCnt();									//获取QEP计数值
    gIPMPos.QepCntPosCal = m_QepCnt;
	m_DetaQep       = (s16)(m_QepCnt - gIPMPos.QepBak);
	m_s32 = (((s32)m_DetaQep<<14) * gMotorExtInfo.Poles + m_RemainAbs);
    m_Add = m_s32/gPGData.PulseNum;       	
	gIPMPos.RotorPos += m_Add; 	
	gIPMPos.QepBak  = m_QepCnt;
	m_RemainAbs = m_s32 - ((s32)m_Add * (s32)gPGData.PulseNum);/*余数*/ 
		
    // 计算相对位置gPGData.PgPos，该位置在基准Z信号达到后不会更改,用于速度环和对ABZ编码器防护飞车
    m_s32 = (((s32)m_DetaQep<<14) * gMotorExtInfo.Poles + m_Remain);/*认为不会溢出*/
    m_Add = m_s32/gPGData.PulseNum; 
    gPGData.RefPos += ((s32)m_Add<<8);
	//gPGData.RefPosLast = gPGData.RefPos;                                  /*Q24格式*/                           
    m_Remain = m_s32 - ((s32)m_Add * (s32)gPGData.PulseNum);/*余数*/
    // 计算相对位置gPGData.PgPosAbs，该位置在基准Z信号达到后会重新复位，用于同步机的转子磁场定向
    	                          	
    gIPMPos.RotorPos += gIPMPos.ZDetaPos;
    gIPMPos.ZDetaPos  = 0;
}
/*************************************************************
	旋转变压器获取位置函数，通过SPI读取位置信号,下溢中断中执行
注意:要求电机极对数是旋变极对数的整数倍，旋变极对数小于等于电机极对数
*************************************************************/
void ReadRTPos(void)
{
	s16     m_DetaPos;
    u16     m_Pos;
    s32     m_s32,m_Mod;
	u32     m_DetaTime;

 	gRotorTrans.ReadPos = GetRotorTransPos();                  // 获取SPI数据(旋变位置)
    RT_SAMPLE_START; 
                                         // 为下一次获取位置设置SAMPLE信号有效
   	m_Pos = gRotorTrans.ReadPos;                                           
	m_DetaPos           = (s16)(m_Pos - gRotorTrans.PosBak);
	gRotorTrans.PosBak  = m_Pos;       	
    m_s32 = ((s32)m_DetaPos * gMotorExtInfo.Poles + gRotorTrans.Remain);
    m_Mod = (s16)(m_s32/gRotorTrans.Poles);
	gRotorTrans.PosComp   = m_Mod;
    gRotorTrans.AddPos   += (m_Mod<<8);                                       // 累加电角度(速度环用)，Q24格式   
    gRotorTrans.Remain    = m_s32 - (m_Mod * gRotorTrans.Poles);                  // 余数 

    // 旋变机械角度转换为电角度处理(同步机直接使用的角度)，同时考虑了补偿角.
    gRotorTrans.RTPos = ((u32)m_Pos * (u32)gMotorExtInfo.Poles)/gRotorTrans.Poles;   //辨识磁极初始位置时使用
 
    gRotorTrans.SampleTime = GetTime();    
    m_DetaTime = abs((s32)(gRotorTrans.TimeBak - gRotorTrans.SampleTime));	
		
    if(m_DetaTime >= C_TIME_045MS)
    {   
        gRotorTrans.TimeBak  = gRotorTrans.SampleTime; 
        gRotorTrans.DetaTime = m_DetaTime;
    	m_DetaPos = (s16)(m_Pos - gRotorTrans.PosRTBak);  
        gRotorTrans.PosRTBak = m_Pos;      
        gRotorTrans.DetaPos  = (m_DetaPos>>2);                              // 为了匹配CalFeedSpeed函数调用，不会损失精度。
        CalRTSpeed();
    }
}

/*************************************************************
	旋转变压器情况下，计算速度函数,最多支持32对极的旋变。
*************************************************************/
void CalRTSpeed(void)
{
    s32     m_Speed;
    s16     m_DetaPos;
    u32     m_DetaTime;
    
    DINT;
    m_DetaTime = gRotorTrans.DetaTime;
    m_DetaPos  = gRotorTrans.DetaPos;
    EINT;
    // 计算反馈频率
    gRotorSpeed.PulseNum = (u32)gRotorTrans.Poles<<12;                         // 旋变固定为4096线
    m_Speed              = CalFeedSpeed(m_DetaPos,m_DetaTime);
    gRotorSpeed.SpeedFeed = m_Speed;     
}

