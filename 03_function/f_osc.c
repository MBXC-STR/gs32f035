/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : sci_osc.c
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 05/18/2010
* Description        : MD380ʾ����SCI��̨����ģ��C�����ļ�
*					ʹ��F320F28035оƬSCI�ӿڣ�ʹ������оƬ��Ҫ�޸ĵײ�ͨѶģ��
*					���ݷ���ʹ����FIFO "4��"ģʽ��
*					���ݽ���ʹ�� "1��"FIFO�ж�ģʽ
*                   ͨѶ��ʽ��	����λ 8λ  ֹͣλ 1λ ��У�� ������
*                               �����ʣ�115��200 57��600 19��200
********************************************************************************
* �޸�				��
* �汾				��V1.00
* ʱ��				��
* ˵��				���޸���������У�飬�޸ķ���˫�������ģʽ��ֹ�������
***************************************************************************************************
* �޸�				��
* �汾				��V1.10
* ʱ��				��
* ˵��				����������SCI������������ֹ�����ʲ���ȷ���SCI�������״̬
*					  �޸��������ʼ����������19200���������ô���
***************************************************************************************************
* �޸�				��
* �汾				��V1.20
* ʱ��				��
* ˵��				������RS485���ݹ��ܣ�����C2000����SCI�ӿ�
*					  �޸�ʹ��FIFO����4�ֽ�оƬ������λ�����͹�����ܳ��ֵ�Bug
*					  ���Ӹ�����뿪�ص�
***************************************************************************************************/
//***********************��ʹ�õı༭���޷����룬�뽫TAB����Ϊ4���ַ�λ��*****************************

#include "f_osc.h"										// ����ͷ�ļ�
#include "f_comm.h"		
#include "f_ui.h"
#define     DEBUG_F_OSC        	0

/************************Һ������ʹ��****************************/
#if FC_KEY_CONTROL_EN
extern Uint16	companyPwdPass4Comm;
extern Uint16 userPwdPass4Comm;
Uint16 LcdKeyFlag = 0;  // Һ�����̲����־λ����ʼ��Ϊ��Ĭ�ϼ��̲�����
Uint16 LcdReadFEflag=0; //Һ�����̶�ȡFE��
extern Uint16 keyFunc;
#endif
/*************************************************/ 

#if DEBUG_F_OSC

#if (SCI_INT_LOAD_RAM == 1)
	#if (OSC_TX_INT_EN == 1)
		#pragma CODE_SECTION(sciaTxFifoIsr, "ramfuncs");	// SCI�����жϼ��ص�RAM����
	#endif
	#pragma CODE_SECTION(sciaRxFifoIsr, "ramfuncs");		// �����жϼ��ص�RAM����
#endif

/*****************************************************************/
// �ⲿ��������
#if OSC_CON_CHECK
extern Uint16 CrcValueByteCalc(const Uint16 *data, Uint16 length);
#endif
// ���������������
#if FC_CODE_CONTROL_EN
Uint16 OscControlWriter(Uint16 addr, Uint16 data);
Uint16 OscControlRead(Uint16 addr, Uint16* result);
#endif

// ������궨�壬�޸ĸò���ѡ��ʾ����������
static Uint16 *OscQuqData = (Uint16 *)((&funcCode.group.u0[0]) -1);
															// ���������ָ��
#define 	GET_OSC_DATA(addr)				OscQuqData[addr]
/*****************************************************************/


static const	Uint16	BautRateCfg[3] = {
									(Uint16)(PERIPHERAL_CLK / 115200 / 8 -1),\
									(Uint16)(PERIPHERAL_CLK / 57600 / 8 -1),\
									(Uint16)(PERIPHERAL_CLK / 19200 / 8 -1)
								};							// �����ʲ���

								
static DSP_OSC_CON_DATA	OscCongData;						// ʾ�������ò��� 
static OSC_DATA_TYPE	OscDataBufA, OscDataBufB;			// ���ݻ������壬˫����ģʽ
static uint8		OscConFrameBuf[10];						// ʾ�������ݽ��ջ��棬����֡�̶�8�ֽ�
static uint8		BaudCfg = 0;							// ���������ñ�־

static SCI_RT_CON_DATA 	SciTxBufStr, SciRxBufStr;			// �����շ�����ģ��

// �ڲ���������
static void InitOsc(void);									// ��ʼ��ʾ����ģ��
static void InitSciOSC(uint8);								// ��ʼ��ʾ��������												
static uint8 SciDataTX(uint8 *buf, uint8 len);				// ���ݷ��ͺ���

#if OSC_TX_INT_EN == 1
#ifdef TARGET_GS32
static __interrupt void sciaTxFifoIsr(void);					// �����ж�
#else
static interrupt void sciaTxFifoIsr(void);					// �����ж�
#endif
#endif
#ifdef TARGET_GS32
static __interrupt void sciaRxFifoIsr(void);					// �����ж�
#else
static interrupt void sciaRxFifoIsr(void);					// �����ж�
#endif
static uint8 SciDataRx(uint8 *buf, uint8 len, uint8 timeout);		// ���ݽ��պ���
static void SciDataRxDeal(void);							// �����
static void OscDataQcq(void);								// ���ݲ���
static void OscDataTxDeal(void);							// ���ݷ���
static uint8 GetOscData(uint8 *databuf);					// ��ȡʾ�������ݣ���һ��ȫͨ������
static void OscSciIoInit(void);								// SCI�ӿ�IO���߳�ʼ��							
#if OSC_DATA_CHECK
static void ClearOscBufEnd(void);							// ��ջ������4�ֽ�
//static uint8 OscDataCk(uint8 *buf, uint8 len);
#endif
/*************************************************/
#if	FC_CODE_APP_EN

#if DEBUG_MD500_SEARIS||DEBUG_MD290_SEARIS||DEBUG_MD380E_SEARIS
static uint8 OscConFrameAPPBuf[69];						// APP���ӱ�ǩ��������buff
#define			APP_CONTROL_FRAME_HEAD				OscConFrameAPPBuf[0]
#define			APP_CONTROL_FRAME_FC				OscConFrameAPPBuf[1]
#define			APP_CONTROL_FRAME_DATA_NUM          OscConFrameAPPBuf[2]
#define			APP_CONTROL_FRAME_CRCL				OscConFrameAPPBuf[67]
#define			APP_CONTROL_FRAME_CRCH				OscConFrameAPPBuf[68]
#else
static uint8 OscConFrameAPPBuf[132];						// APP���ӱ�ǩ��������buff
#define			APP_CONTROL_FRAME_HEAD				OscConFrameAPPBuf[0]
#define			APP_CONTROL_FRAME_FC				OscConFrameAPPBuf[1]
#define			APP_CONTROL_FRAME_CRCL				OscConFrameAPPBuf[130]
#define			APP_CONTROL_FRAME_CRCH				OscConFrameAPPBuf[131]
#endif

#if DEBUG_MD290_SEARIS
/*const unsigned int APPIDDataBuff[8]={
0x0001,//P0 1 ���̴���
0x000B,//P2 3 ��Ʒϵ��
0x010E,//P4 5 ��Ʒ�ͺ�
0x1D7E,//P6 7 ��Ʒ�汾
0x0000,//P8 9 ͨ�Žӿڰ汾 v0.0
0x0000,//P10 11 Boot�汾   v0.0
0x0015,//P12 13 оƬ�ͺ�
0xffff//P14 --P15����
};*/
const unsigned int APPIDDataBuff[8]={
0x0001,//P0 1 ���̴���
0x000B,//P2 3 ��Ʒϵ��
0x0110,//P4 5 ��Ʒ�ͺ�
//0x0B54,//P6 7 ��Ʒ�汾
SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//P6 7 ��Ʒ�汾
0x0000,//P8 9 ͨ�Žӿڰ汾 v0.0
0x0000,//P10 11 Boot�汾   v0.0
0x0015,//P12 13 оƬ�ͺ�
0xffff//P14 --P15����
};
#elif DEBUG_MD500_SEARIS
const unsigned int APPIDDataBuff[8]={
0x0001,//P0 1 ���̴���
0x000B,//P2 3 ��Ʒϵ��
0x0111,//P4 5 ��Ʒ�ͺ�
//0x1D7E,//P6 7 ��Ʒ�汾
SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//P6 7 ��Ʒ�汾
0x0000,//P8 9 ͨ�Žӿڰ汾 v0.0
0x0000,//P10 11 Boot�汾   v0.0
0x0015,//P12 13 оƬ�ͺ�
0xffff//P14 --P15����
};
#elif DEBUG_MD380E_SEARIS
const unsigned int APPIDDataBuff[8]={
0x0001,//P0 1 ���̴���
0x000B,//P2 3 ��Ʒϵ��
0x010F,//P4 5 ��Ʒ�ͺ�
//0x1D7E,//P6 7 ��Ʒ�汾
SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//P6 7 ��Ʒ�汾
0x0000,//P8 9 ͨ�Žӿڰ汾 v0.0
0x0000,//P10 11 Boot�汾   v0.0
0x0015,//P12 13 оƬ�ͺ�
0xffff//P14 --P15����
};
#else
const unsigned int APPIDDataBuff[8]={
0x0001,//P0 1 ���̴���
0x000B,//P2 3 ��Ʒϵ��
0x0107,//P4 5 ��Ʒ�ͺ�
//0x1c86,//P6 7 ��Ʒ�汾
SOFTWARE_VERSION*100+SOFTWARE_VERSION_TEMP,//P6 7 ��Ʒ�汾
0x0000,//P8 9 ͨ�Žӿڰ汾 v0.0
0x0000,//P10 11 Boot�汾   v0.0
0x0015,//P12 13 оƬ�ͺ�
0xffff//P14 --P15����
};
#endif
void copyAPPtoBuff(void);
#endif

/*******************************************************************************
* ��������          : void OscSciFunction(void)
* ��ڲ���			��con		"OSC_SCI_EN"	ʾ��������ʹ��
								����			ʾ����ֹͣ
* ����				��OSC_SCI_EN                ʾ����ģʽ
                      ����                      �˳�ʾ����ģʽ
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: ʾ������̨����������������0.5ms�������øú���
* 
********************************************************************************/
void OscSciFunction(void)
{
	static uint8 count = 0;									// ʱ�������
//	static uint8 countRx = 0;								// ���ռ�ʱ
	static uint8 OscCfg = 0;								// ʾ�����������ñ�־"0"δ����, "1"�������

	if (BaudCfg)											// �޸Ĳ�����
	{
		count ++;
		if (count > 2)										// �ȴ���2����޸Ĳ�����
		{
			InitSciOSC(OscCongData.baudRate);              // ���³�ʼ������	
			BaudCfg = 0;
			count = 0;
		}
		return;
	}
	
	if (0 == OscCfg)										// ��ʼ����־Ϊ��0��
	{
		
		OscDataBufA.frameHead1 = OSC_DATA_FRAME_HEAD_A;		// ��ʼ������
		OscDataBufA.frameHead2 = OSC_DATA_FRAME_HEAD_B;		// ��ʼ������
		OscDataBufB.frameHead1 = OSC_DATA_FRAME_HEAD_A;
		OscDataBufB.frameHead2 = OSC_DATA_FRAME_HEAD_B;
		OscDataBufA.rwPI = 0;
		OscDataBufB.rwPI = 0;
		OscDataBufA.full = 0;                               // ��ջ���
        OscDataBufB.full = 0;

        InitOsc();											// ִ��ʾ������ʼ������
		OscCfg = 1;											// ��λ���ڳ�ʼ����־λ
		return;												// ֱ�ӷ��أ���С��ģ�鴦��ʱ��
	}

//	countRx ++;
//	if (countRx > 4)										// ��4�������������1��
	{
  //		countRx = 0;		
		if (SciDataRx((uint8 *)(&OscConFrameBuf), 8, 0) == TRUE)	// Sci���ݽ����봦��
		{
			SciDataRxDeal();								// ���������
			return;
		}
	}
	OscDataQcq();											// �����ź����ݲɼ�	
	OscDataTxDeal();										// ���ݷ��͵�PC��ѭ�����˫��������
}	

