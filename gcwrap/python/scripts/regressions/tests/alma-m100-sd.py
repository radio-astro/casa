import sys
import os
import string
from locatescript import copydata
from locatescript import locatescript
from casa_stack_manip import stack_frame_find

gl=stack_frame_find( )

def description():
    return "ALMA single dish data (regression test)"

def data():
    ### return the data files that is needed by the regression script
    return []


def run(fetch=False):
    #####locate the regression script
    #lepath=locatescript('ori_hc3n_task_regression.py')
    lepath=locatescript('alma-m100-sd-regression.py')
    print 'Script used is ',lepath
    gl['regstate']=True
    execfile(lepath, gl)
    print 'regstate =', gl['regstate']
    if not gl['regstate']:
        raise Exception, 'regstate = False'

    return []
