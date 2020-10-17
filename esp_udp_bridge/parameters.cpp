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
 * @file parameters.cpp
 * ESP8266 Wifi AP, UART/UDP Bridge
 *
 * @author Masoud Iranmehr <masoud.iranmehr@gmail.com>
 */

#include "parameters.h"

//-- Reserved space for EEPROM persistence. A change in this will cause all values to reset to defaults.
#define EEPROM_SPACE 32 * sizeof(UINT32)
#define EEPROM_CRC_ADD EEPROM_SPACE - (sizeof(UINT32) << 1)

static UINT32 _flash_left;

const char *kDEFAULT_SSID = "EspUdp";
const char *kDEFAULT_PASSWORD = "bridge1234";


static UINT32 _sw_version;
static UINT8 _debug_enabled;
static UINT8 _wifi_mode;
static UINT32 _wifi_channel;
static UINT16 _wifi_udp_hport;
static UINT16 _wifi_udp_cport;
static char _wifi_ssid[16];
static char _wifi_password[16];
static char _wifi_ssidsta[16];
static char _wifi_passwordsta[16];
static UINT32 _wifi_ipsta;
static UINT32 _wifi_gatewaysta;
static UINT32 _wifi_subnetsta;
static UINT32 _uart_baud_rate;

String _wifi_ip_address;

UINT32 t;

