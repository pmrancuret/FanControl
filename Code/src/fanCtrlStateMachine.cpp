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
#include "piController.h"

/* Declare the class objects, which are defined elsewhere */
extern LiquidCrystal lcd;
extern piController  pi1;
extern piController  pi2;

/*******************************************************************************
 * FUNCTION DEFINITIONS
 ******************************************************************************/

/******************************************************************************
* Function:
*   checkDebugMsgs()
*
* Description:
*   Checks serial input for commands to enter a debug mode.  If no message is
*   received, and there is no debug timeout, it will remain in same state.  If
*   already in debug mode, and a debug timeout occurs, it will return to normal
*   state.
*
* Arguments:
*   none
*
* Returns:
*   nextState - state to enter on next loop.
******************************************************************************/
static FANCTRLSTATE_ENUM_TYPE checkDebugMsgs ( FANCTRLSTATE_ENUM_TYPE thisState )
{
  FANCTRLSTATE_ENUM_TYPE nextState = thisState;                   // By default, remain in same state
  int                    cnt;                                     // count variable used for loop
  byte                   readBuffer [ DEBUGBUFFSIZE ];            // buffer for storing serial input
  int                    serialBytesAvail = Serial.available ( ); // number of bytes available to read
  int                    bytesRead;                               // number of bytes read
  static int             numDebugLoops;                           // number of consecutive loops in same debug mode without getting new message
  char                   msgHeader [ DEBUGHEADSIZE + 1 ];         // string containing message header read

  /* Make sure we don't read more bytes than our buffer can store */
  if ( serialBytesAvail > DEBUGBUFFSIZE )
    serialBytesAvail = DEBUGBUFFSIZE;

  /* Put terminator character at end of header buffer */
  msgHeader [ DEBUGHEADSIZE ] = '\0';

  /* Read serial data if enough bytes are available */
  if ( serialBytesAvail >= ( DEBUGMSG_DATWORDS * 2 + DEBUGHEADSIZE ) )
  {
    bytesRead = Serial.readBytes ( readBuffer, serialBytesAvail ); // read until all data is read, or buffer is full

    for ( cnt = 0;
      cnt <= ( bytesRead - ( DEBUGMSG_DATWORDS * 2 + DEBUGHEADSIZE ) );
      cnt++ ) // loop through each character of buffer which could potentially be the start of a debug message command.  Must have sufficient bytes to the right.
    {
      memcpy ( msgHeader, readBuffer + cnt, DEBUGHEADSIZE ); // copy header from read buffer into header string buffer

      /* Compare header with valid headers to see if a valid debug mode was specified */
      if ( strcmp ( msgHeader, DEBUGPI1_HEAD ) == 0 )
        nextState = DEBUG_PI1; // set next state to requested debug state
      else if ( strcmp ( msgHeader, DEBUGPI2_HEAD ) == 0 )
        nextState = DEBUG_PI2; // set next state to requested debug state
      else if ( strcmp ( msgHeader, DEBUGBTN_HEAD ) == 0 )
        nextState = DEBUG_BTNS; // set next state to requested debug state
      else
        continue; // did not find valid message, skip to next buffer value

      /* If we made it to this point, a valid debug message was found */
      memcpy ( debugDatWords, readBuffer + cnt + DEBUGHEADSIZE, DEBUGMSG_DATWORDS * 2 ); // copy data payload into debug words buffer
      numDebugLoops = 0;                                                                 // reset counter of consecutive debug loops without getting new message

      break; // exit loop, since we found the valid message
    }
  }

  /* Check number of consecutive debug loops, and exit to normal mode if timeout occurred */
  if ( nextState == DEBUG_PI1 ||
    nextState == DEBUG_PI2 ||
    nextState == DEBUG_BTNS ) // if we are in (or entering) debug state
  {
    if ( numDebugLoops++ >= DEBUG_TIMEOUT ) // increment count of debug loops, and check for timeout
    {
      numDebugLoops = 0;      // reset debug loop counter
      nextState     = NORMAL; // exit debug mode, go back to normal state
    }
  }

  return nextState;
} // end of checkDebugMsgs();

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
static FANCTRLSTATE_ENUM_TYPE initState ( FANCTRLSTATE_ENUM_TYPE thisState )
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
static FANCTRLSTATE_ENUM_TYPE normalState ( FANCTRLSTATE_ENUM_TYPE thisState )
{
  FANCTRLSTATE_ENUM_TYPE nextState = NORMAL;            // by default, stay in normal state
  char                   lcdBuff [ LCDCOLS * LCDROWS ]; // buffer of chars used for LCD printing

  /* If this is the first time entering this state, send message */
  if ( stateChange )
  {
    Serial.print ( "ENTERING NORMAL STATE\n" ); // write initializing message on serial
  }

  /* Update LCD if needed */
  if ( ++lcdLoops >= LCD_DEC || stateChange ) // if enough loops have occured or if this is first instance of NORMAL state, update LCD
  {
    lcdLoops = 0; // reset LCD loop counter

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

  }

  /* Set desired fan speeds based on temperature */
  setRefFanSpeeds ( );

  /* Regulate Fan Speeds to Track Reference Values */
  regFanSpeeds ( );

  return nextState;
} // end of normalState()



