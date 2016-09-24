// Do not remove the include below
#include "FanControl.h"

/* Define the 'lcd' object */
LiquidCrystal lcd ( 8, 7, 9, 10, 11, 12, 13 );

// The setup function is called once at startup of the sketch
void setup ( )
{
// Add your initialization code here
  lcd.begin ( 16, 2 );             // initialize LCD display (16 columns, 2 rows)
  lcd.print ( "INITIALIZING..." ); // set screen to say initializing

  Serial.begin ( 9600 );
  while ( !Serial )
  {
    ; // wait for serial bus to come on
  }
  Serial.print ( "INITIALIZING...\n" ); // write initializing message on serial bus

}

// The loop function is called in an endless loop
void loop ( )
{
// Add your repeated code here

  if ( Serial.available ( ) )
  {
    Serial.write ( Serial.read ( ) );
  }

}
