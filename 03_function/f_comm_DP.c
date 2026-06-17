/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : f_comm_DP.c
* Version            : V0.0.1
* Date               : 08/17/2009
* Description        : profibus_DP function SCI
********************************************************************************/


#include "f_comm_DP.h"
#include "f_main.h"

#define DEBUG_F_DP             1


#if DEBUG_F_DP

#define SCI_M380_DP_DATA_DEFAULTS   \
{                                   \
    &SciaRegs                       \
}

struct SCI_DATA_DP sciM380DpData = SCI_M380_DP_DATA_DEFAULTS;

Uint16 sendData_DP[SEND_DATA_NUMBER];                        // ���͸�STM32����
Uint16 rcvData_DP[RCV_DATA_NUMBER];                          // ����STM32������
enum DP_SCI_COMM_RCV_FLAG dpSciCommRcvFlag;
enum DP_SCI_COMM_SEND_FLAG dpSciCommSendFlag;

DP_PARAMETER dpParameter = {3,5};

Uint16 dataByteNum; // ��ͬģʽ�����ݴ�����ֽڸ���,PPO1-12,PPO2-20,PPO3-4,PPO5-32

#define FIFO_NUM  16;

void InitSciDpBaudRate(struct SCI_DATA_DP *p); 
#ifdef TARGET_GS32
__interrupt void SCI_DP_RXD_isr(void);
__interrupt void SCI_DP_TXD_isr(void);
#else
interrupt void SCI_DP_RXD_isr(void);
interrupt void SCI_DP_TXD_isr(void);
#endif

void DpDataDeal(void);
void ErrAgainSend(struct SCI_DATA_DP *p);

void UpdateSciDpFormat(struct SCI_DATA_DP *p);
void GetDataByteNum(void);
 

