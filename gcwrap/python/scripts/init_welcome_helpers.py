import contextlib
import os
import sys


def immediate_exit_with_handlers(exit_status=0):
    """
    Exits using os._exit(exit_status), to terminate when running in
    IPython.start_ipython() (which captures SystemExit and all exceptions).
    But before that abrupt exit it ensures that all the exit handlers
    registerd with atexit are run.
    """
    def run_atexit_handlers():
        # Note: the drawback of this approach is that atexit._exithandlers is
        # NOT available in Python 3
        import atexit
        # This could be done with atexit._run_exitfuncs(), but let's avoid
        # using one more protected function:
        for handler_tuple in reversed(atexit._exithandlers):
            # Try/except is to avoid application process hanging when the 
            # background process (tcl in particular)
            # is already destroyed
            try:
                handler_tuple[0]()
            except:
                pass

    run_atexit_handlers()
    import os
    os._exit(exit_status)


###
### redirect_argv(...) is used to present a consistent, clean sys.argv to
### the file being executed (via "-c somescript.py arg1, arg2..."). The
### redirect_exit(...) is used to substitue in the hard exit that comes
### with "os._exit(status)". Otherwise, ipython prevents "sys.exit(...)"
### exit and dumps us back to the ipython command line.
###
def redirect_exit(msg=None):
    if isinstance(msg, int):
        immediate_exit_with_handlers(msg)
    elif msg is not None:
        sys.stderr.write(str(msg))
    immediate_exit_with_handlers(0)


@contextlib.contextmanager
def redirect_argv(args):
    sys._argv = sys.argv[:]
    ###
    ### this could be cleaned up to be just:
    ###
    ###    sys.argv = ['somescript.py', arg1, arg2...]
    ###
    ### but casa scripts seem to have been
    ### expecting the 'casa' arg as well as the '-c' arg
    sys.argv = ['casa', '-c'] + args
    sys._save_exit = sys.exit
    sys.exit = redirect_exit
    yield
    sys.argv = sys._argv
    sys.exit = sys._save_exit
