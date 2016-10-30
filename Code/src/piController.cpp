/*
 * piController.cpp
 *
 *  Created on: Oct 29, 2016
 *      Author: Paul
 */


#include "piController.h"


/*******************************************************************************
 * FUNCTION DEFINITIONS
 ******************************************************************************/


/******************************************************************************
* Function:
*   piController()
*
* Description:
*   Constructor function for a piController object
*
* Arguments:
*   sampleTimeSet - sets sample time (microseconds)
*   maxErrIntSet - sets max integrator value (tenths of error count times seconds)
*   minErrIntSet - sets min integrator value (tenths of error count times seconds)
*   kpSet - sets proportional gain (2^-13 duty counts per error count)
*   kiSet - sets integral gain (2^-17 duty counts per error count per 100 milliseconds)
*
* Returns:
*   none
******************************************************************************/
piController :: piController ( unsigned long sampleTimeSet, int maxErrIntSet, int minErrIntSet, int kpSet, int kiSet )
{
  sampleTime  = sampleTimeSet; // set sample time (microseconds)
  errIntegral = 0;             // integral of error (tenths of error count times seconds)
  maxErrInt   = maxErrIntSet;  // maximum value of errIntegral (tenths of error count times seconds)
  minErrInt   = minErrIntSet;  // maximum value of errIntegral (tenths of error count times seconds)
  kp          = kpSet;         // Proportional gain (2^-13 duty counts per error count)
  ki          = kiSet;         // Integral gain (2^-17 duty counts per error count per 100 milliseconds)
  IntTerm     = 0;             // start with integral term = 0
  PropTerm    = 0;             // start with proportional term = 0

  return; // exit function
}         // end of piController()

/******************************************************************************
* Function:
*   setGains()
*
* Description:
*   Updates gains in the piController object
*
* Arguments:
*   sampleTimeSet - sets sample time (microseconds)
*   maxErrIntSet - sets max integrator value (tenths of error count times seconds)
*   minErrIntSet - sets min integrator value (tenths of error count times seconds)
*   kpSet - sets proportional gain (2^-13 duty counts per error count)
*   kiSet - sets integral gain (2^-17 duty counts per error count per 100 milliseconds)
*
* Returns:
*   none
******************************************************************************/
void piController :: setGains ( unsigned long sampleTimeSet, int maxErrIntSet, int minErrIntSet, int kpSet, int kiSet )
{
  sampleTime = sampleTimeSet; // set sample time (microseconds)
  maxErrInt  = maxErrIntSet;  // maximum value of errIntegral (tenths of error count times seconds)
  minErrInt  = minErrIntSet;  // maximum value of errIntegral (tenths of error count times seconds)
  kp         = kpSet;         // Proportional gain (2^-13 duty counts per error count)
  ki         = kiSet;         // Integral gain (2^-17 duty counts per error count per 100 milliseconds)

  return; // exit function
}         // end of setGains()


/******************************************************************************
* Function:
*   resetInt()
*
* Description:
*   Resets integrator to zero
*
* Arguments:
*   none
*
* Returns:
*   none
******************************************************************************/
void piController :: resetInt ( void )
{
  errIntegral = 0; // set integrator to zero (tenths of error count times seconds)

  return; // exit function
}         // end of resetInt()

/******************************************************************************
* Function:
*   resetInt()
*
* Description:
*   Resets integrator to specified value
*
* Arguments:
*   errInt - value to set integrator to (tenths of error count times seconds)
*
* Returns:
*   none
******************************************************************************/
void piController :: resetInt ( int errInt )
{
  errIntegral = errInt; // set integrator to specified value (tenths of error count times seconds)

  return; // exit function
}         // end of resetInt()

/******************************************************************************
* Function:
*   piControl()
*
* Description:
*   Executes PI control step
*
* Arguments:
*   errVal - error term input to PI
*
* Returns:
*   none
******************************************************************************/
byte piController :: piControl ( int errVal )
{
  byte dutyOut; // output duty

  errIntegral = (int) constrain (
    ( ( (long int) errIntegral ) + ( ( ( (long int) errVal ) * (long int) sampleTime ) / 100000L ) ), // add to integrator, which holds error in tenths of rpm times seconds
    ( (long int) minErrInt ), ( (long int) maxErrInt ) );                                             // constrain to stay within integrator limits

  PropTerm = (int) constrain ( ( (long) errVal * kp ) >> 11, -16384, 16383 );      // proportional term, in quarter of duty counts
  IntTerm  = (int) constrain ( ( (long) errIntegral * ki ) >> 15, -16384, 16383 ); // integral term, in quarter of duty counts

  dutyOut  = (byte) constrain ( ( PropTerm + IntTerm ) >> 2, MINPIOUTPUT, MAXPIOUTPUT ); // set output duty

  return dutyOut; // exit function
}                 // end of piControl()


/******************************************************************************
* Function:
*   piControlIntOff()
*
* Description:
*   Executes PI control step without updating integrator.  This can be useful
*   to avoid anti-windup.  It does not set integrator term to zero, it just
*   neglects to update the integral of error.
*
* Arguments:
*   errVal - error term input to PI
*
* Returns:
*   none
******************************************************************************/
byte piController :: piControlIntOff ( int errVal )
{
  byte dutyOut; // output duty

  PropTerm = (int) constrain ( ( (long) errVal * kp ) >> 11, -16384, 16383 );      // proportional term, in quarter of duty counts
  IntTerm  = (int) constrain ( ( (long) errIntegral * ki ) >> 15, -16384, 16383 ); // integral term, in quarter of duty counts

  dutyOut  = (byte) constrain ( ( PropTerm + IntTerm ) >> 2, MINPIOUTPUT, MAXPIOUTPUT ); // set output duty

  return dutyOut; // exit function
}                 // end of piControlIntOff()


/******************************************************************************
* Function:
*   getPropTerm()
*
* Description:
*   returns the proportional term.  good for debugging.
*
* Arguments:
*   none
*
* Returns:
*   PropTerm - proportional term, in quarters of duty count
******************************************************************************/
int piController :: getPropTerm ( void )
{
  return PropTerm;
} // end of getPropTerm()

/******************************************************************************
* Function:
*   getIntTerm()
*
* Description:
*   returns the integral term.  good for debugging.
*
* Arguments:
*   none
*
* Returns:
*   IntTerm - integral term, in quarters of duty count
******************************************************************************/
int piController :: getIntTerm ( void )
{
  return IntTerm;
} // end of getIntTerm()

/******************************************************************************
* Function:
*   getIntState()
*
* Description:
*   returns the integrator state.  good for debugging.
*
* Arguments:
*   none
*
* Returns:
*   errIntegral - integrator state, (tenths of error count times seconds)
******************************************************************************/
int piController :: getIntState ( void )
{
  return errIntegral;
} // end of getIntState()
