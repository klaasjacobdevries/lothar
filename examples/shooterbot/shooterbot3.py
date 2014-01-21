from lothar import *
from shooterutils import *

class shooterbot3:
    def __init__(self, conn):
        self._lmotor = motor(conn, OUTPUT_C)
        self._rmotor = motor(conn, OUTPUT_B)

        self._gun = motor(conn, OUTPUT_A)

        self._color = sensor(conn, INPUT_3, SENSOR_COLORNONE)

    def __del__(self):
        self._gun.stop()

    def run(self):
        while True:
           self.step()

    def step(self):
        # one iteration of our loop
        self._color.reset(SENSOR_COLORGREEN)
        self.forward()

        self._color.reset(SENSOR_COLORRED)
        msleep(500)
        self.shoot()

        self._color.reset(SENSOR_COLORBLUE)
        self.backward()
        self.turn()       

    def forward(self):
        movevehicle(self._lmotor, self._rmotor, 50, int(1.5 * 360))

    def backward(self):
        movevehicle(self._lmotor, self._rmotor, -75, int(1.5 * 360))

    def turn(self):
        power = 75

        motor_unsync(self._lmotor, self._rmotor)

        degrees = 360
        target = self._lmotor.degrees(False) + degrees
        done = lambda: self._lmotor.degrees(False) >= target

        self._lmotor.run(power)
        self._rmotor.run(-int(power * 0.9))

        sleepuntil(done)
        self.stop()

    def stop(self):
        self._lmotor.stop()
        self._rmotor.stop()

    def shoot(self):
        shoot(self._gun)

def main():
    conn = connection(usb = True, bluetooth = True)
    sb3 = shooterbot3(conn)
    sb3.run()

if __name__ == '__main__':
    main()