//-- Parameters
//   No string support in parameters so we stash a char[16] into 4 UINT32
struct ParameterFields Parameters[] = {
    {"SW_VER", &_sw_version, ID_FWVER, sizeof(UINT32), PARAM_TYPE_UINT32, true},
    {"DEBUG_ENABLED", &_debug_enabled, ID_DEBUG, sizeof(UINT8), PARAM_TYPE_INT8, false},
    {"WIFI_MODE", &_wifi_mode, ID_MODE, sizeof(UINT8), PARAM_TYPE_INT8, false},
    {"WIFI_CHANNEL", &_wifi_channel, ID_CHANNEL, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_UDP_HPORT", &_wifi_udp_hport, ID_HPORT, sizeof(UINT16), PARAM_TYPE_UINT16, false},
    {"WIFI_UDP_CPORT", &_wifi_udp_cport, ID_CPORT, sizeof(UINT16), PARAM_TYPE_UINT16, false},
    {"WIFI_IPADDRESS", &t, ID_IPADDRESS, sizeof(UINT32), PARAM_TYPE_UINT32, true},
    {"WIFI_SSID1", &_wifi_ssid[0], ID_SSID1, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_SSID2", &_wifi_ssid[4], ID_SSID2, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_SSID3", &_wifi_ssid[8], ID_SSID3, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_SSID4", &_wifi_ssid[12], ID_SSID4, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_PASSWORD1", &_wifi_password[0], ID_PASS1, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_PASSWORD2", &_wifi_password[4], ID_PASS2, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_PASSWORD3", &_wifi_password[8], ID_PASS3, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_PASSWORD4", &_wifi_password[12], ID_PASS4, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_SSIDSTA1", &_wifi_ssidsta[0], ID_SSIDSTA1, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_SSIDSTA2", &_wifi_ssidsta[4], ID_SSIDSTA2, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_SSIDSTA3", &_wifi_ssidsta[8], ID_SSIDSTA3, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_SSIDSTA4", &_wifi_ssidsta[12], ID_SSIDSTA4, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_PWDSTA1", &_wifi_passwordsta[0], ID_PASSSTA1, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_PWDSTA2", &_wifi_passwordsta[4], ID_PASSSTA2, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_PWDSTA3", &_wifi_passwordsta[8], ID_PASSSTA3, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_PWDSTA4", &_wifi_passwordsta[12], ID_PASSSTA4, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_IPSTA", &_wifi_ipsta, ID_IPSTA, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_GATEWAYSTA", &_wifi_gatewaysta, ID_GATEWAYSTA, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"WIFI_SUBNET_STA", &_wifi_subnetsta, ID_SUBNETSTA, sizeof(UINT32), PARAM_TYPE_UINT32, false},
    {"UART_BAUDRATE", &_uart_baud_rate, ID_UART, sizeof(UINT32), PARAM_TYPE_UINT32, false}};

//---------------------------------------------------------------------------------
//-- Initialize
void setLocalIPAddress(String ipAddress)
{
    _wifi_ip_address = ipAddress;
}

String getLocalIPAddress() { return _wifi_ip_address; }

//---------------------------------------------------------------------------------
//-- Array accessor
ParameterFields *Param_getAt(int index)
{
    if (index < ID_COUNT)
        return &Parameters[index];
    else
        return NULL;
}

//---------------------------------------------------------------------------------
//-- Parameters
UINT32 getSwVersion() { return _sw_version; }
UINT8 getDebugEnabled() { return _debug_enabled; }
UINT8 getWifiMode() { return _wifi_mode; }
UINT32 getWifiChannel() { return _wifi_channel; }
UINT16 getWifiUdpHport() { return _wifi_udp_hport; }
UINT16 getWifiUdpCport() { return _wifi_udp_cport; }
char *getWifiSsid() { return _wifi_ssid; }
char *getWifiPassword() { return _wifi_password; }
char *getWifiStaSsid() { return _wifi_ssidsta; }
char *getWifiStaPassword() { return _wifi_passwordsta; }
UINT32 getWifiStaIP() { return _wifi_ipsta; }
UINT32 getWifiStaGateway() { return _wifi_gatewaysta; }
UINT32 getWifiStaSubnet() { return _wifi_subnetsta; }
UINT32 getUartBaudRate() { return _uart_baud_rate; }

//---------------------------------------------------------------------------------
//-- Reset all to defaults
void resetToDefaults()
{
    _sw_version = ESP_UDP_BRIDGE_VERSION;
    _debug_enabled = 0;
    _wifi_mode = DEFAULT_WIFI_MODE;
    _wifi_channel = DEFAULT_WIFI_CHANNEL;
    _wifi_udp_hport = DEFAULT_UDP_HPORT;
    _wifi_udp_cport = DEFAULT_UDP_CPORT;
    _uart_baud_rate = DEFAULT_UART_SPEED;
    _wifi_ipsta = 0;
    _wifi_gatewaysta = 0;
    _wifi_subnetsta = 0;
    strncpy(_wifi_ssid, kDEFAULT_SSID, sizeof(_wifi_ssid));
    strncpy(_wifi_password, kDEFAULT_PASSWORD, sizeof(_wifi_password));
    strncpy(_wifi_ssidsta, kDEFAULT_SSID, sizeof(_wifi_ssidsta));
    strncpy(_wifi_passwordsta, kDEFAULT_PASSWORD, sizeof(_wifi_passwordsta));
    Eeprom_deinit();
}

//---------------------------------------------------------------------------------
void setDebugEnabled(UINT8 enabled)
{
    _debug_enabled = enabled;
}

//---------------------------------------------------------------------------------
void setWifiMode(UINT8 mode)
{
    _wifi_mode = mode;
}

//---------------------------------------------------------------------------------
void setWifiChannel(UINT32 channel)
{
    _wifi_channel = channel;
}

//---------------------------------------------------------------------------------
void setWifiUdpHport(UINT16 port)
{
    _wifi_udp_hport = port;
}

//---------------------------------------------------------------------------------
void setWifiUdpCport(UINT16 port)
{
    _wifi_udp_cport = port;
}

//---------------------------------------------------------------------------------
void setWifiSsid(const char *ssid)
{
    strncpy(_wifi_ssid, ssid, sizeof(_wifi_ssid));
        Serial.println("ssid change");
    Serial.println(ssid);
}

//---------------------------------------------------------------------------------
void setWifiPassword(const char *pwd)
{
    strncpy(_wifi_password, pwd, sizeof(_wifi_password));
}

//---------------------------------------------------------------------------------
void setWifiStaSsid(const char *ssid)
{
    strncpy(_wifi_ssidsta, ssid, sizeof(_wifi_ssidsta));
}

//---------------------------------------------------------------------------------
void setWifiStaPassword(const char *pwd)
{
    strncpy(_wifi_passwordsta, pwd, sizeof(_wifi_passwordsta));
}

//---------------------------------------------------------------------------------
void setWifiStaIP(UINT32 addr)
{
    _wifi_ipsta = addr;
}

//---------------------------------------------------------------------------------
void setWifiStaGateway(UINT32 addr)
{
    _wifi_gatewaysta = addr;
}

//---------------------------------------------------------------------------------
void setWifiStaSubnet(UINT32 addr)
{
    _wifi_subnetsta = addr;
}

//---------------------------------------------------------------------------------
void setUartBaudRate(UINT32 baud)
{
    _uart_baud_rate = baud;
}

// -------- EEPROM ----------------------------------

//---------------------------------------------------------------------------------
//-- Initialize
void Eeprom_begin()
{
    EEPROM.begin(EEPROM_SPACE);
    delay(500);
    resetToDefaults();
    Eeprom_loadAllParams();
    delay(500);
}

//---------------------------------------------------------------------------------
//-- Saves all parameters to EEPROM
void Eeprom_loadAllParams()
{
    UINT32 address = 0;
    for (int i = 0; i < ID_COUNT; i++)
    {
        UINT8 *ptr = (UINT8 *)Parameters[i].value;
        for (int j = 0; j < Parameters[i].length; j++, address++, ptr++)
        {
            *ptr = EEPROM.read(address);
        }
#ifdef DEBUG
        Serial1.print("Loading from EEPROM: ");
        Serial1.print(Parameters[i].id);
        Serial1.print(" Value: ");
        if (Parameters[i].type == PARAM_TYPE_UINT32)
            Serial1.println(*((UINT32 *)Parameters[i].value));
        else if (Parameters[i].type == PARAM_TYPE_UINT16)
            Serial1.println(*((UINT16 *)Parameters[i].value));
        else
            Serial1.println(*((UINT8 *)Parameters[i].value));
#endif
    }
#ifdef DEBUG
    Serial1.println("");
#endif
    //-- Version if hardwired
    _sw_version = ESP_UDP_BRIDGE_VERSION;
    _flash_left = ESP.getFreeSketchSpace();
}

//---------------------------------------------------------------------------------
//-- Saves all parameters to EEPROM
void Eeprom_saveAllParams()
{
    //-- Init flash space
    UINT8 *ptr = EEPROM.getDataPtr();
    memset(ptr, 0, EEPROM_SPACE);
    //-- Write all paramaters to flash
    UINT32 address = 0;
    for (int i = 0; i < ID_COUNT; i++)
    {
        ptr = (UINT8 *)Parameters[i].value;
#ifdef DEBUG
        Serial1.print("Saving to EEPROM: ");
        Serial1.print(Parameters[i].id);
        Serial1.print(" Value: ");
        if (Parameters[i].type == PARAM_TYPE_UINT32)
            Serial1.println(*((UINT32 *)Parameters[i].value));
        else if (Parameters[i].type == PARAM_TYPE_UINT16)
            Serial1.println(*((UINT16 *)Parameters[i].value));
        else
            Serial1.println(*((UINT8 *)Parameters[i].value));
#endif
        for (int j = 0; j < Parameters[i].length; j++, address++, ptr++)
        {
            EEPROM.write(address, *ptr);
        }
    }
    UINT32 saved_crc = _getEepromCrc();
    EEPROM.put(EEPROM_CRC_ADD, saved_crc);
    EEPROM.commit();
#ifdef DEBUG
    Serial1.print("Saved CRC: ");
    Serial1.print(saved_crc);
    Serial1.println("");
#endif
}

//---------------------------------------------------------------------------------
//-- Initializes EEPROM. If not initialized, set to defaults and save it.
void Eeprom_init()
{
    
//     //-- Is it uninitialized?
//     UINT32 saved_crc = 0;
//     EEPROM.get(EEPROM_CRC_ADD, saved_crc);
//     UINT32 current_crc = _getEepromCrc();
//     if (saved_crc != current_crc)
//     {
// #ifdef DEBUG
//         Serial1.print("Initializing EEPROM. Saved: ");
//         Serial1.print(saved_crc);
//         Serial1.print(" Current: ");
//         Serial1.println(current_crc);
// #endif
//         //-- Set all defaults
//         resetToDefaults();
//         //-- Save it all and store CRC
//         Eeprom_saveAllParams();
//     }
//     else
//     {
//         //-- Load all parameters from EEPROM
//         Eeprom_loadAllParams();
//     }
}

void Eeprom_deinit()
{
    _flash_left = ESP.getFreeSketchSpace();
}

//---------------------------------------------------------------------------------
//-- Computes EEPROM CRC
UINT32 _getEepromCrc()
{
    UINT32 crc = 0;
    UINT32 size = 0;
    //-- Get size of all parameter data
    for (int i = 0; i < ID_COUNT; i++)
    {
        size += Parameters[i].length;
    }
    //-- Computer CRC
    for (int i = 0; i < (int)size; i++)
    {
        crc = crc_table[(crc ^ EEPROM.read(i)) & 0xff] ^ (crc >> 8);
    }
    return crc;
}
