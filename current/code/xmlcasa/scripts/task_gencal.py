import sys
import os
from taskinit import *

def gencal(vis=None,caltable=None,caltype=None,
           spw=None,antenna=None,pol=None,
           parameter=None):

       """ Externally specify calibration soltuions af various types
       """

       #Python script
       try:

              if ((type(vis)==str) & (os.path.exists(vis))):
                     print 'No scr col generation!!!'
                     cb.open(vis,False,False)  # don't need scr col for this
              else:
                     raise Exception, 'Visibility data set not found - please verify the name'

              if (caltable==''):
                     raise Exception, 'A caltable name must be specified'


              cb.specifycal(caltable=caltable,time="",spw=spw,antenna=antenna,pol=pol,
                            caltype=caltype,parameter=parameter)

              cb.close()

       except Exception, instance:
              print '*** Error ***',instance
              cb.close()
              raise Exception, instance

