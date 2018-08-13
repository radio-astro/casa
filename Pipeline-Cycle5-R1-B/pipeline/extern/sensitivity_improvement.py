#######################################################################
# The code adopted from analysisUtil.py to estimate sensitivity of 
# images in PI resolution.
#
# Date of adoption: 2017/06/22
# CVS revision: 1.3692
# 
# Adopted methods
# * sensitivityImprovement
# * windowFunction
# * onlineChannelAveraging
# 
# Note of modifications for PIPELINE
# * handlings of old CASA versions are removed
# * surmise of nominal effective BW in < Cycle 3 EBs is removed
# * ways to generate of tool instanses are adjusted for PL.
# * print messages to logger instead of STDOUT
#######################################################################
import os
import scipy
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)

c_mks=2.99792458e8

########## Estimate Sensitivity ##########
def sensitivityImprovement(vis, spw, newchanwidth, useCAS8534=True, 
                           window='hanning', cubechanwidth=None):
    """
    Computes the expected factor of improvement in sensitivity expected when 
    making images of ALMA data with channel widths wider than the observed 
    width.
    vis: name of measurement set
    spw: single spw index (integer)
    newchanwidth: width of image channel in units of the uvdata channel width, 
        or a string with any common frequency units ('MHz' etc.) or 'km/s'
    useCAS8534: if True, use the approximate formula developed for the imaging 
                  pipeline;
         if False, use a spline fit to the integral table of values (1,2,4,8,16)
    cubechanwidth: if specified, then compute the improvement with respect to
         this chanwidth (which might be larger than the observed chanwidth).
         Units: Width of image channel in units of the uvdata channel width, 
         or a string with any common frequency units ('MHz' etc.) or 'km/s'
    -Todd Hunter
    """
    mymsmd = casatools.msmd
    mymsmd.open(vis)
    N = onlineChannelAveraging(vis, spw, mymsmd)
    chanwidth = numpy.abs(mymsmd.chanwidths(spw)[0])  # Hz
    LOG.info("online channel averaging: %d, chanwidth = %g" % (N,chanwidth))
    spwchan = mymsmd.nchan(spw)
    meanfreq = mymsmd.meanfreq(spw)
    mymsmd.close()
    myqa = casatools.quanta
    if type(newchanwidth) == str:
        if (newchanwidth.lower().find('km/s') > 0):
            velocity = float(newchanwidth.lower().replace('km/s',''))
            newchanwidth = 1000*velocity*meanfreq/c_mks
        else:  # frequency units
            newchanwidth = myqa.getvalue(myqa.convert(newchanwidth, "Hz"))[0]
    else:
        newchanwidth *= chanwidth  # convert to Hz
    if newchanwidth < chanwidth:
        LOG.warn("You are requesting a narrower channel width (%f MHz) than the original data have (%f MHz)." % (newchanwidth*1e-6, chanwidth*1e-6))
        return
    PI_request_channels = newchanwidth / chanwidth
    LOG.info("requested channel averaging = %f" % PI_request_channels)
    if cubechanwidth is None:
        originalEffBW = windowFunction(window, channelAveraging=N, returnValue='EffectiveBW')
    else:
        if type(cubechanwidth) == str:
            if (cubechanwidth.lower().find('km/s') > 0):
                velocity = float(cubechanwidth.lower().replace('km/s',''))
                cubechanwidth = 1000*velocity*meanfreq/c_mks
            else:  # frequency units
                cubechanwidth = myqa.getvalue(myqa.convert(cubechanwidth, "Hz"))[0]
        else:
            cubechanwidth *= chanwidth  # convert to Hz
        if cubechanwidth < chanwidth:
            LOG.warn("You are using a cube with a narrower channel width (%f MHz) than the original data have (%f MHz)." % (cubechanwidth*1e-6, chanwidth*1e-6))
            return
        originalEffBW = windowFunction(window, channelAveraging=N*float(cubechanwidth)/chanwidth, returnValue='EffectiveBW',useCAS8534=True)
    if useCAS8534:
        newEffBW = windowFunction(window, channelAveraging=N, returnValue='EffectiveBW',useCAS8534=useCAS8534,spwchan=spwchan, nchan=PI_request_channels)
    else:
        newEffBW = windowFunction(window, channelAveraging=N*PI_request_channels, returnValue='EffectiveBW')
    improvement = numpy.sqrt(newEffBW  / originalEffBW) 
    return improvement

