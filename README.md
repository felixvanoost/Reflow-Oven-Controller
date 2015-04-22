Copyright Felix van Oost 2015.
This documentation describes Open Hardware and is licensed under the CERN OHL v1.2. You may redistribute and modify this 
documentation under the terms of the [CERN OHL v1.2](http://ohwr.org/cernohl). This documentation is distributed WITHOUT ANY 
EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please 
see the CERN OHL v1.2 for applicable conditions.

# Reflow Oven Controller
An Arduino shield to convert a standard toaster oven into a reflow soldering oven for SMT assembly

----------
Context
----------

The rapid adoption of surface-mount technology has revolutionised the electronics industry by reducing the size, cost, and power requirements of components. For the hobbyist, however, soldering these ever-smaller components by hand is becoming increasingly difficult. The most consistent and efficient way of soldering SMT components is by reflow soldering, a technique in which the parts are attached to the board using solder paste and the entire board is then heated inside a purpose-built oven. The heat melts the solder paste, thereby permanently soldering the components to the board.

Reflow soldering requires precise control of the oven temperature, so the process is performed commercially in expensive, purpose-built reflow ovens. However, excellent results can be achieved on a budget by using a converted toaster oven and a microcontroller to monitor and adjust the temperature according to a given thermal profile.

![Image of example thermal profile](https://raw.githubusercontent.com/FelixVanOost/Reflow-Oven-Controller/master/References/Example%20Thermal%20Profile.JPG)
*Example thermal profile*

The specific thermal profile followed by the microcontroller will depend on the type of solder paste and components used on the board. It is split into five main stages - ramp to soak, soak, ramp to reflow, reflow, and cooling. Each section dictates the specific temperature, time, and ramp up / down characteristics required for optimal soldering results. A full reflow cycle usually takes in the region of 3-6 minutes to complete.

----------
Overview
----------

For this project, an Arduino will monitor and control the toaster oven to follow a user-defined thermal profile. The required hardware, consisting of the following main functional blocks, will be mounted on an Arduino Proto Shield:

- A thermocouple to measure the temperature inside the oven
- An amplifier circuit to amplify the voltage across the thermocouple to levels readable by the Arduino
- A temperature sensor to enable the Arduino to perform cold-junction compensation on the thermocouple temperature reading
- Pushbuttons to allow the user to set the desired thermal profile parameters and start / stop the reflow process
- LEDs and a buzzer to provide user feedback on the current stage of reflow and alert the user when the process is complete

One crucial conponent not mentioned above is a solid-state relay, which enables the Arduino to switch power to the oven on and off (thereby allowing it to control the temperature). For the scope of this project, I will assume that an SSR has already been installed in the oven - there are a number of very good tutorials online describing how to do this step-by-step.

The software for this project will be kept simple to make future improvements and modifications for different hardware setups straightforward. A finite state machine will be implemented in C, with a state corresponding to each stage in the reflow thermal profile. To control temperature, the code will simply decide whether to turn the oven on and off in order to reach and maintain the desired temperature at each stage of the reflow process (PID control may be integrated in the future).

----------
Required Hardware
----------

- Arduino Uno or equivalent
- Arduino Proto Shield or equivalent
- K-type thermocouple wire (must be rated for at least 300C)
- Texas Instruments LM35Z Temperature Sensor
- Analog Devices OP07 Op-Amp or equivalent
- Texas Instruments LMC7660 Voltage Converter or equivalent (provides +/-5V output to power the op-amp)

The LM35Z is one of the easiest temperature sensors to integrate in both hardware and software, which helps to keep this project simple. The OP07 was chosen for its low input voltage characteristics (as the thermocouple produces an output voltage on the order of microvolts), although any op-amp with similar specifications will work just fine. The same is true for the LMC7660 - any voltage converter providing a +/-5V output from a 5V input can be used.

----------
Required Software
----------

- Arduino IDE (latest version preferred)
- Windows HyperTerminal

HyperTerminal is a far more advanced version of the Arduino serial monitor, making it useful for providing user feedback when setting the thermal parameters. The Arduino serial monitor is designed mainly for debugging purposes and thus lacks many of the features needed to present and format data intuitively for user input / feedback purposes.
