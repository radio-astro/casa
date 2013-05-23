import sys

from taskinit import casalog

import pipeline.h.cli.utils as utils


def hif_fluxscale(vis=None, caltable=None, refintent=None, transintent=None,
                  dryrun=None, applyresults=None, pipelinemode=None,
                  fluxtable=None, reference=None, transfer=None):

    # create a dictionary containing all the arguments given in the
    # constructor
    all_inputs = vars()

    task_name = 'Fluxscale'

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
