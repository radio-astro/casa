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
def run():
    lepath=locatescript('polcal_20080224_cband_regression.py')
    print 'Script used is ',lepath

    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'
    
###resturn the images that will be templated and compared in future runs
    return []

def data():
    ### return the data files that is needed by the regression script
    return ['POLCA_20080224_1']

def description():
    return "Use case script for POLCAL 6cm Data, using POLCAL data 20080224 BnC-config C-band"
