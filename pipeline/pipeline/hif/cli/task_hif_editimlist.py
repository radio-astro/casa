import sys

import pipeline.h.cli.utils as utils


def hif_editimlist(vis=None,
                   imagename=None,
                   intent=None,
                   field=None,
                   spw=None,
                   contfile=None,
                   linesfile=None,
                   uvrange=None,
                   specmode=None,
                   outframe=None,
                   imsize=None,
                   cell=None,
                   phasecenter=None,
                   nchan=None,
                   start=None,
                   width=None,
                   nbin=None,
                   stokes=None,
                   parameter_file=False,
                   nterms=None,
                   editmode=None,
                   pipelinemode=None,
                   dryrun=None,
                   acceptresults=None):

    # create a dictionary containing all the arguments given in the
    # constructor
    all_inputs = vars()

    task_name = 'Editimlist'

    #                                                                        #
    #  CASA task interface boilerplate code starts here. No edits should be  #
    #  needed beyond this point.                                             #
    #                                                                        #

    # get the name of this function for the weblog, eg. 'hif_flagdata'
    fn_name = sys._getframe().f_code.co_name

    # get the context on which this task operates
    context = utils.get_context()

    # execute the task
    results = utils.execute_task(context, task_name, all_inputs, fn_name)

    return results