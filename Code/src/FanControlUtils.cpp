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
volatile unsigned long lastEdgeTime1 = 0; // timestamp of previous edge of hall sensor 1
volatile unsigned long lastEdgeTime2 = 0; // timestamp of previous edge of hall sensor 2
volatile unsigned long hall1Period   = 1; // period count for hall sensor 1
volatile unsigned long hall2Period   = 1; // period count for hall sensor 2
unsigned int           Fan1RPM       = 0; // Fan 1 speed, in rpm
unsigned int           Fan2RPM       = 0; // Fan 2 speed, in rpm
unsigned int           Fan1RPMRef    = 0; // Fan 1 reference speed, in rpm
unsigned int           Fan2RPMRef    = 0; // Fan 2 reference speed, in rpm
byte                   Pwm1Duty      = 0; // PWM 1 duty cycle (0-255 maps to 0%-100%)
byte                   Pwm2Duty      = 0; // PWM 2 duty cycle (0-255 maps to 0%-100%)
unsigned int           Temp1         = 0; // Temperature 1 input, stored digitally (0-1023)
unsigned int           Temp2         = 0; // Temperature 2 input, stored digitally (0-1023)

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
  /* set some dummy fan speed commands for now */
  Fan1RPMRef = 650;
  Fan2RPMRef = 1100;

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
  /* make sure fan reference is within allowable range, and calculate duty */
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


  /* Set duty cycle for pwm outputs */
  analogWrite ( PWM1PIN, Pwm1Duty ); // set pwm1 duty
  analogWrite ( PWM2PIN, Pwm2Duty ); // set pwm2 duty

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
