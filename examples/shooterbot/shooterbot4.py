from lothar import *
from shooterutils import *

class shooterbot4:
    def __init__(self, conn):
        self._conn = conn

        self._lmotor = motor(conn, OUTPUT_C)
        self._rmotor = motor(conn, OUTPUT_B)

        self._gun = motor(conn, OUTPUT_A)

        self._color = sensor(conn, INPUT_3, SENSOR_COLORNONE)
        self._us    = sensor(conn, INPUT_4, SENSOR_LOWSPEED_9V)

        self._idle = False
        
    def __del__(self):
        self._gun.stop()

    def run(self):
        while True:
            self.step()

    def step(self):
        if self.detect():
            self._idle = False
            self.inspect() # enter the 'inspect' sequence
        else:
            # only give 'turn and turn' commands if we are not already doing that
            if not self._idle:
                self.turn()
                self._idle = True

    def inspect(self):
        self._color.reset(SENSOR_COLORBLUE)
        self._lmotor.stop()
        self._rmotor.stop()
        
        self.play('Object')
        msleep(1000)

        if self.detect(): # threat is still there, go to the 'alarm' sequence
            self.alarm()

    def alarm(self):
        self._color.reset(SENSOR_COLORRED)
        self.play('Alarm')
        msleep(1000)
        shoot(self._gun)
                
    def turn(self):
        power = 50

        self._color.reset(SENSOR_COLORGREEN)
        self._lmotor.run(-power)
        self._rmotor.run(power)

    def play(self, message):
        print message
        self._conn.playsoundfile(False, message + '.rso')

    def detect(self):
        distance = 40
       
        val = self._us.value()
        return val < distance

def main():
    conn = connection(usb = True, bluetooth = True)
    sb4 = shooterbot4(conn)
    sb4.run()

if __name__ == '__main__':
    main()
