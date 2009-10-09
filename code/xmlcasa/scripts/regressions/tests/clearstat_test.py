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
        
    isreadlocked = None
    iswritelocked = None
    msg = ''
    
    print "Open an MS table an acquire a read lock on it"
    isopened = tb.open(input_file)
    if isopened == False :
        raise Exception, "Cannot open MS table"
    
    isreadlocked = tb.haslock(write=False)
    if isreadlocked == False :
        msg = "Error in acquiring a read lock for table"
        print >> sys.stderr, msg
        
    print 'Acquire a write lock on table'
    tb.lock()
    
    iswritelocked = tb.haslock(write=True)
    if iswritelocked == False :
        msg = "Error in acquiring a write lock for table"
        print >> sys.stderr, msg         
        
    # Clear all the locks
    print 'Clear all read/write locks'
    clearstat()
    
    isreadlocked = tb.haslock(write=False)
    iswritelocked = tb.haslock(write=True)
    if (isreadlocked == True) or (iswritelocked == True) :
        tb.close()
        raise Exception, "Test failed! Table is still locked; read lock=%s, write "\
                         "lock=%s" %(isreadlocked,iswritelocked)
    else:
        tb.close()
        print "-- Test clearstat() succeeded --"    
    
    
    return []





