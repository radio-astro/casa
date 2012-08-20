#!/usr/bin/python -u
#
# bjb
# nrao
# spring 2012
#
# python functions to return expected flux density from solar system
# bodies.  the flux density depends on the geometry (distance, size of
# body, subearth latitude), and on the model brightness temperature.
# uncertainties on the flux density can also be returned, but are all
# set to 0.0 for now.
#
# the model brightness temperatures for the various bodies are taken
# from a combination of modern models and historical observations.  see
# the written description for a more complete description of the model
# for each body.
#
# for all of the bodies but Mars, the model is contained in a text file
# that has tabulated brightness temperature as a function of frequency.
# for Mars it is also a function of time.  eventually, the full-up
# model calculations should be in the code (for those bodies that have
# proper models) but for now, just live with the tabulated versions.
#
# fix for NaN value issue - 2012.05.17

from numpy import searchsorted
from scipy import array
from scipy.interpolate import interp1d
from math import exp, pi, cos, sin,sqrt, isnan
from os import environ, listdir
from taskinit import gentools 

def solar_system_fd (source_name, MJDs, frequencies, casalog=None):
    '''
    find flux density for solar system bodies:
        Venus - Butler et al. 2001
        Mars - Butler et al. 2012
        Jupiter - Orton et al. 2012
        Uranus - Orton & Hofstadter 2012 (modified ESA4)
        Neptune - Orton & Hofstadter 2012 (modified ESA3)
        Io - Moullet et al. 2012
        Europa - Gurwell et al. 2012
        Ganymede - Gurwell et al. 2012
        Titan - Gurwell et al. 2012
        Callisto - Gurwell et al. 2012
        Ceres - ?
        Juno - ?
        Pallas - ?
        Vesta - ?

    inputs:
        source_name = source name string.  example: "Venus"
        MJDs = list of MJD times (day + fraction).  example:
               [ 56018.232, 56018.273 ]
               must be sorted in ascending order.
        frequencies = list of [start, stop] frequencies for
                      which to calculate the integrated model.
                      example:
                      [ [ 224.234567e9, 224.236567e9 ],
                        [ 224.236567e9, 224.238567e9 ] ]

    returned is a list, first element is the return status:
        0 -> success
        1 -> Error: unsupported body
        2 -> Error: unsupported frequency or time for body
        3 -> Error: Tb model file not found
        4 -> Error: ephemeris table not found, or time out of range
             (note - the case where the MJD times span two ephemeris
              files is not supported)
    second element is a list of flux densities, one per time and
        frequency range, frequency changes fastest.
    third element is list of uncertainties (if known; 0 if unknown),
        one per time and frequency range, frequency changes fastest.
    fourth element is a list of major axis, minor axis, and PAs in
        asec and deg, one per MJD time.
    fifth element is a list of CASA directions, one per MJD time.

    NOTE: haven't put in the background term yet - need to subtract
          it from the model before calculating the flux density.

    bjb
    nrao
    spring 2012
    '''
    (tb,me)=gentools(['tb','me'])
    SUPPORTED_BODIES = [ 'Venus', 'Mars', 'Jupiter', 'Uranus', 'Neptune',
                         'Io', 'Europa', 'Ganymede', 'Callisto', 'Titan',
                         'Ceres', 'Juno', 'Pallas', 'Vesta' ]
    capitalized_source_name = source_name.capitalize()
#
# check that body is supported
#
    statuses = []
    fds = []
    dfds = []
    Rhats = []
    directions = []
    if (not capitalized_source_name in SUPPORTED_BODIES):
        for MJD in MJDs:
            estatuses = []
            efds = []
            edfds = []
            for frequency in frequencies:
                estatuses.append(1)
                efds.append(0)
                edfds.append(0)
            statuses.append(estatuses)
            fds.append(efds)
            dfds.append(edfds)
            Rhats.append([0.0, 0.0, 0.0])
            directions.append(me.direction('J2000',0.0,0.0))
        return [ statuses, fds, dfds, Rhats, directions ]

#
# first, calculate model brightness temperatures
#
    if (capitalized_source_name != 'Mars'):
# Mars is special, because the model is a function of time, so it has
# to be done in the loop over time below.  for all other bodies we
# can just calculate the model once.
        Tbs = []
        dTbs = []
        pstatuses = []
        for frequency in frequencies:
# not optimally efficient, as we're calculating the interpolated
# Tb at the edge frequencies twice in the called function (since
# in most cases the frequencies will be contiguous ranges), but
# oh well
            [status,Tb,dTb] = Tb_planet_int (capitalized_source_name, frequency)
            pstatuses.append(status)
            Tbs.append(Tb)
            dTbs.append(dTb)
