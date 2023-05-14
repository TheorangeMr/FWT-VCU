#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "stm32f10x.h"

#define DEBUG_USARTx                      USART1
#define DEBUG_USART_CLK                   RCC_APB2Periph_USART1
#define DEBUG_USART_GPIO_CLK              RCC_APB2Periph_GPIOA
#define DEBUG_USART_APBxClkCmd            RCC_APB2PeriphClockCmd
#define DEBUG_USART_GPIO_APBxClkCmd       RCC_APB2PeriphClockCmd
#define DEBUG_USART_TX_GPIO_PORT          GPIOA
#define DEBUG_USART_TX_GPIO_PIN           GPIO_Pin_9

#define DEBUG_USART_RX_GPIO_PORT          GPIOA
#define DEBUG_USART_RX_GPIO_PIN           GPIO_Pin_10

#define DEBUG_USART_IRQ                   USART1_IRQn
#define DEBUG_USART_IRQHandler            USART1_IRQHandler

/* ´®¿ÚÍ¨Ñ¶ÃØÔ¿ */

#define USART_ON                            0x1a
#define USART_OFF                           0x2a



void uart_init(u32 bound);

void usart1_send_data(u8 *data, u32 size);

#endif


