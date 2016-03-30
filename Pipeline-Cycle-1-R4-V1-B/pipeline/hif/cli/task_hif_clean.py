import sys

from taskinit import casalog

import pipeline.h.cli.utils as utils


def hif_clean(vis=None, intent=None, field=None, spw=None,
              imagename=None, mode=None, imagermode=None, outframe=None, 
              nchan=None, start=None, width=None, imsize=None, cell=None,
              phasecenter=None, restfreq=None, weighting=None,
              robust=None, noise=None, report=None,
              pipelinemode=None, dryrun=None, acceptresults=None):

    # create a dictionary containing all the arguments given in the
    # constructor
    all_inputs = vars()

    task_name = 'Clean'

    ##########################################################################
    #                                                                        #
    #  CASA task interface boilerplate code starts here. No edits should be  #
    #  needed beyond this point.                                             #
    #                                                                        #
    ##########################################################################
    
    # get the name of this function for the weblog, eg. 'hif_flagdata'
    fn_name = sys._getframe().f_code.co_name

    # get the context on which this task operates
    context = utils.get_context()
    
    # execute the task    
    results = utils.execute_task(context, task_name, all_inputs, fn_name)

    return results