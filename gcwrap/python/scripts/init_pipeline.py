
if casa['flags'].pipeline:
    if casa['dirs']['pipeline'] is not None:
        sys.path.insert(0,casa['dirs']['pipeline'])
        import pipeline
        pipeline.initcli()
    else:
        from init_welcome_helpers import immediate_exit_with_handlers
        print "Unable to locate pipeline installation, exiting"
        immediate_exit_with_handlers( )