/*******************************************************************************
* ��������          : void InitOsc(DSP_OSC_CON_DATA *OscCongData)
* ��ڲ���			��OscCongData		ʾ�������ò���ָ��
* ����				����
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: ʾ����ģ���ʼ��������SCI��ʾ����������ʼ��
* 
********************************************************************************/

void InitOsc(void)
{
	OscCongData.baudRate = 3;                              // ������19200
    OscCongData.interval = 1;                              // �������
    OscCongData.status = 0;                                // ��ͣ����
    OscCongData.runContinue = 1;                           // �������У�ͣ������
    OscCongData.ch1Addr = 1;
    OscCongData.ch2Addr = 0;
    OscCongData.ch3Addr = 0;
    OscCongData.ch4Addr = 0;
	OscCongData.chSum = 1;

    InitSciOSC(OscCongData.baudRate);                      // ��ʼ������
}	


/*******************************************************************************
* ��������          : void InitSciOSC(uint8 baudRate)
* ��ڲ���			��OscCongData		ʾ�������ò���ָ��
* ����				����
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: ʾ����ģ���ʼ��������SCI��ʾ����������ʼ��
* 
********************************************************************************/
void InitSciOSC(uint8 baudRate)
{
    SciTxBufStr.busy = 0;                                   // ��ʼ��������/�����ƿ�
    SciRxBufStr.busy = 0;
//	SciRxBufStr.err  = 0;									// ��մ��ڳ�����־							
	SciRxBufStr.len = 0xff;
#if 	(1 == OSC_SCI_SEL)	
    PieCtrlRegs.PIEIER9.bit.INTx1 = 0;     					// PIE Group 9, INT1	RX�����ж� ��ֹ
	PieCtrlRegs.PIEIER9.bit.INTx2 = 0;     					// PIE Group 9, INT2	TX�����ж�
#else
    PieCtrlRegs.PIEIER9.bit.INTx3 = 0;     					// PIE Group 9, INT3	RX�����ж� ��ֹ
	PieCtrlRegs.PIEIER9.bit.INTx4 = 0;     					// PIE Group 9, INT4	TX�����ж�
#endif
	
	// ����SCI-Aʹ�ùܽ�
	OscSciIoInit();


	// ���ô��ڲ���
	SCI_OSC_REGS.SCICCR.all = 0x0007;  						// 1 stop bit,  No loopback
															// No parity,8 char bits,
															// async mode, idle-line protocol
	SCI_OSC_REGS.SCICTL1.all = 0x0002 | 1<<6; 				// enable TX���� ������ж�
															// Disable RX ERR, SLEEP, TXWAKE
															
   SCI_OSC_REGS.SCILBAUD = BautRateCfg[baudRate-1];	
   SCI_OSC_REGS.SCIHBAUD = BautRateCfg[baudRate-1]>>8;  	// ����������

   SCI_OSC_REGS.SCICTL2.all = 0;							// ���ս�ֹ��
   
   SCI_OSC_REGS.SCIFFTX.all = 0xC000;						// ����FIFO��ǿ���ܣ�	���͵���������ж� �����жϴ���

   SCI_OSC_REGS.SCIFFRX.all = 0x0004;						// ���ý���FIFO��4������ʱ�����ж�
   SCI_OSC_REGS.SCIFFCT.all = 0x00;
   SCI_OSC_REGS.SCIPRI.bit.FREE = 1;						// ���Բ�ֹͣ   

   SCI_OSC_REGS.SCICTL1.bit.SWRESET = 1;     				// Relinquish SCI from Reset	��������

	
// SCI����ж�ȫ�ּĴ���
#if 	(1 == OSC_SCI_SEL)
	PieCtrlRegs.PIEIFR9.bit.INTx1 = 0;						// ����жϱ�־    ����
    PieCtrlRegs.PIEIFR9.bit.INTx2 = 0;                      // ����
#else
	PieCtrlRegs.PIEIFR9.bit.INTx3 = 0;						// ����жϱ�־    ����
    PieCtrlRegs.PIEIFR9.bit.INTx4 = 0;                      // ����
#endif	
	
// �ж�������ʼ��   
	EALLOW;  												// This is needed to write to EALLOW protected registers
#if 	(1 == OSC_SCI_SEL)
	#ifdef TARGET_GS32
	interrupt_register(INT_SCIA_RX, &sciaRxFifoIsr);
	#else
    PieVectTable.SCIRXINTA = &sciaRxFifoIsr;
	#endif
    #if OSC_TX_INT_EN == 1
		#ifdef TARGET_GS32
		interrupt_register(INT_SCIA_TX, &sciaTxFifoIsr);
		#else
	    PieVectTable.SCITXINTA = &sciaTxFifoIsr;
		#endif
    #endif
#else    
		#ifdef TARGET_GS32
		interrupt_register(INT_SCIB_RX, &sciaRxFifoIsr);
		#else
		PieVectTable.SCIRXINTB = &sciaRxFifoIsr;
		#endif
    #if OSC_TX_INT_EN == 1
		#ifdef TARGET_GS32
		interrupt_register(INT_SCIB_TX, &sciaTxFifoIsr);
		#else
	    PieVectTable.SCITXINTB = &sciaTxFifoIsr;
    #endif
#endif
	EDIS;
	
#if 	(1 == OSC_SCI_SEL)	
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;						// ʹ���жϣ�
	#if OSC_TX_INT_EN == 1
		PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
	#else
		PieCtrlRegs.PIEIER9.bit.INTx2 = 0;
	#endif
#else	
	PieCtrlRegs.PIEIER9.bit.INTx3 = 1;						// ʹ���жϣ�
	#if OSC_TX_INT_EN == 1
		PieCtrlRegs.PIEIER9.bit.INTx4 = 1;
	#else
		PieCtrlRegs.PIEIER9.bit.INTx4 = 0;
	#endif
#endif	

//	OscConFrameBuf[9] = 
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   					// Enable the PIE block��ʹ��PIEģ��
	IER |= M_INT9; 											// Enable CPU INT9
}
		
/*******************************************************************************
* ��������          : uint8 SciDataTX(uint8 *buf, uint8 len)
* ��ڲ���			��buf		�ֽ�������ʼ��ַ
*					  num		���ݳ���
* ����				��TRUE		���Ͳ����ɹ�
					  FALSE		�ϴη���δ��ɣ�����ʧ��
* �汾		        : V0.0.1
* ʱ��	            : 05/18/2010
* ˵��				: ͨ���жϷ�ʽ��������
*                     Sci�������ݣ���Ҫ���ڳ�ʼ��SCI����ģ��
********************************************************************************/
uint8 SciDataTX(uint8 *buf, uint8 len)					
{
 //   PieCtrlRegs.PIEIFR9.bit.INTx2 = 0;                      // ���PIE�з����жϱ�־
#if RS485_ENABLE == 1
	RS485_RTS_O = RS485_T_O;
#endif
	if (TRUE == SciTxBufStr.busy)							// ����ģ��æ
		return	FALSE;
	SCI_OSC_REGS.SCIFFTX.all = 0xC000;						// ��λFIFO,FIFO����ֵ��"0"
	SCI_OSC_REGS.SCIFFTX.bit.TXFIFOXRESET = 1;				// ����FIFO
	
	SCI_OSC_REGS.SCITXBUF = *buf++;							// д4��FIFO
	SCI_OSC_REGS.SCITXBUF = *buf++;
	SCI_OSC_REGS.SCITXBUF = *buf++;
	SCI_OSC_REGS.SCITXBUF = *buf++;
	SciTxBufStr.buf = buf;                                  // д�����ַ�볤��
    SciTxBufStr.len = len - 4;
	SciTxBufStr.busy = TRUE;								// ���ݷ�����
	
#if OSC_TX_INT_EN == 1	
    SCI_OSC_REGS.SCIFFTX.bit.TXFFINTCLR = 1;  				// Clear SCI Interrupt flag
    SCI_OSC_REGS.SCIFFTX.bit.TXFFIENA = 1;                  // ʹ�ܷ���FIFO�ж�
#endif
	
//	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;     					// PIE Group 9, INT2	TX�����ж�
	return TRUE;
}


