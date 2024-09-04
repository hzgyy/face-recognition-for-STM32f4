/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-14
 * @brief       ����� ʵ��
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ̽���� F407������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./USMART/usmart.h"
#include "./BSP/KEY/key.h"
#include "./BSP/SRAM/sram.h"
#include "./MALLOC/malloc.h"
#include "./BSP/SDIO/sdio_sdcard.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./TEXT/text.h"
#include "./PICTURE/piclib.h"
#include "./BSP/TIMER/btim.h"
#include "./BSP/OV2640/ov2640.h"
#include "./BSP/OV2640/sccb.h"
#include "./BSP/DCMI/dcmi.h"
#include "./BSP/USART2/usart2.h"
#include "./BSP/BEEP/beep.h"
#include "string.h"
#include "math.h"
#include "face_recognition.h"
#include "atk_frec.h"
#include "./BSP/USART2/usart2.h"
#include "syn6288.h"
#include "./BSP/PWMDAC/pwmdac.h"

volatile uint8_t g_bmp_request = 0;                     /* bmp��������:0,��bmp��������;1,��bmp��������,��Ҫ��֡�ж�����,�ر�DCMI�ӿ� */

uint8_t g_ov_mode = 0;                                  /* bit0: 0, RGB565ģʽ;  1, JPEGģʽ */

#define jpeg_buf_size       300*1024                    /* ����JPEG���ݻ���jpeg_buf�Ĵ�С(�ֽ�) */
#define jpeg_line_size      1*1024                      /* ����DMA��������ʱ,һ�����ݵ����ֵ */

uint32_t *p_dcmi_line_buf[2];                           /* JPEG���� DMA˫����bufָ�� */
uint32_t *p_jpeg_data_buf;                              /* JPEG���ݻ���bufָ�� */

volatile uint32_t g_jpeg_data_len = 0;                  /* buf�е�JPEG��Ч���ݳ��� */

uint8_t SensorFlag;
uint8_t CrossFlag;
uint8_t TimeCount;

/**
 * 0,����û�вɼ���;
 * 1,���ݲɼ�����,���ǻ�û����;
 * 2,�����Ѿ����������,���Կ�ʼ��һ֡����
 */
volatile uint8_t g_jpeg_data_ok = 0;                    /* JPEG���ݲɼ���ɱ�־ */


/**
 * @brief       ����JPEG����
 *   @ntoe      ��DCMI_IRQHandler�жϷ��������汻����
 *              ���ɼ���һ֡JPEG���ݺ�,���ô˺���,�л�JPEG BUF.��ʼ��һ֡�ɼ�.
 *
 * @param       ��
 * @retval      ��
 */
void jpeg_data_process(void)
{
    uint16_t i;
    uint16_t rlen;      /* ʣ�����ݳ��� */
    uint32_t *pbuf;
    
    if (g_ov_mode)      /* ֻ����JPEG��ʽ��,����Ҫ������. */
    { 
        if (g_jpeg_data_ok == 0)                /* jpeg���ݻ�δ�ɼ���? */
        {
            DMA2_Stream1->CR &= ~(1 << 0);      /* ֹͣ��ǰ���� */
    
            while (DMA2_Stream1->CR & 0X01);    /* �ȴ�DMA2_Stream1������ */

            rlen = jpeg_line_size - DMA2_Stream1->NDTR; /* �õ�ʣ�����ݳ��� */
            pbuf = p_jpeg_data_buf + g_jpeg_data_len;   /* ƫ�Ƶ���Ч����ĩβ,������� */

            if (DMA2_Stream1->CR & (1 << 19))
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = p_dcmi_line_buf[1][i];    /* ��ȡbuf1�����ʣ������ */
                }
            }
            else 
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = p_dcmi_line_buf[0][i];    /* ��ȡbuf0�����ʣ������ */
                }
            }

            g_jpeg_data_len += rlen;    /* ����ʣ�೤�� */
            g_jpeg_data_ok = 1;         /* ���JPEG���ݲɼ����,�ȴ������������� */
        }

        if (g_jpeg_data_ok == 2)        /* ��һ�ε�jpeg�����Ѿ��������� */
        {
            DMA2_Stream1->NDTR = jpeg_line_size;    /* ���䳤��Ϊjpeg_line_size*4�ֽ� */
            DMA2_Stream1->CR |= 1 << 0; /* ���´��� */
            g_jpeg_data_ok = 0;         /* �������δ�ɼ� */
            g_jpeg_data_len = 0;        /* �������¿�ʼ */
        }
    }
    else
    {
        lcd_set_cursor(0, 0);
        lcd_write_ram_prepare();        /* ��ʼд��GRAM */
    }
}

