//======================================================================
//
// 通讯锟斤拷锟斤拷锟斤拷ModBus协锟介。
// 
// Time-stamp: <2008-11-23 11:40:25  Shisheng.Zhi, 0354>
//
//======================================================================

#include "f_comm.h"
#include "f_p2p.h"
#include "f_ui.h"
#include "f_osc.h"
#include "f_io.h"

#if F_DEBUG_RAM                     // 锟斤拷锟斤拷锟皆癸拷锟杰ｏ拷锟斤拷CCS锟斤拷build option锟叫讹拷锟斤拷暮锟�
#define DEBUG_F_MODBUS          0   // 锟角凤拷使锟斤拷通讯锟斤拷锟斤拷
#elif 1
#define DEBUG_F_MODBUS          1
#endif

enum COMM_STATUS commStatus;    // 锟斤拷锟节筹拷始锟斤拷为锟饺达拷锟斤拷锟斤拷状态
COMM_SEND_DATA commSendData;
COMM_RCV_DATA commRcvData;
Uint16 commErrInfo;
Uint16 commBaudRate;
Uint16 commType;                 // 锟斤拷锟斤拷通讯协锟斤拷锟斤拷锟斤拷
Uint16 commProtocol;             // 通讯协锟斤拷锟斤拷锟捷革拷式
Uint32 commTicker;               // *_0.5ms
Uint32 commSendTicker;                  
Uint32 canLinkTicker;            // *_0.5ms
Uint16 commRunCmd;               // 通讯锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
Uint16 aoComm[AO_NUMBER+1];      // 通讯AO锟斤拷锟斤拷. 0-HDO, 锟斤拷锟斤拷为AO
union DO_SCI doComm;             // 通讯DO锟斤拷锟斤拷
Uint16 userPwdPass4Comm;         // 1-锟矫伙拷锟斤拷锟斤拷通锟斤拷锟斤拷锟斤拷通讯FP-01使锟斤拷
Uint16 companyPwdPass4Comm;      // 1-锟斤拷锟斤拷锟斤拷锟斤拷通锟斤拷锟斤拷FF锟斤拷使锟斤拷
Uint16 groupHidePassComm;        // 1-锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷通锟斤拷锟斤拷AF锟斤拷使锟斤拷
LOCALF Uint16 sendFrame[36];                           // 锟接伙拷锟斤拷应帧
LOCALF Uint16 rcvFrame[36];                            // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷(锟斤拷锟斤拷锟斤拷锟斤拷帧)
LOCALF Uint16 commReadData[RTU_READ_DATA_NUM_MAX];     // 锟斤拷取锟斤拷锟斤拷锟斤拷
LOCALF union SCI_FLAG sciFlag;                         // SCI使锟矫的憋拷志
Uint16 FuncFeData;              // FE锟斤拷锟斤拷锟斤拷
Uint16 FEHigh_group;
Uint16 FELow_grade;
Uint16 indexgroup;
Uint16 indexgrade;
/************************液锟斤拷锟斤拷锟斤拷使锟斤拷****************************/
#if FC_KEY_CONTROL_EN
extern Uint16 LcdKeyFlag;  // 液锟斤拷锟斤拷锟教诧拷锟斤拷锟街疚�
extern Uint16 LcdReadFEflag; //液锟斤拷锟斤拷锟教讹拷取FE锟斤拷
#endif
/*************************************************/
#if DEBUG_F_MODBUS

#if (DSP_CLOCK == 100)
// 100*10^6/4/(_*8)-1
// 实锟绞诧拷锟斤拷锟斤拷锟斤拷 100*10^6/4/(_*8)
#if !DSP_2803X
LOCALF const DSP_BAUD_REGISTER_DATA dspBaudRegData[BAUD_NUM_MAX + 1] =  // LSPCLK = SYSCLKOUT/4
{
    {   3, 0x0028, 0x00af},           // 0, 300bps
    {   6, 0x0014, 0x0057},           // 1, 600bps
    {  12, 0x000a, 0x002b},           // 2, 1200bps
    {  24, 0x0005, 0x0015},           // 3, 2400bps
    {  48, 0x0002, 0x008a},           // 4, 4800bps
    {  96, 0x0001, 0x0044},           // 5, 9600bps
    { 192, 0x0000, 0x00a1},           // 6, 19200bps
    { 384, 0x0000, 0x0050},           // 7, 38400bps
    { 576, 0x0000, 0x0035},           // 8, 57600bps
    {1152, 0x0000, 0x001a},           // 9, 115200bps
    {1280, 0x0000, 0x000E},           // 10,208300bps
    {2560, 0x0000, 0x000b},           // 11,256000bps
    {5120, 0x0000, 0x0005},           // 12,512000bps
};
#endif
#elif (DSP_CLOCK == 60)                               // DSP锟斤拷锟斤拷频锟斤拷60MHz

LOCALF const DSP_BAUD_REGISTER_DATA dspBaudRegData[BAUD_NUM_MAX + 1] =  // LSPCLK = SYSCLKOUT/4
{
#if DSP_2803X
    {   3, 0x0000, 0x1869,  0},           // 0, 300bps
    {   6, 0x0000, 0x0C34,  0},           // 1, 600bps
    {  12, 0x0000, 0x0619,  8},           // 2, 1200bps
    {  24, 0x0000, 0x030c,  4},           // 3, 2400bps
    {  48, 0x0000, 0x0185, 10},           // 4, 4800bps
    {  96, 0x0000, 0x00c2,  5},           // 5, 9600bps
    { 192, 0x0000, 0x0060, 11},           // 6, 19200bps
    { 384, 0x0000, 0x002f, 13},           // 7, 38400bps
    { 576, 0x0000, 0x001f,  9},           // 8, 57600bps
    {1152, 0x0000, 0x000f,  4},           // 9, 115200bps
    {1280, 0x0000, 0x0008,  0},           // 10, 208300bps
    {2560, 0x0000, 0x0006, 15},           // 11, 256000bps
    {5120, 0x0000, 0x0002, 11},           // 12, 512000bps
#else
    {   3, 0x0018, 0x0069},           //  0, 300bps
    {   6, 0x000c, 0x0034},           //  1, 600bps
    {  12, 0x0006, 0x0019},           //  2, 1200bps
    {  24, 0x0003, 0x000c},           //  3, 2400bps
    {  48, 0x0001, 0x0085},           //  4, 4800bps
    {  96, 0x0000, 0x00c2},           //  5, 9600bps
    { 192, 0x0000, 0x0060},           //  6, 19200bps
    { 384, 0x0000, 0x002f},           //  7, 38400bps
    { 576, 0x0000, 0x001f},           //  8, 57600bps
    {1152, 0x0000, 0x000f},           //  9, 115200bps
    {1280, 0x0000, 0x0008},           // 10, 208300bps
    {2560, 0x0000, 0x0006},           // 11, 256000bps
    {5120, 0x0000, 0x0002},           // 12,512000bps
#endif
};
#endif

#if DSP_2803X
const Uint16 commParitys[4] = {0x10, 0x0c, 0x04, 0x00};  //[STOP PARITYENA PARITY] 100 011 010 000
#else
const Uint16 commParitys[4] = {0x87, 0x67, 0x27, 0x07};
#endif

// 某些锟斤拷锟斤拷锟诫，通讯锟斤拷锟杰斤拷锟斤拷W
#define COMM_NO_W_FC_0  GetCodeIndex(funcCode.code.tuneCmd)              // 锟斤拷谐
#define COMM_NO_W_FC_1  GetCodeIndex(funcCode.code.menuMode)             // 锟剿碉拷模式
#define COMM_NO_W_FC_2  GetCodeIndex(funcCode.code.motorFcM2.tuneCmd)    // 锟斤拷谐
#define COMM_NO_W_FC_5  GetCodeIndex(funcCode.code.funcParaView)         // 锟斤拷锟杰菜碉拷模式锟斤拷锟斤拷
// 某些锟斤拷锟斤拷锟诫，通讯锟斤拷锟杰斤拷锟斤拷R
#define COMM_NO_R_FC_0  GetCodeIndex(funcCode.code.userPassword)         // 锟矫伙拷锟斤拷锟斤拷
// 某些锟斤拷锟斤拷锟诫，通讯锟斤拷锟杰斤拷锟斤拷RW
#define COMM_NO_RW_FC_0 GetCodeIndex(funcCode.code.userPasswordReadOnly) // 只锟斤拷锟矫伙拷锟斤拷锟斤拷
#define COMM_READ_CURRENT_FC GetCodeIndex(funcCode.group.u0[4])          // 通讯锟斤拷取锟斤拷锟斤拷


const Uint16 COMM_ERR_INDEX[8] = {10, 8, 2, 7, 6, 9, 12, 13};

LOCALD void CommRcvDataDeal(void);
LOCALD void CommSendDataDeal(void);
LOCALD void resetLinSci(void);
LOCALD void closeRTX(void);
LOCALD void setRxConfig(void);
LOCALD void setTxConfig(void);
LOCALD void commErrorDeal(void);
LOCALD void commStatusDeal(void);
LOCALD void CommDataReRcv(Uint16 tmp);
LOCALD void CommDataSend(void);
LOCALD Uint16 CommRead(Uint16, Uint16);
LOCALD Uint16 CommRwFuncCode(Uint16, Uint16, Uint16 rwMode);
LOCALD void inline CommStartSend(void);

#define COMM_READ_FC    0       // 通讯锟斤拷锟斤拷锟斤拷锟斤拷
#define COMM_WRITE_FC   1       // 通讯写锟斤拷锟斤拷锟斤拷

// 锟叫讹拷
#if DSP_2803X
void SCI_RXD_isr(void);
void SCI_TXD_isr(void);
interrupt void Lina_Level0_ISR(void);    // LIN-SCI 锟叫讹拷
interrupt void Lina_Level1_ISR(void);    // LIN-SCI 锟叫讹拷
#else
#ifdef TARGET_GS32
__interrupt void SCI_RXD_isr(void);       // SCI锟叫讹拷
__interrupt void SCI_TXD_isr(void);       // SCI锟叫讹拷
#else
interrupt void SCI_RXD_isr(void);       // SCI锟叫讹拷
interrupt void SCI_TXD_isr(void);       // SCI锟叫讹拷
#endif
#endif

