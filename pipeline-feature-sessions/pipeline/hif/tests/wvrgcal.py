__rethrow_casa_exceptions=False
import sys
sys.path.append('/home/jfl/newpipeline/PIPELINE/Heuristics/src')
import pipeline


def test(vis):

    # import measurementsets to context
    context = pipeline.Pipeline().context
    inputs = pipeline.tasks.ImportData.Inputs(context, files=vis)
    task = pipeline.tasks.ImportData(inputs)
    results = task.execute(dry_run=False)
    results.accept(context)

#    # show summary
#    inputs = pipeline.tasks.summary.Summary.Inputs(context)
#    task = pipeline.tasks.summary.Summary(inputs)
#    task = pipeline.tasks.get_report(context, task, plot=True)
#    results = task.execute(dry_run=False)
#    # no results as such, but accept anyway to increment the stage number
##    results.accept()

    interfaceinputs = pipeline.tasks.wvrgcal.WvrgcalInterface.Inputs(
     context, stage_name='hif_wvrgcal', report='standard')
    task = pipeline.tasks.wvrgcal.WvrgcalInterface(interfaceinputs)
    results = task.execute(dry_run=False)
    results.accept(context)

