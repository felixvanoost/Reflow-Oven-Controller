Copyright Felix van Oost 2015.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

# Reflow Oven Controller
An Arduino-based project to convert a standard toaster oven into a reflow soldering oven for PCB assembly.

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

For this project, an Arduino will monitor and control an off-the-shelf toaster oven to follow a user-defined thermal profile. The required hardware, consisting of the following main functional blocks, will be mounted on an Arduino Proto Shield:

- A thermocouple to measure the temperature inside the oven
- An amplifier circuit to amplify the voltage across the thermocouple to levels readable by the Arduino
- A temperature sensor to enable the Arduino to perform cold-junction compensation on the thermocouple temperature reading
- Pushbuttons to allow the user to set the desired thermal profile parameters and start / stop the reflow process
- LEDs and a buzzer to provide user feedback on the current stage of reflow and alert the user when the process is complete

A solid-state relay is also required, which enables the Arduino to switch power to the oven on and off (thereby allowing it to control the temperature). For the scope of this project, I will assume that a suitable oven has been purchased and an SSR has already been installed within - there are a number of very good tutorials describing how to do this step-by-step. Information on choosing a suitable toaster oven for reflow soldering can be found [here](http://www.rocketscream.com/blog/2011/06/19/toaster-convection-or-infrared-oven/).

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
- Buzzer (this project uses a TDK PS1420P02CT)
- 4-Position screw terminal block
- 2x 8-DIP sockets
- 3x Pushbuttons
- 2x 5mm LEDs
- 2x 10uF capacitors
- 2x 100Ω resistors
- 2x 330Ω resistors
- 3x 10kΩ resistors
- 2x 47kΩ resistors

The OP07 was chosen for its low input voltage characteristics (as the thermocouple produces an output voltage on the order of microvolts), although any op-amp with similar specifications will work just fine. The same is true for the LMC7660 - any voltage converter providing a +/-5V output from a 5V input can be used.

I am using an [OSEPP Proto Shield](http://osepp.com/products/shield-arduino-compatible/proto-shield/) for this project (functionally identical to the [Sparkfun ProtoShield Kit](https://www.sparkfun.com/products/7914)), which comes pre-installed with a pushbutton and two LEDs. This reduces the number of components that need to be soldered to the board and simplifies the layout and assembly process.

----------
Required Software
----------

- Arduino IDE (latest version preferred)
- Terminal emulation software

Any terminal emulator of your choice should be suitable for this project. I will be using HyperTerminal, which is a far more advanced version of the Arduino serial monitor. The included Arduino serial monitor is designed mainly for debugging purposes and thus lacks many of the features needed to present and format data intuitively for user input / feedback purposes.
