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

# Description:      1. Prompts user to enter the desired value for each thermal profile parameter and checks user input for validity (within specfied range)
#                   3. Sends the value to the controller and checks to ensure that the controller has received the correct data
# Parameters:       minimum - Minimum acceptable value
#                   maximum - Maximum acceptable value
# Returns:          True if the input is valid and successfully received by controller, False otherwise
def getParameter(minimum, maximum):
    try:
        value = int(input("- "))                                                                        # Read user input
    except(ValueError):                                                                                 # Determine whether input is a number
        print("Error: not a number")
        return False
    if(value < minimum or value > maximum):                                                             # Determine whether input is within acceptable range
        print("Error: outside range")
        return False
    ser.write(str(value).encode())                                                                      # Encode input as binary and send to controller
    time.sleep(0.1)
    if(bytes(ser.readline()) != value)                                                                  # Check controller has received correct data
        return False
    return True

# Description
def data_gen():
    t = data_gen.t
    while True:
        t+=1
        val = bytes(ser.readline())                                                                     # Obtain temperature readings from serial port
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
def onCloseFigure(event):
    sys.exit(0)

# Main code
ser = serial.Serial("COM3", 9600)                                                                       # Configure and open serial port
if(ser.isOpen() == True):
    print("Connection to controller established")
else:
    print("Error: connection to controller failed")
time.sleep(1)

print()
print("Enter soak temperature (" + str(MIN_SOAK_TEMP) + "C - " + str(MAX_SOAK_TEMP) + "C):")            # Display prompt and acceptable range
while(getParameter(MIN_SOAK_TEMP, MAX_SOAK_TEMP) != True):                                              # Obtain and transmit desired soak temperature
    getParameter(MIN_SOAK_TEMP, MAX_SOAK_TEMP)

print()
print("Enter soak time (" + str(MIN_SOAK_TIME) + "s - " + str(MAX_SOAK_TIME) + "s):")                   # Display prompt and acceptable range
while(getParameter(MIN_SOAK_TIME, MAX_SOAK_TIME) != True):                                              # Obtain and transmit desired soak time
    getParameter(MIN_SOAK_TIME, MAX_SOAK_TIME)

print()
print("Enter reflow temperature (" + str(MIN_REFLOW_TEMP) + "C - " + str(MAX_REFLOW_TEMP) + "C):")      # Display prompt and acceptable range
while(getParameter(MIN_REFLOW_TEMP, MAX_REFLOW_TEMP) != True):                                          # Obtain and transmit desired reflow temperature
   getParameter(MIN_REFLOW_TEMP, MAX_REFLOW_TEMP)

print()
print("Enter reflow time (" + str(MIN_REFLOW_TIME) + "s - " + str(MAX_REFLOW_TIME) + "s):")             # Display prompt and acceptable range
while(getParameter(MIN_REFLOW_TIME, MAX_REFLOW_TIME) != True):                                          # Obtain and transmit desired reflow time
    getParameter(MIN_REFLOW_TIME, MAX_REFLOW_TIME)

print()
print("Press 'set' button to begin reflow cycle")
while True:                                                                                             # Wait for start command to be received before ploting graph
    if(ser.readline() == b"Start\n"):
        print()
        print("Starting reflow cycle")
        break

data_gen.t = -1                                                                                         # Plot temperature graph
fig = plt.figure()
fig.canvas.mpl_connect('close_event', onCloseFigure)
ax = fig.add_subplot(111)
line, = ax.plot([], [], lw = 2)
ax.set_ylim(0,255)                                                                                      # Set y-axis scale from 0-255C
ax.set_xlim(0,350)                                                                                      # Set x-axis scale from 0-350s
ax.grid()
xdata, ydata = [], []

ani = animation.FuncAnimation(fig, run, data_gen, blit = False, interval = 100, repeat = False)
plt.show()
