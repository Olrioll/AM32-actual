///*
// * serial_telemetry.c
// *
// *  Created on: May 13, 2020
// *      Author: Alka
// */

#include "serial_telemetry.h"
#include "common.h"
#include "kiss_telemetry.h"

void send_telem_DMA(uint8_t bytes)
{
#ifdef USE_PA14_TELEMETRY
    /* blocking TX on USART2 to avoid DMA channel conflict */
    for (uint8_t i = 0; i < bytes; ++i) {
        while (usart_flag_get(USART2, USART_TDBE_FLAG) == RESET) {
        }
        usart_data_transmit(USART2, aTxBuffer[i]);
    }
#else
    /* set data length and enable channel to start transfer for USART1 */
    DMA1_CHANNEL2->ctrl_bit.chen = FALSE;
    DMA1_CHANNEL2->dtcnt = bytes;
    DMA1_CHANNEL2->ctrl_bit.chen = TRUE;
#endif
}

void telem_UART_Init(void)
{
    gpio_init_type gpio_init_struct;

#ifdef USE_PA14_TELEMETRY
    /* initialize USART2 on PA14 for telemetry (no DMA) */
    crm_periph_clock_enable(CRM_USART2_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

    /* configure USART2 TX pin (PA14) */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_14;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(GPIOA, &gpio_init_struct);
    gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE14, GPIO_MUX_1);

    /* configure USART2 */
    usart_init(USART2, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART2, TRUE);
    usart_receiver_enable(USART2, TRUE);
    usart_single_line_halfduplex_select(USART2, TRUE);
    usart_enable(USART2, TRUE);
#else
    crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);

    /* configure USART1 TX pin (PB6) */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_6;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init(GPIOB, &gpio_init_struct);
    gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE6, GPIO_MUX_0);

    dma_reset(DMA1_CHANNEL2);

    dma_init_type dma_init_struct;
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = sizeof(aTxBuffer);
    dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_base_addr = (uint32_t)aTxBuffer;
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.memory_inc_enable = TRUE;
    dma_init_struct.peripheral_base_addr = (uint32_t)&USART1->dt;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = DMA_PRIORITY_LOW;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(DMA1_CHANNEL2, &dma_init_struct);

    /* configure USART1 */
    usart_init(USART1, 115200, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_transmitter_enable(USART1, TRUE);
    usart_receiver_enable(USART1, TRUE);
    usart_single_line_halfduplex_select(USART1, TRUE);
    usart_dma_transmitter_enable(USART1, TRUE);
    usart_enable(USART1, TRUE);

#endif
}