########## Window Function ##########
def windowFunction(window='', channelAveraging=1, returnValue='FWHM', 
                   splineDegree=3, splineSmoothing=None, ratio=False, 
                   useCAS8534=False, nchan=1, spwchan=128):
    """
    Print the FWHM and Effective sensitivity bandwidth of each of the ALMA correlator
    window functions, or return the value for a specific choice. The values are taken
    from the tables in Richard Hills' note of April 8, 2012.
    window: one of ['', 'uniform','hanning','welch','cosine','hamming','bartlett',
                    'blackmann','blackmann-harris']  ''=prints the whole table
    channelAveraging: 1, 2, 4, 8, or 16;  >16 will return channelAveraging, other values
                      will be spline interpolated
    returnValue: 'FWHM' or 'EffectiveBW' or 'dictionary'
    splineDegree: passed as the k parameter to scipy.interpolate.UnivariateSpline
    splineSmoothing: passed as the s parameter to scipy.interpolate.UnivariateSpline
    ratio: if True, then divide by the channelAveraging factor
    useCAS8534: uses the approximate formula developed for the pipeline rather than a spline
    spwchan: number of channels in spw (only used if useCAS8534=True)
    nchan: number of channels being combined (only used if useCAS8534=True)
    Returns:
    The effective number of native channels supplied by a single channel.
    -Todd Hunter
    """
    mydict = {'uniform': {'FWHM': {1: 1.207, 2: 1.639, 4: 4.063, 8: 8.033, 16: 16.017},
                          'EffectiveBW': {1: 1.0, 2: 2.0, 4: 4.0, 8: 8.0, 16: 16.0}},
              'hanning': {'FWHM': {1: 2.000, 2: 2.312, 4: 3.970, 8: 7.996, 16: 15.999},
                          'EffectiveBW':{1: 2.667, 2: 3.200, 4: 4.923, 8: 8.828, 16: 16.787}},
              'welch': {'FWHM': {1: 1.590, 2: 1.952, 4: 4.007, 8: 8.001, 16: 16.0},
                        'EffectiveBW':{1: 1.875, 2: 2.565, 4: 4.499, 8: 8.470, 16: 16.457}},
              'cosine': {'FWHM': {1: 1.639, 2: 2.000, 4: 4.000, 8: 8.000, 16: 16.0},
                         'EffectiveBW':{1: 2.000, 2: 2.667, 4: 4.571, 8: 8.533, 16: 16.561}},
              'hamming': {'FWHM': {1: 1.815}, 'EffectiveBW': {1: 2.516}},
              'bartlett': {'FWHM': {1: 1.772}, 'EffectiveBW': {1: 3.000}},
              'blackmann': {'FWHM': {1: 2.299}, 'EffectiveBW': {1: 3.283}},
              'blackmann-harris': {'FWHM': {1: 2.666}, 'EffectiveBW': {1: 3.877}}
              }
    window = window.lower()
    if (window in mydict.keys()):
        if (returnValue == 'dictionary'):
            return mydict[window]
        if (channelAveraging not in mydict[window]['FWHM'].keys() or useCAS8534):
            if (channelAveraging < 1 or window not in ['uniform','hanning','welch','cosine']):
                LOG.error("Invalid choice of channelAveraging")
                return
            if useCAS8534:
                approximateEffectiveBW = nchan + 1.12*(spwchan-nchan)/float(spwchan)/channelAveraging
                approximateEffectiveBW *= channelAveraging
                if ratio:
                    return approximateEffectiveBW/channelAveraging
                else:
                    return approximateEffectiveBW
            elif channelAveraging < 17:
                LOG.info("Using spline fit (set useCAS8534=True to use the formula in CAS-8534)")
                if channelAveraging > 8:
                    a = 8; b = 16
                elif channelAveraging > 4:
                    a = 4; b = 8
                elif channelAveraging > 2:
                    a = 2; b = 4
                elif channelAveraging > 1:
                    a = 1; b = 2
                myspline = scipy.interpolate.UnivariateSpline(mydict[window][returnValue].keys(),
                                                              mydict[window][returnValue].values(), 
                                                              k=splineDegree, 
                                                              s=splineSmoothing)
                factor = float(myspline(channelAveraging))
                if abs(channelAveraging - numpy.round(channelAveraging)) >= 0.1:
                    LOG.warn("Warning: spline-interpolating computed table between %d and %d (for %.1f channels)" % (a,b,channelAveraging))
                if ratio:
                    return factor/channelAveraging
                else:
                    return factor
            else:
                return channelAveraging
        if (returnValue not in mydict[window].keys()):
            LOG.error("Invalid choice of returnValue")
            return
        if ratio:
            return(mydict[window][returnValue][channelAveraging]/channelAveraging)
        else:
            return(mydict[window][returnValue][channelAveraging])
    else:
        if (returnValue == 'dictionary'):
            return mydict
        LOG.warn("No window function specified.  All will be displayed")
        LOG.warn("                           in input channels   in output channels")
        LOG.info("%16s  ChanAvg  FWHM  EffectiveBW   FWHM  EffectiveBW" % ('Window type'))
        for key in ['uniform','hanning','welch','cosine','hamming','bartlett',
                    'blackmann','blackmann-harris']:
            for chanAvg in sorted(mydict[key]['FWHM'].keys()):
                LOG.info("%16s     %d    %.3f   %.3f       %.3f    %.3f" % (key, chanAvg, mydict[key]['FWHM'][chanAvg],
                     mydict[key]['EffectiveBW'][chanAvg], mydict[key]['FWHM'][chanAvg]/chanAvg,
                     mydict[key]['EffectiveBW'][chanAvg]/chanAvg))

