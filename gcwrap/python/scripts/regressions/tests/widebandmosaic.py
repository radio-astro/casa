import sys
import os
import string
from locatescript import copydata
from locatescript import locatescript
import inspect

a=inspect.stack()
stacklevel=0
for k in range(len(a)):
    if (string.find(a[k][1], 'ipython console') > 0):
        stacklevel=k
        break
gl=sys._getframe(stacklevel).f_globals

# Short description
def description():
    return "Images simulated wideband data with WB-A-Projection with CS-Clean in one test, and MT-MFS-Clean in another."

# Copy the Data ? Yes. 
def doCopy():
    return [1];

# Input data
def data():
    ### return the data files that is needed by the regression script
    return ['reg_mawproject_apr13.ms']

# Run the test
def run(fetch=False):

    #####fetch data
    if fetch:
        for f in data( ):
            copydata( f, os.getcwd( ) )
    
    #####locate the regression script
    lepath=locatescript('widebandmosaic_regression.py')
    print 'Script used is ',lepath
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'
    ### return the images that will be templated and compared in future runs
    return [];

