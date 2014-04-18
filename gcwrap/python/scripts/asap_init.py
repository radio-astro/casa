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
    from tsdaverage_cli import tsdaverage_cli as tsdaverage
    from tsdbaseline_cli import tsdbaseline_cli as tsdbaseline
    from tsdcal_cli import tsdcal_cli as tsdcal
    from tsdcal2_cli import tsdcal2_cli as tsdcal2
    from sdcoadd_cli import sdcoadd_cli as sdcoadd
    from tsdfit_cli import tsdfit_cli as tsdfit
    from tsdflag_cli import tsdflag_cli as tsdflag
    from sdflagmanager_cli import sdflagmanager_cli as sdflagmanager
    from tsdgrid_cli import tsdgrid_cli as tsdgrid
    from tsdimaging_cli import tsdimaging_cli as tsdimaging
    from sdimprocess_cli import sdimprocess_cli as sdimprocess
    from sdlist_cli import sdlist_cli as sdlist
    from tsdmath_cli import tsdmath_cli as tsdmath
    from tsdplot_cli import tsdplot_cli as tsdplot
    from tsdreduce_cli import tsdreduce_cli as tsdreduce
    from tsdsave_cli import tsdsave_cli as tsdsave
    from sdscale_cli import sdscale_cli as sdscale
    from tsdstat_cli import tsdstat_cli as tsdstat
    from sdtpimaging_cli import sdtpimaging_cli as sdtpimaging
    from sdbaselineold_cli import sdbaselineold_cli as sdbaselineold
    from sdcalold_cli import sdcalold_cli as sdcalold
    from sdcal2old_cli import sdcal2old_cli as sdcal2old
    from sdfitold_cli import sdfitold_cli as sdfitold
    from sdflagold_cli import sdflagold_cli as sdflagold
    from sdflag2old_cli import sdflag2old_cli as sdflag2old
    from sdgridold_cli import sdgridold_cli as sdgridold
    from sdimagingold_cli import sdimagingold_cli as sdimagingold
    from sdmathold_cli import sdmathold_cli as sdmathold
    from sdplotold_cli import sdplotold_cli as sdplotold
    from sdreduceold_cli import sdreduceold_cli as sdreduceold
    from sdsaveold_cli import sdsaveold_cli as sdsaveold
    from sdsmoothold_cli import sdsmoothold_cli as sdsmoothold
    from sdstatold_cli import sdstatold_cli as sdstatold
    myf['sd']=sd
    myf['tsdaverage']=tsdaverage
    myf['tsdbaseline']=tsdbaseline
    myf['tsdcal']=tsdcal
    myf['tsdcal2']=tsdcal2
    myf['sdcoadd']=sdcoadd
    myf['tsdfit']=tsdfit
    myf['tsdflag']=tsdflag
    myf['sdflagmanager']=sdflagmanager
    myf['tsdgrid']=tsdgrid
    myf['tsdimaging']=tsdimaging
    myf['sdimprocess']=sdimprocess
    myf['sdlist']=sdlist
    myf['tsdmath']=tsdmath
    myf['tsdplot']=tsdplot
    myf['tsdreduce']=tsdreduce
    myf['tsdsave']=tsdsave
    myf['sdscale']=sdscale
    myf['tsdstat']=tsdstat
    myf['sdtpimaging']=sdtpimaging
    myf['sdbaselineold']=sdbaselineold
    myf['sdcalold']=sdcalold
    myf['sdcal2old']=sdcal2old
    myf['sdfitold']=sdfitold
    myf['sdflagold']=sdflagold
    myf['sdflag2old']=sdflag2old
    myf['sdgridold']=sdgridold
    myf['sdimagingold']=sdimagingold
    myf['sdmathold']=sdmathold
    myf['sdplotold']=sdplotold
    myf['sdreduceold']=sdreduceold
    myf['sdsaveold']=sdsaveold
    myf['sdsmoothold']=sdsmoothold
    myf['sdstatold']=sdstatold
    ### WARNINGS for interface changes ###
    print("#"*50)
    print("")
    print("Major interface changes to SINGLE DISH tasks are")
    print("planned in CASA 4.2.2 release.")
    print("")
    print("For CASA 4.2.2 these interface changes will be implemented in new versions")
    print("of the existing tasks: sdbaseline, sdcal, sdcal2, sdfit, sdflag,")
    print("sdgrid, sdimaging, sdmath, sdplot, sdsave, and sdstat with the")
    print("current name preceded by the letter 't'. Additionally a new task called")
    print("tsdaverage will also be available. Task sdsmooth is incorporated in the")
    print("new task and will be removed. ")
    print("")
    print("The experimental tasks beginning with the letter 't' will replace the")
    print("current tasks in the next release. The current tasks will be renamed as")
    print("{taskname}_old and kept for one release period. Users are encouraged to")
    print("update your existing scripts.")
    print("")
    print("#"*50)

