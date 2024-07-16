/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Power Manager                                                 *
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

#include "Power.h"
#include "BoardConfig.h"
#include "Globals.h"

#include <Arduino.h>
#include <Wire.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

// Command masks for IRQ/Task communication
#define POWER_EVENT_SHUTDOWN 0x00000001
#define POWER_EVENT_IRQ      0x00000002
#define POWER_EVENT_ALL      0x00000003

// Wake-up period of the power processing task in milliseconds
#define TASK_WAKEUP_PERIOD_MS 100

// Those are the filtering factors for various battery indicators.
// Filtering them avoids current/voltage bursts to propagates
// noise to other values.
#define VOLTAGE_FILTERING_FACTOR       0.80f
#define CURRENT_FILTERING_FACTOR       0.80f
#define BATTERY_LEVEL_FILTERING_FACTOR 0.99f
#define TEMPERATURE_FILTERING_FACTOR   0.98f

// Estimated internal resistance of battery.
#define BATTERY_INTERNAL_RESISTANCE_OHM 0.150f

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                               Globals                                   */
/***************************************************************************/

// Pointer to the class instance. Used by static IRQ callback to pass data to the object instance.
Power *Power::objectPtr;

// Voltage table used to estimate battery charge level
const int32_t Power::voltageTable[VOLTAGE_TABLE_ENTRIES] = {3000, 3650, 3700, 3740, 3760, 3795, 3840, 3910, 3980, 4070, 4150};

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

// Class constructor
Power::Power() : buttonCallback(nullptr), firstBatteryQuery(true)
{
}

// Class destructor
Power::~Power()
{
}

