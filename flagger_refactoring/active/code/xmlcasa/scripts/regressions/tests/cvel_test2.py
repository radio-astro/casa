# wrapper script for the cvel regression test
import sys
import os
import string
from locatescript import locatescript
import inspect

# Short description
def description():
    return "Test of the cvel() and clean() tasks v2: spectral regridding and transformation"

pass_on = { "dataset_name" : "W3OH_MC.UVFITS" }

a=inspect.stack()
stacklevel=0
for k in range(len(a)):
    if (string.find(a[k][1], 'ipython console') > 0):
        stacklevel=k
        break
gl=sys._getframe(stacklevel).f_globals


def run():
    #####locate the regression script
    try: 
        lepath=locatescript('cvel_regression2.py')
        print 'Script used is ',lepath
        execfile(lepath, gl, pass_on)
    except:
        print 'execution failed: ', sys.exc_info()
        raise
###return the images that will be templated and compared in future runs
    return []

def data():
    ### return the data files that are needed by the regression script
    return [pass_on["dataset_name"]]

def doCopy():
    ### return a list of the same length as that returned by data()
    ###   containing 0 if the corresponding file should be just linked
    ###   (i.e. is read-only)
    ###   or 1 if the corresponding file should be really copied to
    ###   the work directory

    return [0]
