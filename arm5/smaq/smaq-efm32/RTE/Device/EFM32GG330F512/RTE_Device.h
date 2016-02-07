/* -----------------------------------------------------------------------------
 * Copyright (C) 2013 ARM Limited. All rights reserved.
 *  
 * $Date:        27. June 2013
 * $Revision:    V1.00
 *  
 * Project:      RTE Device Configuration for Energy Micro EFM32GGxxx
 * -------------------------------------------------------------------------- */

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

#ifndef __RTE_DEVICE_H
#define __RTE_DEVICE_H

#define GPIO_PORT(num) \
 ((num == 0) ? gpioPortA : \
  (num == 1) ? gpioPortB : \
  (num == 2) ? gpioPortC : \
  (num == 3) ? gpioPortD : \
  (num == 4) ? gpioPortE : \
               gpioPortF)


// <e> DMA (DMA Controller)
#define RTE_DMA                         0

//  <o> Channel Protection Control <0=>Non-privileged <1=>Privileged
//  <i> Control whether accesses done by the DMA controller are privileged or not
#define RTE_DMA_CHPROT                  0

//  <e> DMA Channel 0
//  <i> Enable DMA Channel 0
#define RTE_DMA_CH0                     0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH0_DATA_STRUCT         0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH0_PRIORITY            0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH0_ARBI                0
//  </e>

//  <e> DMA Channel 1
//  <i> Enable DMA Channel 1
#define RTE_DMA_CH1                     0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH1_DATA_STRUCT         0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH1_PRIORITY            0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH1_ARBI                0
//  </e>

//  <e> DMA Channel 2
//  <i> Enable DMA Channel 2
#define RTE_DMA_CH2                     0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH2_DATA_STRUCT         0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH2_PRIORITY            0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH2_ARBI                0
//  </e>

//  <e> DMA Channel 3
//  <i> Enable DMA Channel 3
#define RTE_DMA_CH3                     0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH3_DATA_STRUCT         0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH3_PRIORITY            0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH3_ARBI                0
//  </e>

//  <e> DMA Channel 3
//  <i> Enable DMA Channel 3
#define RTE_DMA_CH3                     0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH3_DATA_STRUCT         0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH3_PRIORITY            0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH3_ARBI                0
//  </e>

//  <e> DMA Channel 4
//  <i> Enable DMA Channel 4
#define RTE_DMA_CH4                     0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH4_DATA_STRUCT         0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH4_PRIORITY            0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH4_ARBI                0
//  </e>

//  <e> DMA Channel 5
//  <i> Enable DMA Channel 5
#define RTE_DMA_CH5                     0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH5_DATA_STRUCT         0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH5_PRIORITY            0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH5_ARBI                0
//  </e>

//  <e> DMA Channel 6
//  <i> Enable DMA Channel 6
#define RTE_DMA_CH6                     0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH6_DATA_STRUCT         0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH6_PRIORITY            0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH6_ARBI                0
//  </e>

//  <e> DMA Channel 7
//  <i> Enable DMA Channel 7
#define RTE_DMA_CH7                     0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH7_DATA_STRUCT         0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH7_PRIORITY            0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH7_ARBI                0
//  </e>

//  <e> DMA Channel 8
//  <i> Enable DMA Channel 8
#define RTE_DMA_CH8                     0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH8_DATA_STRUCT         0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH8_PRIORITY            0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH8_ARBI                0
//  </e>

//  <e> DMA Channel 9
//  <i> Enable DMA Channel 9
#define RTE_DMA_CH9                     0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH9_DATA_STRUCT         0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH9_PRIORITY            0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH9_ARBI                0
//  </e>

//  <e> DMA Channel 10
//  <i> Enable DMA Channel 10
#define RTE_DMA_CH10                    0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH10_DATA_STRUCT        0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH10_PRIORITY           0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH10_ARBI               0
//  </e>

//  <e> DMA Channel 11
//  <i> Enable DMA Channel 11
#define RTE_DMA_CH11                    0
//    <o> Data structure <0=>Primary  <1=>Primary + Alternate
//    <i> Select channel data structure used
#define RTE_DMA_CH11_DATA_STRUCT        0
//    <o> Priority <0=>Default  <1=>High
//    <i> Select channel priority
#define RTE_DMA_CH11_PRIORITY           0
//    <o> Arbitration interval <0=>1  <1=>2  <2=>4  <3=>8  <4=>16  <5=>32
//                             <6=>64  <7=>128  <8=>256  <9=>512  <10=>1024
//    <i> Configure how many AHB bus transfers occur before DMA controller rearbitrates.
//    <i> This enables you to reduce the latency to service a higher priority channel.
#define RTE_DMA_CH11_ARBI               0
//  </e>

