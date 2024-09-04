/**
 ****************************************************************************************************
 * @file        key.c
 * @author      ÕıµãÔ­×ÓÍÅ¶Ó(ALIENTEK)
 * @version     V1.0
 * @date        2021-12-30
 * @brief       °´¼üÊäÈë Çı¶¯´úÂë
 * @license     Copyright (c) 2020-2032, ¹ãÖİÊĞĞÇÒíµç×Ó¿Æ¼¼ÓĞÏŞ¹«Ë¾
 ****************************************************************************************************
 * @attention
 *
 * ÊµÑéÆ½Ì¨:ÕıµãÔ­×Ó STM32F407¿ª·¢°å
 * ÔÚÏßÊÓÆµ:www.yuanzige.com
 * ¼¼ÊõÂÛÌ³:www.openedv.com
 * ¹«Ë¾ÍøÖ·:www.alientek.com
 * ¹ºÂòµØÖ·:openedv.taobao.com
 *
 * ĞŞ¸ÄËµÃ÷
 * V1.0 20211230
 * µÚÒ»´Î·¢²¼
 *
 ****************************************************************************************************
 */

#include "./BSP/KEY/key.h"
#include "./SYSTEM/delay/delay.h"


/**
 * @brief       °´¼ü³õÊ¼»¯º¯Êı
 * @param       ÎŞ
 * @retval      ÎŞ
 */
void key_init(void)
{
    KEY0_GPIO_CLK_ENABLE(); /* KEY0Ê±ÖÓÊ¹ÄÜ */
    KEY1_GPIO_CLK_ENABLE(); /* KEY1Ê±ÖÓÊ¹ÄÜ */
    KEY2_GPIO_CLK_ENABLE(); /* KEY2Ê±ÖÓÊ¹ÄÜ */
    WKUP_GPIO_CLK_ENABLE(); /* WKUPÊ±ÖÓÊ¹ÄÜ */

    sys_gpio_set(KEY0_GPIO_PORT, KEY0_GPIO_PIN,
                 SYS_GPIO_MODE_IN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PD);    /* KEY0Òı½ÅÄ£Ê½ÉèÖÃ,downÏÀ­ÊäÈë */

    sys_gpio_set(KEY1_GPIO_PORT, KEY1_GPIO_PIN,
                 SYS_GPIO_MODE_IN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);    /* KEY1Òı½ÅÄ£Ê½ÉèÖÃ,ÉÏÀ­ÊäÈë */

    sys_gpio_set(KEY2_GPIO_PORT, KEY2_GPIO_PIN,
             SYS_GPIO_MODE_IN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);        /* KEY2Òı½ÅÄ£Ê½ÉèÖÃ,ÉÏÀ­ÊäÈë */

    sys_gpio_set(WKUP_GPIO_PORT, WKUP_GPIO_PIN,
                 SYS_GPIO_MODE_IN, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PD);    /* WKUPÒı½ÅÄ£Ê½ÉèÖÃ,ÏÂÀ­ÊäÈë */

}

/**
 * @brief       °´¼üÉ¨Ãèº¯Êı
 * @note        ¸Ãº¯ÊıÓĞÏìÓ¦ÓÅÏÈ¼¶(Í¬Ê±°´ÏÂ¶à¸ö°´¼ü): WK_UP > KEY2 > KEY1 > KEY0!!
 * @param       mode:0 / 1, ¾ßÌåº¬ÒåÈçÏÂ:
 *   @arg       0,  ²»Ö§³ÖÁ¬Ğø°´(µ±°´¼ü°´ÏÂ²»·ÅÊ±, Ö»ÓĞµÚÒ»´Îµ÷ÓÃ»á·µ»Ø¼üÖµ,
 *                  ±ØĞëËÉ¿ªÒÔºó, ÔÙ´Î°´ÏÂ²Å»á·µ»ØÆäËû¼üÖµ)
 *   @arg       1,  Ö§³ÖÁ¬Ğø°´(µ±°´¼ü°´ÏÂ²»·ÅÊ±, Ã¿´Îµ÷ÓÃ¸Ãº¯Êı¶¼»á·µ»Ø¼üÖµ)
 * @retval      ¼üÖµ, ¶¨ÒåÈçÏÂ:
 *              KEY0_PRES, 1, KEY0°´ÏÂ
 *              KEY1_PRES, 2, KEY1°´ÏÂ
 *              KEY2_PRES, 3, KEY2°´ÏÂ
 *              WKUP_PRES, 4, WKUP°´ÏÂ
 */
uint8_t key_scan(uint8_t mode)
{
    static uint8_t key_up = 1;  /* °´¼ü°´ËÉ¿ª±êÖ¾ */
    uint8_t keyval = 0;

    if (mode) key_up = 1;       /* Ö§³ÖÁ¬°´ */

    if (key_up && (KEY0 == 1 || KEY1 == 0 || KEY2 == 0 || WK_UP == 1))  /* °´¼üËÉ¿ª±êÖ¾Îª1, ÇÒÓĞÈÎÒâÒ»¸ö°´¼ü°´ÏÂÁË */
    {
        delay_ms(10);           /* È¥¶¶¶¯ */
        key_up = 0;

        if (KEY0 == 1)  keyval = KEY0_PRES;

        if (KEY1 == 0)  keyval = KEY1_PRES;

        if (KEY2 == 0)  keyval = KEY2_PRES;

        if (WK_UP == 1) keyval = WKUP_PRES;
    }
    else if (KEY0 == 0 && KEY1 == 1 && KEY2 == 1 && WK_UP == 0)         /* Ã»ÓĞÈÎºÎ°´¼ü°´ÏÂ, ±ê¼Ç°´¼üËÉ¿ª */
    {
        key_up = 1;
    }

    return keyval;              /* ·µ»Ø¼üÖµ */
}




















