import os
import sys
import inspect
import string

def asap_init():
    """ Initialize ASAP....: """
    a=inspect.stack()
    stacklevel=0
    for k in range(len(a)):
        #if (string.find(a[k][1], 'ipython console') > 0):
        if a[k][1] == "<string>" or \
               (string.find(a[k][1], 'ipython console') > 0) or \
               string.find(a[k][1],"casapy.py") > 0:
            stacklevel=k
            break
    myf=sys._getframe(stacklevel).f_globals
    casapath=os.environ['CASAPATH']
    print('*** Loading ATNF ASAP Package...')
    import asap as sd
    print('*** ... ASAP (%s rev#%s) import complete ***' % (sd.__version__,sd.__revision__))
    os.environ['CASAPATH']=casapath
    from sdaverage_cli import sdaverage_cli as sdaverage
    from sdbaseline_cli import sdbaseline_cli as sdbaseline
    from sdbaseline2_cli import sdbaseline2_cli as sdbaseline2
    from sdcal_cli import sdcal_cli as sdcal
    from sdcal2_cli import sdcal2_cli as sdcal2
    from sdcoadd_cli import sdcoadd_cli as sdcoadd
    from sdfit_cli import sdfit_cli as sdfit
    from sdflag_cli import sdflag_cli as sdflag
    from sdflagmanager_cli import sdflagmanager_cli as sdflagmanager
    from sdgrid_cli import sdgrid_cli as sdgrid
    from sdimaging_cli import sdimaging_cli as sdimaging
    from sdimprocess_cli import sdimprocess_cli as sdimprocess
    from sdlist_cli import sdlist_cli as sdlist
    from sdmath_cli import sdmath_cli as sdmath
    from sdplot_cli import sdplot_cli as sdplot
    from sdreduce_cli import sdreduce_cli as sdreduce
    from sdsave_cli import sdsave_cli as sdsave
    from sdscale_cli import sdscale_cli as sdscale
    from sdstat_cli import sdstat_cli as sdstat
    from sdtpimaging_cli import sdtpimaging_cli as sdtpimaging
    myf['sd']=sd
    myf['sdaverage']=sdaverage
    myf['sdbaseline']=sdbaseline
    myf['sdbaseline2']=sdbaseline2
    myf['sdcal']=sdcal
    myf['sdcal2']=sdcal2
    myf['sdcoadd']=sdcoadd
    myf['sdfit']=sdfit
    myf['sdflag']=sdflag
    myf['sdflagmanager']=sdflagmanager
    myf['sdgrid']=sdgrid
    myf['sdimaging']=sdimaging
    myf['sdimprocess']=sdimprocess
    myf['sdlist']=sdlist
    myf['sdmath']=sdmath
    myf['sdplot']=sdplot
    myf['sdreduce']=sdreduce
    myf['sdsave']=sdsave
    myf['sdscale']=sdscale
    myf['sdstat']=sdstat
    myf['sdtpimaging']=sdtpimaging
#     ### NOTE on interface changes ###
#     print("#"*50)
#     print("")
#     print("Major interface changes to SINGLE DISH tasks have been")
#     print("taken place in CASA 4.2.2 release")
#     print("")
#     print("The interface of the following tasks are modified:")
#     print("sdbaseline, sdcal, sdcal2, sdfit, sdflag, sdgrid,")
#     print("sdimaging, sdmath, sdplot, sdreduce, sdsave, and sdstat.")
#     print("Additionally, a new task called sdaverage is available. Task")
#     print("sdsmooth has been incorporated in the new task and removed.")
#     print("")
#     print("The tasks with old interfaces are available with name")
#     print("{taskname}old and kept by CASA 4.3 release. Users are")
#     print("adviced to update existing scripts.")
#     print("")
#     print("#"*50)
