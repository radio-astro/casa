from __future__ import absolute_import

import numpy
import time
import datetime

from matplotlib.dates import date2num, DateFormatter, MinuteLocator
from matplotlib.ticker import FuncFormatter, MultipleLocator

import pipeline.infrastructure.casatools as casatools

sd_polmap = {0: 'XX', 1: 'YY', 2: 'XY', 3: 'YX'}

dsyb = '$^\circ$'
hsyb = ':'
msyb = ':'
RArotation = 90
DECrotation = 0

def Deg2HMS(x, arrowance):
    # Transform degree to HHMMSS.sss format
    xx = x % 360 + arrowance
    h = int(xx / 15)
    m = int((xx % 15) * 4)
    s = ((xx % 15) * 4 - m) * 60.0
    return (h, m, s)

def HHMM(x, pos):
    # HHMM format
    (h, m, s) = Deg2HMS(x, 1/40.0)
    #return '%02dh%02dm' % (h, m)
    return '%02d%s%02d' % (h, hsyb, m)

def HHMMSS(x, pos):
    # HHMMSS format
    (h, m, s) = Deg2HMS(x, 1/2400.0)
    #return '%02dh%02dm%02ds' % (h, m, s)
    return '%02d%s%02d%s%02d' % (h, hsyb, m, msyb, s)

def HHMMSSs(x, pos):
    # HHMMSS.s format
    (h, m, s) = Deg2HMS(x, 1/24000.0)
    #return '%02dh%02dm%04.1fs' % (h, m, s)
    return '%02d%s%02d%s%04.1f' % (h, hsyb, m, msyb, s)

def HHMMSSss(x, pos):
    # HHMMSS.ss format
    (h, m, s) = Deg2HMS(x, 1/240000.0)
    #return '%02dh%02dm%05.2fs' % (h, m, s)
    return '%02d%s%02d%s%05.2f' % (h, hsyb, m, msyb, s)

def HHMMSSsss(x, pos):
    # HHMMSS.sss format
    (h, m, s) = Deg2HMS(x, 1/2400000.0)
    #return '%02dh%02dm%06.3fs' % (h, m, s)
    return '%02d%s%02d%s%06.3f' % (h, hsyb, m, msyb, s)


def Deg2DMS(x, arrowance):
    # Transform degree to +ddmmss.ss format
    xxx = (x + 90) % 180 - 90
    xx = abs(xxx) + arrowance
    if xxx < 0: sign = -1
    else: sign = 1
    d = int(xx * sign)
    m = int((xx % 1) * 60)
    s = ((xx % 1) * 60 - m) * 60.0
    return (d, m, s)

def DDMM(x, pos):
    # +DDMM format
    (d, m, s) = Deg2DMS(x, 1/600.0)
    #return '%+02dd%02dm' % (d, m)
    return '%+02d%s%02d\'' % (d, dsyb, m)

def DDMMSS(x, pos):
    # +DDMMSS format
    (d, m, s) = Deg2DMS(x, 1/36000.0)
    #return '%+02dd%02dm%02ds' % (d, m, s)
    return '%+02d%s%02d\'%02d\"' % (d, dsyb, m, s)

def DDMMSSs(x, pos):
    # +DDMMSS.s format
    (d, m, s) = Deg2DMS(x, 1/360000.0)
    #return '%+02dd%02dm%04.1fs' % (d, m, s)
    sint = int(s)
    sstr = ('%3.1f'%(s-int(s))).lstrip('0')
    return '%+02d%s%02d\'%02d\"%s' % (d, dsyb, m, sint, sstr)

def DDMMSSss(x, pos):
    # +DDMMSS.ss format
    (d, m, s) = Deg2DMS(x, 1/3600000.0)
    #return '%+02dd%02dm%05.2fs' % (d, m, s)
    sint = int(s)
    sstr = ('%4.2f'%(s-int(s))).lstrip('0')
    return '%+02d%s%02d\'%02d\"%s' % (d, dsyb, m, sint, sstr)

