/***************************************************************************
 *                                                                         *
 * Project:  MicroNav                                                      *
 * Purpose:  API to encode / decode Micronet message                       *
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

#ifndef MICRONETCODEC_H_
#define MICRONETCODEC_H_

/***************************************************************************/
/*                              Includes                                   */
/***************************************************************************/

#include <stdint.h>

#include "Micronet.h"
#include "NavigationData.h"

/***************************************************************************/
/*                              Constants                                  */
/***************************************************************************/

#define DATA_FIELD_TIME      0x00000001
#define DATA_FIELD_DATE      0x00000002
#define DATA_FIELD_SOGCOG    0x00000004
#define DATA_FIELD_POSITION  0x00000008
#define DATA_FIELD_XTE       0x00000010
#define DATA_FIELD_DTW       0x00000020
#define DATA_FIELD_BTW       0x00000040
#define DATA_FIELD_VMGWP     0x00000080
#define DATA_FIELD_HDG       0x00000100
#define DATA_FIELD_NODE_INFO 0x00000200
#define DATA_FIELD_AWS       0x00000400
#define DATA_FIELD_AWA       0x00000800
#define DATA_FIELD_DPT       0x00001000
#define DATA_FIELD_SPD       0x00002000

/***************************************************************************/
/*                                Types                                    */
/***************************************************************************/

typedef struct
{
    uint32_t deviceId;
    uint32_t start_us;
    uint32_t length_us;
    uint8_t  payloadBytes;
} TxSlotDesc_t;

struct NetworkMap_t
{
    uint32_t     networkId;
    uint32_t     nbDevices;
    uint32_t     masterDevice;
    uint32_t     networkStart;
    uint32_t     networkEnd;
    uint32_t     firstSlot;
    uint32_t     nbSyncSlots;
    TxSlotDesc_t syncSlot[MICRONET_MAX_DEVICES_PER_NETWORK];
    TxSlotDesc_t asyncSlot;
    uint32_t     nbAckSlots;
    TxSlotDesc_t ackSlot[MICRONET_MAX_DEVICES_PER_NETWORK];
};

typedef struct
{
    bool  batteryPresent;
    bool  batteryCharging;
    bool  powerConnected;
    float batteryLevel;
} SystemInfo_t;

/***************************************************************************/
/*                               Classes                                   */
/***************************************************************************/

class MicronetCodec
{
  public:
    NavigationData navData;

    MicronetCodec();
    MicronetCodec(uint8_t swMajorVersion, uint8_t swMinorVersion);
    virtual ~MicronetCodec();

    void     SetSwVersion(uint8_t swMajorVersion, uint8_t swMinorVersion);
    uint32_t GetNetworkId(MicronetMessage_t *message);
    uint8_t  GetDeviceType(MicronetMessage_t *message);
    uint32_t GetDeviceId(MicronetMessage_t *message);
    uint8_t  GetMessageId(MicronetMessage_t *message);
    uint8_t  GetSource(MicronetMessage_t *message);
    uint8_t  GetSignalStrength(MicronetMessage_t *message);
    uint8_t  GetHeaderCrc(MicronetMessage_t *message);
    bool     VerifyHeaderCrc(MicronetMessage_t *message);

    bool         DecodeMessage(MicronetMessage_t *message);
    bool         GetNetworkMap(MicronetMessage_t *message, NetworkMap_t *networkMap);
    TxSlotDesc_t GetSyncTransmissionSlot(NetworkMap_t *networkMap, uint32_t deviceId);
    TxSlotDesc_t GetAsyncTransmissionSlot(NetworkMap_t *networkMap);
    TxSlotDesc_t GetAckTransmissionSlot(NetworkMap_t *networkMap, uint32_t deviceId);
    uint32_t     GetStartOfNetwork(NetworkMap_t *networkMap);
    uint32_t     GetNextStartOfNetwork(NetworkMap_t *networkMap);
    uint32_t     GetEndOfNetwork(NetworkMap_t *networkMap);
    uint8_t      CalculateSignalStrength(MicronetMessage_t *message);
    float        CalculateSignalFloatStrength(MicronetMessage_t *message);
    uint8_t      GetDataMessageLength(uint32_t dataFields);
    uint8_t      EncodeDataMessage(MicronetMessage_t *message, uint8_t signalStrength, uint32_t networkId, uint32_t deviceId, uint32_t dataFields);
    uint8_t      EncodeSlotRequestMessage(MicronetMessage_t *message, uint8_t signalStrength, uint32_t networkId, uint32_t deviceId,
                                          uint8_t payloadLength);
    uint8_t EncodeSlotUpdateMessage(MicronetMessage_t *message, uint8_t signalStrength, uint32_t networkId, uint32_t deviceId, uint8_t payloadLength);
    uint8_t EncodeResetMessage(MicronetMessage_t *message, uint8_t signalStrength, uint32_t networkId, uint32_t deviceId);
    uint8_t EncodeAckParamMessage(MicronetMessage_t *message, uint8_t signalStrength, uint32_t networkId, uint32_t deviceId);
    uint8_t EncodePingMessage(MicronetMessage_t *message, uint8_t signalStrength, uint32_t networkId, uint32_t deviceId);
    uint8_t EncodeAlertMessage(MicronetMessage_t *message, uint8_t signalStrength, uint32_t networkId, uint32_t deviceId, uint8_t alertID);
    void    CalculateTrueWind();

    void SetSystemInfo(SystemInfo_t &systemInfo);

  private:
    uint8_t      swMajorVersion;
    uint8_t      swMinorVersion;
    SystemInfo_t systemInfo;

    void    DecodeSendDataMessage(MicronetMessage_t *message);
    void    DecodeSendCommandMessage(MicronetMessage_t *message);
    void    DecodeSetConfigParameter(MicronetMessage_t *message);
    int     DecodeDataField(MicronetMessage_t *message, int offset);
    void    UpdateMicronetData(uint8_t fieldId, int8_t value);
    void    UpdateMicronetData(uint8_t fieldId, int16_t value);
    void    UpdateMicronetData(uint8_t fieldId, int32_t value1, int32_t value2);
    void    WriteHeaderLengthAndCrc(MicronetMessage_t *message);
    uint8_t AddPositionField(uint8_t *buffer, float latitude, float longitude);
    uint8_t Add16bitField(uint8_t *buffer, uint8_t fieldCode, int16_t value);
    uint8_t AddDual16bitField(uint8_t *buffer, uint8_t fieldCode, int16_t value1, int16_t value2);
    uint8_t AddQuad8bitField(uint8_t *buffer, uint8_t fieldCode, uint8_t value1, uint8_t value2, uint8_t value3, uint8_t value4);
    uint8_t Add16bitAndSix8bitField(uint8_t *buffer, uint8_t fieldCode, int16_t value1, uint8_t const *wpName, uint8_t wpNameLength);
    uint8_t Add24bitField(uint8_t *buffer, uint8_t fieldCode, int32_t value);
    uint8_t Add32bitField(uint8_t *buffer, uint8_t fieldCode, int32_t value);
};

/***************************************************************************/
/*                              Prototypes                                 */
/***************************************************************************/

#endif /* MICRONETCODEC_H_ */
