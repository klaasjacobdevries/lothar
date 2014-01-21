from utils import *
from lothar import *

class colorsorter1:
    power = 50

    def __init__(self, conn):
        self._dispenser = motor(conn, OUTPUT_B)
        self._control = sensor(conn, INPUT_1, SENSOR_SWITCH)

    def run(self):
        while True:
            self.step()

    def step(self):
        self.nextball()
        self.dispense(2)
        msleep(1000)

    def dispense(self, turns):
        self._dispenser.run(colorsorter1.power)
        rununtilcycle(self._dispenser, turns * 360)

    def nextball(self):
        self._dispenser.run(colorsorter1.power)
        touch = lambda: self._control.value()
        busywait(touch)

if __name__ == '__main__':
    main_runner(colorsorter1)
