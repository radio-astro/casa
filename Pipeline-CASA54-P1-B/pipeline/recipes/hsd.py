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
from h_init_cli import h_init_cli as h_init
from hsd_importdata_cli import hsd_importdata_cli as hsd_importdata
from hsd_flagdata_cli import hsd_flagdata_cli as hsd_flagdata
from h_tsyscal_cli import h_tsyscal_cli as h_tsyscal
from hsd_tsysflag_cli import hsd_tsysflag_cli as hsd_tsysflag
from hsd_skycal_cli import hsd_skycal_cli as hsd_skycal
from hsd_k2jycal_cli import hsd_k2jycal_cli as hsd_k2jycal
from hsd_applycal_cli import hsd_applycal_cli as hsd_applycal
from hsd_baseline_cli import hsd_baseline_cli as hsd_baseline
from hsd_blflag_cli import hsd_blflag_cli as hsd_blflag
from hsd_imaging_cli import hsd_imaging_cli as hsd_imaging
from hsd_exportdata_cli import hsd_exportdata_cli as hsd_exportdata
from h_save_cli import h_save_cli as h_save


# Pipeline imports
import pipeline.infrastructure.casatools as casatools

IMPORT_ONLY = 'Import only'

ITERATION = 2


# Run the procedure
def hsdms(vislist, importonly=False, pipelinemode='automatic',
          interactive=True):

    echo_to_screen = interactive
    casatools.post_to_log("Beginning pipeline run ...")

    try:
        # Initialize the pipeline
        h_init()

        # Load the data
        hsd_importdata(vis=vislist, pipelinemode=pipelinemode)
        if importonly:
            raise Exception(IMPORT_ONLY)
       
        # Deterministic flagging
        hsd_flagdata(pipelinemode=pipelinemode)
        
        # Tsys calibration
        h_tsyscal(pipelinemode=pipelinemode)
        
        # Flag system temperature calibration
        hsd_tsysflag(pipelinemode=pipelinemode)
        
        # Compute the sky calibration
        hsd_skycal(pipelinemode=pipelinemode)
    
        # Compute the Kelvin to Jansky calibration
        hsd_k2jycal(pipelinemode=pipelinemode)
    
        # Apply the calibrations
        hsd_applycal(pipelinemode=pipelinemode)
        
        # # Improve line mask for baseline subtraction by executing 
        # # hsd_baseline and hsd_blflag iteratively
        for i in xrange(ITERATION):
            
            # Baseline subtraction with automatic line detection
            hsd_baseline(pipelinemode=pipelinemode)
            
            # Flag data based on baseline quality
            hsd_blflag(pipelinemode=pipelinemode)
            
        # Imaging
        hsd_imaging(pipelinemode=pipelinemode)

        # Export the data
        hsd_exportdata(pipelinemode=pipelinemode)
    
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
