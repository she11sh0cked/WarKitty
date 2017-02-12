/*
  WarKitty.h - Wardriving library for the ESP8266
  Copyright (c) 2017 Erik Berreßem.  All right reserved.
*/

#ifndef WARKITTY_H
#define WARKITTY_H

#include "FS.h"
#include "ESP8266WiFi.h"
#include "SoftwareSerial.h"

#include "ArduinoJson.h"
#include "TinyGPS++.h"

class WarKitty
{
  public:
    WarKitty( int gps_rx, int gps_tx, int gps_baud, bool verbal, int update_rate );
    WarKitty( int gps_rx, int gps_tx, int gps_baud, bool verbal );
    WarKitty( int gps_rx, int gps_tx, int gps_baud, int update_rate );
    WarKitty( int gps_rx, int gps_tx, int gps_baud );
    ~WarKitty();
    #define SCAN 0
    #define VIEW 1
    bool update( int MODE );

  private:
    TinyGPSPlus __gps;
    int __gps_rx;
    int __gps_tx;
    int __gps_baud;
    bool __verbal;
    int __update_rate;
    int __MODE;
    bool __begin( void );
    bool __begin_verbal( void );
    bool __scan( void );
    bool __scan_verbal( void );
    bool __view( void );
    bool __view_verbal( void );
    String  __encryption( int _netItem );
    JsonObject& __getWiFi_Object( int _netItem, JsonBuffer& _jsonBuffer );
    JsonObject& __getGPS_Object( JsonBuffer& _jsonBuffer );
};

#endif
