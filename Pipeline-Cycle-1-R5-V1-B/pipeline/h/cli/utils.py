from __future__ import absolute_import
import pprint
import types

import pipeline.infrastructure.api as api
import pipeline.infrastructure.argmapper as argmapper
import pipeline.infrastructure as infrastructure

import pipeline.h.cli.cli as cli
import pipeline.h.heuristics as heuristics
import pipeline

LOG = infrastructure.get_logger(__name__)

def get_context():
    return cli.stack[cli.PIPELINE_NAME].context

def get_output_dir():
    context = get_context()
    return context.output_dir

def get_ms(vis):
    context = get_context()
    return context.observing_run.get_ms(name=vis)

def get_heuristic(arg):
    if issubclass(arg, api.Heuristic):
        return arg()

    if callable(arg):
        return arg

    # TODO LOOK IN HEURISTICS MODULE
    
    # If the argument is a non-empty string, try to get the class with the
    # given name, or if that class doesn't exist, wrap the input in an
    # EchoHeuristic
    if type(arg) is types.StringType and arg:
        packages = arg.split('.')
        module = '.'.join(packages[:-1])
        # if arg was a raw string with no dots, module is empty
        if not module:
            return heuristics.EchoHeuristic(arg)
        
        try:
            m = __import__(module)
        except ImportError:
            return heuristics.EchoHeuristic(arg)
        for package in packages[1:]:
            m = getattr(m, package, heuristics.EchoHeuristic(arg))
        return m()
    
    return heuristics.EchoHeuristic(arg)

def execute_task(context, taskname, casa_args, fn_name):
    pipelinemode = casa_args.get('pipelinemode', None)
    dry_run = casa_args.get('dryrun', None)
    accept_results = casa_args.get('acceptresults', True)

    # get the pipeline task inputs
    task_inputs = _get_task_inputs(context, taskname, casa_args)
    
    # print them if necessary
    if pipelinemode == 'getinputs':
        _print_inputs(taskname, casa_args, task_inputs)
        return None

    # Execute the class, collecting the results
    results = _execute_task(taskname, fn_name, task_inputs, dry_run)

    # write the command invoked (eg. hif_setjy) to the result so that the
    # weblog can print help from the XML task definition rather than the
    # python class
    results.taskname = fn_name
    
    # accept the results if desired
    if accept_results and not dry_run:
        _merge_results(context, results)

    # before returning them
    return results


def _get_task_inputs(context, taskname, casa_args):
    # convert the CASA arguments to pipeline arguments, renaming and
    # converting as necessary.
    task_args = argmapper.convert_args(taskname, casa_args) 
    
    # get the inputs class for this task
    inputs_cls = pipeline.tasks.__dict__[taskname].Inputs
    
    inputs = inputs_cls(context, **task_args)

    return inputs

def _execute_task(taskname, stagename, task_inputs, dry_run):
    # Given the class and CASA name of the stage and the list
    # of stage arguments, compute and return the results.

    # Find the task and run it
    task_cls = getattr(pipeline.tasks, taskname)
    task = task_cls(task_inputs) 

    # Reporting stuff goes here

    # Error checking ?
    return task.execute(dry_run=dry_run)

def _merge_results(context, results):
    try:
        results.accept(context)
    except Exception, e:
        LOG.critical('Warning: Check merge to context for %s'
                     '' % results.__class__.__name__)
        raise e    

def _print_inputs(taskname, casa_args, task_inputs): 
    task_args = argmapper.convert_args(taskname, casa_args)

    pipeline_perspective = {}
    for arg in task_args:
        if hasattr(task_inputs, arg):
            pipeline_perspective[arg] = getattr(task_inputs, arg)

    casa_args = argmapper.task_to_casa(taskname, pipeline_perspective)

    print 'Pipeline-derived inputs for %s task:' % taskname    
    pprint.pprint(casa_args)
    
    # Resetting pipelinemode after a call to getinputs is not a good idea, as
    # it makes it very easy to unintentionally execute and commit tasks to the
    # context, plus the user most probably wants to tweak and call getinputs 
    # multiple times until the parameters look correct
#    a=inspect.stack()
#    stacklevel=0
#    for k in range(len(a)):
#        if (string.find(a[k][1], 'ipython console') > 0):
#            stacklevel=k
#            break
#    myf=sys._getframe(stacklevel).f_globals
#    myf['pipelinemode'] = 'automatic'