/******************************************************************************
* Function:
*   debugPi1State()
*
* Description:
*   Runs the DEBUG_PI1 state routine
*
* Arguments:
*   none
*
* Returns:
*   nextState - state to enter upon exiting this function
******************************************************************************/
static FANCTRLSTATE_ENUM_TYPE debugPi1State ( FANCTRLSTATE_ENUM_TYPE thisState )
{
  char lcdBuff [ LCDCOLS * LCDROWS ]; // buffer of chars used for LCD printing

  /* If this is the first time entering this state, send message */
  if ( stateChange )
  {
    Serial.print ( "ENTERING DEBUG PI1 STATE\n" ); // write initializing message on serial
  }

  /* Update LCD if needed */
  if ( ++lcdLoops >= LCD_DEC || stateChange ) // if enough loops have occured or if this is first instance of NORMAL state, update LCD
  {
    lcdLoops = 0; // reset LCD loop counter

    /* First line has Int Term (quarter counts), Int State (tenths of error count times seconds), output duty (counts) */
    sprintf ( lcdBuff, "%5d %6d %3u", pi1.getIntTerm ( ), pi1.getIntState ( ), Pwm1Duty ); // set fan info
    lcd.setCursor ( 0, 0 );                                                                // set cursor to start of first line on LCD
    lcd.print ( lcdBuff );                                                                 // print first line

    /* Second line has Prop Term (quarter counts), Error (rpm), speed feedback (rpm) */
    sprintf ( lcdBuff, "%5d %5d %4u", pi1.getPropTerm ( ), (int) Fan1RPMRef - Fan1RPM, Fan1RPM ); // set fan info
    lcd.setCursor ( 0, 1 );                                                                       // set cursor to start of second line on LCD
    lcd.print ( lcdBuff );

  }

  /* Set desired fan speeds based on temperature */
  setRefFanSpeeds ( );

  /* Replace Fan1 reference speed with first data word */
  Fan1RPMRef = *(unsigned int *) ( debugDatWords + 0 );

  /* Update PI1 gains with those specified in message (if different) */
  if ( pi1Kp != debugDatWords [ 1 ] ) // second word is Kp
  {
    pi1Kp = debugDatWords [ 1 ];
    saveVar ( &pi1Kp );
  }
  if ( pi1Ki != debugDatWords [ 2 ] ) // third word is Ki
  {
    pi1Ki = debugDatWords [ 2 ];
    saveVar ( &pi1Ki );
  }
  if ( pi1Ki != debugDatWords [ 3 ] ) // fourth word is Ki
  {
    pi1Ki = debugDatWords [ 3 ];
    saveVar ( &pi1Ki );
  }
  if ( pi1Imax != debugDatWords [ 4 ] ) // fifth word is Imax
  {
    pi1Imax = debugDatWords [ 4 ];
    saveVar ( &pi1Imax );
  }
  if ( pi1Imin != debugDatWords [ 5 ] ) // sixth word is Imin
  {
    pi1Imin = debugDatWords [ 5 ];
    saveVar ( &pi1Imin );
  }
  if ( fan1Filt != *(unsigned int *) ( debugDatWords + 6 ) ) // seventh word is Filt gain
  {
    fan1Filt = *(unsigned int *) ( debugDatWords + 6 );
    saveVar ( &fan1Filt );
  }

  /* Regulate Fan Speeds to Track Reference Values */
  regFanSpeeds ( );

  return thisState; // remain in same state
}                   // end of debugPi1State()


