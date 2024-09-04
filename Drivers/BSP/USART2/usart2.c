/**
 ****************************************************************************************************
 * @file        usart2.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-13
 * @brief       串口初始化代码(串口2)
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20220113
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./BSP/USART2/usart2.h"


#if USART2_EN_RX     /* 如果使能了接收 */

/* 接收缓冲, 最大USART_REC_LEN个字节. */
uint8_t g_usart2_rx_buf[USART2_REC_LEN];

/*  接收状态
 *  bit15，      接收完成标志
 *  bit14，      接收到0x0d
 *  bit13~0，    接收到的有效字节数目
*/
uint16_t g_usart2_rx_sta = 0;

/**
 * @brief       串口X中断服务函数
 * @param       无
 * @retval      无
 */
void USART2_UX_IRQHandler(void)
{
    uint8_t rxdata;
#if SYS_SUPPORT_OS  /* 如果SYS_SUPPORT_OS为真，则需要支持OS. */
    OSIntEnter();
#endif

    if (USART2_UX->SR & (1 << 5))                /* 接收到数据 */
    {
        rxdata = USART2_UX->DR;

        if ((g_usart2_rx_sta & 0x8000) == 0)     /* 接收未完成? */
        {
            if (g_usart2_rx_sta & 0x4000)        /* 接收到了0x0d? */
            {
                if (rxdata != 0x0a)             /* 接收到了0x0a? (必须先接收到到0x0d,才检查0x0a) */
                {
                    g_usart2_rx_sta = 0;         /* 接收错误, 重新开始 */
                }
                else
                {
                    g_usart2_rx_sta |= 0x8000;   /* 收到了0x0a,标记接收完成了 */
                }
            }
            else      /* 还没收到0x0d */
            {
                if (rxdata == 0x0d)
                {
                    g_usart2_rx_sta |= 0x4000;   /* 标记接收到了 0x0d */
                }
                else
                {
                    g_usart2_rx_buf[g_usart2_rx_sta & 0X3FFF] = rxdata;   /* 存储数据到 g_usart_rx_buf */
                    g_usart2_rx_sta++;

                    if (g_usart2_rx_sta > (USART2_REC_LEN - 1))g_usart2_rx_sta = 0;/* 接收数据溢出, 重新开始接收 */
                }
            }
        }
    }

#if SYS_SUPPORT_OS  /* 如果SYS_SUPPORT_OS为真，则需要支持OS. */
    OSIntExit();
#endif
}
#endif


/**
 * @brief       串口2初始化函数
 * @param       sclk: 串口X的时钟源频率(单位: MHz)
 *              串口1 和 串口6 的时钟源来自: rcc_pclk2 = 84Mhz
 *              串口2 - 5 / 7 / 8 的时钟源来自: rcc_pclk1 = 42Mhz
 * @note        注意: 必须设置正确的sclk, 否则串口波特率就会设置异常.
 * @param       baudrate: 波特率, 根据自己需要设置波特率值
 * @retval      无
 */
void usart2_init(uint32_t sclk, uint32_t baudrate)
{
    uint32_t temp;
    /* IO 及 时钟配置 */
    USART2_TX_GPIO_CLK_ENABLE();    /* 使能串口TX脚时钟 */
    USART2_RX_GPIO_CLK_ENABLE();    /* 使能串口RX脚时钟 */
    USART2_UX_CLK_ENABLE();         /* 使能串口时钟 */

    sys_gpio_set(USART2_TX_GPIO_PORT, USART2_TX_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);    /* 串口TX脚 模式设置 */

    sys_gpio_set(USART2_RX_GPIO_PORT, USART2_RX_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);    /* 串口RX脚 模式设置 */

    sys_gpio_af_set(GPIOA, USART2_TX_GPIO_PIN, USART2_TX_GPIO_AF);  /* TX脚 复用功能选择, 必须设置正确 */
    sys_gpio_af_set(GPIOA, USART2_RX_GPIO_PIN, USART2_RX_GPIO_AF);  /* RX脚 复用功能选择, 必须设置正确 */

    temp = (sclk * 1000000 + baudrate / 2) / baudrate;              /* 得到USARTDIV@OVER8 = 0, 采用四舍五入计算 */
    /* 波特率设置 */
    USART2_UX->BRR = temp;      /* 波特率设置@OVER8 = 0 */
    USART2_UX->CR1 = 0;         /* 清零CR1寄存器 */
    USART2_UX->CR1 |= 0 << 12;  /* 设置M = 0, 选择8位字长 */
    USART2_UX->CR1 |= 0 << 15;  /* 设置OVER8 = 0, 16倍过采样 */
    USART2_UX->CR1 |= 1 << 3;   /* 串口发送使能 */

    USART2_UX->CR1 |= 1 << 13;  /* 串口使能 */
}

void USART2_SendData(uint8_t data)
{
	while((USART2->SR & 0X40) == 0);
	USART2->DR = data;
}

void USART2_SendString(uint8_t *DAT, uint8_t len)
{
	uint8_t i;
	for(i = 0; i < len; i++)
	{
		USART2_SendData(*DAT++);
	}
}





