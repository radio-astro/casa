#############################################################################
## $Id:$
# Test Name:                                                                #
#    Regression Test Script for clearstat()                                 #
#                                                                           #
# Rationale for Inclusion:                                                  #
#    It ensures that the task is working properly.                          #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the task working properly?                                       #
#    2) Is the task releasing the locks?                                    #
#                                                                           #
# Input data:                                                               #
#    ic2233_1.ms                                                            #
#                                                                           #
#############################################################################

import os
import string
import sys

from __main__ import default
from tasks import *
from taskinit import *


input_file="ic2233_1.ms"

def description():
    return "Test of clearstat using ic2233_1.ms"

def data():
    return [input_file]

def run():
        
    islocked = None
    
    print "Open an MS table an acquire a lock on it"
    isopened = tb.open(input_file)
    if isopened == False :
        raise Exception, "Cannot open MS table"
    
    tb.lock()
    
    islocked = tb.haslock()
    if islocked == False :
        raise Exception, "Error in acquiring lock for table (islocked=%s)" %islocked
    else:
        print "Table is locked"
        
    # Clear all the locks
    clearstat()
    
    islocked = tb.haslock()
    if islocked == True :
        tb.close()
        raise Exception, "Test failed! Table is still locked (islocked=%s)" %islocked
    else:
        tb.close()
        print "-- Test clearstat() succeeded --"    
    
    
    return []





