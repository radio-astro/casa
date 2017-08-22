import datetime

from taskinit import casalog

import pipeline.h.cli.utils as utils

def h_save(filename=None):
    context = utils.get_context()    
    context.save(filename)
