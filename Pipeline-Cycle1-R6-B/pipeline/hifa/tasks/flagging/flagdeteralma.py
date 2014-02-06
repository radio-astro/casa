
# ------------------------------------------------------------------------------

# flagdeteralma.py

# NB: THESE FlagDeterALMA*() CLASSES INHERIT FlagDeterBase*() CLASSES.  AT
# PRESENT THE FlagDeterALMA*() CLASSES HAVE NO ADDITIONAL INPUT PARAMETERS, SO
# THEY ACT IN EXACTLY THE SAME MANNER AS THE FlagDeterBase*() CLASSES.

# Description:
# ------------
# This file contains the classes to perform ALMA deterministic flagging.

# In a nutshell:
# --------------
# * This class performs all of the deterministic flagging types in the
#   FlagDeterBase*() classes.

# To test these classes by themselves without the rest of the pipeline, enter
# these commands:
#
# import pipeline
#
# vis = [ '<MS name>' ]
# context = pipeline.Pipeline( vis ).context
#
# inputs = pipeline.tasks.flagging.FlagDeterALMA.Inputs( context, vis=vis,
#   output_dir='.', autocorr=True, shadow=True, scan=True, scannumber='4,5,8',
#   intents='*AMPLI*', edgespw=True, fracspw=0.1, fracspwfps=0.1 )
#
# task = pipeline.tasks.flagging.FlagDeterALMA( inputs )
# jobs = task.analyse()
#
# status = task.execute(dry_run=False)
#
# In other words, create a context, create the inputs (which sets the public
# variables to the correct values and creates the temporary flag command file),
# convert the class arguments to arguments to the CASA task tflagdata), create
# the FlatDeterALMA() instance, perform FlatDeterALMA.analyse(), and execute the
# class.

# Classes:
# --------
# FlagDeterALMA        - This class represents the pipeline interface to the
#                        CASA task tflagdata.
# FlagDeterALMAInputs  - This class manages the inputs for the FlagDeterALMA()
#                        class.
# FlagDeterALMAResults - This class manages the results from the FlagDeterALMA()
#                        class.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version, identical behavior to FlagDeterBase.py.
# 2012 May 16 - Lindsey Davis, NRAO
#               Changed file name from FlagDeterALMA.py to flagdeteralma.py.

# ------------------------------------------------------------------------------

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
# class FlagDeterALMAInputs
# ------------------------------------------------------------------------------

# FlagDeterALMAInputs

# Description:
# ------------
# This class manages the inputs for the FlagDeterALMA() class.

# Inherited classes:
# ------------------
# FlagDeterBaseInputs - This is the base class that handles the inputs for
# deterministic flagging.

# Public member variables:
# ------------------------
# None.

# Public member functions:
# ------------------------
# __init__     - This public member function constructs an instance of the
#                FlagDeterALMAInputs() class.  It is overloaded.
# clone        - This public member function creates a cloned instance of an
#                existing instance.  It is overloaded.
# to_casa_args - This public member function translates the input parameters of
#                this class to task parameters and file-based flag commands
#                required by CASA task tflagdata.  It is overloaded.

# Static public member functions:
# -------------------------------
# create_from_context - This static public member function creates an instance
#                       of this class from a context.  It is overloaded.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version created with public member functions __init__(),
#               clone(), and to_casa_args(); and static public member function
#               create_from_context().  All functions are overloaded.

# ------------------------------------------------------------------------------

class FlagDeterALMAInputs( flagdeterbase.FlagDeterBaseInputs ):

# ------------------------------------------------------------------------------

# FlagDeterALMAInputs::__init__

# Description:
# ------------
# This public member function constructs an instance of the
# FlagDeterALMAInputs() class.

# The primary purpose of this class is to initialize the public member
# variables.  The defaults for all parameters (except context) are None.

# NB: This public member function is overloaded.

# Inherited classes:
# ------------------
# FlagDeterBaseInputs - This class manages the inputs for the
#                       FlagDeterBaseInputs() parent class.

