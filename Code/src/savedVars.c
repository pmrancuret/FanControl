/*
 * savedVars.c
 *
 *  Created on: Oct 29, 2016
 *      Author: Paul
 */

/*******************************************************************************
 * INCLUDE HEADERS
 ******************************************************************************/
#include "savedVars.h"
#include <stddef.h>
#include <string.h>
#include <avr/eeprom.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * DEFINE THE SAVED VARIABLES TABLE DEFINITION
 ******************************************************************************/
#define SAVEDVARDEF( a, b, c, d, e, f ) { &a, strcmp (# b, "unsigned" ), sizeof ( b c ), d, e, f },
const SAVED_VAR_TABLE_TYPE savedVarsTbl [] = { SAVEDVARLIST };
#undef SAVEDVARDEF
const size_t               savedVarsTblSize = sizeof ( savedVarsTbl ) / sizeof ( SAVED_VAR_TABLE_TYPE );


/*******************************************************************************
 * EEPROM-STORED GLOBAL VARIABLE DEFAULT DEFINITIONS
 ******************************************************************************/
#define SAVEDVARDEF( a, b, c, d, e, f ) b c a = f;
SAVEDVARLIST
#undef SAVEDVARDEF

/*******************************************************************************
 * FUNCTION DEFINITIONS
 ******************************************************************************/

/******************************************************************************
* Function:
*   checkVarRange()
*
* Description:
*   checks to ensure that the variable pointed to by varPtr falls within the
*   ranges specified in the savedVarTbl, at index tblInd.  If it falls outside
*   range, then a flag will be raised and the value will be adjusted to be
*   within range.
*
* Arguments:
*   varPtr - pointer to variable to check
*   tblInd - index of entry in savedVarTbl[] which tells min/max values to check
*
* Returns:
*   SAVEVAR_SUCCESS - returned value if variable was within range
*   SAVEDVAR_OOR - returned value if variable was outside range
*   INVALID_VAR - returned value if the table index was invalid
******************************************************************************/
static int checkVarRange ( void *varPtr, unsigned int tblInd )
{
  int rtnCode = SAVEVAR_SUCCESS; // start with return value indicating success

  if ( tblInd >= savedVarsTblSize ) // check to make sure index falls within table size
  {
    rtnCode = INVALID_VAR; // set return value indicating invalid variable specified
    return rtnCode;        // exit function
  }

  /* Check to make sure data is within valid ranges */
  if ( savedVarsTbl [ tblInd ].varSigned ) // if this is a signed value
  {
    switch ( savedVarsTbl [ tblInd ].varSize )
    {
    case 1:
      if ( *(int8_t *) varPtr > (int8_t) savedVarsTbl [ tblInd ].varMax )
      {
        rtnCode           |= SAVEDVAR_OOR;                            // set return code to indciate variable is outside valid range
        *(int8_t *) varPtr = (int8_t) savedVarsTbl [ tblInd ].varMax; // set variable to max value
      }
      else if ( *(int8_t *) varPtr < (int8_t) savedVarsTbl [ tblInd ].varMin )
      {
        rtnCode           |= SAVEDVAR_OOR;                            // set return code to indciate variable is outside valid range
        *(int8_t *) varPtr = (int8_t) savedVarsTbl [ tblInd ].varMin; // set variable to min value
      }
      break;

    case 2:
      if ( *(int *) varPtr > (int) savedVarsTbl [ tblInd ].varMax )
      {
        rtnCode        |= SAVEDVAR_OOR;                         // set return code to indciate variable is outside valid range
        *(int *) varPtr = (int) savedVarsTbl [ tblInd ].varMax; // set variable to max value
      }
      else if ( *(int *) varPtr < (int) savedVarsTbl [ tblInd ].varMin )
      {
        rtnCode        |= SAVEDVAR_OOR;                         // set return code to indciate variable is outside valid range
        *(int *) varPtr = (int) savedVarsTbl [ tblInd ].varMin; // set variable to min value
      }
      break;

    case 4:
      if ( *(long *) varPtr > savedVarsTbl [ tblInd ].varMax )
      {
        rtnCode         |= SAVEDVAR_OOR;                   // set return code to indciate variable is outside valid range
        *(long *) varPtr = savedVarsTbl [ tblInd ].varMax; // set variable to max value
      }
      else if ( *(long *) varPtr < savedVarsTbl [ tblInd ].varMin )
      {
        rtnCode         |= SAVEDVAR_OOR;                   // set return code to indciate variable is outside valid range
        *(long *) varPtr = savedVarsTbl [ tblInd ].varMin; // set variable to min value
      }
      break;

    default:
      rtnCode |= INVALID_SIZE; // set return code to indciate variable is too large
    }
  }
  else // if this is an unsigned value
  {
    switch ( savedVarsTbl [ tblInd ].varSize )
    {
    case 1:
      if ( *(uint8_t *) varPtr > (uint8_t) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMax ) )
      {
        rtnCode            |= SAVEDVAR_OOR;                                                     // set return code to indciate variable is outside valid range
        *(uint8_t *) varPtr = (uint8_t) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMax ); // set variable to max value
      }
      else if ( *(uint8_t *) varPtr < (uint8_t) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMin ) )
      {
        rtnCode            |= SAVEDVAR_OOR;                                                     // set return code to indciate variable is outside valid range
        *(uint8_t *) varPtr = (uint8_t) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMin ); // set variable to min value
      }
      break;

    case 2:
      if ( *(unsigned int *) varPtr > (unsigned int) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMax ) )
      {
        rtnCode                 |= SAVEDVAR_OOR;                                                          // set return code to indciate variable is outside valid range
        *(unsigned int *) varPtr = (unsigned int) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMax ); // set variable to max value
      }
      else if ( *(unsigned int *) varPtr < (unsigned int) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMin ) )
      {
        rtnCode                 |= SAVEDVAR_OOR;                                                          // set return code to indciate variable is outside valid range
        *(unsigned int *) varPtr = (unsigned int) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMin ); // set variable to min value
      }
      break;

    case 4:
      if ( *(unsigned long *) varPtr > (unsigned long) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMax ) )
      {
        rtnCode                  |= SAVEDVAR_OOR;                                                           // set return code to indciate variable is outside valid range
        *(unsigned long *) varPtr = (unsigned long) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMax ); // set variable to max value
      }
      else if ( *(unsigned long *) varPtr < (unsigned long) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMin ) )
      {
        rtnCode                  |= SAVEDVAR_OOR;                                                           // set return code to indciate variable is outside valid range
        *(unsigned long *) varPtr = (unsigned long) *(unsigned long *) &( savedVarsTbl [ tblInd ].varMin ); // set variable to min value
      }
      break;

    default:
      rtnCode |= INVALID_SIZE; // set return code to indciate variable is too large
    }
  }

  return rtnCode; // exit function

} // end of checkVarRange();

