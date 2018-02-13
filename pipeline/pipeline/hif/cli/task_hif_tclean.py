import sys

from taskinit import casalog

import pipeline.h.cli.utils as utils


def hif_tclean(vis=None, imagename=None, intent=None, field=None, spw=None, spwsel_lsrk=None, spwsel_topo=None,
               uvrange=None, specmode=None, gridder=None, deconvolver=None, nterms=None, outframe=None, imsize=None,
               cell=None, phasecenter=None, stokes=None, nchan=None, start=None, width=None, nbin=None, weighting=None,
               robust=None, noise=None, npixels=None, restoringbeam=None, hm_masking=None, hm_sidelobethreshold=None,
               hm_noisethreshold=None, hm_lownoisethreshold=None, hm_negativethreshold=None, hm_minbeamfrac=None,
               hm_growiterations=None, hm_cleaning=None, mask=None, niter=None, threshold=None, tlimit=None,
               masklimit=None, maxncleans=None, cleancontranges=None, parallel=None, pipelinemode=None, dryrun=None,
               acceptresults=None):

    ##########################################################################
    #                                                                        #
    #  CASA task interface boilerplate code starts here. No edits should be  #
    #  needed beyond this point.                                             #
    #                                                                        #
    ##########################################################################

    # create a dictionary containing all the arguments given in the
    # constructor
    all_inputs = vars()

    # get the name of this function for the weblog, eg. 'hif_flagdata'
    task_name = sys._getframe().f_code.co_name

    # get the context on which this task operates
    context = utils.get_context()

    # execute the task
    results = utils.execute_task(context, task_name, all_inputs)

    return results
