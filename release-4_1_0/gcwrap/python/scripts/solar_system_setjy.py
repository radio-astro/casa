#!/usr/bin/python -u
#
# bryan butler
# nrao
# spring 2012
#
# python functions to return expected flux density from solar system
# bodies.  the flux density depends on the geometry (distance, size of
# body, subearth latitude), and on the model brightness temperature.
# uncertainties on the flux density can also be returned, but are all
# set to 0.0 for now, because i don't have uncertainties on the model
# brightness temperatures.
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
# version 1.1
# last edited: 2012Oct03
# Modified by TT to avoid uncessary file open: 2012Dec13


from numpy import searchsorted
from scipy import array
from scipy.interpolate import interp1d
from math import exp, pi, cos, sin, isnan, sqrt
#from os import environ, listdir
import os
from taskinit import gentools
(tb,me)=gentools(['tb','me'])
from casac import *
qa = casac.quanta()
HH = qa.constants('H')['value'] 
KK = qa.constants('K')['value']
CC = qa.constants('C')['value'] 

class solar_system_setjy:
    def __init__(self):
        self.models={}
    

    def solar_system_fd (self,source_name, MJDs, frequencies, observatory, casalog=None):
	'''
	find flux density for solar system bodies:
	    Venus - Butler et al. 2001
	    Mars - Butler et al. 2012
	    Jupiter - Orton et al. 2012
	    Uranus - Orton & Hofstadter 2012 (modified ESA4)
	    Neptune - Orton & Hofstadter 2012 (modified ESA3)
	    Io - Butler et al. 2012
	    Europa - Butler et al. 2012
	    Ganymede - Butler et al. 2012
	    Titan - Gurwell et al. 2012
	    Callisto - Butler et al. 2012
	    Ceres - Keihm et al. 2012
	    Juno - ?
	    Pallas - Keihm et al. 2012
	    Vesta - Keihm et al. 2012
	    Hygeia - Keihm et al. 2012

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
	    observatory = observatory name string.  example: "ALMA"

	returned is a list, first element is the return status:
	    0 -> success
	    1 -> Error: unsupported body
	    2 -> Error: unsupported frequency or time for body
	    3 -> Error: Tb model file not found
	    4 -> Error: ephemeris table not found, or time out of range
		 (note - the case where the MJD times span two ephemeris
		  files is not supported)
	    5 -> Error: unknown observatory
	second element is a list of flux densities, one per time and
	    frequency range, frequency changes fastest.
	third element is list of uncertainties (if known; 0 if unknown),
	    one per time and frequency range, frequency changes fastest.
	fourth element is a list of major axis, minor axis, and PAs in
	    asec and deg, one per MJD time.
	fifth element is a list of CASA directions, one per MJD time.

	bjb
	nrao
	spring/summer/fall 2012
	'''

	RAD2ASEC = 2.0626480624710e5
	AU = 1.4959787066e11
	SUPPORTED_BODIES = [ 'Venus', 'Mars', 'Jupiter', 'Uranus', 'Neptune',
			     'Io', 'Europa', 'Ganymede', 'Callisto', 'Titan',
			     'Ceres', 'Juno', 'Pallas', 'Vesta', 'Hygeia' ]

	capitalized_source_name = source_name.capitalize()
	statuses = []
	fds = []
	dfds = []
	Rhats = []
	directions = []

    #
    # check that body is supported
    #
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
    # check that observatory is known
    #
	if not observatory in me.obslist():
	    for MJD in MJDs:
		estatuses = []
		efds = []
		edfds = []
		for frequency in frequencies:
		    estatuses.append(5)
		    efds.append(0)
		    edfds.append(0)
		statuses.append(estatuses)
		fds.append(efds)
		dfds.append(edfds)
		Rhats.append([0.0, 0.0, 0.0])
		directions.append(me.direction('J2000',0.0,0.0))
	    return [ statuses, fds, dfds, Rhats, directions ]

    #
    # before calculating the models be sure that we have the ephemeris 
    # information.  otherwise don't waste our time calculating the model.
    # only really important for mars, but do it for them all.
    #
	ephemeris_path = os.environ['CASAPATH'].split()[0]+'/data/ephemerides/JPL-Horizons/'
    #
    # for testing only...
    #
    #    ephemeris_path = '/home/rishi/ttsutsum/casatest/fluxCal/ephem/'
	file_list = os.listdir(ephemeris_path)
	files = []
	for efile in file_list:
	    if (efile.split('_')[0] == capitalized_source_name and 'J2000' in efile):
		files.append(efile)
	file_OK = 0
    #
    # ephemeris tables have the following keywords:
    # GeoDist, GeoLat, GeoLong, MJD0, NAME, VS_CREATE, VS_DATE, VS_TYPE,
    # VS_VERSION, dMJD, earliest, latest, meanrad, obsloc, radii, rot_per
    # and columns:
    # MJD, RA, DEC, Rho (geodist), RadVel, NP_ang, NP_dist, DiskLong (Ob-long),
    # DiskLat(Ob-lat), Sl_lon, Sl_lat, r (heliodist), rdot, phang
    # Note by TT:
    # The column names, Obs_lon and Obs_lat have been changed to DiskLong and
    # DiskLat respectively to be consistent with what column names assumed for
    # ephemeris tables by casacore's MeasComet class.

	for efile in files:
	    ephemeris_file = ephemeris_path + efile
	    tb.open(ephemeris_file)
	    table_source_name = tb.getkeyword('NAME').capitalize()
	    if (table_source_name != capitalized_source_name):
		continue
	    first_time = tb.getkeyword('earliest')['m0']['value']
	    last_time = tb.getkeyword('latest')['m0']['value']
	    if (first_time < MJDs[0] and last_time > MJDs[-1]):
		file_OK = 1
		break
	    tb.close()
    #
    # if we didn't find an ephemeris file, set the statuses and return.
    #
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

	Req = 1000.0 * (tb.getkeyword('radii')['value'][0] + tb.getkeyword('radii')['value'][1]) / 2
	Rp = 1000.0 * tb.getkeyword('radii')['value'][2]
	times = tb.getcol('MJD').tolist()
	RAs = tb.getcol('RA').tolist()
	DECs = tb.getcol('DEC').tolist()
	distances = tb.getcol('Rho').tolist()
	RadVels = tb.getcol('RadVel').tolist()
	column_names = tb.colnames()
	if ('DiskLat' in column_names):
	    selats = tb.getcol('DiskLat').tolist()
	    has_selats = 1
	else:
	    has_selats = 0
	    selat = 0.0
	if ('NP_ang' in column_names):
	    NPangs = tb.getcol('NP_ang').tolist()
	    has_NPangs= 1
	else:
	    has_NPangs = 0
	    NPang = 0.0
	tb.close()
	MJD_shifted_frequencies = []
	DDs = []
	Rmeans = []

	for ii in range(len(MJDs)):
	    MJD = MJDs[ii]
	    DDs.append(1.4959787066e11 * self.interpolate_list (times, distances, MJD)[1])
	    if (has_selats):
		selat = self.interpolate_list (times, selats, MJD)[1]
		if (selat == -999.0):
		    selat = 0.0
    # apparent polar radius
	    Rpap = sqrt (Req*Req * sin(selat)**2.0 +
			 Rp*Rp * cos(selat)**2.0)
	    Rmean = sqrt (Rpap * Req)
	    Rmeans.append(Rmean)
    #
    # need to check that the convention for NP angle is the
    # same as what is needed in the component list.
    #
	    if (has_NPangs):
		NPang = self.interpolate_list (times, NPangs, MJD)[1]
		if (NPang == -999.0):
		    NPang = 0.0
	    Rhats.append([2*RAD2ASEC*Req/DDs[-1], 2*RAD2ASEC*Rpap/DDs[-1], NPang])
	    RA = self.interpolate_list (times, RAs, MJD)[1]
	    RAstr=str(RA)+'deg'
	    DEC = self.interpolate_list (times, DECs, MJD)[1]
	    DECstr=str(DEC)+'deg'
	    directions.append(me.direction('J2000',RAstr,DECstr))
    #
    # now get the doppler shift
    #
    # NOTE: this is not exactly right, because it doesn't include the
    # distance to the body in any of these calls.  the distance will matter
    # because it will change the line-of-sight vector from the observatory
    # to the body, which will change the doppler shift.  jeff thinks using
    # the comet measures calls might fix this, but i haven't been able to
    # figure them out yet.  i thought i had it figured out, with the
    # call to me.framecomet(), but that doesn't give the right answer,
    # unfortunately.  i spot-checked the error introduced because of this,
    # and it looks to be of order 1 m/s for these bodies, so i'm not going
    # to worry about it.
    #
	    me.doframe(me.observatory(observatory))
	    me.doframe(me.epoch('utc',str(MJD)+'d'))
	    me.doframe(directions[-1])
    #
    # instead of the call to me.doframe() in the line above, i thought the
    # following call to me.framecomet() would be right, but it doesn't give
    # the right answer :/.
    #       me.framecomet(ephemeris_file)
    #
    # RadVel is currently in AU/day.  we want it in km/s.
    #
	    RadVel = self.interpolate_list (times, RadVels, MJD)[1] * AU / 86400000.0
	    rv = me.radialvelocity('geo',str(RadVel)+'km/s')
	    shifted_frequencies = []
	    for frequency in frequencies:
    #
    # the measure for radial velocity could be obtained via:
    # me.measure(rv,'topo')['m0']['value']
    # but what we really want is a frequency shift.  i could do it by
    # hand, but i'd rather do it with casa toolkit calls.  unfortunately,
    # it's a bit convoluted in casa...
    #
		newfreq0 = me.tofrequency('topo',me.todoppler('optical',me.measure(rv,'topo')),me.frequency('topo',str(frequency[0])+'Hz'))['m0']['value']
		newfreq1 = me.tofrequency('topo',me.todoppler('optical',me.measure(rv,'topo')),me.frequency('topo',str(frequency[1])+'Hz'))['m0']['value']
    #
    # should check units to be sure frequencies are in Hz
    #
    # now, we want to calculate the model shifted in the opposite direction
    # as the doppler shift, so take that into account.
    #
		delta_frequency0 = frequency[0] - newfreq0
		newfreq0 = frequency[0] + delta_frequency0
		delta_frequency1 = frequency[1] - newfreq1
		newfreq1 = frequency[1] + delta_frequency1
		shifted_frequencies.append([newfreq0,newfreq1])
		average_delta_frequency = (delta_frequency0 + delta_frequency1)/2
    #
    # should we print this to the log?
    #
    #           print 'MJD, geo & topo velocities (km/s), and shift (MHz) = %7.1f  %5.1f  %5.1f  %6.3f' % \
    #                 (MJD, RadVel, me.measure(rv,'topo')['m0']['value']/1000, average_delta_frequency/1.0e6)
		msg='MJD, geo & topo velocities (km/s), and shift (MHz) = %7.1f  %5.1f  %5.1f  %6.3f' % \
		     (MJD, RadVel, me.measure(rv,'topo')['m0']['value']/1000, average_delta_frequency/1.0e6)
		casalog.post(msg, 'INFO2')
	    MJD_shifted_frequencies.append(shifted_frequencies)
    #       me.done()
	for ii in range(len(MJDs)):
	    shifted_frequencies = MJD_shifted_frequencies[ii]
	    if (capitalized_source_name == 'Mars'):
		[tstatuses,brightnesses,dbrightnesses] = self.brightness_Mars_int ([MJDs[ii]], shifted_frequencies)
		# modified by TT: take out an extra dimension (for times), to match the rest of the operation 
		tstatuses=tstatuses[0] 
		brightnesses=brightnesses[0]
		dbrightnesses=dbrightnesses[0]
	    else:
		tstatuses = []
		brightnesses = []
		dbrightnesses = []
		for shifted_frequency in shifted_frequencies:
		    [status,brightness,dbrightness] = self.brightness_planet_int (capitalized_source_name, shifted_frequency)
		    tstatuses.append(status)
		    brightnesses.append(brightness)
		    dbrightnesses.append(dbrightness)
	    tfds = []
	    tdfds = []
	    for jj in range (len(tstatuses)):
		 if not tstatuses[jj]:
		    flux_density = brightnesses[jj] * 1.0e26 * pi * Rmeans[ii]*Rmeans[ii]/ (DDs[ii]*DDs[ii])
    #
    # mean apparent planet radius, in arcseconds (used if we ever
    # calculate the primary beam reduction)
    #
		    psize = (Rmeans[ii] / DDs[ii]) * RAD2ASEC
    #
    # primary beam reduction factor (should call a function, but
    # just set to 1.0 for now...
    #
		    pbfactor = 1.0
		    flux_density *= pbfactor
		    tfds.append(flux_density)
		    tdfds.append(0.0)
		 else:
		    tfds.append(0.0)
		    tdfds.append(0.0)
	    statuses.append(tstatuses)
	    fds.append(tfds)
	    dfds.append(tdfds)

	return [ statuses, fds, dfds, Rhats, directions ]


    def brightness_Mars_int (self,MJDs, frequencies):
	'''
	Planck brightness for Mars.  this one is different because
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
	#HH = 6.6260755e-34
	#KK = 1.380658e-23
	#CC = 2.99792458e8

	statuses = []
	Tbs = []
	dTbs = []
	try:
	    model_data_path = os.environ['CASAPATH'].split()[0]+'/data/alma/SolarSystemModels/'
	    model_data_filename = model_data_path + 'Mars_Tb.dat'
	    ff = open(model_data_filename)
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
	    nind = self.nearest_index (modelMJDs, MJD)
	    mTbs = []
	    mfds = []
	    for ii in range(len(freqs)):
		lMJD = []
		lTb = []
		for jj in range(nind-10, nind+10):
		    lMJD.append(modelMJDs[jj])
		    lTb.append(modelTbs[jj][ii])
		mTbs.append(self.interpolate_list(lMJD, lTb, MJD)[1])
    #
    # note: here, when we have the planck results, get a proper estimate of
    # the background temperature.
    #
    # note also that we want to do this here because the integral needs to
    # be done on the brightness, not on the brightness *temperature*.
    #
		Tbg = 2.725
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
		    [estatus, eTb, edTb] = self.integrate_Tb (freqs, mfds, frequency)
    #
    # should we print out the Tb we found?  not sure.  i have a
    # vague recollection that crystal requested it, but i'm not
    # sure if it's really needed.  we'd have to back out the 
    # planck correction (along with the background), so it wouldn't
    # be trivial.
    #
		    estatuses.append(estatus)
		    eTbs.append(eTb)
		    edTbs.append(edTb)
	    statuses.append(estatuses)
	    Tbs.append(eTbs)
	    dTbs.append(edTbs)
	return [statuses, Tbs, dTbs ]


    def brightness_planet_int (self,source_name, frequency):
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
	#HH = 6.6260755e-34
	#KK = 1.380658e-23
	#CC = 2.99792458e8
        
        
        if not self.models.has_key(source_name):
	    try:
		model_data_path = os.environ['CASAPATH'].split()[0]+'/data/alma/SolarSystemModels/'
		model_data_filename = model_data_path + source_name + '_Tb.dat'
		ff = open(model_data_filename)
	    except:
		return [ 3, 0.0, 0.0 ]
	    fds = []
	    Tbs = []
	    freqs = []
	    for line in ff:
		[freq,Tb] = line[:-1].split()
		Tbs.append(float(Tb))
		freqs.append(1.0e9*float(freq))
	#
	# note: here, when we have the planck results, get a proper
	# estimate of the background temperature.
	#
	# note also that we want to do this here because the integral
	# needs to be done on the brightness, not on the brightness
	# *temperature*.
	#
		Tbg = 2.725
		fds.append((2.0 * HH * freqs[-1]**3.0 / CC**2.0) * \
			    ((1.0 / (exp(HH * freqs[-1] / (KK * Tbs[-1])) - 1.0)) - \
			     (1.0 / (exp(HH * freqs[-1] / (KK * Tbg)) - 1.0))))
	    ff.close()
            # store 
            srcn=source_name
            self.models[srcn]={}
            self.models[srcn]['fds']=fds
            self.models[srcn]['freqs']=freqs
        else:
            #recover fds, freqs
            fds=self.models[source_name]['fds']
            freqs=self.models[source_name]['freqs']
      
	if (frequency[0] < freqs[0] or frequency[1] > freqs[-1]):
	    return [ 2, 0.0, 0.0 ]
	else:
    #
    # should we print out the Tb we found?  not sure.  i have a
    # vague recollection that crystal requested it, but i'm not
    # sure if it's really needed.  we'd have to back out the 
    # planck correction (along with the background), so it wouldn't
    # be trivial.  and here, we'd have to return a variable and
    # work on that.
    #
	    return self.integrate_Tb (freqs, fds, frequency)

    def nearest_index (self,input_list, value):
	"""
	find the index of the list input_list that is closest to value
	"""

	ind = searchsorted(input_list, value)
	ind = min(len(input_list)-1, ind)
	ind = max(1, ind)
	if value < (input_list[ind-1] + input_list[ind]) / 2.0:
	    ind = ind - 1
	return ind

    def interpolate_list (self,freqs, Tbs, frequency):
	ind = self.nearest_index (freqs, frequency)
	low = max(0,ind-5)
	if (low == 0):
	    high = 11
	else:
	    high = min(len(freqs),ind+6)
	    if (high == len(freqs)):
		low = high - 11
    #
    # i wanted to put in a check for tabulated values that change
    # derivative, since that confuses the interpolator.  benign cases are
    # fine, like radial velocity, but for the model Tbs, where there are
    # sharp spectral lines, then the fitting won't be sensible when you're
    # right at the center of the line, because the inflection is so severe.
    # i thought if i just only took values that had the same derivative as
    # the location where the desired value is that would work, but it
    # doesn't :/.  i'm either not doing it right or there's something
    # deeper.
    #
    #   if freqs[ind] < frequency:
    #       deriv = Tbs[ind+1] - Tbs[ind]
    #   else:
    #       deriv = Tbs[ind] - Tbs[ind-1]
    #   tTbs = []
    #   tfreqs = []
    #   for ii in range(low,high):
    #       nderiv = Tbs[ii+1] - Tbs[ii]
    #       if (nderiv >= 0.0 and deriv >= 0.0) or (nderiv < 0.0 and deriv < 0.0):
    #           tTbs.append(Tbs[ii])
    #           tfreqs.append(freqs[ii])
    #   aTbs = array(tTbs)
    #   afreqs = array(tfreqs)
	aTbs = array(Tbs[low:high])
	afreqs = array(freqs[low:high])
    #
    # cubic interpolation blows up near line centers (see above comment),
    # so check that it doesn't fail completely (put it in a try/catch), and
    # also that it's not a NaN and within the range of the tabulated values
    #
	range = max(aTbs) - min(aTbs)
	try:
	    func = interp1d (afreqs, aTbs, kind='cubic')
	    if isnan(func(frequency)) or func(frequency) < min(aTbs)-range/2 or func(frequency) > max(aTbs)+range/2:            func = interp1d (afreqs, aTbs, kind='linear')
	except:
	    func = interp1d (afreqs, aTbs, kind='linear')
    #
    # if it still failed, even with the linear interpolation, just take the
    # nearest tabulated point.
    #
	if isnan(func(frequency)) or func(frequency) < min(aTbs)-range/2 or func(frequency) > max(aTbs)+range/2:
	    brightness = Tbs[ind]
	else:
	    brightness = float(func(frequency))
	return [ 0, brightness, 0.0 ]


    def integrate_Tb (self,freqs, Tbs, frequency):
	[status,low_Tb,low_dTb] = self.interpolate_list (freqs, Tbs, frequency[0])
	low_index = self.nearest_index (freqs, frequency[0])
	if (frequency[0] > freqs[low_index]):
	    low_index = low_index + 1

	[status,hi_Tb,hi_dTb] = self.interpolate_list (freqs, Tbs, frequency[1])
	hi_index = self.nearest_index (freqs, frequency[1])
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
	       ii += 1
	Tb /= (frequency[1] - frequency[0])
	return [ 0, Tb, 0.0 ]


