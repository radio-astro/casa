# wrapper script for the ALMA NGC3256 analysis regression test
import sys
import os
import string
from locatescript import copydata
from locatescript import locatescript
import inspect

# Short description
def description():
    return "Test of an ALMA SV data analysis (6 MSs from April 2011)"

pass_on = { "tarfile_name" : "NGC3256_Band3_UnCalibratedMSandTablesForReduction.tgz"
            }

a=inspect.stack()
stacklevel=0
for k in range(len(a)):
    if (string.find(a[k][1], 'ipython console') > 0):
        stacklevel=k
        break
gl=sys._getframe(stacklevel).f_globals

def data():
    ### return the data files that are needed by the regression script
    return [pass_on["tarfile_name"]
            ]

def doCopy():
    ### return a list of the same length as that returned by data()
    ###   containing 0 if the corresponding file should be just linked
    ###   (i.e. is read-only)
    ###   or 1 if the corresponding file should be really copied to
    ###   the work directory

    return [0]

def run( fetch=False ):

    #####fetch data
    if fetch:
        for f in data( ):
            copydata( f, os.getcwd( ) )
    
    #####locate the regression script
    try: 
        lepath=locatescript('alma-ngc3256-analysis-regression.py')
        print 'Script used is ',lepath
        execfile(lepath, gl, pass_on)
    except:
        print 'execution failed: ', sys.exc_info()
        raise
###return the images that will be templated and compared in future runs
    return []
