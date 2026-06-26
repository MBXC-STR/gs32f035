/*************** (C) COPYRIGHT 2010  Inovance Technology Co., Ltd****************
* File Name          : f_dspcan.c
* Author             : Yanyi	
* Version            : V0.0.1
* Date               : 08/09/2010
* Description        : DSP CAN总线底层驱动库
*                    : 2013/02/27
                                修改底层硬件为中断接收方式
* 修改                 1012 20130301 V3.02 修改点对多数据帧在中断内过滤，防止点对多数据溢出
                                
********************************************************************************/
//#include "DSP28x_Project.h"     							// DSP2803x Headerfile Include File	
//#include "main.h"											// 包含头文件

#include "f_funcCode.h"
#include "f_dspcan.h"
#include "f_canlink.h"



#define DEBUG_F_CAN              1


#if DEBUG_F_CAN

transmit_mailbox tran_mail_inst[TRAN_SIZE] = {0};
MCAN_TxMessage_t tran_buffer   [TRAN_SIZE] = {0};
receive_mailbox  rec_mail_inst [REC_SIZE]  = {0};
MCAN_RxMessage_t rec_buffer    [REC_SIZE]  = {0};

// 全局变量
volatile uint32_t mcan_bus_off_flag;  // MCAN Bus Off 状态标志
volatile uint32_t mailbox_flag;       // bit 0:1 = 发送/接收 标志
volatile uint32_t rec_mailbox_en;     // 接收邮箱使能位图
volatile uint32_t tran_mailbox_en;    // 发送邮箱使能位图

uint32_t swap_uint32(uint32_t vlaue); // 交换 uint32_t 字节序
void pre_init_mailbox_buffer(void);   // 预先初始化邮箱缓冲区
__interrupt void eCanRxIsr(void);                  // can接收中断

RAM_FUNC_T
uint32_t swap_uint32(uint32_t vlaue){
        return  ((vlaue & 0x000000FF) << 24) |
                        ((vlaue & 0x0000FF00) << 8)  |
                        ((vlaue & 0x00FF0000) >> 8)  |
                        ((vlaue & 0xFF0000FF) >> 24) ;
}

void pre_init_mailbox_buffer(void){
        for(uint32_t i = 0; i < TRAN_SIZE; i++){
                tran_mail_inst[i].mailbox_id = TRAN_BOX_N - i;
                tran_mail_inst[i].inst = &tran_buffer[i];
        }

        for(uint32_t i = 0; i < REC_SIZE; i++){
                rec_mail_inst [i].mailbox_id = REC_BOX_N - i;
                rec_mail_inst [i].rec_flag = 0;
                rec_mail_inst [i].inst = &rec_buffer[i];
        }
}

// MCAN 消息 RAM 配置 (共 4096 字节)
/******************************* START DEFINES ***********************************/
#define M_CAN_MSG_MAX_LENGTH                MCAN_ELEM_SIZE_8BYTES
#define M_CAN_MSG_BUFFER_SIZE                (MCAN_getMsgObjSize(M_CAN_MSG_MAX_LENGTH) + 12U)

#define M_CAN_RAM_BASE_ADDR                                0U
#define M_CAN_STANDARD_FILTER_BASE_ADDR        M_CAN_RAM_BASE_ADDR
#define M_CAN_STANDARD_FILTER_NUM                (4U)
#define M_CAN_STANDARD_FILTER_SIZE                (4U)

/* Note: The size of all memory regions cannot exceed 4096 bytes. */
#define M_CAN_EXTENDED_FILTER_BASE_ADDR        (M_CAN_STANDARD_FILTER_BASE_ADDR + \
                                                (M_CAN_STANDARD_FILTER_NUM * \
                                                 M_CAN_STANDARD_FILTER_SIZE))
#define M_CAN_EXTENDED_FILTER_NUM                (1U)
#define M_CAN_EXTENDED_FILTER_SIZE                (8U)

#define M_CAN_RXFIFO0_BASE_ADDR                    (M_CAN_EXTENDED_FILTER_BASE_ADDR + \
                                                    (M_CAN_EXTENDED_FILTER_NUM * \
                                                     M_CAN_EXTENDED_FILTER_SIZE))