bool Power::Init()
{
    objectPtr = this;

CONSOLE.print("Initializing power: ");
    if (!power) {
        power = new XPowersAXP2101(PMU_WIRE_PORT);
        if (!power->init()) {
            CONSOLE.print("AXP2101 ... not detected, ");
            delete power;
            power = NULL;
        } else {
            CONSOLE.println("AXP2101 ... Success");
        }
    }

    if (!power) {
        power = new XPowersAXP192(PMU_WIRE_PORT);
        if (!power->init()) {
            CONSOLE.println("AXP192 ... not detected. No PMU");
            delete power;
            power = NULL;
        } else {
            CONSOLE.println("AXP192 ... Success");
        }
    }

    if (!power) {
        return false;
    }

    power->setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);

    if (power->getChipModel() == XPOWERS_AXP192) {

        power->setProtectedChannel(   XPOWERS_DCDC3); // ESP32

        power->setPowerChannelVoltage(XPOWERS_LDO2, 3300); // RF
        power->enablePowerOutput(     XPOWERS_LDO2);
        power->setPowerChannelVoltage(XPOWERS_LDO3, 3300); // GNSS
        power->enablePowerOutput(     XPOWERS_LDO3);

        power->setPowerChannelVoltage(XPOWERS_DCDC1, 3300); // I2C OLED
        power->setProtectedChannel(   XPOWERS_DCDC1);
        power->enablePowerOutput(     XPOWERS_DCDC1);

        //disable unused channels
        power->disablePowerOutput(XPOWERS_DCDC2);

        power->disableIRQ(XPOWERS_AXP192_ALL_IRQ);
        power->enableIRQ( // XPOWERS_AXP192_VBUS_REMOVE_IRQ  | XPOWERS_AXP192_VBUS_INSERT_IRQ |
                        // XPOWERS_AXP192_BAT_CHG_DONE_IRQ | XPOWERS_AXP192_BAT_CHG_START_IRQ |
                        // XPOWERS_AXP192_BAT_REMOVE_IRQ   | XPOWERS_AXP192_BAT_INSERT_IRQ |
                       XPOWERS_AXP192_PKEY_SHORT_IRQ   | XPOWERS_AXP192_PKEY_LONG_IRQ  //POWER KEY
                      );

    } else if (power->getChipModel() == XPOWERS_AXP2101) {

        //Unused power channels
        power->disablePowerOutput(XPOWERS_DCDC2);
        power->disablePowerOutput(XPOWERS_DCDC3);
        power->disablePowerOutput(XPOWERS_DCDC4);
        power->disablePowerOutput(XPOWERS_DCDC5);
        power->disablePowerOutput(XPOWERS_ALDO1);
        power->disablePowerOutput(XPOWERS_ALDO4);
        power->disablePowerOutput(XPOWERS_BLDO1);
        power->disablePowerOutput(XPOWERS_BLDO2);
        power->disablePowerOutput(XPOWERS_DLDO1);
        power->disablePowerOutput(XPOWERS_DLDO2);

        // GNSS RTC PowerVDD 3300mV
        power->setPowerChannelVoltage(XPOWERS_VBACKUP, 3300);
        power->enablePowerOutput(XPOWERS_VBACKUP);

        //ESP32 VDD 3300mV
        // ! No need to set, automatically open , Don't close it
        // power->setPowerChannelVoltage(XPOWERS_DCDC1, 3300);
        // power->setProtectedChannel(XPOWERS_DCDC1);
        power->setProtectedChannel(XPOWERS_DCDC1);

        // RF VDD 3300mV
        power->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
        power->enablePowerOutput(XPOWERS_ALDO2);

        //GNSS VDD 3300mV
        power->setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
        power->enablePowerOutput(XPOWERS_ALDO3);

        // Disable all interrupts
        power->disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
        // Clear all interrupt flags
        power->clearIrqStatus();
        // Enable the required interrupt function
        power->enableIRQ(
            // XPOWERS_AXP2101_BAT_INSERT_IRQ    | XPOWERS_AXP2101_BAT_REMOVE_IRQ      |   //BATTERY
            // XPOWERS_AXP2101_VBUS_INSERT_IRQ   | XPOWERS_AXP2101_VBUS_REMOVE_IRQ     |   //VBUS
            XPOWERS_AXP2101_PKEY_SHORT_IRQ    | XPOWERS_AXP2101_PKEY_LONG_IRQ          //POWER KEY
            // XPOWERS_AXP2101_BAT_CHG_DONE_IRQ  | XPOWERS_AXP2101_BAT_CHG_START_IRQ       //CHARGE
            // XPOWERS_AXP2101_PKEY_NEGATIVE_IRQ | XPOWERS_AXP2101_PKEY_POSITIVE_IRQ   |   //POWER KEY
            );

#if defined(T_BEAM_S3_SUPREME)

// it seems these chips are always connected, not managed by the PMU: QMI8658 6-axis IMU + PCF8563 Real-time clock/calendar

        power->setPowerChannelVoltage(XPOWERS_ALDO4, 3300); // GNSS
        power->enablePowerOutput(XPOWERS_ALDO4);

        power->setPowerChannelVoltage(XPOWERS_ALDO3, 3300); // RF
        power->enablePowerOutput(XPOWERS_ALDO3);

        // In order to avoid bus occupation, during initialization, the SD card and QMC sensor are powered off and restarted
        if (ESP_SLEEP_WAKEUP_UNDEFINED == esp_sleep_get_wakeup_cause()) {
            CONSOLE.println("Power off and restart ALDO BLDO..");
            power->disablePowerOutput(XPOWERS_ALDO1);
            power->disablePowerOutput(XPOWERS_ALDO2);
            power->disablePowerOutput(XPOWERS_BLDO1);
            delay(250);
        }

        power->setPowerChannelVoltage(XPOWERS_ALDO1, 3300); // BME280 barometer/altimeter + QMC6310 magnetometer/compass
        power->enablePowerOutput(XPOWERS_ALDO1);

        power->setPowerChannelVoltage(XPOWERS_ALDO2, 3300); // ??
        power->enablePowerOutput(XPOWERS_ALDO2);

        power->setPowerChannelVoltage(XPOWERS_BLDO1, 3300); // SD Card
        power->enablePowerOutput(XPOWERS_BLDO1);

        power->setPowerChannelVoltage(XPOWERS_BLDO2, 3300); // OLED
        power->enablePowerOutput(XPOWERS_BLDO2);

        power->setPowerChannelVoltage(XPOWERS_DCDC3, 3300); // Face M.2
        power->enablePowerOutput(XPOWERS_DCDC3);

        power->setPowerChannelVoltage(XPOWERS_DCDC4, XPOWERS_AXP2101_DCDC4_VOL2_MAX); //
        power->enablePowerOutput(XPOWERS_DCDC4);

        power->setPowerChannelVoltage(XPOWERS_DCDC5, 3300); //
        power->enablePowerOutput(XPOWERS_DCDC5);

        //Unused power channels
        power->disablePowerOutput(XPOWERS_DCDC2);
        power->disablePowerOutput(XPOWERS_DLDO1);
        power->disablePowerOutput(XPOWERS_DLDO2);
        power->disablePowerOutput(XPOWERS_VBACKUP);

        // Set constant current charge current limit
        power->setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_500MA);

        // Set charge cut-off voltage
        power->setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V2);

        // Disable all interrupts
        power->disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
        // Clear all interrupt flags
        power->clearIrqStatus();
        // Enable the required interrupt function
        power->enableIRQ(
            // XPOWERS_AXP2101_BAT_INSERT_IRQ    | XPOWERS_AXP2101_BAT_REMOVE_IRQ      |   //BATTERY
            // XPOWERS_AXP2101_VBUS_INSERT_IRQ   | XPOWERS_AXP2101_VBUS_REMOVE_IRQ     |   //VBUS
            XPOWERS_AXP2101_PKEY_SHORT_IRQ    | XPOWERS_AXP2101_PKEY_LONG_IRQ       // |   //POWER KEY
            // XPOWERS_AXP2101_BAT_CHG_DONE_IRQ  | XPOWERS_AXP2101_BAT_CHG_START_IRQ       //CHARGE
            // XPOWERS_AXP2101_PKEY_NEGATIVE_IRQ | XPOWERS_AXP2101_PKEY_POSITIVE_IRQ   |   //POWER KEY
        );
