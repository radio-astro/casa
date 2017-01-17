import sys
import os
import string
from locatescript import copydata
from locatescript import locatescript
from casa_stack_manip import stack_frame_find

gl=stack_frame_find( )

# Short description
def description():
    return "Images VLA multi-frequency 3C286 data and creates intensity, spectral-index and spectral-curvature maps."

# Copy the Data ? Yes. 
def doCopy():
    return [1];

# Input data
def data():
    ### return the data files that is needed by the regression script
    return ['VLA_multifrequency_3C286.ms']

# Run the test
def run(fetch=False):

    #####fetch data
    if fetch:
        for f in data( ):
            copydata( f, os.getcwd( ) )
    
    #####locate the regression script
    lepath=locatescript('wideband_regression.py')
    print 'Script used is ',lepath
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'
    ### return the images that will be templated and compared in future runs
    return []; #'reg_3C286.image.tt0','reg_3C286.image.alpha','reg_3C286.image.beta']

