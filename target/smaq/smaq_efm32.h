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
#ifndef SMAQ_EFM32_H
#define SMAQ_EFM32_H

#include <inttypes.h>
#include "em_usart.h"
#include "em_gpio.h"

/*
#define LEDS_NUMBER    0

#define LEDS_LIST { }

#define LEDS_MASK      0

#define LEDS_INV_MASK  LEDS_MASK
*/

#define BUTTONS_NUMBER 4

#define BUTTON_UP      (gpioPortC<<4 | 10)
#define BUTTON_SELECT  (gpioPortF<<4 | 12)
#define BUTTON_DOWN    (gpioPortA<<4 | 1)
#define BUTTON_BACK    (gpioPortA<<4 | 6)

#define SPI_OPERATING_FREQUENCY (0x02000000uL << (uint32_t)Freq_8Mbps)  /**< Slave clock frequency. */

#define LCD_VOLTAGE_REG   (gpioPortD<<4 | 6)
#define LCD_ENABLE         (gpioPortD<<4 | 6)
#define LCD_BACKLIGHT      (gpioPortA<<4 | 10)

#define VIBRATION_MOTOR   (gpioPortA<<4 | 8)

#define TIMEOUT_COUNTER         0x3000uL                     /**< Timeout for SPI transaction in units of loop iterations. */

#define EXT_FLASH_SPI USART2
#define EXT_FLASH_SPI_SS        (gpioPortA<<4 | 5)

#define MLCD_SPI USART1
#define MLCD_SPI_SS (gpioPortD<<4 | 3)

//#define ACCEL_INT1				 3u
//#define ACCEL_INT2				 2u

//#define BATTERY_LEVEL_ADC_PIN 	NRF_ADC_CONFIG_INPUT_6
//#define BATTERY_CHARGING_INT_PIN			25u
//#define BATTERY_FULLY_CHARGED_INT_PIN	24u

//#define RX_PIN_NUMBER  15
//#define TX_PIN_NUMBER  16

#endif // SMAQ_EFM32_H
