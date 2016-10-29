/*
 * savedVars.h
 *
 *  Created on: Oct 29, 2016
 *      Author: Paul
 */

#ifndef SAVEDVARS_H_
#define SAVEDVARS_H_

/*******************************************************************************
 * INCLUDED HEADER FILES
 ******************************************************************************/
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * MACRO DEFINITIONS
 ******************************************************************************/
#define CODEVER         0x00000003 // software version code, checked in EEPROM for changes.  Change this value whenever making a new software version to re-load eeprom values.
#define MAXN1           9999       // maximum fan 1 speed measurement (rpm)
#define MAXN2           9999       // maximum fan 2 speed measurement (rpm)
#define MINN1           50         // minimum fan 1 speed measurement (rpm)
#define MINN2           50         // minimum fan 2 speed measurement (rpm)
#define MAXVARSIZE      4          // maximum variable size allowed (bytes)
#define EEPRMAXBYTES    1024       // maximum bytes stored in EEPROM
#define SAVEVAR_SUCCESS 0          // value returned from save/load function success
#define INVALID_VAR     1          // value returned when an invalid variable is specified
#define INVALID_SIZE    2          // value returned when a variable is too big!  if this happens, increase MAXVARSIZE to allow it
#define INVALID_ADDR    4          // value returned when a variable address outside the valid EEPROM address range
#define SAVEDVAR_OOR    8          // value returned when a variabe to read/write was out of range. Clamps to range maximum if this happens.

/*******************************************************************************
 * MACRO USED FOR DEFINING SAVED VARIABLE TABLE
 ******************************************************************************/
#define SAVEDVARLIST \
  /* DO NOT CHANGE THE codeVer ENTRY OF THE TABLE BELOW */ \
  /*           varName,         sign,     type, min,        max,        default */ \
  SAVEDVARDEF ( codeVer,        unsigned, long, 0x00000000, 0xFFFFFFFF, CODEVER ) /* Code Version */\
  SAVEDVARDEF ( Temp1Offset,    signed,   int,  -5000,      5000,       0 ) /* Offset in temperature 1 measurement, mV reading at 0 degC */\
  SAVEDVARDEF ( Temp2Offset,    signed,   int,  -5000,      5000,       0 ) /* Offset in temperature 2 measurement, mV reading at 0 degC */\
  SAVEDVARDEF ( Temp1DegCPer5V, signed,   int,  -5000,      5000,       250 ) /* Scale of temperature 1 measurement, degC per 5 V */\
  SAVEDVARDEF ( Temp2DegCPer5V, signed,   int,  -5000,      5000,       250 ) /* Scale of temperature 2 measurement, degC per 5 V */\
  SAVEDVARDEF ( minRpm1,        unsigned, int,  MINN1,      MAXN1,      300 ) /* minimum fan 1 speed setpoint, rpm */\
  SAVEDVARDEF ( minRpm2,        unsigned, int,  MINN2,      MAXN2,      300 ) /* minimum fan 2 speed setpoint, rpm */\
  SAVEDVARDEF ( useFtemp,       unsigned, int,  0,          1,          0 ) /* When high, temps are displayed in degF instead of degC */

/*******************************************************************************
 * TYPE DEFINITION FOR TABLE OF SAVED VARIABLE INFO
 ******************************************************************************/
typedef struct SAVED_VAR_TABLE {
  void *const varPtr;    // pointer to RAM variable location
  int         varSigned; // high if variable is signed, 0 otherwise
  size_t      varSize;   // number of bytes of this variable
  long        varMin;    // minimum value
  long        varMax;    // maximum value
  long        varDef;    // default value
} SAVED_VAR_TABLE_TYPE;

/*******************************************************************************
 * VARIABLE DECLARATIONS
 ******************************************************************************/
extern const SAVED_VAR_TABLE_TYPE savedVarsTbl []; // table of saved variables
extern const size_t               savedVarsTblSize;

/*******************************************************************************
 * EEPROM-STORED GLOBAL VARIABLE DECLARATIONS
 ******************************************************************************/
#define SAVEDVARDEF( a, b, c, d, e, f ) extern b c a;
SAVEDVARLIST
#undef SAVEDVARDEF

/*******************************************************************************
 * FUNCTION DECLARATIONS
 ******************************************************************************/
int loadVar ( void *varPtr ); // if the pointer matches one of the items in the table, this loads that value from EEPROM.
int saveVar ( void *varPtr ); // if the pointer matches one of the items in the table, this saves that value to EEPROM.
int loadAllVars ( void );     // loads all saved variables from EEPROM.  If code version doesn't match default, then all defaults are loaded and saved.
int saveDefVars ( void );     // saves default values for all variables in EEPROM.

#ifdef __cplusplus
}
#endif

#endif /* SAVEDVARS_H_ */
