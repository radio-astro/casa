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
    return "Based on orion_regression.py"

def run():
    lepath=locatescript('orion_regression.py')
    print 'Script used is ',lepath
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'
#    import lepath+'/g192_regression.py'
###resturn the images that will be templated and compared in future runs
    return ['orion_tfeather.im','orion_tfeather2.im','orion_tsdmem.image','orion_tjoint3.image']

def data():
    ### return the data files that is needed by the regression script
    return []
