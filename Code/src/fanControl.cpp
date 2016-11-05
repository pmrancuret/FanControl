/*******************************************************************************
 * INCLUDE HEADERS
 ******************************************************************************/
#include "FanControl.h"

fanCtrlStateMachine stateMachine; // define the state machine

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
  /* Reset state machine, which will initialize the system */
  stateMachine.reset ( );

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
  static unsigned long lastTime = 0;          // time when last loop began (microseconds/64)
  unsigned long        thisTime = micros ( ); // time now (in microseconds/64)

  /* Check to see if it is time to run a new loop, otherwise return */
  if ( thisTime - lastTime >= LOOPTIME_US * 64 ) // enough time elapsed for new loop
    lastTime = thisTime;                         // store loop time for next iteration
  else
    return; // skip this loop iteration

  /* keep track of total program run time */
  runTime_s = loopsRun * LOOPTIME_US / 1000000; // track runtime in seconds
  loopsRun++;                                   // increment count of loops run

  /* Calculate Fan speeds in RPM */
  measFanSpeeds ( thisTime );

  /* Read temperature measurements */
  Temp1 = analogRead ( TEMP1PIN ); // read temp sensor 1
  Temp2 = analogRead ( TEMP2PIN ); // read temp sensor 2

  /* Check for button clicks and update consecutive button press count */
  checkButtonPress ( );

  /* Run state machine */
  stateMachine.run ( );

  /* Set duty cycle for pwm outputs */
  analogWrite ( PWM1PIN, Pwm1Duty ); // set pwm1 duty
  analogWrite ( PWM2PIN, Pwm2Duty ); // set pwm2 duty

  return; // end of loop()
}         // end of loop()
