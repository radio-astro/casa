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
    from casa_builtin import enable_builtin_protection, register_builtin

    register_builtin("casa")
    register_builtin("cu")
    register_builtin(["viewer", "imview", "msview"])

    enable_builtin_protection()
    #print "CASA Version " + casa['build']['version'] + "\n  Compiled on: " + casa['build']['time']