/*	*/
/*******************************************************************************
* ��������          : void OscTxFifoTask(void)
* ��ڲ���			����
* ����				����
* �汾		        : V0.0.1
* ʱ��	            : 05/31/2010
* ˵��				: ���ж�ģʽ���ݷ���ģ�飬��0.5ms ����һ��
*					  ��Ҫ���дFIFO���� 
********************************************************************************/
#if OSC_TX_INT_EN == 0
void OscTxFifoTask(void)
{
	uint8 fifo, len;
	uint8 *buf;												// �ڲ���ʱ����

	if (SciTxBufStr.busy == 0)								// δ��ʼ����
		return;

	len = SciTxBufStr.len;
	buf = SciTxBufStr.buf;	
	if (len == 0)
	{
		SciTxBufStr.busy = 0;								// �ôη������		
		return;
	}	
	fifo = 4 - SCI_OSC_REGS.SCIFFTX.bit.TXFFST;				// ��ȡ����FIFO����
	if (len < fifo)
	{
		fifo = len;
		len = 0;
	}
	else
	{
		len -= fifo;		
	}	
	while (fifo)
	{
		SCI_OSC_REGS.SCITXBUF = *buf++;						// дFIFO
		fifo--;
	}
	SciTxBufStr.len = len;
	SciTxBufStr.buf = buf;
}
#endif
	
	
/*******************************************************************************
* ��������          : interrupt void sciaTxFifoIsr(void)
* ��ڲ���			����
* ����				����
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: SCI FIFOģʽ���ݷ����ж�
* 					  �ú����Ǽ���ģʽ״̬��������������
********************************************************************************/
#if OSC_TX_INT_EN == 1
#ifdef TARGET_GS32
__interrupt void sciaTxFifoIsr(void)
#else
interrupt void sciaTxFifoIsr(void)
#endif
{
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
	uint8 *buf;												// ڲʱ
	uint8 len;

	buf = SciTxBufStr.buf;
	len = SciTxBufStr.len;
#if SCI_TX_INT_SPEEDUP == 1
	if (len > 3)
    {
    	SCI_OSC_REGS.SCITXBUF = *buf++;						// д4��FIFO
    	SCI_OSC_REGS.SCITXBUF = *buf++;
    	SCI_OSC_REGS.SCITXBUF = *buf++;
    	SCI_OSC_REGS.SCITXBUF = *buf++;
		SciTxBufStr.buf = buf;								// ��дȫ�ֱ���
		len -= 4;
		SciTxBufStr.len = len;
    }
    else
    {
		SciTxBufStr.busy = 0;								// �÷��Ϳ���
		SCI_OSC_REGS.SCIFFTX.bit.TXFFIENA = 0;				// ��ֹFIFO�����ж�
    }
#else	
	if (len > 3)
    {
    	SCI_OSC_REGS.SCITXBUF = *buf++;						// д4��FIFO
    	SCI_OSC_REGS.SCITXBUF = *buf++;
    	SCI_OSC_REGS.SCITXBUF = *buf++;
    	SCI_OSC_REGS.SCITXBUF = *buf++;
		len -= 4;
    }
    else
    {
        while (len)											// ���ݳ���С��FIFO���
        {
			SCI_OSC_REGS.SCITXBUF = *buf++;
			len--;
        }
    }
	if (0 == SCI_OSC_REGS.SCIFFTX.bit.TXFFST)				// FIFO�գ����ݷ������
	{
		SciTxBufStr.busy = 0;								// �÷��Ϳ���
		SCI_OSC_REGS.SCIFFTX.bit.TXFFIENA = 0;				// ��ֹFIFO�����ж�
	}
	else
	{
		SciTxBufStr.buf = buf;								// ��дȫ�ֱ���
		SciTxBufStr.len = len;	
	}
#endif	
    SCI_OSC_REGS.SCIFFTX.bit.TXFFINTCLR = 1;  				// Clear SCI Interrupt flag	
#ifdef TARGET_GS32
    
#else
	PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;      			// Issue PIE ACK
#endif
	#ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}
#endif

/*******************************************************************************
* ��������          : uint8 SciDataRx(uint8 *buf, uint8 len uint8 timeout))
* ��ڲ���			��buf	���ջ����ַ
*					  len	���ݳ���
*					  timeout ��ʱʱ��   *��������   0 ���޵ȴ�
* ����				��TRUE  ���ճɹ�
*					  SCI_RT_BUSY	���ղ�����
*					  FALSE ��ʱ����
*					  SCI_RS485_TX_BUSY SCIʹ��485���ͽӿ�æ

* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: SCI FIFOģʽ���ݽ�������
* 					  Sci���ݽ����봦��
* 					  ֻ�ܽ���4������������
********************************************************************************/
uint8 SciDataRx(uint8 *buf, uint8 len, uint8 timeout)									
{
    static uint8 count = 0;									// ��ʱ����
	static uint8 timeOut = 0;								// ��ʱʱ��
//	static uint8 dataLen = 0;								// ���ȼ��	
	static uint8 countOut = 0;
#if RS485_ENABLE == 1	
	if (SciTxBufStr.busy)
		return SCI_RS485_TX_BUSY;							// 485æ
	if (SCI_OSC_REGS.SCICTL2.bit.TXEMPTY)					// ������ɽ������״̬
		RS485_RTS_O = RS485_R_O;
	else
		return SCI_RS485_TX_BUSY;
#endif	

	count ++;
	
	if (( count > timeOut) && (timeOut != 0) )				// ��ʱ���
	{
	    SciRxBufStr.busy = 0;								// �������״̬
		SCI_OSC_REGS.SCICTL1.bit.RXENA = 0;                 // ��ֹ����
		SCI_OSC_REGS.SCIFFRX.all = 4;	
		count = 0;											// ��ձ�־��λFIFO
		return FALSE;
	}
	
	if (TRUE == SciRxBufStr.busy)							// æ���
	{
		if (SciRxBufStr.rxflag)								// �յ���һ�����ݺ�ʼ��ʱ
		{
			countOut =0;
			SciRxBufStr.rxflag &= 0x10;						// �����λ
		}	
		else
		{
			if (++countOut > 2)
			{
				SciRxBufStr.busy = 0;						// �������״̬
				SCI_OSC_REGS.SCICTL1.bit.RXENA = 0;         // ��ֹ����
				SCI_OSC_REGS.SCIFFRX.all = 4;
				return FALSE;
			}
		}
		return SCI_RT_BUSY;
    }
	if (0 == SciRxBufStr.len)								// ���Ƚ������
	{
		SCI_OSC_REGS.SCIFFRX.all = 4;			
		SciRxBufStr.len = 0xff;
		return TRUE;
	}
	countOut = 0;
//	dataLen = len;											// ���ݳ���	
	SciRxBufStr.buf = buf;									// ��ʼ�����ڷ��Ϳ���ģ��
	SciRxBufStr.len = len;
	SciRxBufStr.busy = TRUE;
	SciRxBufStr.rxflag = 0x10;								// �ø�λ������ʼ��ʱ����
	count = 0;												// �����볬ʱ��ʼ��
	timeOut = timeout;

	// ���ݽ��ղ���
	SCI_OSC_REGS.SCIFFRX.bit.RXFIFORESET = 0;				// ��λFIFO��ʹ�ܽ���
	SCI_OSC_REGS.SCICTL1.bit.RXENA = 1;						//
//	SCI_OSC_REGS.SCICTL1.bit.RXERRINTENA = 1;               // ʹ�ܴ����ж�
    SCI_OSC_REGS.SCIFFRX.all = 1<<14 | 1<<13 | 1<<6 | 1<<5 | 4;	// ����FIFO��4�ֽڴ���
/*    
    SCI_OSC_REGS.SCIFFRX.bit.RXFFINTCLR= 1;                 // ��FIFO�ж�
    SCI_OSC_REGS.SCIFFRX.bit.RXFFOVRCLR = 1;                // ���FIFO���
    SCI_OSC_REGS.SCIFFRX.bit.RXFIFORESET = 1;               // ʹ�ܽ���FIFO    
	SCI_OSC_REGS.SCIFFRX.bit.RXFFIENA = 1;                  // ����FIFO�ж�ʹ��   
	SCI_OSC_REGS.SCIFFRX.bit.RXFFIL = 4;					// FIFO����4��
*/	
//	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;     					// PIE Group 9, INT1	RX�����ж�ʹ��
	return SCI_RT_BUSY;
}

/*******************************************************************************
* ��������          : interrupt void sciaRxFifoIsr(void)
* ��ڲ���			����
* ����				����
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: SCI FIFOģʽ���ݽ����ж�
* 
********************************************************************************/
#ifdef TARGET_GS32
__interrupt void sciaRxFifoIsr(void)
#else
interrupt void sciaRxFifoIsr(void)
#endif
{
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
	uint8 *buf;												// �ڲ���ʱ����
	uint8 len;

//	SciRxBufStr.rxflag = 0x01;								// �յ�����д��1�����ݽ��ձ�־	
	if (SCI_OSC_REGS.SCIRXST.bit.RXERROR)                   // �н��մ���
	{
		BaudCfg = 1;										// �������ô���
	}
	else
	{
		SciRxBufStr.rxflag = 0x01;							// �յ�����д��1�����ݽ��ձ�־		
		buf = SciRxBufStr.buf;
		len = SciRxBufStr.len;	
		
		*buf++ = (uint8)(SCI_OSC_REGS.SCIRXBUF.bit.RXDT);	// ��ȡ����
		*buf++ = (uint8)(SCI_OSC_REGS.SCIRXBUF.bit.RXDT);
		*buf++ = (uint8)(SCI_OSC_REGS.SCIRXBUF.bit.RXDT);
		*buf++ = (uint8)(SCI_OSC_REGS.SCIRXBUF.bit.RXDT);
		len -= 4;
		if (0 == len)
		{
			SciRxBufStr.busy = 0;							// �÷��Ϳ���
			SCI_OSC_REGS.SCICTL1.bit.RXENA = 0;             // ��ֹ����
	        SCI_OSC_REGS.SCIFFRX.all = 1<<14 | 0<<13 | 1<<6 | 0<<5 | 4;
															// ��λFIFO��ֹʹ�ô�FIFOоƬ���ͳ�����ɴ����ж�
	//		PieCtrlRegs.PIEIER9.bit.INTx1 = 0;     			// PIE Group 9, INT1	RX�����ж�		
		}
		SciRxBufStr.buf = buf;
		SciRxBufStr.len = len;

		SCI_OSC_REGS.SCIFFRX.all = 1<<14 | 1<<13 | 1<<6 | 1<<5 | 4;
	/*	
		SCI_OSC_REGS.SCIFFRX.bit.RXFFOVRCLR = 1;   			// Clear Overflow flag
		SCI_OSC_REGS.SCIFFRX.bit.RXFFINTCLR = 1;   			// Clear Interrupt flag
	*/
	}
#ifdef TARGET_GS32
    
#else
	PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;       			// Issue PIE ack
#endif
	#ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}


/*******************************************************************************
* ��������          : void SciDataRxDeal(void)
* ��ڲ���			����
* ����				����
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: ʾ��������֡����
* 
********************************************************************************/
#if DEBUG_MD500_SEARIS||DEBUG_MD290_SEARIS||DEBUG_MD380E_SEARIS
void SciDataRxDeal(void)
{
	Uint16 err = 0;
#if FC_CODE_CONTROL_EN
	Uint16 dat, rwaddr;
#endif

#if OSC_CON_CHECK
	Uint16  crcResult;
	crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
															// CRC16 У��
	if (crcResult != (Uint16)(( (CONTROL_FRAME_CRCH<<8) & 0xff00) | (CONTROL_FRAME_CRCL & 0x00ff) ))
	{
		return ;
	}
#endif

	if (CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD)			// ֡��ʼ����
#if	FC_CODE_APP_EN	 
	  if(CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD_APP)	
#endif
	{
		return;
	}


	if (CONTROL_FRAME_FC == FC_STOP_OSC)					// ֹͣʾ��������
	{
		OscDataBufA.rwPI = 0;
		OscDataBufB.rwPI = 0;
		OscDataBufA.full = 0;                               // ��ջ���
        OscDataBufB.full = 0;
		OscCongData.status = 0;
	}
	else if (OscCongData.status)							// ����������״̬
	{
		err = 0x80;											// �������
	}
	else
	{
		switch (CONTROL_FRAME_FC)							// �����ж�
		{
            case FC_COMM_TEST:                              // ͨѶ����
                // ��������κβ���


                break;

            case FC_CHANNL_SEL:								// ͨ������
				OscCongData.chSum = 0;
				
				OscCongData.ch1Addr = CONTROL_FRAME_P1;
				if (CONTROL_FRAME_P1)	OscCongData.chSum ++;
				OscCongData.ch2Addr = CONTROL_FRAME_P2;
				if (CONTROL_FRAME_P2)	OscCongData.chSum ++;
				OscCongData.ch3Addr = CONTROL_FRAME_P3;
				if (CONTROL_FRAME_P3)	OscCongData.chSum ++;
				OscCongData.ch4Addr = CONTROL_FRAME_P4;
				if (CONTROL_FRAME_P4)	OscCongData.chSum ++;
#if OSC_DATA_CHECK
				if (OscCongData.chSum == 3)
				{
					ClearOscBufEnd();						// ���4�ֽ�				
				}
#endif				
				break;
			
			case FC_PARA_CFG:								// �����ٶ�ѡ��
				if ((CONTROL_FRAME_P1>0) && (CONTROL_FRAME_P1<4))
				{
					OscCongData.baudRate = CONTROL_FRAME_P1;
					BaudCfg = 1;							// �޸�����						
				}
				if ((CONTROL_FRAME_P2>0) && (CONTROL_FRAME_P2<101))
				{
					OscCongData.interval = CONTROL_FRAME_P2;		
				}
//				OscCongData.runContinue = CONTROL_FRAME_P3;		
		
				break;
			case FC_START_OSC:								// ����ʾ����
				OscCongData.status = 1;
				break;
#if FC_CODE_CONTROL_EN
			case FC_READ_FC_DATA:							// ���ڲ�ӳ�书�������
/************************Һ������ʹ��****************************/
#if FC_KEY_CONTROL_EN
 dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
  if (FF_PASSWORD == dat)   // ����������ȷ
            {
                companyPwdPass4Comm = 1;
				userPwdPass4Comm = 1;
				commTicker = 0;   //�������㣬30����ֹ�޸�FF��
            }
	if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//Һ�����̶�FE��
	{
		LcdReadFEflag = 1;
	} 
		
#endif
/****************************************************/
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
															// �ϳɵ�ַ��ȡ������
				err = OscControlRead(rwaddr, &dat);		    // ִ�в���
				CONTROL_FRAME_P3 = dat&0x00ff;				// ����ת����  ���ֽ�
				CONTROL_FRAME_P4 = dat>>8;					// ���ֽ�

				break;
			case FC_WRITE_FC_DATA:							// д�ڲ����������
				rwaddr =  ((CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);				
				dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
/************************Һ������ʹ��****************************/	
		#if FC_KEY_CONTROL_EN
			if((rwaddr==0x2000)&&(LcdKeyFlag==1))
			{
			    if(dat==1)
				{
				keyFunc =KEY_RUN;
			     break;
				 }
			 	if(dat==6)
				{
			   	 	keyFunc =KEY_STOP;
					break;
				}
            }
			if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//Һ�����̶�FE��
			{
				LcdReadFEflag = 1;
			}
	    #endif
/************************Һ������ʹ��****************************/				
				err = OscControlWriter(rwaddr, dat);		// ��������	
							
				break;
#endif
/************************Һ������ʹ��****************************/
#if FC_KEY_CONTROL_EN   	// ���м��̹��ܽ��ܣ���1��ʹ�ܣ���0���ر�	
			case FC_VERSION_DATA:							// �����͡��汾�Ų���							
				 LcdKeyFlag = 1;//����1��ʾҺ�����̽���
				 dat = 1;// 1����Һ��������MD380���ͺ�
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_LOWER_DATA:	
			 //	 keydata = CONTROL_FRAME_P2;
               dat= FuncCodeAdd(CONTROL_FRAME_P2);
			 //����ֵ�Ƿ����Ĺ������й���  
            if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.lowerLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
                dat = funcCode.all[dat];
			}
			else
			{
				dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
			}
				
		/*		 if (GetGradeIndex(dat, CONTROL_FRAME_P1) == 10)//f0-10����ֵ���⴦��
            {
                dat = 50 * decNumber[funcCode.code.frqPoint];
				CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
            }  else*/
			{
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
		    }
				 break;
			case FC_UPPER_DATA:							// д�ڲ�����ֵ����							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
				 if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.upperLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
                dat = funcCode.all[dat];
			}
			else
			{
			 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
			} 
				 
				
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_INIT_DATA:
				dat= FuncCodeAdd(CONTROL_FRAME_P2);						// ���ڲ�����ֵ����							
				 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].init;
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_DATA:							// ���ڲ����Բ���							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
			dat =	upsend(GetGradeIndex(dat, CONTROL_FRAME_P1));
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_UDATA:
               dat =dispAttributeU0[CONTROL_FRAME_P1].all;
			   CONTROL_FRAME_P3 = dat&0x00ff;
			   CONTROL_FRAME_P4 = dat>>8;
			break;
#endif

#if	FC_CODE_APP_EN
            case FC_APP_READ_WRITE:
                rwaddr = ( (CONTROL_FRAME_P1<<8) & 0xff00) | (CONTROL_FRAME_P2 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// ��ת��BOOT��
				{
					// ��¼��װ������boot��ʱΪ1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 0x01;
					else
					{
						EALLOW;//����������Ϊ�������ţ�����Ҫ�ָ������
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 0x02;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 0x01;
					err = 0x0;
				#endif
				
				    CONTROL_FRAME_P1 = 0x02;
				    CONTROL_FRAME_P2 = 0x00;

				}
				else if(rwaddr == FC_APP_STATE)					// ��ȡ��Ƶ��״̬
				{  
				    CONTROL_FRAME_P1 = 0x02;
					CONTROL_FRAME_P3 = 0x01;

				    if(runFlag.bit.run)
				   	{
                        if (FORWARD_DIR == runFlag.bit.dir) // F0-12֮ǰ�ķ���
                       	{
							CONTROL_FRAME_P2 = 0x01;  //��ת
							//err = 0x02;
						}
						else
						{
                            CONTROL_FRAME_P2 = 0x02;  //��ת
                            //err = 0x02;
						}
					}
					else
					{
                        CONTROL_FRAME_P2 = 0x03;     //ͣ��
					}	
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// ��ȡAPP�����ӱ�ǩ
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#endif
/***********************************************************************************/
			default:
				err = 0x80;									// �������
				break;
		}
		if (err > 0)
		{
			CONTROL_FRAME_FC |= 0x80;						// �����Ƿ����
            CONTROL_FRAME_P2 = 0x80;
            CONTROL_FRAME_P1 = 0x01;
            CONTROL_FRAME_P4 = 0;
			CONTROL_FRAME_P3 = err;							// �������س�������
		}
        else if (sciFlag.bit.pwdPass)                      // ����ͨ�������� 0x8888
        {
            CONTROL_FRAME_P4 = 0x88;
			CONTROL_FRAME_P3 = 0x88;
        }
#if OSC_CON_CHECK											// CRC16У�鲢������Ӧ֡
        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
        {
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-1-2);//5���ֽ�
		    //CONTROL_FRAME_CRCL = crcResult;
		    //CONTROL_FRAME_CRCH = crcResult>>8;
		    CONTROL_FRAME_P4 = crcResult;
		    CONTROL_FRAME_CRCL = crcResult>>8;
		}
		else
		{
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);//6���ֽ�
		    CONTROL_FRAME_CRCL = crcResult;
		    CONTROL_FRAME_CRCH = crcResult>>8;
		    //CONTROL_FRAME_P4 = crcResult;
		    //CONTROL_FRAME_CRCL = crcResult>>8;
		}
#endif
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			APP_CONTROL_FRAME_DATA_NUM = 0x40;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 67);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 69);				// ����72�ֽ���Ӧ֡	
		}	
		else
