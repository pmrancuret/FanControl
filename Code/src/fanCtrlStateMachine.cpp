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
      if ( strcmp ( msgHeader, NORMAL_HEAD ) == 0 )
      {
        nextState = NORMAL; // set next state to return to normal
        break;              // exit loop, don't load debug message data
      }
      else if ( strcmp ( msgHeader, DEBUGPI1_HEAD ) == 0 )
        nextState = DEBUG_PI1; // set next state to requested debug state
      else if ( strcmp ( msgHeader, DEBUGPI2_HEAD ) == 0 )
        nextState = DEBUG_PI2; // set next state to requested debug state
      else if ( strcmp ( msgHeader, DEBUGBTN_HEAD ) == 0 )
        nextState = DEBUG_BTNS; // set next state to requested debug state
      else if ( strcmp ( msgHeader, DEBUGTMP_HEAD ) == 0 )
        nextState = DEBUG_TMP; // set next state to requested debug state
      else if ( strcmp ( msgHeader, DEBUGFON_HEAD ) == 0 )
        nextState = DEBUG_FON; // set next state to requested debug state
      else if ( strcmp ( msgHeader, DEBUGTB1_HEAD ) == 0 )
        nextState = DEBUG_TB1; // set next state to requested debug state
      else if ( strcmp ( msgHeader, DEBUGTB2_HEAD ) == 0 )
        nextState = DEBUG_TB2; // set next state to requested debug state
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
    nextState == DEBUG_BTNS ||
    nextState == DEBUG_TMP ||
    nextState == DEBUG_FON ||
    nextState == DEBUG_TB1 ||
    nextState == DEBUG_TB2 ) // if we are in (or entering) debug state
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
  if ( pi1Imax != debugDatWords [ 3 ] ) // fourth word is Imax
  {
    pi1Imax = debugDatWords [ 3 ];
    saveVar ( &pi1Imax );
  }
  if ( pi1Imin != debugDatWords [ 4 ] ) // fifth word is Imin
  {
    pi1Imin = debugDatWords [ 4 ];
    saveVar ( &pi1Imin );
  }
  if ( fan1Filt != *(unsigned int *) ( debugDatWords + 5 ) ) // sixth word is Filt gain
  {
    fan1Filt = *(unsigned int *) ( debugDatWords + 5 );
    saveVar ( &fan1Filt );
  }
  if ( minRpm1 != *(unsigned int *) ( debugDatWords + 6 ) ) // seventh word is min rpm setting
  {
    minRpm1 = *(unsigned int *) ( debugDatWords + 6 );
    saveVar ( &minRpm1 );
  }
  if ( maxRpm1 != *(unsigned int *) ( debugDatWords + 7 ) ) // eighth word is max rpm setting
  {
    maxRpm1 = *(unsigned int *) ( debugDatWords + 7 );
    saveVar ( &maxRpm1 );
  }


  /* Regulate Fan Speeds to Track Reference Values */
  regFanSpeeds ( );
  Pwm2Duty = 0; // set output to zero

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
  if ( pi2Imax != debugDatWords [ 3 ] ) // fourth word is Imax
  {
    pi2Imax = debugDatWords [ 3 ];
    saveVar ( &pi2Imax );
  }
  if ( pi2Imin != debugDatWords [ 4 ] ) // fifth word is Imin
  {
    pi2Imin = debugDatWords [ 4 ];
    saveVar ( &pi2Imin );
  }
  if ( fan2Filt != *(unsigned int *) ( debugDatWords + 5 ) ) // sixth word is Filt gain
  {
    fan2Filt = *(unsigned int *) ( debugDatWords + 5 );
    saveVar ( &fan2Filt );
  }
  if ( minRpm2 != *(unsigned int *) ( debugDatWords + 6 ) ) // seventh word is min rpm setting
  {
    minRpm2 = *(unsigned int *) ( debugDatWords + 6 );
    saveVar ( &minRpm2 );
  }
  if ( maxRpm2 != *(unsigned int *) ( debugDatWords + 7 ) ) // eighth word is max rpm setting
  {
    maxRpm2 = *(unsigned int *) ( debugDatWords + 7 );
    saveVar ( &maxRpm2 );
  }

  /* Regulate Fan Speeds to Track Reference Values */
  regFanSpeeds ( );
  Pwm1Duty = 0; // set output to zero

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
*   debugTmpState()
*
* Description:
*   Runs the DEBUG_TMP state routine
*
* Arguments:
*   none
*
* Returns:
*   nextState - state to enter upon exiting this function
******************************************************************************/
static FANCTRLSTATE_ENUM_TYPE debugTmpState ( FANCTRLSTATE_ENUM_TYPE thisState )
{
  char lcdBuff [ LCDCOLS * LCDROWS ]; // buffer of chars used for LCD printing

  /* If this is the first time entering this state, reset button edge counts */
  if ( stateChange )
  {
    Serial.print ( "ENTERING DEBUG TEMP SENSORS STATE\n" ); // write initializing message on serial
  }

  /* Update Temp Sensor Parameters with those specified in message (if different) */
  if ( useFtemp != *(unsigned int *) ( debugDatWords + 0 ) ) // first word is whether or not to use fahrenheit units
  {
    useFtemp = *(unsigned int *) ( debugDatWords + 0 );
    saveVar ( &useFtemp );
  }
  if ( Temp1Offset != debugDatWords [ 1 ] ) // second word is offset for temp sensor 1
  {
    Temp1Offset = debugDatWords [ 1 ];
    saveVar ( &Temp1Offset );
  }
  if ( Temp1DegCPer5V != debugDatWords [ 2 ] ) // third word is gain for temp sensor 1
  {
    Temp1DegCPer5V = debugDatWords [ 2 ];
    saveVar ( &Temp1DegCPer5V );
  }
  if ( Temp2Offset != debugDatWords [ 3 ] ) // fourth word is offset for temp sensor 2
  {
    Temp2Offset = debugDatWords [ 3 ];
    saveVar ( &Temp2Offset );
  }
  if ( Temp2DegCPer5V != debugDatWords [ 4 ] ) // fifth word is gain for temp sensor 2
  {
    Temp2DegCPer5V = debugDatWords [ 4 ];
    saveVar ( &Temp2DegCPer5V );
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

    /* Second line has raw temp readings */
    sprintf ( lcdBuff, "Raw: %5u %5u", Temp1, Temp2 ); // set raw temp info
    lcd.setCursor ( 0, 1 );                            // set cursor to start of second line on LCD
    lcd.print ( lcdBuff );

  }

  /* Turn off fans */
  Pwm1Duty = 0; // set output to zero
  Pwm2Duty = 0; // set output to zero

  return thisState; // remain in same state
}                   // end of debugTmpState()


