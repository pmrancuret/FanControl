/*
 * FanControlUtils.h
 *
 *  Created on: Nov 4, 2016
 *      Author: Paul
 */

#ifndef FANCONTROLUTILS_H_
#define FANCONTROLUTILS_H_

/*******************************************************************************
 * INCLUDED HEADER FILES
 ******************************************************************************/
#include "Arduino.h"

/*******************************************************************************
 * DEFINITIONS OF CODE VERSION
 ******************************************************************************/
#define CODEVER 0x00000004 // software version code, checked in EEPROM for changes.  Change this value whenever making a new software version to re-load eeprom values.

/*******************************************************************************
 * SYSTEM DEFINITIONS
 ******************************************************************************/
#define BAUDRATE          9600  // baudrate used for serial comms
#define LOOPTIME_US       50000 // number of microseconds between each loop iteration
#define MEANINGLESS_VALUE 0     // this just denotes that the value is meaningless, since it will be over-written at initialization anyway

/*******************************************************************************
 * DEBUGGING DEFINITIONS
 ******************************************************************************/
#define DEBUG_TIMEOUT     2400   // number of loops to remain in a given debug mode
#define DEBUGMSG_DATWORDS 8      // number of data words included in debug message payload
#define DEBUGBUFFSIZE     48     // max buffer size for reading debug message
#define DEBUGHEADSIZE     4      // number of bytes in header of debug message, which tells which debug mode to enter
#define DEBUGPI1_HEAD     "DPI1" // keyword to use in header of debug message telling to enter DEBUG_PI1 mode
#define DEBUGPI2_HEAD     "DPI2" // keyword to use in header of debug message telling to enter DEBUG_PI2 mode
#define DEBUGBTN_HEAD     "DBTN" // keyword to use in header of debug message telling to enter DEBUG_BTNS mode

/*******************************************************************************
 * DEFINITIONS FOR PERIPHERAL USE
 ******************************************************************************/
/* LCD definitions */
#define LCDRSPIN     8  // Arduino pin connected to LCD 'RS' pin
#define LCDRWPIN     7  // Arduino pin connected to LCD 'RW' pin
#define LCDENABLEPIN 9  // Arduino pin connected to LCD 'Enable' pin
#define LCDD0PIN     10 // Arduino pin connected to LCD 'Data 0' pin
#define LCDD1PIN     11 // Arduino pin connected to LCD 'Data 1' pin
#define LCDD2PIN     12 // Arduino pin connected to LCD 'Data 2' pin
#define LCDD3PIN     13 // Arduino pin connected to LCD 'Data 3' pin
#define LCDCOLS      16 // number of columns of LCD screen
#define LCDROWS      2  // number of rows of LCD screen
#define LCD_DEC      4  // decimation of loop iterations to print to lcd (eg 4 means that the LCD screen is updated every fourth loop)

/* PWM output definitions */
#define PWM1PIN 5 // Arduino pin used for PWM 1 output
#define PWM2PIN 6 // Arduino pin used for PWM 2 output

/* Hall sensor input definitions */
#define FAN1PPR  2    // Hall sensor pulses-per-revolution of fan 1
#define FAN2PPR  2    // Hall sensor pulses-per-revolution of fan 2
#define HALL1PIN 2    // Arduino pin used for Hall Sensor 1 input
#define HALL2PIN 3    // Arduino pin used for Hall sensor 2 input
#define MAXN1    9999 // maximum fan 1 speed measurement (rpm)
#define MAXN2    9999 // maximum fan 2 speed measurement (rpm)
#define MINN1    50   // minimum fan 1 speed measurement (rpm)
#define MINN2    50   // minimum fan 2 speed measurement (rpm)

/* Temp sensor input definitions */
#define TEMP1PIN 0 // Arduino analog pin used for temp sensor 1
#define TEMP2PIN 1 // Arduino analog pin used for temp sensor 2

/* Button pin definitions */
#define BTN1PIN 4  // Arduino digital pin used for reading button 1
#define BTN2PIN A2 // Arduino digital pin used for reading button 1
#define BTN3PIN A3 // Arduino digital pin used for reading button 1

/*******************************************************************************
 * USEFUL MACROS FOR TEMPERATURE CONVERSION
 ******************************************************************************/
#define DigTemp1ToC10( a ) \
  /* Converts from digital value to degrees Celsius times 10 */ \
  /* Returns the result as an integer */ \
  ( (int) ( ( ( ( (long int) a * 10 ) - ( ( (long int) Temp1Offset << 10 ) / 500 ) ) * (long int) Temp1DegCPer5V ) >> 10 ) )
#define DigTemp2ToC10( a ) \
  /* Converts from digital value to degrees Celsius times 10 */ \
  /* Returns the result as an integer */ \
  ( (int) ( ( ( ( (long int) a * 10 ) - ( ( (long int) Temp2Offset << 10 ) / 500 ) ) * (long int) Temp2DegCPer5V ) >> 10 ) )
#define DigTemp1ToF10( a ) \
  /* Converts from digital value to degrees Fahrenheit times 10 */ \
  /* Returns the result as an integer */ \
  ( (int) ( ( ( ( ( (long int) a * 90 ) - ( ( ( (long int) Temp1Offset * 9 ) << 10 ) / 500 ) ) * (long int) Temp1DegCPer5V / 5 ) >> 10 ) + 320 ) )
