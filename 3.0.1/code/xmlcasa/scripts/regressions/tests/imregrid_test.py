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
    return "Test of imregrid"

def data():
    return []

def run():
    lepath=locatescript('imregrid_regression.py')
    print 'Script used is ',lepath
    execfile(lepath, gl)
    return []
