/****************************************************************************
 *
 * Copyright (c) 2020, 2021 Masoud Iranmehr. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file bridge.cpp
 * ESP8266 Wifi AP, UART/UDP Bridge
 *
 * @author Masoud Iranmehr <masoud.iranmehr@gmail.com>
 */

#include "bridge.h"
#include "parameters.h"

//---------------------------------------------------------------------------------
ESP8266Bridge::ESP8266Bridge()
    : _udp_port(DEFAULT_UDP_HPORT), _baudrate(DEFAULT_UART_SPEED)
{
    //    memset(&_buf, 0, sizeof(_buf));
}

//---------------------------------------------------------------------------------
//-- Initialize
void ESP8266Bridge::begin(IPAddress gcsIP, UINT16 udpHPort, UINT16 udpCPort, UINT32 serial_baudRate)
{
    // UDP Begin
    {
        _ip = gcsIP;
        //-- Init variables that shouldn't change unless we reboot
        _udp_port = udpHPort;
        //-- Start UDP
        _udp.begin(udpCPort);
    }

    // Serial Begin
    {
        //-- Start UART connected to UAS
        Serial.begin(serial_baudRate);
//-- Swap to TXD2/RXD2 (GPIO015/GPIO013) For ESP12 Only
#ifdef ENABLE_DEBUG
#ifdef ARDUINO_ESP8266_ESP12
        Serial.swap();
#endif
#endif
        // raise serial buffer size (default is 256)
        Serial.setRxBufferSize(DEFAULT_RECEVE_BUFFER_SIZE);
    }
}

//---------------------------------------------------------------------------------
//-- Read message from GCS
void ESP8266Bridge::udp_readMessageRaw()
{
    int udp_count = _udp.parsePacket();
    UINT8 buf[DEFAULT_RECEVE_BUFFER_SIZE];
    int buf_index = 0;

    if (udp_count > 0)
    {
        while (buf_index < udp_count)
        {
            int result = _udp.read();
            if (result >= 0)
            {
                buf[buf_index] = result;
                buf_index++;
            }
        }
        serial_sendMessageRaw(buf, buf_index);
    }
}

//---------------------------------------------------------------------------------
//-- Forward message(s) to the GCS
UINT32 ESP8266Bridge::udp_sendMessageRaw(UINT8 *buffer, UINT32 len)
{
    // if(getWifiMode() == WIFI_MODE_AP)
    // {
    //     _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
    // }
    // else
    // {
    //         // IPAddress broadcastIp(192,168,1,255);
    //         // _udp.beginPacket(broadcastIp, 14550);
    //         // _udp.beginPacketMulticast(broadcastIp, 14550, WiFi.localIP());
    //      _udp.beginPacket(_ip, _udp_port);
    // }

    _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());

    size_t sent = _udp.write(buffer, len);
    _udp.endPacket();
    _udp.flush();
    return sent;
}

void ESP8266Bridge::serial_readMessageRaw()
{
    UINT8 buf[1024];
    UINT16 buf_index = 0;

    // while(Serial.available() && _receivePermission)
    // {
    //     buf_index = Serial.readBytesUntil(0,buf,DEFAULT_RECEVE_BUFFER_SIZE);
    //     buf[buf_index] = 0;
    //     if(buf_index > 0)
    //     {
    //          _forwardTo->sendMessageRaw((UINT8*)buf, buf_index);
    //     }
    // }

    if (Serial.available())
    {
        while (Serial.available())
        {
            int dat = Serial.read();
            if (dat > 0)
            {
                buf[buf_index] = dat;
                buf_index++;
            }
        }

        buf[buf_index] = 0;
        if (buf_index > 0)
        {
            udp_sendMessageRaw(buf, buf_index);
        }
    }
}

//---------------------------------------------------------------------------------
//-- Send message to UAS
UINT32 ESP8266Bridge::serial_sendMessageRaw(UINT8 *buffer, UINT32 len)
{
    Serial.write(buffer, len);
    //Serial.flush();
    return len;
}
