import sys
import os
import string
from locatescript import copydata
from locatescript import locatescript
from casa_stack_manip import stack_frame_find

gl=stack_frame_find( )

# Short description
def description():
    return "Reduces B- and C-configuration VLA continuum polarimetry data at 5 GHz on 3C129 and calibrators, and images the dual-config 3C129 in full polarization"

def data():
    ### return the data files that is needed by the regression script
    return ['AT166_1', 'AT166_2', 'AT166_3']

def run( fetch=False ):

    #####fetch data
    if fetch:
        for f in data( ):
            copydata( f, os.getcwd( ) )
    
    #####locate the regression script
    lepath=locatescript('3c129_tutorial_regression.py')
    print 'Script used is ',lepath
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'

###return the images that will be templated and compared in future runs
#    return ['3C129BC.clean.image', 'at166B.3c129.image', 'at166C.3c129.image']
    return ['3C129BC.core.I']
