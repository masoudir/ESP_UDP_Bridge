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
 * @file bridge.h
 * ESP8266 Wifi AP, UART/UDP Bridge
 *
 * @author Masoud Iranmehr <masoud.iranmehr@gmail.com>
 */

#ifndef BRIDGE_H
#define BRIDGE_H

#include "common.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

//-- UDP Outgoing Packet Queue
#define UAS_QUEUE_SIZE          60
#define UAS_QUEUE_THRESHOLD     20
#define UAS_QUEUE_TIMEOUT       5 // 5ms


class ESP8266Bridge
{
public:
    ESP8266Bridge();

    void        begin(IPAddress gcsIP, UINT16 udpHPort, UINT16 udpCPort, UINT32 serial_baudRate);
    void        udp_readMessageRaw();
    UINT32      udp_sendMessageRaw(UINT8 *buffer, UINT32 len);
    void        serial_readMessageRaw  ();
    UINT32      serial_sendMessageRaw  (UINT8 *buffer, UINT32 len);

private:
    UINT16      _baudrate;
    bool        _receivePermission;

private:
    WiFiUDP     _udp;
    IPAddress   _ip;
    UINT16      _udp_port;
};

#endif
