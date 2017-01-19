import sys
import os
import string
from locatescript import copydata
from locatescript import locatescript
from casa_stack_manip import stack_frame_find

gl=stack_frame_find( )

def description():
    return "Use case script for POLCAL 6cm Data, using POLCAL data 20080224 BnC-config C-band"

def data():
    ### return the data files that is needed by the regression script
    return ['POLCA_20080224_1']

def run(fetch=False):

    #####fetch data
    if fetch:
        for f in data( ):
            copydata( f, os.getcwd( ) )
    
    #####locate the regression script
    lepath=locatescript('polcal_20080224_cband_regression.py')
    print 'Script used is ',lepath

    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'
    
###resturn the images that will be templated and compared in future runs
    return []
