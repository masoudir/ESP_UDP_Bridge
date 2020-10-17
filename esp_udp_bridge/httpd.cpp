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
 * @file httpd.cpp
 * ESP8266 Wifi AP, UART/UDP Bridge
 *
 * @author Masoud Iranmehr <masoud.iranmehr@gmail.com>
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>   // Include the WebServer library

#include "httpd.h"

const char kTEXTPLAIN[] = "text/plain";
const char kTEXTHTML[] = "text/html";
const char kACCESSCTL[] = "Access-Control-Allow-Origin";
const char kUPLOADFORM[] = "<h1><a href='/'> ESP_UDP_BRIDGE (UDP to Serial Bridge)</a></h1><form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='update'><br><input type='submit' value='Update'></form>";
const char kHEADER[] = "<!doctype html><html><head><title>ESP_UDP_BRIDGE (UDP to Serial Bridge)</title></head><body><h1><a href='/'>ESP_UDP_BRIDGE (UDP to Serial Bridge)</a></h1>";
const char kBADARG[] = "BAD ARGS";
const char kAPPJSON[] = "application/json";

const char *kBAUD = "baud";
const char *kPWD = "pwd";
const char *kSSID = "ssid";
const char *kPWDSTA = "pwdsta";
const char *kSSIDSTA = "ssidsta";
const char *kIPSTA = "ipsta";
const char *kGATESTA = "gatewaysta";
const char *kSUBSTA = "subnetsta";
const char *kCPORT = "cport";
const char *kHPORT = "hport";
const char *kCHANNEL = "channel";
const char *kDEBUG = "debug";
const char *kREBOOT = "reboot";
const char *kPOSITION = "position";
const char *kMODE = "mode";

const char *kFlashMaps[7] = {
    "512KB (256/256)",
    "256KB",
    "1MB (512/512)",
    "2MB (512/512)",
    "4MB (512/512)",
    "2MB (1024/1024)",
    "4MB (1024/1024)"};

static UINT32 flash = 0;

ESP8266WebServer webServer(80);
bool started = false;

//---------------------------------------------------------------------------------
void setNoCacheHeaders()
{
    webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    webServer.sendHeader("Pragma", "no-cache");
    webServer.sendHeader("Expires", "0");
}

//---------------------------------------------------------------------------------
void returnFail(String msg)
{
    webServer.send(500, FPSTR(kTEXTPLAIN), msg + "\r\n");
}

//---------------------------------------------------------------------------------
void respondOK()
{
    webServer.send(200, FPSTR(kTEXTPLAIN), "OK");
}

//---------------------------------------------------------------------------------
void handle_update()
{
    webServer.sendHeader("Connection", "close");
    webServer.sendHeader(FPSTR(kACCESSCTL), "*");
    webServer.send(200, FPSTR(kTEXTHTML), FPSTR(kUPLOADFORM));
}

//---------------------------------------------------------------------------------
void handle_upload()
{
    webServer.sendHeader("Connection", "close");
    webServer.sendHeader(FPSTR(kACCESSCTL), "*");
    webServer.send(200, FPSTR(kTEXTPLAIN), (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
}

//---------------------------------------------------------------------------------
void handle_upload_status()
{
    bool success = true;

    HTTPUpload &upload = webServer.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
#ifdef DEBUG_SERIAL
        DEBUG_SERIAL.setDebugOutput(true);
#endif
        WiFiUDP::stopAll();
#ifdef DEBUG_SERIAL
        DEBUG_SERIAL.printf("Update: %s\n", upload.filename.c_str());
#endif
        UINT32 maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace))
        {
#ifdef DEBUG_SERIAL
            Update.printError(DEBUG_SERIAL);
#endif
            success = false;
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
#ifdef DEBUG_SERIAL
            Update.printError(DEBUG_SERIAL);
#endif
            success = false;
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        {
#ifdef DEBUG_SERIAL
            DEBUG_SERIAL.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
#endif
        }
        else
        {
#ifdef DEBUG_SERIAL
            Update.printError(DEBUG_SERIAL);
#endif
            success = false;
        }
#ifdef DEBUG_SERIAL
        DEBUG_SERIAL.setDebugOutput(false);
#endif
    }
    yield();
    if (!success)
    {
        //
    }
}

