/**
 ****************************************************************************************************
 * @file        usart.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-13
 * @brief       串口初始化代码(串口2)
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20220113
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef _USART2_H
#define _USART2_H

#include "stdio.h"
#include "./SYSTEM/sys/sys.h"

/*******************************************************************************************************/
/* 引脚 和 串口 定义 
 * 默认是针对USART2的.
 */
 
#define USART2_TX_GPIO_PORT                 GPIOA
#define USART2_TX_GPIO_PIN                  SYS_GPIO_PIN2
#define USART2_TX_GPIO_AF                   7                                           /* AF功能选择 */
#define USART2_TX_GPIO_CLK_ENABLE()         do{ RCC->AHB1ENR |= 1 << 1; }while(0)       /* PA口时钟使能 */

#define USART2_RX_GPIO_PORT                 GPIOA
#define USART2_RX_GPIO_PIN                  SYS_GPIO_PIN3
#define USART2_RX_GPIO_AF                   7                                           /* AF功能选择 */
#define USART2_RX_GPIO_CLK_ENABLE()         do{ RCC->AHB1ENR |= 1 << 0; }while(0)       /* PA口时钟使能 */

#define USART2_UX                           USART2
#define USART2_UX_CLK_ENABLE()              do{ RCC->APB1ENR |= 1 << 17; }while(0)      /* USART2 时钟使能 */

/*******************************************************************************************************/
#define USART2_REC_LEN               200             /* 定义最大接收字节数 200 */
#define USART2_EN_RX                 1               /* 使能（1）/禁止（0）串口1接收 */


extern uint8_t  g_usart2_rx_buf[USART2_REC_LEN];      /* 接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 */
extern uint16_t g_usart2_rx_sta;                     /* 接收状态标记 */


void usart2_init(uint32_t sclk, uint32_t baudrate); /* 串口2初始化代码 */
void USART2_SendString(uint8_t *DAT, uint8_t len);
#endif







