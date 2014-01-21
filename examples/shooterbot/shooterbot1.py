from lothar import *
from shooterutils import *

class shooterbot1:
    sleeptime_ms = 2000
    power = 75
  
    def __init__(self, conn):
        self._lmotor = motor(conn, OUTPUT_C)
        self._rmotor = motor(conn, OUTPUT_B)

    def run(self):
        self.runforward()
        msleep(self.sleeptime_ms)
        self.runbackward()
        msleep(self.sleeptime_ms)

    def runforward(self):
        print 'going forward'
        movevehicle(self._lmotor, self._rmotor, self.power, 5 * 360)
        print 'stop'

    def runbackward(self):
        print 'going backward'
        movevehicle(self._lmotor, self._rmotor, -self.power, 5 * 360)
        print 'stop'

def main():
    conn = connection(usb = True, bluetooth = True)
    
    print 'battery level is ', conn.getbatterylevel()

    sb1 = shooterbot1(conn)
    sb1.run()

if __name__ == '__main__':
    main()