extern Uint16 ValidateInvType(void);
// RS485锟侥斤拷锟秸凤拷锟斤拷锟叫伙拷
#if DSP_2803X
#define RTS (GpioDataRegs.GPBDAT.bit.GPIO39)
#define SET_RTS_R (GpioDataRegs.GPBCLEAR.bit.GPIO39 = 1)
#define SET_RTS_T (GpioDataRegs.GPBSET.bit.GPIO39 = 1)
#else
#define RTS (GpioDataRegs.GPADAT.bit.GPIO27)
#define SET_RTS_R (GpioDataRegs.GPACLEAR.bit.GPIO27 = 1)
#define SET_RTS_T (GpioDataRegs.GPASET.bit.GPIO27 = 1)
#endif
#define RS485_R     0
#define RS485_T     1


// 通讯协锟斤拷锟斤拷锟捷诧拷锟斤拷锟斤拷锟斤拷
// 目前锟斤拷锟斤拷MODBUS锟斤拷PROFIBUS
// 锟角凤拷锟角斤拷锟街持诧拷锟斤拷锟斤拷锟斤拷协锟斤拷也为锟斤拷锟斤拷锟斤拷锟斤拷
#define PROTOCOL_NUM    1
const protocolDeal protocolFunc[PROTOCOL_NUM] =
{
// MODBUS
    { ModbusRcvDataDeal,       ModbusStartDeal,      
      UpdateModbusCommFormat,  ModbusSendDataDeal,
      ModbusCommErrCheck,
    },
};


//=====================================================================
//
// 通讯锟斤拷锟斤拷锟斤拷锟斤拷
//
//=====================================================================
void SciDeal(void)
{
    Uint16 commErrFlag;

    // 锟斤拷锟铰达拷锟斤拷锟斤拷锟斤拷
    UpdateSciFormat();

    // 锟斤拷锟斤拷通锟斤拷锟斤拷效时锟斤拷
    if (commTicker >= (Uint32)30 * TIME_UNIT_MS_PER_SEC * 2)    // 30s之锟襟，凤拷锟斤拷权失效
    {
        userPwdPass4Comm = 0;
        companyPwdPass4Comm = 0;
        groupHidePassComm = 0;
    }

    // 通讯锟斤拷锟斤拷锟斤拷锟�
	// MODBUS锟斤拷夥绞轿拷锟斤拷锟酵ｏ拷锟绞憋拷涑拷锟酵ㄑ队︼拷锟绞笔憋拷锟斤拷锟斤拷锟�
	// PROFIBUS锟斤拷夥绞轿狢RC校锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锏�10锟斤拷锟斤拷锟斤拷(PROFIBUS锟角凤拷锟叫憋拷要锟斤拷锟斤拷锟斤拷锟斤拷)
    commErrFlag = protocolFunc[commType].CommErrCheck();

    // 通讯锟斤拷锟斤拷
    if (commErrFlag)
    {
        // 通讯锟斤拷锟斤拷锟斤拷锟斤拷
        commErrorDeal();     // 锟斤拷锟斤拷锟较诧拷锟矫达拷锟斤拷为锟斤拷锟斤拷
    }
    else
    {
        // 通讯锟斤拷锟教达拷锟斤拷
        commStatusDeal();
    }
}


