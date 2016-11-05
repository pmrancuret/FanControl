/*
 * piController.h
 *
 *  Created on: Oct 29, 2016
 *      Author: Paul
 */

#ifndef PICONTROLLER_H_
#define PICONTROLLER_H_

#include "Arduino.h"

/*******************************************************************************
 * MACRO DEFINITIONS
 ******************************************************************************/
#define MINPIOUTPUT 1   // minimum PI output control value (must be >= 0, <= 255)
#define MAXPIOUTPUT 255 // maximum PI output control value (must be >=0, <= 255, >= MINOUTPUT)

/*******************************************************************************
 * CLASS DECLARATIONS
 ******************************************************************************/

/*
 * Class:		piController
 * Function:	NA
 * Scope:		global
 * Arguments:	NA
 * Description:	This class contains the PI Controller functions and data
 */
class piController
{
private:
unsigned long sampleTime;  // sample time, (microseconds)
int           errIntegral; // integral of error  (tenths of error count times seconds)
int           maxErrInt;   // maximum value of errIntegral  (tenths of error count times seconds)
int           minErrInt;   // maximum value of errIntegral  (tenths of error count times seconds)
int           kp;          // Proportional gain (2^-13 duty counts per error count)
int           ki;          // Integral gain (2^-17 duty counts per error count per 100 milliseconds)
int           PropTerm;    // proportional term (quarter of duty counts)
int           IntTerm;     // integral term (quarter of duty counts)

public:
piController ( unsigned long sampleTimeSet,
  int                        maxErrIntSet,
  int                        minErrIntSet,
  int                        kpSet,
  int                        kiSet ); // constructor for piController
void setGains ( unsigned long sampleTimeSet,
  int                         maxErrIntSet,
  int                         minErrIntSet,
  int                         kpSet,
  int                         kiSet ); // method for updating gains of PI controller
void resetInt ( void );                // resets integral term to zero
void resetInt ( int errInt );          // resets integral term to specified value
byte piControl ( int errVal );         // executes one iteration of PI control loop
byte piControlIntOff ( int errVal );   // executes the PI control loop without updating integral term, to avoid wind-up if needed.
int  getPropTerm ( void );             // returns the proportional term.  good for debugging.
int  getIntTerm ( void );              // returns the integral term.  good for debugging.
int  getIntState ( void );             // returns the integrator state.  good for debugging.
};

#endif /* PICONTROLLER_H_ */
