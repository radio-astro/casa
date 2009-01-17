# wrapper script for ASDM import regression test
import sys
import os
import string
from locatescript import locatescript
import inspect

# Short description
def description():
    return "Test of importasdm() task, ASDM to MS data conversion"

pass_on = { "asdm_dataset_name" : "uid___X1eb_X59c0_X1" }

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
        lepath=locatescript('asdm-import_regression.py')
        print 'Script used is ',lepath
        execfile(lepath, gl, pass_on)
    except:
        print 'execution failed: ', sys.exc_info()
        raise
###return the images that will be templated and compared in future runs
    return []

def data():
    ### return the data files that are needed by the regression script
    return [pass_on["asdm_dataset_name"]]

def doCopy():
    ### return a list of the same length as that returned by data()
    ###   containing 0 if the corresponding file should be just linked
    ###   (i.e. is read-only)
    ###   or 1 if the corresponding file should be really copied to
    ###   the work directory

    return [0]
