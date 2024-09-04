/**
 ****************************************************************************************************
 * @file        usart.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-13
 * @brief       ���ڳ�ʼ������(����2)
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F407������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20220113
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef _USART2_H
#define _USART2_H

#include "stdio.h"
#include "./SYSTEM/sys/sys.h"

/*******************************************************************************************************/
/* ���� �� ���� ���� 
 * Ĭ�������USART2��.
 */
 
#define USART2_TX_GPIO_PORT                 GPIOA
#define USART2_TX_GPIO_PIN                  SYS_GPIO_PIN2
#define USART2_TX_GPIO_AF                   7                                           /* AF����ѡ�� */
#define USART2_TX_GPIO_CLK_ENABLE()         do{ RCC->AHB1ENR |= 1 << 1; }while(0)       /* PA��ʱ��ʹ�� */

#define USART2_RX_GPIO_PORT                 GPIOA
#define USART2_RX_GPIO_PIN                  SYS_GPIO_PIN3
#define USART2_RX_GPIO_AF                   7                                           /* AF����ѡ�� */
#define USART2_RX_GPIO_CLK_ENABLE()         do{ RCC->AHB1ENR |= 1 << 0; }while(0)       /* PA��ʱ��ʹ�� */

#define USART2_UX                           USART2
#define USART2_UX_CLK_ENABLE()              do{ RCC->APB1ENR |= 1 << 17; }while(0)      /* USART2 ʱ��ʹ�� */

/*******************************************************************************************************/
#define USART2_REC_LEN               200             /* �����������ֽ��� 200 */
#define USART2_EN_RX                 1               /* ʹ�ܣ�1��/��ֹ��0������1���� */


extern uint8_t  g_usart2_rx_buf[USART2_REC_LEN];      /* ���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� */
extern uint16_t g_usart2_rx_sta;                     /* ����״̬��� */


void usart2_init(uint32_t sclk, uint32_t baudrate); /* ����2��ʼ������ */
void USART2_SendString(uint8_t *DAT, uint8_t len);
#endif







