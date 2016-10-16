/*******************************************************************************
 * INCLUDE HEADERS
 ******************************************************************************/
#include "FanControl.h"

/*******************************************************************************
 * VARIABLE DEFINITIONS
 ******************************************************************************/
/* Define the 'lcd' object used for interfacing with LCD screen */
LiquidCrystal lcd ( LCDRSPIN,  // set the RS pin
  LCDRWPIN,                    // set the RW pin
  LCDENABLEPIN,                // set the Enable pin
  LCDD0PIN,                    // set the data 0 pin
  LCDD1PIN,                    // set the data 1 pin
  LCDD2PIN,                    // set the data 2 pin
  LCDD3PIN );                  // set the data 3 pin
unsigned long hall1Period = 1; // period count for hall sensor 1
unsigned long hall2Period = 1; // period count for hall sensor 2
unsigned int  Fan1RPM     = 0; // Fan 1 speed, in rpm
unsigned int  Fan2RPM     = 0; // Fan 2 speed, in rpm

/*******************************************************************************
 * LOCAL FUNCTION DECLARATIONS
 ******************************************************************************/
void hall1ISR ( void ); // hall sensor 1 interrupt service routine
void hall2ISR ( void ); // hall sensor 1 interrupt service routine

/*******************************************************************************
 * FUNCTION DEFINITIONS
 ******************************************************************************/

/******************************************************************************
* Function:
*   setup()
*
* Description:
*   Initialization routines performed upon boot-up of the device.
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
void setup ( void )
{
  /* Initialize the LCD Screen and print initialization message */
  lcd.begin ( LCDCOLS, LCDROWS );  // initialize LCD display (16 cols, 2 rows)
  lcd.print ( "INITIALIZING..." ); // set screen to say initializing

  /* Initialize the serial bus and print initialization message */
  Serial.begin ( BAUDRATE ); // begin serial comms
  while ( !Serial )
  {
    ; // wait for serial bus to come on
  }
  Serial.print ( "INITIALIZING...\n" ); // write initializing message on serial

  /* Configure PWM pins and start with low output */
  digitalWrite ( PWM1PIN, LOW ); // start with pwm1 pin low
  digitalWrite ( PWM2PIN, LOW ); // start with pwm2 pin low
  analogWrite ( PWM1PIN, 0x00 ); // set pwm1 to 0% duty
  analogWrite ( PWM2PIN, 0x00 ); // set pwm2 to 0% duty
  pinMode ( PWM1PIN, OUTPUT );   // set pin 5 as an output (PWM1)
  pinMode ( PWM2PIN, OUTPUT );   // set pin 6 as an output (PWM2)

  /* Change Timer0 clock divider to achieve higher speed (for 62500Hz PWM).
   * This effectively increases the frequency of the Timer0 clock by a factor
   * of 64.
   *
   * IMPORTANT: when changing this timer0 clock speed, it changes the number
   * of timer0 overflows which occur each microsecond.  This, in turn, changes
   * the behavior of micros(), millis(), delay(), etc timing functions.  To
   * correct for this, values returned by micros() or millis() need to be
   * divided by 64.  When using delay(), multiply the time by 64.
   * */
  TCCR0A = _BV ( COM0A1 ) |
    _BV ( COM0B1 ) |
    _BV ( WGM01 ) |
    _BV ( WGM00 );       // set pins 5 and 6 as PWM controlled by timer0
  TCCR0B = _BV ( CS00 ); // set timer0 frequency for 62500 Hz PWM on pins 5 & 6

  /* Attach interrupts to hall sensor input pins (both rising/falling edges) */
  attachInterrupt ( digitalPinToInterrupt ( HALL1PIN ), hall1ISR, CHANGE );
  attachInterrupt ( digitalPinToInterrupt ( HALL2PIN ), hall2ISR, CHANGE );

  /* Clear LCD screen and set to display default info screen */
  lcd.clear ( );                    // clear LCD screen and move cursor to start
  lcd.print ( "TIME:    0 s" );     // Print time info on first line
  lcd.setCursor ( 0, 1 );           // set cursor to start of second line on LCD
  lcd.print ( "F1:9999, F2:9999" ); // Print fan period info on second line

  return; // end of setup()
}         // end of setup()

