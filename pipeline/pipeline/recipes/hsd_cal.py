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
#    Should no longer be necessary
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
from hsd_mstoscantable_cli import hsd_mstoscantable_cli as hsd_mstoscantable
from hsd_inspectdata_cli import hsd_inspectdata_cli as hsd_inspectdata
from hsd_calsky_cli import hsd_calsky_cli as hsd_calsky
from hsd_applycal_cli import hsd_applycal_cli as hsd_applycal
from hsd_simplescale_cli import hsd_simplescale_cli as hsd_simplescale
from hsd_imaging_cli import hsd_imaging_cli as hsd_imaging
from hsd_exportdata_cli import hsd_exportdata_cli as hsd_exportdata
from h_save_cli import h_save_cli as h_save

# Pipeline imports
import pipeline.infrastructure.casatools as casatools

IMPORT_ONLY = 'Import only'

# Run the procedure
def hsd_cal (vislist, importonly=False, pipelinemode='automatic', interactive=True):

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
        hsd_mstoscantable (pipelinemode=pipelinemode)
    
        # Inspect data
        hsd_inspectdata (pipelinemode=pipelinemode)
    
        # Compute the sky calibration
        hsd_calsky (calmode='otfraster', pipelinemode=pipelinemode)
        
        # Apply the calibrations
        hsd_applycal (pipelinemode=pipelinemode)
    
        # Apply non-linearity correction
        hsd_simplescale (pipelinemode=pipelinemode)
    
        # Imaging
        hsd_imaging (pipelinemode=pipelinemode, mode='ampcal')

        # Export the data
        hsd_exportdata (pipelinemode=pipelinemode)
    
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

