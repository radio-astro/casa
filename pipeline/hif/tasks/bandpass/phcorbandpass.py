from __future__ import absolute_import

import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure.logging as logging
from . import bandpassworker
from . import bandpassmode
from .. import gaincal

LOG = logging.get_logger(__name__)


class PhcorBandpassInputs(bandpassmode.BandpassModeInputs):
    def __init__(self, context, mode='channel', phaseupsolint=None, 
                 phaseup=None, phaseupbw=None,  **parameters):
        super(PhcorBandpassInputs, self).__init__(context, mode,
            phaseupsolint=phaseupsolint, phaseup=phaseup,
	    phaseupbw=phaseupbw, **parameters)

    @property
    def phaseupsolint(self):
        return self._phaseupsolint

    @phaseupsolint.setter
    def phaseupsolint(self, value):
        """
        Set the value of phaseupsolint.
        
        If value is None, phaseupsolint will be reset to its default value.
        """
        if value is None:
            value = 'int'
        self._phaseupsolint = value

    @property
    def phaseup(self):
        return self._phaseup

    @phaseup.setter
    def phaseup(self, value):
        if value is None:
            value = True
        self._phaseup = value

    @property
    def phaseupbw(self):
        return self._phaseupbw

    @phaseupbw.setter
    def phaseupbw(self, value):
        if value is None:
            value = ''
        self._phaseupbw = value


class PhcorBandpass(bandpassworker.BandpassWorker):
    Inputs = PhcorBandpassInputs

    def prepare(self, **parameters):
        inputs = self.inputs
        
        # if requested, execute a phaseup job. This will add the resulting
        # caltable to the on-the-fly calibration context, so we don't need any
        # subsequent gaintable manipulation
        if inputs.phaseup: 
            phaseup_result = self._do_phaseup()

        # Now perform the bandpass
        result = self._do_bandpass()
        
        # Attach the preparatory result to the final result so we have a
        # complete log of all the executed tasks. 
        if inputs.phaseup:
            result.preceding.append(phaseup_result.final)

        return result

    def _do_phaseup(self):
        inputs = self.inputs
        phaseup_inputs = gaincal.GTypeGaincal.Inputs(inputs.context,
            vis       = inputs.vis,
            field     = inputs.field,
            spw       = inputs.spw,
            antenna   = inputs.antenna,
            intent    = inputs.intent,
            solint    = inputs.phaseupsolint,
            refant    = inputs.refant,
            calmode   = 'p',
            minsnr    = inputs.minsnr)
            #minsnr    = 0.0)

	# Add the channel ranges in. Note that this currently
	# assumes no prior channel selection.
	if inputs.phaseupbw != '':
	    qat = casatools.quanta
	    bandwidth = qat.convert (qat.quantity(inputs.phaseupbw), 'Hz')
	    spwstr = self._get_phaseupbw (inputs.ms,
	        phaseup_inputs.spw, qat.getvalue(bandwidth)[0])
	    phaseup_inputs.spw = spwstr

        phaseup_task = gaincal.GTypeGaincal(phaseup_inputs)
        
        return self._executor.execute(phaseup_task, merge=True)

    def _do_bandpass(self):
        bandpass_task = bandpassmode.BandpassMode(self.inputs)
        return self._executor.execute(bandpass_task)

    def _get_phaseupchan(self, ms, spwstr, chanfrac):

        '''
	          ms -- measurement set object 
	      spwstr -- comma delimited list of spw ids
	    chanfrac -- fraction of the central channels used to
	                phase up
	'''

	# Turn a comma delimited string of spws into a list.
	inspw = spwstr.split(',')

	# Loop over the spw list creating a new list with channel
	# ranges
	outspw = []
	for spwid in inspw:
	    spw = ms.get_spectral_window(spwid)
	    cen_chan = (spw.num_channels + 1) / 2
	    frac_chan = int (round (chanfrac * spw.num_channels / 2.0 + 0.5))
	    if 2 * frac_chan >= spw.num_channels:
	        outspw.append(str(spw.id))
	    else:
	        minchan = max (0, cen_chan - frac_chan) 
		maxchan = min (cen_chan + frac_chan, spw.num_channels - 1)
		cmd = '{0}:{1}~{2}'.format (spw.id, minchan, maxchan)
		outspw.append(cmd)

	# Turn the output list back into a comma delimited string of spws
	outspwstr = ','.join(outspw)

	return outspwstr 

    def _get_phaseupbw (self, ms, spwstr, bandwidth):

        '''
	           ms -- measurement set object 
	       spwstr -- comma delimited list of spw ids
	    bandwidth -- bandwidth in Hz of central channels used to
	                 phaseup
	'''

	# Convert bandwidth to frequency object
	bw = measures.Frequency (bandwidth, measures.FrequencyUnits.HERTZ)

	# Turn a comma delimited string of spws into a list.
	inspw = spwstr.split(',')

	# Loop over the spw list creating a new list with channel
	# ranges
	outspw = []
	for spwid in inspw:
	    spw = ms.get_spectral_window(spwid)
	    cen_freq = spw.centre_frequency 
	    lo_freq = cen_freq - bw / 2.0  
	    hi_freq = cen_freq + bw / 2.0
	    minchan, maxchan = spw.channel_range(lo_freq, hi_freq)
	    cmd = '{0}:{1}~{2}'.format (spw.id, minchan, maxchan)
	    outspw.append(cmd)

	# Turn the output list back into a comma delimited string of spws
	outspwstr = ','.join(outspw)

	return outspwstr 
