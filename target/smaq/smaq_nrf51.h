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
#ifndef SMAQ_NRF51_H
#define SMAQ_NRF51_H

// SPI0. 
#define SPIS0_SCK     3u
#define SPIS0_MOSI    30u
#define SPIS0_MISO    0u
#define SPIS0_SS      2u

#define MASTER_INT 		29u

#define APP_TIMER_PRESCALER              0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS             12												                  /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE          4                                          /**< Size of timer operation queues. */

#define CENTRAL_LINK_COUNT              0                                          /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                          /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#endif // SMAQ_NRF51_H
