/**
 ****************************************************************************************************
 * @file        ov2640.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-13
 * @brief       OV2640 ��������
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

#include "./BSP/OV2640/sccb.h"
#include "./BSP/OV2640/ov2640.h"
#include "./BSP/OV2640/ov2640cfg.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"

/**
 * @brief       OV2640 ���Ĵ���
 * @param       reg :  �Ĵ�����ַ
 * @retval      �����ļĴ���ֵ
 */
uint8_t ov2640_read_reg(uint16_t reg)
{
    uint8_t data = 0;
    
    sccb_start();                       /* ����SCCB���� */
    sccb_send_byte(OV2640_ADDR);        /* д����ADDR */
    delay_us(100);
    sccb_send_byte(reg);                /* д�Ĵ�����ַ */
    delay_us(100);
    sccb_stop();
    delay_us(100);
    
    /* ���üĴ�����ַ�󣬲��Ƕ� */
    sccb_start();
    sccb_send_byte(OV2640_ADDR | 0X01); /* ���Ͷ����� */
    delay_us(100);
    data = sccb_read_byte();            /* ��ȡ���� */
    sccb_nack();
    sccb_stop();
    
    return data;
}

/**
 * @brief       OV2640 д�Ĵ���
 * @param       reg : �Ĵ�����ַ
 * @param       data: Ҫд��Ĵ�����ֵ
 * @retval      0, �ɹ�; 1, ʧ��;
 */
uint8_t ov2640_write_reg(uint16_t reg, uint8_t data)
{
    uint8_t res = 0;
    
    sccb_start();                           /* ����SCCB���� */
    delay_us(100);
    if (sccb_send_byte(OV2640_ADDR))res = 1;/* д����ID */
    delay_us(100);
    if (sccb_send_byte(reg))res = 1;        /* д�Ĵ�����ַ */
    delay_us(100);
    if (sccb_send_byte(data))res = 1;       /* д���� */
    delay_us(100);
    sccb_stop();
    return res;
}

/**
 * @brief       ��ʼ�� OV2640
 * @param       ��
 * @retval      0, �ɹ�; 1, ʧ��;
 */