#define M_CAN_RXFIFO0_NUM                            (8U)
#define M_CAN_RXFIFO0_SIZE                           (M_CAN_MSG_BUFFER_SIZE)

#define M_CAN_RXFIFO1_BASE_ADDR                    (M_CAN_RXFIFO0_BASE_ADDR + \
                                                    (M_CAN_RXFIFO0_NUM * \
                                                     M_CAN_RXFIFO0_SIZE))
#define M_CAN_RXFIFO1_NUM                                (8U)
#define M_CAN_RXFIFO1_SIZE                                (M_CAN_MSG_BUFFER_SIZE)

#define M_CAN_RXBUFFER_BASE_ADDR                (M_CAN_RXFIFO1_BASE_ADDR + \
                                                (M_CAN_RXFIFO1_NUM * \
                                                 M_CAN_RXFIFO1_SIZE))
#define M_CAN_RXBUFFER_NUM                                (8U)
#define M_CAN_RXBUFFER_SIZE                                (M_CAN_MSG_BUFFER_SIZE)

#define M_CAN_TXEVENTFIFO_BASE_ADDR                (M_CAN_RXBUFFER_BASE_ADDR + \
                                                    (M_CAN_RXBUFFER_NUM * \
                                                     M_CAN_RXBUFFER_SIZE))
#define M_CAN_TXEVENTFIFO_NUM                        (10U)
#define M_CAN_TXEVENTFIFO_SIZE                        (M_CAN_TX_EVENT_FIFO_SIZE)

#define M_CAN_TXBUFFER_BASE_ADDR                (M_CAN_TXEVENTFIFO_BASE_ADDR + \
                                                    (M_CAN_TXEVENTFIFO_NUM * \
                                                     M_CAN_TXEVENTFIFO_SIZE))
#define M_CAN_TXBUFFER_NUM                                (4U)
#define M_CAN_TXBUFFER_SIZE                                (M_CAN_MSG_BUFFER_SIZE)

#define M_CAN_TXFIFOQUEUE_BASE_ADDR                (M_CAN_TXBUFFER_BASE_ADDR + \
                                                    (M_CAN_TXBUFFER_NUM * \
                                                     M_CAN_TXBUFFER_SIZE))
#define M_CAN_TXFIFOQUEUE_NUM                        (6U)
#define M_CAN_TXFIFOQUEUE_SIZE                        (M_CAN_MSG_BUFFER_SIZE)
// 总计: 784 字节 / 4096 字节

const	CAN_BAUD	eCanBaud[CAN_BAUD_SUM] = {
										// GS32: CAN 模块时钟固定 40MHz (由 SysCtl_setCanClkDiv 配置)const CAN_BAUD eCanBaud[CAN_BAUD_SUM] = {
											{50,  7, 30},   // 20Kbps  (40MHz/50/40 = 20K)
											{20,  7, 30},   // 50Kbps  (40MHz/20/40 = 50K)
											{10,  7, 30},   // 100Kbps (40MHz/10/40 = 100K)
											{8,   7, 30},   // 125Kbps (40MHz/8/40  = 125K)
											{4,   7, 30},   // 250Kbps (40MHz/4/40  = 250K)
											{2,   7, 30},   // 500Kbps (40MHz/2/40  = 500K)
											{1,   7, 30},   // 1Mbps   (40MHz/1/40  = 1M)
									};
				// 公式: 40MHz / BRP / (TSEG1+TSEG2+3)// NBRP = BRP-1 = 49, 19, 9, 7, 3, 1, 0// TSEG1=30, TSEG2=7 → SyncSeg(1) + TSEG1(30) + TSEG2(7) = 38 TQ → 采样点约 81.6%

Uint32 eCanTranEnFlag;// = 0;
Uint32 eCanReEnFlag;// = 0;

