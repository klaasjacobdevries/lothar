from lothar import *
from time import sleep

import socket
import sys
import select
import base64
import struct

def dumpbinary(buf):
    sys.stderr.write('%d\n' % len(buf))
    i = 0
    for b in buf:
        sys.stderr.write('%2x' % b)
        i += 1
        if i == 80:
            sys.stderr.write('\n')
            i = 0
    sys.stderr.write('\n')

class Foon:
    def __init__(self, host = 'nerdfoon', port = 3001):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        self.sock.connect((host, port))

    def write(self, buf):
        try:
            sizebuf = struct.pack('<H', len(buf))
            self.sock.sendall(sizebuf + buf)
            return len(buf)
        except Exception, e:
            print e
            return -1
        
    def read(self, n):
        try:
            sizebuf = self.sock.recv(2)
            size = struct.unpack('<H', sizebuf)[0]
            buf = self.sock.recv(size)
            return buf
        except Exception, e:
            print e
            return ''
    
    def close(self):
        self.sock.close()
        return 0
    
#foon = Foon()
c = connection(usb = True)

print c.getbatterylevel()

class Motor:
    def __init__(self, connection, port):
        self.conn = connection
        if port == 'a' or port == 'A':
            self.port = OUTPUT_A
        elif port == 'b' or port == 'B':
            self.port = OUTPUT_B
        elif port == 'c' or port == 'C':
            self.port = OUTPUT_C
        else:
            raise Exception('oei')

    def __del__(self):
        self.stop()

    def run(self, power):
        self.conn.setoutputstate(self.port, 
                                 power, 
                                 MOTOR_MODE_MOTORON | MOTOR_MODE_BRAKE | MOTOR_MODE_REGULATED, 
                                 REGULATION_MODE_SPEED,
                                 0,
                                 RUNSTATE_RUNNING,
                                 0)

    def turn(self, power, degrees): # minimum of 360 ?
        self.conn.setoutputstate(self.port, 
                                 power, 
                                 MOTOR_MODE_MOTORON | MOTOR_MODE_BRAKE | MOTOR_MODE_REGULATED, 
                                 REGULATION_MODE_SPEED,
                                 0,
                                 RUNSTATE_RUNNING,
                                 degrees)

    def turns(self, power, degrees):
        goal = self.degrees()

        if (power > 0 and degrees > 0) or (power < 0 and degrees < 0):
            goal += degrees

            self.run(power)

            while self.degrees() < goal:
                sleep(0.01)

            self.brake()

        elif (power < 0 and degrees > 0) or (power > 0 and degrees < 0):
            goal -= degrees

            self.run(power)

            while self.degrees() > goal:
                pass

            self.brake()

    def stop(self):
        self.conn.setoutputstate(self.port, 
                                 0,
                                 0,
                                 REGULATION_MODE_IDLE,
                                 0,
                                 RUNSTATE_IDLE,
                                 0)

    def brake(self):
        self.conn.setoutputstate(self.port,
                                 0,
                                 MOTOR_MODE_MOTORON | MOTOR_MODE_BRAKE | MOTOR_MODE_REGULATED, 
                                 REGULATION_MODE_SPEED,
                                 0,
                                 RUNSTATE_RUNNING,
                                 0)

    def degrees(self):
        return self.state()[8]

    def state(self):
        return self.conn.getoutputstate(self.port)

    def reset(self, relative):
        self.conn.resetmotorposition(self.port, relative)

    def rotations(self, power, n):
        self.turn(power, n * 360)

    def run_time(self, power, time):
        self.run(power)
        sleep(time)
        self.brake()

class UltraSound:
    def __init__(self, conn, port):
        self.conn = conn
        self.port = port

        self.conn.setinputmode(self.port, SENSOR_LOWSPEED_9V, SENSOR_MODE_RAWMODE)
        sleep(0.5)

        self.active = False

    def __del__(self):
        self.stop()

    def stop(self):
        if self.active:
            msg = chr(0x02) + chr(0x41) + chr(0x00)
            print 'msg \'%s\' %i' % (msg, len(msg))
            self.conn.lswrite(self.port, msg, 0)

    def state(self):
        return self.conn.getinputvalues(self.port)

    def singleshot(self):
        self.active = True

        msg = chr(0x02) + chr(0x41) + chr(0x02)
        print 'msg \'%s\' %i' % (msg, len(msg))
        self.conn.lswrite(self.port, msg, 0)

    def get(self):