/******************************************************************************
* Function:
*   debugPi2State()
*
* Description:
*   Runs the DEBUG_PI2 state routine
*
* Arguments:
*   none
*
* Returns:
*   nextState - state to enter upon exiting this function
******************************************************************************/
static FANCTRLSTATE_ENUM_TYPE debugPi2State ( FANCTRLSTATE_ENUM_TYPE thisState )
{
  char lcdBuff [ LCDCOLS * LCDROWS ]; // buffer of chars used for LCD printing

  /* If this is the first time entering this state, send message */
  if ( stateChange )
  {
    Serial.print ( "ENTERING DEBUG PI2 STATE\n" ); // write initializing message on serial
  }

  /* Update LCD if needed */
  if ( ++lcdLoops >= LCD_DEC || stateChange ) // if enough loops have occured or if this is first instance of NORMAL state, update LCD
  {
    lcdLoops = 0; // reset LCD loop counter

    /* First line has Int Term (quarter counts), Int State (tenths of error count times seconds), output duty (counts) */
    sprintf ( lcdBuff, "%5d %6d %3u", pi2.getIntTerm ( ), pi2.getIntState ( ), Pwm2Duty ); // set fan info
    lcd.setCursor ( 0, 0 );                                                                // set cursor to start of first line on LCD
    lcd.print ( lcdBuff );                                                                 // print first line

    /* Second line has Prop Term (quarter counts), Error (rpm), speed feedback (rpm) */
    sprintf ( lcdBuff, "%5d %5d %4u", pi2.getPropTerm ( ), (int) Fan2RPMRef - Fan2RPM, Fan2RPM ); // set fan info
    lcd.setCursor ( 0, 1 );                                                                       // set cursor to start of second line on LCD
    lcd.print ( lcdBuff );

  }

  /* Set desired fan speeds based on temperature */
  setRefFanSpeeds ( );

  /* Replace Fan2 reference speed with first data word */
  Fan2RPMRef = *(unsigned int *) ( debugDatWords + 0 );

  /* Update PI2 gains with those specified in message (if different) */
  if ( pi2Kp != debugDatWords [ 1 ] ) // second word is Kp
  {
    pi2Kp = debugDatWords [ 1 ];
    saveVar ( &pi2Kp );
  }
  if ( pi2Ki != debugDatWords [ 2 ] ) // third word is Ki
  {
    pi2Ki = debugDatWords [ 2 ];
    saveVar ( &pi2Ki );
  }
  if ( pi2Ki != debugDatWords [ 3 ] ) // fourth word is Ki
  {
    pi2Ki = debugDatWords [ 3 ];
    saveVar ( &pi2Ki );
  }
  if ( pi2Imax != debugDatWords [ 4 ] ) // fifth word is Imax
  {
    pi2Imax = debugDatWords [ 4 ];
    saveVar ( &pi2Imax );
  }
  if ( pi2Imin != debugDatWords [ 5 ] ) // sixth word is Imin
  {
    pi2Imin = debugDatWords [ 5 ];
    saveVar ( &pi2Imin );
  }
  if ( fan2Filt != *(unsigned int *) ( debugDatWords + 6 ) ) // seventh word is Filt gain
  {
    fan2Filt = *(unsigned int *) ( debugDatWords + 6 );
    saveVar ( &fan2Filt );
  }

  /* Regulate Fan Speeds to Track Reference Values */
  regFanSpeeds ( );

  return thisState; // remain in same state
}                   // end of debugPi2State()


