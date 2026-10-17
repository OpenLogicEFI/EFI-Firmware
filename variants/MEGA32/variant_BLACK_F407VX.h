/*
 *******************************************************************************
 * Copyright (c) 2017-2021, STMicroelectronics
 * All rights reserved.
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */
#pragma once

/*----------------------------------------------------------------------------
 *        STM32 pins number
 *----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 *        STM32 pins number (Arduino Mega D0..D53)
 *----------------------------------------------------------------------------*/

// D0..D13
#define PA10                     0   // D0  RX1
#define PA9                      1   // D1  TX1
#define PD4                      2   // D2
#define PD3                      3   // D3
#define PC9                      4   // D4
#define PC8                      5   // D5
#define PC7                      6   // D6
#define PC6                      7   // D7
#define PD15                     8   // D8
#define PD14                     9   // D9
#define PD13                     10  // D10
#define PD12                     11  // D11
#define PA8                      12  // D12
#define PA13                     13  // D13
#define PD5                      14  // D14 USART2 TX
#define PD6                      15  // D15 USART2 RX
#define PB10                     16  // D16 USART3 TX
#define PB11                     17  // D17 USART3 RX
#define PD11                     18  // D18 GPIO
#define PD10                     19  // D19 GPIO
#define PD9                      20  // D20 GPIO / USART3 RX
#define PD8                      21  // D21 GPIO / USART3 TX
#define PB15                     22  // D22
#define PB14                     23  // D23
#define PB13                     24  // D24
#define PA14                     25  // D25 SWCLK
#define PB12                     26  // D26
#define PE15                     27  // D27
#define PD7                      28  // D28
#define PE14                     29  // D29
#define PE13                     30  // D30
#define PB2                      31  // D31 BOOT1
#define PB6                      32  // D32
#define PB7                      33  // D33 
#define PB8                      34  // D34
#define PB9                      35  // D35
#define PE0                      36  // D36
#define PA15                     37  // D37
#define PE12                     38  // D38
#define PE1                      39  // D39 
#define PB5                      40  // D40
#define PE2                      41  // D41
#define PE3                      42  // D42 
#define PE4                      43  // D43 
#define PE5                      44  // D44
#define PB4                      45  // D45
#define PE6                      46  // D46
#define PB3                      47  // D47 
#define PE10                     48  // D48
#define PE11                     49  // D49
#define PE9                      50  // D50
#define PC13                     51  // D51
#define PE7                      52  // D52
#define PE8                      53  // D53

#define PA0                      PIN_A0
#define PA1                      PIN_A1
#define PA2                      PIN_A2
#define PA3                      PIN_A3
#define PA4                      PIN_A4
#define PA5                      PIN_A5
#define PA6                      PIN_A6
#define PA7                      PIN_A7
#define PB0                      PIN_A8
#define PB1                      PIN_A9
#define PC0                      PIN_A10
#define PC1                      PIN_A11
#define PC2                      PIN_A12
#define PC3                      PIN_A13
#define PC4                      PIN_A14
#define PC5                      PIN_A15

#define PA11                     70 // USB_DM
#define PA12                     71 // USB_DP
#define PE1_D72                  72 // SPI-Flash CS, alias of D39 / PE1
#define PB4_D73                  73 // SPI1 MISO, alias of D45 / PB4
#define PB5_D74                  74 // SPI1 MOSI, alias of D40 / PB5
#define PB3_D75                  75 // SPI1 CLK, alias of D47 / PB3
// Compatibility pins used by existing Speeduino STM32 code paths
#define PD0                      76 // CAN RX
#define PD1                      77 // CAN TX
#define PD2                      78 // SD CS
#define PC10                     79 // SPI3 SCK
#define PC11                     80 // SPI3 MISO
#define PC12                     81 // SPI3 MOSI
#define PA14_D82                 82 // Compatibility alias of D25 / PA14
#define PB2_D83                  83 // Compatibility alias of D31 / PB2
#define PC14                     84
#define PC15                     85

