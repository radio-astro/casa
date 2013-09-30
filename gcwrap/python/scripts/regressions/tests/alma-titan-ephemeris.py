# wrapper script for the ALMA Titan Ephemeris regression test
import sys
import os
import string
from locatescript import copydata
from locatescript import locatescript
import inspect

# Short description
def description():
    return "Test of the ephemeris handling in the analysis of a two-part ALMA dataset on Titan"

pass_on = { "asdm_dataset_name" : "X19d.ms.split.cal.titan.spw0",
            "asdm_dataset2_name" : "X346.ms.split.cal.titan.spw0",
            "ephemeris" : 'Titan_55197-59214dUTC_J2000.tab'
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
    return [pass_on["asdm_dataset_name"],
            pass_on["asdm_dataset2_name"],
            pass_on["ephemeris"]
            ]

def doCopy():
    ### return a list of the same length as that returned by data()
    ###   containing 0 if the corresponding file should be just linked
    ###   (i.e. is read-only)
    ###   or 1 if the corresponding file should be really copied to
    ###   the work directory

    return [1,1,1]

def run( fetch=False ):

    #####fetch data
    if fetch:
        for f in data( ):
            copydata( f, os.getcwd( ) )
    
    #####locate the regression script
    try: 
        lepath=locatescript('alma-titan-ephemeris-regression.py')
        print 'Script used is ',lepath
        execfile(lepath, gl, pass_on)
    except:
        print 'execution failed: ', sys.exc_info()
        raise
###return the images that will be templated and compared in future runs
    return []
