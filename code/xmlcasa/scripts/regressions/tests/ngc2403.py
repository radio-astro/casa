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
    return "Based on ngc2403_tutorial_regression.py"

def run():
    #####locate the regression script
    lepath=locatescript('ngc2403_tutorial_regression.py')
    print 'Script used is ',lepath
    regstate=True
    execfile(lepath, gl)
    print 'regstate =', regstate
    if not regstate:
        raise Exception, 'regstate = False'

###return the images that will be templated and compared in future runs

    #not in repository: return ['n2403.tutorial.final.clean.image','n2403.tutorial.dirty.image']
    return []

def data():
    ### return the data files that is needed by the regression script
    return ['AS649_1','AS649_2','AS649_3','AS649_4']
