'''
Format values that are usually associated with a unit.
'''
from __future__ import division
import bisect
import decimal


class UnitOfMagnitude(object):
    '''
    Models a unit of magnitude such as 'Kilobyte', 'milimetre' or 'hectare'.
    '''
    def __init__(self, magnitude, format, html):
        self.magnitude = decimal.Decimal(str(magnitude))
        self.format = format
        self.html = html

    def adjust(self, number):
        return decimal.Decimal(str(number)) / self.magnitude

    def __cmp__(self, otherUnit):
        if self.magnitude < otherUnit.magnitude:
            return -1
        elif self.magnitude == otherUnit.magnitude:
            return 0
        else:
            return 1


class UnitFormat(object):
    def __init__(self):
        self.units = list()

    def addUnitOfMagnitude(self, magnitude, format, html=None):
        '''
        Adds a unit to this format. For example, to add support for centimetres
        to a UnitFormat of metric length, use magnitude of 0.01d (ie 1/100th of
        a metre) and number format '###0.# cm'.
        '''
        if html is None:
            html = format
        bisect.insort(self.units, UnitOfMagnitude(magnitude, format, html))

    def format(self, number):
        bestUnit = self.bestUnitForObject(number)
        numberAdjusted = bestUnit.adjust(number)
        return bestUnit.format.format(numberAdjusted)

    def html(self, number):
        bestUnit = self.bestUnitForObject(number)
        numberAdjusted = bestUnit.adjust(number)
        return bestUnit.html.format(numberAdjusted)

    def bestUnitForObject(self, number):
        if not self.units:
            raise LookupError()

        # take the absolute value as some values, such as offsets, could be 
        # negative
        as_decimal = abs(decimal.Decimal(str(number)))

        # a value of 0 has no best unit. So, return the default unit instead.
        if as_decimal == 0:
            # assume all unit types have one unit with a magnitude of 1, which 
            # is the case so far. 
            return [u for u in self.units if u.magnitude == 1][0]
        
        # find the largest unit not bigger than this value
        for unit in self.units:
            if as_decimal >= unit.magnitude:
                best_unit = unit
            else:
                break
        return best_unit

file_size = UnitFormat()
file_size.addUnitOfMagnitude(1, '{0:.0f} bytes')
file_size.addUnitOfMagnitude(1024, '{0:.1f} KB')
file_size.addUnitOfMagnitude(1024 * 1024, '{0:.1f} MB')
file_size.addUnitOfMagnitude(1024 * 1024 * 1024, '{0:.1f} GB')

# Create a Unit for measuring lengths
distance = UnitFormat()
distance.addUnitOfMagnitude('.000001', '{0:.1f} &#181;m')
distance.addUnitOfMagnitude('.001', '{0:.1f} mm')
distance.addUnitOfMagnitude(1, '{0:.1f} m')
distance.addUnitOfMagnitude(1000, '{0:.1f} km')

# Create a Unit for measuring frequencies
frequency = UnitFormat()
frequency.addUnitOfMagnitude('.000001', '{0:.3f} &#181;Hz')
frequency.addUnitOfMagnitude('.001', '{0:.3f} mHz')
frequency.addUnitOfMagnitude(1, '{0:.3f} Hz')
frequency.addUnitOfMagnitude(1000, '{0:.3f} kHz')
frequency.addUnitOfMagnitude(1000000, '{0:.3f} MHz')
frequency.addUnitOfMagnitude(1000000000, '{0:.3f} GHz')
frequency.addUnitOfMagnitude(1000000000000, '{0:.3f} THz')

# Create a Unit for measuring flux densities
flux = UnitFormat()
flux.addUnitOfMagnitude('.000000001', '{0:.3f} nJy')
flux.addUnitOfMagnitude('.000001', '{0:.3f} &#181;Jy')
flux.addUnitOfMagnitude('.001', '{0:.3f} mJy')
flux.addUnitOfMagnitude(1, '{0:.3f} Jy')
flux.addUnitOfMagnitude(1000, '{0:.3f} kJy')

# Create a Unit for measuring energy
energy = UnitFormat()
energy.addUnitOfMagnitude('.000000000000000000000001', '{0:.3f} yJ')
energy.addUnitOfMagnitude('.000000000000000000001', '{0:.3f} zJ')
energy.addUnitOfMagnitude('.000000000000000001', '{0:.3f} aJ')
energy.addUnitOfMagnitude('.000000000000001', '{0:.3f} fJ')
energy.addUnitOfMagnitude('.000000000001', '{0:.3f} pJ')
energy.addUnitOfMagnitude('.000000001', '{0:.3f} nJ')
energy.addUnitOfMagnitude('.000001', '{0:.3f} &#181;J')
energy.addUnitOfMagnitude('.001', '{0:.3f} mJ')
energy.addUnitOfMagnitude(1, '{0:.3f} J')
energy.addUnitOfMagnitude(1000, '{0:.3f} kJ')
energy.addUnitOfMagnitude(1000000, '{0:.3f} MJ')
energy.addUnitOfMagnitude(1000000000, '{0:.3f} GJ')
energy.addUnitOfMagnitude(1000000000000, '{0:.3f} TJ')
energy.addUnitOfMagnitude(1000000000000000, '{0:.3f} PJ')
energy.addUnitOfMagnitude(1000000000000000000, '{0:.3f} EJ')
energy.addUnitOfMagnitude(1000000000000000000000, '{0:.3f} ZJ')
energy.addUnitOfMagnitude(1000000000000000000000000, '{0:.3f} YJ')

