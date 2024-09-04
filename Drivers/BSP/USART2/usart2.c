/**
 ****************************************************************************************************
 * @file        usart2.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-13
 * @brief       ���ڳ�ʼ������(����2)
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

#include "./BSP/USART2/usart2.h"


#if USART2_EN_RX     /* ���ʹ���˽��� */

/* ���ջ���, ���USART_REC_LEN���ֽ�. */
uint8_t g_usart2_rx_buf[USART2_REC_LEN];

/*  ����״̬
 *  bit15��      ������ɱ�־
 *  bit14��      ���յ�0x0d
 *  bit13~0��    ���յ�����Ч�ֽ���Ŀ
*/
uint16_t g_usart2_rx_sta = 0;

/**
 * @brief       ����X�жϷ�����
 * @param       ��
 * @retval      ��
 */
void USART2_UX_IRQHandler(void)
{
    uint8_t rxdata;
#if SYS_SUPPORT_OS  /* ���SYS_SUPPORT_OSΪ�棬����Ҫ֧��OS. */
    OSIntEnter();
#endif

    if (USART2_UX->SR & (1 << 5))                /* ���յ����� */
    {
        rxdata = USART2_UX->DR;

        if ((g_usart2_rx_sta & 0x8000) == 0)     /* ����δ���? */
        {
            if (g_usart2_rx_sta & 0x4000)        /* ���յ���0x0d? */
            {
                if (rxdata != 0x0a)             /* ���յ���0x0a? (�����Ƚ��յ���0x0d,�ż��0x0a) */
                {
                    g_usart2_rx_sta = 0;         /* ���մ���, ���¿�ʼ */
                }
                else
                {
                    g_usart2_rx_sta |= 0x8000;   /* �յ���0x0a,��ǽ�������� */
                }
            }
            else      /* ��û�յ�0x0d */
            {
                if (rxdata == 0x0d)
                {
                    g_usart2_rx_sta |= 0x4000;   /* ��ǽ��յ��� 0x0d */
                }
                else
                {
                    g_usart2_rx_buf[g_usart2_rx_sta & 0X3FFF] = rxdata;   /* �洢���ݵ� g_usart_rx_buf */
                    g_usart2_rx_sta++;

                    if (g_usart2_rx_sta > (USART2_REC_LEN - 1))g_usart2_rx_sta = 0;/* �����������, ���¿�ʼ���� */
                }
            }
        }
    }

#if SYS_SUPPORT_OS  /* ���SYS_SUPPORT_OSΪ�棬����Ҫ֧��OS. */
    OSIntExit();
#endif
}
#endif


/**
 * @brief       ����2��ʼ������
 * @param       sclk: ����X��ʱ��ԴƵ��(��λ: MHz)
 *              ����1 �� ����6 ��ʱ��Դ����: rcc_pclk2 = 84Mhz
 *              ����2 - 5 / 7 / 8 ��ʱ��Դ����: rcc_pclk1 = 42Mhz
 * @note        ע��: ����������ȷ��sclk, ���򴮿ڲ����ʾͻ������쳣.
 * @param       baudrate: ������, �����Լ���Ҫ���ò�����ֵ
 * @retval      ��
 */
void usart2_init(uint32_t sclk, uint32_t baudrate)
{
    uint32_t temp;
    /* IO �� ʱ������ */
    USART2_TX_GPIO_CLK_ENABLE();    /* ʹ�ܴ���TX��ʱ�� */
    USART2_RX_GPIO_CLK_ENABLE();    /* ʹ�ܴ���RX��ʱ�� */
    USART2_UX_CLK_ENABLE();         /* ʹ�ܴ���ʱ�� */

    sys_gpio_set(USART2_TX_GPIO_PORT, USART2_TX_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);    /* ����TX�� ģʽ���� */

    sys_gpio_set(USART2_RX_GPIO_PORT, USART2_RX_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);    /* ����RX�� ģʽ���� */

    sys_gpio_af_set(GPIOA, USART2_TX_GPIO_PIN, USART2_TX_GPIO_AF);  /* TX�� ���ù���ѡ��, ����������ȷ */
    sys_gpio_af_set(GPIOA, USART2_RX_GPIO_PIN, USART2_RX_GPIO_AF);  /* RX�� ���ù���ѡ��, ����������ȷ */

    temp = (sclk * 1000000 + baudrate / 2) / baudrate;              /* �õ�USARTDIV@OVER8 = 0, ��������������� */
    /* ���������� */
    USART2_UX->BRR = temp;      /* ����������@OVER8 = 0 */
    USART2_UX->CR1 = 0;         /* ����CR1�Ĵ��� */
    USART2_UX->CR1 |= 0 << 12;  /* ����M = 0, ѡ��8λ�ֳ� */
    USART2_UX->CR1 |= 0 << 15;  /* ����OVER8 = 0, 16�������� */
    USART2_UX->CR1 |= 1 << 3;   /* ���ڷ���ʹ�� */

    USART2_UX->CR1 |= 1 << 13;  /* ����ʹ�� */
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





