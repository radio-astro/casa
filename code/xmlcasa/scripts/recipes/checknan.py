from taskinit import *
import numpy as np

def checknan(ms='', fix=False):
    """
    script to check for NaN in uvw
    if fix is true it will flag the row with nan data
    and replace the uvw with 0
    you can see how to do that to other columns too
    """
    tb.open(ms, nomodify=(not fix))
    uvw=tb.getcol('UVW')
    flg=tb.getcol('FLAG_ROW')
    nanoo=np.isnan(uvw)
    for k in range(nanoo.shape[1]):
      if(nanoo[0,k] or nanoo[1,k] or nanoo[2,k]):
          print 'row=', k
          uvw[0,k]=0
          uvw[1,k]=0
          uvw[2,k]=0
          flg[k]=True
    if(fix):
        tb.putcol('UVW', uvw)
        tb.putcol('FLAG_ROW', flg)
    tb.done()
     
