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
    #print('*** ... ASAP (%s rev#%s) import complete ***' % (sd.__version__,sd.__revision__))
    print('*** ... ASAP (rev#%s) import complete ***' % (sd.__revision__))
    os.environ['CASAPATH']=casapath
    from sdimaging_cli import sdimaging_cli as sdimaging
    from sdimprocess_cli import sdimprocess_cli as sdimprocess
    from sdaverageold_cli import sdaverageold_cli as sdaverageold
    from sdbaselineold_cli import sdbaselineold_cli as sdbaselineold
    from sdbaseline2old_cli import sdbaseline2old_cli as sdbaseline2old
    from sdcalold_cli import sdcalold_cli as sdcalold
    from sdcal2old_cli import sdcal2old_cli as sdcal2old
    from sdcoaddold_cli import sdcoaddold_cli as sdcoaddold
    from sdfitold_cli import sdfitold_cli as sdfitold
    from sdflagold_cli import sdflagold_cli as sdflagold
    from sdflagmanagerold_cli import sdflagmanagerold_cli as sdflagmanagerold
    from sdgridold_cli import sdgridold_cli as sdgridold
    from sdlistold_cli import sdlistold_cli as sdlistold
    from sdmathold_cli import sdmathold_cli as sdmathold
    from sdplotold_cli import sdplotold_cli as sdplotold
    from sdreduceold_cli import sdreduceold_cli as sdreduceold
    from sdsaveold_cli import sdsaveold_cli as sdsaveold
    from sdscaleold_cli import sdscaleold_cli as sdscaleold
    from sdstatold_cli import sdstatold_cli as sdstatold
    myf['sd']=sd
    myf['sdimaging']=sdimaging
    myf['sdimprocess']=sdimprocess
    myf['sdaverageold']=sdaverageold
    myf['sdbaselineold']=sdbaselineold
    myf['sdbaseline2old']=sdbaseline2old
    myf['sdcalold']=sdcalold
    myf['sdcal2old']=sdcal2old
    myf['sdcoaddold']=sdcoaddold
    myf['sdfitold']=sdfitold
    myf['sdflagold']=sdflagold
    myf['sdflagmanagerold']=sdflagmanagerold
    myf['sdgridold']=sdgridold
    myf['sdlistold']=sdlistold
    myf['sdmathold']=sdmathold
    myf['sdplotold']=sdplotold
    myf['sdreduceold']=sdreduceold
    myf['sdsaveold']=sdsaveold
    myf['sdscaleold']=sdscaleold
    myf['sdstatold']=sdstatold
