#############################################################################
# $Id:$
# Test Name:                                                                #
#    Regression Test Script for listhistory                                 #
#                                                                           #
# Rationale for Inclusion:                                                  #
#    It ensures that the task is working properly. It tests mainly the      #
#    return status in the case of a non-existing, and existing input        #
#    file.                                                                  #
#                                                                           # 
# Features tested:                                                          #
#    1) Does it give a proper return status?                                #
#                                                                           #
# Input data:                                                               #
#    ngc4826_22apr98.ms                                                            #
#                                                                           #
#############################################################################

import os
import string
import sys
import time
import shutil
import commands

from tasks import *
from taskinit import casalog

startTime = time.time()

input_file="n4826_22apr98.ms"
stars = "*************"

def description():
    return "Test of listhistory using NGC4826_22apr98.ms"

def data():
    return [input_file]

def run():
  
    refnum=40
    total=0
    fail=0

    #Set casalog to a local file
    logfile="./mylisth.log"
    newfile="./newlisth.log"
    open(logfile,"w").close
    casalog.setlogfile(logfile)

    # Test 1: Empty input. Return must be False
    total += 1
    print
    print stars + " Test %s (empty input" % total + ") start " + stars
    file = ""
    return_status=listhistory(file)
    if (return_status != False):
        fail += 1
        msg = "Expected return status False, got %s " % return_status

    print stars + stars + stars + stars
    print
       

    # Test 2: True input. Return must be None
    total += 1
    print stars + " Test %s (good input" % total + ") start " + stars
    file = input_file
    return_status=listhistory(file)
    if (return_status != None):
        fail += 1
        msg = "Expected return status None, got %s " % return_status
        
    print stars + stars + stars + stars
    print
 
    # Test 3: Check length of reference list and returned list
    total += 1
    print stars + " Test %s (number of lines) " %total + stars
    
    # Strip out only what is between Begin and End Task
    cmd="sed -n \"/Begin Task/,/End Task/p\" %s > %s " %(logfile,newfile)
    os.system(cmd)
    
    # Get the number of lines in file
    cmd="wc -l %s |egrep \"[0-9]+\" -o" %newfile    
    output=commands.getoutput(cmd)
    num = int(output)
    
    if (num != refnum):
        fail += 1
        msg = "The number of lines in output is not the same as in reference. %s != %s" %(num,refnum)
        
    print stars + stars + stars + stars
    print
      
    
    # Compare contents of both lists ????
    
    # Check how many failed and exit
    if (fail > 0):
        raise Exception, "End of tests: %s out of %s tests failed. \nLast message: %s" \
        %(fail,total,msg)
    else:
        print stars + stars + stars + stars
        print "All %s tests completed successfully " % total
        print stars + stars + stars + stars

        
    return []
        
    
    
    
