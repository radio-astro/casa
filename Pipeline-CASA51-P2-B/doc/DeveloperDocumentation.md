# Pipeline developer documentation

This is to be populated, as time permits, with information that may be useful for other pipeline developer team members.

Think of it as a place for a new team member to look for useful bits of information, as a refresher for experienced team members, or as a reference for a developer unfamiliar with certain areas of the pipeline.

## Possible sections

### Context

How to restore a pipeline context:

```
CASA <1>: context = h_resume(filename='last')
```
or

```
CASA <1>: import pipeline
CASA <2>: context = pipeline.Pipeline(context='last').context
```

### Imaging

### Task interface

### Domain Object

```
CASA <1>: import pipeline
CASA <2>: context = pipeline.Pipeline(context='last').context

CASA <3>: vis = 'myvis.ms'
CASA <4>: m = context.observing_run.get_ms(vis)
```
and then use the domain object to get an Spectral Window object
```
CASA <5>: spws=m.get_spectral_windows()
```
