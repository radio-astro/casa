from __future__ import absolute_import

import os,sys
#sys.path.insert(1, os.path.dirname(__file__))
#import pipeline.infrastructure.api as api
#import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.tasks as tasks

from pipeline.infrastructure.launcher import Pipeline

def show_weblog(context):
    if context is None:
        return
    
    import os
    import webbrowser
    index_html = os.path.join(context.report_dir, 't1-1.html')
    webbrowser.open(index_html)

def initcli() :
   print "Initializing cli..."
   mypath = os.path.dirname(__file__);
   hifpath = mypath+"/hif/cli/hif.py"
   hpath = mypath+"/h/cli/h.py"
   hsdpath = mypath+"/hsd/cli/hsd.py"
   import inspect
   myglobals = sys._getframe(len(inspect.stack())-1).f_globals

   execfile(hpath, myglobals)
   execfile(hifpath, myglobals)
   execfile(hsdpath, myglobals)
