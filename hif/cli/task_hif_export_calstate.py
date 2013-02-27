from taskinit import casalog

#import pipeline.cli.utils as utils
import pipeline.h.cli.utils as utils

def hif_export_calstate(filename=None):
    context = utils.get_context()
    context.callibrary.export(filename)