/*******************************************************************************
* 函数名称          : interrupt void eCanRxIsr(void)
* 入口参数                        : 无
* 出口                                ：
* 创建                    :
* 版本                        : V0.0.1
* 时间              : 01/30/2013
* 说明                                : eCan中断方式接收数据到缓存
********************************************************************************/
Uint32 recCanCout;
volatile DspCanDataStru temp_buf;
RAM_FUNC_T
__interrupt void eCanRxIsr(void)
{
        SAVE_IRQ_CSR_CONTEXT();
        uint16_t i, dataTy;
    DspCanDataStru *data;

        uint32_t interrupt_status;
        MCAN_RxMessage_t RxMessageBuf;
        MCAN_RxNewDataStatus_t new_dat;
        MCAN_RxFIFOStatus_t fifoStatus;
        MCAN_ErrCntStatus_t errCounter;
        MCAN_ProtocolStatus_t protocolStatus;

        interrupt_status = MCAN_getIntrStatus(CAN_BASE);

    for (i=0; i<REC_MBOX_NUM-1; i++)
    {
        if ( (CanlinkRecBuf.bufFull & (1<<i) ) == 0)        // 判断当前缓存空，跳出
            break;
    }

    recCanCout++;
    data = (DspCanDataStru *)(& (CanlinkRecBuf.buf[i]) );

        if (interrupt_status & MCAN_INT_SRC_RxFIFO0_NEW_MSG) {
                MCAN_receiveMsgFromFifo0(CAN_BASE, &RxMessageBuf);

                fifoStatus.num = MCAN_RX_FIFO_NUM_0;
                MCAN_getRxFIFOStatus(CAN_BASE, &fifoStatus);

                if (fifoStatus.fillLvl == 0)
                        MCAN_clearIntrStatus(CAN_BASE, MCAN_INT_SRC_RxFIFO0_NEW_MSG);
        }

        if (interrupt_status & MCAN_INT_SRC_BUS_OFF_STATUS) {
                MCAN_getErrCounters(CAN_BASE, &errCounter);
                MCAN_getProtocolStatus(CAN_BASE, &protocolStatus);
                mcan_bus_off_flag = 1;
                MCAN_clearIntrStatus(CAN_BASE, MCAN_INT_SRC_BUS_OFF_STATUS);
        }

        // 从MCAN 获取到数据 封装到 user 结构体
        temp_buf.len =          RxMessageBuf.dlc;
        temp_buf.msgid = RxMessageBuf.id;
        memcpy(&temp_buf.data.mdl, &RxMessageBuf.data[0], 4);
        memcpy(&temp_buf.data.mdh, &RxMessageBuf.data[4], 4);

    data->msgid = temp_buf.msgid;                                   //  读ID，读数据
    data->data.mdl = swap_uint32(temp_buf.data.mdl);
    data->data.mdh = swap_uint32(temp_buf.data.mdh);
    data->len= temp_buf.len & 0xf;                                 // 读取接收数据长度

    dataTy = P2bFilte(data->msgid);
    if (dataTy)                                                     // 收到点对多数据
    {
        if (dataTy == 0xcc)
            CanlinkRecBuf.bufFull |= (1<<i);                        // 置接收缓存
    }
    else
        CanlinkRecBuf.bufFull |= (1<<i);                            // 置接收缓存

    RESTORE_IRQ_CSR_CONTEXT();
}
/*******************************************************************************
* 函数名称          : void DisableDspCAN(void)
* 入口参数			: 无
* 出口				：
* 创建	            :
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: 禁止DSP Ecan接口
********************************************************************************/
void DisableDspCAN(void)
{
	SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_CAN);
}

