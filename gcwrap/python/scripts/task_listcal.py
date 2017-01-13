import os
from taskinit import *

def listcal(vis=None,caltable=None,field=None,antenna=None,spw=None,
            listfile=None,pagerows=None):
       """List calibration solutions (amp and phase)."""

       casalog.origin('listcal')

       #Python script

       try:
              mycb=cbtool()
              if ((type(vis)==str) & (os.path.exists(vis))):
                     mycb.open(filename=vis,compress=False,addcorr=False,addmodel=False)
              else:
                     raise Exception, 'Visibility data set not found - please verify the name'
              mycb.listcal(caltable=caltable,field=field,antenna=antenna,spw=spw,
                         listfile=listfile,pagerows=pagerows)
              mycb.close()
       except Exception, instance:
              print '*** Error ***',instance


