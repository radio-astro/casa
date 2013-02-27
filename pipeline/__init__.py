from __future__ import absolute_import

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