// </e>


// <e> UART0 (Universal asynchronous receiver transmitter) [Driver_UART0]
// <i> Configuration settings for for Driver_UART0 in component ::Drivers:UART

#define RTE_UART0                       0

//   <o> UART0 Pins Location <0=>U0_x#0 <1=>U0_x#1 <2=>U0_x#2
#define RTE_UART0_PIN_LOC               0

#if    (RTE_UART0_PIN_LOC == 0)
#define RTE_UART0_TX_PORT               gpioPortF
#define RTE_UART0_TX_BIT                6
#define RTE_UART0_RX_PORT               gpioPortF
#define RTE_UART0_RX_BIT                7
#elif  (RTE_UART0_PIN_LOC == 1)     
#define RTE_UART0_TX_PORT               gpioPortE
#define RTE_UART0_TX_BIT                0
#define RTE_UART0_RX_PORT               gpioPortE
#define RTE_UART0_RX_BIT                1
#elif  (RTE_UART0_PIN_LOC == 2)     
#define RTE_UART0_TX_PORT               gpioPortA
#define RTE_UART0_TX_BIT                3
#define RTE_UART0_RX_PORT               gpioPortA
#define RTE_UART0_RX_BIT                4
#else
#error "Invalid UART0 Pins Location!"
#endif

//   <e> DMA Rx
//     <o1> Channel <0-11>
//     <i>  Selects DMA Channel (channel 0 to 11 can be used)
//   </e>
#define RTE_UART0_RX_DMA                0
#define RTE_UART0_RX_DMA_CHANNEL        0

//   <e> DMA Tx
//     <o1> Channel <0-11>
//     <i>  Selects DMA Channel (channel 0 to 11 can be used)
//   </e>
#define RTE_UART0_TX_DMA                0
#define RTE_UART0_TX_DMA_CHANNEL        1

// </e>

// <e> UART1 (Universal asynchronous receiver transmitter) [Driver_UART1]
// <i> Configuration settings for for Driver_UART1 in component ::Drivers:UART

#define RTE_UART1                       0

//   <o> UART1 Pins Location <1=>U1_x#1 <2=>U1_x#2 <3=>U1_x#3
#define RTE_UART1_PIN_LOC               1

#if    (RTE_UART1_PIN_LOC == 1)
#define RTE_UART1_TX_PORT               gpioPortF
#define RTE_UART1_TX_BIT                10
#define RTE_UART1_RX_PORT               gpioPortF
#define RTE_UART1_RX_BIT                11
#elif  (RTE_UART1_PIN_LOC == 2)     
#define RTE_UART1_TX_PORT               gpioPortB
#define RTE_UART1_TX_BIT                9
#define RTE_UART1_RX_PORT               gpioPortB
#define RTE_UART1_RX_BIT                10
#elif  (RTE_UART1_PIN_LOC == 3)     
#define RTE_UART1_TX_PORT               gpioPortE
#define RTE_UART1_TX_BIT                2
#define RTE_UART1_RX_PORT               gpioPortE
#define RTE_UART1_RX_BIT                3
#else
#error "Invalid UART1 Pins Location!"
#endif

//   <e> DMA Rx
//     <o1> Channel <0-11>
//     <i>  Selects DMA Channel (channel 0 to 11 can be used)
//   </e>
#define RTE_UART1_RX_DMA                0
#define RTE_UART1_RX_DMA_CHANNEL        0

//   <e> DMA Tx
//     <o1> Channel <0-11>
//     <i>  Selects DMA Channel (channel 0 to 11 can be used)
//   </e>
#define RTE_UART1_TX_DMA                0
#define RTE_UART1_TX_DMA_CHANNEL        1

// </e>


// <e> USART0 (Universal synchronous asynchronous receiver transmitter) [Driver_SPI0 or Driver_USART0]
// <i> Configuration settings for Driver_SPI0 in component ::Drivers:SPI when in SPI mode otherwise for Driver_USART0 in component ::Drivers:USART
#define RTE_USART0                      0

//   <o> USART0 Pins Location <0=>US0_x#0 <1=>US0_x#1 <2=>US0_x#2 <3=>US0_x#3 <4=>US0_x#4 <5=>US0_x#5
#define RTE_USART0_PIN_LOC              1