/******************************************************************************
* Function:
*   debugFonState()
*
* Description:
*   Runs the DEBUG_FON state routine
*
* Arguments:
*   none
*
* Returns:
*   nextState - state to enter upon exiting this function
******************************************************************************/
static FANCTRLSTATE_ENUM_TYPE debugFonState ( FANCTRLSTATE_ENUM_TYPE thisState )
{
  char                lcdBuff [ LCDCOLS * LCDROWS ]; // buffer of chars used for LCD printing
  static unsigned int dispCnt  = 0;                  // count of how many times displayed the same message.  Will swap between displaying fan1 and fan2 info as count increases.
  static byte         dispFan2 = 0;                  // display fan 2 when high.  otherwise, display fan 1.

  /* If this is the first time entering this state, reset button edge counts */
  if ( stateChange )
  {
    Serial.print ( "ENTERING DEBUG FAN ON/OFF SETTINGS STATE\n" ); // write initializing message on serial
  }

  /* Update Fan on/off control Parameters with those specified in message (if different) */
  if ( tmpsrc1 != *(unsigned int *) ( debugDatWords + 0 ) ) // first word gives setting of which temp sensor is used for fan 1 control
  {
    tmpsrc1 = *(unsigned int *) ( debugDatWords + 0 );
    saveVar ( &tmpsrc1 );
  }
  if ( fan1TurnOffTmp != *(unsigned int *) ( debugDatWords + 1 ) ) // second word gives fan 1 turnoff temp
  {
    fan1TurnOffTmp = *(unsigned int *) ( debugDatWords + 1 );
    saveVar ( &fan1TurnOffTmp );
  }
  if ( fan1TurnOnTmp != *(unsigned int *) ( debugDatWords + 2 ) ) // third word gives fan 1 turnon temp
  {
    fan1TurnOnTmp = *(unsigned int *) ( debugDatWords + 2 );
    saveVar ( &fan1TurnOnTmp );
  }
  if ( minRpm1 != *(unsigned int *) ( debugDatWords + 3 ) ) // fourth word gives fan 1 min speed (turn-on speed)
  {
    minRpm1 = *(unsigned int *) ( debugDatWords + 3 );
    saveVar ( &minRpm1 );
  }
  if ( tmpsrc2 != *(unsigned int *) ( debugDatWords + 4 ) ) // fifth word gives setting of which temp sensor is used for fan 2 control
  {
    tmpsrc2 = *(unsigned int *) ( debugDatWords + 4 );
    saveVar ( &tmpsrc2 );
  }
  if ( fan2TurnOffTmp != *(unsigned int *) ( debugDatWords + 5 ) ) // sixth word gives fan 2 turnoff temp
  {
    fan2TurnOffTmp = *(unsigned int *) ( debugDatWords + 5 );
    saveVar ( &fan2TurnOffTmp );
  }
  if ( fan2TurnOnTmp != *(unsigned int *) ( debugDatWords + 6 ) ) // seventh word gives fan 2 turnon temp
  {
    fan2TurnOnTmp = *(unsigned int *) ( debugDatWords + 6 );
    saveVar ( &fan2TurnOnTmp );
  }
  if ( minRpm2 != *(unsigned int *) ( debugDatWords + 7 ) ) // eighth word gives fan 2 min speed (turn-on speed)
  {
    minRpm2 = *(unsigned int *) ( debugDatWords + 7 );
    saveVar ( &minRpm2 );
  }

  /* Update LCD if needed */
  if ( ++lcdLoops >= LCD_DEC || stateChange ) // if enough loops have occured or if this is first instance of NORMAL state, update LCD
  {
    lcdLoops = 0; // reset LCD loop counter

    if ( ++dispCnt > DEBUG_DISPSWITCH )
    {
      dispCnt   = 0; // reset display count
      dispFan2 ^= 1; // toggle fan display
    }

    if ( dispFan2 ) // display fan 2 data
    {
      /* Mark Temperature on first line of LCD display */
      if ( useFtemp )
      {
        sprintf ( lcdBuff, "2%cF: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToF10 ( fan2TurnOffTmp ) / 10,
          abs ( DigTemp1ToF10 ( fan2TurnOffTmp ) ) % 10,
          DigTemp2ToF10 ( fan2TurnOnTmp ) / 10,
          abs ( DigTemp2ToF10 ( fan2TurnOnTmp ) ) % 10 ); // set temperatures as first line
      }
      else
      {
        sprintf ( lcdBuff, "2%cC: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToC10 ( fan2TurnOffTmp ) / 10,
          abs ( DigTemp1ToC10 ( fan2TurnOffTmp ) ) % 10,
          DigTemp2ToC10 ( fan2TurnOnTmp ) / 10,
          abs ( DigTemp2ToC10 ( fan2TurnOnTmp ) ) % 10 ); // set temperatures as first line
      }
      lcd.setCursor ( 0, 0 ); // set cursor to start of first line on LCD
      lcd.print ( lcdBuff );  // print first line

      /* Second line has temp source and min speed */
      switch ( tmpsrc2 )
      {
      case TMPSRC_TMP1:                                 // temp sensor 1
        sprintf ( lcdBuff, "2: TEMP1   %5u", minRpm2 ); /// set control info
        break;

      case TMPSRC_TMP2:                                 // temp sensor 2
        sprintf ( lcdBuff, "2: TEMP2   %5u", minRpm2 ); // set control info
        break;

      case TMPSRC_MAX:                                  // Max Temp
        sprintf ( lcdBuff, "2: MAX     %5u", minRpm2 ); // set control info
        break;

      case TMPSRC_MEAN:                                 // Mean Temp
        sprintf ( lcdBuff, "2: MEAN    %5u", minRpm2 ); // set control info
        break;

      default:                                          // invalid selection
        tmpsrc2 = TMPSRC_DEF;                           // set default (should be valid!)
        saveVar ( &tmpsrc2 );                           // save default
        sprintf ( lcdBuff, "2: MAX     %5u", minRpm2 ); // set control info
      }
      lcd.setCursor ( 0, 1 ); // set cursor to start of second line on LCD
      lcd.print ( lcdBuff );

    }
    else // display fan 1 data
    {
      /* Mark Temperature on first line of LCD display */
      if ( useFtemp )
      {
        sprintf ( lcdBuff, "1%cF: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToF10 ( fan1TurnOffTmp ) / 10,
          abs ( DigTemp1ToF10 ( fan1TurnOffTmp ) ) % 10,
          DigTemp2ToF10 ( fan1TurnOnTmp ) / 10,
          abs ( DigTemp2ToF10 ( fan1TurnOnTmp ) ) % 10 ); // set temperatures as first line
      }
      else
      {
        sprintf ( lcdBuff, "1%cC: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToC10 ( fan1TurnOffTmp ) / 10,
          abs ( DigTemp1ToC10 ( fan1TurnOffTmp ) ) % 10,
          DigTemp2ToC10 ( fan1TurnOnTmp ) / 10,
          abs ( DigTemp2ToC10 ( fan1TurnOnTmp ) ) % 10 ); // set temperatures as first line
      }
      lcd.setCursor ( 0, 0 ); // set cursor to start of first line on LCD
      lcd.print ( lcdBuff );  // print first line

      /* Second line has temp source and min speed */
      switch ( tmpsrc1 )
      {
      case TMPSRC_TMP1:                                 // temp sensor 1
        sprintf ( lcdBuff, "1: TEMP1   %5u", minRpm1 ); /// set control info
        break;

      case TMPSRC_TMP2:                                 // temp sensor 2
        sprintf ( lcdBuff, "1: TEMP2   %5u", minRpm1 ); // set control info
        break;

      case TMPSRC_MAX:                                  // Max Temp
        sprintf ( lcdBuff, "1: MAX     %5u", minRpm1 ); // set control info
        break;

      case TMPSRC_MEAN:                                 // Mean Temp
        sprintf ( lcdBuff, "1: MEAN    %5u", minRpm1 ); // set control info
        break;

      default:                                          // invalid selection
        tmpsrc1 = TMPSRC_DEF;                           // set default (should be valid!)
        saveVar ( &tmpsrc1 );                           // save default
        sprintf ( lcdBuff, "1: MAX     %5u", minRpm1 ); // set control info
      }
      lcd.setCursor ( 0, 1 ); // set cursor to start of second line on LCD
      lcd.print ( lcdBuff );

    }

  }

  /* Turn off fans */
  Pwm1Duty = 0; // set output to zero
  Pwm2Duty = 0; // set output to zero

  return thisState; // remain in same state
}                   // end of debugFonState()