//=====================================================================
//
// 通讯锟斤拷锟斤拷锟叫断猴拷锟斤拷
//
//=====================================================================
#if DSP_2803X
void SCI_RXD_isr(void)
#else
#ifdef TARGET_GS32
__interrupt void SCI_RXD_isr(void)
#else
interrupt void SCI_RXD_isr(void)
#endif
#endif
{
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
    Uint16 tmp;

#if DSP_2803X
    tmp = LinaRegs.SCIRD;
#else
    tmp = SciaRegs.SCIRXBUF.all;
#endif
    
    // 锟斤拷锟捷斤拷锟斤拷帧头锟叫讹拷
    if (protocolFunc[commType].StartDeal(tmp))
    {
		// 为锟斤拷锟斤拷锟侥斤拷锟斤拷锟斤拷锟斤拷  0-锟斤拷效   1-锟姐播锟斤拷址    2-锟斤拷锟斤拷锟斤拷址
		if (commRcvData.rcvFlag)
		{
	        // 锟斤拷帧头通讯锟斤拷锟捷斤拷锟斤拷
	        CommDataReRcv(tmp);
		}
    }
    
    commTicker = 0;                     // 锟叫斤拷锟斤拷锟斤拷锟捷ｏ拷锟斤拷锟铰硷拷时
#ifdef TARGET_GS32
    
#else
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;    // Issue PIE ACK
#endif
    #ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}


//=====================================================================
//
// 通讯锟斤拷锟斤拷锟叫断猴拷锟斤拷
//
// 锟斤拷锟斤拷一锟斤拷锟街凤拷锟斤拷桑锟斤拷徒锟斤拷锟斤拷锟叫讹拷
//
//=====================================================================
#if DSP_2803X
void SCI_TXD_isr(void)
#else
#ifdef TARGET_GS32
__interrupt void SCI_TXD_isr(void)
#else
interrupt void SCI_TXD_isr(void)
#endif
#endif
{
#ifdef TARGET_GS32
    SAVE_IRQ_CSR_CONTEXT();
#endif
	// 通讯锟斤拷锟斤拷锟斤拷锟斤拷
    CommDataSend();   
    commTicker = 0;                     // 锟斤拷锟斤拷一锟斤拷锟街凤拷锟斤拷桑锟斤拷锟斤拷录锟绞�                        
#ifdef TARGET_GS32
    
#else
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;    // Issue PIE ACK
#endif
    #ifdef TARGET_GS32
    RESTORE_IRQ_CSR_CONTEXT();
#endif
}


#if DSP_2803X
// 锟斤拷锟斤拷锟饺硷拷锟叫讹拷
interrupt void Lina_Level0_ISR(void)
{
	Uint32 LinL0IntVect;  

	LinL0IntVect = LinaRegs.SCIINTVECT0.all;

	// 锟斤拷锟斤拷锟叫讹拷
	if(LinL0IntVect == 11)
	{
		SCI_RXD_isr();
	}
	//  锟斤拷锟斤拷锟叫讹拷
	else if(LinL0IntVect == 12)
	{
		SCI_TXD_isr();
	}
    // other
    else
    {
        #ifdef TARGET_GS32
    
        #else
        PieCtrlRegs.PIEACK.bit.ACK9 = 1;
        #endif
    }
}

//Low priority BLIN ISR.  Just a placeholder.
interrupt void Lina_Level1_ISR(void)
{
#ifdef TARGET_GS32
    
#else
	PieCtrlRegs.PIEACK.bit.ACK9 = 1; 
#endif
}
#endif


//=====================================================================
//
// 通讯锟斤拷锟秸碉拷锟斤拷锟捷达拷锟斤拷锟斤拷锟斤拷
//
//=====================================================================  
LOCALD void CommRcvDataDeal(void)
{
    Uint16 writeErr;
    Uint16 i,j;
    Uint16 rcvFrameMore[12];
	// 锟斤拷同协锟斤拷慕锟斤拷锟斤拷锟斤拷锟斤拷锟较拷锟斤拷锟�
	// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟筋、锟斤拷址锟斤拷锟斤拷锟捷碉拷锟斤拷息
    protocolFunc[commType].RcvDataDeal();
    // 锟斤拷SCI锟斤拷志
    sciFlag.all = 0;

    // CRC校锟斤拷
    if (commRcvData.crcRcv != CrcValueByteCalc(rcvFrame, commRcvData.crcSize))  // CRC校锟斤拷锟斤拷锟斤拷卸锟�
    {         
        sciFlag.bit.crcChkErr = 1;                      // 锟斤拷位锟斤拷CRCErr锟斤拷send
		commRcvData.rcvCrcErrCounter++;                 // 锟斤拷录CRC校锟斤拷锟斤拷锟斤拷锟斤拷锟�
    }
     // 锟姐播模式
    else if (!commRcvData.slaveAddr)  
    {
		// 锟姐播写锟斤拷锟斤拷
        if ((SCI_CMD_WRITE == commRcvData.commCmd)
          || (SCI_CMD_WRITE_RAM == commRcvData.commCmd)
          || (SCI_CMD_WRITE_MORE == commRcvData.commCmd)
           )
        {
			// 锟斤拷写锟斤拷志
            sciFlag.bit.write = 1;
        }
        else
        {   
            sciFlag.bit.cmdErr = 1;                                                 // 锟斤拷锟斤拷锟斤拷锟�
        }
    }
    else //if (RTUslaveAddress == funcCode.code.commSlaveAddress) // 锟斤拷锟斤拷锟斤拷址锟叫讹拷
    {
		if (SCI_CMD_READ == commRcvData.commCmd)       // 锟斤拷锟斤拷锟斤拷锟斤拷锟�
        {
            sciFlag.bit.read = 1;                           // 锟斤拷位锟斤拷read锟斤拷send
        }
        else if ((SCI_CMD_WRITE == commRcvData.commCmd) 
                || (SCI_CMD_WRITE_RAM == commRcvData.commCmd)
                || (SCI_CMD_WRITE_MORE == commRcvData.commCmd))      // 写锟斤拷锟斤拷锟斤拷锟�
        {
            sciFlag.bit.write = 1;			                        
        }
        else
        {   
            sciFlag.bit.cmdErr = 1;                                                 // 锟斤拷锟斤拷锟斤拷锟�
        }
    }

    // 写锟斤拷锟捷达拷锟斤拷
    if (sciFlag.bit.write)
    {

#if 0
        // 写EEPROM
        if (SCI_CMD_WRITE == commRcvData.commCmd)
        {
            commRcvData.commCmdSaveEeprom = SCI_WRITE_WITH_EEPROM;
        }
#endif
        if(SCI_CMD_WRITE_MORE == commRcvData.commCmd)
        {	
            if(((commRcvData.commData*2) != commRcvData.moreWriteNum)
                ||(commRcvData.commData > RTU_WRITE_DATA_NUM_MAX))
            {
                writeErr = COMM_ERR_ADDR;
            }
            else
            {
                j=0;
                for(i=0;i<commRcvData.moreWriteNum;i++)
                {
                    rcvFrameMore[j] = (rcvFrame[7+i]<<8) + rcvFrame[8+i];
                    i++;
                	j++;
                }
                writeErr = WriteMultiReg(commRcvData.commAddr, (Uint16*) (&rcvFrameMore[0]),commRcvData.commData, 1);
                if(writeErr)
				writeErr = COMM_ERR_PARA;
                sciFlag.bit.write = 1;
            }
        }
        else
        {
            writeErr = CommWrite(commRcvData.commAddr, commRcvData.commData);  
        }
        // 写失锟斤拷
        if (writeErr)
        {
			// 锟斤拷示写失锟杰癸拷锟斤拷
            sciFlag.all |= (0x0001 << COMM_ERR_INDEX[writeErr - 1]);
        }  
    }
}

//=====================================================================
//
// 锟斤拷锟捷斤拷锟秸达拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
//
//=====================================================================
Uint16 SciErrCheck(void)
{
    Uint16 readErr;
	Uint16 operateErr;
	
	// 锟斤拷值锟斤拷锟睫癸拷锟斤拷
	operateErr = COMM_ERR_NONE;
    
    // 通讯锟斤拷锟斤拷锟斤拷
    if (sciFlag.bit.read)               // 通讯锟斤拷锟斤拷锟斤拷取锟斤拷锟斤拷
    {
        if(commRcvData.commData > RTU_READ_DATA_NUM_MAX)    // 锟斤拷锟斤拷取12锟斤拷锟斤拷锟斤拷
        {
            sciFlag.bit.paraOver = 1;   //  锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
        }
        else
        {
            readErr = CommRead(commRcvData.commAddr, commRcvData.commData);
            if (readErr)
            {
				// 锟斤拷锟捷讹拷取. 锟斤拷锟斤拷锟斤拷锟斤拷霉锟斤拷锟轿伙拷锟斤拷锟斤拷锟揭拷锟斤拷锟斤拷亩锟饺�
                sciFlag.all |= (0x0001 << COMM_ERR_INDEX[readErr - 1]);                  
            }
        }
    }

	// 锟斤拷锟斤拷锟斤拷息锟斤拷锟斤拷
    if (sciFlag.bit.pwdErr)                 // 锟斤拷锟斤拷锟斤拷锟紼rr01
    {
        operateErr = COMM_ERR_PWD;
    }
    else if (sciFlag.bit.cmdErr)            // 锟斤拷写锟斤拷锟斤拷锟斤拷锟紼rr02
    {
        operateErr = COMM_ERR_CMD;
    }
    else if (sciFlag.bit.crcChkErr)         // CRC校锟斤拷锟斤拷锟�: Err03
    {
        operateErr = COMM_ERR_CRC;
    }
    else if (sciFlag.bit.addrOver)          // 锟斤拷锟斤拷锟斤拷锟斤拷效锟斤拷址锟斤拷Err04
    {
        operateErr = COMM_ERR_ADDR;
    }
    else if (sciFlag.bit.paraOver)          // 锟斤拷锟斤拷锟斤拷锟斤拷效锟斤拷锟斤拷锟斤拷Err05
    {
        operateErr = COMM_ERR_PARA;
    }
    else if (sciFlag.bit.paraReadOnly)      // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷效锟斤拷Err06
    {
        operateErr = COMM_ERR_READ_ONLY;
    }
    else if (sciFlag.bit.systemLocked)      // 系统锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷0x0007
    {
        operateErr = COMM_ERR_SYSTEM_LOCKED;
    }
#if 1   // 目前eeprom锟斤拷锟斤拷锟斤拷锟斤拷拢锟斤拷锟斤拷锟斤拷懈么锟斤拷螅锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟角憋拷锟斤拷
    else if (sciFlag.bit.saveFunccodeBusy)  // 锟斤拷锟节存储锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷0x0008
    {
        operateErr = COMM_ERR_SAVE_FUNCCODE_BUSY;
    }
#endif

	return operateErr;
}


//=====================================================================
//
// 通讯锟斤拷锟斤拷锟斤拷锟捷达拷锟斤拷锟斤拷锟斤拷
//
//=====================================================================
LOCALF void CommSendDataDeal(void)
{
    int16 error;
    Uint16 crcSend;

    // 锟叫讹拷通讯锟斤拷写锟斤拷锟斤拷锟斤拷锟斤拷
    error = SciErrCheck();

    // 准锟斤拷协锟介发锟斤拷锟斤拷锟斤拷
    protocolFunc[commType].SendDataDeal(error);

    // 准锟斤拷CRC校锟斤拷锟斤拷锟斤拷
    crcSend = CrcValueByteCalc(sendFrame, commSendData.sendNumMax - 2);
    sendFrame[commSendData.sendNumMax - 2] = crcSend & 0x00ff;    // CRC锟斤拷位锟斤拷前
    sendFrame[commSendData.sendNumMax - 1] = crcSend >> 8;
}


//=====================================================================
//
// 通讯写锟斤拷锟捷猴拷锟斤拷
// (通讯也锟斤拷锟斤拷锟睫改碉拷锟斤拷锟斤拷锟�)
//
//=====================================================================
Uint16 ConmmWriteAtribte(Uint16 addr, Uint16 data)
{
    Uint16 errType = COMM_ERR_NONE;   // 锟斤拷取锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
    Uint16 high = (addr & 0xF000);

// 通讯锟借定值(通讯频锟斤拷锟借定)
    if (COMM_SET_VALUE_ADDR == addr)
    {
        if ((-10000 > (int16)data) || ((int16)data > 10000))
        {
            errType = COMM_ERR_PARA;
        }
    }
// 通讯锟斤拷锟斤拷锟斤拷锟筋处锟斤拷
    else if (COMM_CMD1_ADDR == addr)                 
    {
        if ((data == 0)||(data > 8))
        {
            commRunCmd = 0;
            errType = COMM_ERR_PARA;
        }
    }
// DO锟斤拷锟斤拷
    else if ((COMM_DO_ADDR == addr) 
            ||(COMM_KEYBORD_TEST == addr)) // 锟斤拷锟斤拷锟斤拷锟斤拷
    {
        //  锟斤拷锟斤拷锟斤拷锟叫讹拷
    }
// HDO锟斤拷锟斤拷
    else if ((COMM_HDO_ADDR == addr)  // HDO锟斤拷锟斤拷
            ||(COMM_AO1_ADDR == addr) // AO1锟斤拷锟斤拷
            ||(COMM_AO2_ADDR == addr) // AO2锟斤拷锟斤拷
            )
    {
        if (data > 0x7FFF)
        {
            errType = COMM_ERR_PARA;
        }
    }
    else if ((addr & 0xFF00) == 0x2f00)   // 通锟斤拷 FE锟斤拷专锟斤拷
    {
        Uint16 dataH,dataL,IndexTmp;
        if((addr & 0xFF00) == 0x2F00)
        {
            dataH = data & 0xf000;
            dataL = data & 0x00ff;
            if(dataH == 0xf000)                  // F
            {
                IndexTmp = (data & 0xf00)>>8;    // FX-00
            }
            //else if(dataH == 0x1000)// 0x1fxx  fp
			else if((data & 0xff00) == 0x1F00)  //by modfied yz1990 2014-06-16
            {
                IndexTmp = 16;
            }
            else if(dataH == 0xA000)             // A
            {
                IndexTmp = (data & 0xf00)>>8;    // FX-00
                IndexTmp +=17;
            }
            else if(dataH == 0xB000)             // B
            {
                IndexTmp = (data & 0xf00)>>8;    // FX-00
                IndexTmp +=33;
            }
            else if(dataH == 0xC000)             //c
            {
                IndexTmp = (data & 0xf00)>>8;    // FX-00
                IndexTmp +=49;
            }
            else if(dataH == 0x7000)             //U0
            {
                IndexTmp = (data & 0xf00)>>8;    // FX-00
                IndexTmp += 67;
            }
            else
            {
                errType = COMM_ERR_ADDR;
            }
            
            if(dataL >= funcCodeGradeAll[IndexTmp])
            {
                errType = COMM_ERR_ADDR;
            }
            return errType;
        }
    }
// 写锟斤拷锟斤拷锟斤拷
    else if ((high == 0x0000)      // Fx-RAM
             || (high == 0xF000)   // Fx
             || (high == 0xA000)   // Ax
             || (high == 0x4000)   // Ax-RAM
             || (high == 0xB000)   // Bx
             || (high == 0x5000)   // Bx-RAM
             || (high == 0xC000)   // Cx
             || (high == 0x6000)   // Cx-RAM
             || ((addr & 0xFF00) == 0x1F00)          // FP锟斤拷1Fxx
             || ((addr & 0xFF00) == 0x7300)   // U3
        ) 
    {
        errType = CommRwFuncodeAtrrib(addr,data,COMM_WRITE_FC);
    }
    // 为统一锟斤拷址
    else if((addr & 0xFF00) == 0x7000)
    {
        errType = COMM_ERR_READ_ONLY;
    }
// 锟斤拷址越锟斤拷
    else
    {
        errType = COMM_ERR_ADDR;
    }
    return errType;  
}
/*******************************************************************************
// 锟斤拷锟斤拷锟斤拷锟斤拷         : Uint16 CommWrite(Uint16 addr, Uint16 data)
// 锟斤拷诓锟斤拷锟�			: addr  data
// 锟斤拷锟斤拷				锟斤拷errType
// 锟斤拷锟斤拷	            : zhurf1513	
// 锟芥本		        : V0.71
// 时锟斤拷             : 2012-06-28
// 说锟斤拷				: 通讯写锟斤拷锟斤拷,锟斤拷锟斤拷锟叫讹拷锟斤拷锟皆匡拷写锟斤拷写锟斤拷拇锟斤拷锟斤拷锟�
********************************************************************************/
Uint16 CommWrite(Uint16 addr, Uint16 data)
{
   Uint16 errType = COMM_ERR_NONE;
   errType = ConmmWriteAtribte(addr, data);
   if(errType) // 锟斤拷锟皆癸拷锟较ｏ拷直锟斤拷锟剿筹拷
   {
       return errType; 
   }
   else
   {
       errType = CommRwFuncCode(addr, data, COMM_WRITE_FC); // 锟斤拷锟斤拷写锟斤拷
   }
   return errType;

}
//
// 通讯锟斤拷锟斤拷锟捷猴拷锟斤拷

/*******************************************************************************
// 锟斤拷锟斤拷锟斤拷锟斤拷         : Uint16 CommReadAtribte(Uint16 addr, Uint16 data)
// 锟斤拷诓锟斤拷锟�			: addr  data
// 锟斤拷锟斤拷				锟斤拷errType
// 锟斤拷锟斤拷	            : zhurf1513	
// 锟芥本		        : V0.71
// 时锟斤拷             : 2012-06-28
// 说锟斤拷				: 通讯锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷
********************************************************************************/
Uint16 CommReadAtribte(Uint16 addr, Uint16 data)
{
    //int16 i;
    Uint16 high = (addr & 0xF000);
    Uint16 low = (addr & 0x00FF);
    Uint16 errType = COMM_ERR_NONE;
	
// 通讯锟斤拷取停锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷示锟斤拷锟斤拷
// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷取锟斤拷锟斤拷锟斤拷锟�
    if ((addr & 0xFF00) == 0x1000)      // 停锟斤拷/锟斤拷锟叫诧拷锟斤拷
    {
        if (low + data > COMM_PARA_NUM)
        {
            errType = COMM_ERR_ADDR;
        }
    }
// 锟斤拷取锟斤拷频锟斤拷锟斤拷锟斤拷状态
    else if ((COMM_STATUS_ADDR == addr)   // 锟斤拷取锟斤拷频锟斤拷锟斤拷锟斤拷状态
           ||(COMM_INV_ERROR == addr)     // 锟斤拷取锟斤拷锟斤拷
           ||(COMM_KEYBORD_TEST == addr)) // 锟斤拷取锟斤拷锟斤拷锟斤拷锟斤拷值 
    {
        if (data > 0x01)
        {
            errType = COMM_ERR_PARA;
        }
    }
#if FC_KEY_CONTROL_EN
	//锟斤拷取LCDJ锟斤拷锟斤拷DI锟斤拷锟斤拷状态
	else if(((addr & 0xFF00) == 0x7100)) //  &&(LcdKeyFlag == 1)
	{

	} 
#endif
// 锟斤拷取锟斤拷锟斤拷锟斤拷
    else if ((high == 0xF000) ||     // Fx, 锟斤拷取锟斤拷锟斤拷锟斤拷值
             (high == 0xA000) ||     // Ax
             (high == 0xB000) ||     // Bx
             (high == 0xC000) ||     // Cx
             (high == 0x7000) ||     // U0
             ((addr & 0xFF00) == 0x1F00)    // FP
        )
    {
        errType = CommRwFuncodeAtrrib(addr, data, COMM_READ_FC);
    }
// 锟斤拷址越锟斤拷
    else    
    {
        errType = COMM_ERR_ADDR;
    }
    return errType;
}
/*******************************************************************************
// 锟斤拷锟斤拷锟斤拷锟斤拷         : LOCALD Uint16 CommRead(Uint16 addr, Uint16 data)
// 锟斤拷诓锟斤拷锟�			: addr  data
// 锟斤拷锟斤拷				锟斤拷errType
// 锟斤拷锟斤拷	            : zhurf1513	
// 锟芥本		        : V0.71
// 时锟斤拷             : 2012-06-28
// 说锟斤拷				: 通讯锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫断可讹拷锟斤拷锟脚讹拷取锟侥达拷锟斤拷锟斤拷
********************************************************************************/
LOCALD Uint16 CommRead(Uint16 addr, Uint16 data)
{
   Uint16 errType = COMM_ERR_NONE;
   errType = CommReadAtribte(addr, data);
   if(errType) // 锟斤拷锟皆癸拷锟较ｏ拷直锟斤拷锟剿筹拷
   {
       return errType;
   }
   else
   {
       errType = CommRwFuncCode(addr, data, COMM_READ_FC); // 锟斤拷取
   }
   
   return errType;
}


Uint16 comIndex;
/*******************************************************************************
// 锟斤拷锟斤拷锟斤拷锟斤拷         :LOCALD Uint16 CommRwFuncodeAtrrib(Uint16 addr, Uint16 data, Uint16 rwMode)
// 锟斤拷诓锟斤拷锟�			: addr  data rwmode
// 锟斤拷锟斤拷				锟斤拷errType
// 锟斤拷锟斤拷	            : zhurf1513	
// 锟芥本		        : V0.71
// 时锟斤拷             : 2012-06-28
// 说锟斤拷				: 通讯锟斤拷写锟斤拷锟斤拷锟诫（锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟街凤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷卸峡啥锟叫达拷锟斤拷哦锟叫达拷拇锟斤拷锟斤拷锟�
********************************************************************************/
LOCALD Uint16 CommRwFuncodeAtrrib(Uint16 addr, Uint16 data, Uint16 rwMode)
{
    Uint16 index;
       Uint16 group;
       Uint16 grade;
       Uint16 gradeAdd;
       //int16 i;
       Uint16 tmp;
       //Uint16 highH;
       Uint16 funcCodeGradeComm[FUNCCODE_GROUP_NUM];
       Uint16 errType = COMM_ERR_NONE;
       // 锟斤拷霉锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
       CommGetFuncCodeGroupGrade(addr,&group, &grade);

   // 锟斤拷锟斤拷通讯锟斤拷锟斤拷拢锟斤拷锟矫恳籫roup锟斤拷锟矫伙拷锟斤拷锟皆诧拷锟斤拷锟斤拷grade锟斤拷锟斤拷
       UpdataFuncCodeGrade(funcCodeGradeComm);

   // 锟叫讹拷group, grade锟角凤拷锟斤拷锟�
       if (COMM_READ_FC == rwMode)     // 通讯锟斤拷锟斤拷锟斤拷锟斤拷
       {
           if(!data) // 锟斤拷取 0 锟斤拷锟斤拷锟斤拷
           {
              errType = COMM_ERR_PARA;
              return errType;
           }
           gradeAdd = data - 1;
       }
       else        // 通讯写锟斤拷锟斤拷锟斤拷
       {
           gradeAdd = 0;
       }

       if (grade + gradeAdd >= funcCodeGradeComm[group]) // 锟斤拷锟斤拷锟斤拷锟斤拷
       {
           errType = COMM_ERR_ADDR;
           return errType;
       }

    index = GetGradeIndex(group, grade);    // 锟斤拷锟姐功锟斤拷锟斤拷锟斤拷锟�
    
#if FC_KEY_CONTROL_EN 
   if(LcdKeyFlag !=1)   //锟斤拷锟斤拷1时锟斤拷示液锟斤拷锟斤拷锟教斤拷锟斤拷
#endif
   {
    if ((COMM_NO_RW_FC_0 == index) ||       // 某些锟斤拷锟斤拷锟诫，通讯锟斤拷锟杰斤拷锟斤拷RW
        //(COMM_NO_RW_FC_1 == index) ||
        ((COMM_WRITE_FC == rwMode) &&       // 某些锟斤拷锟斤拷耄ㄑ讹拷锟斤拷芙锟斤拷锟絎
         (//(COMM_NO_W_FC_0 == index) ||
          (COMM_NO_W_FC_1 == index) ||
          //(COMM_NO_W_FC_2 == index) ||
             //(COMM_NO_W_FC_3 == index) ||
             //(COMM_NO_W_FC_4 == index) ||
             (COMM_NO_W_FC_5 == index)
          )
         ) ||
        ((COMM_READ_FC == rwMode) &&
         ((COMM_NO_R_FC_0 == index)         // 某些锟斤拷锟斤拷锟诫，通讯锟斤拷锟杰斤拷锟斤拷R
          )
         ) 
           )
       {
           errType = COMM_ERR_ADDR;            // 锟斤拷效锟斤拷址
           return errType;
       }
    }

   #if DEBUG_MD290_SEARIS   // 锟斤拷锟轿癸拷锟斤拷锟诫，通讯锟斤拷锟杰斤拷锟斤拷RW
       if (((index >= GetCodeIndex(funcCode.code.motorParaM1.elem.rsvdF11[0]))
           &&(index <= GetCodeIndex(funcCode.code.pgParaM1.elem.fvcPgLoseTime))
           )
           ||((index >= GetCodeIndex(funcCode.code.errorAction[4]))
           &&(index <= GetCodeIndex(funcCode.code.errorShow[1]))
           )
           ||((index >= GetCodeIndex(funcCode.code.osChkValue))
           &&(index <= GetCodeIndex(funcCode.code.devChkTime))
           )
           ||((index >= GetCodeIndex(funcCode.code.motorFcM2.motorPara.elem.rsvdF11[0]))
           &&(index <= GetCodeIndex(funcCode.code.motorFcM2.pgPara.elem.fvcPgLoseTime))
           )
           ||((index >= GetCodeIndex(funcCode.code.motorFcM2.vcPara.vcSpdLoopKp1))
           &&(index <= GetCodeIndex(funcCode.code.motorFcM2.vcPara.spdLoopI))
           )
           ||(index ==  GetCodeIndex(funcCode.code.svcMode))
          )
       {
           errType = COMM_ERR_ADDR;            // 锟斤拷效锟斤拷址
           return errType;
       }
   #endif

       // 锟斤拷锟斤拷校锟斤拷
       if (COMM_WRITE_FC == rwMode) // 写
       {
           // U3锟斤拷
           if (group == FUNCCODE_GROUP_U3)
           {
               // 写锟斤拷U3 PLC锟斤拷锟斤拷锟接诧拷锟斤拷锟斤拷锟斤拷息
               funcCode.all[index] = data;
               return errType;
           }
           else if (GetCodeIndex(funcCode.code.factoryPassword) == index) // FF-00/0F-00, 锟斤拷锟斤拷锟斤拷锟斤拷
           {
               if (COMPANY_PASSWORD == data)   // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷确
               {
                   companyPwdPass4Comm = 1;
               }
               else
               {
                   errType = COMM_ERR_PARA;        // 锟斤拷效锟斤拷锟斤拷
               }

               return errType;
           }
           else if (GetCodeIndex(funcCode.code.userPassword) == index) // 锟矫伙拷锟斤拷锟斤拷
           {
              #if FC_KEY_CONTROL_EN
             if(LcdKeyFlag !=1)//锟斤拷锟斤拷1锟斤拷示液锟斤拷锟斤拷锟教斤拷锟斤拷
             #endif
             {
               if (data == funcCode.code.userPassword)
               {
                   sciFlag.bit.pwdPass = 1;
                   userPwdPass4Comm = 1;
               }
               else
               {
                errType = COMM_ERR_PWD;
            }
            
            return errType;
        }
      }
    }

    if ((group == FC_GROUP_FACTORY) && (!companyPwdPass4Comm)) // FF锟斤拷
    {
        errType = COMM_ERR_SYSTEM_LOCKED;   // 系统(锟斤拷锟揭癸拷锟斤拷锟斤拷)锟斤拷锟斤拷     
        return errType;
       }

       if ((group == FUNCCODE_GROUP_AF) && (!groupHidePassComm))  // AF锟斤拷
       {
           errType = COMM_ERR_SYSTEM_LOCKED;   // 系统(锟斤拷锟揭癸拷锟斤拷锟斤拷)锟斤拷锟斤拷
           return errType;
       }

       if (COMM_READ_FC == rwMode)     // 通讯锟斤拷锟斤拷锟斤拷锟斤拷
       {
           return errType;     // 锟斤拷锟芥都锟斤拷通讯写锟斤拷锟斤拷锟斤拷拇锟斤拷锟�
       }

       tmp = ModifyFunccodeUpDown(index, &data, 0);
       if (COMM_ERR_PARA == tmp)
       {
           errType = COMM_ERR_PARA;
       }
       else if (COMM_ERR_READ_ONLY == tmp)
       {
           errType = COMM_ERR_READ_ONLY;
       }
       else
       {
           if (GetCodeIndex(funcCode.code.tuneCmd) == index)
           {
               errType = ValidateTuneCmd(data, MOTOR_SN_1);
               return errType;
           }
           else if (GetCodeIndex(funcCode.code.paraInitMode) == index) // 锟斤拷锟斤拷锟斤拷锟绞硷拷锟�
           {
               if (!userPwdPass4Comm)
               {
                   errType = COMM_ERR_SYSTEM_LOCKED;
               }
               else if (FUNCCODE_RW_MODE_NO_OPERATION != funcCodeRwMode) // 锟斤拷锟节诧拷锟斤拷锟斤拷锟斤拷锟斤拷
               {
                   errType = COMM_ERR_SAVE_FUNCCODE_BUSY;
               }
               else if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA == data) // 锟街革拷锟斤拷锟斤拷锟斤拷锟斤拷(锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�)
               {
                   funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA;
               }
               else if (FUNCCODE_paraInitMode_SAVE_USER_PARA == data)    // 锟斤拷锟斤拷锟矫伙拷锟斤拷锟斤拷
               {
                   funcCodeRwModeTmp = FUNCCODE_paraInitMode_SAVE_USER_PARA;
               }
               else if (FUNCCODE_paraInitMode_RESTORE_USER_PARA == data) // 锟街革拷锟矫伙拷锟斤拷锟斤拷牟锟斤拷锟�
               {

                   if ((funcCode.code.saveUserParaFlag1 == USER_PARA_SAVE_FLAG1)
                       && (funcCode.code.saveUserParaFlag2 == USER_PARA_SAVE_FLAG2))
                   {
                       funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_USER_PARA;
                   }
                   else
                   {
                       errType = COMM_ERR_PARA;
                   }
               }
               else if (FUNCCODE_paraInitMode_CLEAR_RECORD == data) // 锟斤拷锟斤拷锟铰硷拷锟较�
               {
                   ClearRecordDeal();
               }

               else if (FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL == data) // 锟街革拷锟斤拷锟斤拷锟斤拷锟斤拷(锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�)
               {
                   //funcCodeRwModeTmp = FUNCCODE_paraInitMode_RESTORE_COMPANY_PARA_ALL;
               }
               return errType;
           }
           if (GetCodeIndex(funcCode.code.inverterType) == index) // 锟睫改憋拷频锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷
           {
               // 锟斤拷锟酵诧拷锟杰筹拷锟斤拷锟斤拷围
               errType = ValidateInvType();
        }
       // 锟斤拷DI5锟斤拷锟皆讹拷锟斤拷为DI_FUNC_APTP_ZERO
           else if ((GetCodeIndex(funcCode.code.diFunc[0]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[1]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[2]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[3]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[5]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[6]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[7]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[8]) == index)
           || (GetCodeIndex(funcCode.code.diFunc[9]) == index)
           )
           {
            if (data == DI_FUNC_APTP_ZERO)
            {
                   errType = COMM_ERR_PARA;
               }
           }

       }

       return errType;
}
/*******************************************************************************
// 锟斤拷锟斤拷锟斤拷锟斤拷         : Uint16 CommRwFuncCode(Uint16 addr, Uint16 data, Uint16 rwMode)
// 锟斤拷诓锟斤拷锟�			: 锟斤拷址addr  锟斤拷锟斤拷data  锟斤拷写模式rwMode
// 锟斤拷锟斤拷				锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷errType 0 锟斤拷示锟睫达拷锟斤拷
// 锟斤拷锟斤拷	            : zhurf1513	
// 锟芥本		        : V0.71
// 时锟斤拷             : 2012-06-28
// 说锟斤拷				: 锟斤拷写锟斤拷锟斤拷锟诫，直锟接讹拷写锟斤拷没锟叫撅拷锟斤拷锟轿猴拷锟斤拷锟皆碉拷锟叫讹拷
********************************************************************************/
Uint16 CommRwFuncCode(Uint16 addr, Uint16 data, Uint16 rwMode)
{
    Uint16 highH;
    Uint16 errType = 0;
    Uint16 group = 0,grade = 0;
    int16 i;
    Uint16 Index;
    static  Uint16 canOpen = 0;
    Uint16 low = (addr & 0x00FF);
    highH = (addr & 0xF000);
    // 锟斤拷取锟斤拷锟杰诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
    CommGetFuncCodeGroupGrade(addr,&group,&grade);
    // 锟斤拷取锟铰憋拷
    Index = GetGradeIndex(group, grade); 
    if(COMM_WRITE_FC==rwMode)
    {
    // 锟斤拷锟斤拷锟斤拷锟斤拷
        if (COMM_KEYBORD_TEST == addr)
        {
            if (data == 1)
            {
                // 锟斤拷始锟斤拷锟斤拷锟叫断诧拷锟斤拷
                keyBordTestFlag = 1;
                keyBordValue = 0;
            }
        }
    // 通讯锟借定值(通讯频锟斤拷锟借定)
        else if (COMM_SET_VALUE_ADDR == addr)
        {
            funcCode.code.frqComm = data;
            // 锟斤拷锟斤拷F0-28锟斤拷锟斤拷锟斤拷锟矫ｏ拷1000锟斤拷址锟斤拷锟斤拷写
            dpFrqAim = (int32)(int16)funcCode.code.frqComm * maxFrq / 10000;
        }
    // 通讯锟斤拷锟斤拷锟斤拷锟筋处锟斤拷
        else if (COMM_CMD1_ADDR == addr)                 
        {
            if (data == 8)
            {
                if (canOpen == 0)
                {
                    commRunCmd = 1;
                    canOpen = 1;
                }
                else
                {
                    commRunCmd = 2;
                    canOpen = 0;                
                }
            }else if ((1 <= data) && (data <= 7)) // 锟斤拷0001-0007锟斤拷锟斤拷
            {
                commRunCmd = data;
            } 

        }
    // DO锟斤拷锟斤拷
        else if (COMM_DO_ADDR == addr)            
        {

            doComm.all = data;
            
        }
    // HDO锟斤拷锟斤拷
        else if (COMM_HDO_ADDR == addr)            
        {

            aoComm[0] = data;
        }
    // AO1锟斤拷锟斤拷
        else if (COMM_AO1_ADDR == addr)            
        {
            aoComm[1] = data;
        }
    // AO2锟斤拷锟斤拷
        else if (COMM_AO2_ADDR == addr)            
        {
            aoComm[2] = data;
        }
		else if ((Index == GetCodeIndex(funcCode.code.tuneCmd))       // F1-12锟斤拷锟斤拷写EEPROM
		|| (Index == GetCodeIndex(funcCode.code.motorFcM2.tuneCmd))   // A2-37锟斤拷锟斤拷写EEPROM
		|| (Index == GetCodeIndex(funcCode.code.paraInitMode))        // F5-01锟斤拷锟斤拷写EEPROM
		|| (Index == GetCodeIndex(funcCode.code.factoryPassword))     // FF-00锟斤拷锟斤拷写EEPROM
		|| (Index == GetCodeIndex(funcCode.code.userPassword))        // F5-03锟斤拷锟斤拷写EEPROM
		|| (highH == 0x7300)                                           // U3锟介不锟斤拷写EEPROM
		)
		{
			errType = COMM_ERR_NONE;
		} 
        else if (Index == GetCodeIndex(funcCode.code.checkDigitalTube))  //F7-00锟斤拷锟斤拷锟斤拷EEPROM
        {
            funcCode.all[Index] = data;   //RAM
        }

    // 写锟斤拷锟斤拷锟斤拷锟斤拷锟�
        else if  ((highH == 0x0000)      // Fx-RAM
                 || (highH == 0xF000)   // Fx
                 || (highH == 0xA000)   // Ax
                 || (highH == 0x4000)   // Ax-RAM
                 || (highH == 0xB000)   // Bx
                 || (highH == 0x5000)   // Bx-RAM
                 || (highH == 0xC000)   // Cx
                 || (highH == 0x6000)   // Cx-RAM
                 || ((addr & 0xFF00) == 0x1F00)   // FP锟斤拷1Fxx
                 || ((addr & 0xFF00) == 0x7300)   // U3
                 ||((addr & 0xFF00) == 0x2F00)    // 通锟斤拷专锟斤拷 FE
            )// 通讯写锟斤拷锟斤拷锟斤拷
        {
            Uint16 dataH,dataL,IndexTmp;
            if((addr & 0xFF00) == 0x2F00)
            {
                dataH = data & 0xf000;
                dataL = data & 0x00ff;
                if(dataH == 0xf000)                  // F
                {
                    IndexTmp = (data & 0xf00)>>8;    // FX-00
                }
               // else if(dataH == 0x1000)// 0x1fxx  fp
			    else if((data & 0xff00) == 0x1F00)  //by modfied yz1990 2014-06-16
                {
                    IndexTmp = 16;
                }
                else if(dataH == 0xA000)             // A
                {
                    IndexTmp = (data & 0xf00)>>8;    // FX-00
                    IndexTmp +=17;
                }
                else if(dataH == 0xB000)             // B
                {
                    IndexTmp = (data & 0xf00)>>8;    // FX-00
                    IndexTmp +=33;
                }
                else if(dataH == 0xC000)             //c
                {
                    IndexTmp = (data & 0xf00)>>8;    // FX-00
                    IndexTmp +=49;
                }
                else if(dataH == 0x7000)             //UX-00
                {
                    IndexTmp = (data & 0xf00)>>8;    // FX-00
                    IndexTmp += 67;
                }
                else
                {
                    errType = COMM_ERR_ADDR;
                    return errType;
                }

                if(dataL >= funcCodeGradeAll[IndexTmp])
                {
                    errType = COMM_ERR_ADDR;
                }
                else
                {
                    data = IndexTmp*100+dataL;
                    ModifyFunccodeEnter(Index, data); 
                }
            }
            else if (COMM_ERR_PARA == ModifyFunccodeEnter(Index, data))
            {
                errType = COMM_ERR_PARA;
            }
            else
            {
                if ((highH >= 0xA000)                                            // 为锟斤拷锟斤拷EEPROM锟斤拷址
                    && (commRcvData.commCmdSaveEeprom == SCI_WRITE_WITH_EEPROM)      // 锟叫憋拷锟斤拷EEPROM锟斤拷锟斤拷
    			)
                {
                    if (FUNCCODE_RW_MODE_NO_OPERATION == funcCodeRwMode)
                    {
                        SaveOneFuncCode(Index);
                    }
                    else                            // 锟斤拷锟节诧拷锟斤拷EEPROM锟斤拷锟斤拷锟斤拷应
                    {
                        errType = COMM_ERR_SAVE_FUNCCODE_BUSY;
                    }
                }
            }
        }     

    }
    // 通讯锟斤拷锟斤拷锟斤拷锟斤拷
    else 
    {
    // 通讯锟斤拷取停锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷示锟斤拷锟斤拷
    // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷取锟斤拷锟斤拷锟斤拷锟�
        if ((addr & 0xFF00) == 0x1000)      // 停锟斤拷/锟斤拷锟叫诧拷锟斤拷
        {
            for (i = 0; i < data; i++)
            {
                commReadData[i] = funcCode.group.u0[commDispIndex[i + low]];
                
                // 为通讯锟斤拷取锟斤拷锟斤拷
                if ((i + low) == DISP_OUT_CURRENT)
                {
                    // 通讯锟斤拷取锟斤拷锟斤拷锟街憋拷锟斤拷为0.1A
                    if ((funcCode.code.commReadCurrentPoint)
                        ||(funcCode.code.commProtocolSec == EXTEND_COM_CAR)  // DP || CANOPEN 锟教伙拷1锟斤拷小锟斤拷锟斤拷 
                        )
                    {
                        if (invPara.type <= invPara.pointLimit) // 小锟斤拷锟斤拷2小锟斤拷锟斤拷
                        commReadData[i] = commReadData[i] / 10;
                    }
                }
            }
        }
    // 锟斤拷取锟斤拷频锟斤拷锟斤拷锟斤拷状态
        else if (COMM_STATUS_ADDR == addr)         
        {

#if FC_KEY_CONTROL_EN
           	 if(tuneCmd&&(LcdKeyFlag ==1))
    		{
    		   if (runFlag.bit.run)
    		   {
    		       commReadData[0] = 0x0005;
    		   }		   
    		   else
    		   {
    		   	   commReadData[0] = 0x0004;
    		   }
    		} 
    		else  
#endif
             if (runFlag.bit.run)
            {
                if (FORWARD_DIR == runFlag.bit.dir) // F0-12之前锟侥凤拷锟斤拷
                {
                    commReadData[0] = 0x0001;
                }
                else
                {
                    commReadData[0] = 0x0002;
                }
            }
            else
            {
                commReadData[0] = 0x0003;
            }
            
        }
    // 锟斤拷取锟斤拷锟斤拷
        else if (COMM_INV_ERROR == addr)     
        {
            commReadData[0] = errorCode;

        }
    // 锟斤拷取锟斤拷锟斤拷锟斤拷锟斤拷值
        else if (COMM_KEYBORD_TEST == addr)    
        {
            if (keyBordValue == 0x01FF)
            {
                commReadData[0] = 1;
            }
            else
            {
                commReadData[0] = 0;
            }

        }
#if FC_KEY_CONTROL_EN
    	//锟斤拷取LCDJ锟斤拷锟斤拷DI锟斤拷锟斤拷状态
    	else if(((addr & 0xFF00) == 0x7100)) //  &&(LcdKeyFlag == 1)
    	{
          
             switch(low)
             {
    	         case 0:
    			     commReadData[0] =(diStatus.a.all>>16) & 0x0000FFFF;   
    	             break;
    			 case 1:
    			      commReadData[0] = diStatus.a.all & 0x0000FFFF;  
    	             break;
    			 case 2:
    			      commReadData[0] = 0; 
    	             break;
    			 case 3:
    			      commReadData[0] = doStatus.a.all & 0x0000FFFF;  
    	             break;
    			case 4:
    			     commReadData[0] = (diFunc.f1.all>>16) & 0x0000FFFF;  
    	             break;
    			case 5:
    			     commReadData[0] = diFunc.f1.all & 0x0000FFFF;  
    	             break;
    			case 6:
    			     commReadData[0] = (diFunc.f2.all>>16) & 0x0000FFFF;   
    	             break;
    			 case 7:
    			      commReadData[0] = diFunc.f2.all & 0x0000FFFF;   
    	             break;
    			default:break;
             } 
            
    	} 
#endif
    // 锟斤拷取锟斤拷锟斤拷锟斤拷
        else if ((highH == 0xF000) ||     // Fx, 锟斤拷取锟斤拷锟斤拷锟斤拷值
                 (highH == 0xA000) ||     // Ax
                 (highH == 0xB000) ||     // Bx
                 (highH == 0xC000) ||     // Cx
                 (highH == 0x7000) ||     // U0
                 ((addr & 0xFF00) == 0x1F00)    // FP
            )
        {
            for (i = 0; i < data; i++, Index++)
            {
                commReadData[i] = funcCode.all[Index];  // U0锟斤拷也锟斤拷锟斤拷

                // 为通讯锟斤拷取锟斤拷锟斤拷
                if (COMM_READ_CURRENT_FC == Index)
                {
                    // 通讯锟斤拷取锟斤拷锟斤拷锟街憋拷锟斤拷为0.1A
                    if ((funcCode.code.commReadCurrentPoint)
                        ||(funcCode.code.commProtocolSec == EXTEND_COM_CAR)  // DP || CANOPEN 锟教伙拷1锟斤拷小锟斤拷锟斤拷 
                        )
                    {   
                        if (invPara.type <= invPara.pointLimit) // 小锟斤拷锟斤拷2小锟斤拷锟斤拷
                        commReadData[i] = commReadData[i] / 10;
                    }
                }
            }
//锟斤拷取FE锟介，锟斤拷实锟斤拷锟斤拷FE锟斤拷锟斤拷锟斤拷锟斤拷取FE锟斤拷锟斤拷映锟斤拷牟锟斤拷锟斤拷锟斤拷锟街� by modfied yz1990 2014-07-15
			if((addr & 0xFF00) == 0xFE00)
			{ 		
                for (i = 0; i < data; i++)
                {				
                    FEHigh_group = (funcCode.group.fe[(addr & 0xFF)+i] / 100); //去FE锟斤拷映锟斤拷锟街凤拷锟轿伙拷锟阶拷锟斤拷锟斤拷锟紽E锟斤拷映锟斤拷锟街凤拷锟轿拷锟斤拷锟� 锟斤拷FE-02=uF3-04 锟斤拷锟斤拷锟斤拷FE02锟斤拷值为304锟斤拷锟斤拷锟斤拷0xF304;A1-05,1805,目锟侥猴拷funcCodeGradeSum[FUNCCODE_GROUP_NUM]锟斤拷哦锟接�
			        FELow_grade  = (funcCode.group.fe[(addr & 0xFF)+i] % 100);
           
                    commReadData[i] = funcCode.all[GetGradeIndex(FEHigh_group, FELow_grade)];  // U0锟斤拷也锟斤拷锟斤拷

                   // 为通讯锟斤拷取锟斤拷锟斤拷				   
				   if (COMM_READ_CURRENT_FC == GetGradeIndex(FEHigh_group, FELow_grade))
				   {
                       // 通讯锟斤拷取锟斤拷锟斤拷锟街憋拷锟斤拷为0.1A
                       if ((funcCode.code.commReadCurrentPoint)
                          ||(funcCode.code.commProtocolSec == EXTEND_COM_CAR)  // DP || CANOPEN 锟教伙拷1锟斤拷小锟斤拷锟斤拷 
                          )
                       {   
                           if (invPara.type <= invPara.pointLimit) // 小锟斤拷锟斤拷2小锟斤拷锟斤拷
                           commReadData[i] = commReadData[i] / 10;
                       }
                   }
               
             	}
			}
        }
    }	
        return errType;
        
}

