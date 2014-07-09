from taskinit import casalog

import pipeline.h.cli.cli as cli
import pipeline.infrastructure.launcher as launcher


def h_init(pipelinemode=None, loglevel=None, plotlevel=None, output_dir=None,
    overwrite=None, dryrun=None, acceptresults=None):

    # TBD: DECIDE WHETHER DRY RUN REALLY MAKES SENSE FOR THIS TASK AND IF
    # SO HOW TO IMPLEMENT IT.

    # TBD: CASA PARAMETER CHECKS BEFORE CREATING A CONTEXT ?
    
    # Create the pipeline and store the Pipeline object in the stack
    pipeline = launcher.Pipeline(output_dir=output_dir, loglevel=loglevel, 
                                 plotlevel=plotlevel)    
    cli.stack[cli.PIPELINE_NAME] = pipeline

    return pipeline.context
