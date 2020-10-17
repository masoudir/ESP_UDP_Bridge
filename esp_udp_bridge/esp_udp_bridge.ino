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
 * @file main.ino
 * ESP8266 Wifi AP, UART/UDP Bridge
 *
 * @author Masoud Iranmehr <masoud.iranmehr@gmail.com>
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>   // Include the WebServer library

#include "common.h"
#include "parameters.h"
#include "bridge.h"
#include "httpd.h"

// #define FACTORY_RESET_PIN_ENABLE
#define GPIO02  2

IPAddress               localIP;
ESP8266Httpd            updateServer;
ESP8266Bridge           bridge;

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);



//---------------------------------------------------------------------------------
//-- Wait for a DHCPD client
void wait_for_client() {
    DEBUG_LOG("Waiting for a client...\n");
#ifdef ENABLE_DEBUG
    int wcount = 0;
#endif
    uint8 client_count = wifi_softap_get_station_num();
    while (!client_count) {
#ifdef ENABLE_DEBUG
        Serial1.print(".");
        if(++wcount > 80) {
            wcount = 0;
            Serial1.println();
        }
#endif
        delay(1000);
        client_count = wifi_softap_get_station_num();
    }
    DEBUG_LOG("Got %d client(s)\n", client_count);
}


//---------------------------------------------------------------------------------
//-- Reset all parameters whenever the reset gpio pin is active
void reset_interrupt(){
    resetToDefaults();
    Eeprom_saveAllParams();
    ESP.reset();
}






void setup()
{
    Eeprom_begin();
    
    delay(1000);


   #ifdef ENABLE_DEBUG
       //   We only use it for non debug because GPIO02 is used as a serial
       //   pin (TX) when debugging.
       Serial1.begin(115200);
   #else
    #ifdef FACTORY_RESET_PIN_ENABLE
       //-- Initialized GPIO02 (Used for "Reset To Factory")
       pinMode(GPIO02, INPUT_PULLUP);
       attachInterrupt(GPIO02, reset_interrupt, FALLING);
    #endif
   #endif

   DEBUG_LOG("\nConfiguring access point...\n");
   DEBUG_LOG("Free Sketch Space: %u\n", ESP.getFreeSketchSpace());

   WiFi.disconnect(true);

   if(getWifiMode() == WIFI_MODE_STA){
       //-- Connect to an existing network
       WiFi.mode(WIFI_STA);
       WiFi.config(getWifiStaIP(), getWifiStaGateway(), getWifiStaSubnet(), 0U, 0U);
       WiFi.begin(getWifiStaSsid(), getWifiStaPassword());

       //-- Wait a minute to connect
       for(int i = 0; i < 120 && WiFi.status() != WL_CONNECTED; i++) {
           #ifdef ENABLE_DEBUG
           Serial.print(".");
           #endif
           delay(500);
       }
       if(WiFi.status() == WL_CONNECTED) {
           localIP = WiFi.localIP();
           WiFi.setAutoReconnect(true);
       } else {
           //-- Fall back to AP mode if no connection could be established
           WiFi.disconnect(true);
           setWifiMode(WIFI_MODE_AP);
       }
   }

   if(getWifiMode() == WIFI_MODE_AP){
       //-- Start AP
       WiFi.mode(WIFI_AP);
       WiFi.encryptionType(AUTH_WPA2_PSK);
       WiFi.softAP(getWifiSsid(), getWifiPassword(), getWifiChannel());
       WiFi.softAPConfig(local_ip, gateway, subnet);
       delay(100);
       localIP = WiFi.softAPIP();
       wait_for_client();
   }


   //-- Boost power to Max
   WiFi.setOutputPower(20.5);


    delay(2000);



   //-- Initialize Comm Links
   DEBUG_LOG("Start WiFi Bridge\n");
   DEBUG_LOG("Local IP: %s\n", localIP.toString().c_str());

   setLocalIPAddress(localIP.toString());


   IPAddress gcs_ip(localIP);
   //-- I'm getting bogus IP from the DHCP server. Broadcasting for now.
   gcs_ip[3] = 255;
   


   bridge.begin(gcs_ip, getWifiUdpHport(), getWifiUdpCport(), DEFAULT_UART_SPEED);
   //-- Initialize Update Server
   updateServer.begin();
}


void loop()
{
    bridge.udp_readMessageRaw();
    // // delay(0);
    bridge.serial_readMessageRaw();
    
    updateServer.checkUpdates();
}
