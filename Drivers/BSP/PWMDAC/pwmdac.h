/**
 ****************************************************************************************************
 * @file        pwmdac.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-11
 * @brief       PWM DAC��� ��������
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
 * V1.0 20220111
 * ��һ�η���
 *
 ****************************************************************************************************
 */


#ifndef __PWMDAC_H
#define __PWMDAC_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* PWM DAC ���� �� ��ʱ�� ���� */

/* PWMDAC Ĭ����ʹ�� PA8, ��Ӧ�Ķ�ʱ��Ϊ TIM1_CH1, �����Ҫ�޸ĳ�����IO���, ����Ӧ
 * �Ķ�ʱ����ͨ��ҲҪ�����޸�. �����ʵ����������޸�.
 */
#define PWMDAC_GPIO_PORT                    GPIOB
#define PWMDAC_GPIO_PIN                     SYS_GPIO_PIN11
#define PWMDAC_GPIO_AF                      1                                       /* AF����ѡ�� */
#define PWMDAC_GPIO_CLK_ENABLE()            do{ RCC->AHB1ENR |= 1 << 0; }while(0)   /* PA��ʱ��ʹ�� */

#define PWMDAC_TIMX                         TIM2
#define PWMDAC_TIMX_CHY                     4                           /* ͨ��Y,  1<= Y <=4 */
#define PWMDAC_TIMX_CCRX                    PWMDAC_TIMX->CCR4           /* ͨ��Y������ȽϼĴ��� */
#define PWMDAC_TIMX_CLK_ENABLE()            do{ RCC->APB1ENR |= 1 << 0; }while(0)   /* TIM2 ʱ��ʹ�� */

/******************************************************************************************/

void pwmdac_init(uint16_t arr, uint16_t psc);   /* PWM DAC��ʼ�� */
void pwmdac_set_voltage(uint16_t vol);          /* PWM DAC���������ѹ */
void close_door(void);
void open_door(void);
#endif

