//---------------------------------------------------------------------------------
void handle_getParameters()
{
        String message = FPSTR(kHEADER);
    message += "<p>Parameters</p><table><tr><td width=\"240\">Name</td><td>Value</td></tr>";
    for (int i = 0; i < ID_COUNT; i++)
    {
        if(i == ID_FWVER)
        {
            message += "<tr><td>";
            message += Param_getAt(i)->id;
            message += "</td>";
            message += "<td>";
            message += ESP_UDP_BRIDGE_VERSION_MAJOR;
            message += ".";
            message += ESP_UDP_BRIDGE_VERSION_MINOR;
            message += ".";
            message += ESP_UDP_BRIDGE_VERSION_BUILD;
            message += "</td></tr>";
        }
        else if(i == ID_MODE)
        {
            message += "<tr><td>";
            message += Param_getAt(i)->id;
            message += "</td>";
            message += "<td>";
            if(getWifiMode == WIFI_MODE_AP)
            {
                message += "AP (Access Point)";
            }
            else
            {
                message += "STA";
            }
            message += "</td></tr>";
        }
        else if(i == ID_IPADDRESS)
        {
            message += "<tr><td>";
            message += Param_getAt(i)->id;
            message += "</td>";
            message += "<td>";
            message += getLocalIPAddress();
            message += "</td></tr>";
        }
        else if(i == ID_SSID1)
        {
            message += "<tr><td>";
            message += Param_getAt(i)->id;
            message += "</td>";
            message += "<td>";
            message += (const char*)getWifiSsid();
            message += "</td></tr>";
        }
        else if(i > ID_SSID1 && i <= ID_SSID4) {}
        else if(i == ID_PASS1)
        {
            message += "<tr><td>";
            message += Param_getAt(i)->id;
            message += "</td>";
            message += "<td>";
            message += (const char*)getWifiPassword();
            message += "</td></tr>";
        }
        else if(i > ID_PASS1 && i <= ID_PASS4) {}
        else if(i == ID_SSIDSTA1)
        {
            message += "<tr><td>";
            message += Param_getAt(i)->id;
            message += "</td>";
            message += "<td>";
            message += (const char*)getWifiStaSsid();
            message += "</td></tr>";
        }
        else if(i > ID_SSIDSTA1 && i <= ID_SSIDSTA4) {}
        else if(i == ID_PASSSTA1)
        {
            message += "<tr><td>";
            message += Param_getAt(i)->id;
            message += "</td>";
            message += "<td>";
            message += (const char*)getWifiStaPassword();
            message += "</td></tr>";
        }
        else if(i > ID_PASSSTA1 && i <= ID_PASSSTA4) {}
        else // integer values
        {
            message += "<tr><td>";
            message += Param_getAt(i)->id;
            message += "</td>";
            unsigned long val = 0;
            if (Param_getAt(i)->type == PARAM_TYPE_UINT32)
                val = (unsigned long)*((UINT32 *)Param_getAt(i)->value);
            else if (Param_getAt(i)->type == PARAM_TYPE_UINT16)
                val = (unsigned long)*((UINT16 *)Param_getAt(i)->value);
            else
                val = (unsigned long)*((UINT8 *)Param_getAt(i)->value);

            message += "<td>";
            message += val;
            message += "</td></tr>";
        }
    }
    message += "</table>";
    message += "</body>";
    webServer.send(200, FPSTR(kTEXTHTML), message);
}

//---------------------------------------------------------------------------------
static void handle_root()
{
    String message = FPSTR(kHEADER);
    message += "Version: ";
    char vstr[30];
    snprintf(vstr, sizeof(vstr), "%u.%u.%u", ESP_UDP_BRIDGE_VERSION_MAJOR, ESP_UDP_BRIDGE_VERSION_MINOR, ESP_UDP_BRIDGE_VERSION_BUILD);
    message += vstr;
    message += "<p>\n";
    message += "<ul>\n";
    message += "<li><a href='/getstatus'>Get Status</a>\n";
    message += "<li><a href='/setup'>Setup</a>\n";
    message += "<li><a href='/getparameters'>Get Parameters</a>\n";
    message += "<li><a href='/update'>Update Firmware</a>\n";
    message += "<li><a href='/reboot'>Reboot</a>\n";
    message += "</ul></body>";
    setNoCacheHeaders();
    webServer.send(200, FPSTR(kTEXTHTML), message);

    // webServer.send(200,  "text/plain", "Hello world!");
}