/******************************************************************************
* Function:
*   loadVar()
*
* Description:
*   checks to see if the variable pointed to exists in the saved variables
*   table, and then loads it from EEPROM if so.
*
* Arguments:
*   varPtr - pointer to variable to check
*
* Returns:
*   SAVEVAR_SUCCESS - returned value if variable was within range
*   SAVEDVAR_OOR - returned value if variable was outside range
*   INVALID_SIZE - returned value if variable size is too large
*   INVALID_ADDR - returned value if variable address is too high for EEPROM
*   INVALID_VAR - returned value if the variable was not found in table
******************************************************************************/
int loadVar ( void *varPtr ) // if the pointer matches one of the items in the table, this loads that value from EEPROM.
{
  int          rtnCode = INVALID_VAR; // value to return upon exit.  start assuming we don't find the table entry
  unsigned int tblCnt;                // loop count variable

  for ( tblCnt = 0; tblCnt < savedVarsTblSize; tblCnt++ ) // look at each table entry
  {
    if ( savedVarsTbl [ tblCnt ].varPtr == varPtr ) // check to see if the pointer matches one from the table
    {
      rtnCode = SAVEVAR_SUCCESS; // set return code to indicate successfully found variable
      if ( savedVarsTbl [ tblCnt ].varSize > MAXVARSIZE )
      {
        rtnCode |= INVALID_SIZE; // set return code to indciate variable is too large
        return rtnCode;          // exit the function
      }
      if ( ( tblCnt * MAXVARSIZE ) + savedVarsTbl [ tblCnt ].varSize > EEPRMAXBYTES )
      {
        rtnCode |= INVALID_ADDR; // set return code to indciate variable has invalid EEPROM address
        return rtnCode;          // exit the function
      }

      /* Read the data */
      eeprom_read_block ( varPtr, (const void *) ( tblCnt * MAXVARSIZE ), savedVarsTbl [ tblCnt ].varSize );

      /* check to ensure the data is within valid range */
      rtnCode |= checkVarRange ( varPtr, tblCnt ); // check variable range

      break; // exit the loop, since we already found the table item and loaded it
    }
  }

  return rtnCode;
} // end of loadVar()