/*******************************************************************************
* 函数名称          : Uint16 InitdspECan(Uint16 baud)
* 入口参数                        : CAN接口波特率编号 (eCanBaud索引)
* 出口                                ：CAN_INIT_TIME         初始化进行中
*                                          CAN_INIT_SUCC  初始化成功
*                                          CAN_INIT_TIMEOUT 初始化超时
*                                          CAN_INIT_BAUD_ERR 波特率出错
* 创建                    :
* 版本                        : V0.0.1
* 时间              : 07/29/2010
* 说明                                : 初始化DSP Ecan接口
********************************************************************************/
#define                IINIT_CAN_TIME                                3
Uint16 InitdspECan(Uint16 baud)                // Initialize eCAN-A module
{
    EALLOW;
    static        Uint16 con = 0;                                                                        // 步骤标识
    static        Uint16 count = 0;                                                                // 超时计数器

    MCAN_InitParams_t                    init_param    = { 0 };
    MCAN_ConfigParams_t                  configParams  = { 0 };
    MCAN_MsgRAMConfigParams_t            RAMConfig     = { 0 };
    MCAN_BitTimingParams_t               configBitrate = { 0 };
    MCAN_StdMsgIDFilterElement_t         std_filter    = { 0 };
    MCAN_ExtMsgIDFilterElement_t         ext_filter    = { 0 };

    if (baud >= CAN_BAUD_SUM)                                                                // 波特率出错
        return CAN_INIT_BAUD_ERR;

    if (count > IINIT_CAN_TIME)                                                                // 初始化超时出错
    {
        con   = 0;
        count = 0;
        return  CAN_INIT_TIMEOUT;
    }

    if (con == 0)
    {        // 初始化CAN引脚复用
        GPIO_setPinConfig(CAN_TX_PIN);
        GPIO_setPinConfig(CAN_RX_PIN);
        con = 1;                                                                                        // 第一阶段完成
    }

    if (con == 1)
    {  // 初始化CAN外设参数
        /* Select the MCAN clock source. */
        SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_CAN);
        SysCtl_setCanClkSrcSel(CAN_CLK_TYPE_PLL);
        SysCtl_setCanClkDiv(DEVICE_PLLCLK_FREQ / 40000000U);
        SysCtl_resetCana();
/**************************** init_param ****************************/
        /* CAN mode configuration. */
        init_param.fdMode = false;
        init_param.fdFormat = MCAN_FD_ISO_11898_1;
        init_param.brsEnable = false;
        init_param.txpEnable = false;
        init_param.efbi = false;
        init_param.pxhddisable = false;
        /* Enable the auto retransmission. */
        init_param.darEnable = true;
        init_param.wkupReqEnable = false;
        init_param.autoWkupEnable = false;
        init_param.emulationEnable = false;
        init_param.wdcPreload = false;
        init_param.wmMarker = MCAN_WMM_8BIT_MODE;
        init_param.tdcEnable = false;
        init_param.tdcConfig.tdcf = 0U;
        init_param.tdcConfig.tdco = 0U;
/**************************** configParams ****************************/
        /* Set CAN extended feature. */
        configParams.asmEnable = false;
        configParams.monEnable = false;
        configParams.timeoutCntEnable = false;
        configParams.timeoutPreload = 1;
        configParams.timeoutSelect = MCAN_TIMEOUT_SELECT_CONT;

        configParams.tsClock = MCAN_INTERNAL_TIMESTAMP;
        configParams.tsSelect = MCAN_TSCNTVAL_ALWAYS0;
        configParams.tsPrescalar = 1;

        /* Set global configuration. */
        configParams.filterConfig.rrfs = true;
        configParams.filterConfig.rrfe = true;
        configParams.filterConfig.anfs = MCAN_NON_MATCH_REJECT_ACCEPT;
        configParams.filterConfig.anfe = MCAN_NON_MATCH_REJECT_ACCEPT;
/**************************** RAMConfig ****************************/
        /* Set standard ID filter. */
        RAMConfig.lss = M_CAN_STANDARD_FILTER_NUM;
        RAMConfig.flssa = M_CAN_STANDARD_FILTER_BASE_ADDR;

        /* Set extended ID filter. */
        RAMConfig.lse = M_CAN_EXTENDED_FILTER_NUM;
        RAMConfig.flesa = M_CAN_EXTENDED_FILTER_BASE_ADDR;

        /* Set Rx fifo 0. */
        RAMConfig.rxFIFO0OpMode = MCAN_RXFIFO_OP_MODE_BLOCKING;
        RAMConfig.rxFIFO0startAddr = M_CAN_RXFIFO0_BASE_ADDR;
        RAMConfig.rxFIFO0size = M_CAN_RXFIFO0_NUM;
        RAMConfig.rxFIFO0waterMark = 0;
        RAMConfig.rxFIFO0ElemSize  = M_CAN_MSG_MAX_LENGTH;

        /* Set Rx fifo 1. */
        RAMConfig.rxFIFO1OpMode = MCAN_RXFIFO_OP_MODE_BLOCKING;
        RAMConfig.rxFIFO1startAddr = M_CAN_RXFIFO1_BASE_ADDR;
        RAMConfig.rxFIFO1size = M_CAN_RXFIFO1_NUM;
        RAMConfig.rxFIFO1waterMark = 0;
        RAMConfig.rxFIFO1ElemSize = M_CAN_MSG_MAX_LENGTH;

        /* Set Rx buffer. */
        RAMConfig.rxBufStartAddr = M_CAN_RXBUFFER_BASE_ADDR;
        RAMConfig.rxBufElemSize = M_CAN_MSG_MAX_LENGTH;

        /* Set Tx buffer */
        RAMConfig.txBufMode = MCAN_TXBUF_OP_IN_FIFO_MODE;
        RAMConfig.txStartAddr = M_CAN_TXBUFFER_BASE_ADDR;
        RAMConfig.txFIFOSize = M_CAN_TXFIFOQUEUE_NUM;
        RAMConfig.txBufNum = M_CAN_TXBUFFER_NUM;
        RAMConfig.txBufElemSize = M_CAN_MSG_MAX_LENGTH;

        /* Set Tx event fifo. */
        RAMConfig.txEventFIFOStartAddr = M_CAN_TXEVENTFIFO_BASE_ADDR;
        RAMConfig.txEventFIFOSize = M_CAN_TXEVENTFIFO_NUM;
        RAMConfig.txEventFIFOWaterMark = 1;
        con = 2;
    }

    if (con == 2)
    {
        // 初始化波特率
        configBitrate.nomRatePrescalar =  (eCanBaud[baud].BRPREG - 1) < 0 ? 1 : (eCanBaud[baud].BRPREG - 1);
        configBitrate.nomTimeSeg1 =           eCanBaud[baud].TSEG1REG;
        configBitrate.nomTimeSeg2 =           eCanBaud[baud].TSEG2REG;
        configBitrate.nomSynchJumpWidth = 2;
        con = 3;
    }
    if (con == 3)
    {
        /* Reset the MCAN module. */
        while (MCAN_isInReset(CAN_BASE));

        /* Check if the MCAN RAM is ready. */
        while (!MCAN_isMemInitDone(CAN_BASE));

        /* Set the MCAN mode to init mode. */
        if (!MCAN_setOpMode(CAN_BASE, MCAN_OPERATION_SW_INIT_MODE))
                while (1);

        /* Initialize the MCAN. */
        if (!MCAN_init(CAN_BASE, &init_param))
                while (1);

        if (!MCAN_config(CAN_BASE, &configParams))
                while (1);

        if (!MCAN_setBitTime(CAN_BASE, &configBitrate))
                while (1);

        if (!MCAN_msgRAMConfig(CAN_BASE, &RAMConfig))
                while (1);
/***************************** standard ID filter *****************************/
        /* Add a new standard ID filter(Use Rx buffer 0). */
#if 0
        std_filter.sfid1 = 0x123;
        std_filter.sfid2 = 0x0;
        std_filter.ssync = MCAN_STDIDF_SYNC_MSG_DISABLE;
        std_filter.sfec = MCAN_STDIDF_ELE_STORE_IN_RXB_OR_DMSG;
        std_filter.sft = MCAN_STDIDF_RANGE_FROM_SFID1_TO_SFID2;
        if (!MCAN_addStdMsgIDFilter(CAN_BASE, 0, &std_filter))
                while (1);
#endif

/***************************** extended ID filter *****************************/
        /* Add a new extended ID filter(Use Rx fifo 0). */
        ext_filter.efid1 = 0x0;
        ext_filter.efid2 = 0x1fffff;
        ext_filter.esync = MCAN_EXTIDF_SYNC_MSG_DISABLE;
        ext_filter.efec = MCAN_EXTIDF_ELE_PRIO_STORE_IN_FO0_OF_MATCH_ID;
        ext_filter.eft = MCAN_EXTIDF_RANGE_FROM_EFID1_TO_EFID2;
        if (!MCAN_addExtMsgIDFilter(CAN_BASE, 0, &ext_filter))
                while (1);

        /* The received extension frame will be subjected to AND operation with Ext_mask. */
        if (!MCAN_setExtIDAndMask(CAN_BASE, 0x1FFFFF))
                while (1);

        /* Enable/Disable Loopback mode. */
        if (!MCAN_lpbkModeEnable(CAN_BASE, MCAN_LPBK_MODE_INTERNAL, false))
                while (1);

        /* Enable MCAN. */
        if (!MCAN_setOpMode(CAN_BASE, MCAN_OPERATION_NORMAL_MODE))
                while (1);

         /* Configuration the external timestamp clock source. */
         if (!MCAN_extTSCounterConfig(CAN_BASE, 0xffff))
                 while (1);

         /* Enable/Disable external timestamp clock source. */
         if (!MCAN_extTSCounterEnable(CAN_BASE, true))
                 while (1);

         /* Enable/Disable external timestamp overflow interrupt. */
         if (!MCAN_extTSEnableIntr(CAN_BASE, false))
                 while (1);

        /* Select MCAN interrupt route to interrupt line 0 or 1. */
        if (!MCAN_selectIntrLine(CAN_BASE, MCAN_INT_SRC_MESSAGE_STORED_TO_RXBUF |
                                                                           MCAN_INT_SRC_RxFIFO0_NEW_MSG,
                                                                           MCAN_INTERRUPT_LINE_0))
                while (1);

        /* Enable MCAN interrupt. */
        if (!MCAN_enableIntr(CAN_BASE, MCAN_INT_SRC_MESSAGE_STORED_TO_RXBUF |
                                                                   MCAN_INT_SRC_RxFIFO0_NEW_MSG, true))
                while (1);

        if (!MCAN_enableIntrLine(CAN_BASE, MCAN_INTERRUPT_LINE_0, true))
                while (1);

        Interrupt_register(CAN_IRQ_LINE0, eCanRxIsr);
        Interrupt_enable(CAN_IRQ_LINE0);
    }

    pre_init_mailbox_buffer();

