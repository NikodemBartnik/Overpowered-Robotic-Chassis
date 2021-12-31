import tkinter as tk
from tkinter import Scrollbar, ttk
from tkinter.constants import ACTIVE, BOTTOM, LEFT
import serial.tools.list_ports

ports = serial.tools.list_ports.comports()
serial_ports = []
for port, desc, hwid in sorted(ports):
    serial_ports.append(port)
    print("{}: {} [{}]".format(port, desc, hwid))

# root window
root = tk.Tk()
root.geometry('600x600')
root.resizable(False, False)
root.title('MIT Mini Cheetah Controler')


# slider current value
value_position = tk.DoubleVar()
value_speed = tk.DoubleVar()
value_kp = tk.DoubleVar()
value_kd = tk.DoubleVar()
value_torque = tk.DoubleVar()

#we need a global var to operate on serial
ser = None


def slider_position_changed(event):
    #print("Current position: " + '{: .2f}'.format(value_position.get()))
    sendData()

def slider_speed_changed(event):
    #print("Current speed: " + '{: .2f}'.format(value_speed.get()))
    sendData()

def slider_kp_changed(event):
    #print("Current kp: " + '{: .2f}'.format(value_kp.get()))
    sendData()

def slider_kd_changed(event):
    #print("Current kd: " + '{: .2f}'.format(value_kd.get()))
    sendData()

def slider_torque_changed(event):
    #print("Current torque: " + '{: .2f}'.format(value_torque.get()))
    sendData()

def connectClick():
    global ser
    ser = serial.Serial(serial_list.get(ACTIVE), 115200)
    label_info.config(text='Connected', fg='green')

def generateSlider(min, max, init_val, val, func):
    slider = tk.Scale(
        root,
        from_=min,
        to=max,
        length=500,
        orient='horizontal',
        resolution=0.01,
        variable=val
    )
    slider.bind('<ButtonRelease-1>', func)
    slider.set(init_val)
    return slider


def sendData():
    if ser.is_open:
        prepared_payload = '{: .2f},{: .2f},{: .2f},{: .2f},{: .2f}\n'.format(
            value_torque.get(), 
            value_position.get(), 
            value_speed.get(), 
            value_kp.get(), 
            value_kd.get()
        )

        print(prepared_payload)
        ser.write(bytes(prepared_payload, encoding='utf-8'))
        

def readFromSerial():
    if ser is not None:
        if ser.in_waiting:
            rep = ser.readline()
            rep = rep.decode("utf-8")
            print(rep)
            rep = rep.strip().split(',')
            if(len(rep) == 3):
                label_position.config(text='Position: ' + rep[0])
                label_speed.config(text='Speed: ' + rep[1])
                label_current.config(text='Current: ' + rep[2])
    root.after(10, readFromSerial)

        

frame1 = tk.Frame(root, pady=10)
serial_list = tk.Listbox(frame1, height=5, width=50)
for i in range(len(serial_ports)):
    serial_list.insert(i, serial_ports[i])
serial_list.pack(side=LEFT)
tk.Button(frame1, text='Connect', padx=20, command=connectClick).pack(side=LEFT)
frame1.pack()
label_info = tk.Label(root, text='Choose the serial port and click connect', padx=20, pady=5)
label_info.pack()

# label for the slider
tk.Label(root,text='Position:', pady=5).pack()
slider_position = generateSlider(-12.5, 12.5, 0, value_position, slider_position_changed)
slider_position.pack()

tk.Label(root,text='Speed:', pady=5).pack()
slider_speed = generateSlider(-45, 45, 0, value_speed, slider_speed_changed)
slider_speed.pack()

tk.Label(root,text='KP:', pady=5).pack()
slider_kp = generateSlider(0, 500, 0, value_kp, slider_kp_changed)
slider_kp.pack()

tk.Label(root,text='Kd:', pady=5).pack()
slider_kd = generateSlider(-0, 5, 0, value_kd, slider_kd_changed)
slider_kd.pack()

tk.Label(root,text='Torque:', pady=5).pack()
slider_torque = generateSlider(-10, 10, 0, value_torque, slider_torque_changed)
slider_torque.pack()

frame2 = tk.Frame(root)
label_position = tk.Label(frame2,text='Position:', padx=15, pady=30)
label_position.pack(side=LEFT)
label_speed = tk.Label(frame2,text='Speed:', padx=15)
label_speed.pack(side=LEFT)
label_current = tk.Label(frame2,text='Current:',padx=15)
label_current.pack(side=LEFT)

frame2.pack()

root.after(10, readFromSerial)
root.mainloop()
