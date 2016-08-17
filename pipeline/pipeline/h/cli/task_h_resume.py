from taskinit import casalog

import pipeline.h.cli.cli as cli
import pipeline.infrastructure.launcher as launcher
import pipeline.infrastructure.basetask as basetask

def h_resume(pipelinemode=None, filename=None, loglevel=None,
             plotlevel=None, weblog=None):
    pipeline = launcher.Pipeline(context=filename, loglevel=loglevel,
                                 plotlevel=plotlevel)
    
    cli.stack[cli.PIPELINE_NAME] = pipeline

    basetask.DISABLE_WEBLOG = not weblog

    return pipeline.context