def RADEClabel(span):
    """
    return (RAlocator, DEClocator, RAformatter, DECformatter)
    """
    RAtick = [15.0, 5.0, 2.5, 1.25, 1/2.0, 1/4.0, 1/12.0, 1/24.0, 1/48.0, 1/120.0, 1/240.0, 1/480.0, 1/1200.0, 1/2400.0, 1/4800.0, 1/12000.0, 1/24000.0, 1/48000.0, -1.0]
    DECtick = [20.0, 10.0, 5.0, 2.0, 1.0, 1/3.0, 1/6.0, 1/12.0, 1/30.0, 1/60.0, 1/180.0, 1/360.0, 1/720.0, 1/1800.0, 1/3600.0, 1/7200.0, 1/18000.0, 1/36000.0, -1.0]
    for RAt in RAtick:
        if span > (RAt * 3.0) and RAt > 0:
            RAlocator = MultipleLocator(RAt)
            break
    #if RAt < 0: RAlocator = MultipleLocator(1/96000.)
    if RAt < 0: RAlocator = AutoLocator()
    for DECt in DECtick:
        if span > (DECt * 3.0) and DECt > 0:
            DEClocator = MultipleLocator(DECt)
            break
    #if DECt < 0: DEClocator = MultipleLocator(1/72000.0)
    if DECt < 0: DEClocator = AutoLocator()
            
    if span < 0.0001:
        RAformatter=FuncFormatter(HHMMSSsss)
        DECformatter=FuncFormatter(DDMMSSss)
    elif span < 0.001:
        RAformatter=FuncFormatter(HHMMSSss)
        DECformatter=FuncFormatter(DDMMSSs)
    elif span < 0.01:
        RAformatter=FuncFormatter(HHMMSSs)
        DECformatter=FuncFormatter(DDMMSS)
    elif span < 1.0:
        RAformatter=FuncFormatter(HHMMSS)
        #DECformatter=FuncFormatter(DDMM)
        DECformatter=FuncFormatter(DDMMSS)
    else:
        RAformatter=FuncFormatter(HHMM)
        DECformatter=FuncFormatter(DDMM)

    return (RAlocator, DEClocator, RAformatter, DECformatter)

def mjd_to_datedict(val, unit='d'):
    mjd = casatools.quanta.quantity(val,unit)
    return casatools.quanta.splitdate(mjd)

def mjd_to_datetime(val):
    mjd = mjd_to_datedict(val, unit='d')
    date_time = datetime.datetime(mjd['year'], mjd['month'],
                                  mjd['monthday'], mjd['hour'],
                                  mjd['min'], mjd['sec'])
    return date_time

def mjd_to_datestring(val, fmt='%Y/%m/%d'):
    date_time = mjd_to_datetime(val)
    return date_time.strftime(fmt)

# vectorized version
mjd_to_datetime_vectorized = numpy.vectorize(mjd_to_datetime)

def mjd_to_plotval(mjd_list):
    datetime_list = mjd_to_datetime_vectorized(mjd_list)
    return date2num(datetime_list)
    
class CustomDateFormatter(DateFormatter):
    """
    Customized date formatter that puts YYYY/MM/DD under usual
    tick labels at the beginning of tick and when date is changed.
    """
    def __call__(self, x, pos=0):
        fmt_saved = self.fmt
        if pos == 0 or x % 1.0 == 0.0:
            self.fmt = '%H:%M\n%Y/%m/%d'
        tick = DateFormatter.__call__(self, x, pos)
        self.fmt = fmt_saved
        return tick
            
def utc_formatter(fmt='%H:%M'):
    return CustomDateFormatter(fmt)

def utc_locator(start_time=None, end_time=None):
    if start_time is None or end_time is None:
        return MinuteLocator()
    else:
        dt = abs(end_time - start_time) * 1440.0 # day -> minutes
        #print dt
        tick_interval = int(dt/10) + 1
        #print tick_interval
        return MinuteLocator(byminute=range(0,60,tick_interval))
