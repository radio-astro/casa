import os
import traceback

from init_welcome_helpers import redirect_argv, immediate_exit_with_handlers


if casa['flags'].execute:
    import os.path

    if os.path.isfile(casa['flags'].execute[0]):
        # Run file with filename given in the command line
        try:
            with redirect_argv(casa['flags'].execute):
                execfile(casa['flags'].execute[0])
        except Exception, err:
            traceback.print_exc()
            immediate_exit_with_handlers(1)

        immediate_exit_with_handlers()

    else:
        # Run code given in the command line
        eval(casa['flags'].execute[0])
        immediate_exit_with_handlers()

else:
    from casa_builtin import enable_builtin_protection, register_builtin

    register_builtin("casa")
    register_builtin("cu")
    register_builtin(["viewer", "imview", "msview"])

    enable_builtin_protection()
    print "CASA Version " + casa['build']['version'] + "\n  Compiled on: " + casa['build']['time']
