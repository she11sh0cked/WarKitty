/*
  Copyright (c) 2017 Erik Berreßem.  All rights reserved.

  /lib/WarKitty.h
*/

#ifndef WARKITTY_H
#define WARKITTY_H

#include "FS.h"
#include "ESP8266WiFi.h"

#include "ArduinoJson.h"
#include "TinyGPS++.h"

class WarKitty
{
  public:
    WarKitty( bool verbal, int update_rate );
    WarKitty( bool verbal );
    WarKitty( int update_rate );
    ~WarKitty();
    #define SCAN 2
    #define VIEW 3
    bool update( int MODE );
    void gps( String latitude, String longitude, TinyGPSTime time, TinyGPSDate date );
    bool reset( void );

  private:
    String __latitude;
    String __longitude;
    String __time;
    String __date;
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
    String __getDate_String( TinyGPSDate _date );
    String __getTime_String( TinyGPSTime _time );
    JsonObject& __getWiFi_Object( int _netItem, JsonBuffer& _jsonBuffer );
    JsonObject& __getGPS_Object( JsonBuffer& _jsonBuffer );
};

#endif
