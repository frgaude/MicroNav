/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Board/Platform related configuration                          *
 * Author:   Ronan Demoment                                                *
 *                                                                         *
 ***************************************************************************
 *   Copyright (C) 2021 by Ronan Demoment                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef BOARDCONFIG_H_
#define BOARDCONFIG_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "Globals.h"

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

// The console to use for menu
#define CONSOLE          Serial
#define CONSOLE_BAUDRATE 115200

// Selects on which I2C bus is connected compass as per Wiring library definition
#define NAVCOMPASS_I2C Wire

// Support board list , Macro definition below, select the board definition to be used

 #define T_BEAM_SX1262
// #define T_BEAM_SX1276
// #define T_BEAM_SX1278

// #define T_BEAM_S3_SUPREME

#define UNUSED_PIN                   (0)

#if defined(T_BEAM_SX1262) || defined(T_BEAM_SX1276) || defined(T_BEAM_SX1278)

#if   defined(T_BEAM_SX1262)
#ifndef USING_SX1262
#define USING_SX1262
#endif
#elif defined(T_BEAM_SX1276)
#ifndef USING_SX1276
#define USING_SX1276
#endif
#elif defined(T_BEAM_SX1278)
#ifndef USING_SX1278
#define USING_SX1278
#endif
#endif // T_BEAM_SX1262

#define RF_CS_PIN                   18
#define RF_MOSI_PIN                 27
#define RF_MISO_PIN                 19
#define RF_SCLK_PIN                  5
#define RF_DIO0_PIN                 26
#define RF_DIO1_PIN                 33
#define RF_RST_PIN                  23
// SX1276/78
#define RF_DIO2_PIN                 32
// SX1262
#define RF_BUSY_PIN                 32

// NMEA GNSS UART pins
#define GNSS_UBLOXM8N 1 // Set to one if your GNSS is a UBLOX M8N/M6N, 0 else.
#define GNSS_SERIAL                 Serial2
#define GNSS_BAUDRATE               9600
#define GNSS_RX_PIN                 34
#define GNSS_TX_PIN                 12

// Button for user interaction
#define BUTTON_PIN                  38
#define BUTTON_PIN_MASK             GPIO_SEL_38

// PMU connections
#define I2C_SDA                     21
#define I2C_SCL                     22
#define PMU_IRQ                     35

#define BOARD_LED                   4
#define LED_ON                      LOW
#define LED_OFF                     HIGH

#define HAS_GNSS
#define HAS_DISPLAY                 //Optional, bring your own board, no OLED !!
#define HAS_PMU

#define PMU_WIRE_PORT               Wire1
#define DISPLAY_MODEL               U8G2_SSD1306_128X64_NONAME_F_HW_I2C
#define DISPLAY_ADDRESS             0x3C // I2C address of the display controller
#define BOARD_VARIANT_NAME          "T-Beam"

#elif defined(T_BEAM_S3_SUPREME)

#ifndef USING_SX1262
#define USING_SX1262
#endif

#define I2C_SDA                     17
#define I2C_SCL                     18

#define I2C1_SDA                    42
#define I2C1_SCL                    41
#define PMU_IRQ                     40

#define GNSS_UBLOXM8N 1 // Set to one if your GNSS is a UBLOX M8N/M6N, 0 else.
#define GNSS_SERIAL                 Serial2
#define GNSS_BAUDRATE               9600
#define GNSS_RX_PIN                 9
#define GNSS_TX_PIN                 8
#define GNSS_WAKEUP_PIN             7
#define GNSS_PPS_PIN                6

#define BUTTON_PIN                  0
#define BUTTON_PIN_MASK             GPIO_SEL_0
#define BUTTON_CONUT                (1)
#define BUTTON_ARRAY                {BUTTON_PIN}

#define RF_SCLK_PIN                 (12)
#define RF_MISO_PIN                 (13)
#define RF_MOSI_PIN                 (11)
#define RF_CS_PIN                   (10)
#define RF_DIO0_PIN                 (-1)
#define RF_RST_PIN                  (5)
#define RF_DIO1_PIN                 (1)
#define RF_BUSY_PIN                 (4)

#define SPI_MOSI                    (35)
#define SPI_SCK                     (36)
#define SPI_MISO                    (37)
#define SPI_CS                      (47)
#define IMU_CS                      (34)
#define IMU_INT                     (33)

#define SDCARD_MOSI                 SPI_MOSI
#define SDCARD_MISO                 SPI_MISO
#define SDCARD_SCLK                 SPI_SCK
#define SDCARD_CS                   SPI_CS

#define PIN_NONE                    (-1)
#define RTC_INT                     (14)

#define HAS_SDCARD
#define HAS_GNSS
#define HAS_DISPLAY
#define HAS_PMU

#define __HAS_SPI1__
#define __HAS_SENSOR__

#define PMU_WIRE_PORT               Wire1
#define DISPLAY_MODEL               U8G2_SH1106_128X64_NONAME_F_HW_I2C
#define DISPLAY_ADDRESS             0x3C // I2C address of the display controller
#define BOARD_VARIANT_NAME          "T-Beam S3"

#endif

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                              Prototypes                                 */
/***************************************************************************/

#endif /* BOARDCONFIG_H_ */