/* Disable all Mailboxes  */
    con = 0;
    count = 0;
    EDIS;

    eCanTranEnFlag = 0;                                     // 清空邮箱初始化标志
    eCanReEnFlag = 0;

    return CAN_INIT_SUCC;                                                                        // 初始化成功
}
/*******************************************************************************
* 函数名称          : void ErroCountReset(void)
* 入口参数                        : 无
* 出口                                ：0  CAN控制正常状态
*                     1  CAN控制器错误复位中
* 创建                    :
* 版本                        : V0.0.1
* 时间              : 05/16/2012
* 说明                                : 手动复位CAN总线错误计数器
********************************************************************************/
Uint16 ErroCountReset(void)
{
    static Uint16 stat = 0;
        if (mcan_bus_off_flag) {
                mcan_bus_off_flag = 0;
                MCAN_setOpMode(CAN_BASE, MCAN_OPERATION_NORMAL_MODE);
        }
    return (stat);
}


/*******************************************************************************
* 函数名称          : void InitTranMbox(Uint16 mbox)
* 入口参数                        : mbox      邮箱编号 0~31，
*                                          *datapi   邮箱初始化数据结构
*                     msgid     bit0~bit28  29位帧ID
*                               bit31       扩展帧标识
* 出口                                ：无
* 创建                    :
* 版本                        : V0.0.1
* 时间              : 07/29/2010
* 说明                                : 初化CAN发送邮箱，可初始化为自动应答邮箱
********************************************************************************/
void InitTranMbox(Uint16 mbox, DspCanDataStru *datapi)//Uint32 msgid, Uint32 *dataPi)
{
        msg_pack user_msg;
        user_msg.all  = datapi->msgid;

        Uint16 mbox_id = mbox & 0x1f;

        eCanTranEnFlag  |= 1ul << mbox;                                                        // 邮箱初始化发送标志
        mailbox_flag    &= ~(1ul<<mbox_id);                                          // 清零设置为发送邮箱
        tran_mailbox_en |= 1ul<<mbox_id;                                                // 使能对应邮箱

        for(uint8_t i = 0; i < TRAN_SIZE; i++){
                if(mbox_id == tran_mail_inst[i].mailbox_id){
                        tran_mail_inst[i].inst->brs  = 0;
                        tran_mail_inst[i].inst->dlc  = MCAN_DATA_LENGTH_8;
                        tran_mail_inst[i].inst->efc  = 0;
                        tran_mail_inst[i].inst->esi  = 0;
                        tran_mail_inst[i].inst->fdf  = 0;
                        tran_mail_inst[i].inst->id   = M_CAN_EXTENDED_ID_W(user_msg.all);
                        tran_mail_inst[i].inst->mm   = 0x0;
                        tran_mail_inst[i].inst->mm1  = 0x0;
                        tran_mail_inst[i].inst->rtr  = 0;
                        tran_mail_inst[i].inst->tsce = 0;
                        tran_mail_inst[i].inst->xtd  = 1;
                }
        }
}
/*******************************************************************************
* 函数名称          : void InitReMbox(Uint16 mbox, union CANMSGID_REG msgid, union CANLAM_REG lam)
* 入口参数			: mbox 邮箱编号 0~31，bit7 “1” 接收远程帧 “0”普通帧	    bit6 "1"覆盖保护(不允许覆盖)
*					  msgid	消息标识ID
*					  lam	接收屏蔽寄存器，写"0"参与验收过滤位
* 出口				：无
* 创建	            :
* 版本		        : V0.0.1
* 时间              : 07/29/2010
* 说明				: 初化CAN接收邮箱
********************************************************************************/
void InitRecMbox(Uint16 mbox, Uint32 msgid, Uint32 lam)
{
	
}


