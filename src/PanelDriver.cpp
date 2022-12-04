/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Driver for T-BEAM 1.1 OLED Panel                              *
 * Author:   Ronan Demoment                                                *
 *                                                                         *
 ***************************************************************************
 *   Copyright (C) 2022 by Ronan Demoment                                  *
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

 /***************************************************************************/
 /*                              Includes                                   */
 /***************************************************************************/

#include "PanelDriver.h"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define logo_micronav_width 128
#define logo_micronav_height 64
static unsigned char logo_micronav_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x83, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x7f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xfb, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x9f, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x7f, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x73, 0xf7, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xef, 0xf6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xdf, 0xf6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xdc, 0xf6, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xf8, 0x01, 0x3c,
   0xf8, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xc0, 0x07, 0xfc, 0xff, 0xff, 0xff,
   0x07, 0xf0, 0x01, 0x3c, 0xf8, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x80, 0x07,
   0xfc, 0xff, 0xff, 0xff, 0x1f, 0xf0, 0x81, 0x7f, 0xfc, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x81, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x1f, 0xf0, 0x80, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x3f, 0xff, 0xff, 0xff, 0xff,
   0x1f, 0xf0, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x3e,
   0xff, 0xff, 0xff, 0xff, 0x9f, 0xe0, 0x04, 0x3f, 0xf8, 0x03, 0x0f, 0x87,
   0x0f, 0xfc, 0x01, 0x3e, 0x3f, 0xf0, 0x00, 0x83, 0x9f, 0xe1, 0x04, 0x1f,
   0xf8, 0x00, 0x06, 0x83, 0x03, 0xf8, 0x11, 0x3c, 0x0f, 0xe0, 0x00, 0x83,
   0x9f, 0xe1, 0x04, 0x1f, 0x78, 0x38, 0x06, 0x80, 0xe1, 0xf0, 0x11, 0x3c,
   0x0f, 0xc0, 0xc1, 0xc7, 0x9f, 0x61, 0x0c, 0x7f, 0x78, 0x38, 0x1e, 0x80,
   0xe1, 0xe0, 0x31, 0x38, 0xe7, 0xc3, 0xc3, 0xe7, 0x9f, 0x41, 0x0e, 0x7f,
   0x38, 0x78, 0x1e, 0xce, 0xe0, 0xe1, 0x31, 0x30, 0xff, 0xc3, 0x83, 0xe7,
   0x8f, 0x43, 0x0e, 0x7f, 0x38, 0xfc, 0x1f, 0xce, 0xe0, 0xe1, 0x71, 0x30,
   0xff, 0xc3, 0x87, 0xf3, 0x8f, 0x43, 0x0e, 0x7f, 0x38, 0xfc, 0x1f, 0xfe,
   0xf0, 0xe1, 0xf1, 0x20, 0x3f, 0xc0, 0x87, 0xf3, 0x8f, 0x03, 0x0e, 0x7e,
   0x38, 0xfc, 0x1f, 0xfe, 0xf0, 0xe1, 0xf1, 0x20, 0x0f, 0xc0, 0x07, 0xf3,
   0x8f, 0x07, 0x0f, 0x7e, 0x38, 0xf8, 0x1f, 0xfe, 0xe0, 0xe1, 0xf1, 0x01,
   0x07, 0xc3, 0x0f, 0xf9, 0xcf, 0x07, 0x0f, 0x7e, 0x38, 0xf8, 0x1f, 0xfe,
   0xe0, 0xe1, 0xf1, 0x01, 0x87, 0xc3, 0x0f, 0xf9, 0xcf, 0x07, 0x0f, 0x7e,
   0x38, 0xf0, 0x1e, 0xfe, 0xe0, 0xe0, 0xf1, 0x03, 0x87, 0xc3, 0x1f, 0xf8,
   0xcf, 0x07, 0x1f, 0x7e, 0x78, 0x00, 0x1e, 0xfe, 0xe1, 0xf0, 0xf1, 0x07,
   0x87, 0xc3, 0x1f, 0xfc, 0x01, 0x0e, 0x07, 0x18, 0xe0, 0x00, 0x07, 0xf8,
   0x03, 0x78, 0xc0, 0x07, 0x07, 0x00, 0x1f, 0xfc, 0x01, 0xfe, 0x07, 0x18,
   0xe0, 0x81, 0x07, 0xf8, 0x07, 0x7c, 0xc0, 0x0f, 0x0f, 0x06, 0x3f, 0xfc,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff };
   
/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                           Static & Globals                              */
/***************************************************************************/

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

PanelDriver::PanelDriver()
{
    uint8_t data, invData;
    for (int i = 0; i < sizeof(logo_micronav_bits); i++)
    {
        data = logo_micronav_bits[i];
        invData = data >> 7;
        invData |= (data >> 5) & 0x02;
        invData |= (data >> 3) & 0x04;
        invData |= (data >> 1) & 0x08;
        invData |= (data << 1) & 0x10;
        invData |= (data << 3) & 0x20;
        invData |= (data << 5) & 0x40;
        invData |= (data << 7) & 0x80;
        logo_micronav_bits[i] = invData;
    }
}

PanelDriver::~PanelDriver()
{
}

bool PanelDriver::Init()
{
    bool returnStatus = false;

    if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        returnStatus = true;
        // Show initial display buffer contents on the screen --
        // the library initializes this with an Adafruit splash screen.
        display.clearDisplay();
        display.drawBitmap(0, 0,
            logo_micronav_bits,
            logo_micronav_width,
            logo_micronav_height,
            1);
        display.invertDisplay(true);
        display.display();
    }

    return returnStatus;
}