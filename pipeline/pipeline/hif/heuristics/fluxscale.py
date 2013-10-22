import math
import numpy as np
import scipy.special as scipy

import solar_system_setjy as ss_setjy

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure as infrastructure

LOG = infrastructure.get_logger(__name__)


def antenna(ms, refsource, refant, peak_frac=0.7):
    """Finds the size of the reference object and the baseline length at 
    which its Fourier transform drops to 80% of peak. Then find antennas
    within this distance of the reference antenna and return them
    as a list, such that gaincals will only use baselines between them.
    refsource -- Name of reference source.
    refant    -- Name of reference antenna.
    peak_frac -- Fraction of refsource peak visibility at which to set 
                 uv distance limit for antennas.
    """

    # find the reference calibrator size
    obs_time = casatools.quanta.quantity(
      '%s%s' % (ms.start_time['m0']['value'], ms.start_time['m0']['unit']))
    # make sure its in MJD
    obs_time = casatools.quanta.convert(obs_time, 'd')
    try:
        # frequencies are set to dummy values, we are only
        # interested in the object size
        # source_name must be a name, not an ID
        if refsource.isdigit():
            fields = ms.get_fields()
            name = [f.name for f in fields if f.id==int(refsource)]
            name = name[0]
        else:
            name = refsource

        rtn = ss_setjy.solar_system_setjy().solar_system_fd(
          source_name=name,
          MJDs=[obs_time['value']], frequencies=[[1.e9,1.1e9]],
          observatory='ALMA', casalog=casatools.casalog)
        calibrator_size = max(rtn[3][0][:2])
    except:
        raise
        calibrator_size = None

    if calibrator_size is None or calibrator_size==0.0:
        # assume calibrator unresolved, set antenna='' so that all antennas
        # will be used in gaincals
        antenna = ''
        LOG.info('calibrator is unresolved')
    else:
        antenna = set()
        LOG.info('calibrator:%s size:%s' % (refsource, calibrator_size))

        # the pattern detected by the interferometer is the Fourier transform
        # of the source. If the source is a circle with radius s (radians),
        # then the transform is J1(kus/2)/(kus/2), where k=2pi/lambda, u is
        # the uv distance. The function falls to 80% of peak at roughly
        # kus/2 = 1.3.

        # get lowest freq/ highest lambda in observation
        spws = ms.get_spectral_windows(science_windows_only=True)
        ref_frequencys = [spw.centre_frequency.value for spw in spws]
        min_freq = min(ref_frequencys)
        LOG.info('minimum frequency: %s' % min_freq)
        max_lambda = 3.0e8 / float(min_freq)

        # get x, y pos of antennas on Earth's surface
        antennas = ms.antennas
        x = {}
        y = {}
        for ant in antennas:
            name = ant.name
            long = ant.longitude['value']
            lat = ant.latitude['value']
            height = ant.height['value']
            x[name] = long * math.cos(lat) * height
            y[name] = lat * height

        # get shortest baseline to reference antenna
        shortest_baseline = 10000.0
        for ant in antennas:
            name = ant.name
            if name == refant:
                continue
            baseline = math.sqrt(pow(x[name] - x[refant], 2) +
              pow(y[name] - y[refant], 2))
            shortest_baseline = min(shortest_baseline, baseline)

        # get the value of the vis pattern at that baseline
        arg = math.pi * shortest_baseline * calibrator_size / (
          206265.0 * max_lambda)
        peak_vis = scipy.jn(1, arg) / arg

        # crudely find baseline at which transform drops to 'peak_frac' of
        # peak
        limit_baseline = 10000.0 + shortest_baseline
        for baseline in np.arange(10000) + shortest_baseline:
            arg = math.pi * baseline * calibrator_size / (
              206265.0 * max_lambda)
            if scipy.jn(1,arg) / arg < peak_frac * peak_vis:
                limit_baseline = baseline
                break

        LOG.info('maximum baseline: %s' % limit_baseline)

        # select antennas within limit_baseline of reference antenna
        for ant in antennas:
            name = ant.name
            if math.sqrt(pow(x[name] - x[refant], 2) +
              pow(y[name] - y[refant], 2)) < limit_baseline:
                antenna.update([name])

        if len(antenna) < 3:
            LOG.warning('fewer than 3 antennas within max baseline, revert to using all antennas')
            antenna = ''
        else:
            # the antennas to be used in the gaincals, suffixed with
            # & to tell CASA tasks to only use baseline between antennas
            # in the list
            antenna = ','.join(ant for ant in antenna)
            antenna += '&'
            LOG.info('antenna selection: %s' % antenna)

    return antenna
