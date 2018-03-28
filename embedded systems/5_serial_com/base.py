#!/usr/bin/env python
import sys
import serial
import time
import datetime
import signal

serial_port = sys.argv[1]
sensor = sys.argv[2]
sampling_rate = int(sys.argv[3])

if (serial_port == None or sensor == None or sampling_rate == None):
    print "usage: ./base /dev/tty.usbXXXX sensor sampling_rate\nsensor = 'POT' | 'CELL'\nsampling rate = 5 | 50"
    sys.exit()

if (sensor != 'POT' and sensor != 'CELL'):
    print(sensor + " " + 'POT')
    print "usage: ./base /dev/tty.usbXXXX sensor sampling_rate\nsensor = 'POT' | 'CELL'\nsampling rate = 5 | 50"
    sys.exit()

if (sampling_rate != 5 and sampling_rate != 50):
    print(sampling_rate)
    print "usage: ./base /dev/tty.usbXXXX sensor sampling_rate\nsensor = 'POT' | 'CELL'\nsampling rate = 5 | 50"
    sys.exit()

ser = serial.Serial(serial_port, 9600)
ser.reset_input_buffer()
ser.reset_output_buffer()

def signal_handle(signal, frame):
    ser.write('S')
    sys.exit(0)

current_datetime = datetime.datetime.now().strftime("%m_%d_%Y_%H_%M_%S")
fileName = current_datetime + '_' + str(sensor) + '_' + str(sampling_rate) + '_'  + '.csv'
file = open(fileName, 'w+')

time.sleep(2)

message = None
if (sensor == 'POT' and sampling_rate == 5):
    message = 'A'
elif (sensor == 'POT' and sampling_rate == 50):
    message = 'B'
elif (sensor == 'CELL' and sampling_rate == 5):
    message = 'C'
elif (sensor == 'CELL' and sampling_rate == 50):
    message = 'D'

ser.write(message)

while True:
    sensor_data = ser.read()
    file.write(sensor_data)
    file.flush()


#while True:
#    ser.write(message)
#    print ser.read()
#    output = ser.read()
#    ser.write(output)
    #if output != '0': # we recieved the signal
        #break;
