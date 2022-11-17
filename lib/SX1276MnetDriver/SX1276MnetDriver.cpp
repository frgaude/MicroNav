/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  Driver for SX1276                                             *
 * Author:   Ronan Demoment                                                *
 *           heavily based on RadioLib driver by Jan Gromes                *
 *           (https://github.com/jgromes/RadioLib)                         *
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

#include "SX1276MnetDriver.h"
#include "SX1276Regs.h"

#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>
#include <cmath>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define SPI_WRITE_COMMAND 0x80

/***************************************************************************/
/*                             Local types                                 */
/***************************************************************************/

/***************************************************************************/
/*                           Local prototypes                              */
/***************************************************************************/

/***************************************************************************/
/*                           Static & Globals                              */
/***************************************************************************/

SX1276MnetDriver *SX1276MnetDriver::driverObject;

/***************************************************************************/
/*                              Functions                                  */
/***************************************************************************/

/*
 * Constructor of SX1276MnetDriver
 * Initialize class attributes, SPI HW and pins
 */
SX1276MnetDriver::SX1276MnetDriver() { driverObject = this; }

/*
 * Destructor of SX1276MnetDriver
 * Release SPI bus and reset pin config
 */
SX1276MnetDriver::~SX1276MnetDriver() {}

/*
  Initialize SX1276 Driver for Micronet RF communications
  @param sckPin Pin number of SX1276 SPI clock
  @param mosiPin Pin number of SX1276 SPI MOSI
  @param miso_Pin Pin number of SX1276 SPI MISO
  @param csPin Pin number of SX1276 SPI CS
  @param dio0Pin Pin number of SX1276 DIO0 digital IO
  @param dio1Pin Pin number of SX1276 DIO1 digital IO
  @param rstPin Pin number of SX1276 Reset line
  @return true if SX1276 has been found on SPI bus and configured
*/
bool SX1276MnetDriver::Init(uint32_t sckPin, uint32_t mosiPin,
                            uint32_t miso_Pin, uint32_t csPin, uint32_t dio0Pin,
                            uint32_t dio1Pin, uint32_t rstPin) {
  // Store pin configuration
  this->sckPin = sckPin;
  this->mosiPin = mosiPin;
  this->miso_Pin = miso_Pin;
  this->csPin = csPin;
  this->dio0Pin = dio0Pin;
  this->dio1Pin = dio1Pin;
  this->rstPin = rstPin;

  // Configure pin directions
  ExtendedPinMode(sckPin, OUTPUT);
  ExtendedPinMode(mosiPin, OUTPUT);
  ExtendedPinMode(miso_Pin, INPUT);
  ExtendedPinMode(csPin, OUTPUT);
  ExtendedPinMode(dio0Pin, INPUT);
  ExtendedPinMode(dio1Pin, INPUT);

  // Pin startup state
  digitalWrite(csPin, HIGH);
  digitalWrite(sckPin, HIGH);
  digitalWrite(mosiPin, LOW);

  // Start SPI driver
  SPI.begin(sckPin, miso_Pin, mosiPin, csPin);
  // In  the context of MicronetToNMEA, only SX1276 is alone on its SPI bus, so
  // we can request SPI access here once for all here In case SX1276 would share
  // the SPI bus with other ICs, beginTransaction and endTransaction should be
  // moved into each SPI access member to avoid race conditions.
  SPI.beginTransaction(spiSettings);

  // Check presence of SX1276 on SPI bus
  if (SpiReadRegister(SX127X_REG_VERSION) != SX1278_CHIP_VERSION) {
    return false;
  }

  // Reset SX1276
  Reset();

  // Set base configuration for Micronet configuration
  SetBaseConfiguration();

  xTaskCreate(DioTask, "DioTask", 1024, (void *)this,
              (configMAX_PRIORITIES - 1), &DioTaskHandle);

  // Attach callback to DIO1 pin
  attachInterrupt(digitalPinToInterrupt(dio1Pin), StaticRfIsr, RISING);

  return true;
}

/*
  Set down converter frequency
  @param frequency Frequency in MHz
*/
void SX1276MnetDriver::SetFrequency(float frequency) {
  uint32_t freqIndex =
      (frequency * (uint32_t(1) << SX127X_DIV_EXPONENT)) / SX127X_CRYSTAL_FREQ;

  SpiWriteRegister(RADIOLIB_SX127X_REG_FRF_MSB, (freqIndex >> 16) & 0xff);
  SpiWriteRegister(RADIOLIB_SX127X_REG_FRF_MID, (freqIndex >> 8) & 0xff);
  SpiWriteRegister(RADIOLIB_SX127X_REG_FRF_LSB, freqIndex & 0xff);
}

/*
  Set demodulator RX bandwidth
  @param bandwidth Bandwidth in kHz
*/
void SX1276MnetDriver::SetBandwidth(float bandwidth) {

  SpiWriteRegister(RADIOLIB_SX127X_REG_RX_BW,
                   CalculateBandwidthRegister(bandwidth));
}

/*
  Set demodulator bitrate
  @param bitrate Bitrate in bps
*/
void SX1276MnetDriver::SetBitrate(float birate) {
  // TODO : is it possible to set fractonal part of bitrate ?
  uint16_t bitRate = roundf((SX127X_CRYSTAL_FREQ * 1000000.0) / birate);
  SpiWriteRegister(SX127X_REG_BITRATE_MSB, (bitRate >> 8) & 0xff);
  SpiWriteRegister(SX127X_REG_BITRATE_LSB, bitRate & 0xff);
}

/*
  Set demodulator Frequency deviation in kHz
  @param freqDev Frequency deviation in kHz
*/
void SX1276MnetDriver::SetDeviation(float deviation) {
  uint32_t FDEV = roundf((deviation * (1 << 19)) / 32000.0);
  SpiWriteRegister(RADIOLIB_SX127X_REG_FDEV_MSB, (FDEV >> 8) & 0xff);
  SpiWriteRegister(RADIOLIB_SX127X_REG_FDEV_LSB, FDEV & 0xff);
}

/*
  Start RF transmission of FIFO data
*/
void SX1276MnetDriver::StartTx(void) {
  SpiWriteRegister(SX127X_REG_OP_MODE, SX127X_TX);
}

/*
  Start RF reception
*/
void SX1276MnetDriver::StartRx(void) {
  SpiWriteRegister(SX127X_REG_OP_MODE, SX127X_RX);
}

int32_t SX1276MnetDriver::GetRssi(void) {
  return SpiReadRegister(SX127X_REG_RSSI_VALUE_FSK);
}

/*
  Put SX1276 in idle (standby) mode
*/
void SX1276MnetDriver::GoToIdle(void) {
  SpiWriteRegister(SX127X_REG_OP_MODE, SX127X_STANDBY);
}

/*
  Put SX1276 in low power mode
*/
void SX1276MnetDriver::LowPower() {}

/*
  Put SX1276 in active power mode
*/
void SX1276MnetDriver::ActivePower() {}

/*
  Set packet decoder sync byte
  @param syncByte Sync byte
*/
void SX1276MnetDriver::SetSyncByte(uint8_t syncByte) {
  SpiWriteRegister(SX127X_REG_SYNC_VALUE_1, syncByte);
}

/*
  Set packet length
  @param length Packet length
*/
void SX1276MnetDriver::SetPacketLength(uint8_t length) {
  SpiWriteRegister(SX127X_REG_PAYLOAD_LENGTH_FSK, length);
}

void SX1276MnetDriver::ReadFifo(uint8_t *buffer, int nbBytes) {}

void SX1276MnetDriver::WriteFifo(uint8_t data) {}

void SX1276MnetDriver::WriteFifo(uint8_t const *buffer, int nbBytes) {}

void SX1276MnetDriver::IrqOnTxFifoUnderflow() {}

void SX1276MnetDriver::IrqOnTxFifoThreshold() {}

void SX1276MnetDriver::IrqOnRxFifoThreshold() {}

void SX1276MnetDriver::SetFifoThreshold(uint8_t fifoThreshold) {}

void SX1276MnetDriver::FlushFifo() {}

/*
  Read one SX1276 register
  @param addr Register address
  @return Value of the register
*/
uint8_t SX1276MnetDriver::SpiReadRegister(uint8_t addr) {
  uint8_t data;

  // Assert CS line
  digitalWrite(csPin, LOW);
  // Read register
  SPI.transfer(addr & 0x7f);
  data = SPI.transfer(0x00);
  // Release CS
  digitalWrite(csPin, HIGH);

  return data;
}

/*
  Read multiple SX1276 registers with a burst access
  @param addr Address of the first register
  @param data pointer to the buffer to store register data in
  @param nbRegs Number of registers to read
*/
void SX1276MnetDriver::SpiBurstReadRegister(uint8_t addr, uint8_t *data,
                                            uint16_t nbRegs) {
  // Assert CS line
  digitalWrite(csPin, LOW);
  // Read register
  SPI.transfer(addr & 0x7f);
  SPI.transferBytes(nullptr, data, nbRegs);
  // Release CS
  digitalWrite(csPin, HIGH);
}

/*
  Write one SX1276 register
  @param addr Register address
  @param value Value to be written
*/
void SX1276MnetDriver::SpiWriteRegister(uint8_t addr, uint8_t value) {
  // Assert CS line
  digitalWrite(csPin, LOW);
  // Write register
  SPI.transfer(SPI_WRITE_COMMAND | addr);
  SPI.transfer(value);
  // Release CS
  digitalWrite(csPin, HIGH);
}

/*
  Write multiple SX1276 registers with a burst access
  @param addr Address of the first register
  @param data pointer to the buffer with data to write to registers
  @param nbRegs Number of registers to write
*/
void SX1276MnetDriver::SpiBurstWriteRegister(uint8_t addr, uint8_t *data,
                                             uint16_t nbRegs) {
  // Assert CS line
  digitalWrite(csPin, LOW);
  // Read register
  SPI.transfer(SPI_WRITE_COMMAND | addr);
  SPI.transferBytes(data, nullptr, nbRegs);
  // Release CS
  digitalWrite(csPin, HIGH);
}

/*
  Reset SX1276
*/
void SX1276MnetDriver::Reset() {
  ExtendedPinMode(rstPin, OUTPUT);
  digitalWrite(rstPin, LOW);
  delay(1);
  digitalWrite(rstPin, HIGH);
  delay(5);
}

/*
  Set SX1276 base configuration for Micronet operation
*/
void SX1276MnetDriver::SetBaseConfiguration() {
  SpiWriteRegister(SX127X_REG_OP_MODE, SX127X_SLEEP);
  SpiWriteRegister(SX127X_REG_OP_MODE, SX127X_STANDBY);
  SetBitrate(76.8f);
  SetDeviation(38.0f);
  SetBandwidth(225.0f);
  // Preamble of 16 bytes for TX operations
  SpiWriteRegister(SX127X_REG_PREAMBLE_MSB_FSK, 0);
  SpiWriteRegister(SX127X_REG_PREAMBLE_LSB_FSK, 16);
  // Sync word detection ON, polarity of 0x55, 1 byte long, 0x99 value
  SpiWriteRegister(SX127X_REG_SYNC_CONFIG,
                   SX127X_PREAMBLE_POLARITY_55 | SX127X_SYNC_ON);
  SpiWriteRegister(SX127X_REG_SYNC_VALUE_1, 0x99);
  // Fixed length packet : 60 bytes, no DC encoding, no adress filtering
  SpiWriteRegister(SX127X_REG_PACKET_CONFIG_1, 0);
  SpiWriteRegister(SX127X_REG_PACKET_CONFIG_2, SX127X_DATA_MODE_PACKET);
  SpiWriteRegister(SX127X_REG_NODE_ADRS, 0x00);
  SpiWriteRegister(SX127X_REG_BROADCAST_ADRS, 0x00);
  SpiWriteRegister(SX127X_REG_PAYLOAD_LENGTH_FSK, 60);
  // Minimum RSSI smoothing
  SpiWriteRegister(SX127X_REG_RSSI_CONFIG, 0);
  // FIFO threshold set to 13 bytes and TX condition is !FifoEmpty
  SpiWriteRegister(SX127X_REG_FIFO_THRESH, 0x8d);
  // IRQ on PacketSend(TX) & FifoLevel (RX)
  SpiWriteRegister(SX127X_REG_DIO_MAPPING_1, 0x00);
}

/*
  Caluclate bandwidth register value
  @param bandwidth Bandwidth in kHz
*/
uint8_t SX1276MnetDriver::CalculateBandwidthRegister(float bandwidth) {
  for (uint8_t e = 7; e >= 1; e--) {
    for (int8_t m = 2; m >= 0; m--) {
      float point = (SX127X_CRYSTAL_FREQ * 1000000.0) /
                    (((4 * m) + 16) * ((uint32_t)1 << (e + 2)));
      if (fabs(bandwidth - ((point / 1000.0) + 0.05)) <= 0.5) {
        return ((m << 3) | e);
      }
    }
  }
  return 0;
}

void SX1276MnetDriver::ExtendedPinMode(int pinNum, int pinDir) {
  // Enable GPIO32 or 33 as output.
  if (pinNum == 32 || pinNum == 33) {
    uint64_t gpioBitMask =
        (pinNum == 32) ? 1ULL << GPIO_NUM_32 : 1ULL << GPIO_NUM_33;
    gpio_mode_t gpioMode =
        (pinDir == OUTPUT) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT;
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = gpioMode;
    io_conf.pin_bit_mask = gpioBitMask;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
  } else
    pinMode(pinNum, pinDir);
}

void SX1276MnetDriver::StaticRfIsr() {
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(driverObject->DioTaskHandle,
                         &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void SX1276MnetDriver::DioTask(void *parameter) {
  ((SX1276MnetDriver *)parameter)->IsrProcessing();
}

void SX1276MnetDriver::IsrProcessing() {
  BaseType_t xEvent;
  const TickType_t xBlockTime = pdMS_TO_TICKS(500);
  uint32_t ulNotifiedValue;

  while (true) {
    ulNotifiedValue = ulTaskNotifyTake(pdFALSE, xBlockTime);
    if (ulNotifiedValue > 0) {
      do {
        Serial.println(SpiReadRegister(SX127X_REG_FIFO), HEX);
      } while (SpiReadRegister(SX127X_REG_IRQ_FLAGS_2) & 0x40);

      // detachInterrupt(digitalPinToInterrupt(dio1Pin));
      // attachInterrupt(digitalPinToInterrupt(dio1Pin), StaticRfIsr, RISING);
    }
  }
}