from taskinit import casalog

import sys
import pipeline.h.cli.utils as utils

def hif_show_calstate():
    context = utils.get_context()  
    sys.stdout.write('Current on-the-fly calibration state:\n\n')
    sys.stdout.write(str(context.callibrary.active))
    sys.stdout.write('\n')
