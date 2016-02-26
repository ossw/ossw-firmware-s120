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

#define SPI_OPERATING_FREQUENCY (0x02000000uL << (uint32_t)Freq_8Mbps)  /**< Slave clock frequency. */

// SPI0. 
#define SPI0_SCK     11u                                     /**< SPI clock GPIO pin number. */
#define SPI0_MOSI    12u                                     /**< SPI Master Out Slave In GPIO pin number. */
#define SPI0_MISO    9u                                      /**< SPI Master In Slave Out GPIO pin number. */
#define SPI0_SS0     8u                                      /**< SPI Slave Select GPIO pin number for device 0 */
#define SPI0_SS1     23u                                     /**< SPI Slave Select GPIO pin number for device 1 */

// SPI1. 
#define SPI1_SCK     31u   //  */    15u //dev board 
#define SPI1_MOSI    28u   //  */    16u //dev board 
#define SPI1_MISO    4u                                      
#define SPI1_SS0     30u   

#define LCD_VOLTAGE_REG   13u
#define LCD_ENABLE         6u
#define LCD_BACKLIGHT      7u

#define VIBRATION_MOTOR   29u

#define TIMEOUT_COUNTER         0x3000uL                     /**< Timeout for SPI transaction in units of loop iterations. */

extern uint32_t * p_spi0_base_address;
extern uint32_t * p_spi1_base_address;

#define EXT_FLASH_SPI p_spi0_base_address
#define EXT_FLASH_SPI_SS        SPI0_SS0
#define EXT_RAM_SPI p_spi0_base_address
#define EXT_RAM_SPI_SS          SPI0_SS1

#define MLCD_SPI p_spi1_base_address
#define MLCD_SPI_SS SPI1_SS0

#define APP_TIMER_PRESCALER              0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS             12												                  /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE          5                                          /**< Size of timer operation queues. */

#define ACCEL_INT1				 3u
#define ACCEL_INT2				 2u

#define BATTERY_LEVEL_ADC_PIN 	NRF_ADC_CONFIG_INPUT_6
#define BATTERY_CHARGING_INT_PIN			25u
#define BATTERY_FULLY_CHARGED_INT_PIN	24u

#define RX_PIN_NUMBER  15
#define TX_PIN_NUMBER  16

#endif // WATCH_BOARD_H
