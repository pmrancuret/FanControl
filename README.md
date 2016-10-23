#Fan Control

*This project is still a work in progress, so the contents are incomplete*

This project contains everything needed to create a Fan Control board.  This board is used to operate two cooling fans to regulate temperature in an enclosed environment.  In our case, it is going to be used to keep a Playstation cool, inside an enclosed cabinet.  The fans are positioned over holes in the back of the cabinet.

Please feel free to contact me with questions, comments, suggestions, or contributions!

## Contents
* [Features and Specifications] (#features)
* [Parts List] (#partslist)
* [Operation Manual] (#operation)
* [Software Load Instructions] (#swload)

# <a name="features"/>Features and Specifications

## Fan Control board is used to power and control two PC fans, independently.

* Works with two standard 3-wire PC fans.
	* Pin 1 is ground.
	* Pin 2 is fan voltage.
	* Pin 3 is hall-effect sensor (for speed feedback).
* May be used with only one fan if desired.
* Continuous current rating of up to 540mA on each fan.
* Combined power rating of two fans is 6 Watts.
	* This power is limited by wall adapter power rating.
	* Using a 12V supply rated at >= 14.2 Watts would allow up to a combined fan power rating of 13 Watts.
* User may specify maximum fan voltage range.
	* Minimum fan voltage >= 3V.
	* Maximum fan voltage <= 12V.
* Speed of each fan is measured independently using hall-effect sensor feedback.
	* Minimum measurable speed of TBD rpm.
	* Maximum measurable speed of TBD rpm.
* Master On/Off switch provided at board input.

## Arduino mini used for control logic.
* An LCD screen and three pushbuttons are provided to allow the user to provide input settings and view status info.
	* Two buttons used as up/down for scrolling through menu items and adjusting values.
	* One button used as an enter key for making selections.
	* Not pressing any button for 10 seconds will return to default display screen.
* A switch is provided to turn backlight LED on/off.  
* Two potentiometers are provided to allow changing brightness and contrast of screen.
* Default display screen shows both fan speeds and both temperature readings in real-time.
* User may select displayed temperature units (C or F).
* User may change proportional and integral gains for each fan speed control loop.
* User may change various speed, voltage, and temperature limits.
* All settings saved in non-volatile EEPROM.
* User may restore all settings to default values.

## Two independent temperature sensors are used for feedback.  
* User can populate these sensors on-board, or use jumper wires to place them remotely.
* User may select whether to use an average or max temperature for determining fan speeds, or control each fan speed independently using respective temperature sensors.
* User may specify up to 8 point pairs each of the two fan speed vs. temperature lookup tables.
	* Min/Max speed values same as min/max measurable speeds listed above.
	* Minimum temperature value of 0 C.
	* Maximum temperature value of 100 C.
	* Sensor accuracy of +/- 2 C.
	* User may specify table settings with resolution of 0.1 C.
	
## Custom designed PCB.
* Dimensions are 3.1" x 3.9".
* Thickness is 0.062".
* Two-layer board, with only through-hole components for easier soldering (depending who you ask).
* Four mounting holes (0.126" drill) provided in corners.
* Refer to [The Eagle Schematic and Board Files] (https://github.com/pmrancuret/FanControl/tree/master/Schematic_and_Board) for board layout and mounting pattern.

## Custom software.
* Refer to (TBD Code Directory) for code.
* Eclipse (TBD version) used to write and compile software.
* Refer to [Software Load Instructions] (#swload) for instructions on building and loading software into the Arduino mini.

# <a name="partslist"/>Parts List

The following parts were used to assemble this board.  Please refer to [The Eagle Schematic and Board Files] (https://github.com/pmrancuret/FanControl/tree/master/Schematic_and_Board) for information about where each component is used.

Prices in table below are as of Aug 13, 2016.  If you are making this, you may choose to get some extras of some components.  The table below only lists the minimum quantity.  Mounting hardware not included.  PC Fans not included.  Tax/shipping not included.  Money could be saved by using a protoboard instead of making the PCB.  But what's the fun in that?

| Description | Manufacturer | Part Number | Quantity | Cost (each) | Cost (total) |
| ----------- | ------------ | ----------- | -------- | ----------- | ------------ |
| Circuit Board | Pentalogix | [Upload Gerbers] (http://www.pentalogix.com) | 1 | $147.90 | $147.90 |
| Wall Adapter | Sparkfun | [TOL-09442] (https://www.sparkfun.com/products/9442) | 1 | $5.95 | $5.95 |
| Arduino Pro Mini 328 | Sparkfun | [DEV-11113] (https://www.sparkfun.com/products/11113) | 1 | $9.95 | $9.95 |
| LCD Screen | Xiamen Ocular | [GDM1602K] (https://www.sparkfun.com/products/709) | 1 | $15.95 | $15.95 |
| Pushbutton | Sparkfun | [COM-09190] (https://www.sparkfun.com/products/9190) | 3 | $0.50 | $1.50 |
| DC Power Jack | CUI Inc | [PJ-202A] (https://www.sparkfun.com/products/119) | 1 | $1.25 | $1.25 |
| Temperature Sensor | Analog Devices | [TMP37] (http://www.digikey.com/product-detail/en/analog-devices-inc/TMP37FT9Z/TMP37FT9Z-ND/1217626) | 2 | $1.54 | $3.08 |
| Female Break-Away Pin Headers | Sparkfun | [PRT-00115] (https://www.sparkfun.com/products/115) | 1 | $1.50 | $1.50 |
| Male Break-Away Pin headers | Sparkfun | [PRT-00116] (https://www.sparkfun.com/products/116) | 1 | $1.50 | $1.50 |
| 10kOhm, 1/6W, 5% Through-Hole Resistor | Sparkfun | [COM-11508] (https://www.sparkfun.com/products/11508) | 7 | NA | $0.95 |
| 5.1Ohm, 1/4W, 5% Through-Hole Resistor | Yageo | [CFR-25JB-52-5R1] (https://www.digikey.com/product-detail/en/yageo/CFR-25JB-52-5R1/5.1QBK-ND/2238) | 6 | $0.10 | $0.60 |
| 1kOhm, 1/6W, 5% Through-Hole Resistor | Yageo | [CFR-12JB-52-1K] (https://www.digikey.com/product-detail/en/yageo/CFR-12JB-52-1K/1.0KEBK-ND/4000) | 2 | $0.10 | $0.20 |
| 100Ohm, 1/2W Potentiometer | Bourns Inc. | [3362P-1-101LF] (https://www.digikey.com/product-detail/en/bourns-inc/3362P-1-101LF/3362P-101LF-ND/1088410) | 1 | $1.02 | $1.02 |
| 10kOhm, 1/2W Potentiometer | Bourns Inc. | [3362P-1-103LF] (https://www.digikey.com/product-detail/en/bourns-inc/3362P-1-103LF/3362P-103LF-ND/1088412) | 1 | $1.02 | $1.02 |
| 10uF, 25V Through-Hole Ceramic Capacitor | TDK Corporation | [FK16X7R1E106K] (https://www.digikey.com/product-detail/en/tdk-corporation/FK16X7R1E106K/445-8351-ND/2815281) | 7 | $0.63 | $4.41 |
| 1.5mH, 540mA Through-Hole Inductor | Abracon LLC | [AIUR-06-152K] (https://www.digikey.com/product-detail/en/abracon-llc/AIUR-06-152K/AIUR-06-152K-ND/2343620) | 2 | $0.99 | $1.98 |
| P-Channel MOSFET (30V, 650mA) | Microchip Technology | [VP3203N3-G] (https://www.digikey.com/product-detail/en/microchip-technology/VP3203N3-G/VP3203N3-G-ND/4902418) | 2 | $1.37 | $2.74 |
| NPN Transistor (40V, 200mA) | Fairchild Semiconductor | [2N3904BU] (https://www.digikey.com/product-detail/en/fairchild-semiconductor/2N3904BU/2N3904FS-ND/1413) | 2 | $0.18 | $0.36 |
| Shottky Diode (1A) | Fairchild Semiconductor | [1N5817] (https://www.digikey.com/product-detail/en/fairchild-semiconductor/1N5817/1N5817FSCT-ND/1532776) | 2 | $0.43 | $0.86 |
| 5V output DC-DC Converter (2.5W) | Recom Power | [R-78E5.0-0.5] (https://www.digikey.com/product-detail/en/recom-power/R-78E5.0-0.5/945-1648-5-ND/2834904) | 1 | $2.84 | $2.84 |
| Slide Switch | CW Industries | [GF-123-0054] (https://www.digikey.com/product-detail/en/cw-industries/GF-123-0054/CWI333-ND/4089770) | 2 | $0.93 | $1.86 |
| 3-Pin fan connector | TE Connectivity | [640456-3] (https://www.digikey.com/product-detail/en/te-connectivity-amp-connectors/640456-3/A19470-ND/259010) | 2 | $0.13 | $0.26 |
| Total Cost  | | | | | $207.68 |


# <a name="operation"/>Operation Manual

*work in progress*

# <a name="swload"/>Software Load Instructions

*work in progress*