import datetime
import serial
import struct
import time
import os

from serial.tools import list_ports

# change these in settings.txt
mode = 0
threshold = 3
trials = 300
time_between_inputs = 30
move_distance = 127
start_delay = 10

with open("settings.txt", "r") as settings:
    for line in settings:
        settings_pair = line.split('=')
        
        match settings_pair[0]:
            case "mode":
                if settings_pair[1].strip() == "click":
                    mode = 0
                elif settings_pair[1].strip() == "motion":
                    mode = 1
                    
            case "threshold":
                threshold = int(settings_pair[1])
                
            case "trials":
                trials = int(settings_pair[1])
                
            case "time_between_inputs":
                time_between_inputs = int(settings_pair[1])
                
            case "move_distance":
                move_distance = int(settings_pair[1])
                
            case "start_delay":
                start_delay = int(settings_pair[1])
searching = True
while searching == True:
    ports = list(list_ports.comports())
    for port in ports:
        if "Arduino" in port.description:
            arduino = serial.Serial(port=port.device, baudrate=115200, timeout=0.1)
            searching = False
            break
        
    time.sleep(1)

time.sleep(start_delay)

arr = bytearray()
arr.extend(struct.pack("h", threshold))
arr.extend(struct.pack("h", trials))
arr.extend(struct.pack("h", time_between_inputs))
arr.extend(struct.pack("b", move_distance))
arr.extend(struct.pack("b", mode))

arduino.write(arr)

results = []
while True:
    time.sleep(0.05)
    if arduino.in_waiting >= 4:
        read = struct.unpack("I", arduino.read(4))[0]
        if read == 1:
            break
        if read == 2:
            raise RuntimeError("Input timed out!")
        results.append(read)

filename = "results/result-" + str(datetime.datetime.now()).replace(':', '_') + ".csv"
file = os.path.join(os.path.dirname(__file__), filename)

with open(file, "x") as f:
    f.write("MouseTester\n")
    f.write("400\n")
    f.write("xCount,yCount,Time (ms),buttonflags\n")

    time = 19
    for r in results:
        time += r / 1000
        f.write("1,0," + str(time) + ",0\n")
    
