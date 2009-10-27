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
#    ic2233_1.ms, n4826_tmom1.im                                            #
#                                                                           #
#############################################################################

import os
import string
import sys

from __main__ import default
from tasks import *
from taskinit import *


#input_file="ic2233_1.ms"
input_files=["ic2233_1.ms", "n4826_tmom1.im"]

def description():
    return "Test of clearstat using ic2233_1.ms and n4826_tmom1.im"

def data():
    return input_files

def run():
        
    tbreadlock = None
    tbwritelock = None
    imlock = []
    msg = ''
    
    print "Open an MS table an acquire a read lock on it"
    isopened = tb.open(input_files[0])
    if isopened == False :
        raise Exception, "Cannot open MS table"
    
    tbreadlock = tb.haslock(write=False)
    if tbreadlock == False :
        msg = "Error in acquiring a read lock for table"
        print >> sys.stderr, msg
        
    print 'Acquire a write lock on table'
    tb.lock()
    
    tbwritelock = tb.haslock(write=True)
    if tbwritelock == False :
        msg = "Error in acquiring a write lock for table"
        print >> sys.stderr, msg         

    # Open an image for analysis
    print "Open an image and acquire a read lock on it"
    isimopen = ia.open(input_files[1])
    if isimopen == False :
        raise Exception, "Cannot open image"
    
#    imreadlock = ia.haslock()
    imlock = ia.haslock()
    if (imlock[0] == False) and (imlock[1] == False) :
        msg = "Error in acquiring a read lock for image"
        print >> sys.stderr, msg
    
    # Acquire a write lock on image
    print "Acquire a write lock on image"
    ia.lock(writelock=True)
#    imwritelock = ia.haslock()
    imlock = ia.haslock()
    if (imlock[0] == False) and (imlock[1] == False) :
        msg = "Error in acquiring a write lock for image"
        print >> sys.stderr, msg
 
        
    # Clear all the locks
    print 'Clear all non-permanent read/write locks'
    clearstat()

    # Test if clearstat cleared all locks
    tbreadlock = tb.haslock(write=False)
    tbwritelock = tb.haslock(write=True)
    imlock = ia.haslock()

    if ((tbreadlock == True) or (tbwritelock == True) or 
       (imlock[0] == True) or (imlock[1] == True)) :
        tb.close()
        ia.close()
        raise Exception, "Test failed! Table and/or image is still locked; \n\t\t\t" \
                         "Table: read lock=%s, write lock=%s \n\t\t\t" \
                         "Image: read lock=%s, write lock=%s" %(tbreadlock,tbwritelock,
                                                                imlock[0],imlock[1])
    else:
        print "-- Test clearstat() succeeded --"    
    
    
    tb.close()
    ia.close()
    return []





