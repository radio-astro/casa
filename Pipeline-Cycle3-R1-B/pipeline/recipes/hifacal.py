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

# Setup paths
# Should no longer be needed
#sys.path.insert (0, os.path.expandvars("$SCIPIPE_HEURISTICS"))
#execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/h/cli/h.py"))
#execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hif/cli/hif.py"))
#execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hifa/cli/hifa.py"))

# CASA imports
#     Clunky but import casa does not work for pipeline tasks
from h_init_cli import h_init_cli as h_init
from hifa_importdata_cli import hifa_importdata_cli as hifa_importdata
from hifa_flagdata_cli import hifa_flagdata_cli as hifa_flagdata
from hifa_fluxcalflag_cli import hifa_fluxcalflag_cli as hifa_fluxcalflag
from hifa_fluxdb_cli import hifa_fluxdb_cli as hifa_fluxdb
from hif_refant_cli import hif_refant_cli as hif_refant
from hifa_tsyscal_cli import hifa_tsyscal_cli as hifa_tsyscal
from hifa_tsysflag_cli import hifa_tsysflag_cli as hifa_tsysflag
from hifa_wvrgcalflag_cli import hifa_wvrgcalflag_cli as hifa_wvrgcalflag
from hif_lowgainflag_cli import hif_lowgainflag_cli as hif_lowgainflag
from hif_setjy_cli import hif_setjy_cli as hif_setjy
from hif_bandpass_cli import hif_bandpass_cli as hif_bandpass
from hif_bpflagchans_cli import hif_bpflagchans_cli as hif_bpflagchans
from hifa_gfluxscale_cli import hifa_gfluxscale_cli as hifa_gfluxscale
from hifa_timegaincal_cli import hifa_timegaincal_cli as hifa_timegaincal
from hif_applycal_cli import hif_applycal_cli as hif_applycal
from hif_makecleanlist_cli import hif_makecleanlist_cli as hif_makecleanlist
from hif_cleanlist_cli import hif_cleanlist_cli as hif_cleanlist
from hif_exportdata_cli import hif_exportdata_cli as hif_exportdata
from h_save_cli import h_save_cli as h_save

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
        hifa_tsysflag (pipelinemode=pipelinemode)

        # Compute the WVR calibration, flag and interpolate over bad antennas
        hifa_wvrgcalflag (pipelinemode=pipelinemode)
    
        # Flag antennas with low gain
        hif_lowgainflag (pipelinemode=pipelinemode)
    
        # Set the flux calibrator model
        hif_setjy (pipelinemode=pipelinemode)
    
        # Compute the bandpass calibration
        hif_bandpass (pipelinemode=pipelinemode)

        # Flag deviant channels in the bandpass calibration
        hif_bpflagchans (pipelinemode=pipelinemode)
    
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