#endif	

        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
       	{
		    SciDataTX(OscConFrameBuf, 8-1);						// ����7�ֽ���Ӧ֡	
		}
		else
		{
		    SciDataTX(OscConFrameBuf, 8);						// ����8�ֽ���Ӧ֡	
		}

#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))
		{
			volatile unsigned char *datatemp;
			//���û�йرտ��Ź�  ��Ҫ�ر�
			#if 	FC_CODE_BOOTPIN_EN		// ��װ��bootʹ�ܽ�ʱʹ��
			while(1);
			#else
			//   �ָ�״̬
			EALLOW;
			SysCtrlRegs.WDCR = 0x0068; 
			SciaRegs. SCIFFCT.all=0x4000; 
			SysCtrlRegs.CLKCTL.all = 0;
			EDIS;
			
			datatemp = &(SciTxBufStr.len);
			while(*datatemp);
			datatemp = &(SciTxBufStr.busy);
			while(*datatemp);
			while (0 == SciaRegs.SCICTL2.bit.TXEMPTY){}

			asm(" MOV        @SP,#0x0004");
			asm(" LCR        0x3ff7dd");
			asm(" MOV		XAR7,ACC");	
			asm(" LCR *XAR7");
			#endif
		}
#endif	
	}
}
#else   //MD380 
void SciDataRxDeal(void)
{
	Uint16 err = 0;
#if FC_CODE_CONTROL_EN
	Uint16 dat, rwaddr;
#endif

#if OSC_CON_CHECK
	Uint16  crcResult;
	crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
															// CRC16 У��
	if (crcResult != (Uint16)(( (CONTROL_FRAME_CRCH<<8) & 0xff00) | (CONTROL_FRAME_CRCL & 0x00ff) ))
	{
		return ;
	}
#endif

	if (CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD)			// ֡��ʼ����
#if	FC_CODE_APP_EN	 
	  if(CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD_APP)	
#endif
	{
		return;
	}
	if (CONTROL_FRAME_FC == FC_STOP_OSC)					// ֹͣʾ��������
	{
		OscDataBufA.rwPI = 0;
		OscDataBufB.rwPI = 0;
		OscDataBufA.full = 0;                               // ��ջ���
        OscDataBufB.full = 0;
		OscCongData.status = 0;
	}
	else if (OscCongData.status)							// ����������״̬
	{
		err = 0x80;											// �������
	}
	else
	{
		switch (CONTROL_FRAME_FC)							// �����ж�
		{
            case FC_COMM_TEST:                              // ͨѶ����
                // ��������κβ���


                break;

            case FC_CHANNL_SEL:								// ͨ������
				OscCongData.chSum = 0;
				
				OscCongData.ch1Addr = CONTROL_FRAME_P1;
				if (CONTROL_FRAME_P1)	OscCongData.chSum ++;
				OscCongData.ch2Addr = CONTROL_FRAME_P2;
				if (CONTROL_FRAME_P2)	OscCongData.chSum ++;
				OscCongData.ch3Addr = CONTROL_FRAME_P3;
				if (CONTROL_FRAME_P3)	OscCongData.chSum ++;
				OscCongData.ch4Addr = CONTROL_FRAME_P4;
				if (CONTROL_FRAME_P4)	OscCongData.chSum ++;
#if OSC_DATA_CHECK
				if (OscCongData.chSum == 3)
				{
					ClearOscBufEnd();						// ���4�ֽ�				
				}
#endif				
				break;
			
			case FC_PARA_CFG:								// �����ٶ�ѡ��
				if ((CONTROL_FRAME_P1>0) && (CONTROL_FRAME_P1<4))
				{
					OscCongData.baudRate = CONTROL_FRAME_P1;
					BaudCfg = 1;							// �޸�����						
				}
				if ((CONTROL_FRAME_P2>0) && (CONTROL_FRAME_P2<101))
				{
					OscCongData.interval = CONTROL_FRAME_P2;		
				}
//				OscCongData.runContinue = CONTROL_FRAME_P3;		
		
				break;
			case FC_START_OSC:								// ����ʾ����
				OscCongData.status = 1;
				break;
#if FC_CODE_CONTROL_EN
			case FC_READ_FC_DATA:							// ���ڲ�ӳ�书�������
/************************Һ������ʹ��****************************/
#if FC_KEY_CONTROL_EN
 dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
  if (FF_PASSWORD == dat)   // ����������ȷ
            {
                companyPwdPass4Comm = 1;
				userPwdPass4Comm = 1;
				commTicker = 0;   //�������㣬30����ֹ�޸�FF��
            }
	if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//Һ�����̶�FE��
	{
		LcdReadFEflag = 1;
	} 
		
			#endif
/****************************************************/
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
															// �ϳɵ�ַ��ȡ������
				err = OscControlRead(rwaddr, &dat);		    // ִ�в���
				CONTROL_FRAME_P3 = dat&0x00ff;				// ����ת����  ���ֽ�
				CONTROL_FRAME_P4 = dat>>8;					// ���ֽ�

				break;
			case FC_WRITE_FC_DATA:							// д�ڲ����������
				rwaddr =  ((CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);				
				dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
/************************Һ������ʹ��****************************/	
		#if FC_KEY_CONTROL_EN
			if((rwaddr==0x2000)&&(LcdKeyFlag==1))
			{
			    if(dat==1)
				{
				keyFunc =KEY_RUN;
			     break;
				 }
			 	if(dat==6)
				{
			   	 	keyFunc =KEY_STOP;
					break;
				}
            }
			if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//Һ�����̶�FE��
			{
				LcdReadFEflag = 1;
			}
	    #endif
/************************Һ������ʹ��****************************/				
				err = OscControlWriter(rwaddr, dat);		// ��������	
							
				break;
#endif
/************************Һ������ʹ��****************************/
#if FC_KEY_CONTROL_EN   	// ���м��̹��ܽ��ܣ���1��ʹ�ܣ���0���ر�	
			case FC_VERSION_DATA:							// �����͡��汾�Ų���							
				 LcdKeyFlag = 1;//����1��ʾҺ�����̽���
				 dat = 1;// 1����Һ��������MD380���ͺ�
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_LOWER_DATA:	
			 //	 keydata = CONTROL_FRAME_P2;
               dat= FuncCodeAdd(CONTROL_FRAME_P2);
			 //����ֵ�Ƿ����Ĺ������й���  
            if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.lowerLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
                dat = funcCode.all[dat];
			}
			else
			{
				dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
			}
				
		/*		 if (GetGradeIndex(dat, CONTROL_FRAME_P1) == 10)//f0-10����ֵ���⴦��
            {
                dat = 50 * decNumber[funcCode.code.frqPoint];
				CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
            }  else*/
			{
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
		    }
				 break;
			case FC_UPPER_DATA:							// д�ڲ�����ֵ����							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
				 if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.upperLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
                dat = funcCode.all[dat];
			}
			else
			{
			 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
			} 
				 
				
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_INIT_DATA:
				dat= FuncCodeAdd(CONTROL_FRAME_P2);						// ���ڲ�����ֵ����							
				 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].init;
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_DATA:							// ���ڲ����Բ���							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
			dat =	upsend(GetGradeIndex(dat, CONTROL_FRAME_P1));
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_UDATA:
               dat =dispAttributeU0[CONTROL_FRAME_P1].all;
			   CONTROL_FRAME_P3 = dat&0x00ff;
			   CONTROL_FRAME_P4 = dat>>8;
			break;
#endif
#if	FC_CODE_APP_EN
			case FC_APP_WRITE:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// ��ת��BOOT��
				{
					// ��¼��װ������boot��ʱΪ1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 1;
					else
					{
						EALLOW;//����������Ϊ�������ţ�����Ҫ�ָ������
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 2;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 1;
					err = 0x0;
				#endif
				}
				else
				{
					if(rwaddr == FC_APP_POPEDOM)			// ��ȡ��¼Ȩ��
					{
						// ---�����û������ж�Ȩ��
						CONTROL_FRAME_P3 = 1;				// 1 ������0 δ����
						CONTROL_FRAME_P4 = *(Uint16 *)(0x3FFFBA);
					}
					else
					{
						CONTROL_FRAME_P3 = 0;
						err = 0x02;
					}
				}
				break;
			case FC_APP_READ:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_STATE)					// ��ȡ��Ƶ��״̬
				{
					if(runFlag.bit.run)
					{
					    CONTROL_FRAME_P3 = 1;					// 1 ���У�3 ֹͣ
						err = 0x02;
					}
					else
					{
					    CONTROL_FRAME_P3 = 3;					// 1 ���У�3 ֹͣ
					}
					
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// ��ȡAPP�����ӱ�ǩ
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#endif
/***********************************************************************************/
			default:
				err = 0x80;									// �������
				break;
		}
		if (err > 0)
		{
			CONTROL_FRAME_FC |= 0x80;						// �����Ƿ����
            CONTROL_FRAME_P2 = 0x80;
            CONTROL_FRAME_P1 = 0x01;
            CONTROL_FRAME_P4 = 0;
			CONTROL_FRAME_P3 = err;							// �������س�������
		}
        else if (sciFlag.bit.pwdPass)                      // ����ͨ�������� 0x8888
        {
            CONTROL_FRAME_P4 = 0x88;
			CONTROL_FRAME_P3 = 0x88;
        }

#if OSC_CON_CHECK											// CRC16У�鲢������Ӧ֡
		crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
		CONTROL_FRAME_CRCL = crcResult;
		CONTROL_FRAME_CRCH = crcResult>>8;
#endif
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 130);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 132);				// ����132�ֽ���Ӧ֡	
		}
		else