#endif // T_BEAM_S3_SUPREME

    } // AXP2101

    power->enableSystemVoltageMeasure();
    power->enableVbusVoltageMeasure();
    power->enableBattVoltageMeasure();

    // Set the time of pressing the button to turn off
    power->setPowerKeyPressOffTime(XPOWERS_POWEROFF_4S);

    powerEventGroup = xEventGroupCreate();
    xTaskCreate(StaticProcessingTask, "PowerTask", 16384, (void *)this, 6, &powerTaskHandle);

    pinMode(PMU_IRQ, INPUT /* INPUT_PULLUP */);
    attachInterrupt(PMU_IRQ, StaticIrqCallback, FALLING);

    return true;
}

void Power::RegisterButtonCallback(ButtonCallback_t callback)
{
    buttonCallback = callback;
}

void Power::Shutdown()
{
    xEventGroupSetBits(powerEventGroup, POWER_EVENT_SHUTDOWN);
}

PowerStatus_t &Power::GetStatus()
{
    return powerStatus;
}

/*
  Static entry point of the command processing task
  @param callingObject Pointer to the calling PanelManager instance
*/
void Power::StaticProcessingTask(void *callingObject)
{
    // Task entry points are static -> switch to non static processing method
    ((Power *)callingObject)->ProcessingTask();
}

void Power::ProcessingTask()
{
    while (true)
    {
        // Wait for the next command
        EventBits_t commandFlags = xEventGroupWaitBits(powerEventGroup, POWER_EVENT_ALL, pdTRUE, pdFALSE, TASK_WAKEUP_PERIOD_MS / portTICK_PERIOD_MS);

        if (commandFlags & POWER_EVENT_SHUTDOWN)
        {
            CommandShutdown();
        }
        if (commandFlags & POWER_EVENT_IRQ)
        {
            power->getIrqStatus();

            if (buttonCallback != nullptr)
            {
                if (power->isPekeyShortPressIrq())
                {
                    buttonCallback(false);
                }
                if (power->isPekeyLongPressIrq())
                {
                    buttonCallback(true);
                }
            }

            power->clearIrqStatus();
        }

        UpdateStatus();
    }
}

void IRAM_ATTR Power::StaticIrqCallback()
{
    BaseType_t scheduleChange = pdFALSE;

    xEventGroupSetBitsFromISR(objectPtr->powerEventGroup, POWER_EVENT_IRQ, &scheduleChange);
    portYIELD_FROM_ISR(scheduleChange);
}