// Alternate pins number
#define PA0_ALT1                (PA0  | ALT1)
#define PA0_ALT2                (PA0  | ALT2)
#define PA1_ALT1                (PA1  | ALT1)
#define PA1_ALT2                (PA1  | ALT2)
#define PA2_ALT1                (PA2  | ALT1)
#define PA2_ALT2                (PA2  | ALT2)
#define PA3_ALT1                (PA3  | ALT1)
#define PA3_ALT2                (PA3  | ALT2)
#define PA4_ALT1                (PA4  | ALT1)
#define PA5_ALT1                (PA5  | ALT1)
#define PA6_ALT1                (PA6  | ALT1)
#define PA7_ALT1                (PA7  | ALT1)
#define PA7_ALT2                (PA7  | ALT2)
#define PA7_ALT3                (PA7  | ALT3)
#define PA15_ALT1               (PA15 | ALT1)
#define PB0_ALT1                (PB0  | ALT1)
#define PB0_ALT2                (PB0  | ALT2)
#define PB1_ALT1                (PB1  | ALT1)
#define PB1_ALT2                (PB1  | ALT2)
#define PB3_ALT1                (PB3  | ALT1)
#define PB4_ALT1                (PB4  | ALT1)
#define PB5_ALT1                (PB5  | ALT1)
#define PB8_ALT1                (PB8  | ALT1)
#define PB9_ALT1                (PB9  | ALT1)
#define PB14_ALT1               (PB14 | ALT1)
#define PB14_ALT2               (PB14 | ALT2)
#define PB15_ALT1               (PB15 | ALT1)
#define PB15_ALT2               (PB15 | ALT2)
#define PC0_ALT1                (PC0  | ALT1)
#define PC0_ALT2                (PC0  | ALT2)
#define PC1_ALT1                (PC1  | ALT1)
#define PC1_ALT2                (PC1  | ALT2)
#define PC2_ALT1                (PC2  | ALT1)
#define PC2_ALT2                (PC2  | ALT2)
#define PC3_ALT1                (PC3  | ALT1)
#define PC3_ALT2                (PC3  | ALT2)
#define PC4_ALT1                (PC4  | ALT1)
#define PC5_ALT1                (PC5  | ALT1)
#define PC6_ALT1                (PC6  | ALT1)
#define PC7_ALT1                (PC7  | ALT1)
#define PC8_ALT1                (PC8  | ALT1)
#define PC9_ALT1                (PC9  | ALT1)
#define PC10_ALT1               (PC10 | ALT1)
#define PC11_ALT1               (PC11 | ALT1)

#define NUM_DIGITAL_PINS        86
#define NUM_ANALOG_INPUTS       16

// On-board LED pin number
//Fix me
#ifndef LED_BUILTIN
  #define LED_BUILTIN           PA7 // On-board LED
#endif

// Below SPI and I2C definitions already done in the core
// Could be redefined here if differs from the default one
// SPI Definitions
#ifndef PIN_SPI_SS
  #define PIN_SPI_SS            PE1_D72 // NRF24 connector
#endif
#ifndef PIN_SPI_SS1
  #define PIN_SPI_SS1           PE1_D72 // W25Q16 (on board flash)
#endif
#ifndef PIN_SPI_MOSI
  #define PIN_SPI_MOSI          PB5_D74 // NRF24 connector & W25Q16 (on board flash)
#endif
#ifndef PIN_SPI_MISO
  #define PIN_SPI_MISO          PB4_D73 // NRF24 connector & W25Q16 (on board flash)
#endif
#ifndef PIN_SPI_SCK
  #define PIN_SPI_SCK           PB3_D75 // NRF24 connector & W25Q16 (on board flash)
#endif

// I2C1 on direct GPIO headers D33 (SDA) and D32 (SCL).
// D20 / PD9 and D21 / PD8 do not support hardware I2C.
#ifndef PIN_WIRE_SDA
  #define PIN_WIRE_SDA          PB7
#endif
#ifndef PIN_WIRE_SCL
  #define PIN_WIRE_SCL          PB6
#endif

// Timer Definitions
// Use TIM6/TIM7 when possible as servo and tone don't need GPIO output pin
#ifndef TIMER_TONE
  #define TIMER_TONE            TIM6
#endif
#ifndef TIMER_SERVO
  #define TIMER_SERVO           TIM7
#endif

// UART Definitions
// Define here Serial instance number to map on Serial generic name
#ifndef SERIAL_UART_INSTANCE
  #define SERIAL_UART_INSTANCE  1
#endif

// Default pin used for 'Serial' instance
// Mandatory for Firmata
#ifndef PIN_SERIAL_RX
  #define PIN_SERIAL_RX         PA10
#endif
#ifndef PIN_SERIAL_TX
  #define PIN_SERIAL_TX         PA9
#endif

/* Extra HAL modules */
#if !defined(HAL_DAC_MODULE_DISABLED)
  #define HAL_DAC_MODULE_ENABLED
#endif
#if !defined(HAL_SD_MODULE_DISABLED)
  #define HAL_SD_MODULE_ENABLED
#endif

// This indicates that there is an external and fixed 1.5k pullup
// on the D+ line. This define is only needed on boards that have
// internal pullups *and* an external pullup. Note that it would have
// been better to omit the pullup and exclusively use the internal
// pullups instead.
#define USBD_FIXED_PULLUP

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus
  // These serial port names are intended to allow libraries and architecture-neutral
  // sketches to automatically default to the correct port name for a particular type
  // of use.  For example, a GPS module would normally connect to SERIAL_PORT_HARDWARE_OPEN,
  // the first hardware serial port whose RX/TX pins are not dedicated to another use.
  //
  // SERIAL_PORT_MONITOR        Port which normally prints to the Arduino Serial Monitor
  //
  // SERIAL_PORT_USBVIRTUAL     Port which is USB virtual serial
  //
  // SERIAL_PORT_LINUXBRIDGE    Port which connects to a Linux system via Bridge library
  //
  // SERIAL_PORT_HARDWARE       Hardware serial port, physical RX & TX pins.
  //
  // SERIAL_PORT_HARDWARE_OPEN  Hardware serial ports which are open for use.  Their RX & TX
  //                            pins are NOT connected to anything by default.
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial1
  #endif
#endif
