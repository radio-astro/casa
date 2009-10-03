#############################################################################
## $Id:$
# Test Name:                                                                #
#    Regression Test Script for plotants                                    #
#                                                                           #
# Rationale for Inclusion:                                                  #
#    It ensures that the task is working properly.                          #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the task working properly?                                       #
#    2) Is the task saving a plot when asked?                               #
#                                                                           #
# Input data:                                                               #
#    ic2233_1.ms                                                            #
#                                                                           #
#############################################################################

import os
import string
import sys
import time

from __main__ import default
from tasks import *
from taskinit import *

startTime = time.time()

input_file="ic2233_1.ms"
plotfile="ic2233_1.plot.png"
stars = "*************"

def description():
    return "Test of plotants using ic2233_1.ms"

def data():
    return [input_file]

def run():
        
    print "Will plot the positions of 29 antennas"
    
    # Switch off the displaying of the GUI
    tp.setgui(gui=False)
    
    # Call the plotants task and check return status
    status = plotants(vis=input_file,figfile=plotfile)
    
    # Switch GUI back on
    tp.setgui(gui=True)
    
    if status == False:
        raise Exception, "Input file does not exist"
    
    # Check if plot exists
    if not os.path.isfile(plotfile):
        raise Exception, "Failed to save plot"
    
    print " -- Test plotants succeeded --"
     
    return []





