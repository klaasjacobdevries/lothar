from lothar import *

def motortest(conn, port):
    print 'motor test'

    m = motor(conn, port)
    
    print 'run(50) 2s'
    m.run(50)
    msleep(2000)
    
    print 'stop() 1s'
    m.stop()
    msleep(1000)

    print 'turn(20, 90, block = True) 1s'
    m.turn(20, 90, block = True)
    msleep(1000)

    print 'rotate(50, 5, 10)'
    m.rotate(50, 5, 10)

    print 'degrees %d' % m.degrees(True)

    print 'done'

def ultrasoundtest(conn, port):
    print 'ultrasound test'

    s = sensor(conn, port, SENSOR_LOWSPEED_9V)

    print 'sensing for 20s with 1s interval'
    print 'value(), min(), max()'
    for i in xrange(20):
        print s.value(), s.min(), s.max()
        msleep(1000)

    print 'done'

def lighttest(conn, port):
    print 'light sensor test'
    
    s = sensor(conn, port, SENSOR_LIGHT_INACTIVE)

    print 'inactive sensor, 10s with 1s interval'
    print 'value(), min(), max()'

    for i in xrange(10):
        print s.value(), s.min(), s.max()
        msleep(1000)

    s.reset(SENSOR_LIGHT_ACTIVE)
    print 'active sensor, 10s with 1s interval'
    print 'value(), min(), max()'

    for i in xrange(10):
        print s.value(), s.min(), s.max()
        msleep(1000)

    print 'inactive sensor, 10s with 1s interval'

    print 'done'

def touchtest(conn, port):
    print 'touch sensor test'

    s = sensor(conn, port, SENSOR_SWITCH)

    print 'sensing for 20s with 1s interval'
    print 'value(), min(), max()'
    for i in xrange(20):
        print s.value(), s.min(), s.max()
        msleep(1000)

    print 'done'

def colortest(conn, port):
    print 'color sensor test'

    s = sensor(conn, port, SENSOR_COLORFULL)

    print 'full, 10s with 1s interval'
    print 'value(), min(), max(), color'

    for i in xrange(10):
        v = s.value()

        if v == COLOR_BLACK:
            c = 'black'
        elif v == COLOR_BLUE:
            c = 'blue'
        elif v == COLOR_GREEN:
            c = 'green'
        elif v == COLOR_YELLOW:
            c = 'yellow'
        elif v == COLOR_RED:
            c = 'red'
        elif v == COLOR_WHITE:
            c = 'white'
        else:
            c = 'unkown'

        print v, s.min(), s.max(), c
    
        msleep(1000)

    s.reset(SENSOR_COLORRED)
    print 'red, 10s with 1s interval'
    print 'value(), min(), max()'
    for i in xrange(10):
        print s.value(), s.min(), s.max()
        msleep(1000)

    s.reset(SENSOR_COLORGREEN)
    print 'green, 10s with 1s interval'
    print 'value(), min(), max()'
    for i in xrange(10):
        print s.value(), s.min(), s.max()
        msleep(1000)

    s.reset(SENSOR_COLORBLUE)
    print 'blue, 10s with 1s interval'
    print 'value(), min(), max()'
    for i in xrange(10):
        print s.value(), s.min(), s.max()
        msleep(1000)

    s.reset(SENSOR_COLORNONE)
    print 'inactive, 10s with 1s interval'
    print 'value(), min(), max()'
    for i in xrange(10):
        print s.value(), s.min(), s.max()
        msleep(1000)
        
    print 'done'
