// Do not remove the include below
#include "FanControl.h"
#include "lcd.h"


// The setup function is called once at startup of the sketch
void setup ( )
{
// Add your initialization code here
  lcd_init ( LCD_DISP_ON );         // initialize and turn on LCD display
  lcd_clrscr ( );                   // clear screen and cursor on LCD
  lcd_puts ( "INITIALIZING...\n" ); // set screen to say initializing

}

// The loop function is called in an endless loop
void loop ( )
{
// Add your repeated code here

}
