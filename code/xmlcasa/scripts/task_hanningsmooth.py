#
# This file was generated using xslt from its XML file
#
# Copyright 2007, Associated Universities Inc., Washington DC
#
import os
from taskinit import *

def hanningsmooth(vis=None, datacolumn=None, outputvis=None):

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
    datacolumn -- the name of the MS column into which to write the smoothed data
                  default='corrected'; example: datacolumn='data'
                  options: 'corrected' or 'data'
    outputvis -- name of the output visibility file (MS)
                 default=none (write to the input MS); example: outputvis='ngc5921_src.ms'

    """

    #Python script
    #
    try:
        casalog.origin('hanningsmooth')
        casalog.post('vis=\''+vis+'\', datacolumn=\''+datacolumn+'\', outputvis=\''+outputvis+'\'', 'INFO')
        newvis = vis;
        
        if os.path.exists(outputvis):
            ms.close()
            raise Exception, "Output MS %s already exists - will not overwrite." % outputvis

        if(type(outputvis)==str and not outputvis==''):
            newvis = outputvis
            casalog.post('copying '+vis+' to '+newvis , 'INFO')
            tb.open(vis)
            tb.copy(newvis, deep=True, valuecopy=True)
            # note that the resulting copy is writable even if the original was read-only
            tb.close()
        else:
            newvis = vis

        if ((type(newvis)==str) & (os.path.exists(newvis))):
            ms.open(thems=newvis,nomodify=False)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'
        
        ms.hanningsmooth(datacolumn=str.lower(datacolumn))
        
        # write history
        ms.writehistory(message='taskname = hanningsmooth',origin='hanningsmooth')
        ms.writehistory(message='vis         = "'+str(vis)+'"',origin='hanningsmooth')
        ms.writehistory(message='datacolumn  = "'+str(datacolumn)+'"',origin='hanningsmooth')
        ms.writehistory(message='outputvis   = "'+str(outputvis)+'"',origin='hanningsmooth')
        ms.close()
        
    except Exception, instance:
        print '*** Error ***',instance
        return
