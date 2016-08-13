#Fan Control

This project contains everything needed to create a Fan Control board.  

This Fan Control board is used to power and control two PC fans, independently.  Works with 3-wire, 12V PC fans.  Speed of each fan is measured using hall-effect sensor feedback.

Arudino mini is used for control logic.  An LCD screen and three pushbuttons are provided to allow the user to provide input settings and view status info.  A switch is provided to turn backlight LED on/off.  Two potentiometers are provided to allow changing brightness and contrast of screen.

Two independent temperature sensors are used for feedback.  User can populate these sensors on-board, or use jumper wires to place them remotely.  User may select whether to use an average or max temperature for determining fan speeds, or control each fan speed independently using respective temperature sensors.  User may also select displayed temperature units (C or F).  User may specify up to 8 point pairs each of the two fan speed vs. temperature lookup tables.

User may also change speed control loop proportional and integral gains, as well as speed and voltage limits.  All settings saved in non-volatile EEPROM.  Easy interface also allows restoring default settings.