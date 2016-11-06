/*
 * fanCtrlStateMachine.h
 *
 *  Created on: Nov 4, 2016
 *      Author: Paul
 */

#ifndef FANCTRLSTATEMACHINE_H_
#define FANCTRLSTATEMACHINE_H_

typedef enum FANCTRLSTATE_ENUM
{
  INIT,       // initialization
  NORMAL,     // normal fan control operation state
  DEBUG_PI1,  // debug mode for PI controller #1
  DEBUG_PI2,  // debug mode for PI controller #2
  DEBUG_BTNS, // debug mode for buttons
  DEBUG_TMP,  // debug mode for temp sensors
  DEBUG_FON,  // debug mode for determining when fans turn on and off

} FANCTRLSTATE_ENUM_TYPE;

/*******************************************************************************
 * CLASS DECLARATIONS
 ******************************************************************************/

/*
 * Class:		fanCtrlStateMachine
 * Function:	NA
 * Scope:		global
 * Arguments:	NA
 * Description:	This class contains the state machine for the Fan Control software
 */
class fanCtrlStateMachine
{
private:
FANCTRLSTATE_ENUM_TYPE state; // fan control state

public:
fanCtrlStateMachine ( void );             // constructor for fanCtrlStateMachine
FANCTRLSTATE_ENUM_TYPE getState ( void ); // returns the state machine state
void                   reset ( void );    // resets state machine to run initialization
void                   run ( void );      // runs the state machine

};



#endif /* FANCTRLSTATEMACHINE_H_ */