/******************************************************************************
* Function:
*   debugTb1State()
*
* Description:
*   Runs the DEBUG_TB1 state routine
*
* Arguments:
*   none
*
* Returns:
*   nextState - state to enter upon exiting this function
******************************************************************************/
static FANCTRLSTATE_ENUM_TYPE debugTb1State ( FANCTRLSTATE_ENUM_TYPE thisState )
{
  char                lcdBuff [ LCDCOLS * LCDROWS ]; // buffer of chars used for LCD printing
  static unsigned int dispCnt     = 0;               // count of how many times displayed the same message.  Will swap between displaying fan1 and fan2 info as count increases.
  static byte         dispSecHalf = 0;               // display second half of table when high.  otherwise, display first half of table.

  /* If this is the first time entering this state, reset button edge counts */
  if ( stateChange )
  {
    Serial.print ( "ENTERING DEBUG LOOKUP TABLE 1 STATE\n" ); // write initializing message on serial
  }

  /* Update Fan Lookup Table 1 Parameters with those specified in message (if different) */
  if ( fan1TblTmp1 != *(unsigned int *) ( debugDatWords + 0 ) ) // first word gives first temp value in table
  {
    fan1TblTmp1 = *(unsigned int *) ( debugDatWords + 0 );
    saveVar ( &fan1TblTmp1 );
  }
  if ( fan1TblTmp2 != *(unsigned int *) ( debugDatWords + 1 ) ) // second word gives second temp value in table
  {
    fan1TblTmp2 = *(unsigned int *) ( debugDatWords + 1 );
    saveVar ( &fan1TblTmp2 );
  }
  if ( fan1TblTmp3 != *(unsigned int *) ( debugDatWords + 2 ) ) // third word gives third temp value in table
  {
    fan1TblTmp3 = *(unsigned int *) ( debugDatWords + 2 );
    saveVar ( &fan1TblTmp3 );
  }
  if ( fan1TblTmp4 != *(unsigned int *) ( debugDatWords + 3 ) ) // fourth word gives fourth temp value in table
  {
    fan1TblTmp4 = *(unsigned int *) ( debugDatWords + 3 );
    saveVar ( &fan1TblTmp4 );
  }
  if ( fan1TblSpd1 != *(unsigned int *) ( debugDatWords + 4 ) ) // fifth word gives first speed value in table
  {
    fan1TblSpd1 = *(unsigned int *) ( debugDatWords + 4 );
    saveVar ( &fan1TblSpd1 );
  }
  if ( fan1TblSpd2 != *(unsigned int *) ( debugDatWords + 5 ) ) // sixth word gives second speed value in table
  {
    fan1TblSpd2 = *(unsigned int *) ( debugDatWords + 5 );
    saveVar ( &fan1TblSpd2 );
  }
  if ( fan1TblSpd3 != *(unsigned int *) ( debugDatWords + 6 ) ) // seventh word gives third speed value in table
  {
    fan1TblSpd3 = *(unsigned int *) ( debugDatWords + 6 );
    saveVar ( &fan1TblSpd3 );
  }
  if ( fan1TblSpd4 != *(unsigned int *) ( debugDatWords + 7 ) ) // eighth word gives fourth speed value in table
  {
    fan1TblSpd4 = *(unsigned int *) ( debugDatWords + 7 );
    saveVar ( &fan1TblSpd4 );
  }

  /* Update LCD if needed */
  if ( ++lcdLoops >= LCD_DEC || stateChange ) // if enough loops have occured or if this is first instance of NORMAL state, update LCD
  {
    lcdLoops = 0; // reset LCD loop counter

    if ( ++dispCnt > DEBUG_DISPSWITCH )
    {
      dispCnt      = 0; // reset display count
      dispSecHalf ^= 1; // toggle fan display
    }

    if ( dispSecHalf ) // display second half of table
    {
      /* Mark Temperature on first line of LCD display */
      if ( useFtemp )
      {
        sprintf ( lcdBuff, " %cF: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToF10 ( fan1TblTmp3 ) / 10,
          abs ( DigTemp1ToF10 ( fan1TblTmp3 ) ) % 10,
          DigTemp2ToF10 ( fan1TblTmp4 ) / 10,
          abs ( DigTemp2ToF10 ( fan1TblTmp4 ) ) % 10 ); // set temperatures as first line
      }
      else
      {
        sprintf ( lcdBuff, " %cC: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToC10 ( fan1TblTmp3 ) / 10,
          abs ( DigTemp1ToC10 ( fan1TblTmp4 ) ) % 10,
          DigTemp2ToC10 ( fan1TblTmp3 ) / 10,
          abs ( DigTemp2ToC10 ( fan1TblTmp4 ) ) % 10 ); // set temperatures as first line
      }
      lcd.setCursor ( 0, 0 ); // set cursor to start of first line on LCD
      lcd.print ( lcdBuff );  // print first line

      /* Mark hall-sensor period values on second line of LCD display */
      sprintf ( lcdBuff, "RPM:  %4hu  %4hu", fan1TblSpd3, fan1TblSpd4 ); // set fan speeds
      lcd.setCursor ( 0, 1 );                                            // set cursor to start of second line on LCD
      lcd.print ( lcdBuff );                                             // print second line

    }
    else // display first half of table
    {
      /* Mark Temperature on first line of LCD display */
      if ( useFtemp )
      {
        sprintf ( lcdBuff, " %cF: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToF10 ( fan1TblTmp1 ) / 10,
          abs ( DigTemp1ToF10 ( fan1TblTmp1 ) ) % 10,
          DigTemp2ToF10 ( fan1TblTmp2 ) / 10,
          abs ( DigTemp2ToF10 ( fan1TblTmp2 ) ) % 10 ); // set temperatures as first line
      }
      else
      {
        sprintf ( lcdBuff, " %cC: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToC10 ( fan1TblTmp1 ) / 10,
          abs ( DigTemp1ToC10 ( fan1TblTmp1 ) ) % 10,
          DigTemp2ToC10 ( fan1TblTmp2 ) / 10,
          abs ( DigTemp2ToC10 ( fan1TblTmp2 ) ) % 10 ); // set temperatures as first line
      }
      lcd.setCursor ( 0, 0 ); // set cursor to start of first line on LCD
      lcd.print ( lcdBuff );  // print first line

      /* Mark hall-sensor period values on second line of LCD display */
      sprintf ( lcdBuff, "RPM:  %4hu  %4hu", fan1TblSpd1, fan1TblSpd2 ); // set fan speeds
      lcd.setCursor ( 0, 1 );                                            // set cursor to start of second line on LCD
      lcd.print ( lcdBuff );                                             // print second line

    }

  }

  /* Turn off fans */
  Pwm1Duty = 0; // set output to zero
  Pwm2Duty = 0; // set output to zero

  return thisState; // remain in same state
}                   // end of debugTb1State()


