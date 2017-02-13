/*
  Copyright (c) 2017 Erik Berreßem.  All rights reserved.

  /main.cpp
  LAST MODIEFIED: 12.02.2017

  CIRCUIT
  * GPS-module is connected to board like:
  **         GPS <-> BOARD
  **         VCC <-  VIN   [5v]
  **         GND  -> GND
  ** Hardware-TX  -> D6    [Software-RX]
  ** Hardware-RX <-  D7    [Software-TX]

  * Switch is connected to board like:
  **       BOARD <-> BOARD
  **          D8 <-  GND        [OFF]
  **          D8 <-  VIN   [5v] [ON]

  TODO
  * Integrate GPS
  * Code VIEW-mode
  * Write dokumentation
  * Delete option

  BUG
  * Altitude always 0
  * GPS doesn't update fast enough
  * Resetting after long while loop
*/

#include "WarKitty.h"

#define SWITCH 15
#define MODE digitalRead( SWITCH ) == HIGH? SCAN : VIEW

WarKitty warkitty = WarKitty( 12, 13, 9600, true, 1000 );

void setup( void ) {
  Serial.begin( 115200 );
  while(!Serial);
  Serial.println();
  pinMode( SWITCH, INPUT_PULLUP );
  delay( 3000 );
}

void loop( void ) {
  if ( !warkitty.update( MODE ) ) Serial.println( "FAILED!" );
  Serial.println("\nFree Heap: " + (String)ESP.getFreeHeap());
}