#
# before calculating the model for mars, since it is time-dependent,
# be sure that we have the ephemeris information.  otherwise don't
# waste our time calculating the model.
#
    sorted_MJDs = sorted (MJDs)
    ephemeris_path = environ['CASAPATH'].split()[0]+'/data/ephemerides/JPL-Horizons/'
    file_list = listdir(ephemeris_path)
    files = []
    for file in file_list:
        if file.split('_')[0] == capitalized_source_name:
            files.append(file)
    file_OK = 0
# ephemeris tables have the following keywords:
# GeoDist, GeoLat, GeoLong, MJD0, NAME, VS_CREATE, VS_DATE, VS_TYPE,
# VS_VERSION, dMJD, earliest, latest, meanrad, obsloc, radii, rot_per
# and columns:
# RA, DEC, Rho (geodist), r (heliodist), phang, RadVel, illu, NP_DEC, NP_RA
    for file in files:
        ephemeris_file = ephemeris_path + file
        tb.open(ephemeris_file)
        table_source_name = tb.getkeyword('NAME').capitalize()
        if (table_source_name != capitalized_source_name):
            continue
        first_time = tb.getkeyword('earliest')['m0']['value']
        last_time = tb.getkeyword('latest')['m0']['value']
        if (first_time < sorted_MJDs[0] and last_time > sorted_MJDs[-1]):
            file_OK = 1
            break
        tb.close()
    if (file_OK == 0):
        for MJD in MJDs:
            estatuses = []
            efds = []
            edfds = []
            for frequency in frequencies:
                estatuses.append(4)
                efds.append(0)
                edfds.append(0)
            statuses.append(estatuses)
            fds.append(efds)
            dfds.append(edfds)
            Rhats.append([0.0, 0.0, 0.0])
            directions.append(me.direction('J2000',0.0,0.0))
        return [ statuses, fds, dfds, Rhats, directions ]

    keys=tb.getkeywords()
    if keys.has_key('radii'):
       Req = 1000.0 * (tb.getkeyword('radii')['value'][0] + tb.getkeyword('radii')['value'][1]) / 2
       Rp = 1000.0 * tb.getkeyword('radii')['value'][2]
    else:
       if keys.has_key('meanrad'):
           Req = 1000.0 * tb.getkeyword('meanrad')['value']
           Rp = Req     
           casalog.post("No triaxial radii in the ephemeris table, using the mean radius instead", "INFO")
       else:
           casalog.post("Cannot find the radius of "+source_name+" in the ephemeris table","SEVERE")
    distances = tb.getcol('Rho').tolist()
    times = tb.getcol('MJD').tolist()
    RAs = tb.getcol('RA').tolist()
    DECs = tb.getcol('DEC').tolist()
    selats = []
    NPangs = []
    fds = []
    dfds = []
    RAD2ASEC = 2.0626480624710e5
    #print "get ephemris info"
# the ephemeris information is there, so go ahead and calculate the model
    if (capitalized_source_name == 'Mars'):
        [mstatuses,Tbs,dTbs] = Tb_Mars_int (sorted_MJDs, frequencies)
    for ii in range(len(MJDs)):
        MJD = MJDs[ii]
        RA = interpolate_list (times, RAs, MJD)[1]
        RAstr=str(RA)+'deg'
        DEC = interpolate_list (times, DECs, MJD)[1]
        DECstr=str(DEC)+'deg'
        directions.append(me.direction('J2000',RAstr,DECstr))
        DD = 1.4959787066e11 * interpolate_list (times, distances, MJD)[1]
# there is no subearth latitude in the ephemeris tables currently, so punt
        selats.append(0.0)
# there is no NP angle in the ephemeris tables currently, so punt
        NPangs.append(0.0)
# apparent polar radius
        Rpap = sqrt (Req*Req * sin(selats[-1])**2.0 +
                     Rp*Rp * cos(selats[-1])**2.0)
        Rmean = sqrt (Rpap * Req)
# sure wish we had proper subearth lats and NP angles.  sigh.
# when we have proper NP angles, be sure the convention is the
# same as what is needed in the component list.
        Rhats.append([2*RAD2ASEC*Req/DD, 2*RAD2ASEC*Rpap/DD, NPangs[-1]])
        #print "Rpap=",Rpap," Rmean=",Rmean," Rhats=",Rhats
        estatuses = []
        efds = []
        edfds = []
        for jj in range(len(frequencies)):
            if (capitalized_source_name != 'Mars'):
                estatuses.append(pstatuses[jj])
                Tb = Tbs[jj]
                dTb = dTbs[jj]
            else:
                estatuses.append(mstatuses[ii][jj])
                Tb = Tbs[ii][jj]
                dTb = dTbs[ii][jj]
            if (estatuses[-1] == 0):
                #casalog.post('Tb=%s for Freq=%s' % (Tb,frequencies[jj]))
                flux_density = Tb * 1.0e26 * pi * Req * Rpap / (DD*DD)