/*******************************************************************************
// 锟斤拷锟斤拷锟斤拷锟斤拷         : Uint16 CommGetFuncCodeIndex(Uint16 addr, Uint16 data)
// 锟斤拷诓锟斤拷锟�			: 锟斤拷址addr  锟斤拷锟斤拷data  
// 锟斤拷锟斤拷				锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷errType 0 锟斤拷示锟睫达拷锟斤拷
// 锟斤拷锟斤拷	            : zhurf1513	
// 锟芥本		        : V0.71
// 时锟斤拷             : 2012-06-28
// 说锟斤拷				: 锟斤拷取通讯锟斤拷锟斤拷锟斤拷锟斤拷卤锟�
********************************************************************************/
void CommGetFuncCodeGroupGrade(Uint16 addr, Uint16 *group, Uint16 *grade)
{
    Uint16 highH;
    //Uint16 Index;
    highH = (addr & 0xF000);
    // 锟斤拷取group, grade
    *group = (addr >> 8) & 0x0F;
    *grade = addr & 0xFF;

    // 为FE锟介功锟杰诧拷锟斤拷
#if DEBUG_MD500_SEARIS||DEBUG_MD290_SEARIS
    if ((((addr & 0xFF00) == 0xFE00)||((addr & 0xFF00) == 0x0E00))
        && (LcdReadFEflag!=1)
        )
#else
    if (((addr & 0xFF00) == 0xFE00)
        && (LcdReadFEflag!=1)
        )
#endif
    {
        *group = funcCode.group.fe[*grade] / 100;
        *grade = funcCode.group.fe[*grade] % 100;
    }

    LcdReadFEflag = 0;
    if((addr & 0xFF00) == 0x2F00)
    {
        *group = FUNCCODE_GROUP_FE;
    }
    
    if ((0xA000 == highH) || (0x4000 == highH))       // Ax
    {
        *group += FUNCCODE_GROUP_A0;
    }
    else if ((0xB000 == highH) || (0x5000 == highH))  // Bx
    {
        *group += FUNCCODE_GROUP_B0;
    }
    else if ((0xC000 == highH) || (0x6000 == highH))  // Cx
    {
        *group += FUNCCODE_GROUP_C0;
    }
    else if ((addr & 0xFF00) == 0x1F00)  // FP
    {
        *group = FUNCCODE_GROUP_FP;
    }
    else if (0x7000 == highH)            // U0
    {
        *group += FUNCCODE_GROUP_U0;
    }
////  锟斤拷取锟铰憋拷锟斤拷锟�
}