uint8_t ov2640_init(void)
{
    uint16_t i = 0;
    uint16_t reg;
    
    OV_PWDN_GPIO_CLK_ENABLE();      /* ʹ��OV_PWDN��ʱ�� */
    OV_RESET_GPIO_CLK_ENABLE();     /* ʹ��OV_RESET��ʱ�� */

    sys_gpio_set(OV_PWDN_GPIO_PORT, OV_PWDN_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* PWDN����ģʽ����,������� */

    sys_gpio_set(OV_RESET_GPIO_PORT, OV_RESET_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* RESET����ģʽ����,������� */

    OV2640_PWDN(0);     /* POWER ON */
    delay_ms(10);
    OV2640_RST(0);      /* ����������OV5640��RST��,���ϵ� */
    delay_ms(20);
    OV2640_RST(1);      /* ������λ */
    delay_ms(20);
    
    sccb_init();        /* ��ʼ��SCCB ��IO�� */
    delay_ms(5);
    ov2640_write_reg(OV2640_DSP_RA_DLMT, 0x01);     /* ����sensor�Ĵ��� */
    ov2640_write_reg(OV2640_SENSOR_COM7, 0x80);     /* ��λOV2640 */
    delay_ms(50);
    reg = ov2640_read_reg(OV2640_SENSOR_MIDH);      /* ��ȡ����ID �߰�λ */
    reg <<= 8;
    reg |= ov2640_read_reg(OV2640_SENSOR_MIDL);     /* ��ȡ����ID �Ͱ�λ */

    if (reg != OV2640_MID)      /* ID �Ƿ����� */
    {
        printf("MID:%d\r\n", reg);
        return 1;               /* ʧ�� */
    }
    
    reg = ov2640_read_reg(OV2640_SENSOR_PIDH);  /* ��ȡ����ID �߰�λ */
    reg <<= 8;
    reg |= ov2640_read_reg(OV2640_SENSOR_PIDL); /* ��ȡ����ID �Ͱ�λ */

    if (reg != OV2640_PID)      /* ID�Ƿ����� */
    {
        printf("HID:%d\r\n", reg);
        return 1;               /* ʧ�� */
    }

    /* ��ʼ�� OV2640 */
    for (i = 0; i < sizeof(ov2640_uxga_init_reg_tbl) / 2; i++)
    {
        ov2640_write_reg(ov2640_uxga_init_reg_tbl[i][0], ov2640_uxga_init_reg_tbl[i][1]);
    }

    return 0;                   /* OV2640��ʼ����� */
}

/**
 * @brief       OV2640 �л�ΪJPEGģʽ
 * @param       ��
 * @retval      ��
 */
void ov2640_jpeg_mode(void)
{
    uint16_t i = 0;

    /* ����:YUV422��ʽ */
    for (i = 0; i < (sizeof(ov2640_yuv422_reg_tbl) / 2); i++)
    {
        ov2640_write_reg(ov2640_yuv422_reg_tbl[i][0], ov2640_yuv422_reg_tbl[i][1]); /* ������������ */
    }
    
    /* ����:���JPEG���� */
    for (i = 0; i < (sizeof(ov2640_jpeg_reg_tbl) / 2); i++)
    {
        ov2640_write_reg(ov2640_jpeg_reg_tbl[i][0], ov2640_jpeg_reg_tbl[i][1]);     /* ������������ */
    }
}

/**
 * @brief       OV2640 �л�ΪRGB565ģʽ
 * @param       ��
 * @retval      ��
 */
void ov2640_rgb565_mode(void)
{
    uint16_t i = 0;

    /* ����:RGB565��� */
    for (i = 0; i < (sizeof(ov2640_rgb565_reg_tbl) / 4); i++)
    {
        ov2640_write_reg(ov2640_rgb565_reg_tbl[i][0], ov2640_rgb565_reg_tbl[i][1]); /* ������������ */
    }
}

/* �Զ��ع����ò�����,֧��5���ȼ� */
const static uint8_t OV2640_AUTOEXPOSURE_LEVEL[5][8]=
{
    {
        0xFF, 0x01,
        0x24, 0x20,
        0x25, 0x18,
        0x26, 0x60,
    },
    {
        0xFF, 0x01,
        0x24, 0x34,
        0x25, 0x1c,
        0x26, 0x00,
    },
    {
        0xFF, 0x01,
        0x24, 0x3e,
        0x25, 0x38,
        0x26, 0x81,
    },
    {
        0xFF, 0x01,
        0x24, 0x48,
        0x25, 0x40,
        0x26, 0x81,
    },
    {
        0xFF, 0x01,
        0x24, 0x58,
        0x25, 0x50,
        0x26, 0x92,
    },
};

/**
 * @brief       OV2640 EV�عⲹ��
 * @param       level : 0~4
 * @retval      ��
 */
void ov2640_auto_exposure(uint8_t level)
{
    uint8_t i;
    uint8_t *p = (uint8_t*)OV2640_AUTOEXPOSURE_LEVEL[level];
    
    for (i = 0; i < 4; i++)
    { 
        ov2640_write_reg(p[i * 2], p[i * 2 + 1]); 
    } 
}


/**
 * @brief       OV2640 ��ƽ������
 * @param       mode : 0~4, ��ƽ��ģʽ
 *   @arg       0: �Զ�   auto
 *   @arg       1: �չ�   sunny
 *   @arg       2: �칫�� office
 *   @arg       3: ����   cloudy
 *   @arg       4: ����   home
 * @retval      ��
 */
void ov2640_light_mode(uint8_t mode)
{
    uint8_t regccval = 0X5E;/* Sunny */
    uint8_t regcdval = 0X41;
    uint8_t regceval = 0X54;
    
    switch (mode)
    { 
        case 0:             /* auto */
            ov2640_write_reg(0XFF, 0X00);
            ov2640_write_reg(0XC7, 0X00);    /* AWB ON  */
            return;
        case 2:             /* cloudy */
            regccval = 0X65;
            regcdval = 0X41;
            regceval = 0X4F;
            break;
        case 3:             /* office */
            regccval = 0X52;
            regcdval = 0X41;
            regceval = 0X66;
            break;
        case 4:             /* home */
            regccval = 0X42;
            regcdval = 0X3F;
            regceval = 0X71;
            break;
        default : break;
    }
    
    ov2640_write_reg(0XFF, 0X00);
    ov2640_write_reg(0XC7, 0X40);            /* AWB OFF  */
    ov2640_write_reg(0XCC, regccval);
    ov2640_write_reg(0XCD, regcdval);
    ov2640_write_reg(0XCE, regceval);
}


/**
 * @brief       OV2640 ɫ�ʱ��Ͷ�����
 * @param       set : 0~4, ����ɫ�ʱ��Ͷ� -2 ~ 2.
 * @retval      ��
 */
void ov2640_color_saturation(uint8_t sat)
{
    uint8_t reg7dval = ((sat + 2) << 4) | 0X08;
    
    ov2640_write_reg(0XFF, 0X00);
    ov2640_write_reg(0X7C, 0X00);
    ov2640_write_reg(0X7D, 0X02);
    ov2640_write_reg(0X7C, 0X03);
    ov2640_write_reg(0X7D, reg7dval);
    ov2640_write_reg(0X7D, reg7dval);
}

/**
 * @brief       OV2640 ��������
 * @param       bright : 0~5, �������� -2 ~ 2.
 * @retval      ��
 */
void ov2640_brightness(uint8_t bright)
{
    ov2640_write_reg(0xff, 0x00);
    ov2640_write_reg(0x7c, 0x00);
    ov2640_write_reg(0x7d, 0x04);
    ov2640_write_reg(0x7c, 0x09);
    ov2640_write_reg(0x7d, bright << 4); 
    ov2640_write_reg(0x7d, 0x00); 
}

/**
 * @brief       OV2640 �Աȶ�����
 * @param       contrast : 0~4, ����Աȶ� -2 ~ 2.
 * @retval      ��
 */
void ov2640_contrast(uint8_t contrast)
{
    uint8_t reg7d0val = 0X20;       /* Ĭ��Ϊ��ͨģʽ */
    uint8_t reg7d1val = 0X20;
    
    switch (contrast)
    {
        case 0:
            reg7d0val = 0X18;       /* -2 */
            reg7d1val = 0X34;
            break;
        case 1:
            reg7d0val = 0X1C;       /* -1 */
            reg7d1val = 0X2A;
            break;
        case 3:
            reg7d0val = 0X24;       /* 1 */
            reg7d1val = 0X16;
            break;
        case 4:
            reg7d0val = 0X28;       /* 2 */
            reg7d1val = 0X0C;
            break;
        default : break;
    }
    
    ov2640_write_reg(0xff, 0x00);
    ov2640_write_reg(0x7c, 0x00);
    ov2640_write_reg(0x7d, 0x04);
    ov2640_write_reg(0x7c, 0x07);
    ov2640_write_reg(0x7d, 0x20);
    ov2640_write_reg(0x7d, reg7d0val);
    ov2640_write_reg(0x7d, reg7d1val);
    ov2640_write_reg(0x7d, 0x06);
}


/**
 * @brief       OV2640 ��Ч����
 * @param       eft : 0~6, ��ЧЧ��
 *   @arg       0: ����
 *   @arg       1: ��Ƭ
 *   @arg       2: �ڰ�
 *   @arg       3: ƫ��ɫ
 *   @arg       4: ƫ��ɫ
 *   @arg       5: ƫ��ɫ
 *   @arg       6: ����
 * @retval      ��
 */
void ov2640_special_effects(uint8_t eft)
{
    uint8_t reg7d0val = 0X00;   /* Ĭ��Ϊ��ͨģʽ */
    uint8_t reg7d1val = 0X80;
    uint8_t reg7d2val = 0X80; 
    
    switch(eft)
    {
        case 1:     /* ��Ƭ */
            reg7d0val = 0X40; 
            break;
        case 2:     /* �ڰ� */
            reg7d0val = 0X18; 
            break;
        case 3:     /* ƫ��ɫ */
            reg7d0val = 0X18; 
            reg7d1val = 0X40;
            reg7d2val = 0XC0; 
            break;
        case 4:     /* ƫ��ɫ */
            reg7d0val = 0X18; 
            reg7d1val = 0X40;
            reg7d2val = 0X40; 
            break;
        case 5:     /* ƫ��ɫ */
            reg7d0val = 0X18; 
            reg7d1val = 0XA0;
            reg7d2val = 0X40; 
            break;
        case 6:     /* ���� */
            reg7d0val = 0X18; 
            reg7d1val = 0X40;
            reg7d2val = 0XA6; 
            break;
    }
    
    ov2640_write_reg(0xff, 0x00);
    ov2640_write_reg(0x7c, 0x00);
    ov2640_write_reg(0x7d, reg7d0val);
    ov2640_write_reg(0x7c, 0x05);
    ov2640_write_reg(0x7d, reg7d1val);
    ov2640_write_reg(0x7d, reg7d2val); 
}

/**
 * @brief       OV2640 ��������
 * @param       mode : 0~2
 *   @arg       0: �ر� 
 *   @arg       1: ����
 *
 * @retval      ��
 */
void ov2640_color_bar(uint8_t mode)
{
    uint8_t reg;
    
    ov2640_write_reg(0XFF, 0X01);
    reg = ov2640_read_reg(0X12);
    reg &= ~(1 << 1);
    if (mode)reg |=1 << 1; 
    ov2640_write_reg(0X12, reg);
}

/**
 * @bref ���ô������������
 * @param sx    : ��ʼ��ַx
 * @param sy    : ��ʼ��ַy
 * @param width : ���(��Ӧ:horizontal)
 * @param hight : �߶�(��Ӧ:vertical)
 *
 * @retval ��
 */
void ov2640_window_set(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    uint16_t endx;
    uint16_t endy;
    uint8_t temp; 
    
    endx = sx + width / 2;
    endy = sy + height / 2;

    ov2640_write_reg(0XFF, 0X01);    
    temp = ov2640_read_reg(0X03);       /* ��ȡVref֮ǰ��ֵ */
    temp &= 0XF0;
    temp |= ((endy & 0X03) << 2) | (sy & 0X03);
    ov2640_write_reg(0X03, temp);       /* ����Vref��start��end�����2λ */ 
    ov2640_write_reg(0X19, sy >> 2);    /* ����Vref��start��8λ */
    ov2640_write_reg(0X1A, endy >> 2);  /* ����Vref��end�ĸ�8λ */

    temp = ov2640_read_reg(0X32);       /* ��ȡHref֮ǰ��ֵ */
    temp &= 0XC0;
    temp |= ((endx & 0X07) << 3) | (sx & 0X07);
    ov2640_write_reg(0X32, temp);       /* ����Href��start��end�����3λ */
    ov2640_write_reg(0X17, sx >> 3);    /* ����Href��start��8λ */
    ov2640_write_reg(0X18, endx >> 3);  /* ����Href��end�ĸ�8λ */
}

/** 
 * @bref    ����ͼ�������С
 * @@param  width : ���(��Ӧ:horizontal)
 * @param   height: �߶�(��Ӧ:vertical)
 * @note    OV2640���ͼ��Ĵ�С(�ֱ���),��ȫ�ɸú���ȷ��
 *          width��height������4�ı���
 *
 * @retval  0     : ���óɹ�
 *          ����  : ����ʧ��
 */
uint8_t ov2640_outsize_set(uint16_t width, uint16_t height)
{
    uint16_t outh;
    uint16_t outw;
    uint8_t temp;
    
    if (width % 4) return 1;
    if (height % 4) return 2;
    
    outw = width / 4;
    outh = height/ 4;
    ov2640_write_reg(0XFF, 0X00);    
    ov2640_write_reg(0XE0, 0X04);
    ov2640_write_reg(0X5A, outw & 0XFF);    /* ����OUTW�ĵͰ�λ */
    ov2640_write_reg(0X5B, outh & 0XFF);    /* ����OUTH�ĵͰ�λ */
    
    temp = (outw >> 8) & 0X03;
    temp |= (outh >> 6) & 0X04;
    ov2640_write_reg(0X5C, temp);           /* ����OUTH/OUTW�ĸ�λ */
    ov2640_write_reg(0XE0, 0X00);
    
    return 0;
}

/**
 * @brief       ����ͼ�񿪴���С
 *              ��:ov2640_imagesize_setȷ������������ֱ��ʴӴ�С.
 *              �ú������������Χ������п���,����ov2640_outsize_set�����
 *   @note      �������Ŀ�Ⱥ͸߶�,������ڵ���ov2640_outsize_set�����Ŀ�Ⱥ͸߶� 
 *              ov2640_outsize_set���õĿ�Ⱥ͸߶�,���ݱ��������õĿ�Ⱥ͸߶�,��DSP
 *              �Զ��������ű���,������ⲿ�豸.
 *
 * @param       offx,offy    : ���ͼ����ov2640_image_window_set�趨����(���賤��Ϊxsize��ysize)�ϵ�ƫ��
 * @param       width,height : ʵ�����ͼ��Ŀ�Ⱥ͸߶�,������4�ı���
 * @retval      0, �ɹ�; ����, ʧ��;
 */
uint8_t ov2640_image_win_set(uint16_t offx, uint16_t offy, uint16_t width, uint16_t height)
{
    uint16_t hsize;
    uint16_t vsize;
    uint8_t temp;
    
    if (width % 4) return 1;
    if (height % 4) return 2;
    hsize = width / 4;
    vsize = height / 4;
    ov2640_write_reg(0XFF, 0X00);
    ov2640_write_reg(0XE0, 0X04);
    ov2640_write_reg(0X51, hsize & 0XFF);           /* ����H_SIZE�ĵͰ�λ */
    ov2640_write_reg(0X52, vsize & 0XFF);           /* ����V_SIZE�ĵͰ�λ */
    ov2640_write_reg(0X53, offx & 0XFF);            /* ����offx�ĵͰ�λ */
    ov2640_write_reg(0X54, offy & 0XFF);            /* ����offy�ĵͰ�λ */
    temp = (vsize >> 1) & 0X80;
    temp |= (offy >> 4) & 0X70;
    temp |= (hsize>>5) & 0X08;
    temp |= (offx >> 8) & 0X07; 
    ov2640_write_reg(0X55, temp);                   /* ����H_SIZE/V_SIZE/OFFX,OFFY�ĸ�λ */
    ov2640_write_reg(0X57, (hsize >> 2) & 0X80);    /* ����H_SIZE/V_SIZE/OFFX,OFFY�ĸ�λ */
    ov2640_write_reg(0XE0, 0X00);
    
    return 0;
}

/**
 * @bref    �ú�������ͼ��ߴ��С,Ҳ������ѡ��ʽ������ֱ���
 * @note    UXGA:1600*1200,SVGA:800*600,CIF:352*288
 * @param   width  : ͼ����
 * @param   height : ͼ��߶�
 *
 * @retval  0      : ���óɹ�
 *          ����   : ����ʧ��
 */
uint8_t ov2640_imagesize_set(uint16_t width,uint16_t height)
{ 
    uint8_t temp; 
    
    ov2640_write_reg(0XFF, 0X00);
    ov2640_write_reg(0XE0, 0X04);
    ov2640_write_reg(0XC0, (width) >> 3 & 0XFF);    /* ����HSIZE��10:3λ */
    ov2640_write_reg(0XC1, (height) >> 3 & 0XFF);   /* ����VSIZE��10:3λ */
    
    temp = (width & 0X07) << 3;
    temp |= height & 0X07;
    temp |= (width >> 4) & 0X80; 
    
    ov2640_write_reg(0X8C, temp);
    ov2640_write_reg(0XE0, 0X00);
    
    return 0;
}


