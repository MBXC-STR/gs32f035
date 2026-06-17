/*
 * DSP_GS32_Device.h
 *
 *  Created on: 2026 Jun 16
 *      Author: Jianrong Lin
 */

#ifndef DSP_GS32_DEVICE_H_
#define DSP_GS32_DEVICE_H_

#include "device.h"

#define                StartCpuTimer1()                CPUTimer_startTimer(CPUTIMER1_BASE)
#define                GetTime()                	   CPUTimer_getTimerCount(CPUTIMER1_BASE)

#endif /* DSP_GS32_DEVICE_H_ */
