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
    print '*** Loading ATNF ASAP Package...'
    import asap as sd
    print '*** ... ASAP (%s rev#%s) import complete ***' % (sd.__version__,sd.__revision__)
    os.environ['CASAPATH']=casapath
    from sdbaseline_cli import sdbaseline_cli as sdbaseline
    from sdcal_cli import sdcal_cli as sdcal
    from sdcal2_cli import sdcal2_cli as sdcal2
    from sdcoadd_cli import sdcoadd_cli as sdcoadd
    from sdfit_cli import sdfit_cli as sdfit
    from sdflag_cli import sdflag_cli as sdflag
    from sdflag2_cli import sdflag2_cli as sdflag2
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
    from sdsmooth_cli import sdsmooth_cli as sdsmooth
    from sdstat_cli import sdstat_cli as sdstat
    from sdtpimaging_cli import sdtpimaging_cli as sdtpimaging
    from tsdcal_cli import tsdcal_cli as tsdcal
    myf['sd']=sd
    myf['sdbaseline']=sdbaseline
    myf['sdcal']=sdcal
    myf['sdcal2']=sdcal2
    myf['sdcoadd']=sdcoadd
    myf['sdfit']=sdfit
    myf['sdflag']=sdflag
    myf['sdflag2']=sdflag2
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
    myf['sdsmooth']=sdsmooth
    myf['sdstat']=sdstat
    myf['sdtpimaging']=sdtpimaging
    myf['tsdcal']=tsdcal
