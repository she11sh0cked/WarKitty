/*
  Copyright (c) 2017 Erik Berreßem.  All rights reserved.

  /lib/WarKitty.cpp
*/

#include "WarKitty.h"

// PUBLIC ////////////////////

// CONSTRUCTOR

WarKitty::WarKitty( bool _verbal, int _update_rate ) {
  __verbal = _verbal;
  __update_rate = _update_rate;
}

WarKitty::WarKitty( bool _verbal ) {
  __verbal = _verbal;
  __update_rate = 0;
}

WarKitty::WarKitty( int _update_rate ) {
  __verbal = false;
  __update_rate = _update_rate;
}

// DECONSTRUCTOR

WarKitty::~WarKitty(){}

// UPDATE

bool WarKitty::update( int _MODE ) {
  if ( __MODE != _MODE ) {
    __MODE = _MODE;
    if ( __verbal ) return __begin_verbal()? true : false;
    else return __begin()? true : false;
  }

  if ( _MODE == SCAN ) {
    if ( __verbal ) return __scan_verbal()? true : false;
    else return __scan()? true : false;
    delay( __update_rate );
  } else if ( _MODE == VIEW ) {
    if ( __verbal ) return __view_verbal()? true : false;
    else return __view()? true : false;
  } else return false;
}

void WarKitty::gps( String _latitude, String _longitude, TinyGPSTime _time, TinyGPSDate _date ) {
  __latitude = _latitude;
  __longitude = _longitude;
  __time = __getTime_String( _time );
  __date = __getDate_String( _date );
}

bool WarKitty::reset( void ) {
  if ( SPIFFS.exists("DATA.LOG") ) {
    Serial.print( "Resetting DATA.LOG.. " );
    SPIFFS.remove( "DATA.LOG" );
    if ( !SPIFFS.open( "DATA.LOG", "w" ) ) {
      Serial.println( "FAILED!" );
      return false;
    } else Serial.println( "DONE!" );
  } else {
    Serial.println( "Nothing to reset.." );
  }
  return true;
}

// PRIVATE ///////////////////

// BEGIN

bool WarKitty::__begin_verbal( void ) {
  switch ( __MODE ) {
    case SCAN:
      Serial.println( "\n// INIT SCAN /////////////////\n" );

      Serial.println( "// FILE SYSTEM //" );
      Serial.print( "Initializing.. " );
      if ( !SPIFFS.begin() ) {
        Serial.println( "FAILED!" );
        return false;
      } else Serial.println( "DONE!" );
      if ( !SPIFFS.exists("DATA.LOG") ) {
        Serial.print( "Creating DATA.LOG.." );
        if ( !SPIFFS.open( "DATA.LOG", "w" ) ) {
          Serial.println( "FAILED!" );
          return false;
        } else Serial.println( "DONE!" );
      }

      Serial.println( "\n// WIFI //" );
      if ( WiFi.status() == WL_CONNECTED ) {
        Serial.print( "Disconnecting.. " );
        if ( !WiFi.disconnect() ) {
          Serial.println( "FAILED!" );
          return false;
        } else Serial.println( "DONE!" );
      }
      Serial.print( "Setting to station mode.. " );
      if ( !WiFi.mode( WIFI_STA ) ) {
        Serial.println( "FAILED!" );
        return false;
      } else Serial.println( "DONE!" );
      Serial.println();

      return true;

    case VIEW:
      Serial.println( "VIEW SETUP" );
      return true;

    default:
      return false;
  }
}


bool WarKitty::__begin( void ) {
  switch ( __MODE ) {
    case SCAN:
      if ( !SPIFFS.begin() ) return false;
      if ( !SPIFFS.exists("DATA.LOG") ) SPIFFS.open( "DATA.LOG", "w" );
      if ( WiFi.status() == WL_CONNECTED )
        if ( !WiFi.disconnect() ) return false;
      if ( !WiFi.mode( WIFI_STA ) ) return false;
      return true;

    case VIEW:
      return true;

    default:
      return false;
      }
}

// SCAN

