/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Config page                                      *
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

#include "ConfigPage1.h"
#include "Globals.h"
#include "MicronetDevice.h"
#include "PanelResources.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

// @brief Number of configuration items on this page
#define NUMBER_OF_CONFIG_ITEMS 3
// @brief Horizontal position of configuration values on display
#define SELECTION_X_POSITION 72

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                           Static & Globals                              */
/***************************************************************************/

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

ConfigPage1::ConfigPage1()
    : editMode(false), editPosition(0), configFreqSel(0), configNmeaSel(0), configSogCogFilter(false)
{
}

ConfigPage1::~ConfigPage1()
{
}

// @brief Draw the page on display
// @param force Force redraw, even if the content did not change
bool ConfigPage1::Draw(bool force, bool flushDisplay)
{
    int16_t  xStr, yStr;
    uint16_t wStr, hStr;

    // Forced draw occurs when user enters the page : load configuration locally
    if (force && !editMode)
    {
        configFreqSel       = (uint32_t)gConfiguration.eeprom.freqSystem;
        configNmeaSel       = (uint32_t)gConfiguration.eeprom.nmeaLink;
        configSogCogFilter  = gConfiguration.eeprom.sogcogFilter;
    }

    if (display != nullptr)
    {
        display->clearDisplay();

        // Config items
        display->setTextColor(SSD1306_WHITE);
        display->setTextSize(1);
        display->setFont(nullptr);
        display->setCursor(0, 0);
        display->println("Frequency");
        display->println("NMEA link");
        display->println("SOG Filter");
        
        // Config values
        for (int i = 0; i < NUMBER_OF_CONFIG_ITEMS; i++)
        {
            display->setCursor(SELECTION_X_POSITION, i * 8);
            if (editMode && (editPosition == i))
            {
                display->fillRect(SELECTION_X_POSITION, i * 8, SCREEN_WIDTH - SELECTION_X_POSITION, 8, SSD1306_WHITE);
                display->setTextColor(SSD1306_BLACK);
            }
            else
            {
                display->setTextColor(SSD1306_WHITE);
            }
            display->getTextBounds(String(ConfigString(i)), 0, 0, &xStr, &yStr, &wStr, &hStr);
            display->setCursor(SCREEN_WIDTH - wStr, i * 8);
            display->print(ConfigString(i));
        }

        if (flushDisplay)
        {
            display->display();
        }
    }

    return true;
}

// @brief Function called by PanelManager when the button is pressed
// @param longPress true if a long press was detected
// @return Action to be executed by PanelManager
PageAction_t ConfigPage1::OnButtonPressed(ButtonId_t buttonId, bool longPress)
{
    PageAction_t action = PAGE_ACTION_NONE;

    if (editMode)
    {
        // Short press on power button
        if ((buttonId == BUTTON_ID_0) && (!longPress))
        {
            // Cycle configuration item values
            ConfigCycle(editPosition);
            action = PAGE_ACTION_REFRESH;
        }
        // App button
        else if (buttonId == BUTTON_ID_1)
        {
            if (longPress)
            {
                // Long press : deploy configuration ans exit edit mode
                DeployConfiguration();
                editMode = false;
            }
            else
            {
                // Short press : cycle through configuration items
                editPosition = (editPosition + 1) % NUMBER_OF_CONFIG_ITEMS;
                action       = PAGE_ACTION_REFRESH;
            }
        }
    }
    else
    {
        if ((buttonId == BUTTON_ID_1) && longPress)
        {
            editMode     = true;
            editPosition = 0;
            action       = PAGE_ACTION_REFRESH;
        }
        else if ((buttonId == BUTTON_ID_0) && !longPress)
        {
            action = PAGE_ACTION_EXIT_TOPIC;
        }
        else if ((buttonId == BUTTON_ID_1) && !longPress)
        {
            action = PAGE_ACTION_EXIT_PAGE;
        }
    }

    return action;
}

// @brief Return the string of a given configuration item
// @param index Configuration item
// @return String naming the value of the configuration item
char const *ConfigPage1::ConfigString(uint32_t index)
{
    switch (index)
    {
    case 0:
        return ConfigFreqString();
    case 1:
        return ConfigNmeaString();
    case 2:
        return ConfigSogCogFilterString();
    }

    return "---";
}

// @brief Return the string of a the frequency system configuration item
// @return String naming the value of the frequency
char const *ConfigPage1::ConfigFreqString()
{
    switch (configFreqSel)
    {
    case 0:
        return "868Mhz";
    case 1:
        return "915Mhz";
    }

    return "---";
}

// Return the string of a the NMEA link configuration item
// @return String naming the value of the NMEA link
char const *ConfigPage1::ConfigNmeaString()
{
    switch (configNmeaSel)
    {
    case 0:
        return "USB";
    case 1:
        return "Bluetooth";
    case 2:
        return "WiFi TCP";
    case 3:
        return "WiFi UDP";
    }

    return "---";
}

// Return the string of a the SOG/COG filter configuration item
// @return String naming the value of the SOG/COG filter
char const *ConfigPage1::ConfigSogCogFilterString()
{
    if (configSogCogFilter)
    {
        return "Yes";
    }
    return "No";
}

// @brief Cycle the value of a given configuration item
// @param index Configuration item
void ConfigPage1::ConfigCycle(uint32_t index)
{
    switch (index)
    {
    case 0:
        ConfigFreqCycle();
        break;
    case 1:
        ConfigNmeaCycle();
        break;
    case 2:
        ConfigSogCogFilterCycle();
        break;
    }
}

// @brief Cycle the value of the frequency system configuration item
void ConfigPage1::ConfigFreqCycle()
{
    configFreqSel = (configFreqSel + 1) % 2;
}

// @brief Cycle the value of the NMEA Link configuration item
void ConfigPage1::ConfigNmeaCycle()
{
    configNmeaSel = (configNmeaSel + 1) % 4;
}

// @brief Cycle the value of the SOG/COG filter configuration item
void ConfigPage1::ConfigSogCogFilterCycle()
{
    configSogCogFilter = !configSogCogFilter;
}

// @brief Deploy the local configuration to the overall system and save it to
// EEPROM
void ConfigPage1::DeployConfiguration()
{
    gConfiguration.eeprom.freqSystem    = (FreqSystem_t)configFreqSel;
    gConfiguration.eeprom.nmeaLink      = (SerialType_t)configNmeaSel;
    gConfiguration.eeprom.sogcogFilter = configSogCogFilter;

    gConfiguration.DeployConfiguration(&gMicronetDevice);
    gConfiguration.SaveToEeprom();
}
