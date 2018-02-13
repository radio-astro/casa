import sys

from taskinit import casalog

import pipeline.h.cli.utils as utils


def hifa_session_bandpass(vis=None, caltable=None, field=None, intent=None, spw=None, antenna=None, hm_phaseup=None,
                          phaseupsolint=None, phaseupbw=None, phaseupsnr=None, phaseupnsols=None, hm_bandpass=None,
                          solint=None, maxchannels=None, evenbpints=None, bpsnr=None, bpnsols=None, hm_bandtype=None,
                          combine=None, refant=None, solnorm=None, minblperant=None, minsnr=None, degamp=None,
                          degphase=None, pipelinemode=None, dryrun=None, acceptresults=None, parallel=None):

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