# mean apparent planet radius, in arcseconds (used if we ever
# calculate the primary beam reduction)
                psize = (Rmean / DD) * 2.0626480624710e5
# primary beam reduction factor (should call a function, but
# just set to 1.0 for now...
                pbfactor = 1.0
                flux_density *= pbfactor
                efds.append(flux_density)
                edfds.append(0.0)
            else:
                efds.append(0.0)
                edfds.append(0.0)
        statuses.append(estatuses)
        fds.append(efds)
        dfds.append(edfds)
    return [ statuses, fds, dfds, Rhats, directions ]


def Tb_Mars_int (MJDs, frequencies):
    '''
    brightness temperature for Mars.  this one is different because
    the model is tabulated vs. frequency *and* time.
    inputs:
        MJDs = list of MJD times
        frequencies = list of [start, stop] frequencies for
                    which to calculate the integrated model.
                    example: [ [ 224.234567e9, 224.236567e9 ] ]
    '''

#
# constants.  these might be in CASA internally somewhere, but
# i don't know where to pull them out, so oh well, define my own
#
    HH = 6.6260755e-34
    KK = 1.380658e-23
    CC = 2.99792458e8

    statuses = []
    Tbs = []
    dTbs = []
    try:
        # path for the Mars model
        #ff = open('/users/bbutler/python/Mars_Tb.dat')
        ff = open(environ['CASAPATH'].split()[0]+'/data/alma/SolarSystemModels/Mars_Tb.dat')
    except:
        for MJD in MJDs:
            estatuses = []
            eTbs = []
            eTbds = []
            for frequency in frequencies:
                estatuses.append(3)
                eTbs.append(0)
                edTbs.append(0)
            statuses.append(estatuses)
            Tbs.append(eTbs)
            dTbs.append(edTbs)
        return [ statuses, Tbs, dTbs ]

# first line holds frequencies, like:
# 30.0 80.0 115.0 150.0 200.0 230.0 260.0 300.0 330.0 360.0 425.0 650.0 800.0 950.0 1000.0
    line = ff.readline()[:-1]
    fields = line.split()
    freqs = []
    for field in fields:
        freqs.append(1.0e9*float(field))
# model output lines look like:
#2010 01 01 00  55197.00000 189.2 195.8 198.9 201.2 203.7 204.9 205.9 207.1 207.8 208.5 209.8 213.0 214.6 214.8 214.5 
    modelMJDs = []
    modelTbs = []
    for line in ff:
        fields = line[:-1].split()
        modelMJDs.append(float(fields[4]))
        lTbs = []
        for ii in range(len(freqs)):
            lTbs.append(float(fields[5+ii]))
        modelTbs.append(lTbs)
    ff.close()
    for MJD in MJDs:
        nind = nearest_index (modelMJDs, MJD)
        mTbs = []
        mfds = []
        for ii in range(len(freqs)):
            lMJD = []
            lTb = []
            for jj in range(nind-10, nind+10):
                lMJD.append(modelMJDs[jj])
                lTb.append(modelTbs[jj][ii])
        # background subtraction from model requested change
        # from Bryan - 2012.05.17
	#== old code
        #    mTbs.append(interpolate_list(lMJD, lTb, MJD)[1])
        #    mfds.append((2.0 * HH * freqs[ii]**3.0 / CC**2.0) * \
        #                (1.0 / (exp(HH * freqs[ii] / (KK * mTbs[-1])) - 1.0)))
	    mTbs.append(interpolate_list(lMJD, lTb, MJD)[1])
        # note: here, when we have the planck results, get a proper
        # estimate of the background temperature.
	    Tbg = 2.72
	    mfds.append((2.0 * HH * freqs[ii]**3.0 / CC**2.0) * \
	           ((1.0 / (exp(HH * freqs[ii] / (KK * mTbs[-1])) - 1.0)) - \
		   (1.0 / (exp(HH * freqs[ii] / (KK * Tbg)) - 1.0))))
        estatuses = []
        eTbs = []
        edTbs = []
        for frequency in frequencies:
            if (frequency[0] < freqs[0] or frequency[1] > freqs[-1]):
                estatuses.append(2)
                eTbs.append(0.0)
                edTbs.append(0.0)
            else:
                [estatus, eTb, edTb] = integrate_Tb (freqs, mfds, frequency)
                estatuses.append(estatus)
                eTbs.append(eTb)
                edTbs.append(edTb)
        statuses.append(estatuses)
        Tbs.append(eTbs)
        dTbs.append(edTbs)
    return [statuses, Tbs, dTbs ]