/**
 * @brief       JPEG���ݽ��ջص�����
 *   @ntoe      ��DMA2_Stream1_IRQHandler�жϷ��������汻����
 *
 * @param       ��
 * @retval      ��
 */
void jpeg_dcmi_rx_callback(void)
{
    uint16_t i;
    volatile uint32_t *pbuf;
    pbuf = p_jpeg_data_buf + g_jpeg_data_len;   /* ƫ�Ƶ���Ч����ĩβ */

    if (DMA2_Stream1->CR & (1 << 19))           /* buf0����,��������buf1 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = p_dcmi_line_buf[0][i];    /* ��ȡbuf0��������� */
        }
        
        g_jpeg_data_len += jpeg_line_size;      /* ƫ�� */
    }
    else    /* buf1����,��������buf0 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = p_dcmi_line_buf[1][i];    /* ��ȡbuf1��������� */
        }
        
        g_jpeg_data_len += jpeg_line_size;      /* ƫ�� */
    }
}

/**
 * @brief       �л�ΪOV2640ģʽ
 *   @note      �л�PC8/PC9/PC11ΪDCMI���ù���(AF13)
 * @param       ��
 * @retval      ��
 */
void sw_ov2640_mode(void)
{
    OV2640_PWDN(0);                             /* OV2640 power up */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 13);  /* PC8,AF13  DCMI_D2 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 13);  /* PC9,AF13  DCMI_D3 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 13); /* PC11,AF13 DCMI_D4 */
}


/**
 * @brief       �л�ΪSD��ģʽ
 *   @note      �л�PC8/PC9/PC11ΪSDIO���ù���(AF12)
 * @param       ��
 * @retval      ��
 */
void sw_sdcard_mode(void)
{
    OV2640_PWDN(1);                             /* OV2640 power down */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 12);  /* PC8,AF12  SDIO_D0 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 12);  /* PC9,AF12  SDIO_D1 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 12); /* PC11,AF12 SDIO_D3 */
}

/**
 * @brief       �ļ������������⸲�ǣ�
 *   @note      bmp��ϳ�: ���� "0:PHOTO/PIC13141.bmp" ���ļ���
 *              jpg��ϳ�: ���� "0:PHOTO/PIC13141.jpg" ���ļ���
 * @param       pname : ��Ч���ļ���
 * @param       mode  : 0, ����.bmp�ļ�;  1, ����.jpg�ļ�;
 * @retval      ��
 */
void camera_new_pathname(uint8_t *pname, uint8_t mode)
{
    uint8_t res;
    uint16_t index = 0;
    FIL *ftemp;
    
    ftemp = (FIL *)mymalloc(SRAMIN, sizeof(FIL));   /* ����FIL�ֽڵ��ڴ����� */

    if (ftemp == NULL) return;  /* �ڴ�����ʧ�� */

    while (index < 0XFFFF)
    {
        if (mode == 0)  /* ����.bmp�ļ��� */
        {
            sprintf((char *)pname, "0:PHOTO/PIC%05d.bmp", index);
        }
        else  /* ����.jpg�ļ��� */
        {
            sprintf((char *)pname, "0:PHOTO/PIC%05d.jpg", index);
        }
        
        res = f_open(ftemp, (const TCHAR *)pname, FA_READ); /* ���Դ�����ļ� */

        if (res == FR_NO_FILE)break;    /* ���ļ���������, ����������Ҫ�� */

        index++;
    }
    myfree(SRAMIN, ftemp);
}


/**
 * @brief       OV2640����jpgͼƬ
 * @param       pname : Ҫ������jpg�ļ���(��·��)
 * @retval      0, �ɹ�; ����,�������;
 */
