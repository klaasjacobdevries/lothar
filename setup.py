from distutils.core import setup, Extension
import os
import sys

def globsources(dir, ext = ('.c',)):
    sources = []
    for file in os.listdir(dir):
        file = os.path.join(dir, file)
        i = file.rfind('.')
        e = file[i:]
        if os.path.isfile(file) and e in ext:
            sources.append(file)
        elif os.path.isdir(file):
            sources.extend(globsources(file, ext))
    return sources

def currentdir():
    return os.path.dirname(__file__)

# configure, if not already done so
configname = os.path.join('include', 'config.h')
configname_in = os.path.join(currentdir(), 'src', 'include', 'config.h.in')

# run cmake if appropriate
if not os.path.exists(configname) or os.path.getmtime(configname) < os.path.getmtime(configname_in):
    if os.system('cmake %s' % currentdir()) or not os.path.exists(configname):
        sys.stderr('Configuring with cmake failed\n')
        sys.exit(1)

# inspect config.h, to find libraries to link to
def hasconfig(filename, symbol):
    f = open(filename, 'r')
    define = '#define HAVE_%s\n' % symbol   
    for line in f:
        if line == define:
            f.close()
            return True
    f.close()
    return False

def libraries(filename):
    l = []
    if hasconfig(filename, 'LIBUSB'):
        l.append('usb-1.0')
    if hasconfig(filename, 'LIBUSBX'):
        l.append('libusb-1.0')
    if hasconfig(filename, 'LIBUSB_0_1'):
        l.append('usb')
    if hasconfig(filename, 'BLUEZ'):
        l.append('bluetooth')
    return l

# it is a lot easier to just compile in the lothar sources than to link against them
lothar = Extension('lothar',
                   sources = globsources(os.path.join(currentdir(), 'src')) + globsources(os.path.join(currentdir(), 'wrappers', 'python')),
                   include_dirs = ['%s/src/include' % currentdir(), 'include'],
                   libraries = libraries(configname))

setup(name = 'lothar', version = '1.0', description = 'lothar', ext_modules = [lothar])
