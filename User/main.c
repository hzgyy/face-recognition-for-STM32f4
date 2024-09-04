/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ÕıµãÔ­×ÓÍÅ¶Ó(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-14
 * @brief       ÕÕÏà»ú ÊµÑé
 * @license     Copyright (c) 2020-2032, ¹ãÖİÊĞĞÇÒíµç×Ó¿Æ¼¼ÓĞÏŞ¹«Ë¾
 ****************************************************************************************************
 * @attention
 *
 * ÊµÑéÆ½Ì¨:ÕıµãÔ­×Ó Ì½Ë÷Õß F407¿ª·¢°å
 * ÔÚÏßÊÓÆµ:www.yuanzige.com
 * ¼¼ÊõÂÛÌ³:www.openedv.com
 * ¹«Ë¾ÍøÖ·:www.alientek.com
 * ¹ºÂòµØÖ·:openedv.taobao.com
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

volatile uint8_t g_bmp_request = 0;                     /* bmpÅÄÕÕÇëÇó:0,ÎŞbmpÅÄÕÕÇëÇó;1,ÓĞbmpÅÄÕÕÇëÇó,ĞèÒªÔÚÖ¡ÖĞ¶ÏÀïÃæ,¹Ø±ÕDCMI½Ó¿Ú */

uint8_t g_ov_mode = 0;                                  /* bit0: 0, RGB565Ä£Ê½;  1, JPEGÄ£Ê½ */

#define jpeg_buf_size       300*1024                    /* ¶¨ÒåJPEGÊı¾İ»º´æjpeg_bufµÄ´óĞ¡(×Ö½Ú) */
#define jpeg_line_size      1*1024                      /* ¶¨ÒåDMA½ÓÊÕÊı¾İÊ±,Ò»ĞĞÊı¾İµÄ×î´óÖµ */

uint32_t *p_dcmi_line_buf[2];                           /* JPEGÊı¾İ DMAË«»º´æbufÖ¸Õë */
uint32_t *p_jpeg_data_buf;                              /* JPEGÊı¾İ»º´æbufÖ¸Õë */

volatile uint32_t g_jpeg_data_len = 0;                  /* bufÖĞµÄJPEGÓĞĞ§Êı¾İ³¤¶È */

uint8_t SensorFlag;
uint8_t CrossFlag;
uint8_t TimeCount;

/**
 * 0,Êı¾İÃ»ÓĞ²É¼¯Íê;
 * 1,Êı¾İ²É¼¯ÍêÁË,µ«ÊÇ»¹Ã»´¦Àí;
 * 2,Êı¾İÒÑ¾­´¦ÀíÍê³ÉÁË,¿ÉÒÔ¿ªÊ¼ÏÂÒ»Ö¡½ÓÊÕ
 */
volatile uint8_t g_jpeg_data_ok = 0;                    /* JPEGÊı¾İ²É¼¯Íê³É±êÖ¾ */


/**
 * @brief       ´¦ÀíJPEGÊı¾İ
 *   @ntoe      ÔÚDCMI_IRQHandlerÖĞ¶Ï·şÎñº¯ÊıÀïÃæ±»µ÷ÓÃ
 *              µ±²É¼¯ÍêÒ»Ö¡JPEGÊı¾İºó,µ÷ÓÃ´Ëº¯Êı,ÇĞ»»JPEG BUF.¿ªÊ¼ÏÂÒ»Ö¡²É¼¯.
 *
 * @param       ÎŞ
 * @retval      ÎŞ
 */
void jpeg_data_process(void)
{
    uint16_t i;
    uint16_t rlen;      /* Ê£ÓàÊı¾İ³¤¶È */
    uint32_t *pbuf;
    
    if (g_ov_mode)      /* Ö»ÓĞÔÚJPEG¸ñÊ½ÏÂ,²ÅĞèÒª×ö´¦Àí. */
    { 
        if (g_jpeg_data_ok == 0)                /* jpegÊı¾İ»¹Î´²É¼¯Íê? */
        {
            DMA2_Stream1->CR &= ~(1 << 0);      /* Í£Ö¹µ±Ç°´«Êä */
    
            while (DMA2_Stream1->CR & 0X01);    /* µÈ´ıDMA2_Stream1¿ÉÅäÖÃ */

            rlen = jpeg_line_size - DMA2_Stream1->NDTR; /* µÃµ½Ê£ÓàÊı¾İ³¤¶È */
            pbuf = p_jpeg_data_buf + g_jpeg_data_len;   /* Æ«ÒÆµ½ÓĞĞ§Êı¾İÄ©Î²,¼ÌĞøÌí¼Ó */

            if (DMA2_Stream1->CR & (1 << 19))
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = p_dcmi_line_buf[1][i];    /* ¶ÁÈ¡buf1ÀïÃæµÄÊ£ÓàÊı¾İ */
                }
            }
            else 
            {
                for (i = 0; i < rlen; i++)
                {
                    pbuf[i] = p_dcmi_line_buf[0][i];    /* ¶ÁÈ¡buf0ÀïÃæµÄÊ£ÓàÊı¾İ */
                }
            }

            g_jpeg_data_len += rlen;    /* ¼ÓÉÏÊ£Óà³¤¶È */
            g_jpeg_data_ok = 1;         /* ±ê¼ÇJPEGÊı¾İ²É¼¯Íê³É,µÈ´ıÆäËûº¯Êı´¦Àí */
        }

        if (g_jpeg_data_ok == 2)        /* ÉÏÒ»´ÎµÄjpegÊı¾İÒÑ¾­±»´¦ÀíÁË */
        {
            DMA2_Stream1->NDTR = jpeg_line_size;    /* ´«Êä³¤¶ÈÎªjpeg_line_size*4×Ö½Ú */
            DMA2_Stream1->CR |= 1 << 0; /* ÖØĞÂ´«Êä */
            g_jpeg_data_ok = 0;         /* ±ê¼ÇÊı¾İÎ´²É¼¯ */
            g_jpeg_data_len = 0;        /* Êı¾İÖØĞÂ¿ªÊ¼ */
        }
    }
    else
    {
        lcd_set_cursor(0, 0);
        lcd_write_ram_prepare();        /* ¿ªÊ¼Ğ´ÈëGRAM */
    }
}

