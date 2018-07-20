import sys

import pipeline.h.cli.utils as utils


def hifa_tsysflag(vis=None, caltable=None,
                  flag_nmedian=None, fnm_limit=None, fnm_byfield=None,
                  flag_derivative=None, fd_max_limit=None,
                  flag_edgechans=None, fe_edge_limit=None,
                  flag_fieldshape=None, ff_refintent=None, ff_max_limit=None,
                  flag_birdies=None, fb_sharps_limit=None,
                  flag_toomany=None, tmf1_limit=None, tmef1_limit=None,
                  metric_order=None, normalize_tsys=None, filetemplate=None,
                  pipelinemode=None, dryrun=None, acceptresults=None):

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
