from taskinit import casalog

import pipeline.h.cli.utils as utils

def h_import_calstate(filename):
    context = utils.get_context()  
    context.callibrary.import_state(filename)
        