uint8_t ov2640_jpg_photo(uint8_t *pname)
{
    FIL *f_jpg;
    uint8_t res = 0, headok = 0;
    uint32_t bwr;
    uint32_t i, jpgstart, jpglen;
    uint8_t *pbuf;
    
    uint16_t datasize = 0;          /* ����д�������� */
    uint32_t datalen = 0;           /* ��д�������� */
    uint8_t  *databuf;              /* ���ݻ��棬����ֱ��д�ⲿSRAM���ݵ�SD��������д��������� */
    
    f_jpg = (FIL *)mymalloc(SRAMIN, sizeof(FIL));   /* ����FIL�ֽڵ��ڴ����� */
    databuf = mymalloc(SRAMIN, 4096);   /* ����4K�ڴ� */
    
    if (databuf == NULL) return 0XFF;   /* �ڴ�����ʧ�� */

    g_ov_mode = 1;
    g_jpeg_data_ok = 0;
    
    sw_ov2640_mode();               /* �л�ΪOV2640ģʽ */
    ov2640_jpeg_mode();             /* JPEGģʽ */
    
    dcmi_rx_callback = jpeg_dcmi_rx_callback;   /* JPEG�������ݻص����� */
    dcmi_dma_init((uint32_t)p_dcmi_line_buf[0], (uint32_t)p_dcmi_line_buf[1], jpeg_line_size, 2, 1); /* DCMI DMA���� */

    ov2640_image_win_set(0, 0, 1600, 1200);
    ov2640_outsize_set(1600, 1200);  /* ��������ߴ�(1600 * 1200) */
    
    dcmi_start();                   /* �������� */
    while (g_jpeg_data_ok != 1);    /* �ȴ���һ֡ͼƬ�ɼ��� */
    g_jpeg_data_ok = 2;             /* ���Ա�֡ͼƬ,������һ֡�ɼ� */
    while (g_jpeg_data_ok != 1);    /* �ȴ��ڶ�֡ͼƬ�ɼ���  */
    g_jpeg_data_ok = 2;             /* ���Ա�֡ͼƬ,������һ֡�ɼ� */
    while (g_jpeg_data_ok != 1);    /* �ȴ�����֡ͼƬ�ɼ���,����֡,�ű��浽SD��ȥ */

    dcmi_stop();                    /* ֹͣDMA���� */
    g_ov_mode = 0;
    sw_sdcard_mode();               /* �л�ΪSD��ģʽ */
    
    printf("jpeg data size:%d\r\n", g_jpeg_data_len * 4);   /* ���ڴ�ӡJPEG�ļ���С */
    pbuf = (uint8_t *)p_jpeg_data_buf;
    jpglen = 0;                     /* ����jpg�ļ���СΪ0 */
    headok = 0;                     /* ���jpgͷ��� */

    for (i = 0; i < g_jpeg_data_len * 4; i++)   /* ����0XFF,0XD8��0XFF,0XD9,��ȡjpg�ļ���С */
    {
        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD8)) /* �ҵ�FF D8 */
        {
            jpgstart = i;
            headok = 1;             /* ����ҵ�jpgͷ(FF D8) */
        }

        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD9) && headok)   /* �ҵ�ͷ�Ժ�,����FF D9 */
        {
            jpglen = i - jpgstart + 2;
            break;
        }
    }

    if (jpglen)                     /* ������jpeg���� */
    {
        res = f_open(f_jpg, (const TCHAR *)pname, FA_WRITE | FA_CREATE_NEW);    /* ģʽ0,���߳��Դ�ʧ��,�򴴽����ļ� */

        if (res == 0)
        {
            pbuf += jpgstart;       /* ƫ�Ƶ�0XFF,0XD8�� */
            
            while(datalen < jpglen) /* ѭ��д�룡����ֱ��д�ⲿSRAM���ݵ�SDIO�������������FIFO������� */
            {
                if((jpglen - datalen) > 4096)
                {
                    datasize = 4096;
                }else
                {
                    datasize = jpglen - datalen;    /* �������� */
                }

                my_mem_copy(databuf, pbuf, datasize);
                res = f_write(f_jpg, databuf, datasize, (UINT *)&bwr); /* д������ */
                pbuf += datasize;
                jpglen -= datasize;

                if (res)break;
            }
        }

        f_close(f_jpg);
    }
    else
    {
        res = 0XFD;
    }
    
    g_jpeg_data_len = 0;
    sw_ov2640_mode();       /* �л�ΪOV2640ģʽ */
    ov2640_rgb565_mode();   /* RGB565ģʽ */
    
    dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMA����,MCU��,���� */
    myfree(SRAMIN, f_jpg);
    myfree(SRAMIN, databuf);
    return res;
}




void SensorGPIOConfig(void){
	RCC->AHB1ENR |= 1 << 0;
	sys_gpio_set(GPIOA,SYS_GPIO_PIN1,SYS_GPIO_MODE_IN,SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);
}

void SensorStatus(void){
	if(sys_gpio_pin_get(GPIOA, SYS_GPIO_PIN1) == 1){
		SensorFlag = 0;
	}else{
		SensorFlag = 1;
	}
}