//=====================================================================
//
// 通讯锟叫断凤拷锟酵达拷锟斤拷锟斤拷锟斤拷
//
//=====================================================================
LOCALF void inline CommStartSend(void)
{
#if DSP_2803X
    LinaRegs.SCITD = sendFrame[0];     // 锟斤拷锟酵碉拷一锟斤拷锟斤拷锟斤拷
#else
    SciaRegs.SCITXBUF = sendFrame[0];     // 锟斤拷锟酵碉拷一锟斤拷锟斤拷锟斤拷
#endif
}


//=====================================================================
//
// 通讯锟斤拷锟捷斤拷锟斤拷锟斤拷锟斤拷
//
//=====================================================================
#if DSP_2803X
void resetLinSci(void)
{
    LinaRegs.SCIGCR0.bit.RESET = 0;
    LinaRegs.SCIGCR0.bit.RESET = 1;
    LinaRegs.SCIGCR1.bit.SWnRST = 0; 
}


void closeRTX(void)
{
    LinaRegs.SCIGCR1.bit.RXENA = 0;
    LinaRegs.SCIGCR1.bit.TXENA = 0;
}


void setRxConfig(void)
{
    LinaRegs.SCIGCR1.bit.RXENA = 1;
    LinaRegs.SCIGCR1.bit.TXENA = 0;
}