void Power::UpdateStatus()
{
    float batVoltage_V  = power->getBattVoltage() / 1000.0f;
    float batCurrent_mA = 0; //FGA hack AXPDriver->getBatteryChargeCurrent() - AXPDriver->getBattDischargeCurrent();

    powerStatus.batteryConnected = power->isBatteryConnect();
    powerStatus.batteryCharging  = power->isCharging();
    powerStatus.usbConnected     = power->isVbusIn();

    if (firstBatteryQuery)
    {
        firstBatteryQuery             = false;
        powerStatus.batteryVoltage_V  = batVoltage_V;
        powerStatus.batteryCurrent_mA = batCurrent_mA;
        powerStatus.batteryLevel_per  = GetBatteryLevel(batVoltage_V, batCurrent_mA);
        powerStatus.usbVoltage_V      = power->getVbusVoltage();
        powerStatus.usbCurrent_mA     = 0; // FGA hack AXPDriver->getVbusCurrent();
        powerStatus.temperature_C     = 0; // FGA hack power->getTemperature();
    }
    else
    {
        powerStatus.batteryVoltage_V = (VOLTAGE_FILTERING_FACTOR * powerStatus.batteryVoltage_V) + ((1.0f - VOLTAGE_FILTERING_FACTOR) * batVoltage_V);
        powerStatus.batteryCurrent_mA =
            (CURRENT_FILTERING_FACTOR * powerStatus.batteryCurrent_mA) + ((1.0f - CURRENT_FILTERING_FACTOR) * batCurrent_mA);
        powerStatus.usbVoltage_V =
            (VOLTAGE_FILTERING_FACTOR * powerStatus.usbVoltage_V) + ((1.0f - VOLTAGE_FILTERING_FACTOR) * power->getVbusVoltage() / 1000.0f);
        powerStatus.usbCurrent_mA =
            0.0;
            // FGA hack (CURRENT_FILTERING_FACTOR * powerStatus.usbCurrent_mA) + ((1.0f - CURRENT_FILTERING_FACTOR) * power->getVbusCurrent());
        powerStatus.temperature_C =
            0.0;
            // FGA hack (TEMPERATURE_FILTERING_FACTOR * powerStatus.temperature_C) + ((1.0f - TEMPERATURE_FILTERING_FACTOR) * power->getTemperature());
        powerStatus.batteryLevel_per =
            BATTERY_LEVEL_FILTERING_FACTOR * powerStatus.batteryLevel_per +
            (1.0f - BATTERY_LEVEL_FILTERING_FACTOR) * GetBatteryLevel(powerStatus.batteryVoltage_V, powerStatus.batteryCurrent_mA);
    }
}

void Power::CommandShutdown()
{
    power->setChargingLedMode(XPOWERS_CHG_LED_OFF);

    power->disablePowerOutput(XPOWERS_VBACKUP); // power->disableButtonBatteryCharge();

    power->disablePowerOutput(XPOWERS_ALDO2);
    power->disablePowerOutput(XPOWERS_ALDO3);

    delay(20);

    /*
    * Complete power off
    *
    * to power back on either:
    * - press and hold PWR button for 1-2 seconds then release, or
    * - cycle micro-USB power
    */
    power->shutdown();
}

uint16_t Power::GetBatteryLevel(float voltage_V, float current_mA)
{
    // if (!power->isBatteryConnect())
    // {
    //     return -1;
    // }
    
    int32_t correctedVoltage_mV = voltage_V * 1000.0f - BATTERY_INTERNAL_RESISTANCE_OHM * current_mA;

    if (correctedVoltage_mV < voltageTable[0])
        return 0;
    
    for (int i = 1; i < VOLTAGE_TABLE_ENTRIES; i++)
    {
        if (correctedVoltage_mV < voltageTable[i])
            return i * (100 / (VOLTAGE_TABLE_ENTRIES - 1)) - ((VOLTAGE_TABLE_ENTRIES - 1) * (voltageTable[i] - correctedVoltage_mV)) / (voltageTable[i] - voltageTable[i - 1]);
    }

    return 100;
}
