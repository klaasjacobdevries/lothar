from utils import *
from lothar import *
from colorsorter2 import colorsorter2

class basket:
    RED, YELLOW, BLUE, GREEN = (0, 90, 180, 270) # enum

    def __init__(self, motor, control):
        self._motor = motor
        self._control = control
        
        self._motor.reset(False)

    def __del__(self):
        self._motor.stop()

    def angle(self):
        return self._motor.degrees(False)

    def turn(self, heading):        
        if heading < 0 or heading >= 360:
            raise ValueError('Heading should be in the range (0, 360)')

        power = 30
       
        abscurrent = self.angle()
        current = abscurrent % 360

        target = heading - current
        if target < 0:
            target += 360

        print abscurrent, current, heading, target
        if target:
            self._motor.turn(power = power, degrees = target, block = True, timeout = 10000)
            #self._motor.stop()
        a = self.angle()
        print a, a % 360, a % 90

class colorsorter3:

    def __init__(self, conn):
        self._basket = basket(motor(conn, OUTPUT_C), 
                              sensor(conn, INPUT_2, SENSOR_SWITCH))

        self._dispenser = colorsorter2(conn)

        # maps colors to basket positions
        self._colormap = {COLOR_BLUE   : basket.BLUE,
                          COLOR_GREEN  : basket.GREEN,
                          COLOR_YELLOW : basket.YELLOW,
                          COLOR_RED    : basket.RED}

    def run(self):
        while True:
            self.step()

    def wait(self):
        msleep(2000)
        raw_input() # wait for keypress

    def say(self, color):
        self._dispenser.say(color)

    def step(self):
        self._dispenser.nextball()
        self._dispenser.dispense()
        msleep(300)
        color = self._dispenser.color()
        self.say(color)
            
        try:
            heading = self._colormap[color]
            self._basket.turn(heading)
        except KeyError:
            pass # can't do anything meaningfull
 
if __name__ == '__main__':
    main_runner(colorsorter3)