/**
 * @brief       JPEGÊı¾İ½ÓÊÕ»Øµ÷º¯Êı
 *   @ntoe      ÔÚDMA2_Stream1_IRQHandlerÖĞ¶Ï·şÎñº¯ÊıÀïÃæ±»µ÷ÓÃ
 *
 * @param       ÎŞ
 * @retval      ÎŞ
 */
void jpeg_dcmi_rx_callback(void)
{
    uint16_t i;
    volatile uint32_t *pbuf;
    pbuf = p_jpeg_data_buf + g_jpeg_data_len;   /* Æ«ÒÆµ½ÓĞĞ§Êı¾İÄ©Î² */

    if (DMA2_Stream1->CR & (1 << 19))           /* buf0ÒÑÂú,Õı³£´¦Àíbuf1 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = p_dcmi_line_buf[0][i];    /* ¶ÁÈ¡buf0ÀïÃæµÄÊı¾İ */
        }
        
        g_jpeg_data_len += jpeg_line_size;      /* Æ«ÒÆ */
    }
    else    /* buf1ÒÑÂú,Õı³£´¦Àíbuf0 */
    {
        for (i = 0; i < jpeg_line_size; i++)
        {
            pbuf[i] = p_dcmi_line_buf[1][i];    /* ¶ÁÈ¡buf1ÀïÃæµÄÊı¾İ */
        }
        
        g_jpeg_data_len += jpeg_line_size;      /* Æ«ÒÆ */
    }
}

/**
 * @brief       ÇĞ»»ÎªOV2640Ä£Ê½
 *   @note      ÇĞ»»PC8/PC9/PC11ÎªDCMI¸´ÓÃ¹¦ÄÜ(AF13)
 * @param       ÎŞ
 * @retval      ÎŞ
 */
