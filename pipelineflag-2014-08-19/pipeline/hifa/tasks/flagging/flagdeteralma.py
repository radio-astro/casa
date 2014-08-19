
# Imports
# -------

from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures

from pipeline.hif.tasks.flagging import flagdeterbase 


# ------------------------------------------------------------------------------

# Initialize a logger
# -------------------

LOG = infrastructure.get_logger(__name__)

# ------------------------------------------------------------------------------

class FlagDeterALMAInputs( flagdeterbase.FlagDeterBaseInputs ):


        edgespw  = basetask.property_with_default('edgespw', True)
        fracspw  = basetask.property_with_default('fracspw', 0.0625)
        template  = basetask.property_with_default('template', True)

        @basetask.log_equivalent_CASA_call
	def __init__( self, context, vis=None, output_dir=None, flagbackup=None,
	    autocorr=None, shadow=None, scan=None, scannumber=None,
	    intents=None, edgespw=None, fracspw=None, fracspwfps=None, online=None,
	    fileonline=None, template=None, filetemplate=None, hm_tbuff=None, tbuff=None ):

		# Initialize the public member variables of the inherited class
		# FlagDeterBaseInputs()

		super( FlagDeterALMAInputs, self ).__init__( context, vis=vis,
		    output_dir=output_dir, flagbackup=flagbackup, autocorr=autocorr,
		    shadow=shadow, scan=scan, scannumber=scannumber, intents=intents,
		    edgespw=edgespw, fracspw=fracspw, fracspwfps=fracspwfps, online=online,
		    fileonline=fileonline, template=template, filetemplate=filetemplate,
		    hm_tbuff=hm_tbuff, tbuff=tbuff)

                self.fracspwfps  = fracspwfps

        @property
        def fracspwfps(self):
                return self._fracspwfps

        @fracspwfps.setter
        def fracspwfps(self, value):
                if value is None:
                     value = 0.048387
                self._fracspwfps = value



	def to_casa_args( self ):

		# Initialize the arguments from the inherited
		# FlagDeterBaseInputs() class

		task_args = super( FlagDeterALMAInputs, self ).to_casa_args()


		# Return the tflagdata task arguments

		return task_args



class FlagDeterALMAResults( flagdeterbase.FlagDeterBaseResults ):
	pass



class FlagDeterALMA( flagdeterbase.FlagDeterBase ):

# ------------------------------------------------------------------------------

	# Make the member functions of the FlagDeterALMAInputs() class member
	# functions of this class

	Inputs = FlagDeterALMAInputs
	
	def get_fracspw(self, spw):    
	        # override the default fracspw getter with our ACA-aware code
                if spw.num_channels in (62, 124, 248):
                    return self.inputs.fracspwfps
                else:
                    return self.inputs.fracspw
	
	def verify_spw(self, spw):
	        # override the default verifier, adding an extra test that bypasses
                # flagging of TDM windows
                super(FlagDeterALMA, self).verify_spw(spw)
	
	        # Skip if TDM mode where TDM modes are defined to be modes with 
                # <= 256 channels per correlation
                dd = self.inputs.ms.get_data_description(spw=spw)
                ncorr = len(dd.corr_axis)
                if ncorr*spw.num_channels > 256:
                    raise ValueError('Skipping edge flagging for FDM spw %s' % spw.id)
	
	
	
        '''
        def _get_autocorr_cmd (self):
                inputs = self.inputs
	        spwlist = inputs.ms.get_spectral_windows(science_windows_only=False)
                spwids = []
	        for spw in spwlist:
		    if spw.num_channels == 4:
	                spwids.append(spw.id)
	        if not spwids:
                    #return 'mode=manual antenna=*&&&'
                    return 'mode=manual autocorr=True'
	        else:
                    #return 'mode=manual antenna=*&&& spw=>0'
                    return 'mode=manual autocorr=True'

        def _get_edgespw_cmds(self):
                """
                Return a flagdata flagging command that will flag the edge channels
                of spectral windows in this measurement set.

                The number of channels to flag is calculated using the 'fracspw' value
                on this task's associated Inputs.

                :rtype: a string
                """
                inputs = self.inputs

                # to_flag is the list to which flagging commands will be appended
                to_flag = []

                # loop over the spectral windows, generate a flagging command for each
                # spw in the ms. Calling get_spectral_windows() with no arguments
                # returns just the science windows, which is exactly what we want.
                for spw in inputs.ms.get_spectral_windows():


			# Get the data description for this spw
                        dd = inputs.ms.get_data_description(spw=spw)
			if dd is None:
                                LOG.debug('Missing data description for spw %s ' % spw.id)
			        continue

			# Determine the number of correlations
			#   Check that they are between 1 and 4
			ncorr = len (dd.corr_axis)
			if ncorr not in set ([1, 2, 4]):
                                LOG.debug('Wrong number of correlations %s for spw %s ' % (ncorr, spw.id))
			        continue

                        # Skip if TDM mode where TDM modes are defined to
			# be modes with <= 256 channels per correlation
			if (ncorr * spw.num_channels > 256):
				quanta = casatools.quanta
				bw_quantity = quanta.convert(quanta.quantity('1875MHz'), 'Hz')   
				bandwidth = measures.Frequency(quanta.getvalue(bw_quantity)[0],
				    measures.FrequencyUnits.HERTZ)
				cen_freq = spw.centre_frequency
				lo_freq = cen_freq - bandwidth / 2.0
				hi_freq = cen_freq + bandwidth / 2.0
				l_max, r_min = spw.channel_range(lo_freq, hi_freq)
				r_max = spw.num_channels - 1
				
				if l_max <= 0 and r_min >= r_max:
                                    LOG.debug('Skipping edge flagging for FDM spw %s ' % spw.id)
			            continue
                                cmd = '{0}:0~{1};{2}~{3}'.format(spw.id, l_max, r_min, r_max)
                                to_flag.append(cmd)

			else:

                                # If the twice the number of flagged channels is greater than the
                                # number of channels for a given spectral window, skip it.
				if spw.num_channels in set([62, 124, 248]):
                                    frac_chan = int(round(inputs.fracspwfps * spw.num_channels))
				else:
                                    frac_chan = int(round(inputs.fracspw * spw.num_channels))
                                if 2*frac_chan >= spw.num_channels:
                                        LOG.debug('Too many flagged channels %s for spw %s '% (spw.num_channels, spw.id))
                                        continue

                                # calculate the channel ranges to flag. No need to calculate the
                                # left minimum as it is always channel 0.
                                l_max = frac_chan - 1
                                r_min = spw.num_channels - frac_chan
                                r_max = spw.num_channels - 1

                                # state the spw and channels to flag in flagdata format, adding
                                # the statement to the list of flag commands
                                cmd = '{0}:0~{1};{2}~{3}'.format(spw.id, l_max, r_min, r_max)
                                to_flag.append(cmd)

		if len(to_flag) <= 0:
		    return '# No valid edge spw flagging command'
		else:
                    return 'mode=manual spw={0}'.format(','.join(to_flag))

        '''

