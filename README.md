Copyright Felix van Oost 2015.
This documentation describes Open Hardware and is licensed under the CERN OHL v1.2. You may redistribute and modify this 
documentation under the terms of the [CERN OHL v1.2](http://ohwr.org/cernohl). This documentation is distributed WITHOUT ANY 
EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please 
see the CERN OHL v1.2 for applicable conditions.

# Reflow Oven Controller
An Arduino-based project to convert a standard toaster oven into a reflow soldering oven for SMT assembly

----------
Context
----------

The rapid adoption of surface-mount technology has revolutionised the electronics industry by reducing the size, cost, and power requirements of components. For the hobbyist, however, soldering these ever-smaller components by hand is becoming increasingly difficult. The most consistent and efficient way of soldering SMT components is by reflow soldering, a technique in which the parts are attached to the board using solder paste and the entire board is then heated inside a purpose-built oven. The heat melts the solder paste, thereby permanently soldering the components to the board.

Reflow soldering requires precise control of the oven temperature, so the process is performed commercially in expensive, purpose-built reflow ovens. However, excellent results can be achieved on a budget by using a converted toaster oven and a microcontroller to monitor and adjust the temperature according to a given thermal profile.

![Image of example thermal profile](https://raw.githubusercontent.com/FelixVanOost/BMS-Interface-Board/master/Photos%20%26%20Renderings/BMS%20Interface%20Board%20-%20Thumbnail.JPG)
*Example thermal profile*

The specific thermal profile followed by the microcontroller will depend on the type of solder paste and components used on the board. It is split into five main sections - ramp to soak, soak, ramp to reflow, reflow, and cooling. Each section dictates the specific temperature, time, and ramp up / down characteristics required for optimal soldering results. A full reflow cycle usually takes in the region of 3-6 minutes to complete.