/******************************************************************************
* Function:
*   debugTb2State()
*
* Description:
*   Runs the DEBUG_TB2 state routine
*
* Arguments:
*   none
*
* Returns:
*   nextState - state to enter upon exiting this function
******************************************************************************/
static FANCTRLSTATE_ENUM_TYPE debugTb2State ( FANCTRLSTATE_ENUM_TYPE thisState )
{
  char                lcdBuff [ LCDCOLS * LCDROWS ]; // buffer of chars used for LCD printing
  static unsigned int dispCnt     = 0;               // count of how many times displayed the same message.  Will swap between displaying fan1 and fan2 info as count increases.
  static byte         dispSecHalf = 0;               // display second half of table when high.  otherwise, display first half of table.

  /* If this is the first time entering this state, reset button edge counts */
  if ( stateChange )
  {
    Serial.print ( "ENTERING DEBUG LOOKUP TABLE 2 STATE\n" ); // write initializing message on serial
  }

  /* Update Fan Lookup Table 2 Parameters with those specified in message (if different) */
  if ( fan2TblTmp1 != *(unsigned int *) ( debugDatWords + 0 ) ) // first word gives first temp value in table
  {
    fan2TblTmp1 = *(unsigned int *) ( debugDatWords + 0 );
    saveVar ( &fan2TblTmp1 );
  }
  if ( fan2TblTmp2 != *(unsigned int *) ( debugDatWords + 1 ) ) // second word gives second temp value in table
  {
    fan2TblTmp2 = *(unsigned int *) ( debugDatWords + 1 );
    saveVar ( &fan2TblTmp2 );
  }
  if ( fan2TblTmp3 != *(unsigned int *) ( debugDatWords + 2 ) ) // third word gives third temp value in table
  {
    fan2TblTmp3 = *(unsigned int *) ( debugDatWords + 2 );
    saveVar ( &fan2TblTmp3 );
  }
  if ( fan2TblTmp4 != *(unsigned int *) ( debugDatWords + 3 ) ) // fourth word gives fourth temp value in table
  {
    fan2TblTmp4 = *(unsigned int *) ( debugDatWords + 3 );
    saveVar ( &fan2TblTmp4 );
  }
  if ( fan2TblSpd1 != *(unsigned int *) ( debugDatWords + 4 ) ) // fifth word gives first speed value in table
  {
    fan2TblSpd1 = *(unsigned int *) ( debugDatWords + 4 );
    saveVar ( &fan2TblSpd1 );
  }
  if ( fan2TblSpd2 != *(unsigned int *) ( debugDatWords + 5 ) ) // sixth word gives second speed value in table
  {
    fan2TblSpd2 = *(unsigned int *) ( debugDatWords + 5 );
    saveVar ( &fan2TblSpd2 );
  }
  if ( fan2TblSpd3 != *(unsigned int *) ( debugDatWords + 6 ) ) // seventh word gives third speed value in table
  {
    fan2TblSpd3 = *(unsigned int *) ( debugDatWords + 6 );
    saveVar ( &fan2TblSpd3 );
  }
  if ( fan2TblSpd4 != *(unsigned int *) ( debugDatWords + 7 ) ) // eighth word gives fourth speed value in table
  {
    fan2TblSpd4 = *(unsigned int *) ( debugDatWords + 7 );
    saveVar ( &fan2TblSpd4 );
  }

  /* Update LCD if needed */
  if ( ++lcdLoops >= LCD_DEC || stateChange ) // if enough loops have occured or if this is first instance of NORMAL state, update LCD
  {
    lcdLoops = 0; // reset LCD loop counter

    if ( ++dispCnt > DEBUG_DISPSWITCH )
    {
      dispCnt      = 0; // reset display count
      dispSecHalf ^= 1; // toggle fan display
    }

    if ( dispSecHalf ) // display second half of table
    {
      /* Mark Temperature on first line of LCD display */
      if ( useFtemp )
      {
        sprintf ( lcdBuff, " %cF: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToF10 ( fan2TblTmp3 ) / 10,
          abs ( DigTemp1ToF10 ( fan2TblTmp3 ) ) % 10,
          DigTemp2ToF10 ( fan2TblTmp4 ) / 10,
          abs ( DigTemp2ToF10 ( fan2TblTmp4 ) ) % 10 ); // set temperatures as first line
      }
      else
      {
        sprintf ( lcdBuff, " %cC: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToC10 ( fan2TblTmp3 ) / 10,
          abs ( DigTemp1ToC10 ( fan2TblTmp4 ) ) % 10,
          DigTemp2ToC10 ( fan2TblTmp3 ) / 10,
          abs ( DigTemp2ToC10 ( fan2TblTmp4 ) ) % 10 ); // set temperatures as first line
      }
      lcd.setCursor ( 0, 0 ); // set cursor to start of first line on LCD
      lcd.print ( lcdBuff );  // print first line

      /* Mark hall-sensor period values on second line of LCD display */
      sprintf ( lcdBuff, "RPM:  %4hu  %4hu", fan2TblSpd3, fan2TblSpd4 ); // set fan speeds
      lcd.setCursor ( 0, 1 );                                            // set cursor to start of second line on LCD
      lcd.print ( lcdBuff );                                             // print second line

    }
    else // display first half of table
    {
      /* Mark Temperature on first line of LCD display */
      if ( useFtemp )
      {
        sprintf ( lcdBuff, " %cF: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToF10 ( fan2TblTmp1 ) / 10,
          abs ( DigTemp1ToF10 ( fan2TblTmp1 ) ) % 10,
          DigTemp2ToF10 ( fan2TblTmp2 ) / 10,
          abs ( DigTemp2ToF10 ( fan2TblTmp2 ) ) % 10 ); // set temperatures as first line
      }
      else
      {
        sprintf ( lcdBuff, " %cC: %3hu.%hu %3hu.%hu",
          0xDF,
          DigTemp1ToC10 ( fan2TblTmp1 ) / 10,
          abs ( DigTemp1ToC10 ( fan2TblTmp1 ) ) % 10,
          DigTemp2ToC10 ( fan2TblTmp2 ) / 10,
          abs ( DigTemp2ToC10 ( fan2TblTmp2 ) ) % 10 ); // set temperatures as first line
      }
      lcd.setCursor ( 0, 0 ); // set cursor to start of first line on LCD
      lcd.print ( lcdBuff );  // print first line

      /* Mark hall-sensor period values on second line of LCD display */
      sprintf ( lcdBuff, "RPM:  %4hu  %4hu", fan2TblSpd1, fan2TblSpd2 ); // set fan speeds
      lcd.setCursor ( 0, 1 );                                            // set cursor to start of second line on LCD
      lcd.print ( lcdBuff );                                             // print second line

    }

  }

  /* Turn off fans */
  Pwm1Duty = 0; // set output to zero
  Pwm2Duty = 0; // set output to zero

  return thisState; // remain in same state
}                   // end of debugTb2State()


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
    state = debugBtnState ( thisState ); // run debug button state then move on to next state
    break;

  case DEBUG_TMP:
    state = debugTmpState ( thisState ); // run debug temp sensor state then move on to next state
    break;

  case DEBUG_FON:
    state = debugFonState ( thisState ); // run fan on/off settings debug state then move on to next state
    break;

  case DEBUG_TB1:
    state = debugTb1State ( thisState ); // run fan on/off settings debug state then move on to next state
    break;

  case DEBUG_TB2:
    state = debugTb2State ( thisState ); // run fan on/off settings debug state then move on to next state
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
