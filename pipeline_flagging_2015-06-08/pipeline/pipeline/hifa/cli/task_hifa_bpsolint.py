from taskinit import casalog
import sys

import pipeline.h.cli.utils as utils

def hifa_bpsolint(vis=None, field=None, intent=None, spw=None, phaseupsnr=None,
          minphaseupints=None, bpsnr=None, minbpnchan=None, hm_nantennas=None,
	  maxfracflagged=None, pipelinemode=None, dryrun=None,
	  acceptresults=None):

    # create a dictionary containing all the arguments given in the
    # constructor
    all_inputs = vars()

    # set the name of the pipeline task to execute 
    task_name = 'BpSolint'

    ##########################################################################
    #                                                                        #
    #  CASA task interface boilerplate code starts here. No edits should be  #
    #  needed beyond this point.                                             #
    #                                                                        #
    ##########################################################################
    
    # get the name of this function for the weblog, eg. 'hifa_flagdata'
    fn_name = sys._getframe().f_code.co_name

    # get the context on which this task operates
    context = utils.get_context()
    
    # execute the task    
    results = utils.execute_task(context, task_name, all_inputs, fn_name)

    return results
