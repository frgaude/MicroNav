/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Configuration handler                                         *
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

#include "Configuration.h"
#include "BoardConfig.h"

#include <Arduino.h>
#include <EEPROM.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define CONFIGURATION_EEPROM_SIZE 128
#define EEPROM_CONFIG_OFFSET      0
#define CONFIG_MAGIC_NUMBER       0x4D544E4D

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

#pragma pack(1)
typedef struct
{
	uint32_t magicWord;
	uint32_t attachedNetworkId;
	uint32_t deviceId;
	float waterSpeedFactor_per;
	float waterTemperatureOffset_C;
	float depthOffset_m;
	float windSpeedFactor_per;
	float windDirectionOffset_deg;
	float headingOffset_deg;
	float magneticVariation_deg;
	float windShift;
	float xMagOffset;
	float yMagOffset;
	float zMagOffset;
	float rfFrequencyOffset_MHz;
	int8_t timeZone_h;
	uint8_t checksum;
} ConfigBlock_t;
#pragma pack()

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                               Globals                                   */
/***************************************************************************/

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

void Configuration::Init()
{
	EEPROM.begin(CONFIGURATION_EEPROM_SIZE);

	if ((void*)(&NMEA_EXT) == (void*)(&BT_NMEA))
	{
		serialType = SERIAL_TYPE_BT;
	}
	else if ((void*)(&NMEA_EXT) == (void*)(&WIRED_NMEA))
	{
		serialType = SERIAL_TYPE_WIRED;
	}
	else if ((void*)(&NMEA_EXT) == (void*)(&USB_NMEA))
	{
		serialType = SERIAL_TYPE_USB;
	}
}

Configuration::Configuration()
{
	// Set default configuration
	navCompassAvailable = false;
	displayAvailable = false;
	serialType = SERIAL_TYPE_USB;
	networkId = 0;
	waterSpeedFactor_per = 1.0f;
	waterTemperatureOffset_C = 0;
	depthOffset_m = 0;
	windSpeedFactor_per = 1.0f;
	windDirectionOffset_deg = 0;
	headingOffset_deg = 0;
	magneticVariation_deg = 0;
	windShift = 10;
	deviceId = 0x03123456;
	xMagOffset = 0;
	yMagOffset = 0;
	zMagOffset = 0;
	rfFrequencyOffset_MHz = 0;
	timeZone_h = 0;
}

Configuration::~Configuration()
{
}

void Configuration::LoadFromEeprom()
{
	ConfigBlock_t configBlock = { 0 };

	EEPROM.get(0, configBlock);
	uint8_t* pConfig = (uint8_t*)(&configBlock);

	if (configBlock.magicWord == CONFIG_MAGIC_NUMBER)
	{
		uint8_t checksum = 0;
		for (uint32_t i = 0; i < (sizeof(ConfigBlock_t) - 1); i++)
		{
			checksum += pConfig[i];
		}

		if (checksum == configBlock.checksum)
		{
			networkId = configBlock.attachedNetworkId;
			deviceId = configBlock.deviceId;
			waterSpeedFactor_per = configBlock.waterSpeedFactor_per;
			waterTemperatureOffset_C = configBlock.waterTemperatureOffset_C;
			depthOffset_m = configBlock.depthOffset_m;
			windSpeedFactor_per = configBlock.windSpeedFactor_per;
			windDirectionOffset_deg = configBlock.windDirectionOffset_deg;
			headingOffset_deg = configBlock.headingOffset_deg;
			magneticVariation_deg = configBlock.magneticVariation_deg;
			windShift = configBlock.windShift;
			xMagOffset = configBlock.xMagOffset;
			yMagOffset = configBlock.yMagOffset;
			zMagOffset = configBlock.zMagOffset;
			rfFrequencyOffset_MHz = configBlock.rfFrequencyOffset_MHz;
			timeZone_h = configBlock.timeZone_h;
		}
	}
}

void Configuration::SaveToEeprom()
{
	ConfigBlock_t eepromBlock = { 0 };
	ConfigBlock_t configBlock = { 0 };

	uint8_t* pEepromBlock = (uint8_t*)(&eepromBlock);
	uint8_t* pConfig = (uint8_t*)(&configBlock);
	uint8_t checksum = 0;

	EEPROM.get(0, eepromBlock);

	configBlock.magicWord = CONFIG_MAGIC_NUMBER;
	configBlock.attachedNetworkId = networkId;
	configBlock.deviceId = deviceId;
	configBlock.waterSpeedFactor_per = waterSpeedFactor_per;
	configBlock.waterTemperatureOffset_C = waterTemperatureOffset_C;
	configBlock.depthOffset_m = depthOffset_m;
	configBlock.windSpeedFactor_per = windSpeedFactor_per;
	configBlock.windDirectionOffset_deg = windDirectionOffset_deg;
	configBlock.headingOffset_deg = headingOffset_deg;
	configBlock.magneticVariation_deg = magneticVariation_deg;
	configBlock.windShift = windShift;
	configBlock.xMagOffset = xMagOffset;
	configBlock.yMagOffset = yMagOffset;
	configBlock.zMagOffset = zMagOffset;
	configBlock.rfFrequencyOffset_MHz = rfFrequencyOffset_MHz;
	configBlock.timeZone_h = timeZone_h;

	for (uint32_t i = 0; i < sizeof(ConfigBlock_t) - 1; i++)
	{
		checksum += pConfig[i];
	}
	configBlock.checksum = checksum;

	for (uint32_t i = 0; i < sizeof(ConfigBlock_t); i++)
	{
		if (pEepromBlock[i] != pConfig[i])
		{
			EEPROM.put(0, configBlock);
			EEPROM.commit();
			break;
		}
	}
}

void Configuration::SaveCalibration(MicronetCodec& micronetCodec)
{
	waterSpeedFactor_per = micronetCodec.navData.waterSpeedFactor_per;
	waterTemperatureOffset_C = micronetCodec.navData.waterTemperatureOffset_degc;
	depthOffset_m = micronetCodec.navData.depthOffset_m;
	windSpeedFactor_per = micronetCodec.navData.windSpeedFactor_per;
	windDirectionOffset_deg = micronetCodec.navData.windDirectionOffset_deg;
	headingOffset_deg = micronetCodec.navData.headingOffset_deg;
	magneticVariation_deg = micronetCodec.navData.magneticVariation_deg;
	windShift = micronetCodec.navData.windShift_min;
	timeZone_h = micronetCodec.navData.timeZone_h;

	SaveToEeprom();
}

void Configuration::LoadCalibration(MicronetCodec* micronetCodec)
{
	micronetCodec->navData.waterSpeedFactor_per = waterSpeedFactor_per;
	micronetCodec->navData.waterTemperatureOffset_degc = waterTemperatureOffset_C;
	micronetCodec->navData.depthOffset_m = depthOffset_m;
	micronetCodec->navData.windSpeedFactor_per = windSpeedFactor_per;
	micronetCodec->navData.windDirectionOffset_deg = windDirectionOffset_deg;
	micronetCodec->navData.headingOffset_deg = headingOffset_deg;
	micronetCodec->navData.magneticVariation_deg = magneticVariation_deg;
	micronetCodec->navData.windShift_min = windShift;
	micronetCodec->navData.timeZone_h = timeZone_h;
}
