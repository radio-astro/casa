#
# This file was generated using xslt from its XML file
#
# Copyright 2007, Associated Universities Inc., Washington DC
#
import os
from taskinit import *

def hanningsmooth(vis=None):

    """Hanning smooth frequency channel data to remove Gibbs ringing

    hanningsmooth(vis='ngc5921.ms')

    This function Hanning smooths the frequency channels with
    a weighted running average of smoothedData[i] =
    0.25*correctedData[i-1] + 0.50*correctedData[i] +
    0.25*correctedData[i+1].  The first and last channels are flagged.
    Inclusion of a flagged value in an average causes that data value
    to be flagged.

    Keyword arguments:
    vis -- Name of input visibility file (MS)
           default: none; example: vis='ngc5921.ms'


    """

    #Python script
    #
    try:
        casalog.origin('hanningsmooth')
        if ((type(vis)==str) & (os.path.exists(vis))):
            ms.open(thems=vis,nomodify=False)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'
        
#        return
        ms.hanningsmooth()
        
        # write history
        ms.writehistory(message='taskname = hanningsmooth',origin='hanningsmooth')
        ms.writehistory(message='vis         = "'+str(vis)+'"',origin='hanningsmooth')
        ms.close()
        
#        return
    except Exception, instance:
        print '*** Error ***',instance
        return