#endif	
		SciDataTX(OscConFrameBuf, 8);						// ����8�ֽ���Ӧ֡	
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))
		{
			volatile unsigned char *datatemp;
			//���û�йرտ��Ź�  ��Ҫ�ر�
			#if 	FC_CODE_BOOTPIN_EN		// ��װ��bootʹ�ܽ�ʱʹ��
			while(1);
			#else
			//   �ָ�״̬
			EALLOW;
			SysCtrlRegs.WDCR = 0x0068; 
			SciaRegs. SCIFFCT.all=0x4000; 
			SysCtrlRegs.CLKCTL.all = 0;
			EDIS;
			
			datatemp = &(SciTxBufStr.len);
			while(*datatemp);
			datatemp = &(SciTxBufStr.busy);
			while(*datatemp);
			while (0 == SciaRegs.SCICTL2.bit.TXEMPTY){}

			asm(" MOV        @SP,#0x0004");
			asm(" LCR        0x3ff7dd");
			asm(" MOV		XAR7,ACC");	
			asm(" LCR *XAR7");
			#endif
		}
#endif	
	}

}
#endif

#if 0
void SciDataRxDeal(void)
{
	Uint16 err = 0;
#if FC_CODE_CONTROL_EN
	Uint16 dat, rwaddr;
#endif

#if OSC_CON_CHECK
	Uint16  crcResult;
	crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
															// CRC16 У��
	if (crcResult != (Uint16)(( (CONTROL_FRAME_CRCH<<8) & 0xff00) | (CONTROL_FRAME_CRCL & 0x00ff) ))
	{
		return ;
	}
#endif

	if (CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD)			// ֡��ʼ����
#if	FC_CODE_APP_EN	 
	  if(CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD_APP)	
#endif
	{
		return;
	}


	if (CONTROL_FRAME_FC == FC_STOP_OSC)					// ֹͣʾ��������
	{
		OscDataBufA.rwPI = 0;
		OscDataBufB.rwPI = 0;
		OscDataBufA.full = 0;                               // ��ջ���
        OscDataBufB.full = 0;
		OscCongData.status = 0;
	}
	else if (OscCongData.status)							// ����������״̬
	{
		err = 0x80;											// �������
	}
	else
	{
		switch (CONTROL_FRAME_FC)							// �����ж�
		{
            case FC_COMM_TEST:                              // ͨѶ����
                // ��������κβ���


                break;

            case FC_CHANNL_SEL:								// ͨ������
				OscCongData.chSum = 0;
				
				OscCongData.ch1Addr = CONTROL_FRAME_P1;
				if (CONTROL_FRAME_P1)	OscCongData.chSum ++;
				OscCongData.ch2Addr = CONTROL_FRAME_P2;
				if (CONTROL_FRAME_P2)	OscCongData.chSum ++;
				OscCongData.ch3Addr = CONTROL_FRAME_P3;
				if (CONTROL_FRAME_P3)	OscCongData.chSum ++;
				OscCongData.ch4Addr = CONTROL_FRAME_P4;
				if (CONTROL_FRAME_P4)	OscCongData.chSum ++;
#if OSC_DATA_CHECK
				if (OscCongData.chSum == 3)
				{
					ClearOscBufEnd();						// ���4�ֽ�				
				}
#endif				
				break;
			
			case FC_PARA_CFG:								// �����ٶ�ѡ��
				if ((CONTROL_FRAME_P1>0) && (CONTROL_FRAME_P1<4))
				{
					OscCongData.baudRate = CONTROL_FRAME_P1;
					BaudCfg = 1;							// �޸�����						
				}
				if ((CONTROL_FRAME_P2>0) && (CONTROL_FRAME_P2<101))
				{
					OscCongData.interval = CONTROL_FRAME_P2;		
				}
//				OscCongData.runContinue = CONTROL_FRAME_P3;		
		
				break;
			case FC_START_OSC:								// ����ʾ����
				OscCongData.status = 1;
				break;
#if FC_CODE_CONTROL_EN
			case FC_READ_FC_DATA:							// ���ڲ�ӳ�书�������
/************************Һ������ʹ��****************************/
#if FC_KEY_CONTROL_EN
 dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
  if (FF_PASSWORD == dat)   // ����������ȷ
            {
                companyPwdPass4Comm = 1;
				userPwdPass4Comm = 1;
				commTicker = 0;   //�������㣬30����ֹ�޸�FF��
            }
	if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//Һ�����̶�FE��
	{
		LcdReadFEflag = 1;
	} 
		
			#endif
/****************************************************/
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
															// �ϳɵ�ַ��ȡ������
				err = OscControlRead(rwaddr, &dat);		    // ִ�в���
				CONTROL_FRAME_P3 = dat&0x00ff;				// ����ת����  ���ֽ�
				CONTROL_FRAME_P4 = dat>>8;					// ���ֽ�

				break;
			case FC_WRITE_FC_DATA:							// д�ڲ����������
				rwaddr =  ((CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);				
				dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
/************************Һ������ʹ��****************************/	
		#if FC_KEY_CONTROL_EN
			if((rwaddr==0x2000)&&(LcdKeyFlag==1))
			{
			    if(dat==1)
				{
				keyFunc =KEY_RUN;
			     break;
				 }
			 	if(dat==6)
				{
			   	 	keyFunc =KEY_STOP;
					break;
				}
            }
			if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//Һ�����̶�FE��
			{
				LcdReadFEflag = 1;
			}
	    #endif
/************************Һ������ʹ��****************************/				
				err = OscControlWriter(rwaddr, dat);		// ��������	
							
				break;
#endif
/************************Һ������ʹ��****************************/
#if FC_KEY_CONTROL_EN   	// ���м��̹��ܽ��ܣ���1��ʹ�ܣ���0���ر�	
			case FC_VERSION_DATA:							// �����͡��汾�Ų���							
				 LcdKeyFlag = 1;//����1��ʾҺ�����̽���
				 dat = 1;// 1����Һ��������MD380���ͺ�
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_LOWER_DATA:	
			 //	 keydata = CONTROL_FRAME_P2;
               dat= FuncCodeAdd(CONTROL_FRAME_P2);
			 //����ֵ�Ƿ����Ĺ������й���  
            if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.lowerLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
                dat = funcCode.all[dat];
			}
			else
			{
				dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
			}
				
		/*		 if (GetGradeIndex(dat, CONTROL_FRAME_P1) == 10)//f0-10����ֵ���⴦��
            {
                dat = 50 * decNumber[funcCode.code.frqPoint];
				CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
            }  else*/
			{
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
		    }
				 break;
			case FC_UPPER_DATA:							// д�ڲ���ֵ����							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
				 if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.upperLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
                dat = funcCode.all[dat];
			}
			else
			{
			 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
			} 
				 
				
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_INIT_DATA:
				dat= FuncCodeAdd(CONTROL_FRAME_P2);						// ���ڲ�����ֵ����							
				 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].init;
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_DATA:							// ���ڲ����Բ���							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
			dat =	upsend(GetGradeIndex(dat, CONTROL_FRAME_P1));
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_UDATA:
               dat =dispAttributeU0[CONTROL_FRAME_P1].all;
			   CONTROL_FRAME_P3 = dat&0x00ff;
			   CONTROL_FRAME_P4 = dat>>8;
			break;
#endif
#if	FC_CODE_APP_EN

#if DEBUG_MD500_SEARIS
            case FC_APP_READ_WRITE:
                rwaddr = ( (CONTROL_FRAME_P1<<8) & 0xff00) | (CONTROL_FRAME_P2 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// ��ת��BOOT��
				{
					// ��¼��װ������boot��ʱΪ1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 0x01;
					else
					{
						EALLOW;//����������Ϊ�������ţ�����Ҫ�ָ������
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 0x02;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 0x01;
					err = 0x0;
				#endif
				
				    CONTROL_FRAME_P1 = 0x02;
				    CONTROL_FRAME_P2 = 0x00;

				}
				else if(rwaddr == FC_APP_STATE)					// ��ȡ��Ƶ��״̬
				{  
				    CONTROL_FRAME_P1 = 0x02;
					CONTROL_FRAME_P3 = 0x01;

				    if(runFlag.bit.run)
				   	{
                        if (FORWARD_DIR == runFlag.bit.dir) // F0-12֮ǰ�ķ���
                       	{
							CONTROL_FRAME_P2 = 0x01;  //��ת
							//err = 0x02;
						}
						else
						{
                            CONTROL_FRAME_P2 = 0x02;  //��ת
                            //err = 0x02;
						}
					}
					else
					{
                        CONTROL_FRAME_P2 = 0x03;     //ͣ��
					}	
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// ��ȡAPP�����ӱ�ǩ
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#else //MD380

			case FC_APP_WRITE:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// ��ת��BOOT��
				{
					// ��¼��װ������boot��ʱΪ1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 1;
					else
					{
						EALLOW;//����������Ϊ�������ţ�����Ҫ�ָ������
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 2;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 1;
					err = 0x0;
				#endif
				}
				else
				{
					if(rwaddr == FC_APP_POPEDOM)			// ��ȡ��¼Ȩ��
					{
						// ---�����û������ж�Ȩ��
						CONTROL_FRAME_P3 = 1;				// 1 ������0 δ����
						CONTROL_FRAME_P4 = *(Uint16 *)(0x3FFFBA);
					}
					else
					{
						CONTROL_FRAME_P3 = 0;
						err = 0x02;
					}
				}
				break;
			case FC_APP_READ:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_STATE)					// ��ȡ��Ƶ��״̬
				{
					if(runFlag.bit.run)
					{
					    CONTROL_FRAME_P3 = 1;					// 1 ���У�3 ֹͣ
						err = 0x02;
					}
					else
					{
					    CONTROL_FRAME_P3 = 3;					// 1 ���У�3 ֹͣ
					}
					
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// ��ȡAPP�����ӱ�ǩ
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#endif

#endif
/***********************************************************************************/
			default:
				err = 0x80;									// �������
				break;
		}
		if (err > 0)
		{
			CONTROL_FRAME_FC |= 0x80;						// �����Ƿ����
            CONTROL_FRAME_P2 = 0x80;
            CONTROL_FRAME_P1 = 0x01;
            CONTROL_FRAME_P4 = 0;
			CONTROL_FRAME_P3 = err;							// �������س�������
		}
        else if (sciFlag.bit.pwdPass)                      // ����ͨ�������� 0x8888
        {
            CONTROL_FRAME_P4 = 0x88;
			CONTROL_FRAME_P3 = 0x88;
        }

#if OSC_CON_CHECK											// CRC16У�鲢������Ӧ֡
#if DEBUG_MD500_SEARIS
        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
        {
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-1-2);//5���ֽ�
		    //CONTROL_FRAME_CRCL = crcResult;
		    //CONTROL_FRAME_CRCH = crcResult>>8;
		    CONTROL_FRAME_P4 = crcResult;
		    CONTROL_FRAME_CRCL = crcResult>>8;
		}
		else
		{
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);//6���ֽ�
		    CONTROL_FRAME_CRCL = crcResult;
		    CONTROL_FRAME_CRCH = crcResult>>8;
		    //CONTROL_FRAME_P4 = crcResult;
		    //CONTROL_FRAME_CRCL = crcResult>>8;
		}
		
#else
		crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);//6���ֽ�
		CONTROL_FRAME_CRCL = crcResult;
		CONTROL_FRAME_CRCH = crcResult>>8;
#endif
#endif
#if	FC_CODE_APP_EN
#if DEBUG_MD500_SEARIS
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			APP_CONTROL_FRAME_DATA_NUM = 0x40;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 67);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 69);				// ����72�ֽ���Ӧ֡	
		}
