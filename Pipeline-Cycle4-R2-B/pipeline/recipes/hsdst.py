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
#sys.path.insert (0, os.path.expandvars("$SCIPIPE_HEURISTICS"))
#execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/h/cli/mytasks.py"))
#execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hif/cli/mytasks.py"))
#execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hifa/cli/mytasks.py"))
#execfile(os.path.join( os.path.expandvars("$SCIPIPE_HEURISTICS"), "pipeline/hsd/cli/mytasks.py"))

# CASA imports
from h_init_cli import h_init_cli as h_init
from hsd_importdata_cli import hsd_importdata_cli as hsd_importdata
from hsd_flagdata_cli import hsd_flagdata_cli as hsd_flagdata
from hifa_tsyscal_cli import hifa_tsyscal_cli as hifa_tsyscal
from hifa_tsysflag_cli import hifa_tsysflag_cli as hifa_tsysflag
from hsdst_mstoscantable_cli import hsdst_mstoscantable_cli as hsdst_mstoscantable
from hsdst_inspectdata_cli import hsdst_inspectdata_cli as hsdst_inspectdata
from hsdst_calsky_cli import hsdst_calsky_cli as hsdst_calsky
from hsdst_applycal_cli import hsdst_applycal_cli as hsdst_applycal
from hsdst_baseline_cli import hsdst_baseline_cli as hsdst_baseline
from hsdst_blflag_cli import hsdst_blflag_cli as hsdst_blflag
from hsdst_imaging_cli import hsdst_imaging_cli as hsdst_imaging
from hsdst_exportdata_cli import hsdst_exportdata_cli as hsdst_exportdata
from h_save_cli import h_save_cli as h_save


# Pipeline imports
import pipeline.infrastructure.casatools as casatools

IMPORT_ONLY = 'Import only'

ITERATION = 2

# Run the procedure
def hsd (vislist, importonly=False, pipelinemode='automatic', interactive=True):

    echo_to_screen = interactive
    casatools.post_to_log ("Beginning pipeline run ...")

    try:
        # Initialize the pipeline
        h_init()

        # Load the data
        hsd_importdata (vis=vislist, pipelinemode=pipelinemode)
        if importonly:
            raise Exception(IMPORT_ONLY)
       
        # Deterministic flagging
        hsd_flagdata (pipelinemode=pipelinemode)
        
        # Tsys calibration
        hifa_tsyscal (pipelinemode=pipelinemode)
        
        # Flag system temperature calibration
        hifa_tsysflag (pipelinemode=pipelinemode)
        
        # Convert MSs to Scantables
        hsdst_mstoscantable (pipelinemode=pipelinemode)
    
        # Inspect data
        hsdst_inspectdata (pipelinemode=pipelinemode)
    
        # Compute the sky calibration
        hsdst_calsky (pipelinemode=pipelinemode)
        
        # Apply the calibrations
        hsdst_applycal (pipelinemode=pipelinemode)
        
        # Improve line mask for baseline subtraction by executing 
        # hsd_baseline and hsd_blflag iteratively
        for i in xrange(ITERATION):
            
            # Baseline subtraction with automatic line detection
            hsdst_baseline (pipelinemode=pipelinemode)
            
            # Flag data based on baseline quality
            hsdst_blflag (pipelinemode=pipelinemode)
            
        # Imaging
        hsdst_imaging (pipelinemode=pipelinemode)

        # Export the data
        hsdst_exportdata (pipelinemode=pipelinemode)
    
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

