# wrapper script for the async segfault test
import sys
import os
import string
from locatescript import locatescript
import inspect

# Short description
def description():
    return "Test of the occurence of a segfault in the fits file handling in tasks with async==false"

pass_on = { "ms_name" : "test.ms",
            "image_name" : "test.clean.image"
            }

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
        lepath=locatescript('async-segfault_regression.py')
        print 'Script used is ',lepath
        execfile(lepath, gl, pass_on)
    except:
        print 'execution failed: ', sys.exc_info()
        raise
###return the images that will be templated and compared in future runs
    return []

def data():
    ### return the data files that are needed by the regression script
    myfiles = []
    for i in pass_on.keys():
        myfiles.append(pass_on[i])
    return myfiles

def doCopy():
    ### return a list of the same length as that returned by data()
    ###   containing 0 if the corresponding file should be just linked
    ###   (i.e. is read-only)
    ###   or 1 if the corresponding file should be really copied to
    ###   the work directory

    return [1,1]
