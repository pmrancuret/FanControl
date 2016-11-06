/*
 * FanControlUtils.cpp
 *
 *  Created on: Nov 4, 2016
 *      Author: Paul
 */

/*******************************************************************************
 * INCLUDED HEADER FILES
 ******************************************************************************/
#include "FanControlUtils.h"
#include "savedVars.h"
#include "piController.h"
#include "LiquidCrystal.h"

/*******************************************************************************
 * CLASS DEFINITIONS
 ******************************************************************************/
/* Define the 'lcd' object used for interfacing with LCD screen */
LiquidCrystal lcd ( LCDRSPIN, // set the RS pin
  LCDRWPIN,                   // set the RW pin
  LCDENABLEPIN,               // set the Enable pin
  LCDD0PIN,                   // set the data 0 pin
  LCDD1PIN,                   // set the data 1 pin
  LCDD2PIN,                   // set the data 2 pin
  LCDD3PIN );                 // set the data 3 pin
piController pi1 ( LOOPTIME_US,
  pi1Imax,
  pi1Imin,
  pi1Kp,
  pi1Ki ); // PI controller #1 class object
piController pi2 ( LOOPTIME_US,
  pi2Imax,
  pi2Imin,
  pi2Kp,
  pi2Ki ); // PI controller #1 class object

/*******************************************************************************
 * GLOBAL VARIABLE DEFINITIONS
 ******************************************************************************/
volatile unsigned long lastEdgeTime1                       = 0;     // timestamp of previous edge of hall sensor 1
volatile unsigned long lastEdgeTime2                       = 0;     // timestamp of previous edge of hall sensor 2
volatile unsigned long hall1Period                         = 1;     // period count for hall sensor 1
volatile unsigned long hall2Period                         = 1;     // period count for hall sensor 2
unsigned int           Fan1RPM                             = 0;     // Fan 1 speed, in rpm
unsigned int           Fan2RPM                             = 0;     // Fan 2 speed, in rpm
unsigned int           Fan1RPMRef                          = 0;     // Fan 1 reference speed, in rpm
unsigned int           Fan2RPMRef                          = 0;     // Fan 2 reference speed, in rpm
byte                   Pwm1Duty                            = 0;     // PWM 1 duty cycle (0-255 maps to 0%-100%)
byte                   Pwm2Duty                            = 0;     // PWM 2 duty cycle (0-255 maps to 0%-100%)
unsigned int           Temp1                               = 0;     // Temperature 1 input, stored digitally (0-1023)
unsigned int           Temp2                               = 0;     // Temperature 2 input, stored digitally (0-1023)
unsigned long          loopsRun                            = 0;     // total number of loops run since reset
unsigned long          runTime_s                           = 0;     // program run-time since reset (seconds)
byte                   stateChange                         = 0;     // high when a state change occurs
unsigned int           btn1PressCnt                        = 0;     // number of consecutive times button 1 was pressed
unsigned int           btn2PressCnt                        = 0;     // number of consecutive times button 1 was pressed
unsigned int           btn3PressCnt                        = 0;     // number of consecutive times button 1 was pressed
byte                   lcdLoops                            = 0;     // number of loops run since last LCD update
int                    debugDatWords [ DEBUGMSG_DATWORDS ] = { 0 }; // buffer of data words included in payload of debug messages

