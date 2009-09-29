import sys
import os
from taskinit import *

def gencal(vis=None,caltable=None,spw=None,antenna=None,pol=None,caltype=None,parameter=None):

       """ Externally specify calibration soltuions af various types:

       TBD

       Keyword arguments:
       vis --   Name of input visibility file
                default: none; example: vis='ngc5921.ms'
       caltable -- Input cumulative calibration table
                ''=> first
                  in accumtime.  Then use output table from previous runs.

       Example use:

       """

       #Python script
       try:

              if ((type(vis)==str) & (os.path.exists(vis))):
                     cb.open(vis)
              else:
                     raise Exception, 'Visibility data set not found - please verify the name'

              cb.specifycal(caltable=caltable,time="",spw=spw,antenna=antenna,pol=pol,
                            caltype=caltype,parameter=parameter)

              cb.close()

       except Exception, instance:
              print '*** Error ***',instance
              cb.close()
              raise Exception, instance