bool WarKitty::__scan_verbal( void ) {
  Serial.println( "\n\n// SCAN //////////////////////\n" );

  int networks = WiFi.scanNetworks();

  if ( networks == 1 ) Serial.println( "1 network found!" );
  else Serial.println( (String)networks + " networks found!" );
  if ( networks > 0 ) {
    int netId;
    int netFound = 0;

    DynamicJsonBuffer jsonBuffer;
    JsonObject& Root_Object = jsonBuffer.createObject();
    Root_Object["count"] = networks;
    Root_Object["max"] = networks;
    JsonArray& Data_Array = Root_Object.createNestedArray( "data" );
    JsonObject& Data_Object = jsonBuffer.createObject();
    JsonObject& WiFi_Object = Data_Object.createNestedObject( "wifi" );
    JsonObject& GPS_Object = Data_Object.createNestedObject( "gps" );

    File file = SPIFFS.open( "DATA.LOG", "r" );
    String json = file.readString();

    if ( json == "" ) {
      for ( int i = 0; i < networks; ++i ) {
        JsonObject& Data_Object_tmp = jsonBuffer.createObject();

        Data_Object_tmp["id"] = i;
        Data_Object_tmp["wifi"] = __getWiFi_Object( i, jsonBuffer );
        Data_Object_tmp["gps"] = __getGPS_Object( jsonBuffer );

        Serial.print( "Add - " );Serial.printf( Data_Object_tmp["wifi"]["ssid"] );Serial.println();
        Data_Array.add( Data_Object_tmp );
      }
    } else {
    JsonObject& Root_Object_file = jsonBuffer.parseObject( json );
    if ( !Root_Object_file.success() ) {
      Serial.println( "FAILED to parse DATA.LOG!" );
      Serial.print( "Resetting corrupted DATA.LOG.. " );
      if ( !SPIFFS.remove( "DATA.LOG" ) ) {
        Serial.println( "FAILED to remove!" );
        return false;
      }
      if ( !SPIFFS.open( "DATA.LOG", "w" ) ) {
        Serial.println( "FAILED to create!" );
      }
      Serial.println( "DONE!" );
      return false;
    }

    int netNum_file = Root_Object_file["count"];
    int netMax_file = Root_Object_file["max"];
    netId = netMax_file;

    JsonArray& Data_Array_file = Root_Object_file["data"];
    for ( JsonArray::iterator it = Data_Array_file.begin(); it != Data_Array_file.end(); ++it ) {
      JsonObject& Data_Object_file = *it;
      Data_Array.add( Data_Object_file );
    }

    for ( int i = 0; i < networks; ++i ) {
      bool knownNet = false;

      for ( int j = 0; j < netNum_file; ++j ) {
        String bssid1 = WiFi.BSSIDstr( i );
        String bssid2 = Data_Array[j]["wifi"]["bssid"];

        if ( bssid1 == bssid2 ) {
          // TODO: check gps?
          knownNet = true;

          Data_Array[j]["wifi"] = __getWiFi_Object( i, jsonBuffer );
          Data_Array[j]["gps"] = __getGPS_Object( jsonBuffer );

          Serial.print( "Edit - " );Serial.printf( Data_Array[j]["wifi"]["ssid"] );Serial.println();
        }
      }

      if ( !knownNet ) {
        JsonObject& Data_Object_tmp = jsonBuffer.createObject();

        Data_Object_tmp["id"] = netId;
        Data_Object_tmp["wifi"] = __getWiFi_Object( i, jsonBuffer );
        Data_Object_tmp["gps"] = __getGPS_Object( jsonBuffer );

        Serial.print( "Add - " );Serial.printf( Data_Object_tmp["wifi"]["ssid"] );Serial.println();
        Data_Array.add( Data_Object_tmp );

        ++netFound;
        ++netId;
      }
    }

    Root_Object["count"] = netNum_file + netFound;
    Root_Object["max"] = netId;
  }

  file.close();
  SPIFFS.remove( "DATA.LOG" );
  file = SPIFFS.open( "DATA.LOG", "w" );

  json = "";
  Root_Object.printTo( json );
  file.print( json );
  file.close();
  Root_Object["count"].printTo( Serial ); Serial.println( " Networks total" );
  }

  return true;
}