#if    (RTE_USART0_PIN_LOC == 0)
#define RTE_USART0_TX_PORT              gpioPortE
#define RTE_USART0_TX_BIT               10
#define RTE_USART0_RX_PORT              gpioPortE
#define RTE_USART0_RX_BIT               11
#define RTE_USART0_CLK_PORT             gpioPortE
#define RTE_USART0_CLK_BIT              12
#define RTE_USART0_CS_PORT              gpioPortE
#define RTE_USART0_CS_BIT               13
#elif  (RTE_USART0_PIN_LOC == 1)
#define RTE_USART0_TX_PORT              gpioPortE
#define RTE_USART0_TX_BIT               7
#define RTE_USART0_RX_PORT              gpioPortE
#define RTE_USART0_RX_BIT               6
#define RTE_USART0_CLK_PORT             gpioPortE
#define RTE_USART0_CLK_BIT              5
#define RTE_USART0_CS_PORT              gpioPortE
#define RTE_USART0_CS_BIT               4
#elif  (RTE_USART0_PIN_LOC == 2)
#define RTE_USART0_TX_PORT              gpioPortC
#define RTE_USART0_TX_BIT               11
#define RTE_USART0_RX_PORT              gpioPortC
#define RTE_USART0_RX_BIT               10
#define RTE_USART0_CLK_PORT             gpioPortC
#define RTE_USART0_CLK_BIT              9
#define RTE_USART0_CS_PORT              gpioPortC
#define RTE_USART0_CS_BIT               8
#elif  (RTE_USART0_PIN_LOC == 3)
#define RTE_USART0_TX_PORT              gpioPortE
#define RTE_USART0_TX_BIT               13
#define RTE_USART0_RX_PORT              gpioPortE
#define RTE_USART0_RX_BIT               12
#elif  (RTE_USART0_PIN_LOC == 4)
#define RTE_USART0_TX_PORT              gpioPortB
#define RTE_USART0_TX_BIT               7
#define RTE_USART0_RX_PORT              gpioPortB
#define RTE_USART0_RX_BIT               8
#define RTE_USART0_CLK_PORT             gpioPortB
#define RTE_USART0_CLK_BIT              13
#define RTE_USART0_CS_PORT              gpioPortB
#define RTE_USART0_CS_BIT               14
#elif  (RTE_USART0_PIN_LOC == 5)
#define RTE_USART0_TX_PORT              gpioPortC
#define RTE_USART0_TX_BIT               0
#define RTE_USART0_RX_PORT              gpioPortC
#define RTE_USART0_RX_BIT               1
#define RTE_USART0_CLK_PORT             gpioPortB
#define RTE_USART0_CLK_BIT              13
#define RTE_USART0_CS_PORT              gpioPortB
#define RTE_USART0_CS_BIT               14
#else
#error "Invalid USART0 Pins Location!"
#endif

//   <o> CLK Pin <0=>Not used <1=>Used
#define RTE_USART0_CLK_PIN              0

//   <e> SPI mode
#define RTE_USART0_SPI_MODE             0
//     <o> CS Pin <0=>Not used <1=>Used
#define RTE_USART0_CS_PIN               1
//   </e>

#if    (RTE_USART_CLK_PIN && (RTE_USART0_PIN_LOC == 0))
#error "USART0 cannot operate in synchronous mode on location US0_x#3!"
#endif

//   <e> DMA
//     <o1> Channel <0-11>
//     <i>  Selects DMA Channel (channel 0 to 11 can be used)
//     <o2> Channel <0-11>
//     <i>  Selects DMA Channel (channel 0 to 11 can be used)
//   </e>
#define RTE_USART0_DMA                  1
#define RTE_USART0_RX_DMA_CHANNEL       0
#define RTE_USART0_TX_DMA_CHANNEL       1

// </e>

// <e> USART1 (Universal synchronous asynchronous receiver transmitter) [Driver_SPI1 or Driver_USART1]
// <i> Configuration settings for Driver_SPI1 in component ::Drivers:SPI when in SPI mode otherwise for Driver_USART1 in component ::Drivers:USART
#define RTE_USART1                      0

//   <o> USART1 Pins Location <0=>US0_x#0 <1=>US0_x#1 <2=>US0_x#2
#define RTE_USART1_PIN_LOC              1

