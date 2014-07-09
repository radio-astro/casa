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

    interfaceinputs = pipeline.tasks.tsyscal.TsyscalInterface.Inputs(
     context, stage_name='hif_tsyscal', report='standard')
    task = pipeline.tasks.tsyscal.TsyscalInterface(interfaceinputs)
    results = task.execute(dry_run=False)
    results.accept(context)

    interfaceinputs = pipeline.tasks.tsyscal.TsysflagInterface.Inputs(
     context, stage_name='hif_tsysflag', report='standard', 
     flag_hilo=True, fhl_limit=10.0, fhl_minsample=5, fhl_colour='red',
     flag_tmf1=True, tmf1_axis='Time', tmf1_limit=0.4, tmf1_colour='yellow',
     niter=1)
    task = pipeline.tasks.tsyscal.TsysflagInterface(interfaceinputs)
    results = task.execute(dry_run=False)
    results.accept(context)

    interfaceinputs = pipeline.tasks.tsyscal.TsyscalflagInterface.Inputs(
     context, report='standard')
    task = pipeline.tasks.tsyscal.TsyscalflagInterface(interfaceinputs)
    results = task.execute(dry_run=False)
    results.accept(context)

#    interfaceinputs = pipeline.tasks.tsyscal.TsyscalflagInterface.Inputs(
#     context, report='standard',
#     flag_hilo=True, fhl_limit=10.0, fhl_minsample=5, fhl_colour='red',
#     flag_tmf1=True, tmf1_axis='Antenna', tmf1_limit=0.1, tmf1_colour='blue',
#     niter=1)
#    task = pipeline.tasks.tsyscal.TsyscalflagInterface(interfaceinputs)
#    results = task.execute(dry_run=False)
#    results.accept(context)

