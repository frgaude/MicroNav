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

#ifndef SX1276MNETDRIVER_H_
#define SX1276MNETDRIVER_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include <Arduino.h>
#include <SPI.h>

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define SX1276_FIFO_MAX_SIZE 64

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class SX1276MnetDriver {
public:
  SX1276MnetDriver();
  ~SX1276MnetDriver();

  bool Init(uint32_t sckPin, uint32_t mosiPin, uint32_t miso_Pin,
            uint32_t csPin, uint32_t dio0Pin, uint32_t dio1Pin,
            uint32_t rstPin);
  void SetFrequency(float frequency);
  void SetBandwidth(float bandwidth);
  void SetBitrate(float bitrate);
  void SetDeviation(float deviation);
  void StartTx(void);
  void StartRx(void);
  int32_t GetRssi(void);
  void GoToIdle(void);
  void LowPower();
  void ActivePower();
  void SetSyncByte(uint8_t syncByte);
  void SetPacketLength(uint8_t length);
  void ReadFifo(uint8_t *buffer, int nbBytes);
  void WriteFifo(uint8_t data);
  void WriteFifo(uint8_t const *buffer, int nbBytes);
  void IrqOnTxFifoUnderflow();
  void IrqOnTxFifoThreshold();
  void IrqOnRxFifoThreshold();
  void SetFifoThreshold(uint8_t fifoThreshold);
  void FlushFifo();

private:
  SPISettings spiSettings;
  uint32_t sckPin, mosiPin, miso_Pin, csPin, dio0Pin, dio1Pin, rstPin;
  TaskHandle_t DioTaskHandle;

  uint8_t SpiReadRegister(uint8_t addr);
  void SpiBurstReadRegister(uint8_t addr, uint8_t *data, uint16_t length);
  void SpiWriteRegister(uint8_t addr, uint8_t value);
  void SpiBurstWriteRegister(uint8_t addr, uint8_t *data, uint16_t length);

  void Reset();
  void SetBaseConfiguration();
  uint8_t CalculateBandwidthRegister(float bandwidth);
  void ExtendedPinMode(int pinNum, int pinDir);

  static SX1276MnetDriver *driverObject;
  static void StaticRfIsr();
  static void DioTask(void *parameter);

  void IsrProcessing();
};

#endif