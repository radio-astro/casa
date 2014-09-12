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
from hsd_inspectdata_cli import hsd_inspectdata_cli as hsd_inspectdata
from hsd_caltsys_cli import hsd_caltsys_cli as hsd_caltsys
from hsd_calsky_cli import hsd_calsky_cli as hsd_calsky
from hsd_applycal_cli import hsd_applycal_cli as hsd_applycal
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
        hsd_importdata (infiles=vislist, pipelinemode=pipelinemode)
        if importonly:
            raise Exception(IMPORT_ONLY)
    
        # Inspect data
        hsd_inspectdata (pipelinemode=pipelinemode)
    
        # Compute the system temperature calibration
        hsd_caltsys (pipelinemode=pipelinemode)

        # Compute the sky calibration
        hsd_calsky (pipelinemode=pipelinemode, calmode='otfraster')
        
        # Apply the calibrations
        hsd_applycal (pipelinemode=pipelinemode)
    
        # Imaging
        hsd_imaging (pipelinemode=pipelinemode)

        # Export the data
        hsd_exportdata(pipelinemode=pipelinemode)
    
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

