# Copyright Felix van Oost 2015.
# This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
# See the GNU General Public License for more details.

# Reflow Oven Controller for the Arduino Uno

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys, time, math, serial

MIN_SOAK_TEMP = 120                                                                                     # Minimum values for thermal profile parameters
MIN_SOAK_TIME = 30
MIN_REFLOW_TEMP = 200
MIN_REFLOW_TIME = 15

MAX_SOAK_TEMP = 180                                                                                     # Maximum values for thermal profile parameters
MAX_SOAK_TIME = 90
MAX_REFLOW_TEMP = 240
MAX_REFLOW_TIME = 60

# Description:      Prompts user to enter a desired soak temperature, checks for validity, and sends the input to the controller if valid
# Parameters:       -
# Returns:          True if the input is valid and successfully sent, False otherwise
def getSoakTemp():
    print()
    print("Enter soak temperature (" + str(MIN_SOAK_TEMP) + "C - " + str(MAX_SOAK_TEMP) + "C):")        # Display prompt and acceptable parameter range
    try:
        value = int(input("- "))                                                                        # Read user input
    except(ValueError):                                                                                 # Determine whether input is a number
        print("Error: not a number")
        return False
    if(value < MIN_SOAK_TEMP or value > MAX_SOAK_TEMP):                                                 # Determine whether input is outside range
        print("Error: outside range")
        return False
    value = str.encode(str(value))                                                                      # Encode input as binary for Arduino
    ser.write(value)
    return True

# Description:      Prompts user to enter a desired soak time, checks for validity, and sends the input to the controller if valid
# Parameters:       -
# Returns:          True if the input is valid and successfully sent, False otherwise
def getSoakTime():
    print()
    print("Enter soak time (" + str(MIN_SOAK_TIME) + "s - " + str(MAX_SOAK_TIME) + "s):")               # Display prompt and acceptable parameter range
    try:
        value = int(input("- "))                                                                        # Read user input
    except(ValueError):                                                                                 # Determine whether input is a number
        print("Error: not a number")
        return False
    if(value < MIN_SOAK_TIME or value > MAX_SOAK_TIME):                                                 # Determine whether input is outside range
        print("Error: outside range")
        return False
    value = str.encode(str(value))                                                                      # Encode input as binary for Arduino
    ser.write(value)
    return True

# Description:      Prompts user to enter a desired reflow temperature, checks for validity, and sends the input to the controller if valid
# Parameters:       -
# Returns:          True if the input is valid and successfully sent, False otherwise
def getReflowTemp():
    print()
    print("Enter reflow temperature (" + str(MIN_REFLOW_TEMP) + "C - " + str(MAX_REFLOW_TEMP) + "C):")  # Display prompt and acceptable parameter range
    try:
        value = int(input("- "))                                                                        # Read user input
    except(ValueError):                                                                                 # Determine whether input is a number
        print("Error: not a number")
        return False
    if(value < MIN_REFLOW_TEMP or value > MAX_REFLOW_TEMP):                                             # Determine whether input is outside range
        print("Error: outside range")
        return False
    value = str.encode(str(value))                                                                      # Encode input as binary for Arduino
    ser.write(value)
    return True

# Description:      Prompts user to enter a desired reflow time, checks for validity, and sends the input to the controller if valid
# Parameters:       -
# Returns:          True if the input is valid and successfully sent, False otherwise
def getReflowTime():
    print()
    print("Enter reflow time (" + str(MIN_REFLOW_TIME) + "s - " + str(MAX_REFLOW_TIME) + "s):")         # Display prompt and acceptable parameter range
    try:
        value = int(input("- "))                                                                        # Read user input
    except(ValueError):                                                                                 # Determine whether input is a number
        print("Error: not a number")
        return False
    if(value < MIN_REFLOW_TIME or value > MAX_REFLOW_TIME):                                             # Determine whether input is outside range
        print("Error: outside range")
        return False
    value = str.encode(str(value))                                                                      # Encode input as binary for Arduino
    ser.write(value)
    return True

# Description
def data_gen():
    t = data_gen.t
    while True:
        t+=1
        val = ser.readline()                                                                            # Obtain temperature readings from serial port
        yield t, val

# Description
def run(data):
    t, y = data                                                                                         # Plot data
    if t > -1:
        xdata.append(t)
        ydata.append(y)
        line.set_data(xdata, ydata)
    return line,

# Description
def on_close_figure(event):
    sys.exit(0)

# Main code
ser = serial.Serial("COM3", 9600)                                                                       # Configure and open serial port
if(ser.isOpen() == True):
    print("Connection to controller established")

while(getSoakTemp() != True):                                                                           # Obtain thermal profile parameters
    getSoakTemp()
while(getSoakTime() != True):
    getSoakTime()
while(getReflowTemp() != True):
    getReflowTemp()
while(getReflowTime() != True):
    getReflowTime()

print()
print("Press 'set' button to begin reflow cycle")
while True:                                                                                             # Wait for start command to be received before showing plot
    if(ser.readline() == 'START'):
        break

data_gen.t = -1
fig = plt.figure()
fig.canvas.mpl_connect('close_event', on_close_figure)
ax = fig.add_subplot(111)
line, = ax.plot([], [], lw = 2)
ax.set_ylim(0,255)                                                                                      # Set y-axis scale from 0-255C
ax.set_xlim(0,350)                                                                                      # Set x-axis scale from 0-350s
ax.grid()
xdata, ydata = [], []

ani = animation.FuncAnimation(fig, run, data_gen, blit = False, interval = 100, repeat = False)
plt.show()
