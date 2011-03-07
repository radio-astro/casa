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
    return "Based on ngc1333_regression.py"

def run():
    lepath=locatescript('ngc1333_regression.py')
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'
#    import lepath+'/g192_regression.py'
###resturn the images that will be templated and compared in future runs
#   return ['src.task.image', 'src.tmom0.red', 'src.tmom0.blu','src.tmom0.all', 'src.tmom1.all' ]
    return ['ngc1333_regression/n1333_both.image', 'ngc1333_regression/n1333_both.src.tmom0.red', 'ngc1333_regression/n1333_both.src.tmom0.blu', 'ngc1333_regression/n1333_both.src.tmom0.all', 'ngc1333_regression/n1333_both.src.tmom1.all']

def data():
    ### return the data files that is needed by the regression script
    return []
