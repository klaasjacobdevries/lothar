from lothar import *
from shootertutils import *

class shooterbot2:
    def __init__(self, conn):
        self._lmotor = motor(conn, OUTPUT_C)
        self._rmotor = motor(conn, OUTPUT_B)
        self._color = sensor(conn, INPUT_3, SENSOR_COLORFULL)

    def _sync(self):
        motor_sync(self._lmotor, self._rmotor, reset = True)

    def _unsync(self):
        motor_unsync(self._lmotor, self._rmotor)

    def run(self):
        self.forward()
        self.turn()
        self.backward()

    def stop(self):
        self._lmotor.stop()
        self._rmotor.stop()

    def forward(self):
        power = 50
        
        self._sync()

        self._lmotor.run(power)
        self._rmotor.run(power)

        done = lambda: logcolor(self._color) == COLOR_RED
        sleepuntil(done)
        
        self.stop()

    def turn(self):
        power = 75
        
        self._unsync()

        degrees = 2.65 * 360
        target = self._lmotor.degrees(False) + degrees

        self._lmotor.run(power)
        self._rmotor.run(-int(power * 0.9))

        done = lambda: logangle(self._lmotor) >= target
        sleepuntil(done)

        self.stop()

    def backward(self):
        power = 75

        self._sync()
        
        degrees = 4 * 360
        target = self._lmotor.degrees(False) + degrees

        self._lmotor.run(power)
        self._rmotor.run(power)

        done = lambda: logangle(self._lmotor) >= target
        sleepuntil(done)

        self.stop()

def main():
    conn = connection(usb = True, bluetooth = True)
    sb2 = shooterbot2(conn)
    sb2.run()

if __name__ == '__main__':
    main()