# Inputs to initialize the FlagDeterBaseInputs() class:
# -----------------------------------------------------
# context      - This python dictionary contains the pipeline context (state).
#                It has no default.
#
# vis          - This python string contains the MS name.
#
# output_dir   - This python string contains the output directory name.
#
# flagbackup   - This python boolean determines whether the existing flags are
#                backed up before the new flagging begins.
#
# autocorr     - This python boolean determines whether autocorrelations are
#                flagged or not.
#
# shadow       - This python boolean determines whether shadowed antennas are
#                flagged or not.
#
# scan         - This python boolean determines whether scan flagging is
#                performed.
# scannumber   - This python string contains the comma-delimited scan numbers.
#                In the task interface, it is a subparameter of the scan
#                parameter.  Standard data selection syntax is valid.
# intents      - This python string contains the comma-delimited intents.  In
#                the task interface, it is a subparameter of the scan parameter.
#                Wildcards (* character) are allowed.
#
# edgespw      - This python boolean determines whether edge channels are
#                flagged.
# fracspw      - This python float contains the fraction (between 0.0 and 1.0)
#                of channels removed from the edge for the ALMA baseline correlator.
#                In the task interface, it is a subparameter of the edgespw parameter.
#
# fracspwfps    - This python float contains the fraction (between 0.0 and 1.0)
#                of channels removed from the edge for the ACS correlator.  In the
#                task interface, it it is a subparameter of the edgespw parameter.
#
# online       - This python boolean determines whether the online flags are
#                applied.
# fileonline   - This python string contains the name of the ASCII file that
#                has the flagging commands.  It is a subparameter of the
#                online parameter.
#
# template     - This python boolean determines whether flagging templates are
#                applied.
# filetemplate - This python string contains the name of the ASCII file that
#                has the flagging template (for RFI, birdies, telluric lines,
#                etc.).  It is a subparameter of the template parameter.

# Inputs:
# -------
# None.

# Outputs:
# --------
# None, returned via the function value.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------
        edgespw  = basetask.property_with_default('edgespw', True)
        fracspw  = basetask.property_with_default('fracspw', 0.0625)
        template  = basetask.property_with_default('template', True)

	def __init__( self, context, vis=None, output_dir=None, flagbackup=None,
	    autocorr=None, shadow=None, scan=None, scannumber=None,
	    intents=None, edgespw=None, fracspw=None, fracspwfps=None, online=None,
	    fileonline=None, template=None, filetemplate=None ):

		# Initialize the public member variables of the inherited class
		# FlagDeterBaseInputs()

		super( FlagDeterALMAInputs, self ).__init__( context, vis=vis,
		    output_dir=output_dir, flagbackup=flagbackup, autocorr=autocorr,
		    shadow=shadow, scan=scan, scannumber=scannumber, intents=intents,
		    edgespw=edgespw, fracspw=fracspw, online=online,
		    fileonline=fileonline, template=template, filetemplate=filetemplate )

                self.fracspwfps  = fracspwfps

        @property
        def fracspwfps(self):
                return self._fracspwfps

        @fracspwfps.setter
        def fracspwfps(self, value):
                if value is None:
                     value = 0.048387
                self._fracspwfps = value


# -----------------------------------------------------------------------------

# ::to_casa_args

# Description:
# ------------
# This public member function translates the input parameters of this class to
# task parameters and file-based flag commands required by CASA task tflagdata.

# NB: This public member function is overloaded.

# Inputs:
# -------
# None.

# Outputs:
# --------
# The python dictionary containing the arguments (and their values) for CASA
# task tflagdata, returned via the function value.  The temporary file that
# contains the flagging commands for the tflagdata task, located in the output
# directory.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

	def to_casa_args( self ):

		# Initialize the arguments from the inherited
		# FlagDeterBaseInputs() class

		task_args = super( FlagDeterALMAInputs, self ).to_casa_args()


		# Return the tflagdata task arguments

		return task_args

# ------------------------------------------------------------------------------
# class FlagDeterALMAResults
# ------------------------------------------------------------------------------

# FlagDeterALMAResults

# Description:
# ------------
# This class manages the results from the FlagDeterALMA() class.

# Inherited classes:
# ------------------
# FlagDeterBaseResults - This class manages the results from the FlagDeterBase()
#                        class.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version created with no new member functions.

# ------------------------------------------------------------------------------

class FlagDeterALMAResults( flagdeterbase.FlagDeterBaseResults ):
	pass

# ------------------------------------------------------------------------------
# class FlagDeterALMA
# ------------------------------------------------------------------------------

# FlagDeterALMA

# Description:
# ------------
# This class represents the pipeline interface to the CASA task tflagdata.

# Inherited classes:
# ------------------
# FlagDeterBase - This class represents the pipeline interface to the CASA task
#                 tflagdata.

# Public member functions:
# ------------------------
# All public member functions from the FlagDeterALMAInputs() class.

# Modification history:
# ---------------------
# 2012 May 10 - Nick Elias, NRAO
#               Initial version.

# ------------------------------------------------------------------------------

class FlagDeterALMA( flagdeterbase.FlagDeterBase ):

# ------------------------------------------------------------------------------

	# Make the member functions of the FlagDeterALMAInputs() class member
	# functions of this class

	Inputs = FlagDeterALMAInputs

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



