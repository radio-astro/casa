# wrapper script for the ASDM import/export and analysis regression test
import sys
import os
import string
from locatescript import copydata
from locatescript import locatescript
import inspect

# Short description
def description():
    return "Test of the importasdm and exportasdm tasks, ASDM <-> MS data conversion, including simple analysis"

pass_on = { "asdm_dataset_name" : "uid___X5f_X18951_X1",
            "ms_dataset_name" : "M51.ms",
            "asdm_dataset2_name" : 'uid___X02_X56142_X1',
            "wvr_correction_file" : 'N3256_B6_0.WVR'
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
            pass_on["ms_dataset_name"],
            pass_on["asdm_dataset2_name"],
            pass_on["wvr_correction_file"]
            ]

def doCopy():
    ### return a list of the same length as that returned by data()
    ###   containing 0 if the corresponding file should be just linked
    ###   (i.e. is read-only)
    ###   or 1 if the corresponding file should be really copied to
    ###   the work directory

    return [0,0,0,0]

def run( fetch=False ):

    #####fetch data
    if fetch:
        for f in data( ):
            copydata( f, os.getcwd( ) )
    
    #####locate the regression script
    try: 
        lepath=locatescript('alma_asdm+reimported-asdm_sf.py')
        print 'Script used is ',lepath
        execfile(lepath, gl, pass_on)
    except:
        print 'execution failed: ', sys.exc_info()
        raise
###return the images that will be templated and compared in future runs
    return []