void sw_ov2640_mode(void)
{
    OV2640_PWDN(0);                             /* OV2640 power up */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 13);  /* PC8,AF13  DCMI_D2 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 13);  /* PC9,AF13  DCMI_D3 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 13); /* PC11,AF13 DCMI_D4 */
}


/**
 * @brief       ÇĞ»»ÎªSD¿¨Ä£Ê½
 *   @note      ÇĞ»»PC8/PC9/PC11ÎªSDIO¸´ÓÃ¹¦ÄÜ(AF12)
 * @param       ÎŞ
 * @retval      ÎŞ
 */
void sw_sdcard_mode(void)
{
    OV2640_PWDN(1);                             /* OV2640 power down */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN8, 12);  /* PC8,AF12  SDIO_D0 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN9, 12);  /* PC9,AF12  SDIO_D1 */
    sys_gpio_af_set(GPIOC, SYS_GPIO_PIN11, 12); /* PC11,AF12 SDIO_D3 */
}

/**
 * @brief       ÎÄ¼şÃû×ÔÔö£¨±ÜÃâ¸²¸Ç£©
 *   @note      bmp×éºÏ³É: ĞÎÈç "0:PHOTO/PIC13141.bmp" µÄÎÄ¼şÃû
 *              jpg×éºÏ³É: ĞÎÈç "0:PHOTO/PIC13141.jpg" µÄÎÄ¼şÃû
 * @param       pname : ÓĞĞ§µÄÎÄ¼şÃû
 * @param       mode  : 0, ´´½¨.bmpÎÄ¼ş;  1, ´´½¨.jpgÎÄ¼ş;
 * @retval      ÎŞ
 */
void camera_new_pathname(uint8_t *pname, uint8_t mode)
{
    uint8_t res;
    uint16_t index = 0;
    FIL *ftemp;
    
    ftemp = (FIL *)mymalloc(SRAMIN, sizeof(FIL));   /* ¿ª±ÙFIL×Ö½ÚµÄÄÚ´æÇøÓò */

    if (ftemp == NULL) return;  /* ÄÚ´æÉêÇëÊ§°Ü */

    while (index < 0XFFFF)
    {
        if (mode == 0)  /* ´´½¨.bmpÎÄ¼şÃû */
        {
            sprintf((char *)pname, "0:PHOTO/PIC%05d.bmp", index);
        }
        else  /* ´´½¨.jpgÎÄ¼şÃû */
        {
            sprintf((char *)pname, "0:PHOTO/PIC%05d.jpg", index);
        }
        
        res = f_open(ftemp, (const TCHAR *)pname, FA_READ); /* ³¢ÊÔ´ò¿ªÕâ¸öÎÄ¼ş */

        if (res == FR_NO_FILE)break;    /* ¸ÃÎÄ¼şÃû²»´æÔÚ, ÕıÊÇÎÒÃÇĞèÒªµÄ */

        index++;
    }
    myfree(SRAMIN, ftemp);
}


/**
 * @brief       OV2640ÅÄÕÕjpgÍ¼Æ¬
 * @param       pname : Òª´´½¨µÄjpgÎÄ¼şÃû(º¬Â·¾¶)
 * @retval      0, ³É¹¦; ÆäËû,´íÎó´úÂë;
 */
