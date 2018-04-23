import sys

from taskinit import casalog

import pipeline.h.cli.utils as utils


def hif_makeimlist(vis=None, imagename=None, intent=None, field=None,
                   spw=None, contfile=None, linesfile=None, uvrange=None,
                   specmode=None, outframe=None, hm_imsize=None, hm_cell=None,
                   calmaxpix=None, phasecenter=None,
                   nchan=None, start=None, width=None, nbins=None, clearlist=None,
                   per_eb=None, pipelinemode=None, dryrun=None, acceptresults=None):

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
