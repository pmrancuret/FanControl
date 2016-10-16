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
#define FAN1PPR  2 // Hall sensor pulses-per-revolution of fan 1
#define FAN2PPR  2 // Hall sensor pulses-per-revolution of fan 2
#define HALL1PIN 2 // Arduino pin used for Hall Sensor 1 input
#define HALL2PIN 3 // Arduino pin used for Hall sensor 2 input

/*******************************************************************************
 * GLOBAL FUNCTION DECLARATIONS
 ******************************************************************************/
// no global function declarations yet


/*******************************************************************************
 * GLOBAL VARIABLE DECLARATIONS
 ******************************************************************************/
extern LiquidCrystal lcd;         // lcd screen class object
extern unsigned long hall1Period; // period count for hall sensor 1 (microseconds)
extern unsigned long hall2Period; // period count for hall sensor 2 (microseconds)
extern unsigned int  Fan1RPM;     // Fan 1 speed, in rpm
extern unsigned int  Fan2RPM;     // Fan 2 speed, in rpm

#endif /* _FanControl_H_ */
