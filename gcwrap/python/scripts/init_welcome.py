import os
import sys
import traceback

if casa['flags'].execute:
    import os.path
    if os.path.isfile(casa['flags'].execute[0]):
        import contextlib

        ###
        ### redirect_argv(...) is used to present a consistent, clean sys.argv to
        ### the file being executed (via "-c somescript.py arg1, arg2..."). The
        ### redirect_exit(...) is used to substitue in the hard exit that comes
        ### with "os._exit(status)". Otherwise, ipython prevents "sys.exit(...)"
        ### exit and dumps us back to the ipython command line.
        ###
        def redirect_exit(msg=None):
            if isinstance(msg,int):
                os._exit(msg)
            elif msg is not None:
                sys.stderr.write(str(msg))
            os._exit(0)

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
            sys.argv=['casa','-c'] + args
            sys._save_exit = sys.exit
            sys.exit=redirect_exit
            yield
            sys.argv = sys._argv
            sys.exit = sys._save_exit

        try:
            with redirect_argv(casa['flags'].execute):
                execfile(casa['flags'].execute[0])
        except Exception, err:
            traceback.print_exc()
            os._exit(1)

        os._exit(0)
    else:
        eval(casa['flags'].execute[0])
        os._exit(0)
else:
    from casa_builtin import enable_builtin_protection,register_builtin

    register_builtin("casa")
    register_builtin("cu")
    register_builtin(["viewer","imview","msview"])

    enable_builtin_protection( )
    print "CASA Version " + casa['build']['version'] + "\n  Compiled on: " + casa['build']['time']


