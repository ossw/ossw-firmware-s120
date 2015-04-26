/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef WATCH_BOARD_H
#define WATCH_BOARD_H
/*
#define LEDS_NUMBER    0

#define LEDS_LIST { }

#define LEDS_MASK      0

#define LEDS_INV_MASK  LEDS_MASK
*/

#define BUTTONS_NUMBER 4

#define BUTTON_UP      20
#define BUTTON_SELECT  18
#define BUTTON_DOWN    19
#define BUTTON_BACK    17
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP
/*
#define BUTTONS_LIST { BUTTON_UP, BUTTON_SELECT, BUTTON_DOWN, BUTTON_BACK }

#define BSP_BUTTON_0   BUTTON_UP
#define BSP_BUTTON_1   BUTTON_SELECT
#define BSP_BUTTON_2   BUTTON_DOWN
#define BSP_BUTTON_3   BUTTON_PULL

#define BSP_BUTTON_0_MASK (1<<BSP_BUTTON_0)
#define BSP_BUTTON_1_MASK (1<<BSP_BUTTON_1)
#define BSP_BUTTON_2_MASK (1<<BSP_BUTTON_2)
#define BSP_BUTTON_3_MASK (1<<BSP_BUTTON_3)
*/
#define SPI_OPERATING_FREQUENCY (0x02000000uL << (uint32_t)Freq_8Mbps)  /**< Slave clock frequency. */

// SPI0. 
#define SPI0_SCK     11u                                     /**< SPI clock GPIO pin number. */
#define SPI0_MOSI    12u                                     /**< SPI Master Out Slave In GPIO pin number. */
#define SPI0_MISO    9u                                      /**< SPI Master In Slave Out GPIO pin number. */
#define SPI0_SS0     8u                                      /**< SPI Slave Select GPIO pin number for device 0 */
#define SPI0_SS1     23u                                     /**< SPI Slave Select GPIO pin number for device 1 */

// SPI1. 
#define SPI1_SCK    /* 31u   // */     15u //dev board 
#define SPI1_MOSI   /* 28u   // */     16u //dev board 
#define SPI1_MISO    4u                                      
#define SPI1_SS0     30u   

#define LCD_VOLTAGE_REG   13u
#define LCD_ENABLE       6u
#define LCD_BACKLIGHT    7u

#define TIMEOUT_COUNTER         0x3000uL                     /**< Timeout for SPI transaction in units of loop iterations. */

extern uint32_t * p_spi0_base_address;
extern uint32_t * p_spi1_base_address;

#define EXT_FLASH_SPI p_spi0_base_address
#define EXT_FLASH_SPI_SS        SPI0_SS0
#define EXT_RAM_SPI p_spi0_base_address
#define EXT_RAM_SPI_SS          SPI0_SS1

#define MLCD_SPI p_spi1_base_address
#define MLCD_SPI_SS SPI1_SS0

//#define BUTTONS_MASK   0x001E0000
/*
#define RX_PIN_NUMBER  11
#define TX_PIN_NUMBER  9
#define CTS_PIN_NUMBER 10
#define RTS_PIN_NUMBER 8
#define HWFC           true
*/

/*
// serialization APPLICATION board
#define SER_APP_RX_PIN              12    // UART RX pin number.
#define SER_APP_TX_PIN              13    // UART TX pin number.
#define SER_APP_CTS_PIN             15    // UART Clear To Send pin number.
#define SER_APP_RTS_PIN             14    // UART Request To Send pin number.

#define SER_APP_SPIM0_SCK_PIN       29     // SPI clock GPIO pin number.
#define SER_APP_SPIM0_MOSI_PIN      25     // SPI Master Out Slave In GPIO pin number
#define SER_APP_SPIM0_MISO_PIN      28     // SPI Master In Slave Out GPIO pin number
#define SER_APP_SPIM0_SS_PIN        12     // SPI Slave Select GPIO pin number
#define SER_APP_SPIM0_RDY_PIN       14     // SPI READY GPIO pin number
#define SER_APP_SPIM0_REQ_PIN       13     // SPI REQUEST GPIO pin number

// serialization CONNECTIVITY board
#define SER_CON_RX_PIN              13    // UART RX pin number.
#define SER_CON_TX_PIN              12    // UART TX pin number.
#define SER_CON_CTS_PIN             14    // UART Clear To Send pin number. Not used if HWFC is set to false.
#define SER_CON_RTS_PIN             15    // UART Request To Send pin number. Not used if HWFC is set to false.


#define SER_CON_SPIS_SCK_PIN        29    // SPI SCK signal.
#define SER_CON_SPIS_MOSI_PIN       25    // SPI MOSI signal.
#define SER_CON_SPIS_MISO_PIN       28    // SPI MISO signal.
#define SER_CON_SPIS_CSN_PIN        12    // SPI CSN signal.
#define SER_CON_SPIS_RDY_PIN        14    // SPI READY GPIO pin number.
#define SER_CON_SPIS_REQ_PIN        13    // SPI REQUEST GPIO pin number.
*/
#endif // WATCH_BOARD_H
