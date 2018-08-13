import sys

import pipeline.h.cli.utils as utils


def hif_editimlist(imagename=None,
                   search_radius_arcsec=None,
                   cell=None,
                   conjbeams=None,
                   cyclefactor=None,
                   cycleniter=None,
                   deconvolver=None,
                   editmode=None,
                   field=None,
                   imaging_mode=None,
                   imsize=None,
                   intent=None,
                   gridder=None,
                   mask=None,
                   nbin=None,
                   nchan=None,
                   niter=None,
                   nterms=None,
                   parameter_file=None,
                   phasecenter=None,
                   reffreq=None,
                   robust=None,
                   scales=None,
                   specmode=None,
                   spw=None,
                   start=None,
                   stokes=None,
                   sensitivity=None,
                   threshold=None,
                   threshold_nsigma=None,
                   uvtaper=None,
                   uvrange=None,
                   width=None,
                   pipelinemode=None,
                   dryrun=None,
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
