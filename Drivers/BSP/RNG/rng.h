/**
 ****************************************************************************************************
 * @file        rng.h
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

#ifndef __RNG_H
#define __RNG_H 

#include "./SYSTEM/sys/sys.h"

uint8_t rng_init(void);             /* RNG��ʼ�� */
uint32_t rng_get_random_num(void);  /* �õ������ */
int rng_get_random_range(int min,int max);  /* �õ�����ĳ����Χ�ڵ������ */

#endif

