/*******************************************************************************
* 函数名称: Uint16 eCanDataTran(Uint16 mbox, Uint16 len, Uint32 msgid, Uint32 *dataPi)
* 入口参数 : mbox      邮箱编号 0~31，
*           len       发送数据长度字节数
*            msgid     消息标识ID           只包含有效ID位
*        dataPi    数据缓冲区
* 出口：CAN_MBOX_NUM_ERROR                邮箱号出错，该邮箱未被初始化为发送邮箱
*      CAN_MBOX_BUSY                                邮箱忙
*      CAN_MBOX_TRAN_SUCC                发送成功
* 创建 :
* 版本 : V0.0.1
* 时间 : 08/25/2010
* 说明   : 指定邮箱发送数据，邮箱必须被初始化为发送邮箱
********************************************************************************/
uint32_t mailbox_busy = 0;
RAM_FUNC_T
Uint16 eCanDataTran(Uint16 mbox, Uint16 len, DspCanDataStru *data)
{
    Uint32 msgid;
        uint8_t DATA[8] = {0x0};
        uint16_t mbox_id =  mbox & 0x1f;

        if ( (eCanTranEnFlag & (1ul << mbox)) != (1ul << mbox) )
        {
                return (CAN_MBOX_NUM_ERROR);                                                // CAN邮箱号出错，邮箱未初始化
        }

        // 获取到处于忙的状态邮箱标识
        mailbox_busy = MCAN_getTxBufReqPend(CAN_BASE);

    msgid = data->msgid;
        mbox &= 0x1f;


        // 选择对应的发送邮箱
        for(uint8_t mailbox_i = 0; mailbox_i < TRAN_SIZE; mailbox_i++){
            if(mbox_id == tran_mail_inst[mailbox_i].mailbox_id){
                // 判断当前邮箱是否在忙
                if( mailbox_busy & (0x01 << mailbox_i)){
                        return (CAN_MBOX_BUSY);
                }
                // 写id
                tran_mail_inst[mailbox_i].inst->id  = M_CAN_EXTENDED_ID_W(msgid);
                // 写长度
                tran_mail_inst[mailbox_i].inst->dlc = len;
                // 数据字节序转换
                volatile uint32_t temp;
                temp =  swap_uint32(data->data.mdl);
                memcpy(&DATA,    &temp,  4);
                temp =  swap_uint32(data->data.mdh);
//                        temp = mbox;
                memcpy(&DATA[4], &temp,  4);
                // 填充数据
                for(uint32_t data_i = 0; data_i < len; data_i++){
                        tran_mail_inst[mailbox_i].inst->data[data_i] = DATA[data_i];
                }
                tran_mail_inst[mailbox_i].send_couter++;
                // 使能发送
                MCAN_transmitMsgBuffer(CAN_BASE, tran_mail_inst[mailbox_i].inst, mailbox_i);                // 使能发送
            }
        }

        return (CAN_MBOX_TRAN_SUCC);
}
/*******************************************************************************
* 函数名称          : Uint16 eCanDataRec(Uint16 mbox, Uint32 *dataPi)
* 入口参数			: mbox      邮箱编号 0~31，
*					  *	data    接收缓存
* 出口				：CAN_MBOX_NUM_ERROR		邮箱号出错，该邮箱未被初始化为发送邮箱
*					  CAN_MBOX_EMPTY			接收邮箱空
*					  CAN_MBOX_REC_SUCC			接收数据成功
*					  CAN_MBOX_REC_OVER			接收数据有覆盖
* 创建	            :
* 版本		        : V0.0.1
* 时间              : 08/25/2010
* 说明				: 接收数据接收缓存区
********************************************************************************/
/*
Uint16 eCanDataRec(Uint16 mbox, DspCanDataStru *data)
{
	Uint32 *pi;
	
	mbox &= 0x1f;
//	if ( (eCanReEnFlag & (1ul << mbox)) != (1ul << mbox))
//	{
//		return (CAN_MBOX_NUM_ERROR);						// CAN邮箱号出错，邮箱未初始化
//	}
	if (ECANREGS.CANRMP.all & (1ul << mbox) )				// 检查是否有接收消息挂起
	{
		pi = (Uint32 *)(&ECANMBOXES.MBOX0.MSGID);			
		data->msgid = pi[mbox<<2];							//  读ID，读数据
		data->data.mdl= pi[(mbox<<2) + 2];	
		data->data.mdh = pi[(mbox<<2) + 3];
        data->len= pi[(mbox<<2) + 1] & 0xf;                 // 读取接收数据长度

//		ECanaShadow = 1ul<<mbox;
		
		if (ECANREGS.CANRML.all & (1ul << mbox))			// 检查邮箱是否被覆盖过
		{
			ECANREGS.CANRMP.all = 1ul<<mbox;				// 清除消息挂起寄存器
			return (CAN_MBOX_REC_OVER);
		}	
		else
		{
			ECANREGS.CANRMP.all = 1ul<<mbox;				// 清除消息挂起寄存器
			return (CAN_MBOX_REC_SUCC);		
		}
	}
	else
	{
		return (CAN_MBOX_EMPTY);							// CAN邮箱空，无可读取数据
	}
}
*/

/*******************************************************************************
* 函数名称          : Uint16 CanMailBoxEmp()
* 入口参数                        :
* 出口                                : 1  有空闲
*                   : 0  无空闲
* 创建                    :
* 版本                        : V0.0.1
* 时间              : 08/25/2010
* 说明                                : CAN 发送邮箱空检测
********************************************************************************/
#define TX_MAILBOX_MAST (( (1ul << TRAN_MBOX_NUM) - 1) << TRAN_BOX_N)
Uint16 CanMailBoxEmp(void)
{
        mailbox_busy = MCAN_getTxBufReqPend(CAN_BASE);
        // 判断所有发送邮箱
        for(uint8_t mailbox_i = 0; mailbox_i < TRAN_SIZE; mailbox_i++){
                if( mailbox_busy & (0x01 << mailbox_i)){
                        continue;
                }else{
                        return 1;
                }
        }
         return 0;                                           // 所有邮箱忙
}



#elif 1



#endif




