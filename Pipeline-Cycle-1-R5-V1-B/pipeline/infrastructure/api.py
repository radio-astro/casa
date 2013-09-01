"""
The API module defines the common classes around which the pipeline is
structured.

A |Task| represents a coherent step in the data reduction process. A Task may
be composed of several other Tasks in order to create a 'super task'; this
'super task' could chains together several related steps in order to select
the best method to use for a particular step in the reduction process, for
example.

In addition, a |Task| may be composed of one or more Heuristics. Within the
pipeline, a heuristic is considered as a piece of code used to analyse data
and/or help influence a pipeline decision, such as whether to stop processing,
whether to run again with different parameters, etc. Where appropriate, this
code is extracted from the main body of the pipeline code and exposed as a 
|Heuristic|.

*Something about Task parameters goes here*

If a user desires more than to simply override a Heuristic-derived value with
a Task parameter, it is anticipated that they will edit the appropriate
Heuristic in order to tweak the existing implementation or introduce a new
algorithm into the pipeline. Concrete |Heuristic| implementations can be found
in the :mod:`pipeline.heuristics` package. 

|Inputs|, in a general sense, are considered as the mandatory arguments for a
pipeline |Task|. Tasks and Inputs are closely aligned, and just as Tasks
differ in functionality, so the number and type of |Inputs| arguments will
differ from implementation to implementation. Refer to the documentation of
the implementing subclass for definitive information on the |Inputs| for a
specific Task.

It is anticipated that the Inputs for a Task will be created using the
:attr:`Task.Inputs` reference rather than locating and instantiating the
partner class directly, eg.::

    i = ImplementingTask.Inputs.create_from_context(context)

.. |Task| replace:: :class:`Task`
.. |Heuristic| replace:: :class:`Heuristic`
.. |Inputs| replace:: :class:`Inputs`
.. |Results| replace:: :class:`Results`

"""
from __future__ import absolute_import
import abc


class Heuristic(object):
    """
    Heuristic is the superclass of all user-accessible heuristics code in the
    pipeline. 

    A heuristic is a small, self-contained piece of code that calculate the
    optimal value(s) for a particular task or argument. Tasks may be composed
    of multiple heuristics; while the Task and Heuristic may be closely
    coupled, the Heuristic itself should not depend on the calling task,
    allowing it to be used in other tasks.

    Examples of heuristics are functions to score caltables, allowing them to 
    be ranked, or a function to calculate the optimal solution interval for a
    particular measurement set.
    """
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def calculate(self, *args, **parameters):
        """
        Make a calculation based on the given parameters.

        This is an abstract method and must be implemented by all Heuristic
        subclasses.

        .. note::
            The signature and return types of :meth:`~Heuristic.calculate`
            are intended to be implementation specific. Refer to the
            documentation of the implementing class for the appropriate
            signature.

        """
        raise NotImplementedError

    def __call__(self, *args, **parameters):
        """
        Redirects calls on a Heuristic instance to Heuristic.calculate().
        """
        return self.calculate(*args, **parameters)

    def __repr__(self):
        return self.__class__.__name__


class Inputs(object):
    """
    Inputs defines the interface used to create a constructor argument for a
    :class:`Task`.    
    """
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def to_casa_args(self):
        """
        Get this Inputs object as a dictionary of CASA arguments.

        :rtype: a dictionary of CASA arguments
        """
        raise NotImplementedError


class Results(object):
    """
    Results defines the interface used to hold the output of a |Task| plus
    some common parameters used by all weblog templates. This class is
    expected to be the base class of a more specialised task-specific class.
    Refer to the sub-class for details on the structure expected by the 
    task-specific weblog template.

    .. py:attribute:: task_class

        the Class of the |Task| that generated this Results object.

    .. py:attribute:: inputs

        the |Inputs| used by the |Task| that generated this |Results|

    .. py:attribute:: timestamps

        the (:class:`~pipeline.infrastructure.basetask.Timestamps`) named
        tuple holding the start and end timestamps for the |Task|

    .. py:attribute:: stage_number

        the stage number of the task that generated this |Results|

.. |Inputs| replace:: :class:`Inputs`
.. |Task| replace:: :class:`Task`
.. |Results| replace:: :class:`Results`

    """
    __metaclass__ = abc.ABCMeta

    @abc.abstractproperty
    def uuid(self):
        """
        The unique identifier for this results object.
        """
        raise NotImplementedError

    @abc.abstractmethod
    def accept(self, context):
        """
        Accept these results, registering objects with the context and incrementing
        stage counters as necessary in preparation for the next task.
        """


class ResultRenderer(object):
    """
    ResultRenderer is the interface for task-specific weblog renderers
    (T2-4M details pages in weblog nomenclature).

    Every ResultRenderer implementation must define two things: the task
    whose results the implementation renderers and the 'render' implementation
    itself.

    The main weblog renderer queries the abstract base class registrations to
    find all ResultRenderers. Multiple results renderers may be registered for
    a single task; the main renderer will select the appropriate renderer
    b ased on sort order.
    """
    __metaclass__ = abc.ABCMeta

    @abc.abstractproperty
    def task(self):
        """
        The result class this renderer should handle.
        """
        raise NotImplementedError

    @abc.abstractmethod
    def render(result, context):
        raise NotImplementedError


class Task(object):
    """
    The Runnable interface should be implemented by any class whose
    instances are intended to be executed by an Executor.

    Duck typing means that implementing classes need not extend this
    'interface', but doing so registers that class as an implementation with
    the Python abstract base class mechanism. Future versions of the pipeline
    may query these registrations.

    Implementing classes:
    pipeline.tasks.TaskTemplate
    pipeline.infrastructure.JobRequest
    """
    __metaclass__ = abc.ABCMeta

    @abc.abstractproperty
    def Inputs(self):
        """A reference to the accompanying :class:`Inputs` partner class that
        comprises the mandatory arguments for this Task.

        """
        raise NotImplementedError

    def __init__(self, inputs):
        """
        Create a new Task with an initial state based on the given inputs.

        :param Inputs inputs: inputs required for this Task.
        """
        # complain if we were given the wrong type of inputs
        if not isinstance(inputs, self.Inputs):
            msg = '{0} requires inputs of type {1} but got {2}.'.format(
                self.__class__.__name__,
                self.Inputs.__name__,  
                inputs.__class__.__name__)
            raise TypeError, msg

        self.inputs = inputs

    @abc.abstractmethod
    def execute(self, dry_run=True, **parameters):
        """
        Run this task and return the :class:`Results`.

        The contract of the method execute is that it may take any action
        whatsoever, providing that it returns an instance implementing Results
        that summarises those actions.

        :param boolean dry_run: when set to True, runs the Task and logs any
            operations that would be performed without executing them.

        """
        raise NotImplementedError