//=====================================================================
//
// ͨѶ�����жϺ���
//
//=====================================================================
#ifdef TARGET_GS32
__interrupt void SCI_DP_RXD_isr(void)
#else
interrupt void SCI_DP_RXD_isr(void)
#endif
{
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
    Uint16 tmp;
    struct SCI_DATA_DP *p = &sciM380DpData;  
    tmp = p->pSciRegs->SCIRXBUF.all; 
    
    if(p->rcvDataJuageFlag == 0)  // ֡ͷ�жϱ�־
    {
#if BUG_SCI_BACK_DATA
        // �������ݸ�ʽ
        if(tmp == 0x5A)
        {
            p->frameFlagDp  = 1;      // ��ͬ����֡                  
            p->rcvDataJuageFlag = 1;
        }
        else 
#endif
        if(tmp == 0xAA)
        {
            p->rcvRigthFlag = 1;      // ����֡ͷ��һ������ȷ
            p->rcvData_SCI[0] = tmp;
              
        }
        else if(p->rcvRigthFlag == 1) // ����֡ͷ��һ������ȷ
        {   
            p->rcvRigthFlag = 0;      // �����֡ͷ��һ������ȷ
            if(tmp == 0x55)
            {
                p->commDpRcvNumber = 1; // �������ݼ���
                p->frameFlagDp  = 2;    // ��ͬ����֡                    
                p->rcvDataJuageFlag = 1;
            }              
        }
        else
        {
            p->rcvCrcErrCounter  = 0;
            p->commDpRcvNumber  = 0;   // �������ݼ���
            p->commDpSendNum  = 0;     // ���͵����ݼ���
            p->commDpSendNumMax  = 0;  // ÿ�η��͵����ݸ���
            p->rcvDataJuageFlag = 0;       
            p->frameFlagDp = 0;
            p->sciRcvFlag = SCI_RCV_NO;
            UpdateSciDpFormat(&sciM380DpData); 
        }

     }
#if BUG_SCI_BACK_DATA    
    // ��������
    if(p->frameFlagDp == 1)
    {
        // ����һ֡���ݻ�û�����
        if (p->commDpRcvNumber < SCI_SEND_READ_NUMBER)  
        {
            p->rcvData_SCI[p->commDpRcvNumber++] = tmp;
            if(p->commDpRcvNumber >= SCI_SEND_READ_NUMBER)
            {
                p->rcvDataJuageFlag = 0;     // ���ݽ��������0
                p->commDpRcvNumber = 0;      // �������������0
                p->sciRcvFlag = SCI_RCV_YES; // ���ݽ���OK
                p->commDpSendNumMax = SCI_SEND_READ_NUMBER;  // �÷������ݸ���
            }
        }                 
    }
    // DP
    else 
#endif
    if(p->frameFlagDp == 2)
    {
       if (p->commDpRcvNumber < (dataByteNum + 4))  // ����һ֡���ݻ�û�����
       {
           p->rcvData_SCI[p->commDpRcvNumber++] = tmp;
           if(p->commDpRcvNumber >= (dataByteNum + 4))
           {
               p->rcvDataJuageFlag = 0;
               p->commDpRcvNumber = 0;
               p->sciRcvFlag = SCI_RCV_YES;
               p->commDpSendNumMax = (dataByteNum + 4);
           }
       }   
    }
#ifdef TARGET_GS32
    
#else 
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;                // Issue PIE ACK
#endif
    #ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}


//=====================================================================
//
// ͨѶ�����жϺ���
//
// ����һ���ַ���ɣ��ͽ�����ж�
//
//=====================================================================
#ifdef TARGET_GS32
__interrupt void SCI_DP_TXD_isr(void)
#else
interrupt void SCI_DP_TXD_isr(void)
#endif
{    
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
    struct SCI_DATA_DP *p = &sciM380DpData;
    
#if NO_FIFO    

    if (p->commDpSendNum < p->commDpSendNumMax)           // ����һ֡����û�����
    {
         p->pSciRegs->SCITXBUF = p->sendData_SCI[p->commDpSendNum++];
    }                       
    
#else

    Uint16 i;
    if (p->commDpSendNumMax < FIFO_NUM)
    {
        for (i = 0; i < p->commDpSendNumMax; i++)
        {
            p->pSciRegs->SCITXBUF = p->sendData_SCI[i];    
        }
    }
    else
    {
        for (i = 0; i < FIFO_NUM; i++)
        {
            p->pSciRegs->SCITXBUF = p->sendData_SCI[i];    
        }
    }
#endif
#ifdef TARGET_GS32
    
#else
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;            // Issue PIE ACK
#endif
    #ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}


//=====================================================================
//
// ͨѶ�жϷ��ʹ�������
//
//=====================================================================
void inline CommDpStartSend(struct SCI_DATA_DP *p)
{
    p->pSciRegs->SCITXBUF = p->sendData_SCI[0];     // ���͵�һ������
    p->commDpSendNum = 1;                           //���жϷ���ʣ�������
}


//=====================================================================
//
// ͨѶ���յ����ݴ�������
//
//=====================================================================
void CommDpRcvDataDeal(struct SCI_DATA_DP *p)
{  
    Uint16 i;
    Uint16 crcValue;
    if (p->sciRcvFlag == SCI_RCV_YES)
    {
        p->sciRcvFlag = SCI_RCV_NO;

#if BUG_SCI_BACK_DATA        
        if (p->frameFlagDp == 1)   // DP�������ò��� ������ַ��ͨѶģʽ
        {                
            crcValue = (p->rcvData_SCI[SCI_SEND_READ_NUMBER - 1] << 8) + p->rcvData_SCI[SCI_SEND_READ_NUMBER - 2]; //��λ�ں� ��λ��ǰ
            if (crcValue == CrcValueByteCalc(p->rcvData_SCI,(SCI_SEND_READ_NUMBER-2)))
            {
                // �������ݴ���
                p->frameSendStart.bit.frameType = DSP_TO_DP_PARAMETER;
                p->sendData_SCI[0] = 0x5A;  // �ظ�����������ͷ
                p->sendData_SCI[1] = dpParameter.dpAddress;
                p->sendData_SCI[2] = dpParameter.dpDataFormat;
                crcValue = CrcValueByteCalc(p->sendData_SCI,(SCI_SEND_READ_NUMBER-2));
                p->sendData_SCI[SCI_SEND_READ_NUMBER-1] = (crcValue >> 8)&0x00ff;
                p->sendData_SCI[SCI_SEND_READ_NUMBER-2] = (crcValue&0x00ff); 
                CommDpStartSend(&sciM380DpData);                     
            }
            else
            {
                p->rcvCrcErrCounter ++; //У�����
            }
            
            p->frameFlagDp = 0;  //��״̬��־
            
        }
        else 
#endif
        if (p->frameFlagDp == 2) //����ͨѶ״̬ 
        {
            crcValue = (p->rcvData_SCI[dataByteNum + 3] << 8) + p->rcvData_SCI[dataByteNum +2];
            if(crcValue == CrcValueByteCalc(p->rcvData_SCI,(dataByteNum + 2)))
            {
                for( i = 0; i < dataByteNum; i++)
                {
                    rcvData_DP[i] = p->rcvData_SCI[i+2];
                    #if BUG_SCI_BACK_DATA
                    sendData_DP[i] = rcvData_DP[i];                             
                    #endif
                }    
                #if BUG_SCI_BACK_DATA
                funcCode.code.dpDataBuffer[1] = (((rcvData_DP[0] << 8)&0xff00) + rcvData_DP[1]);
                funcCode.code.dpDataBuffer[2] = (((rcvData_DP[2] << 8)&0xff00) + rcvData_DP[3]);
                funcCode.code.dpDataBuffer[3] = (((rcvData_DP[4] << 8)&0xff00) + rcvData_DP[5]);
                #endif
                dpSciCommRcvFlag = DP_SCI_COMM_RCV_YES;
            }
            else
            {
                p->rcvCrcErrCounter++;
            }
    
            p->frameFlagDp = 0;  //��״̬��־
    
        }

    }
    
    if(p->rcvCrcErrCounter > RCV_CRC_ERR_NUMBER)
    {
        p->rcvCrcErrCounter  = 0;
        p->commDpRcvNumber  = 0;   // �������ݼ���
        p->commDpSendNum  = 0;     //���͵����ݼ���
        p->commDpSendNumMax  = 0;  //ÿ�η��͵����ݸ���
        p->rcvDataJuageFlag = 0;    
        p->frameFlagDp = 0;
        p->sciRcvFlag = SCI_RCV_NO;
        UpdateSciDpFormat(&sciM380DpData);  // ��������
    }
}


void CommDpSendDataDeal(struct SCI_DATA_DP *p)
{
     Uint16 i;
     Uint16 crcValue;
	 if ( dpSciCommSendFlag == DP_SCI_COMM_SEND_YES )	
     {
        dpSciCommSendFlag = DP_SCI_COMM_SEND_NO;
        // �������ݴ���
        p->frameSendStart.bit.frameType = DSP_TO_DP;
        p->sendData_SCI[0] = 0xAA; 
        p->sendData_SCI[1] = 0x55; // ����ͨѶ����ͷ

        for (i = 0; i < dataByteNum; i++)
        {
            p->sendData_SCI[i + 2] = rcvData_DP[i];
        }
        crcValue = CrcValueByteCalc(p->sendData_SCI,(dataByteNum+2));
        p->sendData_SCI[dataByteNum+3] = (crcValue >> 8)&0x00ff;
        p->sendData_SCI[dataByteNum+2] = (crcValue&0x00ff); 
        CommDpStartSend(&sciM380DpData);
    }
}


//===========================================================================
// Function Name  : sciDpDeal
// Description    : DP��صĴ������ݴ���
// Input          : None
// Output         : None
// Return         : None
//===========================================================================
void SciDpDeal(struct SCI_DATA_DP *p)
{
    
    //����DP���Ĳ���	
    dpParameter.dpAddress = 3;     // funcCode.group.c2[1];        // Profibus DP��ַ
    dpParameter.dpDataFormat = 5;  // funcCode.group.c2[2];    // ���ݴ��͸�ʽѡ��

    GetDataByteNum();   //�������ݴ����ʽ�õ����ݴ�����ֽ���
    
    UpdateSciDpFormat(&sciM380DpData); 
    CommDpRcvDataDeal(&sciM380DpData);  

    // DP���ݴ�������
    //DpDataDeal(); 
    #if BUG_SCI_BACK_DATA
    if( dpSciCommRcvFlag == DP_SCI_COMM_RCV_YES)
    {
        dpSciCommRcvFlag = DP_SCI_COMM_RCV_NO;      // ���ձ�־����
        dpSciCommSendFlag = DP_SCI_COMM_SEND_YES;   // �÷��ͱ�־
    }
    #else
    DpDataDeal(); 
    #endif
    CommDpSendDataDeal(&sciM380DpData);

}

//===========================================================================
// Function Name  : ErrAgainSend
// Description    : CRCУ����� �����ط�
// Input          : DP�����������ṹ������
// Output         : None
// Return         : None
//===========================================================================
void ErrAgainSend(struct SCI_DATA_DP *p)
{
     Uint16 crcValue;
     Uint16 i;
     
     p->frameSendStart.bit.frameType = ERR_FRAME;
     p->sendData_SCI[0] = p->frameSendStart.all;

     for(i=0; i<dataByteNum; i++)
     {
        p->sendData_SCI[i+1] = 0x55;
     }
     crcValue = CrcValueByteCalc(p->sendData_SCI,(dataByteNum+1));
     p->sendData_SCI[dataByteNum+2] = (crcValue >> 8)&0x00ff;
     p->sendData_SCI[dataByteNum+1] = (crcValue&0x00ff); 

     CommDpStartSend(&sciM380DpData); 
}

//=====================================================================
//
// ͨѶ��ʼ��
//
//=====================================================================
void InitSetSciDp(struct SCI_DATA_DP *p)
{ 
#if NO_FIFO 
    p->pSciRegs->SCICTL1.all = 0x0001;   // SCI������λ������Ч
    p->pSciRegs->SCICTL2.all = 0x00C2;   
    p->pSciRegs->SCICCR.all = 0x0087;    // 2 stop bit, No loopback, No parity,8 bits,async mode,idle-line
    p->pSciRegs->SCIPRI.bit.FREE = 1;
    p->pSciRegs->SCICTL1.all = 0x0023;   // �����ж�                     
    p->pSciRegs->SCICTL2.all = 0x00C3;   // ���������ж�
    p->pSciRegs->SCICCR.all = 0x0087;    // ����żУ��ʱ��2��ֹͣλ��ModbusЭ��Ҫ��    
    InitSciDpBaudRate(&sciM380DpData);   // ��ʼ�����ڲ�����
#else
    p->pSciRegs->SCICTL1.all = 0x0001;   // SCI������λ������Ч
    p->pSciRegs->SCICTL2.all = 0x0082;   
    p->pSciRegs->SCIFFTX.all = 0xE060;
    p->pSciRegs->SCICCR.all = 0x0087;    // 2 stop bit, No loopback, No parity,8 bits,async mode,idle-line
    p->pSciRegs->SCIPRI.bit.FREE = 1;
    p->pSciRegs->SCICTL1.all = 0x0023;   // �����ж�                     
    p->pSciRegs->SCICTL2.all = 0x0083;   // ���������ж�  
    p->pSciRegs->SCICCR.all = 0x0087;         
    InitSciDpBaudRate(&sciM380DpData);    //��ʼ�����ڲ�����
#endif
}


//=====================================================================
//
// sciͨѶ�����޸ĺ���
//
//=====================================================================
void UpdateSciDpFormat(struct SCI_DATA_DP *p)
{   
    if (p->pSciRegs->SCIRXST.bit.RXERROR)       // ���ֽ��չ���ʱ����
    {
        InitSetSciDp(&sciM380DpData);  
    }

    p->pSciRegs->SCICCR.all = 0x0087;       // ����żУ��ʱ��2��ֹͣλ��ModbusЭ��Ҫ��    
    InitSciDpBaudRate(&sciM380DpData);      // ��ʼ�����ڲ�����
}



void InitSciaGpioDp(void)
{
    EALLOW;
    
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // Enable pull-up for GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;    // Enable pull-up for GPIO29 (SCITXDA)
    
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // Asynch input GPIO28 (SCIRXDA)
    
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // Configure GPIO28 for SCIRXDA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // Configure GPIO29 for SCITXDA operation

    EDIS;

    // ͨѶ����ʹ���жϣ���ʼ��
    EALLOW;
    #ifdef TARGET_GS32
    Interrupt_register(INT_SCIA_RX, &SCI_DP_RXD_isr);
    Interrupt_register(INT_SCIA_TX, &SCI_DP_TXD_isr);
    #else
    PieVectTable.SCIRXINTA = SCI_DP_RXD_isr;
	PieVectTable.SCITXINTA = SCI_DP_TXD_isr;
    #endif
    EDIS;
	// IER |= M_INT9;   	            //  Enable interrupts:
    #ifdef TARGET_GS32
    Interrupt_enable(INT_SCIA_RX);
    Interrupt_enable(INT_SCIA_TX);
    #else
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
    #endif
}




void InitScibGpioDp(void)
{

    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // Enable pull-up for GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;    // Enable pull-up for GPIO29 (SCITXDA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // Asynch input GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // Configure GPIO28 for SCIRXDA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // Configure GPIO29 for SCITXDA operation
    EDIS;
    // ͨѶ����ʹ���жϣ���ʼ��
    EALLOW;
    #ifdef TARGET_GS32
    Interrupt_register(INT_SCIA_RX, &SCI_DP_RXD_isr);
    Interrupt_register(INT_SCIA_TX, &SCI_DP_TXD_isr);
    #else
    PieVectTable.SCIRXINTA = SCI_DP_RXD_isr;
	PieVectTable.SCITXINTA = SCI_DP_TXD_isr;
    #endif
    EDIS;
	// IER |= M_INT9;   	            //  Enable interrupts:
    #ifdef TARGET_GS32
    Interrupt_enable(INT_SCIA_RX);
    Interrupt_enable(INT_SCIA_TX);
    #else
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
    #endif
}

//=====================================================================
//
// ��ʼ�����ڲ�����
// ������=100*10^6/4/((BAUD+1)*8)
//
//=====================================================================
void InitSciDpBaudRate(struct SCI_DATA_DP *p)
{
#if 0
    switch(funcCode.group.c2[3])	//���ڲ�����
    {                    
         case SCI_BAUD_RATE1:             
            p->pSciRegs->SCIHBAUD = 0x0000;
            p->pSciRegs->SCILBAUD = 0x001a; //115200bps 
            break;
            
         case SCI_BAUD_RATE2:             
            p->pSciRegs->SCIHBAUD = 0x0000;
            p->pSciRegs->SCILBAUD = 0x000e; //208333bps 
            break;
            
         case SCI_BAUD_RATE3:             
            p->pSciRegs->SCIHBAUD = 0x0000;
            p->pSciRegs->SCILBAUD = 0x000b; //256000bps 
            break;
            
		 case SCI_BAUD_RATE4:             
            p->pSciRegs->SCIHBAUD = 0x0000; 
            p->pSciRegs->SCILBAUD = 0x0005; //512000bps,modefied by sjw 2009-12-09
            break;

         default: 
            break; 
    }
#elif 1
			p->pSciRegs->SCIHBAUD = 0x0000;
            p->pSciRegs->SCILBAUD = 0x000e; //208333bps 
#endif
}



//=====================================================================
//
// �������ݴ����ʽ��ȡ���ݴ����ֽڸ���
//
//=====================================================================
const Uint16 dataByteNums[5] = {12, 20, 0, 4, 32};
void GetDataByteNum(void)
{
    dataByteNum = dataByteNums[dpParameter.dpDataFormat - 1];
}


Uint16 dpTest = 2;
void DpDataDeal(void)
{
    // û�н��յ��µ�����
    if (DP_SCI_COMM_RCV_NO == dpSciCommRcvFlag)
    {
        return;
    }
#if 0
    for (i = 0; i < RCV_DATA_NUMBER; i++)
    {
        sendData_DP[i] = rcvData_DP[i] + dpTest;
    }
#elif 0	//modefied by sjw 2009-12-24 for test
    ethDpPara.rcvFlag = 1;
    for (i = 0; i < 16; i++)
    {       
       ethDpPara.rcv[i] = (rcvData_DP[2*i+1]&0x00ff)  + (rcvData_DP[2*i]<< 8&0xff00); //PLC���͵����� ���ֽ���ǰ�����ֽ��ں�
    }
    ethDpDeal();
    for (i = 0; i < ETH_PLC_RCV_LENGTH; i++)
    {    
        sendData_DP[2*i] = ((ethDpPara.reply[ i ] >> 8)&0x00ff);
        sendData_DP[2*i + 1] = (ethDpPara.reply[ i ]&0x00ff);//DSP���������� ���ֽ���ǰ�����ֽ��ں�
    }
#endif

    dpSciCommRcvFlag = DP_SCI_COMM_RCV_NO;      // ���ձ�־����
    dpSciCommSendFlag = DP_SCI_COMM_SEND_YES;   // �÷��ͱ�־
}



#elif 1


void InitScibGpioDp(void){}
void InitSetSciDp(struct SCI_DATA_DP *p){}
void SciDpDeal(struct SCI_DATA_DP *p){}


#endif


























