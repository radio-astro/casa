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
from hifa_imageprecheck_cli import hifa_imageprecheck_cli as hifa_imageprecheck
from hif_checkproductsize_cli import hif_checkproductsize_cli as hif_checkproductsize
from hif_mstransform_cli import hif_mstransform_cli as hif_mstransform
from hifa_flagtargets_cli import hifa_flagtargets_cli as hifa_flagtargets
from hif_findcont_cli import hif_findcont_cli as hif_findcont
from hif_uvcontfit_cli import hif_uvcontfit_cli as hif_uvcontfit
from hif_uvcontsub_cli import hif_uvcontsub_cli as hif_uvcontsub
from hif_makeimlist_cli import hif_makeimlist_cli as hif_makeimlist
from hif_makeimages_cli import hif_makeimages_cli as hif_makeimages
from hifa_exportdata_cli import hifa_exportdata_cli as hifa_exportdata
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
        hifa_importdata (vis=vislist, dbservice=True, pipelinemode=pipelinemode)
        if importonly:
            raise Exception(IMPORT_ONLY)

        # Split out the target data
        hif_mstransform (pipelinemode=pipelinemode)

        # Flag the target data
        hifa_flagtargets (pipelinemode=pipelinemode)
 
        # Check imaging parameters against PI specified values
        hifa_imageprecheck(pipelinemode=pipelinemode)

        # Check product size limits and mitigate imaging parameters
        hif_checkproductsize(maxcubesize=40.0, maxcubelimit=60.0, maxproductsize=350.0)

        # Make a list of expected targets to be cleaned in mfs mode (used for continuum subtraction)
        hif_makeimlist (specmode='mfs', pipelinemode=pipelinemode)
 
        # Find continuum frequency ranges
        hif_findcont(pipelinemode=pipelinemode)

        # Fit the continuum using frequency ranges from hif_findcont
        hif_uvcontfit(pipelinemode=pipelinemode)

        # Subtract the continuum fit
        hif_uvcontsub(pipelinemode=pipelinemode)

        # Make clean mfs images for the selected targets
        hif_makeimages (pipelinemode=pipelinemode)

        # Make a list of expected targets to be cleaned in cont (aggregate over all spws) mode
        hif_makeimlist (specmode='cont', pipelinemode=pipelinemode)
 
        # Make clean cont images for the selected targets
        hif_makeimages (pipelinemode=pipelinemode)

        # Make a list of expected targets to be cleaned in continuum subtracted cube mode
        hif_makeimlist (pipelinemode=pipelinemode)

        # Make clean continuum subtracted cube images for the selected targets
        hif_makeimages (pipelinemode=pipelinemode)

        # Make a list of expected targets to be cleaned in continuum subtracted PI cube mode
        hif_makeimlist (specmode='repBW', pipelinemode=pipelinemode)

        # Make clean continuum subtracted PI cube
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
