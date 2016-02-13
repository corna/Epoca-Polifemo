EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:LM3914
LIBS:polifemo-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 3
Title "Polifemo"
Date "2016-04-01"
Rev "1.1"
Comp "Nicola Corna - Polifactory"
Comment1 "Copyright 2016 Nicola Corna <nicola@corna.info>"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 5850 3000 1050 1000
U 56B5E5F0
F0 "Handle" 60
F1 "handle.sch" 60
F2 "BAR0_SIGNAL" I L 5850 3200 60 
F3 "BAR1_SIGNAL" I L 5850 3350 60 
F4 "+3.3V" I L 5850 3650 60 
F5 "GND" I L 5850 3800 60 
$EndSheet
$Sheet
S 3900 3000 1050 1000
U 56B62316
F0 "Blade" 60
F1 "blade.sch" 60
F2 "+3.3V" O R 4950 3650 60 
F3 "GND" O R 4950 3800 60 
F4 "BAR0_SIGNAL" O R 4950 3200 60 
F5 "BAR1_SIGNAL" O R 4950 3350 60 
$EndSheet
Wire Wire Line
	5850 3800 4950 3800
Wire Wire Line
	4950 3650 5850 3650
Wire Wire Line
	5850 3350 4950 3350
Wire Wire Line
	4950 3200 5850 3200
Text Notes 7000 6450 0    60   ~ 0
Copyright 2016 Nicola Corna <nicola@corna.info>\n\nThis documentation describes Open Hardware and is licensed under the\nCERN OHL v. 1.2.\n\nYou may redistribute and modify this documentation under the terms of the\nCERN OHL v.1.2. (http://ohwr.org/cernohl). This documentation is distributed\nWITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF\nMERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A\nPARTICULAR PURPOSE. Please see the CERN OHL v.1.2 for applicable\nconditions\n
Text Notes 7000 5350 0    60   ~ 0
Licensed under CERN OHL v.1.2
$EndSCHEMATC