void setTxConfig(void)
{ 
    LinaRegs.SCIGCR1.bit.TXENA = 1;
    LinaRegs.SCIGCR1.bit.RXENA = 0;
}
#endif


//====================================================================
//
// 通讯锟斤拷锟斤拷锟斤拷锟斤拷
//
//===================================================================
void commErrorDeal(void)
{
    errorOther = ERROR_COMM;
    commErrInfo = COMM_ERROR_MODBUS;
    commStatus = SCI_RECEIVE_DATA;        // 锟斤拷为锟斤拷锟斤拷状态
    SET_RTS_R;  // RTS = RS485_R;         // RTS锟斤拷为锟斤拷锟斤拷
    
    #if DSP_2803X
    EALLOW;        
    setRxConfig();
    EDIS;
    #else
    SciaRegs.SCICTL1.all = 0x0021;  // 锟斤拷锟斤拷
    SciaRegs.SCICTL2.all = 0x00C2;  // 锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷
    #endif
}


//====================================================================
//
// 通讯状态锟斤拷锟斤拷
//
//===================================================================
void commStatusDeal(void)
{
    switch (commStatus)
    {
        // 锟斤拷锟斤拷锟斤拷锟斤拷
        case SCI_RECEIVE_DATA:
            SET_RTS_R;             // RTS = RS485_R;   // RTS锟斤拷为锟斤拷锟斤拷,锟斤拷止状态锟斤拷锟斤拷
            break;
			
        case SCI_RECEIVE_OK:
            CommRcvDataDeal();
            // 锟斤拷锟斤拷锟斤拷锟斤拷
            if ((commRcvData.slaveAddr)              // 锟角广播
                && ((!sciFlag.bit.crcChkErr)         // CRC校锟斤拷晒锟斤拷锟轿狿ROFIBUS协锟斤拷
                || (funcCode.code.commProtocolSec == EXTEND_COM_CAR))
                )
            {
                CommSendDataDeal();                 // 锟斤拷锟斤拷锟斤拷锟斤拷准锟斤拷
                commStatus = SCI_SEND_DATA_PREPARE; // 锟斤拷锟秸达拷锟斤拷锟斤拷桑锟阶硷拷锟斤拷锟斤拷锟�
            }
            else                                    // 锟姐播锟斤拷DSP锟斤拷应之锟襟不凤拷锟酵ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷
            {
                commStatus = SCI_RECEIVE_DATA;
                SET_RTS_R;  // RTS = RS485_R;       // RTS锟斤拷为锟斤拷锟斤拷
                #if DSP_2803X
                EALLOW;
                setRxConfig();
                EDIS;
                #else
                SciaRegs.SCICTL1.all = 0x0021;  // 锟斤拷锟斤拷
                SciaRegs.SCICTL2.all = 0x00C2;  // 锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷
                #endif
                break;
            }

        // 锟斤拷锟斤拷锟斤拷锟斤拷准锟斤拷    
        case SCI_SEND_DATA_PREPARE:
            if ((commTicker >= commRcvData.delay)               // 应锟斤拷锟接筹拷
                && (commTicker > commRcvData.frameSpaceTime))   // MODBUS为3.5锟斤拷锟街凤拷时锟斤拷
            {
                SET_RTS_T;  // RTS = RS485_T;                          // RTS锟斤拷为锟斤拷锟斤拷
                #if DSP_2803X
                EALLOW;            
                setTxConfig();
                EDIS;
                #else                    
                SciaRegs.SCICTL1.all = 0x0022;          // 锟斤拷锟斤拷
                SciaRegs.SCICTL2.all = 0x00C1;          // 锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷
                #endif
                commStatus = SCI_SEND_DATA;
                commSendData.sendNum = 1;               // 锟斤拷前锟斤拷锟斤拷锟斤拷锟捷革拷锟斤拷锟斤拷为1
                CommStartSend();                        // 锟斤拷始锟斤拷锟斤拷
                commSendTicker = 0;
            }
            break;

        // 锟斤拷锟斤拷锟斤拷锟斤拷
        case SCI_SEND_DATA:
            commSendTicker++;
            SET_RTS_T;  // RTS = RS485_T;     // RTS锟斤拷为锟斤拷锟斤拷,锟斤拷止状态锟斤拷锟斤拷
            // 锟斤拷时锟斤拷锟斤拷锟捷凤拷锟酵诧拷锟缴癸拷(1.5锟斤拷)
            if (commSendTicker >= ((Uint32)825L*commSendData.sendNumMax/(commBaudRate*10) + 1))   // *2ms
            {
                commStatus = SCI_SEND_OK;
            }
            else
            {
                break;
            }
            
        // 锟斤拷锟斤拷锟斤拷锟斤拷OK
        case SCI_SEND_OK:
            #if DSP_2803X
			if (LinaRegs.SCIFLR.bit.TXEMPTY)
            #else
            if (SciaRegs.SCICTL2.bit.TXEMPTY)   // Transmitter empty flag, 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
            #endif
            {
                commStatus = SCI_RECEIVE_DATA;  // 锟斤拷锟斤拷锟斤拷虾锟斤拷锟轿拷锟斤拷锟阶刺�
                SET_RTS_R;  // RTS = RS485_R;                  // RTS锟斤拷为锟斤拷锟斤拷
                #if DSP_2803X
                EALLOW;
                setRxConfig();
                EDIS;
                #else
                SciaRegs.SCICTL1.all = 0x0021;  // 锟斤拷锟斤拷
                SciaRegs.SCICTL2.all = 0x00C2;  // 锟斤拷锟斤拷锟斤拷锟斤拷锟叫讹拷
                #endif
            }
            break;

        default:
            break;
    }
}


