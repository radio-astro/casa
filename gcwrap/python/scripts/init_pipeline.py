
if casa['flags'].pipeline:
    if casa['dirs']['pipeline'] is not None:
        sys.path.insert(0,casa['dirs']['pipeline'])
        import pipeline
        pipeline.initcli()
    else:
        print "Unable to locate pipeline installation, exiting"
        sys.exit(1)