/******************************************************************************
* Function:
*   measFanSpeeds()
*
* Description:
*   Calculates fan speeds, in RPM
*
* Arguments:
*   thisTime - timestamp value at which this function call occurs.
*
* Returns:
*   none
******************************************************************************/
void measFanSpeeds ( unsigned long thisTime )
{
  static unsigned int lastFan1RPM = 0; // last measured fan1 rpm
  static unsigned int lastFan2RPM = 0; // last measured fan2 rpm

  /* Calculate Fan speeds in RPM */
  if ( ( hall1Period >= ( ( (unsigned long) 1000000L / MINN1 ) * 60 / FAN1PPR ) ) ||                       // if fan period is too large
    ( ( ( thisTime - lastEdgeTime1 ) >> 5 ) >= ( ( (unsigned long) 1000000L / MINN1 ) * 60 / FAN1PPR ) ) ) // or time since last edge is too large
    Fan1RPM = 0;                                                                                           // use zero RPM value
  else
    Fan1RPM = (unsigned int) ( ( 1000000L / hall1Period ) * 60 / FAN1PPR );                                // Fan1 speed
  if ( Fan1RPM > MAXN1 )                                                                                   // if RPM is too large
    Fan1RPM = MAXN1;                                                                                       // restrict to max value
  if ( ( hall2Period >= ( ( (unsigned long) 1000000L / MINN2 ) * 60 / FAN2PPR ) ) ||                       // if fan period is too large
    ( ( ( thisTime - lastEdgeTime2 ) >> 5 ) >= ( ( (unsigned long) 1000000L / MINN2 ) * 60 / FAN2PPR ) ) ) // or time since last edge is too large
    Fan2RPM = 0;                                                                                           // use zero RPM value
  else
    Fan2RPM = (unsigned int) ( ( 1000000L / hall2Period ) * 60 / FAN2PPR );  // Fan2 speed
  if ( Fan2RPM > MAXN2 )                                                     // if RPM is too large
    Fan2RPM = MAXN2;                                                         // restrict to max value

  /* Low-pass filter the measured fan speeds */
  Fan1RPM     = (unsigned int) ( ( (unsigned long) lastFan1RPM * fan1Filt + (unsigned long) Fan1RPM * ( 1024 - fan1Filt ) ) >> 10 );
  lastFan1RPM = Fan1RPM;
  Fan2RPM     = (unsigned int) ( ( (unsigned long) lastFan2RPM * fan2Filt + (unsigned long) Fan2RPM * ( 1024 - fan2Filt ) ) >> 10 );
  lastFan2RPM = Fan2RPM;
  return;
} // end of measFanSpeeds()


/******************************************************************************
* Function:
*   checkButtonPress()
*
* Description:
*   checks if buttons were pressed, and updates consecutive press count
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
void checkButtonPress ( void )
{

  /* Check Button 1 */
  if ( digitalRead ( BTN1PIN ) ) // if button 1 input pin is high
    btn1PressCnt++;              // increase button press count (don't prevent overflow)
  else                           // if button 1 input pin is low
    btn1PressCnt = 0;            // set button press count to zero

  /* Check Button 2 */
  if ( digitalRead ( BTN2PIN ) ) // if button 2 input pin is high
    btn2PressCnt++;              // increase button press count (don't prevent overflow)
  else                           // if button 2 input pin is low
    btn2PressCnt = 0;            // set button press count to zero

  /* Check Button 3 */
  if ( digitalRead ( BTN3PIN ) ) // if button 3 input pin is high
    btn3PressCnt++;              // increase button press count (don't prevent overflow)
  else                           // if button 3 input pin is low
    btn3PressCnt = 0;            // set button press count to zero

  return;
} // end of checkButtonPress()


