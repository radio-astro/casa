from taskinit import casalog
import sys

import pipeline.h.cli.utils as utils

def hifa_gaincalsnr(vis=None, field=None, intent=None, spw=None, phasesnr=None,
          bwedgefrac=None, hm_nantennas=None, maxfracflagged=None, pipelinemode=None, dryrun=None,
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