//---------------------------------------------------------------------------------
static void handle_setup()
{
    String message = FPSTR(kHEADER);
    message += "<h1>Setup</h1>\n";
    message += "<form action='/setparameters' method='post'>\n";

    message += "WiFi Mode:&nbsp;";
    message += "<input type='radio' name='mode' value='0'";
    if (getWifiMode() == WIFI_MODE_AP)
    {
        message += " checked";
    }
    message += ">AccessPoint\n";
    message += "<input type='radio' name='mode' value='1'";
    if (getWifiMode() == WIFI_MODE_STA)
    {
        message += " checked";
    }
    message += ">Station<br>\n";

    message += "AP SSID:&nbsp;";
    message += "<input type='text' name='ssid' value='";
    message += getWifiSsid();
    message += "'><br>";

    message += "AP Password (min len 8):&nbsp;";
    message += "<input type='text' name='pwd' value='";
    message += getWifiPassword();
    message += "'><br>";

    message += "WiFi Channel:&nbsp;";
    message += "<input type='text' name='channel' value='";
    message += getWifiChannel();
    message += "'><br>";

    message += "Station SSID:&nbsp;";
    message += "<input type='text' name='ssidsta' value='";
    message += getWifiStaSsid();
    message += "'><br>";

    message += "Station Password:&nbsp;";
    message += "<input type='text' name='pwdsta' value='";
    message += getWifiStaPassword();
    message += "'><br>";

    IPAddress IP;
    message += "Station IP:&nbsp;";
    message += "<input type='text' name='ipsta' value='";
    IP = getWifiStaIP();
    message += IP.toString();
    message += "'><br>";

    message += "Station Gateway:&nbsp;";
    message += "<input type='text' name='gatewaysta' value='";
    IP = getWifiStaGateway();
    message += IP.toString();
    message += "'><br>";

    message += "Station Subnet:&nbsp;";
    message += "<input type='text' name='subnetsta' value='";
    IP = getWifiStaSubnet();
    message += IP.toString();
    message += "'><br>";

    // message += "Host Port:&nbsp;";
    // message += "<input type='text' name='hport' value='";
    // message += getWifiUdpHport();
    // message += "'><br>";

    message += "Client Port:&nbsp;";
    message += "<input type='text' name='cport' value='";
    message += getWifiUdpCport();
    message += "'><br>";

    message += "Baudrate:&nbsp;";
    message += "<input type='text' name='baud' value='";
    message += getUartBaudRate();
    message += "'><br>";

    message += "<input type='submit' value='Save'>";
    message += "</form>";
    setNoCacheHeaders();
    webServer.send(200, FPSTR(kTEXTHTML), message);
}

//---------------------------------------------------------------------------------
static void handle_getStatus()
{
    if (!flash)
        flash = ESP.getFreeSketchSpace();
    // String message = FPSTR(kHEADER);
    String message = kHEADER;
    message += "<p>System Status</p><table>\n";
    message += "<tr><td width=\"240\">Flash Size</td><td>";
    message += ESP.getFlashChipRealSize();
    message += "</td></tr>\n";
    message += "<tr><td width=\"240\">Flash Available</td><td>";
    message += flash;
    message += "</td></tr>\n";
    message += "<tr><td>RAM Left</td><td>";
    message += String(ESP.getFreeHeap());
    message += "</td></tr>\n";
    message += "</table>";
    message += "</body>";
    setNoCacheHeaders();
    webServer.send(200, FPSTR(kTEXTHTML), message);
}


//---------------------------------------------------------------------------------
void handle_getJSysInfo()
{
    if (!flash)
        flash = ESP.getFreeSketchSpace();
    UINT32 fid = spi_flash_get_id();
    char message[512];
    snprintf(message, 512,
             "{ "
             "\"size\": \"%s\", "
             "\"id\": \"0x%02lX 0x%04lX\", "
             "\"flashfree\": \"%u\", "
             "\"heapfree\": \"%u\", "
             "\"logsize\": \"%u\", "
             " }",
             kFlashMaps[system_get_flash_size_map()],
             (long unsigned int)(fid & 0xff), (long unsigned int)((fid & 0xff00) | ((fid >> 16) & 0xff)),
             flash,
             ESP.getFreeHeap(),
             0);
    webServer.send(200, "application/json", message);
}


