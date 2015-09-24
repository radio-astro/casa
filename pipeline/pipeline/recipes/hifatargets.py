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

# CASA imports
#     Clunky but import casa does not work for pipeline tasks
from h_init_cli import h_init_cli as h_init
from hifa_importdata_cli import hifa_importdata_cli as hifa_importdata
from hif_findcont_cli import hif_findcont_cli as hif_findcont
from hif_makeimlist_cli import hif_makeimlist_cli as hif_makeimlist
from hif_makeimages_cli import hif_makeimages_cli as hif_makeimages
from hif_exportdata_cli import hif_exportdata_cli as hif_exportdata
from h_save_cli import h_save_cli as h_save

# Pipeline imports
import pipeline.infrastructure.casatools as casatools

IMPORT_ONLY = 'Import only'

# Run the procedure
def hifatargets (vislist, importonly=False, pipelinemode='automatic', interactive=True):

    echo_to_screen = interactive
    casatools.post_to_log ("Beginning pipeline target imaging run ...")

    try:
        # Initialize the pipeline
        h_init()

        # Load the data
        hifa_importdata (vis=vislist, pipelinemode=pipelinemode)
        if importonly:
            raise Exception(IMPORT_ONLY)

        # Make a list of expected targets to be cleaned in cont (aggregate over all spws) mode
        hif_makeimlist (specmode='cont', pipelinemode=pipelinemode)
 
        # Find continuum frequency ranges
        hif_findcont(pipelinemode=pipelinemode)

        # Make clean cont images for the selected targets
        hif_makeimages (pipelinemode=pipelinemode)

        # Make a list of expected targets to be cleaned in mfs mode (used for continuum subtraction)
        hif_makeimlist (specmode='mfs', pipelinemode=pipelinemode)
 
        # Make clean mfs images for the selected targets
        hif_makeimages (pipelinemode=pipelinemode)

        # Make a list of expected targets to be cleaned in continuum subtracted cube mode
        hif_makeimlist (pipelinemode=pipelinemode)

        # Make clean continuum subtracted cube images for the selected targets
        hif_makeimages (pipelinemode=pipelinemode)

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