/******************************************************************************
* Function:
*   setRefFanSpeeds()
*
* Description:
*   sets reference fan speeds to track desired temperature
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
void setRefFanSpeeds ( void )
{
  unsigned int tempRef1;       // reference temperature used for fan 1
  unsigned int tempRef2;       // reference temperature used for fan 2
  long int     x0, x2, y0, y2; // variables used for interpolation

  /* Calculate reference temperatures based on source selection */
  switch ( tmpsrc1 ) // switch on temperature source for fan 1
  {
  case TMPSRC_TMP1:   // temp sensor 1
    tempRef1 = Temp1; // set temperature sensor 1
    break;

  case TMPSRC_TMP2:   // temp sensor 2
    tempRef1 = Temp2; // set temperature sensor 2
    break;

  case TMPSRC_MAX:                                 // maximum temperature
    tempRef1 = ( Temp1 >= Temp2 ) ? Temp1 : Temp2; // set maximum temperature
    break;

  case TMPSRC_MEAN:                    // mean temperature
    tempRef1 = ( Temp1 + Temp2 ) >> 1; // set mean temperature
    break;

  default:                                         // invalid source selection
    tmpsrc1  = TMPSRC_DEF;                         // set default (should be valid!)
    saveVar ( &tmpsrc1 );                          // save default
    tempRef1 = ( Temp1 >= Temp2 ) ? Temp1 : Temp2; // just use max temp for now - next time around it will use default (if default is different)
  }
  switch ( tmpsrc2 ) // switch on temperature source for fan 2
  {
  case TMPSRC_TMP1:   // temp sensor 1
    tempRef2 = Temp1; // set temperature sensor 1
    break;

  case TMPSRC_TMP2:   // temp sensor 2
    tempRef2 = Temp2; // set temperature sensor 2
    break;

  case TMPSRC_MAX:                                 // maximum temperature
    tempRef2 = ( Temp1 >= Temp2 ) ? Temp1 : Temp2; // set maximum temperature
    break;

  case TMPSRC_MEAN:                    // mean temperature
    tempRef2 = ( Temp1 + Temp2 ) >> 1; // set mean temperature
    break;

  default:                                         // invalid source selection
    tmpsrc2  = TMPSRC_DEF;                         // set default (should be valid!)
    saveVar ( &tmpsrc2 );                          // save default
    tempRef2 = ( Temp1 >= Temp2 ) ? Temp1 : Temp2; // just use max temp for now - next time around it will use default (if default is different)
  }

  /* Calculate Reference Fan 1 speed */
  if ( tempRef1 <= fan1TurnOffTmp )     // below turn-off temperature
    Fan1RPMRef = 0;                     // set speed to zero (turn off)
  else if ( tempRef1 <= fan1TurnOnTmp ) // between turn-off and turn-on temperature
  {
    if ( Fan1RPMRef > 0 )    // if fan was previously on
      Fan1RPMRef = minRpm1;  // set speed to minimum
    // if fan was previously off, it will remain off until turn-on temperature is reached
  }
  else if ( tempRef1 < fan1TblTmp4 ) // above turn-on temperature but below max table value
  {
    if ( tempRef1 < fan1TblTmp1 ) // below first table value, at or above turn-on point
    {
      x0 = (long int) fan1TurnOnTmp; // x-axis value at beginning of range
      x2 = (long int) fan1TblTmp1;   // x-axis value at end of range
      y0 = (long int) minRpm1;       // y-axis value at beginning of range
      y2 = (long int) fan1TblSpd1;   // y-axis value at end of range
    }
    else if ( tempRef1 < fan1TblTmp2 ) // below second table value, at or above first value
    {
      x0 = (long int) fan1TblTmp1; // x-axis value at beginning of range
      x2 = (long int) fan1TblTmp2; // x-axis value at end of range
      y0 = (long int) fan1TblSpd1; // y-axis value at beginning of range
      y2 = (long int) fan1TblSpd2; // y-axis value at end of range
    }
    else if ( tempRef1 < fan1TblTmp3 ) // below third table value, at or above second value
    {
      x0 = (long int) fan1TblTmp2; // x-axis value at beginning of range
      x2 = (long int) fan1TblTmp3; // x-axis value at end of range
      y0 = (long int) fan1TblSpd2; // y-axis value at beginning of range
      y2 = (long int) fan1TblSpd3; // y-axis value at end of range
    }
    else // below fourth table value, at or above third value
    {
      x0 = (long int) fan1TblTmp3; // x-axis value at beginning of range
      x2 = (long int) fan1TblTmp4; // x-axis value at end of range
      y0 = (long int) fan1TblSpd3; // y-axis value at beginning of range
      y2 = (long int) fan1TblSpd4; // y-axis value at end of range
    }
    Fan1RPMRef = (unsigned int) ( ( ( ( y2 - y0 ) * ( (long int) tempRef1 - x0 ) ) / ( x2 - x0 ) ) + y0 ); // interpolate to get results
  }
  else                         // at or above fourth table value
    Fan1RPMRef = fan1TblSpd4;  // use fourth table value

  /* Calculate Reference Fan 2 speed */
  if ( tempRef2 <= fan2TurnOffTmp )     // below turn-off temperature
    Fan2RPMRef = 0;                     // set speed to zero (turn off)
  else if ( tempRef2 <= fan2TurnOnTmp ) // between turn-off and turn-on temperature
  {
    if ( Fan2RPMRef > 0 )    // if fan was previously on
      Fan2RPMRef = minRpm2;  // set speed to minimum
    // if fan was previously off, it will remain off until turn-on temperature is reached
  }
  else if ( tempRef2 < fan2TblTmp4 ) // above turn-on temperature but below max table value
  {
    if ( tempRef2 < fan2TblTmp1 ) // below first table value, at or above turn-on point
    {
      x0 = (long int) fan2TurnOnTmp; // x-axis value at beginning of range
      x2 = (long int) fan2TblTmp1;   // x-axis value at end of range
      y0 = (long int) minRpm2;       // y-axis value at beginning of range
      y2 = (long int) fan2TblSpd1;   // y-axis value at end of range
    }
    else if ( tempRef2 < fan2TblTmp2 ) // below second table value, at or above first value
    {
      x0 = (long int) fan2TblTmp1; // x-axis value at beginning of range
      x2 = (long int) fan2TblTmp2; // x-axis value at end of range
      y0 = (long int) fan2TblSpd1; // y-axis value at beginning of range
      y2 = (long int) fan2TblSpd2; // y-axis value at end of range
    }
    else if ( tempRef2 < fan2TblTmp3 ) // below third table value, at or above second value
    {
      x0 = (long int) fan2TblTmp2; // x-axis value at beginning of range
      x2 = (long int) fan2TblTmp3; // x-axis value at end of range
      y0 = (long int) fan2TblSpd2; // y-axis value at beginning of range
      y2 = (long int) fan2TblSpd3; // y-axis value at end of range
    }
    else // below fourth table value, at or above third value
    {
      x0 = (long int) fan2TblTmp3; // x-axis value at beginning of range
      x2 = (long int) fan2TblTmp4; // x-axis value at end of range
      y0 = (long int) fan2TblSpd3; // y-axis value at beginning of range
      y2 = (long int) fan2TblSpd4; // y-axis value at end of range
    }
    Fan2RPMRef = (unsigned int) ( ( ( ( y2 - y0 ) * ( (long int) tempRef2 - x0 ) ) / ( x2 - x0 ) ) + y0 ); // interpolate to get results
  }
  else                         // at or above fourth table value
    Fan2RPMRef = fan2TblSpd4;  // use fourth table value

  return;
} // end of setRefFanSpeeds()


