import sys

from taskinit import casalog

import pipeline.h.cli.utils as utils


def hifa_tsysflag(vis=None, caltable=None,
  flag_nmedian=None, fnm_limit=None,
  flag_derivative=None, fd_max_limit=None,
  flag_edgechans=None, fe_edge_limit=None,
  flag_fieldshape=None, ff_refintent=None, ff_max_limit=None,
  ff_tmf1_limit=None,
  flag_birdies=None, fb_sharps_limit=None,
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