def Tb_planet_int (source_name, frequency):
    '''
    brightness temperature for supported planets.  integrates over
    a tabulated model.  inputs:
        source_name = source name string
        frequency = list of [start, stop] frequencies for
                    which to calculate the integrated model.
                    example: [ 224.234567e9, 224.236567e9 ]
    '''

#
# constants.  these might be in CASA internally somewhere, but
# i don't know where to pull them out, so oh well, define my own
#
    HH = 6.6260755e-34
    KK = 1.380658e-23
    CC = 2.99792458e8

    try:
        #ff = open('/users/bbutler/python/' + source_name + '_Tb.dat')
        ff = open(environ['CASAPATH'].split()[0]+'/data/alma/SolarSystemModels/' + source_name + '_Tb.dat')
    except:
        return [ 3, 0.0, 0.0 ]
    fds = []
    Tbs = []
    freqs = []
    for line in ff:
        [freq,Tb] = line[:-1].split()
        #Tbs.append(float(Tb))
        #freqs.append(1.0e9*float(freq))
        #fds.append((2.0 * HH * freqs[-1]**3.0 / CC**2.0) * \
        #            (1.0 / (exp(HH * freqs[-1] / (KK * Tbs[-1])) - 1.0)))
        Tbs.append(float(Tb))
	freqs.append(1.0e9*float(freq))
    # note: here, when we have the planck results, get a proper
    # estimate of the background temperature.
        Tbg = 2.72
	fds.append((2.0 * HH * freqs[-1]**3.0 / CC**2.0) * \
	     ((1.0 / (exp(HH * freqs[-1] / (KK * Tbs[-1])) - 1.0)) - \
             (1.0 / (exp(HH * freqs[-1] / (KK * Tbg)) - 1.0))))
    ff.close()
    #print "next freqs=",freqs, " frequency=",frequency
    if (frequency[0] < freqs[0] or frequency[1] > freqs[-1]):
        return [ 2, 0.0, 0.0 ]
    else:
        return integrate_Tb (freqs, fds, frequency)


def nearest_index (input_list, value):
    """
    find the index of the list input_list that is closest to value
    """

    ind = searchsorted(input_list, value)
    ind = min(len(input_list)-1, ind)
    ind = max(1, ind)
    if value < (input_list[ind-1] + input_list[ind]) / 2.0:
        ind = ind - 1
    return ind


def interpolate_list (freqs, Tbs, frequency):
    ind = nearest_index (freqs, frequency)
    low = max(0,ind-5)
    if (low == 0):
        high = 11
    else:
        high = min(len(freqs),ind+6)
        if (high == len(freqs)):
            low = high - 11
    aTbs = array(Tbs[low:high])
    afreqs = array(freqs[low:high])
    func = interp1d (afreqs, aTbs, kind='cubic')
    if isnan(func(frequency)):
        func = interp1d(afreqs, aTbs, kind='linear')
    return [ 0, float(func(frequency)), 0.0 ]


def integrate_Tb (freqs, Tbs, frequency):
    [status,low_Tb,low_dTb] = interpolate_list (freqs, Tbs, frequency[0])
    low_index = nearest_index (freqs, frequency[0])
    if (frequency[0] > freqs[low_index]):
        low_index = low_index + 1

    [status,hi_Tb,hi_dTb] = interpolate_list (freqs, Tbs, frequency[1])
    hi_index = nearest_index (freqs, frequency[1])
    if (frequency[1] < freqs[hi_index]):
        hi_index = hi_index - 1

    if (low_index > hi_index):
        Tb = (frequency[1] - frequency[0]) * (low_Tb + hi_Tb) / 2
    else:
        Tb = (freqs[low_index] - frequency[0]) * (low_Tb + Tbs[low_index]) / 2 + \
             (frequency[1] - freqs[hi_index]) * (hi_Tb + Tbs[hi_index]) / 2
        ii = low_index
        while (ii < hi_index):
           Tb += (freqs[ii+1] - freqs[ii]) * (Tbs[ii+1] + Tbs[ii]) / 2
           ii+=1
    Tb /= (frequency[1] - frequency[0])
    return [ 0, Tb, 0.0 ]

