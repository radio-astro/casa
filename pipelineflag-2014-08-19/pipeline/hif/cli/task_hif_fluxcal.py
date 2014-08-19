import sys

from taskinit import casalog

import pipeline.h.cli.utils as utils


def hif_fluxcal(refintent=None, transintent=None, dryrun=None, 
                pipelinemode=None, reference=None, transfer=None):

    raise Exception, ('The CASA interface for hif_fluxcal needs to be '
                      'updated to match the task inputs')
    
    # create a dictionary containing all the arguments given in the
    # constructor
    all_inputs = vars()

    # set the name of the pipeline task to execute 
    task_name = 'Fluxcal'

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
