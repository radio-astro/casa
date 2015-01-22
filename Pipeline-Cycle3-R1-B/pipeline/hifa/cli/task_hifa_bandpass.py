import sys

from taskinit import casalog

import pipeline.h.cli.utils as utils


def hifa_bandpass(vis=None, caltable=None, field=None, intent=None, spw=None, 
                 antenna=None, hm_phaseup=None, phaseupsolint=None, 
                 phaseupbw=None, phaseupsnr=None, phaseupnsols=None,
		 hm_bandpass=None, solint=None, maxchannels=None,
		 bpsnr=None, bpnsols=None, hm_bandtype=None, 
                 combine=None, refant=None, solnorm=None, minblperant=None, 
                 minsnr=None, degamp=None, degphase=None, pipelinemode=None, 
                 dryrun=None, acceptresults=None):

    # create a dictionary containing all the arguments given in the
    # constructor
    all_inputs = vars()

    task_name = 'ALMAPhcorBandpass'

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