#if    (RTE_USART1_PIN_LOC == 0)
#define RTE_USART1_TX_PORT              gpioPortC
#define RTE_USART1_TX_BIT               0
#define RTE_USART1_RX_PORT              gpioPortC
#define RTE_USART1_RX_BIT               1
#define RTE_USART1_CLK_PORT             gpioPortB
#define RTE_USART1_CLK_BIT              7
#define RTE_USART1_CS_PORT              gpioPortB
#define RTE_USART1_CS_BIT               8
#elif  (RTE_USART1_PIN_LOC == 1)
#define RTE_USART1_TX_PORT              gpioPortD
#define RTE_USART1_TX_BIT               0
#define RTE_USART1_RX_PORT              gpioPortD
#define RTE_USART1_RX_BIT               1
#define RTE_USART1_CLK_PORT             gpioPortD
#define RTE_USART1_CLK_BIT              2
#define RTE_USART1_CS_PORT              gpioPortD
#define RTE_USART1_CS_BIT               3
#elif  (RTE_USART1_PIN_LOC == 2)
#define RTE_USART1_TX_PORT              gpioPortD
#define RTE_USART1_TX_BIT               7
#define RTE_USART1_RX_PORT              gpioPortD
#define RTE_USART1_RX_BIT               6
#define RTE_USART1_CLK_PORT             gpioPortF
#define RTE_USART1_CLK_BIT              0
#define RTE_USART1_CS_PORT              gpioPortF
#define RTE_USART1_CS_BIT               1
#else
#error "Invalid USART1 Pins Location!"
#endif

//   <o> CLK Pin <0=>Not used <1=>Used
#define RTE_USART1_CLK_PIN              0

//   <e> SPI mode
#define RTE_USART1_SPI_MODE             0
//     <o> CS Pin <0=>Not used <1=>Used
#define RTE_USART1_CS_PIN               1
//   </e>

//   <e> DMA
//     <o1> Channel <0-11>
//     <i>  Selects DMA Channel (channel 0 to 11 can be used)
//     <o2> Channel <0-11>
//     <i>  Selects DMA Channel (channel 0 to 11 can be used)
//   </e>
#define RTE_USART1_DMA                  0
#define RTE_USART1_RX_DMA_CHANNEL       0
#define RTE_USART1_TX_DMA_CHANNEL       1

// </e>


// <e> USART2 (Universal synchronous asynchronous receiver transmitter) [Driver_SPI2 or Driver_USART2]
// <i> Configuration settings for Driver_SPI2 in component ::Drivers:SPI when in SPI mode otherwise for Driver_USART2 in component ::Drivers:USART
#define RTE_USART2                      0

//   <o> USART2 Pins Location <0=>US0_x#0 <1=>US0_x#1
#define RTE_USART2_PIN_LOC              1

#if    (RTE_USART2_PIN_LOC == 0)
#define RTE_USART2_TX_PORT              gpioPortC
#define RTE_USART2_TX_BIT               2
#define RTE_USART2_RX_PORT              gpioPortC
#define RTE_USART2_RX_BIT               3
#define RTE_USART2_CLK_PORT             gpioPortC
#define RTE_USART2_CLK_BIT              4
#define RTE_USART2_CS_PORT              gpioPortC
#define RTE_USART2_CS_BIT               5
#elif  (RTE_USART2_PIN_LOC == 1)
#define RTE_USART2_TX_PORT              gpioPortB
#define RTE_USART2_TX_BIT               3
#define RTE_USART2_RX_PORT              gpioPortB
#define RTE_USART2_RX_BIT               4
#define RTE_USART2_CLK_PORT             gpioPortB
#define RTE_USART2_CLK_BIT              5
#define RTE_USART2_CS_PORT              gpioPortB
#define RTE_USART2_CS_BIT               6
#else
#error "Invalid USART2 Pins Location!"
#endif

//   <o> CLK Pin <0=>Not used <1=>Used
#define RTE_USART2_CLK_PIN              0

//   <e> SPI mode
#define RTE_USART2_SPI_MODE             0
//     <o> CS Pin <0=>Not used <1=>Used
#define RTE_USART2_CS_PIN               1
//   </e>

//   <e> DMA
//     <o1> Channel <0-11>
//     <i>  Selects DMA Channel (channel 0 to 11 can be used)
//     <o2> Channel <0-11>
//     <i>  Selects DMA Channel (channel 0 to 11 can be used)
//   </e>
#define RTE_USART2_DMA                  0
#define RTE_USART2_RX_DMA_CHANNEL       0
#define RTE_USART2_TX_DMA_CHANNEL       1

