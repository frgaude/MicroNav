/***************************************************************************
 *                                                                         *
 * Project:  MicronetToNMEA                                                *
 * Purpose:  Decode data from Micronet devices send it on an NMEA network  *
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

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "BoardConfig.h"

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <axp20x.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                               Globals                                   */
/***************************************************************************/

AXP20X_Class pmu;

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

void setup()
{
    Serial.begin(115200);

    Wire.begin(PMU_I2C_SDA, PMU_I2C_SCL);
    if (!pmu.begin(Wire, AXP192_SLAVE_ADDRESS)) {
        pmu.setPowerOutPut(AXP192_LDO2, AXP202_ON);
        pmu.setPowerOutPut(AXP192_LDO3, AXP202_ON);
        pmu.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
        pmu.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
        pmu.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
    } else {
        Serial.println("AXP192 Begin FAIL");
    }
}

void loop()
{
    delay(1000);                       // wait for a second
    Serial.println("Hello World");
}
