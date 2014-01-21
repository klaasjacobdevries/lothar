from utils import *
from lothar import *
from colorsorter1 import colorsorter1

class colorsorter2:
    def __init__(self, conn):
        self._base = colorsorter1(conn) # much of this can be reused

        self._conn = conn
        self._color = sensor(conn, INPUT_3, SENSOR_COLORFULL)

        # map colors to sounds (file names)
        self._colormap = {COLOR_BLUE   : 'Blue',
                          COLOR_GREEN  : 'Green',
                          COLOR_YELLOW : 'Yellow',
                          COLOR_RED    : 'Red'}

    def run(self):
        while True:
            self.step()

    def nextball(self):
        self._base.nextball()
        
    def dispense(self):
        self._base.dispense(0.8)

    def step(self):
        self.nextball()
        self.dispense()
        msleep(300)
        color = self.color()
        self.say(color)

    def say(self, color):
        sound = self._colormap.get(color, 'Error')

        print sound
        self._conn.playsoundfile(False, sound + '.rso')
        msleep(1000)

    def color(self):
        return self._color.value()

if __name__ == '__main__':
    main_runner(colorsorter2)
