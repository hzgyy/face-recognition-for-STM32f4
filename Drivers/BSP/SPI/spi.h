/**
 ****************************************************************************************************
 * @file        spi.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-11
 * @brief       SPI ��������
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

#ifndef __SPI_H
#define __SPI_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* SPI1 ���� ���� */

#define SPI1_SCK_GPIO_PORT              GPIOB
#define SPI1_SCK_GPIO_PIN               SYS_GPIO_PIN3
#define SPI1_SCK_GPIO_AF                5
#define SPI1_SCK_GPIO_CLK_ENABLE()      do{ RCC->AHB1ENR |= 1 << 1; }while(0)   /* PB��ʱ��ʹ�� */

#define SPI1_MISO_GPIO_PORT             GPIOB
#define SPI1_MISO_GPIO_PIN              SYS_GPIO_PIN4
#define SPI1_MISO_GPIO_AF               5
#define SPI1_MISO_GPIO_CLK_ENABLE()     do{ RCC->AHB1ENR |= 1 << 1; }while(0)   /* PB��ʱ��ʹ�� */

#define SPI1_MOSI_GPIO_PORT             GPIOB
#define SPI1_MOSI_GPIO_PIN              SYS_GPIO_PIN5
#define SPI1_MOSI_GPIO_AF               5
#define SPI1_MOSI_GPIO_CLK_ENABLE()     do{ RCC->AHB1ENR |= 1 << 1; }while(0)   /* PB��ʱ��ʹ�� */

/* SPI1��ض��� */
#define SPI1_SPI                        SPI1
#define SPI1_SPI_CLK_ENABLE()           do{ RCC->APB2ENR |= 1 << 12; }while(0)  /* SPI1ʱ��ʹ�� */

/******************************************************************************************/


/* SPI�����ٶ����� */
#define SPI_SPEED_2         0
#define SPI_SPEED_4         1
#define SPI_SPEED_8         2
#define SPI_SPEED_16        3
#define SPI_SPEED_32        4
#define SPI_SPEED_64        5
#define SPI_SPEED_128       6
#define SPI_SPEED_256       7


void spi1_init(void);
void spi1_set_speed(uint8_t speed);
uint8_t spi1_read_write_byte(uint8_t txdata);

#endif
























