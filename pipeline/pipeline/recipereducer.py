"""
recipereducer is a utility to reduce data using a standard pipeline procedure.
It parses a XML reduction recipe, converts it to pipeline tasks, and executes
the tasks for the given data. It was written to give pipeline developers 
without access to PPRs and/or a PPR generator a way to reduce data using the
latest standard recipe.

Note: multiple input datasets can be specified. Doing so will reduce the data
      as part of the same session.

Example #1: process uid123.tar.gz using the standard recipe. 

    import pipeline.recipereducer
    pipeline.recipereducer.reduce(vis=['uid123.tar.gz'])

Example #2: process uid123.tar.gz using a named recipe.

    import pipeline.recipereducer
    pipeline.recipereducer.reduce(vis=['uid123.tar.gz'], 
                                  procedure='procedure_hif.xml')

Example #3: process uid123.tar.gz and uid124.tar.gz using the standard recipe.

    import pipeline.recipereducer
    pipeline.recipereducer.reduce(vis=['uid123.tar.gz', 'uid124.tar.gz']) 

Example #4: process uid123.tar.gz, naming the context 'testrun', thus
            directing all weblog output to a directory called 'testrun'. 

    import pipeline.recipereducer
    pipeline.recipereducer.reduce(vis=['uid123.tar.gz'], name='testrun') 
    
Example #5: process uid123.tar.gz with a log level of TRACE

    import pipeline.recipereducer
    pipeline.recipereducer.reduce(vis=['uid123.tar.gz'], loglevel='trace') 

"""
import os
import traceback
import xml.etree.ElementTree as ElementTree

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.casataskdict as casataskdict
import pipeline.infrastructure.launcher as launcher

LOG = logging.get_logger(__name__)

recipes_dir = os.path.join(os.path.dirname(__file__), 'recipes')

def _create_context(loglevel='info', name=None):
    return launcher.Pipeline(loglevel=loglevel, name=name).context

def _get_context_name(procedure):
    root, _ = os.path.splitext(procedure)
    return 'pipeline-%s' % root

def _get_task_class(cli_command):
    for k, v in casataskdict.classToCASATask.items():
        if v == cli_command:
            return k
    raise KeyError, '%s not registered in casataskdict' % cli_command

def _get_tasks(context, vis, infiles, procedure='procedure_hifa.xml'):
    procedure_file = os.path.join(recipes_dir, procedure)
    
    processingprocedure = ElementTree.parse(procedure_file)
    if not processingprocedure:
        LOG.error('Could not parse procedure file at %s.\n'
                  'Execution halted' % procedure_file)
    
    for processingcommand in processingprocedure.findall('ProcessingCommand'):
        cli_command = processingcommand.findtext('Command')
        task_class = _get_task_class(cli_command)

        task_args = {}
        
        if cli_command in ['hif_importdata',
                           'hifa_importdata',
                           'hif_restoredata']:
            task_args['vis'] = vis 
        elif cli_command in ['hsd_importdata', 'hsd_restoredata']:
            task_args['infiles'] = infiles

        for parameterset in processingcommand.findall('ParameterSet'):
            for parameter in parameterset.findall('Parameter'):
                argname = parameter.findtext('Keyword')
                argval = parameter.findtext('Value')
                task_args[argname] = argval

        task_inputs = task_class.Inputs(context, **task_args)
        task = task_class(task_inputs)
        task._hif_call = _as_task_call(task_class, task_args) 
        # we yield rather than return so that the context can be updated
        # between task executions 
        yield task

def _format_arg_value(arg_val):
    arg, val = arg_val
    return '%s=%r' % (arg, val)

def _as_task_call(task_class, task_args):
    kw_args = map(_format_arg_value, task_args.items())
    return '%s(%s)' % (task_class.__name__, ', '.join(kw_args))
            
def reduce(vis=[], infiles=[], procedure='procedure_hifa.xml', context=None, name=None, 
           loglevel='info'):
    if context is None:
        name = name if name else _get_context_name(procedure)
        context = _create_context(loglevel=loglevel, name=name)

    task_generator = _get_tasks(context, vis, infiles, procedure)    
    try:
        while True:
            task = next(task_generator)
            LOG.info('Executing pipeline task %s' % task._hif_call)
    
            try:
                result = task.execute(dry_run=False)
                result.accept(context)
            except:
                LOG.error('Error executing pipeline task %s.' % task._hif_call)
                traceback.print_exc()
                return context
    except StopIteration:
        pass
    finally:
        LOG.info('Saving context...')
        context.save()
        
    return context
