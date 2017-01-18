###
### Initialize the crash dump feature
###

import signal
import tempfile

try:
    temporaryDirectory = tempfile.gettempdir()
    posterApp = casa['helpers']['crashPoster']
    if posterApp is None: posterApp = "" # handle case where it wasn't found
    postingUrl = "https://casa.nrao.edu/cgi-bin/crash-report.pl"
    theLogFile = casa['files']['logfile']
    message = casac.utils()._crash_reporter_initialize(temporaryDirectory, posterApp, postingUrl, theLogFile)
    if len (message) > 0:
        print ("***\n*** Crash reporter failed to initialize: " + message)
except Exception as e:
    print "***\n*** Crash reporter initialization failed.\n***"
    print "*** exception={0}\n***".format (e)
