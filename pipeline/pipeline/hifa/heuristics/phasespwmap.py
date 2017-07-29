import decimal

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures

LOG = infrastructure.get_logger(__name__)

def combine_spwmap (allspws, scispws, combine_spwid):

# Heuristics for computing a simple combined spw map
#    At present all this does is assume that the science
#    windows are mapped to a single spwid 
# Here an spw is the spw object stored in the domain object.
#        allspws - List of all spws in the MS (not actually used)
#        scipws  - List of all science spws in the MS

    # Find the maximum science spw id
    max_spwid = 0
    for scispw in scispws:

        # Find the maximum science spw id
        if scispw.id > max_spwid:
	    max_spwid = scispw.id

    # Initialize the spwmap. All spw ids up to the maximum
    # science spw id must be defined.
    combinespwmap = []
    for i in range(max_spwid + 1):
        combinespwmap.append(i)

    # Make a reference copy for comparison
    refspwmap = list(combinespwmap)

    for scispw in scispws:
        combinespwmap[scispw.id] = combine_spwid

    # Return  the new map
    if combinespwmap == refspwmap:
        return []
    else:
        return combinespwmap

# Heuristics for computing an spwmap which uses SNR information.
# Here an spw is the spw object stored in the domain object.
#        allspws - List of all spws in the MS (not actually used)
#        scipws  - List of all science spws in the MS
#          snrs  - List of snr values for scispws
#      goodsnrs  - Determines whether the SNR is good (True), bad (False), or undefined (None)
#                - At least one value per receiver band should be good.

def snr_n2wspwmap (allspws, scispws, snrs, goodsnrs):

    quanta = casatools.quanta

    # Find the spw with largest good SNR for each receiver band 
    snrdict = {}
    for scispw, snr, goodsnr in zip (scispws, snrs, goodsnrs):
        if goodsnr is not True:
            continue
	if snrdict.has_key(scispw.band):
            if snr > snrdict[scispw.band]:
	        snrdict[scispw.band] = snr
	else:
	    snrdict[scispw.band] = snr
    LOG.debug('Maximum SNR per receiver band dictionary %s' % (snrdict))

    # Find a matching spw for each science spw
    matchedspws = []; matchedgoodsnrs = []
    for scispw, snr, goodsnr in zip (scispws, snrs, goodsnrs):

        LOG.debug('Looking for match to spw id %s' % scispw.id)

        # Good SNR so match spw to itself regardless of any other criteria
        if goodsnr is True:
	    matchedspws.append(scispw)
	    matchedgoodsnrs.append(goodsnr)
            LOG.debug('Good SNR so matched spw id %s to itself' % scispw.id)
	    continue

        if not snrdict.has_key(scispw.band):
	    matchedspws.append(scispw)
	    matchedgoodsnrs.append(None)
            LOG.debug('No good SNR spw in receiver band so match spw id %s to itself' % scispw.id)
	    continue

	# Loop through the other science windows looking for a match
	bestspw = None; bestnsr = None; bestgoodsnr = None
	for matchspw, matchsnr, matchgoodsnr  in zip (scispws, snrs, goodsnrs):

	    # Skip self
	    if matchspw.id == scispw.id:
                LOG.debug('Skipping match of spw %s to itself' % matchspw.id)
	        continue

	    # Don't match across receiver bands
	    if matchspw.band != scispw.band:
                LOG.debug('Skipping bad receiver band match to spw id %s' % matchspw.id)
	        continue

            # Skip bad SNR matches if at least one good SNR window in the receiver band exists
            if snrdict.has_key(matchspw.band) and matchgoodsnr is not True:
                LOG.debug('Skipping match with poor SNR spw id %s' % matchspw.id)
	        continue

	    # First candidate match
	    if bestspw is None:
	        bestspw = matchspw
                bestsnr = matchsnr
                bestgoodsnr = matchgoodsnr
                LOG.debug('First possible match is to spw id %s' % matchspw.id)
	    elif matchsnr > bestsnr: 
	        bestspw = matchspw
                bestsnr = matchsnr
                bestgoodsnr = matchgoodsnr
                LOG.debug('Found higher SNR match spw id %s' % matchspw.id)
            else:
                LOG.debug('SNR lower than previous match skipping spw id %s' % matchspw.id)
                pass
	
	# Append the matched spw to the list
	if bestspw is None:
	    matchedspws.append(scispw)
	    matchedgoodsnrs.append(goodsnr)
	else:
	    matchedspws.append(bestspw)
	    matchedgoodsnrs.append(bestgoodsnr)

    # Find the maximum science spw id
    max_spwid = 0
    for scispw in scispws:
        if scispw.id > max_spwid:
            max_spwid = scispw.id

    # Initialize the spwmap. All spw ids up to the maximum
    # science spw id must be defined.
    phasespwmap = []; snrmap= []
    for i in range(max_spwid + 1):
        phasespwmap.append(i)
        snrmap.append(None)

    # Make a reference copy for comparison
    refphasespwmap = list(phasespwmap)

    # Set the science window spw map using the matching spw ids
    goodmap = True
    for scispw, matchspw, matchedgoodsnr in zip(scispws, matchedspws, matchedgoodsnrs):
        phasespwmap[scispw.id] = matchspw.id
        snrmap[scispw.id] = matchedgoodsnr
        if matchedgoodsnr is not True:
            goodmap = False
        
    # Return  the new map
    if goodmap is True and phasespwmap == refphasespwmap:
        return True, [], []
    else:
        return goodmap, phasespwmap, snrmap

