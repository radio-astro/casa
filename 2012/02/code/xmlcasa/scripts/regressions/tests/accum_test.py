########################################################################
#       This script will call the main regression script               #
#       accum_regression.py. The returned data set from                #
#       is copied by the runTest() method to the working               #
#       directory, defaulted to /tmp.                                  #
#       Exceptions will be handled if accum() fails to run or if       #
#       the regression tests do not pass.                              #
#                                                                      #
########################################################################

import sys
import os
import string
from locatescript import copydata
from locatescript import locatescript
import inspect

a=inspect.stack()
stacklevel=0
for k in range(len(a)):
    if (string.find(a[k][1], 'ipython console') > 0):
        stacklevel=k
        break
gl=sys._getframe(stacklevel).f_globals

def description():
    return "Based on accum_regression.py"

def data():
    ### return the data files that is needed by the regression script
    return ['ngc5921.fits']
#    return []

def run( fetch=False ):

    #####fetch data
    if fetch:
        for f in data( ):
            copydata( f, os.getcwd( ) )
    
    #####locate the regression script
    lepath=locatescript('accum_regression.py')
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'

    return []
