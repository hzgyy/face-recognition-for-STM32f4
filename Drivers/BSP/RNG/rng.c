/**
 ****************************************************************************************************
 * @file        rng.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-09
 * @brief       RNG(�����������) ��������
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
 * V1.0 20220109
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "./BSP/RNG/rng.h"
#include "./SYSTEM/delay/delay.h"

/**
 * @brief       ��ʼ��RNG
 * @param       ��
 * @retval      0,�ɹ�;1,ʧ��
 */
uint8_t rng_init(void)
{
    uint16_t retry = 0;
    RCC->AHB2ENR = 1 << 6;      /* ����RNGʱ�� */
    RNG->CR |= 1 << 2;          /* ʹ��RNG */

    while ((RNG->SR & 0X01) == 0 && retry < 10000)  /* �ȴ���������� */
    {
        retry++;
        delay_us(100);
    }

    if (retry >= 10000)return 1;    /* ��������������������� */

    return 0;
}

/**
 * @brief       �õ������
 * @param       ��
 * @retval      ��ȡ���������(32bit)
 */
uint32_t rng_get_random_num(void)
{
    while ((RNG->SR & 0X01) == 0);  /* �ȴ���������� */

    return RNG->DR;
}

/**
 * @brief       �õ�ĳ����Χ�ڵ������
 * @param       min,max: ��С,���ֵ.
 * @retval      �õ��������(rval),����:min<=rval<=max
 */
int rng_get_random_range(int min, int max)
{
    return rng_get_random_num() % (max - min + 1) + min;
}
















