# Create a pipeline task

The purpose of this module is to bootstrap the creation of a task, thus 
saving time and making it easier to not miss essential steps.

* To create a task, simply execute new_pipeline_task.py from the command line 
or use the new_pipeline_task module within a CASA session.  For example, if I 
want to create task "foo" in the package "hif", I could do one of the following.

    Command line:
    ```
    casa --nogui --nologger -c $SCIPIPE_HEURISTICS/pipeline/infrastructure/new_pipeline_task/new_pipeline_task.py --package hif --task foo
    ```
    
    Within CASA:
    ```python
     from pipeline.infrastructure.new_pipeline_task import new_pipeline_task
       newtask = new_pipeline_task.NewTask()
       newtask.create('hif', 'foo')
    ```
    
* There is an optional "--module" parameter if you want to give the task module
file a different name than the task itself.  For example, if task is 'foo' 
and module is 'bar' then 'tasks/foo/bar.py' is created.

    Command line:
    ```
    casa --nogui --nologger -c $SCIPIPE_HEURISTICS/pipeline/infrastructure/new_pipeline_task/new_pipeline_task.py --package hif --task foo --module bar
    ```


* To verify the new task is working, you should run the module both as a pipeline 
task and by calling it directly as registered CASA task.  There is a script that 
allows you to do this automatically.  For example, to verify the task above we 
can the following.  Note that a test MS is required to run the tests.

    Command line:
    ```
    casa --nogui --nologger -c $SCIPIPE_HEURISTICS/pipeline/infrastructure/new_pipeline_task/verify_new_pipeline_task.py --package hif --task foo --test-ms small.ms
    ```