/******************************************************************************
* Function:
*   regFanSpeeds()
*
* Description:
*   regulates fan speeds to reference values
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
void regFanSpeeds ( void )
{
  /* make sure fan1 reference is within allowable range, and calculate duty */
  if ( Fan1RPMRef < minRpm1 )
  {
    Fan1RPMRef = 0; // set speed command to zero
    Pwm1Duty   = 0; // set output to zero
  }
  else if ( Fan1RPMRef > maxRpm1 )
  {
    Fan1RPMRef = maxRpm1;
    Pwm1Duty   = pi1.piControl ( (int) Fan1RPMRef - Fan1RPM ); // use constant duty for now
  }
  else
    Pwm1Duty = pi1.piControl ( (int) Fan1RPMRef - Fan1RPM ); // use constant duty for now

  /* make sure fan2 reference is within allowable range, and calculate duty */
  if ( Fan2RPMRef < minRpm2 )
  {
    Fan2RPMRef = 0; // set speed command to zero
    Pwm2Duty   = 0; // set output to zero
  }
  else if ( Fan2RPMRef > maxRpm2 )
  {
    Fan2RPMRef = maxRpm2;
    Pwm2Duty   = pi2.piControl ( (int) Fan2RPMRef - Fan2RPM ); // use constant duty for now
  }
  else
    Pwm2Duty = pi2.piControl ( (int) Fan2RPMRef - Fan2RPM ); // use constant duty for now

  return;
} // end of regFanSpeeds()

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
  unsigned long thisEdgeTime = micros ( ); // timestamp of current edge

  /* Hall Period (microseconds) calculated using the difference between
   * rising and falling edges, multiplied by two.  The multiply by two
   * is required because we are counting between any rising and falling
   * edge, which counts the time length of half a period. Since the timer0
   * speed is increased by 64, we need to also divide by 64.  So, right bit
   * shifting by 5 yields the overall divide by 32.*/
  hall1Period   = ( thisEdgeTime - lastEdgeTime1 ) >> 5; // calculate period in microseconds

  lastEdgeTime1 = thisEdgeTime; // save current edge time for next iteration

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
  unsigned long thisEdgeTime = micros ( ); // timestamp of current edge

  /* Hall Period (microseconds) calculated using the difference between
   * rising and falling edges, multiplied by two.  The multiply by two
   * is required because we are counting between any rising and falling
   * edge, which counts the time length of half a period. Since the timer0
   * speed is increased by 64, we need to also divide by 64.  So, right bit
   * shifting by 5 yields the overall divide by 32.*/
  hall2Period   = ( thisEdgeTime - lastEdgeTime2 ) >> 5; // calculate period in microseconds

  lastEdgeTime2 = thisEdgeTime; // save current edge time for next iteration

  return; // end of hall2ISR()
}         // end of hall2ISR()
