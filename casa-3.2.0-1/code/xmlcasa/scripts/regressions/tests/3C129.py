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

# Short description
def description():
    return "Reduces B- and C-configuration VLA continuum polarimetry data at 5 GHz on 3C129 and calibrators, and images the dual-config 3C129 in full polarization"

def run():
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

def data():
    ### return the data files that is needed by the regression script
    return ['AT166_1', 'AT166_2', 'AT166_3']
