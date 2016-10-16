// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _FanControl_H_
#define _FanControl_H_

#define PRESCALE_FACTOR 1

/*******************************************************************************
 * GLOBALLY INCLUDED HEADER FILES
 ******************************************************************************/
#include "Arduino.h"
#include "LiquidCrystal.h"
#include "EEPROM.h"

/*******************************************************************************
 * GLOBALLY DEFINED MACROS
 ******************************************************************************/
/* General system operation definitions */
#define BAUDRATE    9600  // baudrate used for serial comms
#define LOOPTIME_US 50000 // number of microseconds between each loop iteration

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

/* Temp sensor input definitions */
#define TEMP1PIN 0 // Arduino analog pin used for temp sensor 1
#define TEMP2PIN 1 // Arduino analog pin used for temp sensor 2
#define DigTemp1ToC10( a ) \
  /* Converts from digital value to degrees Celsius times 10 */ \
  /* Returns the result as a short integer */ \
  ( (short) ( ( ( ( (int) a * 10 ) - ( ( (int) Temp1Offset << 10 ) / 500 ) ) * (int) Temp1DegCPer5V ) >> 10 ) )
#define DigTemp2ToC10( a ) \
  /* Converts from digital value to degrees Celsius times 10 */ \
  /* Returns the result as a short integer */ \
  ( (short) ( ( ( ( (int) a * 10 ) - ( ( (int) Temp2Offset << 10 ) / 500 ) ) * (int) Temp2DegCPer5V ) >> 10 ) )
#define DigTemp1ToF10( a ) \
  /* Converts from digital value to degrees Fahrenheit times 10 */ \
  /* Returns the result as a short integer */ \
  ( (short) ( ( ( ( (int) a * 90 ) - ( ( ( (int) Temp1Offset * 9 ) << 10 ) / 500 ) ) * (int) Temp1DegCPer5V / 5 ) >> 10 ) )
#define DigTemp2ToF10( a ) \
  /* Converts from digital value to degrees Fahrenheit times 10 */ \
  /* Returns the result as a short integer */ \
  ( (short) ( ( ( ( (int) a * 90 ) - ( ( ( (int) Temp2Offset * 9 ) << 10 ) / 500 ) ) * (int) Temp2DegCPer5V / 5 ) >> 10 ) )
#define C10ToDigTemp1( a ) \
  /* Converts from signed integer Celsius temperature times to to digital value */ \
  /* Returns the result as an unsigned short integer */ \
  ( (unsigned short) ( ( ( ( ( (int) a ) << 10 ) / (int) Temp1DegCPer5V ) + ( ( (int) Temp1Offset << 10 ) / 500 ) ) / 10 ) )
#define C10ToDigTemp2( a ) \
  /* Converts from signed integer Celsius temperature times to to digital value */ \
  /* Returns the result as an unsigned short integer */ \
  ( (unsigned short) ( ( ( ( ( (int) a ) << 10 ) / (int) Temp2DegCPer5V ) + ( ( (int) Temp2Offset << 10 ) / 500 ) ) / 10 ) )
#define F10ToDigTemp1( a ) \
  /* Converts from signed integer Fahrenheit temperature times to to digital value */ \
  /* Returns the result as an unsigned short integer */ \
  ( (unsigned short) ( ( ( ( ( ( (int) a - 320 ) * 5 / 9 ) << 10 ) / (int) Temp1DegCPer5V ) + ( ( (int) Temp1Offset << 10 ) / 500 ) ) / 10 ) )
#define F10ToDigTemp2( a ) \
  /* Converts from signed integer Fahrenheit temperature times to to digital value */ \
  /* Returns the result as an unsigned short integer */ \
  ( (unsigned short) ( ( ( ( ( ( (int) a - 320 ) * 5 / 9 ) << 10 ) / (int) Temp2DegCPer5V ) + ( ( (int) Temp2Offset << 10 ) / 500 ) ) / 10 ) )
#define C10toC( a ) \
  /* Converts from Celsius times 10 to Celsius */ \
  /* Returns the result as a signed short */ \
  ( (short) ( a / 10 ) )
#define F10toC( a ) \
  /* Converts from Fahrenheit times 10 to Celsius */ \
  /* Returns the result as a signed short */ \
  ( (short) ( ( (int) a - 320 ) * 5 / 90 ) )

/*******************************************************************************
 * GLOBAL FUNCTION DECLARATIONS
 ******************************************************************************/
// no global function declarations yet


/*******************************************************************************
 * GLOBAL VARIABLE DECLARATIONS
 ******************************************************************************/
extern LiquidCrystal  lcd;           // lcd screen class object
extern unsigned long  hall1Period;   // period count for hall sensor 1 (microseconds)
extern unsigned long  hall2Period;   // period count for hall sensor 2 (microseconds)
extern unsigned short Fan1RPM;       // Fan 1 speed, in rpm
extern unsigned short Fan2RPM;       // Fan 2 speed, in rpm
extern byte           Pwm1Duty;      // PWM 1 duty cycle (0-255 maps to 0%-100%)
extern byte           Pwm2Duty;      // PWM 2 duty cycle (0-255 maps to 0%-100%)
extern unsigned short Temp1;         // Temperature 1 input, stored digitally (0-1023)
extern unsigned short Temp2;         // Temperature 2 input, stored digitally (0-1023)
extern unsigned long  lastEdgeTime1; // timestamp of previous edge of hall sensor 1
extern unsigned long  lastEdgeTime2; // timestamp of previous edge of hall sensor 2

/*******************************************************************************
 * EEPROM-STORED GLOBAL VARIABLE DECLARATIONS
 ******************************************************************************/
extern short        Temp1Offset;    // Offset of Temperature 1 measurement, mV at 0 degC
extern short        Temp2Offset;    // Offset of Temperature 2 measurement, mV at 0 degC
extern int          Temp1DegCPer5V; // Scale of temperature 1 measurement, in degrees celcius per 5 Volts
extern int          Temp2DegCPer5V; // Scale of temperature 2 measurement, in degrees celcius per 5 Volts
extern unsigned int minRpm1;        // minimum speed measurement for fan 1
extern unsigned int minRpm2;        // minimum speed measurement for fan 2

#endif /* _FanControl_H_ */