/******************************************************************************
* Function:
*   debugBtnState()
*
* Description:
*   Runs the DEBUG_BTN state routine
*
* Arguments:
*   none
*
* Returns:
*   nextState - state to enter upon exiting this function
******************************************************************************/
static FANCTRLSTATE_ENUM_TYPE debugBtnState ( FANCTRLSTATE_ENUM_TYPE thisState )
{

  static unsigned int btn1EdgCnt = 0;                // Number of rising edges in button 1 since beginning debug
  static unsigned int btn2EdgCnt = 0;                // Number of rising edges in button 1 since beginning debug
  static unsigned int btn3EdgCnt = 0;                // Number of rising edges in button 1 since beginning debug
  char                lcdBuff [ LCDCOLS * LCDROWS ]; // buffer of chars used for LCD printing

  /* If this is the first time entering this state, reset button edge counts */
  if ( stateChange )
  {
    btn1EdgCnt = 0;
    btn2EdgCnt = 0;
    btn3EdgCnt = 0;
    Serial.print ( "ENTERING DEBUG BUTTON STATE\n" ); // write initializing message on serial
  }

  /* If a rising edge occurs on a button, update the edge count */
  if ( btn1PressCnt == 1 )
    btn1EdgCnt++;
  if ( btn2PressCnt == 1 )
    btn2EdgCnt++;
  if ( btn3PressCnt == 1 )
    btn3EdgCnt++;

  /* Update LCD if needed */
  if ( ++lcdLoops >= LCD_DEC || stateChange ) // if enough loops have occured or if this is first instance of NORMAL state, update LCD
  {
    lcdLoops = 0; // reset LCD loop counter

    /* First line has button rising edge counts */
    sprintf ( lcdBuff, "  %4u %4u %4u", btn1EdgCnt, btn2EdgCnt, btn3EdgCnt ); // set button info
    lcd.setCursor ( 0, 0 );                                                   // set cursor to start of first line on LCD
    lcd.print ( lcdBuff );                                                    // print first line

    /* Second line has button consecutive counts */
    sprintf ( lcdBuff, "  %4u %4u %4u", btn1PressCnt, btn2PressCnt, btn3PressCnt ); // set button info
    lcd.setCursor ( 0, 1 );                                                         // set cursor to start of second line on LCD
    lcd.print ( lcdBuff );

  }

  /* Turn off fans */
  Pwm1Duty = 0; // set output to zero
  Pwm2Duty = 0; // set output to zero

  return thisState; // remain in same state
}                   // end of debugBtnState()

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
  FANCTRLSTATE_ENUM_TYPE thisState = state;

  switch ( thisState )
  {
  case INIT:
    state = initState ( thisState ); // initialize system then set next state
    break;

  case NORMAL:
    state = normalState ( thisState ); // run normal state then move on to next state
    break;

  case DEBUG_PI1:
    state = debugPi1State ( thisState ); // run debug PI1 state then move on to next state
    break;

  case DEBUG_PI2:
    state = debugPi2State ( thisState ); // run debug PI2  state then move on to next state
    break;

  case DEBUG_BTNS:
    state = debugBtnState ( thisState ); // run debug button  state then move on to next state
    break;

  default:
    reset ( ); // reset device, invalid state reached
  }

  /* Check to see if any debug messages are present.  If so, switch states. */
  state = checkDebugMsgs ( state );

  /* Check to see if state changed.  If so, raise state change flag */
  if ( state != thisState )
    stateChange = HIGH;
  else
    stateChange = LOW;

  return;
} // end of run
