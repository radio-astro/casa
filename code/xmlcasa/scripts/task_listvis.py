# Task listvis

import os
from taskinit import *

def listvis(vis,options,datacolumn,field,spw,selectdata,antenna,timerange,correlation,scan,feed,array,uvrange,average,showflags,pagerows,listfile):
    """List visibilities on terminal."""
        
    casalog.origin('listvis')
    
    isInteractive=False;
    
    try:
        if ((type(vis)==str) & (os.path.exists(vis))):
            ms.open(vis)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'
                
        ms.lister(options,datacolumn,field,spw,antenna,timerange,correlation,scan,feed,array,uvrange,average,showflags,"",pagerows,listfile)
        
        ms.close()
    except Exception, instance:
        print '*** Error *** ',instance
    # raise Exception, instance
    
