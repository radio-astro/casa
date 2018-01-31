import sys

import pipeline.h.cli.utils as utils


def hifa_wvrgcalflag(
        vis=None, caltable=None, offsetstable=None, hm_toffset=None,
        toffset=None, segsource=None, sourceflag=None, hm_tie=None, tie=None,
        nsol=None, disperse=None, wvrflag=None, hm_smooth=None, smooth=None,
        scale=None, maxdistm=None, minnumants=None, mingoodfrac=None,
        refant=None, flag_intent=None, qa_intent=None,
        qa_bandpass_intent=None, accept_threshold=None, flag_hi=None,
        fhi_limit=None, fhi_minsample=None, ants_with_wvr_thresh=None,
        pipelinemode=None, dryrun=None, acceptresults=None):

    # create a dictionary containing all the arguments given in the
    # constructor
    all_inputs = vars()

    task_name = 'Wvrgcalflag'

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