//====================================================================
//
// 通讯锟斤拷锟捷斤拷锟秸达拷锟斤拷
//
//===================================================================
void CommDataReRcv(Uint16 tmp)
{
    if (commRcvData.rcvNum < commRcvData.rcvNumMax)  // 锟斤拷锟斤拷一帧锟斤拷锟捷伙拷没锟斤拷锟斤拷锟�
    {
        rcvFrame[commRcvData.rcvNum++] = tmp;
    }

    if(funcCode.code.commProtocolSec == EXTEND_COM_CAR)    //   锟叫讹拷锟斤拷DP模式锟铰碉拷专锟斤拷协锟介还锟角憋拷准MODBUS
    {
        if((rcvFrame[1] ==SCI_CMD_READ)
            ||(rcvFrame[1] ==SCI_CMD_WRITE)
            ||(rcvFrame[1] ==SCI_CMD_WRITE_MORE))
        {
            commRcvData.dpOrModbus = MD; //  2:modbus
            commRcvData.rcvNumMax = 8;
            // 01 10 f0 08 00 02 04
            if((rcvFrame[1] ==SCI_CMD_WRITE_MORE)&&(commRcvData.rcvNum>=8))
            {
                commRcvData.rcvNumMax = rcvFrame[6] + 9;
            }
        }
        else
        {
            #if 1
            commRcvData.dpOrModbus = DP; // 1:dp 
            commRcvData.rcvNumMax = 34;
            #endif
        }
    }
    else
    {
        commRcvData.dpOrModbus = MD;
        commRcvData.rcvNumMax = 8;
        // 01 10 f0 08 00 02 04
        if((rcvFrame[1] ==SCI_CMD_WRITE_MORE)&&(commRcvData.rcvNum>=8))
        {
            commRcvData.rcvNumMax = rcvFrame[6] + 9;
        }
        
    }
    if (commRcvData.rcvNum >= commRcvData.rcvNumMax) // 锟斤拷锟斤拷一帧锟斤拷锟斤拷锟斤拷锟�
    {
        // PROFIBUS锟斤拷帧头锟叫断憋拷志
        commRcvData.rcvDataJuageFlag = 0;
        
        if (2 == commRcvData.rcvFlag)                // 锟斤拷锟斤拷锟斤拷址锟脚凤拷锟斤拷(DSP锟叫伙拷锟缴凤拷锟斤拷状态)
        {
            #if DSP_2803X
            EALLOW;
            closeRTX();                  // 锟截闭凤拷锟酵斤拷锟斤拷
            EDIS;
            #else
            SciaRegs.SCICTL1.all = 0x0004;      // 锟截闭凤拷锟酵斤拷锟秸ｏ拷sleep
            SciaRegs.SCICTL2.all = 0x00C0;      // 锟截闭斤拷锟秸凤拷锟斤拷锟叫讹拷
            #endif
        }
        
        commStatus = SCI_RECEIVE_OK;            // 锟斤拷志锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
        commRcvData.rcvFlag = 0;
    }
}


