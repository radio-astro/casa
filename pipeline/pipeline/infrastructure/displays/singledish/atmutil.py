from __future__ import absolute_import

from taskinit import gentools, casac, qa
import math
import numpy
import pylab as pl
import os

import pipeline.extern.adopted as adopted

class AtmType(object):
    tropical = 1
    midLatitudeSummer = 2
    midLatitudeWinter = 3
    subarcticSummer = 4
    subarcticWinter = 5

def init_at(at, humidity=20.0, temperature=270.0, pressure=560.0,
            atmtype=AtmType.midLatitudeWinter, altitude=5000.0,
            fcenter=100.0, nchan=4096, resolution=0.001):
    """
    at: atmosphere tool
    humidity: relative humidity [%]
    temperature: temperature [K]
    pressure: pressure [mbar]
    atmtype: AtmType enum
    altitude: altitude [m]
    fcenter: center frequency [GHz]
    nchan: number of channels
    resolution: channel width [GHz]
    """
    at.initAtmProfile(humidity=humidity,
                        temperature=qa.quantity(temperature, 'K'),
                        altitude=qa.quantity(altitude, 'm'),
                        pressure=qa.quantity(pressure, 'mbar'),
                        atmType=atmtype)
    fwidth = nchan * resolution
    at.initSpectralWindow(nbands=1,
                          fCenter=qa.quantity(fcenter, 'GHz'),
                          fWidth=qa.quantity(fwidth, 'GHz'),
                          fRes=qa.quantity(resolution, 'GHz'))

def calc_airmass(elevation=45.0):
    """
    elevation: elevation [deg]
    """
    return 1.0 / math.cos((90.0 - elevation) * math.pi / 180.) 

def calc_transmission(airmass, dry_opacity, wet_opacity):
    """
    """
    return numpy.exp(-airmass * (dry_opacity + wet_opacity))

def get_dry_opacity(at):
    """
    at: atmosphere tool
    """
    dry_opacity_result = at.getDryOpacitySpec(0)
    dry_opacity = numpy.asarray(dry_opacity_result[1])
    return dry_opacity

def get_wet_opacity(at):
    """
    at: atmosphere tool
    """
    wet_opacity_result = at.getWetOpacitySpec(0)
    wet_opacity = numpy.asarray(wet_opacity_result[1]['value'])
    return wet_opacity

def test(pwv=1.0, elevation=45.0):
    """
    pwv: water vapor content [mm]
    elevation: elevation [deg]
    """
    myat = casac.atmosphere()
    init_at(myat)
    myat.setUserWH2O(qa.quantity(pwv, 'mm'))

    airmass = calc_airmass(elevation)

    dry_opacity = get_dry_opacity(myat)
    wet_opacity = get_wet_opacity(myat)
    transmission = calc_transmission(airmass, dry_opacity, wet_opacity)

    plot(dry_opacity, wet_opacity, transmission)

    return transmission

def plot(frequency, dry_opacity, wet_opacity, transmission):
    
    pl.clf()
    a1 = pl.gcf().gca()
    pl.plot(frequency, dry_opacity, label='dry')
    pl.plot(frequency, wet_opacity, label='wet')
    pl.legend(loc='upper left', bbox_to_anchor=(0., 0.5))
    a2 = a1.twinx()
    a2.yaxis.set_major_formatter(pl.NullFormatter())
    a2.yaxis.set_major_locator(pl.NullLocator())
    pl.gcf().sca(a2)
    pl.plot(frequency, transmission, 'm-')
    M = transmission.min()
    Y = 0.8
    ymin = (M - Y) / (1.0 - Y)
    ymax = transmission.max() + (1.0 - transmission.max()) * 0.1
    pl.ylim([ymin, ymax])
    

def get_spw_spec(vis, spw_id):
    """
    vis: MS name
    spw_id: spw id

    return: center frequency [GHz], number of channels, and resolution [GHz]
    """
    (mytb,) = gentools(['tb'])
    mytb.open(os.path.join(vis, 'SPECTRAL_WINDOW'))
    nrow = mytb.nrows()
    if spw_id < 0 or spw_id >= nrow:
        raise RuntimeError('spw_id {} is out of range'.format(spw_id))
    try:
        nchan = mytb.getcell('NUM_CHAN', spw_id)
        bandwidth = mytb.getcell('TOTAL_BANDWIDTH', spw_id)
        chan_freq = mytb.getcell('CHAN_FREQ', spw_id)
    finally:
        mytb.close()

    center_freq = (chan_freq.min() + chan_freq.max()) / 2.0
    #resolution = bandwidth / nchan
    resolution = chan_freq[1] - chan_freq[0]

    # Hz -> GHz
    toGHz = 1.0e-9
    center_freq *= toGHz
    resolution *= toGHz
    
    return center_freq, nchan, resolution

def get_median_elevation(vis, antenna_id):
    (mytb,) = gentools(['tb'])
    mytb.open(os.path.join(vis, 'POINTING'))
    tsel = mytb.query('ANTENNA_ID == {}'.format(antenna_id))
    # default elevation
    elevation = 45.0
    try:
        if tsel.nrows() > 0:
            colkeywords = tsel.getcolkeywords('DIRECTION')
            if colkeywords['MEASINFO']['Ref'] == 'AZELGEO':
                elevation_list = tsel.getcol('DIRECTION')[1][0]
                elevation = numpy.median(elevation_list) * 180.0 / math.pi
    finally:
        tsel.close()
        mytb.close()

    return elevation        

def get_transmission(vis, antenna_id=0, spw_id=0, doplot=False):
    """
    calculate atmospheric transmission assuming PWV=1mm.
    
    vis -- MS name
    antenna_id -- antenna ID
    spw_id -- spw ID
    
    Returns:
        (frequency array [GHz], atm transmission)
    """
    center_freq, nchan, resolution = get_spw_spec(vis, spw_id)
    elevation = get_median_elevation(vis, antenna_id)

    # set pwv to 1.0 
    #pwv = 1.0
    # get median PWV using Todd's script
    (pwv, pwvmad) = adopted.getMedianPWV(vis=vis)
    
    myat = casac.atmosphere()
    init_at(myat, fcenter=center_freq, nchan=nchan, resolution=resolution)
    myat.setUserWH2O(qa.quantity(pwv, 'mm'))

    airmass = calc_airmass(elevation)

    dry_opacity = get_dry_opacity(myat)
    wet_opacity = get_wet_opacity(myat)
    transmission = calc_transmission(airmass, dry_opacity, wet_opacity)
    #frequency = numpy.fromiter((center_freq + (float(i) - 0.5 * nchan) * resolution for i in xrange(nchan)), dtype=numpy.float64)
    frequency = qa.convert(myat.getSpectralWindow(0), "GHz")['value']
   
    if doplot:
        plot(frequency, dry_opacity, wet_opacity, transmission)

    return frequency, transmission
    
    
