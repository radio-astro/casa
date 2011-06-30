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
    from sdbaseline_cli import sdbaseline_cli as sdbaseline
    from sdcal_cli import sdcal_cli as sdcal
    from sdcoadd_cli import sdcoadd_cli as sdcoadd
    from sdfit_cli import sdfit_cli as sdfit
    from sdflag_cli import sdflag_cli as sdflag
    from sdflagmanager_cli import sdflagmanager_cli as sdflagmanager
    from sdimaging_cli import sdimaging_cli as sdimaging
    from sdimprocess_cli import sdimprocess_cli as sdimprocess
    from sdlist_cli import sdlist_cli as sdlist
    from sdmath_cli import sdmath_cli as sdmath
    from sdplot_cli import sdplot_cli as sdplot
    from sdsave_cli import sdsave_cli as sdsave
    from sdscale_cli import sdscale_cli as sdscale
    from sdsmooth_cli import sdsmooth_cli as sdsmooth
    from sdstat_cli import sdstat_cli as sdstat
    from sdtpimaging_cli import sdtpimaging_cli as sdtpimaging
    myf['sd']=sd
    myf['sdaverage']=sdaverage
    myf['sdbaseline']=sdbaseline
    myf['sdcal']=sdcal
    myf['sdcoadd']=sdcoadd
    myf['sdfit']=sdfit
    myf['sdflag']=sdflag
    myf['sdflagmanager']=sdflagmanager
    myf['sdimaging']=sdimaging
    myf['sdimprocess']=sdimprocess
    myf['sdlist']=sdlist
    myf['sdmath']=sdmath
    myf['sdplot']=sdplot
    myf['sdsave']=sdsave
    myf['sdscale']=sdscale
    myf['sdsmooth']=sdsmooth
    myf['sdstat']=sdstat
    myf['sdtpimaging']=sdtpimaging

    print ""
    print "*** Notice: As announced in the previous version, the following SD task"
    print "***         parameters have been renamed:"
    print "***"
    print "***         PreviousName   NewName   Task"
    print "***         ----------------------------------------------------------"
    print "***         sdfile     -> infile     all SD tasks except for sdcoadd"
    print "***         sdfilelist -> infiles    sdcoadd"
    print "***         sdimages   -> infiles    sdimprocess"
    print "***         imagename  -> outfile    sdimaging/sdimprocess/sdtpimaging"
    print "***         fitfile    -> outfile    sdfit"
    print "***         listfile   -> outfile    sdlist"
    print "***         plotfile   -> outfile    sdplot"
    print "***         statfile   -> outfile    sdstat"
    print ""