//---------------------------------------------------------------------------------
void handle_setParameters()
{
    if (webServer.args() == 0)
    {
        returnFail(kBADARG);
        return;
    }
    bool ok = false;
    bool reboot = false;
    if (webServer.hasArg(kBAUD))
    {
        ok = true;
        setUartBaudRate(webServer.arg(kBAUD).toInt());
    }
    if (webServer.hasArg(kPWD))
    {
        ok = true;
        setWifiPassword(webServer.arg(kPWD).c_str());
    }
    if (webServer.hasArg(kSSID))
    {
        ok = true;
        setWifiSsid(webServer.arg(kSSID).c_str());
    }
    if (webServer.hasArg(kPWDSTA))
    {
        ok = true;
        setWifiStaPassword(webServer.arg(kPWDSTA).c_str());
    }
    if (webServer.hasArg(kSSIDSTA))
    {
        ok = true;
        setWifiStaSsid(webServer.arg(kSSIDSTA).c_str());
    }
    if (webServer.hasArg(kIPSTA))
    {
        IPAddress ip;
        ip.fromString(webServer.arg(kIPSTA).c_str());
        setWifiStaIP(ip);
    }
    if (webServer.hasArg(kGATESTA))
    {
        IPAddress ip;
        ip.fromString(webServer.arg(kGATESTA).c_str());
        setWifiStaGateway(ip);
    }
    if (webServer.hasArg(kSUBSTA))
    {
        IPAddress ip;
        ip.fromString(webServer.arg(kSUBSTA).c_str());
        setWifiStaSubnet(ip);
    }
    if (webServer.hasArg(kCPORT))
    {
        ok = true;
        setWifiUdpCport(webServer.arg(kCPORT).toInt());
    }
    if (webServer.hasArg(kHPORT))
    {
        ok = true;
        setWifiUdpHport(webServer.arg(kHPORT).toInt());
    }
    if (webServer.hasArg(kCHANNEL))
    {
        ok = true;
        setWifiChannel(webServer.arg(kCHANNEL).toInt());
    }
    if (webServer.hasArg(kDEBUG))
    {
        ok = true;
        setDebugEnabled(webServer.arg(kDEBUG).toInt());
    }
    if (webServer.hasArg(kMODE))
    {
        ok = true;
        setWifiMode(webServer.arg(kMODE).toInt());
    }
    if (webServer.hasArg(kREBOOT))
    {
        ok = true;
        reboot = webServer.arg(kREBOOT) == "1";
    }
    if (ok)
    {
        Eeprom_saveAllParams();
        //-- Send new parameters back
        handle_getParameters();
        // if (reboot)
        // {
        //     delay(100);
        //     ESP.restart();
        // }
    }
    else
        returnFail(kBADARG);
}

//---------------------------------------------------------------------------------
static void handle_reboot()
{
    String message = FPSTR(kHEADER);
    message += "rebooting ...</body>\n";
    setNoCacheHeaders();
    webServer.send(200, FPSTR(kTEXTHTML), message);
    delay(500);
    ESP.restart();
}

//---------------------------------------------------------------------------------
//-- 404
void handle_notFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += webServer.uri();
    message += "\nMethod: ";
    message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += webServer.args();
    message += "\n";
    for (UINT8 i = 0; i < webServer.args(); i++)
    {
        message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
    }
    webServer.send(404, FPSTR(kTEXTPLAIN), message);
}

//---------------------------------------------------------------------------------
ESP8266Httpd::ESP8266Httpd()
{
}

//---------------------------------------------------------------------------------
//-- Initialize
void ESP8266Httpd::begin()
{
    webServer.on("/", handle_root);
    webServer.on("/getparameters", handle_getParameters);
    webServer.on("/setparameters", handle_setParameters);
    webServer.on("/getstatus", handle_getStatus);
    webServer.on("/reboot", handle_reboot);
    webServer.on("/setup", handle_setup);
    webServer.on("/info.json", handle_getJSysInfo);
    webServer.on("/update", handle_update);
    webServer.on("/upload", HTTP_POST, handle_upload, handle_upload_status);
    webServer.onNotFound(handle_notFound);
    webServer.begin();
}

//---------------------------------------------------------------------------------
//-- Initialize
void ESP8266Httpd::checkUpdates()
{
    webServer.handleClient();
}
