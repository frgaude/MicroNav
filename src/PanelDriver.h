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

#ifndef PANELDRIVER_H_
#define PANELDRIVER_H_

 /***************************************************************************/
 /*                              Includes                                   */
 /***************************************************************************/

#include <Arduino.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

enum {
    PAGE_WELCOME = 0,
    PAGE_NETWORK,
    PAGE_MAX_PAGES
} PanelPages_t;

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class PanelDriver
{
public:
    PanelDriver();
    ~PanelDriver();

    bool Init();
    void SetPage(uint32_t pageNumber);
    void DrawPage();

private:
    bool displayAvailable;
    uint32_t pageNumber;

    void DrawWelcomePage();
    void DrawNetworkPage();

    void DrawDeviceIcon(uint8_t const* icon, uint32_t position, uint32_t radioLevel);
};

#endif
