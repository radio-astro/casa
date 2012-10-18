########################################################################
# Task to print a summary of the content of an SDM dataset
#
# v1.0: 2012.02.13, M. Caillat
#
import os
from taskinit import *

def asdmsummary(asdm=None):
    """Prints a description of the content of an SDM dataset to the CASA logger.

    Keyword argument:

    asdm -- Name of the input SDM directory.

    """

    try:
        casalog.origin('asdmsummary')
        theexecutable = 'asdmSummary'
        execute_string = theexecutable + ' ' + '--logfile \"' +casalog.logfile() +'\"'+ ' ' + asdm
#        execute_string = theexecutable + ' '  + asdm
        casalog.post('Running ' + execute_string)
        exitcode = os.system(execute_string) >> 8
        if exitcode != 0 :
            casalog.post(theexecutable+' terminated with exit code '+str(exitcode), 'SEVERE')
            raise Exception, "Error while trying to produce a summary of '+" + asdm + "'."
    except Exception, instance:
        print '*** Error ***', instance

 

