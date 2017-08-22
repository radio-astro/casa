# General imports

import traceback

# Make sure CASA exceptions are rethrown
try:
    if not __rethrow_casa_exceptions:
        def_rethrow = False
    else:
        def_rethrow = __rethrow_casa_exceptions
except:
    def_rethrow = False

__rethrow_casa_exceptions = False

# CASA imports
from h_init_cli import h_init_cli as h_init
from hifv_importdata_cli import hifv_importdata_cli as hifv_importdata
from hif_editimlist_cli import hif_editimlist_cli as hif_editimlist
from hif_transformimagedata import hif_transformimagedata as hif_transformimagedata
from hif_makeimages_cli import hif_makeimages_cli as hif_makeimages
from hifv_exportdata_cli import hifv_exportdata_cli as hifv_exportdata
from h_save_cli import h_save_cli as h_save

# Pipeline imports
import pipeline.infrastructure.casatools as casatools

# IMPORT_ONLY = 'Import only'
IMPORT_ONLY = ''


# Run the procedure
def vlassQLIP(vislist, editimlist_infile, importonly=False, pipelinemode='automatic', interactive=True):
    echo_to_screen = interactive
    casatools.post_to_log("Beginning VLA Sky Survey quick look imaging pipeline run ...")

    try:
        # Initialize the pipeline
        h_init(plotlevel='summary')

        # Load the data
        hifv_importdata(vis=vislist, pipelinemode=pipelinemode, nocopy=True)
        if importonly:
            raise Exception(IMPORT_ONLY)

        # add imaging target
        hif_editimlist(parameter_file=editimlist_infile)

        # split out selected target data from full MS
        hif_transformimagedata(datacolumn="corrected", clear_pointing=True, modify_weights=False)

        # run tclean and create images
        hif_makeimages(pipelinemode=pipelinemode, hm_cleaning='manual', hm_masking='none')

        # apply a primary beam correction on images before rms and cutouts
        hifv_pbcor(pipelinemode=pipelinemode)

        # make uncertainty (rms) image
        hif_makermsimages(pipelinemode=pipelinemode)

        # make sub-images of final, primary beam, rms and psf images.
        hif_makecutoutimages(pipelinemode=pipelinemode)

        # Export the data to products directory
        #hifv_exportdata(pipelinemode=pipelinemode)

    except Exception, e:
        if str(e) == IMPORT_ONLY:
            casatools.post_to_log("Exiting after import step ...", echo_to_screen=echo_to_screen)
        else:
            casatools.post_to_log("Error in procedure execution ...", echo_to_screen=echo_to_screen)
            errstr = traceback.format_exc()
            casatools.post_to_log(errstr, echo_to_screen=echo_to_screen)

    finally:

        # Save the results to the context
        h_save()

        casatools.post_to_log("VLASS quick look imaging pipeline finished.  Terminating procedure execution ...",
                              echo_to_screen=echo_to_screen)

        # Restore previous state
        __rethrow_casa_exceptions = def_rethrow