Copyright Felix van Oost 2015.
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

# Reflow Oven Controller
A project to convert a standard toaster oven into a reflow soldering oven for PCB assembly using an Arduino and Python.

----------
Context
----------

The rapid adoption of surface-mount technology has revolutionised the electronics industry by reducing the size, cost, and power requirements of components. For the hobbyist, however, soldering these ever-smaller components by hand is becoming increasingly difficult. The most consistent and efficient way of soldering SMT components is by reflow soldering, a technique in which parts are attached to the PCB using solder paste and the entire board is then heated inside a purpose-built oven. The heat melts the solder paste, thereby permanently soldering the components to the board.

Reflow soldering requires precise control of the oven temperature, so the process is performed commercially in expensive, purpose-built reflow ovens. However, excellent results can be achieved on a budget by using a converted toaster oven and a microcontroller to monitor and adjust the temperature according to a specified thermal profile.

![Image of example thermal profile](https://raw.githubusercontent.com/FelixVanOost/Reflow-Oven-Controller/master/References/Example%20Thermal%20Profile.JPG)
*Example thermal profile*

The specific thermal profile followed by the microcontroller will depend on the type of solder paste and components used on the board (temperature guidelines are often provided in component datasheets). It is split into five main stages - ramp to soak, soak, ramp to reflow, reflow, and cooling. Each section dictates the specific temperature, time, and ramp up / down characteristics required for optimal soldering results. A full reflow cycle usually takes in the region of 4-6 minutes to complete.

----------
Overview
----------

This project consists of three main components:

- An off-the-shelf toaster oven with SSR installed.
- An Arduino Uno to monitor and control the oven to follow a user-defined thermal profile.
- A Python script to provide a text-based UI for the Arduino and display a live plot of the oven temperature data on a computer.

To help limit the scope somewhat, I will assume that a suitable oven has been purchased and an SSR has already been installed within - there are a number of very good tutorials describing how to do this step-by-step. Information on choosing a suitable toaster oven for reflow soldering can be found [here](http://www.rocketscream.com/blog/2011/06/19/toaster-convection-or-infrared-oven/).

Four thermal profile parameters are user-configurable:

- Soak temperature
- Soak time
- Reflow temperature
- Reflow time

The Python script prompts the user to enter the desired values for each parameter, checks the input is within a pre-determined valid range, then sends the values to the Arduino via USB. Once the reflow cycle has been started, it receives an oven temperature reading from the Arduino every second and plots this on a graph for the user in real-time. The script also notifies the user when the cycle is complete or has been stopped for some reason.

The Arduino uses a finite state machine to implement the control logic, with a state corresponding to each stage in the reflow cycle. It controls the toaster oven using time-proportional control (a slower form of PWM). User feedback is provided via a set of LEDs and a buzzer.

----------
Hardware
----------

The hardware is split into the following main functional blocks:

- A thermocouple to measure the temperature inside the oven
- An amplifier circuit to amplify the voltage across the thermocouple to a level readable by the Arduino
- A temperature sensor to perform cold-junction compensation on the thermocouple temperature reading
- Pushbuttons to allow the user to start / stop the reflow process and implement any additional functionality
- LEDs and a buzzer to provide user feedback on the current stage in the reflow cycle

The following components are used:

- Arduino Uno or equivalent
- Arduino Proto Shield or equivalent
- K-type thermocouple wire (must be rated for at least 300C)
- Texas Instruments LM35Z Temperature Sensor
- Analog Devices OP07 Op-Amp or equivalent
- Texas Instruments LMC7660 Voltage Converter or equivalent (provides +/-5V bipolar output for the op-amp)
- Buzzer (this project uses a TDK PS1420P02CT)
- 4-Position screw terminal block
- 2x 8-DIP sockets
- 3x Pushbuttons (1 for start / stop, others for future additional functionality)
- 2x 5mm LEDs
- 2x 10uF capacitors
- 2x 100Ω resistors
- 2x 330Ω resistors
- 3x 10kΩ resistors
- 2x 47kΩ resistors

The OP07 was chosen for its low input voltage characteristics (as the thermocouple produces an output voltage on the order of microvolts), although any op-amp with similar specifications will work just fine. The same is true for the LMC7660 - any voltage converter providing a +/-5V output from a 5V input can be used.

I decided to mount the hardware on an [OSEPP Proto Shield](http://osepp.com/products/shield-arduino-compatible/proto-shield/) (functionally identical to the [Sparkfun ProtoShield Kit](https://www.sparkfun.com/products/7914)), which comes pre-installed with a pushbutton and two LEDs. This reduces the number of components that need to be soldered to the board and simplifies the layout and assembly process. A full hardware schematic is provided in the 'Hardware' folder, and a few photos of the completed shield (one of many possible board layouts) can be found in the 'Photos' folder.

----------
Software
----------

The following software is required:

- Arduino IDE (latest version preferred)
- Python 3.4.2 (I opted to use IDLE in [WinPython](http://winpython.github.io/) 3.4.3.3 to run the Python UI script)

Further information about software / firmware setup and a detailed overview of the control logic is provided with each major release in the 'releases' section. Releases are versioned in the form vX.Y.ZZ, where X indicates a major new release, Y indicates a minor feature addition / update, and ZZ denotes a simple bug fix.