#define DigTemp2ToF10( a ) \
  /* Converts from digital value to degrees Fahrenheit times 10 */ \
  /* Returns the result as an integer */ \
  ( (int) ( ( ( ( ( (long int) a * 90 ) - ( ( ( (long int) Temp2Offset * 9 ) << 10 ) / 500 ) ) * (long int) Temp2DegCPer5V / 5 ) >> 10 ) + 320 ) )
#define C10ToDigTemp1( a ) \
  /* Converts from signed integer Celsius temperature times to to digital value */ \
  /* Returns the result as an unsigned integer */ \
  ( (unsigned int) ( ( ( ( ( (long int) a ) << 10 ) / (long int) Temp1DegCPer5V ) + ( ( (long int) Temp1Offset << 10 ) / 500 ) ) / 10 ) )
#define C10ToDigTemp2( a ) \
  /* Converts from signed integer Celsius temperature times to to digital value */ \
  /* Returns the result as an unsigned integer */ \
  ( (unsigned int) ( ( ( ( ( (long int) a ) << 10 ) / (long int) Temp2DegCPer5V ) + ( ( (long int) Temp2Offset << 10 ) / 500 ) ) / 10 ) )
#define F10ToDigTemp1( a ) \
  /* Converts from signed integer Fahrenheit temperature times to to digital value */ \
  /* Returns the result as an unsigned integer */ \
  ( (unsigned int) ( ( ( ( ( ( (long int) a - 320 ) * 5 / 9 ) << 10 ) / (long int) Temp1DegCPer5V ) + ( ( (long int) Temp1Offset << 10 ) / 500 ) ) / 10 ) )
#define F10ToDigTemp2( a ) \
  /* Converts from signed integer Fahrenheit temperature times to to digital value */ \
  /* Returns the result as an unsigned integer */ \
  ( (unsigned int) ( ( ( ( ( ( (long int) a - 320 ) * 5 / 9 ) << 10 ) / (long int) Temp2DegCPer5V ) + ( ( (long int) Temp2Offset << 10 ) / 500 ) ) / 10 ) )
#define C10toC( a ) \
  /* Converts from Celsius times 10 to Celsius */ \
  /* Returns the result as a signed integer */ \
  ( (int) ( a / 10 ) )
#define F10toC( a ) \
  /* Converts from Fahrenheit times 10 to Celsius */ \
  /* Returns the result as a signed integer */ \
  ( (int) ( ( (long int) a - 320 ) * 5 / 90 ) )

/*******************************************************************************
 * VARIABLE DECLARATIONS
 ******************************************************************************/
extern volatile unsigned long lastEdgeTime1;                       // timestamp of previous edge of hall sensor 1
extern volatile unsigned long lastEdgeTime2;                       // timestamp of previous edge of hall sensor 2
extern volatile unsigned long hall1Period;                         // period count for hall sensor 1 (microseconds)
extern volatile unsigned long hall2Period;                         // period count for hall sensor 2 (microseconds)
extern unsigned int           Fan1RPM;                             // Fan 1 speed, in rpm
extern unsigned int           Fan2RPM;                             // Fan 2 speed, in rpm
extern unsigned int           Fan1RPMRef;                          // Fan 1 reference speed, in rpm
extern unsigned int           Fan2RPMRef;                          // Fan 2 reference speed, in rpm
extern byte                   Pwm1Duty;                            // PWM 1 duty cycle (0-255 maps to 0%-100%)
extern byte                   Pwm2Duty;                            // PWM 2 duty cycle (0-255 maps to 0%-100%)
extern unsigned int           Temp1;                               // Temperature 1 input, stored digitally (0-1023)
extern unsigned int           Temp2;                               // Temperature 2 input, stored digitally (0-1023)
extern unsigned long          loopsRun;                            // total number of loops run since reset
extern unsigned long          runTime_s;                           // program run-time since reset (seconds)
extern byte                   stateChange;                         // high when a state change occurs
extern unsigned int           btn1PressCnt;                        // number of consecutive times button 1 was pressed
extern unsigned int           btn2PressCnt;                        // number of consecutive times button 1 was pressed
extern unsigned int           btn3PressCnt;                        // number of consecutive times button 1 was pressed
extern int                    debugDatWords [ DEBUGMSG_DATWORDS ]; // buffer of data words included in payload of debug messages
extern byte                   lcdLoops;                            // number of loops run since last LCD update

/*******************************************************************************
 * FUNCTION DECLARATIONS
 ******************************************************************************/
void initializeSystem ( void );                // initializes the fan control system
void measFanSpeeds ( unsigned long thisTime ); // Calculates fan speeds, in RPM
void checkButtonPress ( void );                // checks if buttons were pressed, and updates consecutive press count
void setRefFanSpeeds ( void );                 // sets reference fan speeds to track desired temperature
void regFanSpeeds ( void );                    // regulates fan speeds to reference values
void hall1ISR ( void );                        // hall sensor 1 interrupt service routine
void hall2ISR ( void );                        // hall sensor 1 interrupt service routine

#endif /* FANCONTROLUTILS_H_ */
