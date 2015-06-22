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

# Description:      1. Prompts user to enter the desired value for a given thermal profile parameter and checks user input for validity (within specfied range)
#                   2. Sends the value to the controller and checks to ensure that the controller has received the correct data
# Parameters:       minimum - Minimum acceptable value
#                   maximum - Maximum acceptable value
# Returns:          The reflow parameter value if valid and successfully sent, False otherwise
def getParameter(minimum, maximum):
    try:
        value = int(input("- "))                                                                        # Read user input
    except(ValueError):                                                                                 # Determine whether input is a number
        print("Error: not a number")
        return False
    if(value < minimum or value > maximum):                                                             # Determine whether input is within acceptable range
        print("Error: outside range")
        return False
    ser.write(str(value).encode())                                                                      # Encode input as binary data and send to controller
    if(int(ser.readline()) != value):                                                                   # Check controller has received correct data
        print("Error: data not successfully received by controller")
        return False
    
    return value

# Description:      Generates data for the temperature graph by obtaining readings from the controller through the serial port
# Parameters:       -
# Yields:           The latest reading obtained from the controller and a relative time index for when it was received (1s interval)
def data_gen():
    t = data_gen.t
    while True:
        t += 1
        value = ser.readline()                                                                          # Obtain temperature readings from serial port
        if(value == b"Therm\n"):
            print("Error: thermocouple not correctly placed in oven")
            # Close plot and stop updating
            quit()
        if(value == b"Stop\n"):
            print("Reflow process stopped by user")
            # Close plot and stop updating
            quit()
        if(value == b"Complete\n"):
            print("Reflow process complete")
            # Close plot and stop updating
            quit()

        yield t, bytes(value)

# Description:
def run(data):
    t, y = data
    if t > -1:
        xdata.append(t)
        ydata.append(y)
        line.set_data(xdata, ydata)
    return line,

# Description:      Exits the Python script
# Parameters:       event -
# Returns:          -
def handleClose(event):
    quit()
    #sys.exit(0)

# ------------------------------------------------------------------------------------------------------------------------------------------------------
# Main code
try:
    ser = serial.Serial("COM3", 9600)                                                                   # Configure and open serial port
    if(ser.isOpen() == True):
        print("Connection to controller established")
except(IOError):                                                                                        # Determine whether port has been successfully opened
    print("Error: connection to controller failed")
    quit()                                                                                              # Exit Python script if port could not be opened
time.sleep(1)

print()
print("Enter soak temperature (" + str(MIN_SOAK_TEMP) + "C - " + str(MAX_SOAK_TEMP) + "C):")            # Display prompt and acceptable range
while(getParameter(MIN_SOAK_TEMP, MAX_SOAK_TEMP) == False):                                             # Obtain and transmit desired soak temperature
    soakTemp = getParameter(MIN_SOAK_TEMP, MAX_SOAK_TEMP)

print()
print("Enter soak time (" + str(MIN_SOAK_TIME) + "s - " + str(MAX_SOAK_TIME) + "s):")                   # Display prompt and acceptable range
while(getParameter(MIN_SOAK_TIME, MAX_SOAK_TIME) == False):                                             # Obtain and transmit desired soak time
    soakTime = getParameter(MIN_SOAK_TIME, MAX_SOAK_TIME)

print()
print("Enter reflow temperature (" + str(MIN_REFLOW_TEMP) + "C - " + str(MAX_REFLOW_TEMP) + "C):")      # Display prompt and acceptable range
while(getParameter(MIN_REFLOW_TEMP, MAX_REFLOW_TEMP) == False):                                         # Obtain and transmit desired reflow temperature
   reflowTemp = getParameter(MIN_REFLOW_TEMP, MAX_REFLOW_TEMP)

print()
print("Enter reflow time (" + str(MIN_REFLOW_TIME) + "s - " + str(MAX_REFLOW_TIME) + "s):")             # Display prompt and acceptable range
while(getParameter(MIN_REFLOW_TIME, MAX_REFLOW_TIME) == False):                                         # Obtain and transmit desired reflow time
    reflowTime = getParameter(MIN_REFLOW_TIME, MAX_REFLOW_TIME)

print()
print("Press 'set' button to begin reflow cycle")
while True:                                                                                             # Wait for start command to be received before ploting graph
    if(ser.readline() == b"Start\n"):
        print()
        print("Starting reflow cycle")
        break

#ser.Close()

data_gen.t = -1                                                                                         # Start data generator
fig = plt.figure()                                                                                      # Initialise graph axes
fig.canvas.mpl_connect('close_event', handleClose)                                                      # Exit Python script if graph window is closed
ax = fig.add_subplot(1, 1, 1)                                                                           # Define a single subplot of grid size 1 x 1

ax.set_title('Reflow Oven Controller vX.XX')                                                            # Format axes labels
ax.set_ylabel('Temperature (C)')
ax.set_xlabel('Cycle time (s)')

line, = ax.plot([], [], lw = 2)
ax.set_ylim(0,255)                                                                                      # Set y-axis scale from 0-255C (maximum controller temperature)
ax.set_xlim(0,480)                                                                                      # Set x-axis scale from 0-480s (average total cycle time)
ax.grid()
xdata, ydata = [], []

ani = animation.FuncAnimation(fig, run, data_gen, blit = False, interval = 100, repeat = False)
plt.show()