#else   //MD380
		if((CONTROL_FRAME_FC == FC_APP_READ) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 130);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 132);				// ����132�ֽ���Ӧ֡	
		}
#endif		
		else

#endif	
#if DEBUG_MD500_SEARIS
        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
       	{
		    SciDataTX(OscConFrameBuf, 8-1);						// ����7�ֽ���Ӧ֡	
		}
		else
		{
		    SciDataTX(OscConFrameBuf, 8);						// ����8�ֽ���Ӧ֡	
		}
#else
		SciDataTX(OscConFrameBuf, 8);						// ����8�ֽ���Ӧ֡	
#endif
#if	FC_CODE_APP_EN

#if DEBUG_MD500_SEARIS
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))

#else
		if((CONTROL_FRAME_FC == FC_APP_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))
#endif
		{
			volatile unsigned char *datatemp;
			//���û�йرտ��Ź�  ��Ҫ�ر�
			#if 	FC_CODE_BOOTPIN_EN		// ��װ��bootʹ�ܽ�ʱʹ��
			while(1);
			#else
			//   �ָ�״̬
			EALLOW;
			SysCtrlRegs.WDCR = 0x0068; 
			SciaRegs. SCIFFCT.all=0x4000; 
			SysCtrlRegs.CLKCTL.all = 0;
			EDIS;
			
			datatemp = &(SciTxBufStr.len);
			while(*datatemp);
			datatemp = &(SciTxBufStr.busy);
			while(*datatemp);
			while (0 == SciaRegs.SCICTL2.bit.TXEMPTY){}

			asm(" MOV        @SP,#0x0004");
			asm(" LCR        0x3ff7dd");
			asm(" MOV		XAR7,ACC");	
			asm(" LCR *XAR7");
			#endif
		}
#endif	
	}
}
#endif

#if 0
void SciDataRxDeal(void)
{
	Uint16 err = 0;
#if FC_CODE_CONTROL_EN
	Uint16 dat, rwaddr;
#endif

#if OSC_CON_CHECK
	Uint16  crcResult;
	crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
															// CRC16 У��
	if (crcResult != (Uint16)(( (CONTROL_FRAME_CRCH<<8) & 0xff00) | (CONTROL_FRAME_CRCL & 0x00ff) ))
	{
		return ;
	}
#endif

	if (CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD)			// ֡��ʼ����
#if	FC_CODE_APP_EN	 
	  if(CONTROL_FRAME_HEAD != OSC_CON_FRAME_HEAD_APP)	
#endif
	{
		return;
	}


	if (CONTROL_FRAME_FC == FC_STOP_OSC)					// ֹͣʾ��������
	{
		OscDataBufA.rwPI = 0;
		OscDataBufB.rwPI = 0;
		OscDataBufA.full = 0;                               // ��ջ���
        OscDataBufB.full = 0;
		OscCongData.status = 0;
	}
	else if (OscCongData.status)							// ����������״̬
	{
		err = 0x80;											// �������
	}
	else
	{
		switch (CONTROL_FRAME_FC)							// �����ж�
		{
            case FC_COMM_TEST:                              // ͨѶ����
                // ��������κβ���


                break;

            case FC_CHANNL_SEL:								// ͨ������
				OscCongData.chSum = 0;
				
				OscCongData.ch1Addr = CONTROL_FRAME_P1;
				if (CONTROL_FRAME_P1)	OscCongData.chSum ++;
				OscCongData.ch2Addr = CONTROL_FRAME_P2;
				if (CONTROL_FRAME_P2)	OscCongData.chSum ++;
				OscCongData.ch3Addr = CONTROL_FRAME_P3;
				if (CONTROL_FRAME_P3)	OscCongData.chSum ++;
				OscCongData.ch4Addr = CONTROL_FRAME_P4;
				if (CONTROL_FRAME_P4)	OscCongData.chSum ++;
#if OSC_DATA_CHECK
				if (OscCongData.chSum == 3)
				{
					ClearOscBufEnd();						// ���4�ֽ�				
				}
#endif				
				break;
			
			case FC_PARA_CFG:								// �����ٶ�ѡ��
				if ((CONTROL_FRAME_P1>0) && (CONTROL_FRAME_P1<4))
				{
					OscCongData.baudRate = CONTROL_FRAME_P1;
					BaudCfg = 1;							// �޸�����						
				}
				if ((CONTROL_FRAME_P2>0) && (CONTROL_FRAME_P2<101))
				{
					OscCongData.interval = CONTROL_FRAME_P2;		
				}
//				OscCongData.runContinue = CONTROL_FRAME_P3;		
		
				break;
			case FC_START_OSC:								// ����ʾ����
				OscCongData.status = 1;
				break;
#if FC_CODE_CONTROL_EN
			case FC_READ_FC_DATA:							// ���ڲ�ӳ�书�������
/************************Һ������ʹ��****************************/
#if FC_KEY_CONTROL_EN
 dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
  if (FF_PASSWORD == dat)   // ����������ȷ
            {
                companyPwdPass4Comm = 1;
				userPwdPass4Comm = 1;
				commTicker = 0;   //�������㣬30����ֹ�޸�FF��
            }
	if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//Һ�����̶�FE��
	{
		LcdReadFEflag = 1;
	} 
		
			#endif
/****************************************************/
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
															// �ϳɵ�ַ��ȡ������
				err = OscControlRead(rwaddr, &dat);		    // ִ�в���
				CONTROL_FRAME_P3 = dat&0x00ff;				// ����ת����  ���ֽ�
				CONTROL_FRAME_P4 = dat>>8;					// ���ֽ�

				break;
			case FC_WRITE_FC_DATA:							// д�ڲ����������
				rwaddr =  ((CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);				
				dat = ( (CONTROL_FRAME_P4<<8) & 0xff00) | (CONTROL_FRAME_P3 & 0x00ff);
/************************Һ������ʹ��****************************/	
		#if FC_KEY_CONTROL_EN
			if((rwaddr==0x2000)&&(LcdKeyFlag==1))
			{
			    if(dat==1)
				{
				keyFunc =KEY_RUN;
			     break;
				 }
			 	if(dat==6)
				{
			   	 	keyFunc =KEY_STOP;
					break;
				}
            }
			if((LcdKeyFlag ==1)&&(CONTROL_FRAME_P2==0xfe))//Һ�����̶�FE��
			{
				LcdReadFEflag = 1;
			}
	    #endif
/************************Һ������ʹ��****************************/				
				err = OscControlWriter(rwaddr, dat);		// ��������	
							
				break;
#endif
/************************Һ������ʹ��****************************/
#if FC_KEY_CONTROL_EN   	// ���м��̹��ܽ��ܣ���1��ʹ�ܣ���0���ر�	
			case FC_VERSION_DATA:							// �����͡��汾�Ų���							
				 LcdKeyFlag = 1;//����1��ʾҺ�����̽���
				 dat = 1;// 1����Һ��������MD380���ͺ�
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_LOWER_DATA:	
			 //	 keydata = CONTROL_FRAME_P2;
               dat= FuncCodeAdd(CONTROL_FRAME_P2);
			 //����ֵ�Ƿ����Ĺ������й���  
            if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.lowerLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
                dat = funcCode.all[dat];
			}
			else
			{
				dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].lower;
			}
				
		/*		 if (GetGradeIndex(dat, CONTROL_FRAME_P1) == 10)//f0-10����ֵ���⴦��
            {
                dat = 50 * decNumber[funcCode.code.frqPoint];
				CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
            }  else*/
			{
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
		    }
				 break;
			case FC_UPPER_DATA:							// д�ڲ�����ֵ����							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
				 if (funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].attribute.bit.upperLimit)
               { 
                dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
                dat = funcCode.all[dat];
			}
			else
			{
			 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].upper;
			} 
				 
				
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_INIT_DATA:
				dat= FuncCodeAdd(CONTROL_FRAME_P2);						// ���ڲ�����ֵ����							
				 dat = funcCodeAttribute[GetGradeIndex(dat, CONTROL_FRAME_P1)].init;
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_DATA:							// ���ڲ����Բ���							
			dat= FuncCodeAdd(CONTROL_FRAME_P2);
			dat =	upsend(GetGradeIndex(dat, CONTROL_FRAME_P1));
				 CONTROL_FRAME_P3 = dat&0x00ff;
				 CONTROL_FRAME_P4 = dat>>8;
				 break;
			case FC_ATTRIBUTE_UDATA:
               dat =dispAttributeU0[CONTROL_FRAME_P1].all;
			   CONTROL_FRAME_P3 = dat&0x00ff;
			   CONTROL_FRAME_P4 = dat>>8;
			break;
