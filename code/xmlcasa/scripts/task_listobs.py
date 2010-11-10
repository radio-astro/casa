import os
from taskinit import *

def listobs(vis=None,verbose=None):
       """List data set summary in the logger:

       Lists following properties of a measurement set:
       scan list, field list, spectral window list with
       correlators, antenna locations, ms table information.

       Keyword arguments:
       vis -- Name of input visibility file
               default: none. example: vis='ngc5921.ms'
       verbose -- level of detail
             verbose=True: (default); scan and antenna lists
             verbose=False: less information

      Additional comments:  The simplest way to obtain a hard copy
      of the output is: 1) Open a file in your favorite editor;
      2) Cut relevant portion of message log.  You may have to use
      cntr-O to store the text; 3) Paste text in the editor window.

      You can also copy the ascii text from the end of the casapy.log
      file in the working directory.

       """

       if pCASA.is_mms(vis):
              pCASA.execute("listobs", vis, locals())
              return

       casalog.origin('listobs')

       #Python script
       #parameter_printvalues(arg_names,arg_values,arg_types)
       try:
               if ((type(vis)==str) & (os.path.exists(vis))):
                       ms.open(thems=vis)
               else:
                       raise Exception, 'Visibility data set not found - please verify the name'
               ms.summary(verbose=verbose)
               ms.close()
       except Exception, instance:
               print '*** Error ***',instance

