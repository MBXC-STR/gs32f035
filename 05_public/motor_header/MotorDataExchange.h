/***************************************************************
文件功能:
文件版本：VERSION 1.0
最新更新：2007.09.27
************************************************************/
#ifndef MOTOR_DATA_EXCHANGE_H
#define MOTOR_DATA_EXCHANGE_H

#ifdef __cplusplus
extern "C" {
#endif


/***********************外部函数的声明********************************/
/*********************************************************************/
extern void ParSend05Ms(void);
extern void ParGet05Ms(void);
extern void ParSend2Ms(void);
extern void ParGet2Ms(void);
extern void ParSendTune(void);
extern void ParSendTuneBeforeRun(void);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif  // end of definition

//===========================================================================
// End of file.
//===========================================================================