//====================================================================
//
// 通讯锟斤拷锟捷凤拷锟酵达拷锟斤拷
//
//===================================================================
void CommDataSend(void)
{
	 // 锟斤拷锟斤拷一帧锟斤拷锟斤拷没锟斤拷锟斤拷锟�
    if (commSendData.sendNum< commSendData.sendNumMax)          
    {
#if DSP_2803X
        LinaRegs.SCITD = sendFrame[commSendData.sendNum++];
#else
        SciaRegs.SCITXBUF = sendFrame[commSendData.sendNum++];
#endif
    }
	// 锟斤拷锟斤拷一帧锟斤拷锟斤拷全锟斤拷锟斤拷锟�
    else if (commSendData.sendNum >= commSendData.sendNumMax)     
    {
		// 锟斤拷志锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
        commStatus = SCI_SEND_OK;      
    }
}


//=====================================================================
//
// 通讯锟斤拷始锟斤拷
//
//=====================================================================
void InitSetScia(void)
{
    // 应锟矫凤拷锟斤拷前锟斤拷
    commStatus = SCI_RECEIVE_DATA;              // 锟斤拷锟斤拷状态   
    SET_RTS_R;  // RTS = RS485_R;               // RTS锟斤拷为锟斤拷锟斤拷
#if DSP_2803X
    EALLOW;
    // reset
	resetLinSci();

	LinaRegs.SCIGCR1.bit.SLEEP = 0;
    LinaRegs.SCIFLR.bit.TXWAKE = 0;  
    LinaRegs.SCIFLR.bit.TXEMPTY = 1;
    LinaRegs.SCIFLR.bit.TXRDY = 1;
	// 锟斤拷锟斤拷为锟斤拷锟捷斤拷锟斤拷
    setRxConfig(); 

	LinaRegs.SCIGCR1.bit.TIMINGMODE = 1; //Asynchronous Timing
	LinaRegs.SCIGCR1.bit.CLK_MASTER = 1; //Enable SCI Clock
	LinaRegs.SCIGCR1.bit.CONT = 1;		 //Continue on Suspend
	if (funcCode.code.commProtocolSec == EXTEND_COM_CAR)          // 锟斤拷校锟斤拷(8-N-1)
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[3];
    else
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[funcCode.code.commParity];
    LinaRegs.SCISETINT.bit.SETRXINT = 1;
    LinaRegs.SCISETINT.bit.SETTXINT = 1;
    LinaRegs.SCIFORMAT.bit.CHAR = 0x7;
    LinaRegs.SCIGCR1.bit.SWnRST = 1; 
       
EDIS;
#else
    SciaRegs.SCICTL1.all = 0x0001;              // SCI锟斤拷锟斤拷锟斤拷位锟斤拷锟斤拷锟斤拷效
    SciaRegs.SCICTL2.all = 0x00C2;
    SciaRegs.SCICCR.all = 0x0087;               // 2 stop bit, No loopback, No parity,8 bits,async mode,idle-line
    SciaRegs.SCIPRI.bit.FREE = 1;
    SciaRegs.SCICTL1.all = 0x0021;              // Relinquish SCI from Reset
#endif
}


//=====================================================================
//
// SCI通讯锟斤拷锟斤拷锟睫改猴拷锟斤拷
//
//=====================================================================
void UpdateSciFormat(void)
{
    Uint16 digit[5];
    Uint16 tmp;
    static Uint16 commParityBak = 0;
	// 锟斤拷锟酵ㄑ讹拷锟斤拷锟斤拷锟�
    GetNumberDigit1(digit, funcCode.code.commBaudRate);
    //commType = MODBUS;     // 通讯锟斤拷锟斤拷
    commType = 0;     // 通讯锟斤拷锟斤拷 MD DP CANOPEN 锟斤拷锟斤拷
    // 为PROFIBUS
    if (funcCode.code.commProtocolSec == EXTEND_COM_CAR)          // CANOPEN锟斤拷Modbus锟斤拷锟斤拷
    {
		tmp = digit[1] + 9;  // 锟斤拷锟斤拷锟斤拷 
    }
    // 为MODBUS
    else 
    {
		tmp = digit[commType]; // 锟斤拷锟斤拷锟斤拷
    }
    
    // 锟斤拷锟酵ㄑ讹拷锟斤拷荽锟斤拷透锟绞�   
    GetNumberDigit1(digit, funcCode.code.commProtocol);
    commProtocol = digit[commType];
    
	// 锟斤拷锟斤拷通讯锟斤拷锟斤拷锟侥硷拷
    protocolFunc[commType].UpdateCommFormat(dspBaudRegData[tmp].baudRate);

    commBaudRate = dspBaudRegData[tmp].baudRate;
    
 // 锟斤拷锟街斤拷锟秸癸拷锟斤拷时锟斤拷锟斤拷    
#if DSP_2803X
	if ( LinaRegs.SCIFLR.bit.BRKDT
        || LinaRegs.SCIFLR.bit.PE 
        || LinaRegs.SCIFLR.bit.OE
        || LinaRegs.SCIFLR.bit.FE)
#else
    if (SciaRegs.SCIRXST.bit.RXERROR)      
#endif
    {
        InitSetScia();   // 锟斤拷始锟斤拷SCI锟侥达拷锟斤拷
    }
#if DSP_2803X

EALLOW;
    if (funcCode.code.commProtocolSec == EXTEND_COM_CAR)          // 锟斤拷校锟斤拷(8-N-1)
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[3];
    else
        LinaRegs.SCIGCR1.all = (LinaRegs.SCIGCR1.all&0xFFFFFFE3)^commParitys[funcCode.code.commParity];
    
    LinaRegs.BRSR.bit.SCI_LIN_PSH = dspBaudRegData[tmp].high;
    LinaRegs.BRSR.bit.SCI_LIN_PSL = dspBaudRegData[tmp].low;
    LinaRegs.BRSR.bit.M = dspBaudRegData[tmp].M;
EDIS;
    if(commParityBak != funcCode.code.commParity ) // 锟斤拷锟捷革拷式锟斤拷锟斤拷锟侥憋拷锟斤拷要锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷 8-N-1 锟叫伙拷锟斤拷8-E/O-1锟斤拷锟斤拷锟斤拷
    {
        InitSetScia();   // 锟斤拷始锟斤拷SCI锟侥达拷锟斤拷
    }
    commParityBak = funcCode.code.commParity;
#else
    if (funcCode.code.commProtocolSec == EXTEND_COM_CAR)
        SciaRegs.SCICCR.all = commParitys[3];               // 锟斤拷校锟斤拷(8-N-1)
    else
        SciaRegs.SCICCR.all = commParitys[funcCode.code.commParity];
    
    SciaRegs.SCIHBAUD = dspBaudRegData[tmp].high;
    SciaRegs.SCILBAUD = dspBaudRegData[tmp].low;
    SciaRegs.SCICTL1.bit.SWRESET = 1;
#endif

}


#if 1//F_DEBUG_RAM    // 锟斤拷锟斤拷锟皆癸拷锟杰ｏ拷锟斤拷CCS锟斤拷build option锟叫讹拷锟斤拷暮锟�
void InitSciaGpio(void)
{
#if DSP_2803X  // 2803x平台
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO14 = 0;		// Enable pull-up for GPIO14 (LIN TX)
    GpioCtrlRegs.GPAPUD.bit.GPIO15 = 0;		// Enable pull-up for GPIO15 (LIN RX)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 3;  // Asynch input GPIO15 (LINRXA)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 0x01;  // No qualification for all group A GPIO 0-15
    GpioCtrlRegs.GPACTRL.bit.QUALPRD1 = 0x03;
    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 2;   // Configure GPIO14 for LIN TX operation (2-Enable,0-Disable)
    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 2;   // Configure GPIO15 for LIN RX operation (2-Enable,0-Disable)
    GpioCtrlRegs.GPBPUD.bit.GPIO39 = 0;    
    GpioCtrlRegs.GPBMUX1.bit.GPIO39 = 0;        // Configure GPIO39, RTS
    GpioCtrlRegs.GPBDIR.bit.GPIO39 = 1;         // output
    GpioDataRegs.GPBDAT.bit.GPIO39 = RS485_R;   // Receive
    EDIS;
// 通讯锟斤拷锟斤拷使锟斤拷锟叫断ｏ拷锟斤拷始锟斤拷
    EALLOW;
    #ifdef TARGET_GS32
    interrupt_register(INT_LINA, &Lina_Level0_ISR);
    interrupt_register(INT_LINB, &Lina_Level1_ISR);
    #else
    PieVectTable.LIN0INTA = &Lina_Level0_ISR;
    PieVectTable.LIN1INTA = &Lina_Level1_ISR;
    #endif
    EDIS;
	// IER |= M_INT9;   	                 // Enable interrupts:
    #ifdef TARGET_GS32
    interrupt_enable(INT_LINA);
    interrupt_enable(INT_LINB);
    #else
	PieCtrlRegs.PIEIER9.bit.INTx3=1;     // PIE Group 9, INT3
	PieCtrlRegs.PIEIER9.bit.INTx4=1;     // PIE Group 9, INT4	
    #endif
#else
    EALLOW;    
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // Enable pull-up for GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;    // Enable pull-up for GPIO29 (SCITXDA)
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // Asynch input GPIO28 (SCIRXDA)
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // Configure GPIO28 for SCIRXDA operation
    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // Configure GPIO29 for SCITXDA operation
    GpioCtrlRegs.GPAPUD.bit.GPIO27 = 0;    
    GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;        // Configure GPIO27, RTS
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;         // output
    GpioDataRegs.GPADAT.bit.GPIO27 = RS485_R;   // Receive
    EDIS;
    // 通讯锟斤拷锟斤拷使锟斤拷锟叫断ｏ拷锟斤拷始锟斤拷
    EALLOW;
    #ifdef TARGET_GS32
    Interrupt_register(INT_SCIA_RX, &SCI_RXD_isr);
    Interrupt_register(INT_SCIA_TX, &SCI_TXD_isr);
    #else
    PieVectTable.SCIRXINTA = SCI_RXD_isr;
	PieVectTable.SCITXINTA = SCI_TXD_isr;
    #endif
    EDIS;
	// IER |= M_INT9;   	            //  Enable interrupts:
    #ifdef TARGET_GS32
    interrupt_enable(INT_SCIA_RX);
    interrupt_enable(INT_SCIA_TX);
    #else
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
    #endif
 #endif   
	
}
#endif  // #if F_DEBUG_RAM

#else   // #if DEBUG_F_MODBUS
void InitSetScia(void){}
void InitSciaGpio(void){}
void SciDeal(void){}
void UpdateSciFormat(void){}
interrupt void SCI_RXD_isr(void){}
interrupt void SCI_TXD_isr(void){}
Uint16 CommWrite(Uint16 addr, Uint16 data){}
Uint16 CommRead(Uint16 addr, Uint16 data){}
#endif










