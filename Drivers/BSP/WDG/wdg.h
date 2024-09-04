/**
 ****************************************************************************************************
 * @file        wdg.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2021-12-30
 * @brief       ���Ź� ��������
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
 * V1.0 20211230
 * ��һ�η���
 * V1.1 20211230
 * ����wwdg_init��wwdg_set_counter����
 *
 ****************************************************************************************************
 */

#ifndef __WDG_H
#define __WDG_H

#include "./SYSTEM/sys/sys.h"


void iwdg_init(uint8_t prer, uint16_t rlr);
void iwdg_feed(void);
void wwdg_init(uint8_t tr, uint8_t wr, uint8_t fprer);
void wwdg_set_counter(uint8_t cnt);

#endif




























