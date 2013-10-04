########################################################################
#       This script will call the main regression script               #
#       leo2pt_regression.py. The returned data set from it            #
#       is copied by the runTest() method to the working               #
#       directory, defaulted to /tmp.                                  #
#       Exceptions will be handled if the script fails to run or if    #
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
    return "Script for LeoRing EVLA 20cm Data"

def run():
    lepath=locatescript('leo2pt_regression.py')
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'

    return []

def data():
    ### return the data files that are needed by the regression script
    return ['leo2pt.55183.452640752315']

def doCopy():
    return [0]

