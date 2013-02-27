from taskinit import casalog

#import pipeline.cli.utils as utils
import pipeline.h.cli.utils as utils
import pipeline.tasks as tasks
import pipeline.tasks.summary as summary

def hif_summary(pipelinemode=None, plot=True, dryrun=None):
    context = utils.get_context()
    
    # vis is not yet enabled as it requires changes to the display code
#    if vis in ('', None):
#        vis = [ms.name for ms in context.measurement_sets]

    inputs = summary.Summary.Inputs(context)
    if pipelinemode == 'getinputs':
        return utils.print_inputs(inputs)

    task = summary.Summary(inputs)
    if not dryrun:
        task = tasks.get_report(context, task, plot=plot)

    results = task.execute(dry_run=dryrun)
    # no results as such, but accept anyway to increment the stage number
    if not dryrun:
        results.accept()
    
    return results
