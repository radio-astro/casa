# Task listvis

import os
from taskinit import *

def listvis(vis, options, datacolumn, field,spw, selectdata, antenna, timerange,
            correlation, scan, feed, array, observation, uvrange, average,
            showflags, pagerows, listfile):
    """List visibilities on terminal."""
        
    casalog.origin('listvis')
    myms = casac.ms()
    
    isInteractive=False;
    
    try:
        if ((type(vis)==str) & (os.path.exists(vis))):
            myms.open(vis)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'
                
        myms.lister(options, datacolumn, field, spw, antenna, timerange,
                    correlation, scan, feed, array, str(observation), uvrange,
                    average, showflags, "", pagerows, listfile)
        myms.close()
    except Exception, instance:
        print '*** Error in listvis *** ', instance
    
