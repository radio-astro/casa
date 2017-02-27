__rethrow_casa_exceptions=False
import sys
sys.path.append('/Users/jfl/PipelineRefactoring-2012-05-B/PIPELINE/Heuristics/src')
import pipeline


def test(vis, dry_run=True):

    # import measurementsets to context
    context = pipeline.Pipeline().context
    inputs = pipeline.tasks.ImportData.Inputs(context, vis=vis)
    task = pipeline.tasks.ImportData(inputs)
    results = task.execute(dry_run=dry_run)
    results.accept(context)

    inputs = pipeline.tasks.clean.Clean.Inputs(
     context, field='3c279', cell='0.2arcsec', spw='1', mode='mfs', report='standard')
#     context, intent='TARGET', cell='0.2arcsec', spw='1', mode='frequency', nchan=10, report='standard')
    task = pipeline.tasks.clean.Clean(inputs)
    results = task.execute(dry_run=dry_run, pass_multiple_vis=True)
    results.accept(context)

