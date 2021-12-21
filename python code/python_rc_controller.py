import tkinter as tk
from tkinter import Scrollbar, ttk
from tkinter.constants import ACTIVE, BOTTOM, LEFT
import serial.tools.list_ports
import keyboard


ports = serial.tools.list_ports.comports()
serial_ports = []
for port, desc, hwid in sorted(ports):
    serial_ports.append(port)
    print("{}: {} [{}]".format(port, desc, hwid))

# root window
root = tk.Tk()
root.geometry('500x300')
root.resizable(False, False)
root.title('Robot Controller')


# slider current value
value_speed = tk.DoubleVar()

#we need a global var to operate on serial
ser = None

#global direction variable (0,1,2,3,4) = (stop, forward, left, right, back)
dir = 0
prev_dir = 0
velocity = 0


def slider_speed_changed(event):
    global velocity
    velocity = '{}'.format(value_speed.get())
    print("Current speed: " + '{:}'.format(value_speed.get()))
    


def connectClick():
    global ser
    ser = serial.Serial(serial_list.get(ACTIVE), 115200)
    label_info.config(text='Connected', fg='green')

def generateSlider(min, max, init_val, val, func):
    slider = tk.Scale(
    root,
    from_=min,
    to=max,
    length=400,
    orient='horizontal',
    resolution=1,
    variable=val
    )
    slider.bind('<ButtonRelease-1>', func)
    slider.set(init_val)
    return slider

def sendData():
    if ser != None:
        ser.write(bytes(str(chr((int(float(velocity)) << 3) | dir)), encoding='utf-8'))

def checkKeys():
    global dir
    global prev_dir
    if keyboard.is_pressed('up'):
        dir = 1
    elif keyboard.is_pressed('down'):
        dir = 4
    elif keyboard.is_pressed('left'):
        dir = 2
    elif keyboard.is_pressed('right'):
        dir = 3
    else:
        dir = 0
    if prev_dir != dir:
        sendData()
        prev_dir = dir
    if ser is not None:
        if ser.in_waiting:
            rep = ser.readline()
            rep = rep.decode("utf-8")
    root.after(100, checkKeys)
        


frame1 = tk.Frame(root, pady=10)
serial_list = tk.Listbox(frame1, height=5, width=40)
for i in range(len(serial_ports)):
    serial_list.insert(i, serial_ports[i])
serial_list.pack(side=LEFT)
tk.Button(frame1, text='Connect', padx=20, command=connectClick).pack(side=LEFT)
frame1.pack()
label_info = tk.Label(root, text='Choose the serial port and click connect', padx=20, pady=5)
label_info.pack()



tk.Label(root,text='Speed:', pady=5).pack()
slider_speed = generateSlider(0, 8, 0, value_speed, slider_speed_changed)
slider_speed.pack()


root.after(100, checkKeys)
root.mainloop()