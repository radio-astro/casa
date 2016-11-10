from casa_stack_manip import stack_find

def add_shutdown_hook( func ):
    shutdown_handlers = stack_find('casa_shutdown_handlers','root')
    if shutdown_handlers is None:
        raise RuntimeError("unable to find casa shutdown hook list")
    else:
        shutdown_handlers.append(func)
