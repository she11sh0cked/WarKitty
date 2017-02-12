#include "WarKitty.h"

#define SWITCH 15
#define MODE digitalRead( SWITCH ) == HIGH? SCAN : VIEW

WarKitty warkitty = WarKitty( 12, 13, 9600, true, 0 );

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
