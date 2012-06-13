import os
from taskinit import *

def listfits(fitsfile=None):
    """

    """
    #Python script
    try:
        if ((type(fitsfile)==str) & (os.path.exists(fitsfile))):
            ms.listfits(fitsfile);    
        else:
            raise Exception, 'fits file not found - please verify the name'
    except Exception, instance:
        print '*** Error ***',instance
