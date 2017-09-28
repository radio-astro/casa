# General imports

import os
import sys
import traceback
import inspect

# Make sure CASA exceptions are rethrown
try:
    if not __rethrow_casa_exceptions:
        def_rethrow = False
    else:
        def_rethrow = __rethrow_casa_exceptions
except:
    def_rethrow = False

__rethrow_casa_exceptions = True

# CASA imports
#     Clunky but import casa does not work for pipeline tasks
from h_init_cli import h_init_cli as h_init
from hifa_importdata_cli import hifa_importdata_cli as hifa_importdata
from hifa_flagdata_cli import hifa_flagdata_cli as hifa_flagdata
from hifa_fluxcalflag_cli import hifa_fluxcalflag_cli as hifa_fluxcalflag
from hif_rawflagchans_cli import hif_rawflagchans_cli as hif_rawflagchans
# from hifa_fluxdb_cli import hifa_fluxdb_cli as hifa_fluxdb
from hif_refant_cli import hif_refant_cli as hif_refant
from h_tsyscal_cli import h_tsyscal_cli as h_tsyscal
from hifa_tsysflag_cli import hifa_tsysflag_cli as hifa_tsysflag
from hifa_antpos_cli import hifa_antpos_cli as hifa_antpos
from hifa_wvrgcalflag_cli import hifa_wvrgcalflag_cli as hifa_wvrgcalflag
from hif_lowgainflag_cli import hif_lowgainflag_cli as hif_lowgainflag
from hif_setmodels_cli import hif_setmodels_cli as hif_setmodels
from hifa_bandpassflag_cli import hifa_bandpassflag_cli as hifa_bandpassflag
from hifa_spwphaseup_cli import hifa_spwphaseup_cli as hifa_spwphaseup
from hifa_gfluxscaleflag_cli import hifa_gfluxscaleflag_cli as hifa_gfluxscaleflag
from hifa_gfluxscale_cli import hifa_gfluxscale_cli as hifa_gfluxscale
from hifa_timegaincal_cli import hifa_timegaincal_cli as hifa_timegaincal
from hif_applycal_cli import hif_applycal_cli as hif_applycal
from hifa_imageprecheck_cli import hifa_imageprecheck_cli as hifa_imageprecheck
from hif_checkproductsize_cli import hif_checkproductsize_cli as hif_checkproductsize
# from hif_makecleanlist_cli import hif_makecleanlist_cli as hif_makecleanlist
from hif_makeimlist_cli import hif_makeimlist_cli as hif_makeimlist
# from hif_cleanlist_cli import hif_cleanlist_cli as hif_cleanlist
from hif_makeimages_cli import hif_makeimages_cli as hif_makeimages
from hifa_exportdata_cli import hifa_exportdata_cli as hifa_exportdata
from h_save_cli import h_save_cli as h_save

# Pipeline imports
import pipeline.infrastructure.casatools as casatools

IMPORT_ONLY = 'Import only'


# Run the procedure
def hifacal(vislist, importonly=True, pipelinemode='automatic',
            interactive=True):

    echo_to_screen = interactive
    casatools.post_to_log("Beginning pipeline run ...")

    try:
        # Initialize the pipeline
        h_init()

        # Load the data
        hifa_importdata(vis=vislist, dbservice=True, pipelinemode=pipelinemode)
        if importonly:
            raise Exception(IMPORT_ONLY)
    
        # Flag known bad data
        hifa_flagdata(pipelinemode=pipelinemode)
    
        # Flag lines in solar system calibrators and compute the default
        # reference spectral window map.
        hifa_fluxcalflag(pipelinemode=pipelinemode)

        # Flag bad channels in the raw data
        hif_rawflagchans(pipelinemode=pipelinemode)
    
        # Compute the prioritized lists of reference antennas
        hif_refant(pipelinemode=pipelinemode)
    
        # Compute the system temperature calibration
        h_tsyscal(pipelinemode=pipelinemode)
    
        # Flag system temperature calibration
        hifa_tsysflag(pipelinemode=pipelinemode)

        # Flag system temperature calibration
        hifa_antpos(pipelinemode=pipelinemode)

        # Compute the WVR calibration, flag and interpolate over bad antennas
        hifa_wvrgcalflag(pipelinemode=pipelinemode)
    
        # Flag antennas with low gain
        hif_lowgainflag(pipelinemode=pipelinemode)

        # Set the flux calibrator model
        hif_setmodels(pipelinemode=pipelinemode)
    
        # Compute the bandpass calibration
        hifa_bandpassflag(pipelinemode=pipelinemode)

        # Compute the bandpass calibration
        hifa_spwphaseup(pipelinemode=pipelinemode)

        # Derive the flux density scale from standard calibrators, and flag
        # outliers in corrected - model amplitudes for flux and phase
        # calibrators.
        hifa_gfluxscaleflag(pipelinemode=pipelinemode)

        # Determine flux values for the bandpass and gain calibrators
        # assuming point sources and set their model fluxes
        hifa_gfluxscale(pipelinemode=pipelinemode)
    
        # Compute the time dependent gain calibration
        hifa_timegaincal(pipelinemode=pipelinemode)
    
        # Apply the calibrations
        hif_applycal(pipelinemode=pipelinemode)
    
        # Check imaging parameters against PI specified values
        hifa_imageprecheck(pipelinemode=pipelinemode)
    
        # Make a list of expected point source calibrators to be cleaned
        hif_makeimlist(intent='PHASE,BANDPASS,CHECK', pipelinemode=pipelinemode)
    
        # Make clean images for the selected calibrators
        hif_makeimages(pipelinemode=pipelinemode)

        # Check product size limits and mitigate imaging parameters
        hif_checkproductsize(maxcubesize=30.0, maxcubelimit=40.0, maxproductsize=400.0)
    
        # Export the data
        hifa_exportdata(pipelinemode=pipelinemode)
    
    except Exception, e:
        if str(e) == IMPORT_ONLY:
            casatools.post_to_log("Exiting after import step ...",
                                  echo_to_screen=echo_to_screen)
        else:
            casatools.post_to_log("Error in procedure execution ...",
                                  echo_to_screen=echo_to_screen)
            errstr = traceback.format_exc()
            casatools.post_to_log(errstr, echo_to_screen=echo_to_screen)

    finally:

        # Save the results to the context
        h_save()

        casatools.post_to_log("Terminating procedure execution ...",
                              echo_to_screen=echo_to_screen)

        # Restore previous state
        __rethrow_casa_exceptions = def_rethrow
