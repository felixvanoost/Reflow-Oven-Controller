Copyright Felix van Oost 2015.
This documentation describes Open Hardware and is licensed under the CERN OHL v1.2. You may redistribute and modify this 
documentation under the terms of the [CERN OHL v1.2](http://ohwr.org/cernohl). This documentation is distributed WITHOUT ANY 
EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please 
see the CERN OHL v1.2 for applicable conditions.

# Reflow Oven Controller
An Arduino shield to convert a standard toaster oven into a reflow soldering oven for SMT assembly

----------
v1.0
----------

v1.0 of the Reflow Oven Controller has the following features:

- Temperature measurement range of 0-255C (readings taken and transmitted to the serial monitor every second)
- Thermocouple error detection (stops reflow process if oven does not reach a specified tempearture within the first 30 seconds)
- Programmable soak temperature, soak time, reflow temperature, and reflow time parameters (within defined minimum and maximum bounds to prevent damage to the PCB inside the oven)
- Simple UI in HyperTerminal (for displaying information and providing simple user feedback)
- 3 Pushbuttons for user input (set, increment, and decrement)
- 4 LEDs for visual user feedback (each LED turns on at a particular stage in the reflow process)
- A buzzer for auditory user feedback (beeps when thermal profile parameters are being set, at the start / end of the reflow process, and at every stage during the process)
