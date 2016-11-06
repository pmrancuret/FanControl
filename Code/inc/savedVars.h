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
#include "fanControlUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * MACRO DEFINITIONS
 ******************************************************************************/
#define MAXVARSIZE      4    // maximum variable size allowed (bytes)
#define EEPRMAXBYTES    1024 // maximum bytes stored in EEPROM
#define SAVEVAR_SUCCESS 0    // value returned from save/load function success
#define INVALID_VAR     1    // value returned when an invalid variable is specified
#define INVALID_SIZE    2    // value returned when a variable is too big!  if this happens, increase MAXVARSIZE to allow it
#define INVALID_ADDR    4    // value returned when a variable address outside the valid EEPROM address range
#define SAVEDVAR_OOR    8    // value returned when a variabe to read/write was out of range. Clamps to range maximum if this happens.

/*******************************************************************************
 * MACRO USED FOR DEFINING SAVED VARIABLE TABLE
 ******************************************************************************/
#define SAVEDVARLIST \
  /* DO NOT CHANGE THE codeVer ENTRY OF THE TABLE BELOW */ \
  /*           varName,         sign,     type, min,        max,        default */ \
  SAVEDVARDEF ( codeVer,        unsigned, long, 0x00000000, 0xFFFFFFFF, CODEVER )    /* Code Version */ \
  SAVEDVARDEF ( Temp1Offset,    signed,   int,  -5000,      5000,       0 )          /* Offset in temperature 1 measurement, mV reading at 0 degC */ \
  SAVEDVARDEF ( Temp2Offset,    signed,   int,  -5000,      5000,       0 )          /* Offset in temperature 2 measurement, mV reading at 0 degC */ \
  SAVEDVARDEF ( Temp1DegCPer5V, signed,   int,  -5000,      5000,       250 )        /* Scale of temperature 1 measurement, degC per 5 V */ \
  SAVEDVARDEF ( Temp2DegCPer5V, signed,   int,  -5000,      5000,       250 )        /* Scale of temperature 2 measurement, degC per 5 V */ \
  SAVEDVARDEF ( minRpm1,        unsigned, int,  MINN1,      MAXN1,      650 )        /* minimum fan 1 speed setpoint, rpm */ \
  SAVEDVARDEF ( minRpm2,        unsigned, int,  MINN2,      MAXN2,      650 )        /* minimum fan 2 speed setpoint, rpm */ \
  SAVEDVARDEF ( maxRpm1,        unsigned, int,  MINN1,      MAXN1,      1100 )       /* minimum fan 1 speed setpoint, rpm */ \
  SAVEDVARDEF ( maxRpm2,        unsigned, int,  MINN2,      MAXN2,      1100 )       /* minimum fan 2 speed setpoint, rpm */ \
  SAVEDVARDEF ( useFtemp,       unsigned, int,  0,          1,          0 )          /* When high, temps are displayed in degF instead of degC */ \
  SAVEDVARDEF ( pi1Kp,          signed,   int,  0,          32767,      5000 )       /* PI controller 1 proportional gain */ \
  SAVEDVARDEF ( pi1Ki,          signed,   int,  0,          32767,      5000 )       /* PI controller 1 integral gain */ \
  SAVEDVARDEF ( pi1Imax,        signed,   int,  0,          32767,      30000 )      /* PI controller 1 integrator max limit */ \
  SAVEDVARDEF ( pi1Imin,        signed,   int,  -32768,     0,          -30000 )     /* PI controller 1 integrator min limit */ \
  SAVEDVARDEF ( pi2Kp,          signed,   int,  0,          32767,      5000 )       /* PI controller 2 proportional gain */ \
  SAVEDVARDEF ( pi2Ki,          signed,   int,  0,          32767,      5000 )       /* PI controller 2 integral gain */ \
  SAVEDVARDEF ( pi2Imax,        signed,   int,  0,          32767,      30000 )      /* PI controller 2 integrator max limit */ \
  SAVEDVARDEF ( pi2Imin,        signed,   int,  -32768,     0,          -30000 )     /* PI controller 2 integrator min limit */ \
  SAVEDVARDEF ( fan1Filt,       unsigned, int,  0,          1023,       768 )        /* fan 1 speed filter gain, between 0 and 1023.  Larger value is slower filter response. */ \
  SAVEDVARDEF ( fan2Filt,       unsigned, int,  0,          1023,       768 )        /* fan 2 speed filter gain, between 0 and 1023.  Larger value is slower filter response. */ \
  SAVEDVARDEF ( tmpsrc1,        unsigned, int,  0,          3,          TMPSRC_DEF ) /* Source of temp feedback for fan 1.  0=TMP1, 1=TMP2, 2=MAX, 3=MEAN. */ \
  SAVEDVARDEF ( fan1TurnOffTmp, unsigned, int,  0,          1023,       109 )        /* Raw temperature value at which fan 1 turns off. */ \
  SAVEDVARDEF ( fan1TurnOnTmp,  unsigned, int,  0,          1023,       132 )        /* Raw temperature value at which fan 1 turns on (goes to minRpm1). */ \
  SAVEDVARDEF ( fan1TblTmp1,    unsigned, int,  0,          1023,       155 )        /* Raw temperature value of first point in fan1 lookup table. */ \
  SAVEDVARDEF ( fan1TblTmp2,    unsigned, int,  0,          1023,       189 )        /* Raw temperature value of second point in fan1 lookup table. */ \
  SAVEDVARDEF ( fan1TblTmp3,    unsigned, int,  0,          1023,       223 )        /* Raw temperature value of third point in fan1 lookup table. */ \
  SAVEDVARDEF ( fan1TblTmp4,    unsigned, int,  0,          1023,       246 )        /* Raw temperature value of fourth point in fan1 lookup table. */ \
  SAVEDVARDEF ( fan1TblSpd1,    unsigned, int,  MINN1,      MAXN1,      660 )        /* Speed value at first point in fan1 lookup table. */ \
  SAVEDVARDEF ( fan1TblSpd2,    unsigned, int,  MINN1,      MAXN1,      750 )        /* Speed value at second point in fan1 lookup table. */ \
  SAVEDVARDEF ( fan1TblSpd3,    unsigned, int,  MINN1,      MAXN1,      1100 )       /* Speed value at third point in fan1 lookup table. */ \
  SAVEDVARDEF ( fan1TblSpd4,    unsigned, int,  MINN1,      MAXN1,      1100 )       /* Speed value at fourth point in fan1 lookup table. */ \
  SAVEDVARDEF ( tmpsrc2,        unsigned, int,  0,          3,          TMPSRC_DEF ) /* Source of temp feedback for fan 2.  0=TMP1, 1=TMP2, 2=MAX, 3=MEAN. */ \
  SAVEDVARDEF ( fan2TurnOffTmp, unsigned, int,  0,          1023,       109 )        /* Raw temperature value at which fan 2 turns off. */ \
  SAVEDVARDEF ( fan2TurnOnTmp,  unsigned, int,  0,          1023,       132 )        /* Raw temperature value at which fan 2 turns on (goes to minRpm2). */ \
  SAVEDVARDEF ( fan2TblTmp1,    unsigned, int,  0,          1023,       155 )        /* Raw temperature value of first point in fan2 lookup table. */ \
  SAVEDVARDEF ( fan2TblTmp2,    unsigned, int,  0,          1023,       189 )        /* Raw temperature value of second point in fan2 lookup table. */ \
  SAVEDVARDEF ( fan2TblTmp3,    unsigned, int,  0,          1023,       223 )        /* Raw temperature value of third point in fan2 lookup table. */ \
  SAVEDVARDEF ( fan2TblTmp4,    unsigned, int,  0,          1023,       246 )        /* Raw temperature value of fourth point in fan2 lookup table. */ \
  SAVEDVARDEF ( fan2TblSpd1,    unsigned, int,  MINN1,      MAXN1,      660 )        /* Speed value at first point in fan2 lookup table. */ \
  SAVEDVARDEF ( fan2TblSpd2,    unsigned, int,  MINN1,      MAXN1,      750 )        /* Speed value at second point in fan2 lookup table. */ \
  SAVEDVARDEF ( fan2TblSpd3,    unsigned, int,  MINN1,      MAXN1,      1100 )       /* Speed value at third point in fan2 lookup table. */ \
  SAVEDVARDEF ( fan2TblSpd4,    unsigned, int,  MINN1,      MAXN1,      1100 )       /* Speed value at fourth point in fan2 lookup table. */


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
