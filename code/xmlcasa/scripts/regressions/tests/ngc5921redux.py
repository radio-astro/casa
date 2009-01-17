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
    return "Based on ngc5921_regression.py"

def run():
    lepath=locatescript('ngc5921_regression.py')
    print 'Script used is ',lepath
    regstate=True
    execfile(lepath, gl)
    print 'regstate =', regstate
    if not regstate:
        raise Exception, 'regstate = False'

###return the images that will be templated and compared in future runs
    return ['ngc5921_regression/ngc5921.clean.image']

def data():
    ### return the data files that is needed by the regression script
    return []
