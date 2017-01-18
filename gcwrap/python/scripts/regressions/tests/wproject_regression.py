import sys
import os
import string
import time
from locatescript import copydata
from locatescript import locatescript
from casa_stack_manip import stack_frame_find

gl=stack_frame_find( )

def data():
    ### return the data files that is needed by the regression script
    return ['coma.ms']

def run(fetch=False):

    #####fetch data
    if fetch:
        for f in data( ):
            copydata( f, os.getcwd( ) )
    
    #####locate the regression script
    lepath=locatescript('wproject_regression.py')
    print 'Script used is ',lepath
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'
    ### return the images that will be templated and compared in future runs
    return ['bla1re.restored', 'outlier1re.restored']

