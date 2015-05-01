import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys, time, math, serial

xsize = 10

# Configure RS-232 serial port on COM6
ser = serial.Serial(
    port = 'COM3',
    baudrate = 115200,
    parity = serial.PARITY_NONE,
    stopbits = serial.STOPBITS_TWO,
    bytesize = serial.EIGHTBITS
)
ser.isOpen()

def data_gen():
    t = data_gen.t
    while True:
        t+=1
        val = ser.readline()                # Read from serial port
        yield t, val

def run(data):
    # Update data
    t, y = data
    if t > -1:
        xdata.append(t)
        ydata.append(y)
        line.set_data(xdata, ydata)

    return line,

def on_close_figure(event):
    sys.exit(0)

data_gen.t = -1
fig = plt.figure()
fig.canvas.mpl_connect('close_event', on_close_figure)
ax = fig.add_subplot(111)
line, = ax.plot([], [], lw = 2)
ax.set_ylim(0,250)
ax.set_xlim(0,350)
ax.grid()
xdata, ydata = [], []

ani = animation.FuncAnimation(fig, run, data_gen, blit = False, interval = 100, repeat = False)
plt.show()
