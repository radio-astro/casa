import sys
import os
import numpy
import numpy.random as random
import shutil

from taskinit import *
from applycal import applycal
import types
import sdutil

# Calibrator tool
(cb,) = gentools(['cb'])

def sdgaincal(infile=None, calmode=None, radius=None, smooth=None, 
              antenna=None, field=None, spw=None, scan=None, intent=None, 
              applytable=None, outfile='', overwrite=False): 
       
    casalog.post('This is sdgaincal!')