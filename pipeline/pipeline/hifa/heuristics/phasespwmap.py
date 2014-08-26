import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures

LOG = infrastructure.get_logger(__name__)

# Heuristics for computing a simple phase up wide to narrow spwmap
# Here an spw is the spw object stored in the domain object.
#        allspws - List of all spws in the MS
#        scipws  - List of all science spws in the MS
#    maxnarrowbw - Maximum narrow bandwidth, e.g. '300MHz'
#      maxbwfrac - Width must be > maxbwfrac * maximum bandwidth for a match
#         samebb - If possible match within a baseband
#

def simple_w2nspwmap (allspws, scispws, maxnarrowbw, maxbwfrac, samebb):

    quanta = casatools.quanta

    # Find the maximum science spw bandwidth for each science receiver band.
    # Refine the dictionary to account for maximum per baseband later should
    # that be necessary.
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
    matchedspws = []
    for scispw in scispws:

	#  Wide spw, match spw to itself.
        if scispw.bandwidth > maxnbw:
	    matchedspws.append(scispw)
	    continue
	 
	# Loop through the other science
	# windows looking for a match
	bestspw = None
	for matchspw in scispws:

	    # Skip self
	    if matchspw.id == scispw.id:
                #LOG.info('Skipping match spw id %s' % matchspw.id)
	        continue

	    # Don't match across receiver bands
	    if matchspw.band != scispw.band:
                #LOG.info('Skipping bandwidth match spw id %s' % matchspw.id)
	        continue

	    # Skip if the match window is narrower than the
	    # window in question or narrower than a certain
	    # fraction of the maximum bandwidth
	    if matchspw.bandwidth < scispw.bandwidth or \
	        matchspw.bandwidth < maxbwfrac * maxnbw:
                #LOG.info('Skipping condition match spw id %s' % matchspw.id)
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
		# If the candidate  match is in the same baseband
		# as the science spw but the current best match is not
		# then switch matches.

	        if matchspw.baseband == scispw.basband and \
		    bestspw.baseband != scispw.baseband:
		    bestspw = matchspw
		else:
		    if abs(scispw.centre_frequency.value - matchspw.centre_frequency.value) <  \
		        abs(scispw.centre_frequency.value - bestspw.centre_frequency.value):
		        bestspw = matchspw
	
	# Append the matched spw to the list
	if bestspw is None:
	    matchedspws.append(scispw)
	else:
	    matchedspws.append(bestspw)

    # Find the maximum science spw id
    max_spwid = 0
    for scispw in scispws:

        # Find the maximum science spw id
        if scispw.id > max_spwid:
	    max_spwid = scispw.id


    # Initialize the spwmap. All spw ids up to the maximum
    # science spw id must be defined.
    phasespwmap = []
    for i in range(max_spwid + 1):
        phasespwmap.append(i)

    # Make a reference copy for comparison
    refphasespwmap = list(phasespwmap)

    # Set the science window spw map using the matching
    # spw ids
    for scispw, matchspw in zip(scispws, matchedspws):
        phasespwmap[scispw.id] = matchspw.id

    # Return  the new map
    if phasespwmap == refphasespwmap:
        return []
    else:
        return phasespwmap
