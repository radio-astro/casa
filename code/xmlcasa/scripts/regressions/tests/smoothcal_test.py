########################################################################
#       This script will call the main regression script               #
#       smoothcal_regression.py. The 3 returned data sets from         #
#       data are copied by the runTest() method to the working         #
#       directory, defaulted to /tmp.                                  #
#       Exceptions will be handled if smoothal() fails to run or if    #
#       the regression tests do not pass.                              #
#                                                                      #
########################################################################

import sys
import os
import string
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
    return "Based on smoothcal_regression.py"

def run():
    lepath=locatescript('smoothcal_regression.py')
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'

    return []

def data():
    ### return the data files that is needed by the regression script
    return ['N1333_1.UVFITS','3C147_Q.im','ngc1333.ref.smoothed']
#    return []
