import sys
from lothar import *

def busywait(condition):
    while not condition():
        pass

def sleepwait(condition, sleeptime_ms):
    while not condition():
        msleep(sleeptime_ms)

def rununtilcycle(motor, degrees):
    current = motor.degrees(False)
    target = current + degrees
    
    done = lambda: motor.degrees(False) >= target
    busywait(done)

    motor.stop()

def main_runner(botbuilder, argv = sys.argv):
    conn = connection(usb = True, bluetooth = True)

    bot = botbuilder(conn)
    bot.run()
