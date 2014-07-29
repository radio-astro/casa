# simulation tasks regression
#
#  components only
#  multiple pointings
#  single interferometric measurement
#  no noise

import os, time, string, sys, inspect
from locatescript import locatescript

a=inspect.stack()
stacklevel=0
for k in range(len(a)):
    if (string.find(a[k][1], 'ipython console') > 0):
        stacklevel=k
        break
gl=sys._getframe(stacklevel).f_globals


# Short description
def description():
    return "Simulates a mosaic 12m ALMA pointings from a components list. No noise, imaged."


def data():
    ### return the data files that is needed by the regression script
    return []


def run(fetch=False):
    #####locate the regression script
    lepath=locatescript('testcompsim.py')
    print 'Script used is ',lepath
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'
###return the images that will be templated and compared in future runs
    return ['cltest/cltest.alma.cycle0.extended.ms','cltest/cltest.alma.cycle0.extended.image','cltest/cltest.alma.cycle0.extended.diff']

