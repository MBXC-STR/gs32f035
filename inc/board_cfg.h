/*
 *   Copyright (c) Gejian Semiconductors 2023
 *   All rights reserved.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


/**
*   @file    board_cfg.h
*   @brief   board level configurations
*
*/

#ifndef __BOARD_CFG_H__
#define __BOARD_CFG_H__

#ifdef __cplusplus
extern "C"{
#endif

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
  * @brief Configuration of the CRG
  */

/* 使能外部高速晶振 */
#define __HSE_ENABLE                    1 

/* 外部晶振类型: 0 无源晶振, 1 有源晶振 */
#define __HSE_CLOCK_TYPE                0

/* 设定PLL时钟源频率: 10MHz IRC或者2MHz~25MHz外部晶振 */
#if __HSE_ENABLE == 0
    #define DEVICE_OSCSRC_FREQ          (10*1000*1000U)
#else
    #define DEVICE_OSCSRC_FREQ          (20*1000*1000U)
#endif

/* 系统时钟 */
#define DEVICE_SYSCLK_FREQ              (240*1000*1000U)

/* 系统时钟分频 */
#define DEVICE_SYSCLK_DIV               (2)

/* PLL时钟 */
#define DEVICE_PLLCLK_FREQ              (DEVICE_SYSCLK_FREQ * DEVICE_SYSCLK_DIV)

/* AHB总线时钟分频 */
#define DEVICE_AHBCLK_DIV               (2)

/* AHB总线时钟等于系统时钟 */
#define DEVICE_AHBCLK_FREQ              (DEVICE_SYSCLK_FREQ/DEVICE_AHBCLK_DIV)

/* APB总线时钟分频 = 1，2，4，8 */
#define DEVICE_APBCLK_DIV               (4)

/* APB总线时钟频率 MHz */
#define DEVICE_APBCLK_FREQ              (DEVICE_SYSCLK_FREQ/DEVICE_APBCLK_DIV)

/* 定时器时钟频率等于APB总线时钟频率 */
#define SOC_TIMER_FREQ                  (DEVICE_APBCLK_FREQ)

/**
  * @brief Configuration of the LED pins
  */


/**
  * @brief Configuration of the SCI pins
  */



#define LOG_SCI_BASE                    SCIA_BASE

#ifdef __cplusplus
}
#endif

#endif  /* __BOARD_CFG_H__ */
