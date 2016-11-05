/*
 * fanCtrlStateMachine.cpp
 *
 *  Created on: Nov 4, 2016
 *      Author: Paul
 */


#include "fanCtrlStateMachine.h"
#include "FanControlUtils.h"
#include "LiquidCrystal.h"
#include "savedVars.h"


/* Define the 'lcd' object used for interfacing with LCD screen */
LiquidCrystal lcd ( LCDRSPIN, // set the RS pin
  LCDRWPIN,                   // set the RW pin
  LCDENABLEPIN,               // set the Enable pin
  LCDD0PIN,                   // set the data 0 pin
  LCDD1PIN,                   // set the data 1 pin
  LCDD2PIN,                   // set the data 2 pin
  LCDD3PIN );                 // set the data 3 pin


/*******************************************************************************
 * FUNCTION DEFINITIONS
 ******************************************************************************/

/******************************************************************************
* Function:
*   initState()
*
* Description:
*   Runs the INIT state routine
*
* Arguments:
*   none
*
* Returns:
*   nextState - state to enter upon exiting this function
******************************************************************************/
static FANCTRLSTATE_ENUM_TYPE initState ( FANCTRLSTATE_ENUM_TYPE lastState )
{
  /* Load all variables from EEPROM */
  loadAllVars ( );

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

  /* Configure Button Pins as Inputs */
  pinMode ( BTN1PIN, INPUT ); // button 1 is an input
  pinMode ( BTN2PIN, INPUT ); // button 2 is an input
  pinMode ( BTN3PIN, INPUT ); // button 3 is an input

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
  lcd.print ( "  C: 999.9 999.9" ); // Print temperature info on first line
  lcd.setCursor ( 0, 1 );           // set cursor to start of second line on LCD
  lcd.print ( "RPM:  9999  9999" ); // Print fan period info on second line

  return NORMAL; // return next state, which is NORMAL
}                // end of initState()

/******************************************************************************
* Function:
*   normalState()
*
* Description:
*   Runs the NORMAL state routine
*
* Arguments:
*   none
*
* Returns:
*   nextState - state to enter upon exiting this function
******************************************************************************/
static FANCTRLSTATE_ENUM_TYPE normalState ( FANCTRLSTATE_ENUM_TYPE lastState )
{
  FANCTRLSTATE_ENUM_TYPE nextState = NORMAL;            // by default, stay in normal state
  static byte            lcdLoops  = 0;                 // number of loops run since last LCD update
  char                   lcdBuff [ LCDCOLS * LCDROWS ]; // buffer of chars used for LCD printing

  /* Update LCD if needed */
  if ( ++lcdLoops >= LCD_DEC || lastState != NORMAL ) // if enough loops have occured or if this is first instance of NORMAL state, update LCD
  {
    lcdLoops = 0; // reset LCD loop counter

#ifndef DEBUG_FANCTRL2
    /* Mark Temperature on first line of LCD display */
    if ( useFtemp )
    {
      sprintf ( lcdBuff, " %cF: %3hu.%hu %3hu.%hu",
        0xDF,
        DigTemp1ToF10 ( Temp1 ) / 10,
        abs ( DigTemp1ToF10 ( Temp1 ) ) % 10,
        DigTemp2ToF10 ( Temp2 ) / 10,
        abs ( DigTemp2ToF10 ( Temp2 ) ) % 10 ); // set temperatures as first line
    }
    else
    {
      sprintf ( lcdBuff, " %cC: %3hu.%hu %3hu.%hu",
        0xDF,
        DigTemp1ToC10 ( Temp1 ) / 10,
        abs ( DigTemp1ToC10 ( Temp1 ) ) % 10,
        DigTemp2ToC10 ( Temp2 ) / 10,
        abs ( DigTemp2ToC10 ( Temp2 ) ) % 10 ); // set temperatures as first line
    }
    lcd.setCursor ( 0, 0 ); // set cursor to start of first line on LCD
    lcd.print ( lcdBuff );  // print first line

    /* Mark hall-sensor period values on second line of LCD display */
    sprintf ( lcdBuff, "RPM:  %4hu  %4hu", Fan1RPM, Fan2RPM ); // set fan speeds
    lcd.setCursor ( 0, 1 );                                    // set cursor to start of second line on LCD
    lcd.print ( lcdBuff );                                     // print second line
#else
    /* First line has Int Term (quarter counts), Int State (tenths of error count times seconds), output duty (counts) */
    sprintf ( lcdBuff, "%5d %6d %3u", pi2.getIntTerm ( ), pi2.getIntState ( ), Pwm2Duty ); // set fan info
    lcd.setCursor ( 0, 0 );                                                                // set cursor to start of first line on LCD
    lcd.print ( lcdBuff );                                                                 // print first line

    /* Second line has Prop Term (quarter counts), Error (rpm), speed feedback (rpm) */
    sprintf ( lcdBuff, "%5d %5d %4u", pi2.getPropTerm ( ), (int) Fan2RPMRef - Fan2RPM, Fan2RPM ); // set fan info
    lcd.setCursor ( 0, 1 );                                                                       // set cursor to start of second line on LCD
    lcd.print ( lcdBuff );                                                                        // print second line
#endif
  }

  /* Read serial data if available, and echo it back */
  if ( Serial.available ( ) )
  {
    Serial.write ( Serial.read ( ) );
  }

  /* Set desired fan speeds based on temperature */
  setRefFanSpeeds ( );

  /* Regulate Fan Speeds to Track Reference Values */
  regFanSpeeds ( );

  return nextState;
} // end of normalState()

/******************************************************************************
* Function:
*   fanCtrlStateMachine()
*
* Description:
*   Constructor function for a fanCtrlStateMachine object
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
fanCtrlStateMachine :: fanCtrlStateMachine ( void )
{
  state = INIT;
} // end of fanCtrlStateMachine()


/******************************************************************************
* Function:
*   getState()
*
* Description:
*   returns the state machine state
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
FANCTRLSTATE_ENUM_TYPE fanCtrlStateMachine :: getState ( void )
{
  return state; // returns the state machine state
}               // end of getState


/******************************************************************************
* Function:
*   reset()
*
* Description:
*   reset the state machine state to initialization, and runs initialization
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
void fanCtrlStateMachine :: reset ( void )
{
  state = INIT; // set state to initialize
  run ( );      // runs initialization state step

  return;
} // end of reset()


/******************************************************************************
* Function:
*   run()
*
* Description:
*   runs the fan control state machine
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
void fanCtrlStateMachine :: run ( void )
{
  switch ( state )
  {
  case INIT:
    state = initState ( state ); // initialize system then set next state
    break;

  case NORMAL:
    state = normalState ( state ); // run normal state then move on to next state
    break;

  default:
    reset ( ); // reset device, invalid state reached
  }
  return;
} // end of run
