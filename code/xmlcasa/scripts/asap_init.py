import os
import sys
import inspect
import string

def asap_init():
    """ Initialize ASAP....: """
    a=inspect.stack()
    stacklevel=0
    for k in range(len(a)):
        if (string.find(a[k][1], 'ipython console') > 0):
            stacklevel=k
            break
    myf=sys._getframe(stacklevel).f_globals
    casapath=os.environ['CASAPATH']
    print '*** Loading ATNF ASAP Package...'
    import asap as sd
    print '*** ... ASAP (%s rev#%s) import complete ***' % (sd.__version__,sd.__revision__)
    os.environ['CASAPATH']=casapath
    from sdaverage_cli import sdaverage_cli as sdaverage
    from sdsmooth_cli import sdsmooth_cli as sdsmooth
    from sdbaseline_cli import sdbaseline_cli as sdbaseline
    from sdcal_cli import sdcal_cli as sdcal
    from sdcoadd_cli import sdcoadd_cli as sdcoadd
    from sdsave_cli import sdsave_cli as sdsave
    from sdscale_cli import sdscale_cli as sdscale
    from sdfit_cli import sdfit_cli as sdfit
    from sdplot_cli import sdplot_cli as sdplot
    from sdstat_cli import sdstat_cli as sdstat
    from sdlist_cli import sdlist_cli as sdlist
    from sdflag_cli import sdflag_cli as sdflag
    from sdtpimaging_cli import sdtpimaging_cli as sdtpimaging
    from sdmath_cli import sdmath_cli as sdmath
    from sdimaging_cli import sdimaging_cli as sdimaging
    from sdimprocess_cli import sdimprocess_cli as sdimprocess
    myf['sd']=sd
    myf['sdaverage']=sdaverage
    myf['sdsmooth']=sdsmooth
    myf['sdbaseline']=sdbaseline
    myf['sdcal']=sdcal
    myf['sdcoadd']=sdcoadd
    myf['sdsave']=sdsave
    myf['sdscale']=sdscale
    myf['sdfit']=sdfit
    myf['sdplot']=sdplot
    myf['sdstat']=sdstat
    myf['sdlist']=sdlist
    myf['sdflag']=sdflag
    myf['sdtpimaging']=sdtpimaging
    myf['sdmath']=sdmath
    myf['sdimaging']=sdimaging
    myf['sdimprocess']=sdimprocess

