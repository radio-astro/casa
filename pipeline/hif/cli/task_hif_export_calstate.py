from taskinit import casalog

import pipeline.h.cli.utils as utils

def hif_export_calstate(filename=None, state=None):
    context = utils.get_context()
    if state == 'applied':
        context.callibrary.export_applied(filename)
    else:
        context.callibrary.export(filename)