#endif

#if	FC_CODE_APP_EN
#if DEBUG_MD500_SEARIS
            case FC_APP_READ_WRITE:
                rwaddr = ( (CONTROL_FRAME_P1<<8) & 0xff00) | (CONTROL_FRAME_P2 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// ��ת��BOOT��
				{
					// ��¼��װ������boot��ʱΪ1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 0x01;
					else
					{
						EALLOW;//����������Ϊ�������ţ�����Ҫ�ָ������
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 0x02;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 0x01;
					err = 0x0;
				#endif
				
				    CONTROL_FRAME_P1 = 0x02;
				    CONTROL_FRAME_P2 = 0x00;

				}
				else if(rwaddr == FC_APP_STATE)					// ��ȡ��Ƶ��״̬
				{  
				    CONTROL_FRAME_P1 = 0x02;
					CONTROL_FRAME_P3 = 0x01;

				    if(runFlag.bit.run)
				   	{
                        if (FORWARD_DIR == runFlag.bit.dir) // F0-12֮ǰ�ķ���
                       	{
							CONTROL_FRAME_P2 = 0x01;  //��ת
							//err = 0x02;
						}
						else
						{
                            CONTROL_FRAME_P2 = 0x02;  //��ת
                            //err = 0x02;
						}
					}
					else
					{
                        CONTROL_FRAME_P2 = 0x03;     //ͣ��
					}	
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// ��ȡAPP�����ӱ�ǩ
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#else //MD380

			case FC_APP_WRITE:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_RETURN_BOOT)			// ��ת��BOOT��
				{
					// ��¼��װ������boot��ʱΪ1
				#if 	FC_CODE_BOOTPIN_EN
					EALLOW;
					//SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1; 
					GpioDataRegs.GPBDAT.bit.GPIO37 = 0;     // 
					GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;     // input
					GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37
					EDIS;
					if(GpioDataRegs.GPBDAT.bit.GPIO37 == 0)
						CONTROL_FRAME_P3 = 1;
					else
					{
						EALLOW;//����������Ϊ�������ţ�����Ҫ�ָ������
						GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0;    // Configure GPIO37, FAN
                        GpioCtrlRegs.GPBDIR.bit.GPIO37 = 1;     // output
                        GpioDataRegs.GPBDAT.bit.GPIO37 = 1;     // 
                        EDIS;
                        CONTROL_FRAME_P3 = 2;
						err = 0x02;
					}
				#else
					CONTROL_FRAME_P3 = 1;
					err = 0x0;
				#endif
				}
				else
				{
					if(rwaddr == FC_APP_POPEDOM)			// ��ȡ��¼Ȩ��
					{
						// ---�����û������ж�Ȩ��
						CONTROL_FRAME_P3 = 1;				// 1 ������0 δ����
						CONTROL_FRAME_P4 = *(Uint16 *)(0x3FFFBA);
					}
					else
					{
						CONTROL_FRAME_P3 = 0;
						err = 0x02;
					}
				}
				break;
			case FC_APP_READ:
				rwaddr = ( (CONTROL_FRAME_P2<<8) & 0xff00) | (CONTROL_FRAME_P1 & 0x00ff);
				if(rwaddr == FC_APP_STATE)					// ��ȡ��Ƶ��״̬
				{
					if(runFlag.bit.run)
					{
					    CONTROL_FRAME_P3 = 1;					// 1 ���У�3 ֹͣ
						err = 0x02;
					}
					else
					{
					    CONTROL_FRAME_P3 = 3;					// 1 ���У�3 ֹͣ
					}
					
				}
				else
				{
					if(rwaddr == FC_APP_LABEL)				// ��ȡAPP�����ӱ�ǩ
					{
						CONTROL_FRAME_P3 = 0;
					}
					else
					{
						CONTROL_FRAME_P3 = 1;
						err = 0x02;
					}
				}
				break;
#endif

#endif
/***********************************************************************************/
			default:
				err = 0x80;									// �������
				break;
		}
		if (err > 0)
		{
			CONTROL_FRAME_FC |= 0x80;						// �����Ƿ����
            CONTROL_FRAME_P2 = 0x80;
            CONTROL_FRAME_P1 = 0x01;
            CONTROL_FRAME_P4 = 0;
			CONTROL_FRAME_P3 = err;							// �������س�������
		}
        else if (sciFlag.bit.pwdPass)                      // ����ͨ�������� 0x8888
        {
            CONTROL_FRAME_P4 = 0x88;
			CONTROL_FRAME_P3 = 0x88;
        }
#if DEBUG_MD500_SEARIS//yz-------------------------
#if OSC_CON_CHECK											// CRC16У�鲢������Ӧ֡
        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
        {
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-1-2);//5���ֽ�
		    //CONTROL_FRAME_CRCL = crcResult;
		    //CONTROL_FRAME_CRCH = crcResult>>8;
		    CONTROL_FRAME_P4 = crcResult;
		    CONTROL_FRAME_CRCL = crcResult>>8;
		}
		else
		{
            crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);//6���ֽ�
		    CONTROL_FRAME_CRCL = crcResult;
		    CONTROL_FRAME_CRCH = crcResult>>8;
		    //CONTROL_FRAME_P4 = crcResult;
		    //CONTROL_FRAME_CRCL = crcResult>>8;
		}
#endif
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			APP_CONTROL_FRAME_DATA_NUM = 0x40;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 67);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 69);				// ����72�ֽ���Ӧ֡	
		}	
		else
#endif	

        if(CONTROL_FRAME_FC == FC_APP_READ_WRITE)
       	{
		    SciDataTX(OscConFrameBuf, 8-1);						// ����7�ֽ���Ӧ֡	
		}
		else
		{
		    SciDataTX(OscConFrameBuf, 8);						// ����8�ֽ���Ӧ֡	
		}

#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))
		{
			volatile unsigned char *datatemp;
			//���û�йرտ��Ź�  ��Ҫ�ر�
			#if 	FC_CODE_BOOTPIN_EN		// ��װ��bootʹ�ܽ�ʱʹ��
			while(1);
			#else
			//   �ָ�״̬
			EALLOW;
			SysCtrlRegs.WDCR = 0x0068; 
			SciaRegs. SCIFFCT.all=0x4000; 
			SysCtrlRegs.CLKCTL.all = 0;
			EDIS;
			
			datatemp = &(SciTxBufStr.len);
			while(*datatemp);
			datatemp = &(SciTxBufStr.busy);
			while(*datatemp);
			while (0 == SciaRegs.SCICTL2.bit.TXEMPTY){}

			asm(" MOV        @SP,#0x0004");
			asm(" LCR        0x3ff7dd");
			asm(" MOV		XAR7,ACC");	
			asm(" LCR *XAR7");
			#endif
		}
#endif	

//-----------------------------------------yz
#else //MD380
///////////yz
#if OSC_CON_CHECK											// CRC16У�鲢������Ӧ֡
		crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameBuf, FC_FRAME_LEN-2);
		CONTROL_FRAME_CRCL = crcResult;
		CONTROL_FRAME_CRCH = crcResult>>8;
#endif
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_READ) && (rwaddr == FC_APP_LABEL))
		{
			APP_CONTROL_FRAME_HEAD = CONTROL_FRAME_HEAD;
			APP_CONTROL_FRAME_FC = CONTROL_FRAME_FC;
			copyAPPtoBuff();
			crcResult = CrcValueByteCalc((const Uint16 *)OscConFrameAPPBuf, 130);
			APP_CONTROL_FRAME_CRCL = crcResult;
			APP_CONTROL_FRAME_CRCH = crcResult>>8;
			SciDataTX(OscConFrameAPPBuf, 132);				// ����132�ֽ���Ӧ֡	
		}
		else
#endif	
		SciDataTX(OscConFrameBuf, 8);						// ����8�ֽ���Ӧ֡	
#if	FC_CODE_APP_EN
		if((CONTROL_FRAME_FC == FC_APP_WRITE) && (rwaddr == FC_APP_RETURN_BOOT))
		{
			volatile unsigned char *datatemp;
			//���û�йرտ��Ź�  ��Ҫ�ر�
			#if 	FC_CODE_BOOTPIN_EN		// ��װ��bootʹ�ܽ�ʱʹ��
			while(1);
			#else
			//   �ָ�״̬
			EALLOW;
			SysCtrlRegs.WDCR = 0x0068; 
			SciaRegs. SCIFFCT.all=0x4000; 
			SysCtrlRegs.CLKCTL.all = 0;
			EDIS;
			
			datatemp = &(SciTxBufStr.len);
			while(*datatemp);
			datatemp = &(SciTxBufStr.busy);
			while(*datatemp);
			while (0 == SciaRegs.SCICTL2.bit.TXEMPTY){}

			asm(" MOV        @SP,#0x0004");
			asm(" LCR        0x3ff7dd");
			asm(" MOV		XAR7,ACC");	
			asm(" LCR *XAR7");
			#endif
		}
#endif	

#endif
	}