bool WarKitty::__scan( void ) {
  int networks = WiFi.scanNetworks();
  if ( networks > 0 ) {
    int netId;
    int netFound = 0;

    DynamicJsonBuffer jsonBuffer;
    JsonObject& Root_Object = jsonBuffer.createObject();
    Root_Object["count"] = networks;
    Root_Object["max"] = networks;
    JsonArray& Data_Array = Root_Object.createNestedArray( "data" );
    JsonObject& Data_Object = jsonBuffer.createObject();
    JsonObject& WiFi_Object = Data_Object.createNestedObject( "wifi" );
    JsonObject& GPS_Object = Data_Object.createNestedObject( "gps" );

    File file = SPIFFS.open( "DATA.LOG", "r" );

    String json = file.readString();

    if ( json == "" ) {
      for ( int i = 0; i < networks; ++i ) {
        JsonObject& Data_Object_tmp = jsonBuffer.createObject();

        Data_Object_tmp["id"] = i;
        Data_Object_tmp["wifi"] = __getWiFi_Object( i, jsonBuffer );
        Data_Object_tmp["gps"] = __getGPS_Object( jsonBuffer );

        Data_Array.add( Data_Object_tmp );
      }
    } else {
    JsonObject& Root_Object_file = jsonBuffer.parseObject( json );
    if ( !Root_Object_file.success() ) {
      SPIFFS.remove( "DATA.LOG" );
      SPIFFS.open( "DATA.LOG", "w" );
      return false;
    }

    int netNum_file = Root_Object_file["count"];
    int netMax_file = Root_Object_file["max"];
    netId = netMax_file;

    JsonArray& Data_Array_file = Root_Object_file["data"];
    for ( JsonArray::iterator it = Data_Array_file.begin(); it != Data_Array_file.end(); ++it ) {
      JsonObject& Data_Object_file = *it;
      Data_Array.add( Data_Object_file );
    }

    for ( int i = 0; i < networks; ++i ) {
      bool knownNet = false;

      for ( int j = 0; j < netNum_file; ++j ) {
        String bssid1 = WiFi.BSSIDstr( i );
        String bssid2 = Data_Array[j]["wifi"]["bssid"];

        if ( bssid1 == bssid2 ) {
          // TODO: check gps?
          knownNet = true;

          Data_Array[j]["wifi"] = __getWiFi_Object( i, jsonBuffer );
          Data_Array[j]["gps"] = __getGPS_Object( jsonBuffer );
        }
      }

      if ( !knownNet ) {
        JsonObject& Data_Object_tmp = jsonBuffer.createObject();
        Data_Object_tmp["id"] = netId;
        Data_Object_tmp["wifi"] = __getWiFi_Object( i, jsonBuffer );
        Data_Object_tmp["gps"] = __getGPS_Object( jsonBuffer );

        Data_Array.add( Data_Object_tmp );

        ++netFound;
        ++netId;
      }
    }
    Root_Object["count"] = netNum_file + netFound;
    Root_Object["max"] = netId;
  }
  file.close();
  SPIFFS.remove( "DATA.LOG" );
  file = SPIFFS.open( "DATA.LOG", "w" );

  json = "";
  Root_Object.printTo( json );
  file.print( json );
  file.close();
  }

  return true;
}

// VIEW

bool WarKitty::__view_verbal( void ) {
  Serial.println( "VIEW LOOP" );
  return true;
}

bool WarKitty::__view( void ) {
  return true;
}

// HELPER

String  WarKitty::__encryption( int _netItem ) {
  switch( _netItem ) {
    case ENC_TYPE_AUTO:
      return "AUTO";
    case ENC_TYPE_CCMP:
      return "CCMP";
    case ENC_TYPE_NONE:
      return "NONE";
    case ENC_TYPE_TKIP:
      return "TKIP";
    case ENC_TYPE_WEP:
      return "WEP";
    default:
      return "UNDEFINED";
  }
}

String WarKitty::__getDate_String( TinyGPSDate _date ) {
  String day = "";
  String month = "";
  String year = "";

  if ( _date.day() < 10 ) day = "0";
  day += _date.day();
  if ( _date.month() < 10 ) month = "0";
  month += _date.month();
  year = _date.year();

  return day + "/" + month + "/" + year;
}

String WarKitty::__getTime_String( TinyGPSTime _time ) {
  String second = "";
  String minute = "";
  String hour = "";

  if ( _time.second() < 10 ) second = "0";
  second += _time.second();
  if ( _time.minute() < 10 ) minute = "0";
  minute += _time.minute();
  if ( _time.hour() < 10 ) hour = "0";
  hour += _time.hour();

  return hour + ":" + minute + ":" + second;
}

JsonObject& WarKitty::__getWiFi_Object( int _netItem, JsonBuffer& _jsonBuffer ) {
  JsonObject& tmp_Object = _jsonBuffer.createObject();
  tmp_Object["ssid"] = WiFi.SSID( _netItem );
  tmp_Object["enc"] = __encryption( WiFi.encryptionType( _netItem ) );
  tmp_Object["rssi"] = WiFi.RSSI( _netItem );
  tmp_Object["bssid"] = WiFi.BSSIDstr( _netItem );
  tmp_Object["ch"] = WiFi.channel( _netItem );
  tmp_Object["isHidden"] = WiFi.isHidden( _netItem );
  return tmp_Object;
}

// TODO: get gps data
JsonObject& WarKitty::__getGPS_Object( JsonBuffer& _jsonBuffer ) {
  JsonObject& tmp_Object = _jsonBuffer.createObject();
  tmp_Object["longitude"] = __longitude;
  tmp_Object["latitude"] = __latitude;
  tmp_Object["date"] = __date;
  tmp_Object["time"] = __time;
  return tmp_Object;
}