/******************************************************************************
* Function:
*   saveVar()
*
* Description:
*   checks to see if the variable pointed to exists in the saved variables
*   table, and then saves it to EEPROM if so.
*
* Arguments:
*   varPtr - pointer to variable to check
*
* Returns:
*   SAVEVAR_SUCCESS - returned value if variable was within range
*   SAVEDVAR_OOR - returned value if variable was outside range
*   INVALID_SIZE - returned value if variable size is too large
*   INVALID_ADDR - returned value if variable address is too high for EEPROM
*   INVALID_VAR - returned value if the variable was not found in table
******************************************************************************/
int saveVar ( void *varPtr ) // if the pointer matches one of the items in the table, this loads that value from EEPROM.
{
  int          rtnCode = INVALID_VAR; // value to return upon exit.  start assuming we don't find the table entry
  unsigned int tblCnt;                // loop count variable

  for ( tblCnt = 0; tblCnt < savedVarsTblSize; tblCnt++ ) // look at each table entry
  {
    if ( savedVarsTbl [ tblCnt ].varPtr == varPtr ) // check to see if the pointer matches one from the table
    {
      rtnCode = SAVEVAR_SUCCESS; // set return code to indicate successfully found variable
      if ( savedVarsTbl [ tblCnt ].varSize > MAXVARSIZE )
      {
        rtnCode |= INVALID_SIZE; // set return code to indciate variable is too large
        return rtnCode;          // exit the function
      }
      if ( ( tblCnt * MAXVARSIZE ) + savedVarsTbl [ tblCnt ].varSize > EEPRMAXBYTES )
      {
        rtnCode |= INVALID_ADDR; // set return code to indciate variable has invalid EEPROM address
        return rtnCode;          // exit the function
      }

      /* check to ensure the data is within valid range */
      rtnCode |= checkVarRange ( varPtr, tblCnt ); // check variable range

      /* Write the data */
      eeprom_write_block ( varPtr, (const void *) ( tblCnt * MAXVARSIZE ), savedVarsTbl [ tblCnt ].varSize );

      break; // exit the loop, since we already found the table item and loaded it
    }
  }

  return rtnCode;
} // end of saveVar()



/******************************************************************************
* Function:
*   loadAllVars()
*
* Description:
*   loads all variables from EEPROM.  If the 'code version' variable doesn't
*   match the default value, then EEPROM is assumed corrupted or uninitialized,
*   so it is all replaced with default values.
*
* Arguments:
*   none
*
* Returns:
*   SAVEVAR_SUCCESS - returned value if variable was within range
*   SAVEDVAR_OOR - returned value if variable was outside range
*   INVALID_SIZE - returned value if variable size is too large
*   INVALID_ADDR - returned value if variable address is too high for EEPROM
*   INVALID_VAR - returned value if the variable was not found in table
******************************************************************************/
int loadAllVars ( void )
{
  int          rtnCode = SAVEVAR_SUCCESS; // value to return upon exit.  start assuming we don't find the table entry
  unsigned int tblCnt;                    // loop count variable

  rtnCode |= loadVar ( &codeVer ); // read the code version variable

  /* Check code version variable to see if it has changed, and variables need re-loaded */
  for ( tblCnt = 0; tblCnt < savedVarsTblSize; tblCnt++ ) // look at each table entry
  {
    if ( savedVarsTbl [ tblCnt ].varPtr == &codeVer ) // if we are looking at the codeVer entry
    {
      if ( codeVer != *(unsigned long *) &( savedVarsTbl [ tblCnt ].varDef ) ) // if code versions don't match table default
      {
        rtnCode |= saveDefVars ( ); // load and save default values for all variables
        return rtnCode;             // exit function
      }
      break; // exit loop, since we already found codeVer entry
    }
  }

  /* Code version matched, so we can go ahead and load all variables from EEPROM */
  for ( tblCnt = 0; tblCnt < savedVarsTblSize; tblCnt++ ) // look at each table entry
  {
    rtnCode |= loadVar ( savedVarsTbl [ tblCnt ].varPtr ); // load the value from this table entry
  }

  return rtnCode;
} // end of loadAllVars()