uint8_t ov2640_jpg_photo(uint8_t *pname)
{
    FIL *f_jpg;
    uint8_t res = 0, headok = 0;
    uint32_t bwr;
    uint32_t i, jpgstart, jpglen;
    uint8_t *pbuf;
    
    uint16_t datasize = 0;          /* µ¥´ÎĞ´ÈëÊı¾İÁ¿ */
    uint32_t datalen = 0;           /* ×ÜĞ´ÈëÊı¾İÁ¿ */
    uint8_t  *databuf;              /* Êı¾İ»º´æ£¬±ÜÃâÖ±½ÓĞ´Íâ²¿SRAMÊı¾İµ½SD¿¨£¬µ¼ÖÂĞ´ÈëÏÂÒç´íÎó */
    
    f_jpg = (FIL *)mymalloc(SRAMIN, sizeof(FIL));   /* ¿ª±ÙFIL×Ö½ÚµÄÄÚ´æÇøÓò */
    databuf = mymalloc(SRAMIN, 4096);   /* ÉêÇë4KÄÚ´æ */
    
    if (databuf == NULL) return 0XFF;   /* ÄÚ´æÉêÇëÊ§°Ü */

    g_ov_mode = 1;
    g_jpeg_data_ok = 0;
    
    sw_ov2640_mode();               /* ÇĞ»»ÎªOV2640Ä£Ê½ */
    ov2640_jpeg_mode();             /* JPEGÄ£Ê½ */
    
    dcmi_rx_callback = jpeg_dcmi_rx_callback;   /* JPEG½ÓÊÕÊı¾İ»Øµ÷º¯Êı */
    dcmi_dma_init((uint32_t)p_dcmi_line_buf[0], (uint32_t)p_dcmi_line_buf[1], jpeg_line_size, 2, 1); /* DCMI DMAÅäÖÃ */

    ov2640_image_win_set(0, 0, 1600, 1200);
    ov2640_outsize_set(1600, 1200);  /* ÉèÖÃÊä³ö³ß´ç(1600 * 1200) */
    
    dcmi_start();                   /* Æô¶¯´«Êä */
    while (g_jpeg_data_ok != 1);    /* µÈ´ıµÚÒ»Ö¡Í¼Æ¬²É¼¯Íê */
    g_jpeg_data_ok = 2;             /* ºöÂÔ±¾Ö¡Í¼Æ¬,Æô¶¯ÏÂÒ»Ö¡²É¼¯ */
    while (g_jpeg_data_ok != 1);    /* µÈ´ıµÚ¶şÖ¡Í¼Æ¬²É¼¯Íê  */
    g_jpeg_data_ok = 2;             /* ºöÂÔ±¾Ö¡Í¼Æ¬,Æô¶¯ÏÂÒ»Ö¡²É¼¯ */
    while (g_jpeg_data_ok != 1);    /* µÈ´ıµÚÈıÖ¡Í¼Æ¬²É¼¯Íê,µÚÈıÖ¡,²Å±£´æµ½SD¿¨È¥ */

    dcmi_stop();                    /* Í£Ö¹DMA°áÔË */
    g_ov_mode = 0;
    sw_sdcard_mode();               /* ÇĞ»»ÎªSD¿¨Ä£Ê½ */
    
    printf("jpeg data size:%d\r\n", g_jpeg_data_len * 4);   /* ´®¿Ú´òÓ¡JPEGÎÄ¼ş´óĞ¡ */
    pbuf = (uint8_t *)p_jpeg_data_buf;
    jpglen = 0;                     /* ÉèÖÃjpgÎÄ¼ş´óĞ¡Îª0 */
    headok = 0;                     /* Çå³ıjpgÍ·±ê¼Ç */

    for (i = 0; i < g_jpeg_data_len * 4; i++)   /* ²éÕÒ0XFF,0XD8ºÍ0XFF,0XD9,»ñÈ¡jpgÎÄ¼ş´óĞ¡ */
    {
        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD8)) /* ÕÒµ½FF D8 */
        {
            jpgstart = i;
            headok = 1;             /* ±ê¼ÇÕÒµ½jpgÍ·(FF D8) */
        }

        if ((pbuf[i] == 0XFF) && (pbuf[i + 1] == 0XD9) && headok)   /* ÕÒµ½Í·ÒÔºó,ÔÙÕÒFF D9 */
        {
            jpglen = i - jpgstart + 2;
            break;
        }
    }

    if (jpglen)                     /* Õı³£µÄjpegÊı¾İ */
    {
        res = f_open(f_jpg, (const TCHAR *)pname, FA_WRITE | FA_CREATE_NEW);    /* Ä£Ê½0,»òÕß³¢ÊÔ´ò¿ªÊ§°Ü,Ôò´´½¨ĞÂÎÄ¼ş */

        if (res == 0)
        {
            pbuf += jpgstart;       /* Æ«ÒÆµ½0XFF,0XD8´¦ */
            
            while(datalen < jpglen) /* Ñ­»·Ğ´Èë£¡²»ÄÜÖ±½ÓĞ´Íâ²¿SRAMÊı¾İµ½SDIO£¬·ñÔò¿ÉÄÜÒıÆğFIFOÏÂÒç´íÎó */
            {
                if((jpglen - datalen) > 4096)
                {
                    datasize = 4096;
                }else
                {
                    datasize = jpglen - datalen;    /* ×îºóµÄÊı¾İ */
                }

                my_mem_copy(databuf, pbuf, datasize);
                res = f_write(f_jpg, databuf, datasize, (UINT *)&bwr); /* Ğ´ÈëÄÚÈİ */
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
    sw_ov2640_mode();       /* ÇĞ»»ÎªOV2640Ä£Ê½ */
    ov2640_rgb565_mode();   /* RGB565Ä£Ê½ */
    
    dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMAÅäÖÃ,MCUÆÁ,ÊúÆÁ */
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
        while(!SensorFlag)                    //??100ms × imax ?????
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
    uint8_t *pname;                             /* ´øÂ·¾¶µÄÎÄ¼şÃû */
    uint8_t key;                                /* ¼üÖµ */
    uint8_t i;
    uint8_t sd_ok = 1;                          /* 0,sd¿¨²»Õı³£;1,SD¿¨Õı³£ */
    uint8_t scale = 1;                          /* Ä¬ÈÏÊÇÈ«³ß´çËõ·Å */
    uint8_t msgbuf[15];                         /* ÏûÏ¢»º´æÇø */
		uint16_t *ImageData;								// initialize the picture buffer
		uint8_t person;

    sys_stm32_clock_init(336, 8, 2, 7);         /* ÉèÖÃÊ±ÖÓ,168Mhz */
    delay_init(168);                            /* ÑÓÊ±³õÊ¼»¯ */
    usart_init(84, 115200);                     /* ´®¿Ú³õÊ¼»¯Îª115200 */
    usart2_init(42,9600);
		usmart_dev.init(84);                        /* ³õÊ¼»¯USMART */
		pwmdac_init(20000-1, 83);                    /* PWM DAC³õÊ¼»¯, Fpwm = 84M / 256 = 328.125Khz */
    led_init();                                 /* ³õÊ¼»¯LED */
    lcd_init();                                 /* ³õÊ¼»¯LCD */
    key_init();                                 /* ³õÊ¼»¯°´¼ü */
    sram_init();                                /* SRAM³õÊ¼»¯ */
    norflash_init();                            /* ³õÊ¼»¯NORFLASH */
    beep_init();                                /* ³õÊ¼»¯·äÃùÆ÷ */
    btim_timx_int_init(10000 - 1, 8400 - 1);    /* 10KHz¼ÆÊıÆµÂÊ£¬1ÃëÖĞ¶ÏÒ»´Î */
    piclib_init();                              /* ³õÊ¼»¯»­Í¼ */
    SensorGPIOConfig();
		
		
    my_mem_init(SRAMIN);                        /* ³õÊ¼»¯ÄÚ²¿SRAMÄÚ´æ³Ø */
    my_mem_init(SRAMEX);                        /* ³õÊ¼»¯Íâ²¿SRAMÄÚ´æ³Ø */
    my_mem_init(SRAMCCM);                       /* ³õÊ¼»¯CCMÄÚ´æ³Ø */

    exfuns_init();                              /* ÎªfatfsÏà¹Ø±äÁ¿ÉêÇëÄÚ´æ */
    f_mount(fs[0], "0:", 1);                    /* ¹ÒÔØSD¿¨ */
    f_mount(fs[1], "1:", 1);                    /* ¹ÒÔØFLASH */

    while (fonts_init())                        /* ¼ì²é×Ö¿â */
    {
        lcd_show_string(30, 50, 200, 16, 16, "Font Error!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 240, 66, WHITE);       /* Çå³ıÏÔÊ¾ */
        delay_ms(200);
    }
			
    text_show_string(30, 50, 200, 16, "ÕıµãÔ­×ÓSTM32¿ª·¢°å", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "ÕÕÏà»úÊµÑé", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "KEY0:ÅÄÕÕ(bmp¸ñÊ½)", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY1:ÅÄÕÕ(jpg¸ñÊ½)", 16, 0, RED);
    text_show_string(30, 130, 200, 16, "WK_UP:FullSize/Scale", 16, 0, RED);
    sprintf((char*)msgbuf,"id:%x,width:%d,",lcddev.id,lcddev.width);
		lcd_show_string(30, 250, 200, 16, 16, (char *)msgbuf, RED);
		sprintf((char*)msgbuf,"height:%d",lcddev.height);
		lcd_show_string(30, 270, 200, 16, 16, (char *)msgbuf, RED);
    res = f_mkdir("0:/PHOTO");              /* ´´½¨PHOTOÎÄ¼ş¼Ğ */
    
    if (res != FR_EXIST && res != FR_OK)    /* ·¢ÉúÁË´íÎó */
    {
        res = f_mkdir("0:/PHOTO");          /* ´´½¨PHOTOÎÄ¼ş¼Ğ */
        text_show_string(30, 150, 240, 16, "SD¿¨´íÎó!", 16, 0, RED);
        delay_ms(200);
        text_show_string(30, 150, 240, 16, "ÅÄÕÕ¹¦ÄÜ½«²»¿ÉÓÃ!", 16, 0, RED);
        delay_ms(200);
        sd_ok = 0;
    }
		
		ImageData = mymalloc(SRAMIN,30*40*2);
    p_dcmi_line_buf[0] = mymalloc(SRAMIN, jpeg_line_size * 4);  /* Îªjpeg dma½ÓÊÕÉêÇëÄÚ´æ */
    p_dcmi_line_buf[1] = mymalloc(SRAMIN, jpeg_line_size * 4);  /* Îªjpeg dma½ÓÊÕÉêÇëÄÚ´æ */
    p_jpeg_data_buf = mymalloc(SRAMEX, jpeg_buf_size);          /* ÎªjpegÎÄ¼şÉêÇëÄÚ´æ */
    pname = mymalloc(SRAMIN, 30);                               /* Îª´øÂ·¾¶µÄÎÄ¼şÃû·ÖÅä30¸ö×Ö½ÚµÄÄÚ´æ */

    while (pname == NULL || !p_dcmi_line_buf[0] || !p_dcmi_line_buf[1] || !p_jpeg_data_buf) /* ÄÚ´æ·ÖÅä³ö´í */
    {
        text_show_string(30, 150, 240, 16, "ÄÚ´æ·ÖÅäÊ§°Ü!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 150, 240, 146, WHITE); /* Çå³ıÏÔÊ¾ */
        delay_ms(200);
    }

    while (ov2640_init())   /* ³õÊ¼»¯OV2640 */
    {
        text_show_string(30, 170, 240, 16, "OV2640 ´íÎó!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 150, 239, 206, WHITE);
        delay_ms(200);
    }

    delay_ms(100);
    text_show_string(30, 170, 230, 16, "OV2640 Õı³£", 16, 0, RED);
    
    ov2640_rgb565_mode();       /* RGB565Ä£Ê½ */
    dcmi_init();                /* DCMIÅäÖÃ */
    dcmi_dma_init((uint32_t)&LCD->LCD_RAM, 0, 1, 1, 0); /* DCMI DMAÅäÖÃ,MCUÆÁ,ÊúÆÁ */
    
    ov2640_outsize_set(lcddev.width, lcddev.height);    /* ÂúÆÁËõ·ÅÏÔÊ¾ */
    dcmi_start();               /* Æô¶¯´«Êä */
    lcd_clear(BLACK);
		atk_frec_initialization();
    while (1)
    {
        key = key_scan(0);
        if (key)
        {
            dcmi_stop();                /* Í£Ö¹ÏÔÊ¾ */
            if (sd_ok)             /* SD¿¨Õı³£²Å¿ÉÒÔÅÄÕÕ */
            {
                sw_sdcard_mode();       /* ÇĞ»»ÎªSD¿¨Ä£Ê½ */
								switch(key){
									case KEY0_PRES:{			//recognize a face
										frec_get_image_data(ImageData,0,0,lcddev.width,30);
										sprintf((char*)msgbuf,"ÕıÔÚÊ¶±ğ...");
										text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										res=atk_frec_recognition_face(ImageData,&person);//½øĞĞÊ¶±ğ
										if(res==ATK_FREC_MODEL_DATA_ERR)
										{	 
											sprintf((char*)msgbuf,"Ã»ÓĞ¿ÉÓÃÄ£°å,°´KEY_UPÌí¼ÓÄ£°å!");
											text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										}else if(res==ATK_FREC_UNREC_FACE_ERR)
										{	
											sprintf((char*)msgbuf,"ÎŞ·¨Ê¶±ğ¸ÃÈËÁ³,ÇëÖØÊÔ!");
											text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
											error_say();
											delay_ms(3000);
										}else 
										{
											sprintf((char*)msgbuf,"ÇóÊÇÔ°»¶Ó­Äú:ÓÃ»§%02d",person);
											text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
											open_say();
											cross();
										}
										break;
									}
									case WKUP_PRES:{			//record a face
										frec_get_image_data(ImageData,0,0,lcddev.width,30);
										res = atk_frec_add_a_face(ImageData,&person);
										sprintf((char*)msgbuf,"ÕıÔÚÂ¼Èë...");
										text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										if(res == 0){
											sprintf((char*)msgbuf,"Ìí¼Ó³É¹¦,ÓÃ»§:%02d   ",person);
											atk_frec_load_data_model();	//ÖØĞÂ¼ÓÔØËùÓĞÊ¶±ğÄ£ĞÍ(½«Ìí¼ÓµÄÈËÁ³,¼ÓÔØ½øÀ´)
										}else{
											sprintf((char*)msgbuf,"Ìí¼ÓÊ§°Ü,´íÎó´úÂë:%02d",res);
										}
										break;
									}
									case KEY1_PRES:{
										sprintf((char*)msgbuf,"ÕıÔÚÉ¾³ı...");
										text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										for(i=0;i<MAX_LEBEL_NUM;i++)
										{
											res=atk_frec_delete_data(i);//É¾³ıÄ£°å
										}
										atk_frec_load_data_model();	//ÖØĞÂ¼ÓÔØËùÓĞÊ¶±ğÄ£ĞÍ(±»É¾µôµÄ,½«ÎŞ·¨¼ÓÔØ½øÀ´.)
										sprintf((char*)msgbuf,"É¾³ıÍê³É");
										text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										delay_ms(1000);
										break;
									}
									default:break;
								}
                text_show_string(30,300,200,20,(char*)msgbuf,16,0,RED);
										delay_ms(500);
                sw_ov2640_mode();   /* ÇĞ»»ÎªOV2640Ä£Ê½ */ 
            }
            else    /* ÌáÊ¾SD¿¨´íÎó */
            {
                text_show_string(30, 130, 240, 16, "SD¿¨´íÎó!", 16, 0, RED);
                text_show_string(30, 150, 240, 16, "¹¦ÄÜ²»¿ÉÓÃ!", 16, 0, RED);
            }
            if (key != WKUP_PRES) delay_ms(1800);
            
            dcmi_start();   /* ¿ªÊ¼ÏÔÊ¾ */
        }

        delay_ms(10);
        i++;

        if (i == 20)    /* DS0ÉÁË¸ */
        {
            i = 0; 
            LED0_TOGGLE();
        }
    }
}

















