import sys

from taskinit import casalog

import pipeline.h.cli.utils as utils


def hifa_tsysflag(vis=None, caltable=None, metric=None,
  intentgroups=None, refintent=None,
  flag_hi=None, fhi_limit=None, fhi_minsample=None,
  flag_maxabs=None, fmax_limit=None, flag_minabs=None, fmin_limit=None,
  flag_nmedian=None, fnm_limit=None, 
  flag_tmf1=None, tmf1_axis=None, tmf1_limit=None,
  niter=None,
  pipelinemode=None, dryrun=None, acceptresults=None):

    # create a dictionary containing all the arguments given in the
    # constructor
    all_inputs = vars()

    task_name = 'Tsysflag'

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
