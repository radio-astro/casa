from taskinit import casalog

import pipeline
import pipeline.h.cli.utils as utils

def h_weblog():
    context = utils.get_context()    
    pipeline.show_weblog(context)
