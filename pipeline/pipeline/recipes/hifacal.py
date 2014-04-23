# General imports

import os
import sys
import traceback
import inspect

# Make sure CASA exceptions are rethrown
try:
    if not  __rethrow_casa_exceptions:
        def_rethrow = False
    else:
        def_rethrow = __rethrow_casa_exceptions
except:
    def_rethrow = False

__rethrow_casa_exceptions = True

#myf=sys._getframe(len(inspect.stack())-1).f_globals
#try:
    #default__rethrow_casa_exceptions = myf['__rethrow_casa_exceptions']
#except Exception, e:
    #default__rethrow_casa_exceptions = False
    #myf['__rethrow_casa_exceptions'] = True

# Setup paths
sys.path.insert (0, os.path.expandvars("$SCIPIPE_HEURISTICS"))
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/h/cli/h.py"))
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hif/cli/hif.py"))
execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hifa/cli/hifa.py"))

# Pipeline imports
import pipeline.infrastructure.casatools as casatools

IMPORT_ONLY = 'Import only'

# Run the procedure
def hifacal (vislist, importonly=True, pipelinemode='automatic', interactive=True):

    echo_to_screen = interactive
    casatools.post_to_log ("Beginning pipeline run ...")

    try:
	# Initialize the pipeline
        h_init()

        # Load the data
        hifa_importdata (vis=vislist, pipelinemode=pipelinemode)
	if importonly:
	   raise Exception(IMPORT_ONLY)
    
        # Flag known bad data
        hifa_flagdata (pipelinemode=pipelinemode)
    
        # Flag lines in solar system calibrators and compute the default
	# reference spectral window map.
        hifa_fluxcalflag (pipelinemode=pipelinemode)
    
        # Compute the prioritized lists of reference antennas
        hif_refant (pipelinemode=pipelinemode)
    
        # Compute the system temperature calibration
        hifa_tsyscal (pipelinemode=pipelinemode)
    
        # Flag system temperature calibration

	# Tsys spectra with extreme values
        hifa_tsysflag (pipelinemode=pipelinemode)

	# Tsys spectra with extreme derivative values
        hifa_tsysflag (metric='derivative', flag_maxabs=True, fmax_limit=2,
            flag_nmedian=False, pipelinemode=pipelinemode)

	# Deviant Tsys edge channels
        hifa_tsysflagchans (intentgroups=['ATMOSPHERE', 'BANDPASS', 'AMPLITUDE'],
	    pipelinemode=pipelinemode)

	# Tsys shapes significantly different from reference
        hifa_tsysflag (metric='fieldshape', refintent='BANDPASS',
	    flag_maxabs=True, fmax_limit=5, flag_tmf1=True,
	    tmf1_axis='Antenna1', tmf1_limit=0.666,
            flag_nmedian=False, pipelinemode=pipelinemode)

	# Deviant non-ede Tsys channels, e.g. birdies
        hifa_tsysflagchans (metric='antenna_diff', flag_edges=False,
            flag_sharps2=True, sharps2_limit=0.1, pipelinemode=pipelinemode)
    
        # Compute the WVR calibration, flag and interpolate over bad antennas
        hifa_wvrgcalflag (pipelinemode=pipelinemode)
    
        # Flag antennas with low gain
        hif_lowgainflag (pipelinemode=pipelinemode)
    
        # Set the flux calibrator model
        hif_setjy (pipelinemode=pipelinemode)
    
        # Compute the bandpass calibration
        hif_bandpass (pipelinemode=pipelinemode)

        # Flag deviant channels in the bandpass calibration
        hif_bpflagchans (diffmad_nchan_limit=10000, pipelinemode=pipelinemode)
    
        # Determine flux values for the bandpass and gain calibrators
        # assuming point sources and set their model fluxes
        hifa_gfluxscale (pipelinemode=pipelinemode)
    
        # Compute the time dependent gain calibration
        hifa_timegaincal (pipelinemode=pipelinemode)
    
        # Apply the calibrations
        hif_applycal (pipelinemode=pipelinemode)
    
        # Make a list of expected point source calibrators to be cleaned
        hif_makecleanlist (intent='PHASE,BANDPASS,CHECK', pipelinemode=pipelinemode)
    
        # Make clean images for the selected calibrators
        hif_cleanlist (pipelinemode=pipelinemode)
    
        # Export the data
        hif_exportdata(pipelinemode=pipelinemode)
    
    except Exception, e:
	if str(e) == IMPORT_ONLY:
	    casatools.post_to_log ("Exiting after import step ...",
	        echo_to_screen=echo_to_screen)
	else:
	    casatools.post_to_log ("Error in procedure execution ...",
	        echo_to_screen=echo_to_screen)
	    errstr = traceback.format_exc()
	    casatools.post_to_log (errstr, echo_to_screen=echo_to_screen)

    finally:

        # Save the results to the context
        h_save()

	casatools.post_to_log ("Terminating procedure execution ...",
	    echo_to_screen=echo_to_screen)

	# Restore previous state
	__rethrow_casa_exceptions = def_rethrow