velocity = UnitFormat()
velocity.addUnitOfMagnitude(1, '{0:.3f} m/s')
velocity.addUnitOfMagnitude(1000, '{0:.3f} km/s')

proper_motion = UnitFormat()
proper_motion.addUnitOfMagnitude(1, '{0:.3f} mas/yr')
proper_motion.addUnitOfMagnitude(1000, '{0:.3f} arcsec/yr')
proper_motion.addUnitOfMagnitude(60*1000, '{0:.3f} arcmin/yr')
proper_motion.addUnitOfMagnitude(60*60*1000, '{0:.3f} deg/yr')	

temperature = UnitFormat()
temperature.addUnitOfMagnitude('.000000000000000000000001', '{0:.3f} yK')
temperature.addUnitOfMagnitude('.000000000000000000001', '{0:.3f} zK')
temperature.addUnitOfMagnitude('.000000000000000001', '{0:.3f} aK')
temperature.addUnitOfMagnitude('.000000000000001', '{0:.3f} fK')
temperature.addUnitOfMagnitude('.000000000001', '{0:.3f} pK')
temperature.addUnitOfMagnitude('.000000001', '{0:.3f} nK')
temperature.addUnitOfMagnitude('.000001', '{0:.3f} &#181;K')
temperature.addUnitOfMagnitude('.001', '{0:.3f} mK')
temperature.addUnitOfMagnitude(1, '{0:.3f} K')

arc = UnitFormat()
arc.addUnitOfMagnitude(1, '{0:.3f}\u00b0')
arc.addUnitOfMagnitude(1/60, '{0:.3f}\u2032')
arc.addUnitOfMagnitude(1/3600, '{0:.3f}\u2033')
arc.addUnitOfMagnitude(1/3600000, '{0:.3f} mas')
arc.addUnitOfMagnitude(1/3600000000, '{0:.3f} &#181;as')

time = UnitFormat()
time.addUnitOfMagnitude('.001', '{0:.0f} ms')
time.addUnitOfMagnitude(1, '{0:.1f} s')
time.addUnitOfMagnitude(60, '{0:.0f} mins')
time.addUnitOfMagnitude(3600, '{0:.0f} hours')
time.addUnitOfMagnitude(86400, '{0:.0f} days')
time.addUnitOfMagnitude(604800, '{0:.0f} weeks')
time.addUnitOfMagnitude(31536000, '{3:.0f} years')


if __name__ == '__main__':
    # print some sample file sizes
    x722b = 722L
    x498kb = 509952L
    x15mb = 15728640L
    x300mb = 314572800L
    x40gb = 42949672960L
    print file_size.format(x722b)
    print file_size.format(x498kb)
    print file_size.format(x498kb + x722b)
    print file_size.format(x15mb)
    print file_size.format(x15mb + x498kb + x722b)
    print file_size.format(x300mb)
    print file_size.format(x300mb + x15mb + x498kb + x722b)
    print file_size.format(x40gb)
    print file_size.format(x40gb + x300mb + x15mb + x498kb + x722b)

    # print some lengths
    x315um = 0.000315
    x867mm = 0.867
    x128m = 128
    print distance.format(x315um)
    print distance.format(x867mm)
    print distance.format(x128m)
    print distance.format(x128m + x867mm)

    x300deg = 300.0
    x3deg = 3.0
    x3minArc = 0.05
    x9secArc = 0.0025
    x1d25secArc = 1/3600/25
    print arc.format(x300deg)
    print arc.format(x3deg)
    print arc.format(x3minArc)
    print arc.format(x9secArc)
    print arc.format(x1d25secArc)

    x1maspm = 1.0
    x1aspm = 1000.0
    x1ampm = 1000*60.0
    x1degpm = 1000*3600.0
    print proper_motion.format(x1maspm)
    print proper_motion.format(x1aspm)
    print proper_motion.format(x1ampm)
    print proper_motion.format(x1degpm)

    x218ms = 0.218
    x18s = 18
    x2m = 120
    x3h = 10800
    print time.format(x218ms)
    print time.format(x18s)
    print time.format(x218ms + x18s)
    print time.format(x2m)
    print time.format(x2m + x18s)
    print time.format(x3h)
    print time.format(x2m + x18s)
    
    