/******************************************************************************
* Function:
*   saveDefVars()
*
* Description:
*   saves all default values into EEPROM
*
* Arguments:
*   none
*
* Returns:
*   SAVEVAR_SUCCESS - returned value if variable was within range
*   SAVEDVAR_OOR - returned value if variable was outside range
*   INVALID_SIZE - returned value if variable size is too large
*   INVALID_ADDR - returned value if variable address is too high for EEPROM
*   INVALID_VAR - returned value if the variable was not found in table
******************************************************************************/
int saveDefVars ( void )
{
  int          rtnCode = SAVEVAR_SUCCESS; // value to return upon exit.  start assuming we don't find the table entry
  unsigned int tblCnt;                    // loop count variable

  for ( tblCnt = 0; tblCnt < savedVarsTblSize; tblCnt++ ) // look at each table entry
  {

    if ( savedVarsTbl [ tblCnt ].varSigned ) // if this is a signed value
    {
      switch ( savedVarsTbl [ tblCnt ].varSize )
      {
      case 1:
        *(int8_t *) savedVarsTbl [ tblCnt ].varPtr = (int8_t) savedVarsTbl [ tblCnt ].varDef;    // set default value
        rtnCode                                   |= saveVar ( savedVarsTbl [ tblCnt ].varPtr ); // save value in EEPROM
        break;

      case 2:
        *(int *) savedVarsTbl [ tblCnt ].varPtr = (int) savedVarsTbl [ tblCnt ].varDef;       // set default value
        rtnCode                                |= saveVar ( savedVarsTbl [ tblCnt ].varPtr ); // save value in EEPROM
        break;

      case 4:
        *(long *) savedVarsTbl [ tblCnt ].varPtr = savedVarsTbl [ tblCnt ].varDef;             // set default value
        rtnCode                                 |= saveVar ( savedVarsTbl [ tblCnt ].varPtr ); // save value in EEPROM
        break;

      default:
        rtnCode |= INVALID_SIZE; // set return code to indciate variable is too large
      }
    }
    else // if this is an unsigned value
    {
      switch ( savedVarsTbl [ tblCnt ].varSize )
      {
      case 1:
        *(uint8_t *) savedVarsTbl [ tblCnt ].varPtr = (uint8_t) savedVarsTbl [ tblCnt ].varDef;   // set default value
        rtnCode                                    |= saveVar ( savedVarsTbl [ tblCnt ].varPtr ); // save value in EEPROM
        break;

      case 2:
        *(unsigned int *) savedVarsTbl [ tblCnt ].varPtr = (unsigned int) savedVarsTbl [ tblCnt ].varDef; // set default value
        rtnCode                                         |= saveVar ( savedVarsTbl [ tblCnt ].varPtr );    // save value in EEPROM
        break;

      case 4:
        *(unsigned long *) savedVarsTbl [ tblCnt ].varPtr = (unsigned long) savedVarsTbl [ tblCnt ].varDef; // set default value
        rtnCode                                          |= saveVar ( savedVarsTbl [ tblCnt ].varPtr );     // save value in EEPROM
        break;

      default:
        rtnCode |= INVALID_SIZE; // set return code to indciate variable is too large
      }
    }


  }

  return rtnCode;
} // end of saveDefVars()


#ifdef __cplusplus
}
#endif