########## Obtain channel averaging by online system for data >= Cycle 3 ##########
def onlineChannelAveraging(vis, spw, mymsmd=''):
    """
    For Cycle 3-onward data, determines the channel averaging factor from
    the ratio of the effective channel bandwidth to the channel width.
    -Todd Hunter
    """
    if (not os.path.exists(vis)):
        print "Could not find measurement set."
        return
    effBW = windowFunction('hanning', returnValue='dictionary')['EffectiveBW']
    closeTool = False
    if mymsmd == '':
        mymsmd = casatools.msmd
        mymsmd.open(vis)
        closeTool = True
    chanwidths = mymsmd.chanwidths(spw)
    nchan = len(chanwidths)
    if (nchan < 5):
        if closeTool: mymsmd.close()
        return 1
    cqa = casatools.quanta
    chanwidth = abs(chanwidths[0])
    chaneffwidth = mymsmd.chaneffbws(spw)[0]
    
    # warning of nominal effective bandwidth for < Cycle 3
    start_time = mymsmd.timerangeforobs(0)['begin']
    if (cqa.time(start_time['m0'], 0, ['ymd', 'no_time'])[0] < "2015-10-01" and chanwidth== chaneffwidth):
        LOG.warn("This is pre-Cycle 3 data, thus the averaging is not definitely known. I will instead use 1")
        return 1

    if closeTool:
        mymsmd.close()
    Ns = []
    ratios = []
    for N in effBW.keys():
        ratios.append(effBW[N] / N)
        Ns.append(N)
    ratio = chaneffwidth/chanwidth
    ratios = numpy.array(ratios)
    return Ns[numpy.argmin(abs(ratios - ratio))]

