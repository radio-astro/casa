import os
import shutil
from taskinit import casalog
from casac import casac


def msuvbin(vis=None, field=None, spw=None, taql=None, outvis=None, phasecenter=None, nx=None, ny=None, cell=None,
           ncorr=None, nchan=None, fstart=None, fstep=None, wproject=None, memfrac=None):
    
    casalog.origin('msuvbin ') 
    cmd='msuvbin '
    pnames=msuvbin.func_code.co_varnames[:msuvbin.func_code.co_argcount]
    for p in pnames:
        pval=eval(p)
        if(not ((pval == '') or (pval ==None))):
            if(str(pval).find(' ') > 0):
                pval='"'+str(pval)+'"'
            if(p=='cell'):
                cmd=cmd+' '+'cellx'+'='+str(pval)+' '+'celly'+'='+str(pval)
            else:
                cmd=cmd+' '+p+'='+str(pval)

##############3
    print 'command ', cmd

############
    os.system(cmd)
