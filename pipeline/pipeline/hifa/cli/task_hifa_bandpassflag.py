import sys

import pipeline.h.cli.utils as utils


def hifa_bandpassflag(
        vis=None, caltable=None,
        intent=None, field=None, spw=None, antenna=None,
        hm_phaseup=None, phaseupsolint=None, phaseupbw=None,
        phaseupsnr=None, phaseupnsols=None,
        hm_bandpass=None, solint=None, maxchannels=None, evenbpints=None,
        bpsnr=None, bpnsols=None, combine=None, refant=None, minblperant=None,
        minsnr=None, solnorm=None,
        antnegsig=None, antpossig=None, tmantint=None, tmint=None, tmbl=None,
        antblnegsig=None, antblpossig=None, relaxed_factor=None,
        pipelinemode=None, dryrun=None, acceptresults=None):

    # create a dictionary containing all the arguments given in the
    # constructor
    all_inputs = vars()

    task_name = 'Bandpassflag'

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
