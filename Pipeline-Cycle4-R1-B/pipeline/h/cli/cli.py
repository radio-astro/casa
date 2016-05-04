import inspect
import string
import sys

def __get_globals():
    stack = inspect.stack()
    stacklevel=0
    for i in range(len(stack)):
        if (string.find(stack[i][1], 'ipython console') > 0):
            return sys._getframe(stacklevel).f_globals

    # Lindsey's ipython sessions do not have an 'ipython console' frame
    return globals()

# the name of the ipython global variable that will hold the pipeline
PIPELINE_NAME = '_heuristics_context'

stack = __get_globals()