#        if not self.active:
#            self.singleshot()

        msg = chr(0x02) + chr(0x42)
        self.conn.lswrite(self.port, msg, 1)
#        self.active = False

        i = 0
        while self.conn.lsgetstatus() == 0:
            i += 1
            if i > 10:
                self.stop()
                raise Exception('timeout')

        return self.conn.lsread(self.port)

class Light:
    def __init__(self, conn, port, active):
        self.conn = conn
        self.port = port

        if active:
            m = SENSOR_LIGHT_ACTIVE
        else:
            m = SENSOR_LIGHT_INACTIVE

        self.conn.setinputmode(self.port, SENSOR_REFLECTION, SENSOR_MODE_PCTFULLSCALEMODE)

    def get(self):
        return self.conn.getinputvalues(self.port)

class Color:
    def __init__(self, conn, port):
        self.conn = conn
        self.port = port

        self.conn.setinputmode(self.port, SENSOR_COLORFULL, SENSOR_MODE_RAWMODE)
        self.active = False

        sleep(0.5)

    def __del__(self):
        self.stop()

    def stop(self):
        self.conn.setinputmode(self.port, SENSOR_NO_SENSOR, SENSOR_MODE_RAWMODE)
#        if self.active:
#            msg = chr(0x02) + chr(0x41) + chr(0x00)
#            print 'msg \'%s\' %i' % (msg, len(msg))
#            self.conn.lswrite(self.port, msg, 0)
#            self.active = False

    def get(self):
        return self.conn.getinputvalues(self.port)
        self.active = True

        msg = chr(0x02) + chr(66)
        self.conn.lswrite(self.port, msg, 4)

        i = 0
        while self.conn.lsgetstatus() == 0:
            i += 1
            if i > 10:
                self.stop()
                raise Exception('timeout')

        return self.conn.lsread(self.port)

class Touch:
    def __init__(self, conn, port):
        self.conn = conn
        self.port = port

        self.conn.setinputmode(self.port, SENSOR_SWITCH, SENSOR_MODE_BOOLEANMODE)

    def get(self):
        return self.conn.getinputvalues(self.port)

def testmotor():
    m = motor(c, OUTPUT_A)
    
    m.run(50)
    print m.power()
    sleep(2)

    m.stop()

    sleep(1)
    m.turn(20, 90, block = True)
    sleep(1)

    m.rotate(50, 5, 10)
    
    print m.degrees(1)

    print 'done'

def test_us():
    s = UltraSound(c, INPUT_3)

    for i in range(10):
        print ord(s.get())
        sleep(1)
        
def testus():
    s = sensor(c, INPUT_3, SENSOR_LOWSPEED_9V)

    while True:
        print s.value(), s.min(), s.max()
        sleep(1)
    s.stop()
 
def testlight():
    s = Light(c, INPUT_1, False)

    for i in range(10):
        print s.get()
        sleep(1)
    s.stop()

def testtouch():
    s = sensor(c, SENSOR_SWITCH, INPUT_2)

    print s.min(), s.max()
    for i in range(10):
        print s.value(), s.min(), s.max()
        s.stop()
        sleep(1)
    s.stop()

from random import choice
def testcolor():
    l = [SENSOR_COLORFULL, SENSOR_COLORRED, SENSOR_COLORGREEN, SENSOR_COLORBLUE, SENSOR_COLORNONE]

    s = sensor(c, INPUT_4, choice(l))

    for i in range(30):
        print s.value(), s.min(), s.max()
        sleep(1)
        s.stop()
        s.reset(choice(l))
    s.stop()

def testlight():
    s = sensor(c, INPUT_1, SENSOR_LIGHT_INACTIVE)

    for i in range(10):
        print s.value(), s.min(), s.max()

        msleep(1000)
    s.stop()

#n = c.lsgetstatus()
#print n

#testmotor()
#test_us()
#testus()
#testlight()
#testtouch()
testcolor()

#m = motor(c, OUTPUT_A)
#m.run(10)