// </e>


// <e> USB
#define RTE_USB                         0

//   <e> Device [Driver_USBD0]
//   <i> Configuration settings for Driver_USBD0 in component ::Drivers:USB Device
#define RTE_USB_DEV                     0

//     <h> Endpoints
//     <i> Reduce memory requirements of Driver by disabling unused endpoints
//       <e0.1> Endpoint 1
//         <o1.1>  Bulk OUT
//         <o1.17> Bulk IN
//         <o2.1>  Interrupt OUT
//         <o2.17> Interrupt IN
//         <o3.1>  Isochronous OUT
//         <o3.17> Isochronous IN
//       </e>
//       <e0.2> Endpoint 2
//         <o1.2>  Bulk OUT
//         <o1.18> Bulk IN
//         <o2.2>  Interrupt OUT
//         <o2.18> Interrupt IN
//         <o3.2>  Isochronous OUT
//         <o3.18> Isochronous IN
//       </e>
//       <e0.3> Endpoint 3
//         <o1.3>  Bulk OUT
//         <o1.19> Bulk IN
//         <o2.3>  Interrupt OUT
//         <o2.19> Interrupt IN
//         <o3.3>  Isochronous OUT
//         <o3.19> Isochronous IN
//       </e>
//       <e0.4> Endpoint 4
//         <o1.4>  Bulk OUT
//         <o1.20> Bulk IN
//         <o2.4>  Interrupt OUT
//         <o2.20> Interrupt IN
//         <o3.4>  Isochronous OUT
//         <o3.20> Isochronous IN
//       </e>
//       <e0.5> Endpoint 5
//         <o1.5>  Bulk OUT
//         <o1.21> Bulk IN
//         <o2.5>  Interrupt OUT
//         <o2.21> Interrupt IN
//         <o3.5>  Isochronous OUT
//         <o3.21> Isochronous IN
//       </e>
//       <e0.6> Endpoint 6
//         <o1.6>  Bulk OUT
//         <o1.22> Bulk IN
//         <o2.6>  Interrupt OUT
//         <o2.22> Interrupt IN
//         <o3.6>  Isochronous OUT
//         <o3.22> Isochronous IN
//       </e>
//     </h>
#define RTE_USB_DEV_EP                  0x0000007F
#define RTE_USB_DEV_EP_BULK             0x007E007E
#define RTE_USB_DEV_EP_INT              0x007E007E
#define RTE_USB_DEV_EP_ISO              0x007E007E

//   </e>

//   <e> Host [Driver_USBH0]
//   <i> Configuration settings for Driver_USBH0 in component ::Drivers:USB Host
#define RTE_USB_HOST                    0

//     <e> VBUS Power On/Off Pin
//     <i> Configure Pin for driving VBUS
//     <i> GPIO Pxy (x = A..H, y = 0..15) or (x = I, y = 0..11)
//       <o1> Active State <0=>Low <1=>High
//       <i>  Selects Active State Logical Level
//       <o2> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//                 <4=>GPIOE <5=>GPIOF <6=>GPIOG <7=>GPIOH <8=>GPIOI
//       <i>  Selects Port Name
//       <o3> Bit <0-15>
//       <i>  Selects Port Bit
//     </e>
#define RTE_USB_VBUS_PIN             1
#define RTE_USB_VBUS_ACTIVE          0
#define RTE_USB_VBUS_PORT            gpioPortF
#define RTE_USB_VBUS_BIT             5

//     <e> Overcurrent Detection Pin
//     <i> Configure Pin for overcurrent detection
//     <i> GPIO Pxy (x = A..H, y = 0..15) or (x = I, y = 0..11)
//       <o1> Active State <0=>Low <1=>High
//       <i>  Selects Active State Logical Level
//       <o2> Port <0=>GPIOA <1=>GPIOB <2=>GPIOC <3=>GPIOD
//                 <4=>GPIOE <5=>GPIOF <6=>GPIOG <7=>GPIOH <8=>GPIOI
//       <i>  Selects Port Name
//       <o3> Bit <0-15>
//       <i>  Selects Port Bit
//     </e>
#define RTE_USB_OC_PIN               1
#define RTE_USB_OC_ACTIVE            0
#define RTE_USB_OC_PORT              gpioPortE
#define RTE_USB_OC_BIT               2
//   </e>

// </e>


#endif  /* __RTE_DEVICE_H */
