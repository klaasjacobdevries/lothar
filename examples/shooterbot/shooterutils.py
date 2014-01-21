from lothar import *

def logcolor(sensor):
    v = sensor.value()
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
    print 'color:', c
    return v

def logangle(m):
    d = m.degrees(False)
    print 'angle:', d, 'degrees'
    return d

def sleepuntil(condition, poll_ms = 5, timeout_ms = 30000):
    start = timer()
    end = start + timeout_ms

    while not condition():
        msleep(poll_ms)

        if timer(start) >= end:
            print 'timeout'
            return

def movevehicle(lmotor, rmotor, power, degrees):
    motor_sync(lmotor, rmotor, reset = True)

    angle = lmotor.degrees(False)

    if power >= 0:
        target = angle + degrees
        done = lambda: lmotor.degrees(False) >= target
    else:
        target = angle - degrees
        done = lambda: lmotor.degrees(False) <= target

    lmotor.run(power)
    rmotor.run(power)

    sleepuntil(done)

    lmotor.stop()
    rmotor.stop()

def shoot(gun):
    power = 100
    margin = 0.01
    
    gun.rotate(power, 1, block = True, margin = margin, timeout = 2000)
