/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Global variables used by SW                                   *
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

#ifndef GLOBALS_H_
#define GLOBALS_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include "Configuration.h"
#include "MenuManager.h"
#include "Micronet/MicronetCodec.h"
#include "Micronet/MicronetDevice.h"
#include "Micronet/MicronetMessageFifo.h"
#include "NavCompass.h"
#include "NavigationData.h"
#include "NmeaBridge.h"
#include "PanelManager.h"
#include "Power.h"
#include "RfDriver.h"
#include "UbloxDriver.h"

#include <BluetoothSerial.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                               Globals                                   */
/***************************************************************************/

extern RfDriver            gRfDriver;
extern MenuManager         gMenuManager;
extern MicronetMessageFifo gRxMessageFifo;
extern Configuration       gConfiguration;
extern NavCompass          gNavCompass;
extern UbloxDriver         gM8nDriver;
extern PanelManager        gPanelDriver;
extern MicronetCodec       gMicronetCodec;
extern BluetoothSerial     gBtSerial;
extern NmeaBridge          gDataBridge;
extern MicronetDevice      gMicronetDevice;
extern Power               gPower;

/***************************************************************************/
/*                              Prototypes                                 */
/***************************************************************************/

#endif /* GNSSDECODER_H_ */
