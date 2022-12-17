/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Handler of the Command page                                   *
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

#include "CommandPage.h"
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
#define NUMBER_OF_COMMANDS 4

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

CommandPage::CommandPage() : editMode(false), editPosition(0)
{
}

CommandPage::~CommandPage()
{
}

// @brief Draw the page on display
// @param force Force redraw, even if the content did not change
void CommandPage::Draw(bool force)
{
    char     lineStr[22];
    int16_t  xStr, yStr;
    uint16_t wStr, hStr;

    if (display != nullptr)
    {
        display->clearDisplay();

        // Config items
        display->setTextSize(1);
        display->setFont(nullptr);

        if ((editPosition == 0) && (editMode))
        {
            display->fillRect(0, 0 * 8, SCREEN_WIDTH, 8, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
        }
        else
        {
            display->setTextColor(SSD1306_WHITE);
        }
        PrintCentered(0 * 8, "Shutdown MicroNav");

        if ((editPosition == 1) && (editMode))
        {
            display->fillRect(0, 1 * 8, SCREEN_WIDTH, 8, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
        }
        else
        {
            display->setTextColor(SSD1306_WHITE);
        }
        PrintCentered(1 * 8, "Attach to network");

        if ((editPosition == 2) && (editMode))
        {
            display->fillRect(0, 2 * 8, SCREEN_WIDTH, 8, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
        }
        else
        {
            display->setTextColor(SSD1306_WHITE);
        }
        PrintCentered(2 * 8, "Calibrate compass");

        if ((editPosition == 3) && (editMode))
        {
            display->fillRect(0, 3 * 8, SCREEN_WIDTH, 8, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
        }
        else
        {
            display->setTextColor(SSD1306_WHITE);
        }
        PrintCentered(3 * 8, "Select Heading Vector");

        if (editMode)
        {
            if (editPosition == 4)
            {
                display->fillRect(0, 64 - 8, SCREEN_WIDTH, 8, SSD1306_WHITE);
                display->setTextColor(SSD1306_BLACK);
            }
            else
            {
                display->setTextColor(SSD1306_WHITE);
            }
            PrintCentered(64 - 8, "Exit");
        }
        else
        {
            display->setTextColor(SSD1306_WHITE);
            PrintCentered(64 - 8, "Commands");
        }

        display->display();
    }
}

// @brief Function called by PanelManager when the button is pressed
// @param longPress true if a long press was detected
// @return Action to be executed by PanelManager
PageAction_t CommandPage::OnButtonPressed(bool longPress)
{
    PageAction_t action = PAGE_ACTION_NEXT_PAGE;

    if (editMode)
    {
        // In edit mode, the button is used to cycle through the configuration items
        if (longPress)
        {
            if (editPosition == NUMBER_OF_COMMANDS)
            {
                // Long press on "Exit"
                editMode = false;
            }
            else
            {
                // Long press on a command
                switch (editPosition)
                {
                case 0:
                    gPower.Shutdown();
                    break;
                }
            }
            action = PAGE_ACTION_REFRESH;
        }
        else
        {
            // Short press : cycle through configuration items
            editPosition = (editPosition + 1) % (NUMBER_OF_COMMANDS + 1);
            action       = PAGE_ACTION_REFRESH;
        }
    }
    else
    {
        if (longPress)
        {
            // Long press while not in edit mode : enter edit mode
            editMode     = true;
            editPosition = 0;
            action       = PAGE_ACTION_REFRESH;
        }
        else
        {
            // Short press while not in edit mode : cycle to next page
            action = PAGE_ACTION_NEXT_PAGE;
        }
    }

    return action;
}

void CommandPage::PrintCentered(int32_t yPos, String const &text)
{
    int16_t  xStr, yStr;
    uint16_t wStr, hStr;

    display->getTextBounds(text, 0, 0, &xStr, &yStr, &wStr, &hStr);
    display->setCursor((SCREEN_WIDTH - wStr) / 2, yPos);
    display->println(text);
}