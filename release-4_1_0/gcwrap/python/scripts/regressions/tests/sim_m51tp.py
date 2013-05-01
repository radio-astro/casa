# simulation tasks regression
#
#  multiple pointings (mapping)
#  single total power measurement
#  thermal noise ("tsys-manual")

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
    return "Simulates a ACA total power mapping from a model image. With thermal noise (tsys-manual), imaged."


def data():
    ### return the data files that is needed by the regression script
    return []


def run(fetch=False):
    #####locate the regression script
    lepath=locatescript('m51_tpsim_regression.py')
    print 'Script used is ',lepath
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'
###return the images that will be templated and compared in future runs
    return ['m51sd_co32/m51sd_co32.aca.tp.sd.ms','m51sd_co32/m51sd_co32.aca.tp.noisy.sd.ms','m51sd_co32/m51sd_co32.sd.image']

