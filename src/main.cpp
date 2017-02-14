/*
 *  Copyright (c) 2017 Erik Berreßem.  All rights reserved.
 *
 *  /main.cpp
 *  LAST MODIEFIED: 14.02.2017
 *
 *  CIRCUIT
 *  * GPS-module is connected to board like:
 *  **         GPS <-> BOARD
 *  **         VCC <-  VIN   [5v]
 *  **         GND  -> GND
 *  ** Hardware-TX  -> D6    [Software-RX]
 *  ** Hardware-RX <-  D7    [Software-TX]
 *
 *  * Switch is connected to board like:
 *  **       BOARD <-> BOARD
 *  **          D8 <-  GND        [OFF]
 *  **          D8 <-  VIN   [5v] [ON]
 *
 *  TODO
 *  !* Integrate GPS
 *   * Code VIEW-mode
 *  !* Write dokumentation
 *  !* Delete option
 *
 *  BUG
 *  !* GPS doesn't update correctly
 *  !* It's always "ADD" [files gets resettet or something?]
 */

#include "WarKitty.h"
#include "SoftwareSerial.h"
#include "TinyGPS++.h"

#define SWITCH 15
#define MODE digitalRead( SWITCH ) == HIGH? VIEW : SCAN

SoftwareSerial gps_serial( 12, 13 );
TinyGPSPlus gps;

WarKitty warkitty = WarKitty( true, 1000 );

void setup( void ) {
  Serial.begin( 115200 );
  gps_serial.begin( 9600 );
  while( !Serial || !gps_serial );
  Serial.println();

  pinMode( SWITCH, INPUT_PULLUP );
  delay( 3000 );
}

void loop( void ) {
  while ( gps_serial.available() ) gps.encode( gps_serial.read() );
  while ( gps.date.year() == 2000 )
    while ( gps_serial.available() ) gps.encode( gps_serial.read() );

  warkitty.getGPS( (String)gps.location.lat(), (String)gps.location.lng(), gps.time, gps.date );

  if ( !warkitty.update( MODE ) ) Serial.println( "FAILED!" );
  Serial.println("\nFree Heap: " + (String)ESP.getFreeHeap());
}
