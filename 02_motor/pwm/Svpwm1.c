#include "MotorInclude.h"
#include "Svpwm.h"
//#if 0
u16     ZeroLengthPhase_Dpwm[12] = {0,2,2,1,1,0,0,2,2,1,1,0}; //U,V,W 哪一相为全脉宽
s32     Ratio;
void  OutPutPWM1(void)
{
	u16     m_iSec30;
	u16 	m_iSec60;		
	u16 	m_iSec120;
	u16 	m_iRamainPhase;
	u16 	SinAlfa,SinAlfa120;
	u16		m_iTaLength,m_iTbLength,m_iTcLength,m_iZeroLength;
	u16		phase;
    u32     m_iTALength,m_iTBLength,m_iTmaxLength,m_iTminLength,m_iTmidLength;
	s32     m_long;

    phase = gPhase.OutPhase + 32768; //gPWM.Phase;
//...计算30度范围的扇区等
    m_iSec30 = ((u32)phase * 12) >> 16;		 
	m_iSec60 = m_iSec30 >> 1;       
	m_iSec120 = m_iSec60 >> 1;	
	m_iRamainPhase = phase - ((u32)m_iSec120<<16)/3;
	
	SinAlfa120 = qsin(21845 - m_iRamainPhase);  
	SinAlfa = qsin(m_iRamainPhase);

	if(gRatio < 6000)/*过调制1.05*/
	{
		Ratio = gRatio;
	}
	else
	{
		m_long = gRatio - 6000;
		m_long = m_long * m_long/182;
		Ratio = m_long + 6000;
	}
	/*gPWM.Ratio=8100时，Ratio=30000*/
	if(Ratio > 28000)
	{
		Ratio = 28000;
	}
    if(Ratio < 31000)
	{
	    m_iTALength = ((u32)SinAlfa120 * (u32)Ratio)>>15;
	    m_iTALength = (m_iTALength * (u32)gPWM.gPWMPrdApply)>>12;

	    m_iTBLength = ((u32)SinAlfa * (u32)Ratio)>>15;
	    m_iTBLength = (m_iTBLength * (u32)gPWM.gPWMPrdApply)>>12;
	}
	else                           //输出标准的六阶梯波
	{
	    if(m_iRamainPhase < 5461)
		{
		    m_iTALength = gPWM.gPWMPrdApply;
            m_iTBLength = 0;
		}
		else if(m_iRamainPhase >= 16384)
		{
		    m_iTALength = 0;
            m_iTBLength = gPWM.gPWMPrdApply;
   		}
		else
		{
		    m_iTALength = gPWM.gPWMPrdApply;
            m_iTBLength = gPWM.gPWMPrdApply;
   		}
	}


    m_iTmaxLength = Max(m_iTALength,m_iTBLength);
	m_iTminLength = Min(m_iTALength,m_iTBLength);

    if(m_iTmaxLength > gPWM.gPWMPrdApply)
	{
	    m_iTmidLength = (m_iTmaxLength - (u32)gPWM.gPWMPrdApply)>>1;
		if(m_iTmidLength >= m_iTminLength)
		{
		    m_iTmidLength = 0;
		}
		else
		{
		    m_iTmidLength = m_iTminLength - m_iTmidLength;
		}
		if(m_iTmidLength > gPWM.gPWMPrdApply)
		{
		    m_iTmidLength = (u32)gPWM.gPWMPrdApply;
		}

		switch(m_iSec60)
		{
		    case 0:
			{
			    m_iTaLength = gPWM.gPWMPrdApply;
				m_iTbLength = (u16)m_iTmidLength;
				m_iTcLength = 0;
			}
			break;
			case 1:
			{
			    m_iTaLength = (u16)m_iTmidLength;
				m_iTbLength = gPWM.gPWMPrdApply;
				m_iTcLength = 0;
			}
			break;
			case 2:
			{
			    m_iTaLength = 0;
				m_iTbLength = gPWM.gPWMPrdApply;
				m_iTcLength = (u16)m_iTmidLength;
			}
			break;
			case 3:
			{
			    m_iTaLength = 0;
				m_iTbLength = (u16)m_iTmidLength;
				m_iTcLength = gPWM.gPWMPrdApply;
			}
			break;
			case 4:
			{
			    m_iTaLength = (u16)m_iTmidLength;
				m_iTbLength = 0;
				m_iTcLength = gPWM.gPWMPrdApply;
			}
			break;
			case 5:
			{
			    m_iTaLength = gPWM.gPWMPrdApply;
				m_iTbLength = 0;
				m_iTcLength = (u16)m_iTmidLength;
			}
			break;
		}
	}
	else                                                   //没有发生过调制的发波处理
	{
        if(gPWM.PWMModle == MODLE_CPWM)
		{
            m_iZeroLength = (u16)(((u32)gPWM.gPWMPrdApply - m_iTmaxLength)>>1);
			m_iTALength = m_iTALength + (u32)m_iZeroLength;
			m_iTBLength = m_iTBLength + (u32)m_iZeroLength;
			gPWM.gZeroLengthPhase = (ZERO_LENGTH_PHASE_SELECT_ENUM)ZERO_VECTOR_NONE;
	    }
		else
		{
		    m_iZeroLength = 0;
			gPWM.gZeroLengthPhase = (ZERO_LENGTH_PHASE_SELECT_ENUM)ZeroLengthPhase_Dpwm[m_iSec30];

//..........DPWM模式全脉宽高低电平切换处理
			if((((m_iSec30+1)>>1)&0x01) == 0)
			{
			    if(m_iTALength > m_iTBLength)
			    {
                    m_iZeroLength = (u16)((u32)gPWM.gPWMPrdApply - m_iTALength);
			        m_iTALength = (u32)gPWM.gPWMPrdApply;
					m_iTBLength = m_iTBLength + (u32)m_iZeroLength;
				}
				else
				{
                    m_iZeroLength = (u16)((u32)gPWM.gPWMPrdApply - m_iTBLength);				
			        m_iTBLength = (u32)gPWM.gPWMPrdApply;
					m_iTALength = m_iTALength + (u32)m_iZeroLength;
				}
			}
		}

	    switch(m_iSec120)
		{
		    case 0:
			{
			    m_iTaLength = (u16)m_iTALength;
				m_iTbLength = (u16)m_iTBLength;
				m_iTcLength = m_iZeroLength;
			}
			break;
			case 1:
			{
			    m_iTaLength = m_iZeroLength;
				m_iTbLength = (u16)m_iTALength;
				m_iTcLength = (u16)m_iTBLength;
			}
			break;
			case 2:
			{
			    m_iTaLength = (u16)m_iTBLength;
				m_iTbLength = m_iZeroLength;
				m_iTcLength = (u16)m_iTALength;
			}
			break;
		}
	}

//开始设置输出UVW的比较值
	gPWM.USet = m_iTaLength;
	gPWM.VSet = m_iTbLength;
	gPWM.WSet = m_iTcLength;
}
//#endif
/******************************* END OF FILE***********************************/