# Heuristics for computing a simple phase up wide to narrow spwmap
# Here an spw is the spw object stored in the domain object.
#        allspws - List of all spws in the MS (not actually used)
#        scipws  - List of all science spws in the MS
#    maxnarrowbw - Maximum narrow bandwidth, e.g. '300MHz'
#      maxbwfrac - Width must be > maxbwfrac * maximum bandwidth for a match
#         samebb - If possible match within a baseband
#

def simple_n2wspwmap (allspws, scispws, maxnarrowbw, maxbwfrac, samebb):

    quanta = casatools.quanta

    # Find the maximum science spw bandwidth for each science receiver band.
    bwmaxdict = {}
    for scispw in scispws:
        bandwidth = scispw.bandwidth
	if bwmaxdict.has_key(scispw.band):
	    if bandwidth > bwmaxdict[scispw.band]:
	        bwmaxdict[scispw.band] = bandwidth
	else:
	    bwmaxdict[scispw.band] = bandwidth

    # Convert the maximum narrow bandwidth to the the correct format
    maxnbw = quanta.convert(quanta.quantity(maxnarrowbw), 'Hz')
    maxnbw = measures.Frequency(quanta.getvalue(maxnbw)[0],
                                           measures.FrequencyUnits.HERTZ)

    # Find a matching spw each science spw
    matchedspws = []; failedspws = []
    for scispw in scispws:

	#  Wide spw, match spw to itself.
        if scispw.bandwidth > maxnbw:
	    matchedspws.append(scispw)
            LOG.debug('Matched spw id %s to itself' % scispw.id)
	    continue
	 
	# Loop through the other science
	# windows looking for a match
	bestspw = None
	for matchspw in scispws:

	    # Skip self
	    if matchspw.id == scispw.id:
                LOG.debug('Skipping match with self for spw id %s' % matchspw.id)
	        continue

	    # Don't match across receiver bands
	    if matchspw.band != scispw.band:
                LOG.debug('Skipping bad receiver band match for spw id %s' % matchspw.id)
	        continue

	    # Skip if the match window is narrower than the window in question
            # or narrower than a certain fraction of the maximum bandwidth. 
	    if matchspw.bandwidth <= scispw.bandwidth or \
	        matchspw.bandwidth < decimal.Decimal(str(maxbwfrac)) * bwmaxdict[scispw.band]:
                LOG.debug('Skipping bandwidth match condition spw id %s' % matchspw.id)
	        continue

	    # First candidate match
	    if bestspw is None:
	        bestspw = matchspw

	    # Find the spw with the closest center frequency
	    elif samebb == False:

		if abs(scispw.centre_frequency.value - matchspw.centre_frequency.value) <  \
		    abs(scispw.centre_frequency.value - bestspw.centre_frequency.value):
		    bestspw = matchspw
                else:
                    pass
		
	    else:
		# If the candidate  match is in the same baseband as the science spw but the current best
                # match is not then switch matches.

	        if matchspw.baseband == scispw.baseband and \
		    bestspw.baseband != scispw.baseband:
		    bestspw = matchspw
		else:
		    if abs(scispw.centre_frequency.value - matchspw.centre_frequency.value) <  \
		        abs(scispw.centre_frequency.value - bestspw.centre_frequency.value):
		        bestspw = matchspw
                    else:
                        pass
	
	# Append the matched spw to the list
	if bestspw is None:
            LOG.debug('    Simple spw mapping failed for spw id %s' % scispw.id)
	    matchedspws.append(scispw)
            failedspws.append (scispw) 
	else:
	    matchedspws.append(bestspw)

    # Issue a warning if any spw failed spw mapping
    if len(failedspws) > 0:
        LOG.warn('Cannot map narrow spws %s to wider ones - defaulting these to standard mapping' % [spw.id for spw in failedspws])

    # Find the maximum science spw id
    max_spwid = 0
    for scispw in scispws:
        if scispw.id > max_spwid:
            max_spwid = scispw.id

    # Initialize the spwmap. All spw ids up to the maximum science spw id must be
    # defined in the map passed to CASA.
    phasespwmap = []
    for i in range(max_spwid + 1):
        phasespwmap.append(i)

    # Make a reference copy for comparison
    refphasespwmap = list(phasespwmap)

    # Set the science window spw map using the matching spw ids
    for scispw, matchspw in zip(scispws, matchedspws):
        phasespwmap[scispw.id] = matchspw.id

    # Return  the new map
    if phasespwmap == refphasespwmap:
        return []
    else:
        return phasespwmap
