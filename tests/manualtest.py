#!/usr/bin/python

import sys
import os
import getopt
from lothar import *
from testcases import motortest, ultrasoundtest, lighttest, touchtest, colortest

# helper functions for getopt
def usage(out):
    out.write('''Usage: manualtest.py [options] testcase
where testcase can be any of:
motor      : basic motor test
ultrasound : ultrasound sensor test
light      : light sensor test
touch      : touch sensor test
color      : color sensor test

options can be:
-h           : print this help message
-p <port>    : the connected port. A, B or C for motor tests, 1, 2, 3 or 4 or sensor tests
               Defaults are A for motor tests and 1 for sensor tests
-u <vid,pid> : Use the usb connection with given vendor and product id
-b <address> : Use the bluetooth connection with the given address.
''')

def inputport(p):
    if p == None:
        return INPUT_1
    elif p == '1':
        return INPUT_1
    elif p == '2':
        return INPUT_2
    elif p == '3':
        return INPUT_3
    elif p == '4':
        return INPUT_4
    raise getopt.GetoptError('Invalid input port ' + str(p) + '\n')

def outputport(p):
    if p == None:
        return OUTPUT_A
    elif p == 'a' or p == 'A':
        return OUTPUT_A
    elif p == 'b' or p == 'B':
        return OUTPUT_B
    elif p == 'c' or p == 'C':
        return OUTPUT_C
    raise getopt.GetoptError('Invalid output port ' + str(p) + '\n')

if __name__ == '__main__':
    port = None
    conn = None

    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hp:u:b:')
        
        for o, a in opts:
            if o == '-h':
                usage(sys.stdout)
                sys.exit(0)
            elif o == '-p':
                port = a
            elif o == '-u':
                if not conn:
                    v, p = a.split(',', 1)
                    v = int(v)
                    p = int(p)
                    conn = connection(usb = True, vid = v, pid = p)
            elif o == '-b':
                if not conn:
                    conn = connection(bluetooth = True, address = a)
            else:
                usage(sys.stderr)
                sys.exit(1)

        if not conn:
            conn = connection(usb = True, bluetooth = True)
        
        for case in args:
            if case == 'motor':
                motortest(conn, outputport(port))
            elif case == 'ultrasound':
                ultrasoundtest(conn, inputport(port))
            elif case == 'light':
                lighttest(conn, inputport(port))
            elif case == 'touch':
                touchtest(conn, inputport(port))
            elif case == 'color':
                colortest(conn, inputport(port))
            else:
                usage(sys.stderr)
                sys.exit(1)
    except getopt.GetoptError, err:
        sys.stderr.write(str(err))
        usage(sys.stderr)
        sys.exit(1)