/******************************************************************************
* Function:
*   loop()
*
* Description:
*   This is the main loop of the program.  It runs endlessly after
*   initialization is complete.
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
void loop ( void )
{
  static unsigned short lcdLoops = 0;          // number of loops run since last LCD update
  static unsigned long  loopsRun = 0;          // total number of loops run
  static unsigned long  lastTime = 0;          // time when last loop began (microseconds/64)
  unsigned long         thisTime = micros ( ); // time now (in microseconds/64)
  unsigned long         runTime_s;             // program run-time (seconds)
  char                  lcdBuff [ LCDCOLS ];   // buffer of chars used for LCD printing

  /* Check to see if it is time to run a new loop, otherwise return */
  if ( thisTime - lastTime >= LOOPTIME_US * 64 ) // enough time elapsed for new loop
    lastTime = thisTime;                         // store loop time for next iteration
  else
    return; // skip this loop iteration

  /* keep track of total program run time */
  runTime_s = loopsRun * LOOPTIME_US / 1000000; // track runtime in seconds
  loopsRun++;                                   // increment count of loops run

  /* Calculate Fan speeds in RPM */
  Fan1RPM = (unsigned int) ( ( 1000000L / hall1Period ) * 60 / FAN1PPR ); // Fan1 speed
  Fan2RPM = (unsigned int) ( ( 1000000L / hall2Period ) * 60 / FAN2PPR ); // Fan2 speed

  /* Update LCD if needed */
  if ( ++lcdLoops >= LCD_DEC ) // if enough loops have occured, update LCD
  {
    lcdLoops = 0; // reset LCD loop counter

    /* Mark Run-time on first line of LCD display */
    sprintf ( lcdBuff, "TIME: %4lu s", runTime_s ); // set time as first line
    lcd.setCursor ( 0, 0 );                         // set cursor to start of first line on LCD
    lcd.print ( lcdBuff );                          // print first line

    /* Mark hall-sensor period values on second line of LCD display */
    sprintf ( lcdBuff, "F1:%4u, F2:%4u", Fan1RPM, Fan2RPM ); // set fan speeds
    lcd.setCursor ( 0, 1 );                                  // set cursor to start of second line on LCD
    lcd.print ( lcdBuff );                                   // print second line
  }

  /* Read serial data if available, and echo it back */
  if ( Serial.available ( ) )
  {
    Serial.write ( Serial.read ( ) );
  }

  /* Set duty cycle for pwm outputs */
  analogWrite ( PWM1PIN, 0xFF ); // set pwm1 to 100% duty
  analogWrite ( PWM2PIN, 0x01 ); // set pwm2 to 50% duty


  return; // end of loop()
}         // end of loop()

/******************************************************************************
* Function:
*   hall1ISR()
*
* Description:
*   Interrupt Service Routine called whenever the hall-effect sensor for Fan1
*   changes state.  Used to track timing of the hall sensor pulses, which
*   indicates fan speed.
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
void hall1ISR ( void )
{
  static unsigned long lastEdgeTime = 0;          // timestamp of previous edge
  unsigned long        thisEdgeTime = micros ( ); // timestamp of current edge

  /* Hall Period (microseconds) calculated using the difference between
   * rising and falling edges, multiplied by two.  The multiply by two
   * is required because we are counting between any rising and falling
   * edge, which counts the time length of half a period. Since the timer0
   * speed is increased by 64, we need to also divide by 64.  So, right bit
   * shifting by 5 yields the overall divide by 32.*/
  hall1Period  = ( thisEdgeTime - lastEdgeTime ) >> 5; // calculate period in microseconds

  lastEdgeTime = thisEdgeTime; // save current edge time for next iteration

  return; // end of hall1ISR()
}         // end of hall1ISR()

/******************************************************************************
* Function:
*   hall2ISR()
*
* Description:
*   Interrupt Service Routine called whenever the hall-effect sensor for Fan2
*   changes state.  Used to track timing of the hall sensor pulses, which
*   indicates fan speed.
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
void hall2ISR ( void )
{
  static unsigned long lastEdgeTime = 0;          // timestamp of previous edge
  unsigned long        thisEdgeTime = micros ( ); // timestamp of current edge

  /* Hall Period (microseconds) calculated using the difference between
   * rising and falling edges, multiplied by two.  The multiply by two
   * is required because we are counting between any rising and falling
   * edge, which counts the time length of half a period. Since the timer0
   * speed is increased by 64, we need to also divide by 64.  So, right bit
   * shifting by 5 yields the overall divide by 32.*/
  hall2Period  = ( thisEdgeTime - lastEdgeTime ) >> 5; // calculate period in microseconds

  lastEdgeTime = thisEdgeTime; // save current edge time for next iteration

  return; // end of hall2ISR()
}         // end of hall2ISR()
