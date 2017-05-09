At the highest level of abstraction, we can execute a pipline processing request (ppr)
This can be done at the command line, or at a CASA command prompt.

Execute PPR from the command line:
```
$ casa --nologger --nogui -c $SCIPIPE_HEURISTICS/pipeline/runvlapipeline.py PPRnew_VLAT003.xml

$ casa --nogui --log2term -c $SCIPIPE_HEURISTICS/pipeline/runvlapipeline.py PPRnew_VLAT003.xml
```
At a casa command prompt:

```
$ casa
# execute a pipeline processing request (PPR)
CASA <1>: import pipeline.infrastructure.executevlappr as eppr
CASA <2>: eppr.executeppr('PPR_VLAT003.xml', importonly=False)
```

A little lower level of abstraction would be to run the pipeline as a series of
steps as described in the VLA pipeline [casaguide](
https://casaguides.nrao.edu/index.php/VLA_CASA_Pipeline-CASA4.5.3)

At the lowest level of abstraction, we can run the pipeline as a series of steps
like the following

A pipeline run will generate a file like the following
```
  pipeline_test_data/VLAT003/working/pipeline-20161014T172229/html/casa_pipescript.py
 ```
 It contains a series of steps like those described in the casaguide.

We can execute this script from CASA
```
CASA <1>: execfile('casa_pipescript.py')
```

Or we can run it from the command line:
```
casa --nogui --log2term -c casa_pipescript.py
```

We can edit the script and turn on memory usage for each task:
```
CASA <1>: h_init()
CASA <2>: import pipeline
CASA <3>: pipeline.infrastructure.utils.enable_memstats()
```

We can also turn weblog and plotting off:
```
CASA <1>: h_init(pipelinemode="automatic",loglevel="info",plotlevel="summary",output_dir="./",weblog=False,overwrite=True,dryrun=False,acceptresults=True)
```

Or we can turn debug mode on, weblog off:
```
CASA <1>: h_init(pipelinemode="automatic",loglevel="debug",plotlevel="summary",output_dir="./",weblog=True,overwrite=True,dryrun=False,acceptresults=True)
```

At the lowest level of abstraction

```
CASA <1>: h_init()
CASA <2>: h_save()
CASA <3>: import pipeline
CASA <4>: import pipeline.infrastructure.casatools as casatools
CASA <5>: hifv_importdata(vis=['../rawdata/13A-537.sb24066356.eb24324502.56514.05971091435'], session=['session_1'], overwrite=False)
CASA <6>: h_save()
CASA <7>: exit
```

```
casa
CASA <1>: context = h_resume()
```

Alternatively
```
CASA <1>: context = pipeline.Pipeline(context='last').context

CASA <1>: vis='13A-537.sb24066356.eb24324502.56514.05971091435.ms'
CASA <2>: m = context.observing_run.get_ms(vis)
CASA <3>: spws=m.get_spectral_windows()

CASA <4>: inputs = pipeline.hifv.tasks.hanning.Hanning.Inputs(context)
CASA <5>: task = pipeline.hifv.tasks.hanning.Hanning(inputs)
CASA <6>: result = task.execute(dry_run=False)
CASA <7>: result.accept(context)
CASA <8>: context.save() 
```

If we don't have a PPR or an executable script available.

```
casa
import pipeline
import pipeline.recipes.hifv as hifv
# the next line will only importevla and save a context, b/c importonly=True
hifv.hifv(['../rawdata/13A-537.foofoof.eb.barbar.2378.2934723984397'], importonly=True)

context = pipeline.Pipeline(context='last').context
vis = '13A-537.foofoof.eb.barbar.2378.2934723984397.ms'
# get the domain object
m = context.observering_run.get_ms(vis)
type(m)
# study this m object for INTENTS
# <class 'pipeline.domain.measurementset.MeasurementSet>
m.intents  # shows a python set of the MS intents
m.polarization  # show a list of polarization objects
```

To run one of the standard recipes we can use a recipereducer

```
import pipeline.recipereducer
pipeline.recipereducer.reduce(vis=['../rawdata/yourasdm'], procedure='procedure_hifv.xml')
```

Known issues

Don't worry about the following intermittent message at the end of a pipeline run. It's a bug
  but it doesn't mean the pipeline was unsuccessful.

```
invalid command name "102990944filter_destroy"
    while executing
"102990944filter_destroy 3657 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 0 ?? ?? .86583632 17 ?? ?? ??"
    invoked from within
"if {"[102990944filter_destroy 3657 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 0 ?? ?? .86583632 17 ?? ?? ??]" == "break"} break"
    (command bound to event)
```