void cross()             
{
    SensorFlag = 0;
    CrossFlag = 0;
    TimeCount = 0;
    open_door();
    //?????????
    while(CrossFlag==0)
    {
        SensorStatus();
        while(!SensorFlag){
            SensorStatus();
            }
        delay_ms(500);
        SensorStatus();
        if(SensorFlag==1)
            CrossFlag = 1;
    }
    close_door();
    //??????????
		delay_ms(500);
    while(TimeCount<10)
    {
			SensorStatus();
        while(!SensorFlag)                    //??100ms � imax ?????
        {
					SensorStatus();
            delay_ms(100);
            TimeCount++;
						if(TimeCount==10)
							break;
        }
        delay_ms(500);                        //200ms????????
				SensorStatus();
        if(SensorFlag==1){
            TimeCount = 10;
            alarm_say();
        }
    }
    TimeCount = 0;
		PWMDAC_TIMX_CCRX = 1500;
}

int main(void)
{
    uint8_t res;
    float fac;
    uint8_t *pname;                             /* ��·�����ļ��� */
    uint8_t key;                                /* ��ֵ */
    uint8_t i;
    uint8_t sd_ok = 1;                          /* 0,sd��������;1,SD������ */
    uint8_t scale = 1;                          /* Ĭ����ȫ�ߴ����� */
    uint8_t msgbuf[15];                         /* ��Ϣ������ */
		uint16_t *ImageData;								// initialize the picture buffer
		uint8_t person;

    sys_stm32_clock_init(336, 8, 2, 7);         /* ����ʱ��,168Mhz */
    delay_init(168);                            /* ��ʱ��ʼ�� */
    usart_init(84, 115200);                     /* ���ڳ�ʼ��Ϊ115200 */
    usart2_init(42,9600);
		usmart_dev.init(84);                        /* ��ʼ��USMART */
		pwmdac_init(20000-1, 83);                    /* PWM DAC��ʼ��, Fpwm = 84M / 256 = 328.125Khz */
    led_init();                                 /* ��ʼ��LED */
    lcd_init();                                 /* ��ʼ��LCD */
    key_init();                                 /* ��ʼ������ */
    sram_init();                                /* SRAM��ʼ�� */
    norflash_init();                            /* ��ʼ��NORFLASH */
    beep_init();                                /* ��ʼ�������� */
    btim_timx_int_init(10000 - 1, 8400 - 1);    /* 10KHz����Ƶ�ʣ�1���ж�һ�� */
    piclib_init();                              /* ��ʼ����ͼ */
    SensorGPIOConfig();
		
		
    my_mem_init(SRAMIN);                        /* ��ʼ���ڲ�SRAM�ڴ�� */
    my_mem_init(SRAMEX);                        /* ��ʼ���ⲿSRAM�ڴ�� */
    my_mem_init(SRAMCCM);                       /* ��ʼ��CCM�ڴ�� */

    exfuns_init();                              /* Ϊfatfs��ر��������ڴ� */
    f_mount(fs[0], "0:", 1);                    /* ����SD�� */
    f_mount(fs[1], "1:", 1);                    /* ����FLASH */

    while (fonts_init())                        /* ����ֿ� */
    {
        lcd_show_string(30, 50, 200, 16, 16, "Font Error!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 240, 66, WHITE);       /* �����ʾ */
        delay_ms(200);
    }
			
    text_show_string(30, 50, 200, 16, "����ԭ��STM32������", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "�����ʵ��", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "KEY0:����(bmp��ʽ)", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY1:����(jpg��ʽ)", 16, 0, RED);
    text_show_string(30, 130, 200, 16, "WK_UP:FullSize/Scale", 16, 0, RED);
    sprintf((char*)msgbuf,"id:%x,width:%d,",lcddev.id,lcddev.width);
		lcd_show_string(30, 250, 200, 16, 16, (char *)msgbuf, RED);
		sprintf((char*)msgbuf,"height:%d",lcddev.height);
		lcd_show_string(30, 270, 200, 16, 16, (char *)msgbuf, RED);
    res = f_mkdir("0:/PHOTO");              /* ����PHOTO�ļ��� */
    
    if (res != FR_EXIST && res != FR_OK)    /* �����˴��� */
    {
        res = f_mkdir("0:/PHOTO");          /* ����PHOTO�ļ��� */
        text_show_string(30, 150, 240, 16, "SD������!", 16, 0, RED);
        delay_ms(200);
        text_show_string(30, 150, 240, 16, "���չ��ܽ�������!", 16, 0, RED);
        delay_ms(200);
        sd_ok = 0;
    }
		
		ImageData = mymalloc(SRAMIN,30*40*2);
    p_dcmi_line_buf[0] = mymalloc(SRAMIN, jpeg_line_size * 4);  /* Ϊjpeg dma���������ڴ� */
    p_dcmi_line_buf[1] = mymalloc(SRAMIN, jpeg_line_size * 4);  /* Ϊjpeg dma���������ڴ� */
    p_jpeg_data_buf = mymalloc(SRAMEX, jpeg_buf_size);          /* Ϊjpeg�ļ������ڴ� */
    pname = mymalloc(SRAMIN, 30);                               /* Ϊ��·�����ļ�������30���ֽڵ��ڴ� */

    while (pname == NULL || !p_dcmi_line_buf[0] || !p_dcmi_line_buf[1] || !p_jpeg_data_buf) /* �ڴ������� */
    {
        text_show_string(30, 150, 240, 16, "�ڴ����ʧ��!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 150, 240, 146, WHITE); /* �����ʾ */
        delay_ms(200);
    }

    while (ov2640_init())   /* ��ʼ��OV2640 */
    {
        text_show_string(30, 170, 240, 16, "OV2640 ����!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 150, 239, 206, WHITE);
        delay_ms(200);
    }

    delay_ms(100);
    text_show_string(30, 170, 230, 16, "OV2640 ����", 16, 0, RED);
    
    ov2640_rgb565_mode();       /* RGB565ģʽ */
    dcmi_init();                /* DCMI���� */
    dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMA����,MCU��,���� */
    
    ov2640_outsize_set(lcddev.width, lcddev.height);    /* ����������ʾ */
    dcmi_start();               /* �������� */
    lcd_clear(BLACK);
		atk_frec_initialization();
    while (1)
    {
        key = key_scan(0);
        if (key)
        {
            dcmi_stop();                /* ֹͣ��ʾ */
            if (sd_ok)             /* SD�������ſ������� */
            {
                sw_sdcard_mode();       /* �л�ΪSD��ģʽ */
								switch(key){
									case KEY0_PRES:{			//recognize a face
										frec_get_image_data(ImageData,0,0,lcddev.width,30);
										sprintf((char*)msgbuf,"����ʶ��...");
										text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										res=atk_frec_recognition_face(ImageData,&person);//����ʶ��
										if(res==ATK_FREC_MODEL_DATA_ERR)
										{	 
											sprintf((char*)msgbuf,"û�п���ģ��,��KEY_UP���ģ��!");
											text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										}else if(res==ATK_FREC_UNREC_FACE_ERR)
										{	
											sprintf((char*)msgbuf,"�޷�ʶ�������,������!");
											text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
											error_say();
											delay_ms(3000);
										}else 
										{
											sprintf((char*)msgbuf,"����԰��ӭ��:�û�%02d",person);
											text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
											open_say();
											cross();
										}
										break;
									}
									case WKUP_PRES:{			//record a face
										frec_get_image_data(ImageData,0,0,lcddev.width,30);
										res = atk_frec_add_a_face(ImageData,&person);
										sprintf((char*)msgbuf,"����¼��...");
										text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										if(res == 0){
											sprintf((char*)msgbuf,"��ӳɹ�,�û�:%02d   ",person);
											atk_frec_load_data_model();	//���¼�������ʶ��ģ��(����ӵ�����,���ؽ���)
										}else{
											sprintf((char*)msgbuf,"���ʧ��,�������:%02d",res);
										}
										break;
									}
									case KEY1_PRES:{
										sprintf((char*)msgbuf,"����ɾ��...");
										text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										for(i=0;i<MAX_LEBEL_NUM;i++)
										{
											res=atk_frec_delete_data(i);//ɾ��ģ��
										}
										atk_frec_load_data_model();	//���¼�������ʶ��ģ��(��ɾ����,���޷����ؽ���.)
										sprintf((char*)msgbuf,"ɾ�����");
										text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										delay_ms(1000);
										break;
									}
									default:break;
								}
                text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										delay_ms(500);
                sw_ov2640_mode();   /* �л�ΪOV2640ģʽ */ 
            }
            else    /* ��ʾSD������ */
            {
                text_show_string(30, 130, 240, 16, "SD������!", 16, 0, RED);
                text_show_string(30, 150, 240, 16, "���ܲ�����!", 16, 0, RED);
            }
            if (key != WKUP_PRES) delay_ms(1800);
            
            dcmi_start();   /* ��ʼ��ʾ */
        }

        delay_ms(10);
        i++;

        if (i == 20)    /* DS0��˸ */
        {
            i = 0; 
            LED0_TOGGLE();
        }
    }
}

















