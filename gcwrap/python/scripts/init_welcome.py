import os
import traceback

from init_welcome_helpers import redirect_argv, immediate_exit_with_handlers

if casa['flags'].execute:
    import os.path

    if '/' in casa['flags'].execute[0]:
        ## qualified path
        __paths_to_check = [ '' ]
    else:
        ## non-qualified path
        __paths_to_check = [ "./", casa['dirs']['python'] + '/' ]

    __candidates = filter( os.path.isfile, map(lambda dir: dir + casa['flags'].execute[0], __paths_to_check) )

    if len(__candidates) > 0:
        # Run file with filename given in the command line
        try:
            with redirect_argv(casa['flags'].execute):
                execfile(__candidates[0])
        except Exception, err:
            traceback.print_exc()
            immediate_exit_with_handlers(1)

        immediate_exit_with_handlers()

    else:
        try:
            exec(casa['flags'].execute[0])
        except Exception, err:
            traceback.print_exc()
            os._exit(0)

        immediate_exit_with_handlers()

else:

    ###
    ### Revisit if we ever get rid of plotcal al a matplotlib...
    ###
    ### ----------------------------------------------------------------------
    ### without this we get errors when our customized TkInter backend
    ### is being used in an open matplotlib window like:
    ### ----------------------------------------------------------------------
    ### Error in sys.exitfunc:
    ### Traceback (most recent call last):
    ###  File "/opt/casa/02/lib/python2.7/atexit.py", line 24, in _run_exitfuncs
    ###    func(*targs, **kargs)
    ###  File "/opt/casa/02/lib/python2.7/site-packages/matplotlib/_pylab_helpers.py", line 82, in destroy_all
    ###    manager.destroy()
    ###  File "/opt/casa/02/lib/python2.7/site-packages/matplotlib/backends/backend_tkagg.py", line 453, in destroy
    ###    self.window.destroy()
    ###  File "/opt/casa/02/lib/python2.7/lib-tk/Tkinter.py", line 1860, in destroy
    ###    self.tk.call('destroy', self._w)
    ###_tkinter.TclError: can't invoke "destroy" command:  application has been destroyed
    class ___protect_exit(object):
        "direct shutdown errors to /dev/null"
        def __init__( self, handler ):
            self.__handler = handler

        def __call__( self ):
            try:
                self.handler( )
            except:
                pass

    sys.exitfunc = ___protect_exit(sys.exitfunc)

    from casa_builtin import enable_builtin_protection, register_builtin

    ###
    ### backward compatibility at the command line...
    ###
    T = True
    F = False
    true = True
    false = False
    register_builtin("T")
    register_builtin("F")
    register_builtin("true")
    register_builtin("false")

    register_builtin("casa")
    register_builtin("cu")
    register_builtin(["viewer", "imview", "msview"])

    enable_builtin_protection()
    #print "CASA Version " + casa['build']['version'] + "\n  Compiled on: " + casa['build']['time']
