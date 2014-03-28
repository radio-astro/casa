from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
from . import bandpassworker
from . import bandpassmode
from .. import gaincal

LOG = infrastructure.get_logger(__name__)



class PhcorBandpassInputs(bandpassmode.BandpassModeInputs):
    phaseup       = basetask.property_with_default('phaseup', True)
    phaseupbw     = basetask.property_with_default('phaseupbw', '500MHz')
    phaseupsolint = basetask.property_with_default('phaseupsolint', 'int')
    #solint        = basetask.property_with_default('solint', 'inf,7.8125MHz')
    solint        = basetask.property_with_default('solint', 'inf')
    maxchannels   = basetask.property_with_default('maxchannels', 240)
    
    @basetask.log_equivalent_CASA_call
    def __init__(self,
                 # parameters for BandpassModeInputs:
                 context, mode='channel',
                 # parameters specific to this task:
                 phaseup=None, phaseupbw=None, phaseupsolint=None,
                 # parameters overridden by this task:
                 solint=None, maxchannels=None,
                 # other parameters to be passed to child bandpass task
                 **parameters):
        super(PhcorBandpassInputs, self).__init__(context, mode=mode,
            phaseup=phaseup, phaseupbw=phaseupbw, phaseupsolint=phaseupsolint,
            solint=solint, maxchannels=maxchannels, **parameters)


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
	if inputs.maxchannels <= 0:
            result = self._do_bandpass()
	else:
            #result = self._do_fixed_bandpass()
            result = self._do_smoothed_bandpass()
        
        # Attach the preparatory result to the final result so we have a
        # complete log of all the executed tasks. 
        if inputs.phaseup:
            result.preceding.append(phaseup_result.final)

        return result

    def _do_phaseup(self):
        inputs = self.inputs

        phaseup_inputs = gaincal.GTypeGaincal.Inputs(inputs.context,
            vis         = inputs.vis,
            field       = inputs.field,
            spw         = self._get_phaseup_spw(),
            antenna     = inputs.antenna,
            intent      = inputs.intent,
            solint      = inputs.phaseupsolint,
            refant      = inputs.refant,
            minblperant = inputs.minblperant,
            calmode     = 'p',
            minsnr      = inputs.minsnr)

        phaseup_task = gaincal.GTypeGaincal(phaseup_inputs)
        
        return self._executor.execute(phaseup_task, merge=True)

    def _do_bandpass(self):
        bandpass_task = bandpassmode.BandpassMode(self.inputs)
        return self._executor.execute(bandpass_task)

    def _do_fixed_bandpass(self):
	orig_solint = self.inputs.solint
        try:
	    # Determine the minimum resolution required to get at
	    # least 240 channels for all spws
	    spwlist = self.inputs.ms.get_spectral_windows(self.inputs.spw)
	    chanreslist = []
	    for spw in spwlist:
		# TDM or FDM
                dd = self.inputs.ms.get_data_description(spw=spw)
                if dd is None:
                    LOG.debug('Missing data description for spw %s ' % spw.id)
                    continue
                ncorr = len(dd.corr_axis)
                if ncorr not in set([1,2,4]):
                    LOG.debug('Wrong number of correlations %s for spw %s ' % (ncorr, spw.id))
                    continue
		bandwidth = spw.bandwidth.to_units( \
		    otherUnits=measures.FrequencyUnits.MEGAHERTZ)
                if (ncorr * spw.num_channels > 256):
		    chanres = bandwidth / self.inputs.maxchannels
		else:
		    chanres = bandwidth / spw.num_channels
		chanreslist.append(chanres)
	    min_chanres = min(chanreslist)

	    self.inputs.solint=orig_solint + ',' + str(min_chanres) + 'MHz'
            bandpass_task = bandpassmode.BandpassMode(self.inputs)
            return self._executor.execute(bandpass_task)

        finally:
	    self.inputs.solint = orig_solint

    def _do_smoothed_bandpass(self):

	# Store original values of some parameters.
        orig_spw = self.inputs.spw
	orig_solint = self.inputs.solint
	orig_append = self.inputs.append

        try:
	    # initialize the caltable and list of spws
	    self.inputs.caltable = self.inputs.caltable
	    spwlist = self.inputs.ms.get_spectral_windows(orig_spw)

	    # Loop through the spw appending the results of each spw
	    # to the results of the previous one.
	    for spw in spwlist:

		# TDM or FDM
                dd = self.inputs.ms.get_data_description(spw=spw)
                if dd is None:
                    LOG.debug('Missing data description for spw %s ' % spw.id)
                    continue
                ncorr = len(dd.corr_axis)
                if ncorr not in set([1,2,4]):
                    LOG.debug('Wrong number of correlations %s for spw %s ' % (ncorr, spw.id))
                    continue

		# Smooth if FDM and if it makes sense
                if (ncorr * spw.num_channels > 256):
		    if (spw.num_channels / self.inputs.maxchannels) < 1:
		        self.inputs.solint = orig_solint
		    else:
		        bandwidth = spw.bandwidth.to_units( \
		            otherUnits=measures.FrequencyUnits.MEGAHERTZ)
		        self.inputs.solint=orig_solint + ',' + \
		            str(bandwidth / self.inputs.maxchannels) + 'MHz'
                else:
		    self.inputs.solint=orig_solint

		# Compute and append bandpass solution
		self.inputs.spw=spw.id
                bandpass_task = bandpassmode.BandpassMode(self.inputs)
                result = self._executor.execute(bandpass_task)
		self.inputs.append=True
		self.inputs.caltable=result.final[-1].gaintable

	    # Reset the calto spw list
	    result.pool[0].calto.spw = orig_spw
	    result.final[0].calto.spw = orig_spw
	    return result

        finally:
            self.inputs.spw = orig_spw
            self.inputs.solint = orig_solint
            self.inputs.append = orig_append

    def _get_phaseup_spw(self):
        '''
	           ms -- measurement set object 
	       spwstr -- comma delimited list of spw ids
	    bandwidth -- bandwidth in Hz of central channels used to
	                 phaseup
    	'''
        inputs = self.inputs

        # Add the channel ranges in. Note that this currently assumes no prior
        # channel selection.
        if inputs.phaseupbw == '':
            return inputs.spw

        # Convert bandwidth input to CASA quantity and then on to pipeline
        # domain Frequency object
        quanta = casatools.quanta
        bw_quantity = quanta.convert(quanta.quantity(inputs.phaseupbw), 'Hz')        
        bandwidth = measures.Frequency(quanta.getvalue(bw_quantity)[0], 
                                       measures.FrequencyUnits.HERTZ)
    
        # Loop over the spws creating a new list with channel ranges
        outspw = []
        for spw in self.inputs.ms.get_spectral_windows(self.inputs.spw):
            cen_freq = spw.centre_frequency 
            lo_freq = cen_freq - bandwidth / 2.0  
            hi_freq = cen_freq + bandwidth / 2.0
            minchan, maxchan = spw.channel_range(lo_freq, hi_freq)
            cmd = '{0}:{1}~{2}'.format(spw.id, minchan, maxchan)
            outspw.append(cmd)
    
        return ','.join(outspw)
