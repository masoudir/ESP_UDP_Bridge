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
 * @file common.h
 * ESP8266 Wifi AP, UART/UDP Bridge
 *
 * @author Masoud Iranmehr <masoud.iranmehr@gmail.com>
 */


#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>

 extern "C" {
    // Espressif SDK
    #include "user_interface.h"
}

#define UINT8                       uint8_t
#define UINT16                      uint16_t
#define UINT32                      uint32_t
#define INT8                        int8_t
#define INT16                       int16_t
#define INT32                       int32_t


#define DEFAULT_WIFI_MODE           WIFI_MODE_AP
#define DEFAULT_UART_SPEED          115200
#define DEFAULT_WIFI_CHANNEL        11
#define DEFAULT_UDP_HPORT           13580
#define DEFAULT_UDP_CPORT           13585

#define DEFAULT_RECEVE_BUFFER_SIZE  1024

#define TIMEOUT                     10 * 1000

//-- The version is set from the build system (major, minor and build)
#define ESP_UDP_BRIDGE_VERSION_MAJOR     1
#define ESP_UDP_BRIDGE_VERSION_MINOR     0
#define ESP_UDP_BRIDGE_VERSION_BUILD     0

#define ESP_UDP_BRIDGE_VERSION          ((ESP_UDP_BRIDGE_VERSION_MAJOR << 24) & 0xFF00000) | ((ESP_UDP_BRIDGE_VERSION_MINOR << 16) & 0x00FF0000) | (ESP_UDP_BRIDGE_VERSION_BUILD & 0xFFFF)


/** @brief Specifies the datatype of ESP_UDP bridge parameter types. */
#ifndef HAVE_ENUM_PARAM_TYPE
#define HAVE_ENUM_PARAM_TYPE
typedef enum PARAM_TYPE
{
   PARAM_TYPE_UINT8=1, /* 8-bit unsigned integer | */
   PARAM_TYPE_INT8=2, /* 8-bit signed integer | */
   PARAM_TYPE_UINT16=3, /* 16-bit unsigned integer | */
   PARAM_TYPE_INT16=4, /* 16-bit signed integer | */
   PARAM_TYPE_UINT32=5, /* 32-bit unsigned integer | */
   PARAM_TYPE_INT32=6, /* 32-bit signed integer | */
   PARAM_TYPE_UINT64=7, /* 64-bit unsigned integer | */
   PARAM_TYPE_INT64=8, /* 64-bit signed integer | */
   PARAM_TYPE_REAL32=9, /* 32-bit floating-point | */
   PARAM_TYPE_REAL64=10, /* 64-bit floating-point | */
   PARAM_TYPE_ENUM_END=11, /*  | */
} PARAM_TYPE;
#endif

/** @brief Specifies the datatype of extended parameter. */
#ifndef HAVE_ENUM_PARAM_EXT_TYPE
#define HAVE_ENUM_PARAM_EXT_TYPE
typedef enum PARAM_EXT_TYPE
{
   PARAM_EXT_TYPE_UINT8=1, /* 8-bit unsigned integer | */
   PARAM_EXT_TYPE_INT8=2, /* 8-bit signed integer | */
   PARAM_EXT_TYPE_UINT16=3, /* 16-bit unsigned integer | */
   PARAM_EXT_TYPE_INT16=4, /* 16-bit signed integer | */
   PARAM_EXT_TYPE_UINT32=5, /* 32-bit unsigned integer | */
   PARAM_EXT_TYPE_INT32=6, /* 32-bit signed integer | */
   PARAM_EXT_TYPE_UINT64=7, /* 64-bit unsigned integer | */
   PARAM_EXT_TYPE_INT64=8, /* 64-bit signed integer | */
   PARAM_EXT_TYPE_REAL32=9, /* 32-bit floating-point | */
   PARAM_EXT_TYPE_REAL64=10, /* 64-bit floating-point | */
   PARAM_EXT_TYPE_CUSTOM=11, /* Custom Type | */
   PARAM_EXT_TYPE_ENUM_END=12, /*  | */
} PARAM_EXT_TYPE;
#endif


//-- Debug sent out to Serial1 (GPIO02), which is TX only (no RX).
//#define ENABLE_DEBUG


int log(const char *format, ...);

#ifdef ENABLE_DEBUG
#define DEBUG_LOG(format, ...) do { log(format, ## __VA_ARGS__); } while(0)
#else
#define DEBUG_LOG(format, ...) do { } while(0)
#endif


///// TODO: define time...


#endif