//yz/////////////
}
#endif
#if	FC_CODE_APP_EN
void copyAPPtoBuff(void)
{
	uint8 i;
	uint16 *ptr;
	ptr = (uint16 *)APPIDDataBuff;
	
#if DEBUG_MD500_SEARIS||DEBUG_MD290_SEARIS||DEBUG_MD380E_SEARIS
    for (i=0;i<64;i++)
	{
		if(i<16)
		{
			OscConFrameAPPBuf[3+i++] = *ptr>>8;
			OscConFrameAPPBuf[3+i]   = *ptr++;
		}
		else
		{
			OscConFrameAPPBuf[3+i++] = 0xff;
			OscConFrameAPPBuf[3+i] = 0xff;
		}
		
	}
#else
	for (i=0;i<128;i++)
	{
		if(i<16)
		{
			OscConFrameAPPBuf[2+i++] = *ptr;
			OscConFrameAPPBuf[2+i] = (*ptr++)>>8;
		}
		else
		{
			OscConFrameAPPBuf[2+i++] = 0xff;
			OscConFrameAPPBuf[2+i] = 0xff;
		}
		
	}
#endif
}
#endif	
/*******************************************************************************
* ��������          : void OscDataQcq(void)
* ��ڲ���			����
* ����				����
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: ���趨�Ĳ������������д��ɼ�˫����
* 
********************************************************************************/
void OscDataQcq(void)
{
	static uint8 count = 0;
	static uint8 bufSel = 0;
//	static uint8 check = 0;
	uint8 *dataBuf;
	
	if (OscCongData.status != 1)							// ֹͣ״̬
	{
		bufSel = 0;											// ����
		count = 0;
		OscDataBufA.rwPI = 0;								// ��ջ���
		OscDataBufA.full = 0;	
		OscDataBufB.rwPI = 0;
		OscDataBufB.full = 0;	
		return;
	}
	count ++;
	if (count == OscCongData.interval)						// �������ڲ������
	{
		if (bufSel == 0)									// ��������A
		{
#if OSC_DATA_CHECK
			if (OscDataBufA.rwPI == 0)
				OscDataBufA.check = OSC_DATA_FRAME_HEAD_A ^ OSC_DATA_FRAME_HEAD_B;			// ͷУ��
#endif			
			dataBuf = (uint8 *)(&(OscDataBufA.oscDataBuf[OscDataBufA.rwPI]) );			
			OscDataBufA.check ^= GetOscData(dataBuf);		// ��ȡ����ȡ��У��
			OscDataBufA.rwPI += OscCongData.chSum << 1;		// �޸Ļ���ָ��
			if (OscDataBufA.rwPI > (OSC_BUF_DATA_LEN - (OscCongData.chSum << 1) ) )	
			{												// ������
				OscDataBufA.rwPI = 0;
				OscDataBufA.full = 1;
				bufSel = 1;
			}	
		}
		else												// ��������B
		{
#if OSC_DATA_CHECK			
			if (OscDataBufB.rwPI == 0)
				OscDataBufB.check = OSC_DATA_FRAME_HEAD_A ^ OSC_DATA_FRAME_HEAD_B;			// ͷУ��
#endif			
			dataBuf = (uint8 *)(&OscDataBufB.oscDataBuf[OscDataBufB.rwPI]);			
			OscDataBufB.check ^= GetOscData(dataBuf);		// ��ȡ����ȡ��У��
			OscDataBufB.rwPI += OscCongData.chSum << 1;		// 
			if (OscDataBufB.rwPI > (OSC_BUF_DATA_LEN - (OscCongData.chSum << 1) ) )	
			{												// ������
				OscDataBufB.rwPI = 0;
				OscDataBufB.full = 1;
				bufSel = 0;	
			}		
		}
		count = 0;
	}
}
/*******************************************************************************
* ��������          : void OscDataTxDeal(void)
* ��ڲ���			����
* ����				����
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: ʾ�������ݷ��ʹ�������⵱ǰ״̬����ʾ��������
* 
********************************************************************************/
void OscDataTxDeal(void)
{
	static uint8  frame = 0;
	static uint8  bufSel = 0;
	if (OscCongData.status != 1)							// ֹͣ״̬
	{
		bufSel = 0;											// ����
		return;
	}
	
	if (bufSel == 0)
	{
		if (OscDataBufA.full)								// ����A��
		{
			OscDataBufA.frameNum = frame;
#if OSC_DATA_CHECK
			OscDataBufA.check ^= frame ;
#endif
//			OscDataCk( (uint8 *)(&OscDataBufA), 67);
			if (SciDataTX((uint8 *)(&OscDataBufA), 68))
			{
				OscDataBufA.full = 0;						// ���ͻ���A
				frame++;
				bufSel = 1;			
			}		
		}
		return;
	}
	if (OscDataBufB.full)
	{
		OscDataBufB.frameNum = frame;
#if OSC_DATA_CHECK
			OscDataBufB.check ^= frame;
#endif		
//		OscDataCk( (uint8 *)(&OscDataBufB), 67);

		if (SciDataTX((uint8 *)(&OscDataBufB), 68))
		{
			OscDataBufB.full = 0;
			frame++;
			bufSel = 0;		
		}
	}
}

#if OSC_DATA_CHECK
	#define OSC_GET_CHECK		check ^= *databuf;	databuf++
#else
	#define OSC_GET_CHECK		databuf++
#endif

/*******************************************************************************
* ��������          : uint8 GetOscData(Uint16 *databuf)
* ��ڲ���			��databuf		����ָ��
* ����				����������У��ֵ
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: ����ʾ��������
* 
********************************************************************************/
uint8 GetOscData(uint8 *databuf)
{
	uint8 check = 0;;
	if (OscCongData.ch1Addr)								// ͨ��1����
	{
		*databuf = GET_OSC_DATA(OscCongData.ch1Addr);// & 0xff;
		OSC_GET_CHECK;
		*databuf = GET_OSC_DATA(OscCongData.ch1Addr) >> 8;
		OSC_GET_CHECK;
	}
	if (OscCongData.ch2Addr)
	{
		*databuf = GET_OSC_DATA(OscCongData.ch2Addr);// & 0xff;
		OSC_GET_CHECK;
		*databuf = GET_OSC_DATA(OscCongData.ch2Addr) >> 8;
		OSC_GET_CHECK;
	}
	if (OscCongData.ch3Addr)
	{
		*databuf = GET_OSC_DATA(OscCongData.ch3Addr);// & 0xff;
		OSC_GET_CHECK;
		*databuf = GET_OSC_DATA(OscCongData.ch3Addr) >> 8;
		OSC_GET_CHECK;
	}
	if (OscCongData.ch4Addr)
	{
		*databuf = GET_OSC_DATA(OscCongData.ch4Addr);// & 0xff;
		OSC_GET_CHECK;
		*databuf = GET_OSC_DATA(OscCongData.ch4Addr) >> 8;
		OSC_GET_CHECK;
	}
	return(check);
}												

/*******************************************************************************
* ��������          : void ClearOscBufEnd(void)
* ��ڲ���			����
* ����				����
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: ��ͨ������Ϊ3ʱ��ջ������4�ֽ�
* 
********************************************************************************/
#if OSC_DATA_CHECK
void ClearOscBufEnd(void)
{
	OscDataBufA.oscDataBuf[OSC_BUF_DATA_LEN-1] = 0;
	OscDataBufA.oscDataBuf[OSC_BUF_DATA_LEN-2] = 0;
	OscDataBufA.oscDataBuf[OSC_BUF_DATA_LEN-3] = 0;
	OscDataBufA.oscDataBuf[OSC_BUF_DATA_LEN-4] = 0;
	OscDataBufB.oscDataBuf[OSC_BUF_DATA_LEN-1] = 0;
	OscDataBufB.oscDataBuf[OSC_BUF_DATA_LEN-2] = 0;
	OscDataBufB.oscDataBuf[OSC_BUF_DATA_LEN-3] = 0;
	OscDataBufB.oscDataBuf[OSC_BUF_DATA_LEN-4] = 0;
}
#endif


/*******************************************************************************
* ��������          : uint8  OscDataCk(uint8 *buf, uint8 len)
* ��ڲ���			��buf				������ʼ��ַ
*					  len				����
* ����				��У����
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 05/18/2010
* ˵��				: ����У�鹦�ܣ����������������������
* 
*******************************************************************************
#if OSC_DATA_CHECK
uint8  OscDataCk(uint8 *buf, uint8 len)
{
	uint8 outData = *buf++;
	while (--len)
	{
		outData ^= *buf++;
	}
	return outData;											// ����������
}
#endif

*/
/*******************************************************************************
* ��������          : void OscSciIoInit(void)
* ��ڲ���			����
* ����				��
* ����	            : Yanyi	
* �汾		        : V0.0.1
* ʱ��              : 06/9/2010
* ˵��				: ʾ������̨SCIʹ��IO�ڳ�ʼ��
* 					  ��Ϊ28035 SCIA��ʼ�����룬����ʵ��оƬʹ��IO�����޸�
*******************************************************************************/
void OscSciIoInit(void)
{
	EALLOW;
    
	SysCtrlRegs.PCLKCR0.bit.SCIAENCLK = 1;     				// SCI-A ʱ��ʹ��    
	GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    					// Enable pull-up for GPIO28 (SCIRXDA)
	GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;	   					// Enable pull-up for GPIO29 (SCITXDA)
	GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  					// Asynch input GPIO28 (SCIRXDA)
	GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   					// Configure GPIO28 for SCIRXDA operation
	GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   					// Configure GPIO29 for SCITXDA operation

#if RS485_ENABLE == 1	

    GpioCtrlRegs.GPAPUD.bit.GPIO27 = 0;    
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;        			// Configure GPIO27, RTS
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;         			// output
    GpioDataRegs.GPADAT.bit.GPIO27 = RS485_R_O;   			// Receive

#endif
	
	
	EDIS;
}


//===================================================================
// ��������: ��̨д����
// ����    ��addr  ��ַ
//			 dat   д����
// return  ��ִ��״̬(Ϊ0ִ�гɹ�)
// ����    : Yanyi	
//===================================================================
Uint16 OscControlWriter(Uint16 addr, Uint16 data)
{
    Uint16 oscReturn;
	// ����д����
	commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;  // Ĭ��дEEPROM
    sciFlag.all = 0;                                        // ����ͨѶ״̬��־
    oscReturn = CommWrite(addr, data);
    commRcvData.commCmdSaveEeprom = SCI_WRITE_NO_EEPROM;
    return oscReturn;
}


//===================================================================
// ��������: ��̨������
// ��ڲ�����addr   ��ַ   
//			 dat	��ȡ����ָ��
// return	 ��ִ��״̬(Ϊ0ִ�гɹ�)
// ����	   : Yanyi	
//===================================================================
Uint16 OscControlRead(Uint16 addr, Uint16* result)
{
    
    Uint16 oscReturn = 0;
    // ���ݶ�����
    sciFlag.all = 0;                                        // ����ͨѶ״̬��־
    oscReturn = CommRead(addr, 1);
    *result = commReadData[0];
    return oscReturn;
}



#else


void OscSciFunction(void)
{}



#endif
#if FC_KEY_CONTROL_EN
//===================================================================
// ��������: �������ַ��������
// ��ڲ�����group   ͨ�Ž��յ�������  
// ����	 �����ص�ǰ����������ַ
// ����	   : lichaochao	
//��ע��ĿǰҺ�����̲���ȡBC�����Խ�ʡ380�ռ�
//===================================================================
uint16 FuncCodeAdd(uint8 group)
{
    uint16 keydata,keyhighH,keygroup;

        keydata = group;
        keyhighH = (keydata & 0xF0);
        keygroup = keydata&0x0F;

        if ((0x00 == keyhighH) || (0xF0 == keyhighH))         // Fx
        {
            keygroup = group&0x0f;
        }
        else
        if ((0xA0 == keyhighH) || (0x40 == keyhighH))       // Ax
        {
            keygroup += FUNCCODE_GROUP_A0;
        }
     if ((keydata & 0xFF) == 0x1F)  // FP
        {
            keygroup = FUNCCODE_GROUP_FP;
        }
        return keygroup;
}



//===================================================================
// ��������: ����Һ��������ʾ����
// ��ڲ�����curFcindex   ��ǰ����������index   
// ����	 �����ص�ǰ�����������ֵ
// ����	   : lichaochao	
//��ע�� ��FR���������ͨ�Ŷ�ȡ�����¹�����С������MD380�д���  
//===================================================================



uint16 upsend(uint16 curFcindex)
{

    union FUNC_ATTRIBUTE attribute;    
    attribute = funcCodeAttribute[curFcindex].attribute;
#if F_FRQ_UINT_ONLY_ONE
    // ���һ�ι���Ƶ��
    if (GetCodeIndex(funcCode.code.errorScene3.elem.errorFrq) == curFcindex)
    {
        attribute.bit.point = funcCode.code.errorFrqUnit & 0x000F;
    }
    // �ڶ��ι���Ƶ��
    else if (GetCodeIndex(funcCode.code.errorScene2.elem.errorFrq) == curFcindex)
    {
        attribute.bit.point = (funcCode.code.errorFrqUnit >> 4) & 0x000F;
    }
    // ��һ�ι���Ƶ��
    else if (GetCodeIndex(funcCode.code.errorScene1.elem.errorFrq) == curFcindex)
    {
        attribute.bit.point = (funcCode.code.errorFrqUnit >> 8 ) & 0x000F;
    }
#endif
return attribute.all;
}
#endif
//end
