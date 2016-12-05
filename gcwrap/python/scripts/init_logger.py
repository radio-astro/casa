
def casalogger(logfile=''):
    """
    Spawn a new casalogger using logfile as the filename.
    You should only call this if the casalogger dies or you close it
    and restart it again.

    Note: if you changed the name of the log file using casalog.setlogfile
    you will need to respawn casalogger with the new log filename. Eventually,
    we will figure out how to signal the casalogger with the new name but not
    for a while.
    """

    if logfile == '':
        if casa.has_key('files') and casa['files'].has_key('logfile') :
            logfile = casa['files']['logfile']
        else:
            casa['files']['logfile'] = os.getcwd( ) + '/casa.log'
            logfile = 'casa.log'

    if (os.uname()[0]=='Darwin'):
        if casa['helpers']['logger'] == 'console':
            casa['procmgr'].create("logger",['/usr/bin/open','-a','console', logfile])
        else:
            casa['procmgr'].create("logger",[casa['helpers']['logger'],logfile])

    elif (os.uname()[0]=='Linux'):
        casa['procmgr'].create("logger",[casa['helpers']['logger'],logfile])

    else:
        print 'Unrecognized OS: No logger available'


thelogfile = ''

if casa['files'].has_key('logfile') :
    thelogfile = casa['files']['logfile']
if casa['flags'].nologfile:
    thelogfile = 'null'

deploylogger = True

if not os.access('.', os.W_OK) :
    print 
    print "********************************************************************************"
    print "Warning: no write permission in current directory, no log files will be written."
    print "********************************************************************************"
    deploylogger = False
    thelogfile = 'null'
    
if casa['flags'].nologger :
    deploylogger = False

if casa['flags'].nogui :
    deploylogger = False

if thelogfile == 'null':
    pass
else:
    if thelogfile.strip() != '' :
        if deploylogger:
            casalogger(thelogfile)
    else:
        thelogfile = 'casapy-'+time.strftime("%Y%m%d-%H%M%S", time.gmtime())+'.log'
        try:
            open(thelogfile, 'a').close()
        except:
            pass
        if deploylogger:
            casalogger(thelogfile)


casalog = casac.logsink()

if casa.has_key('files') and casa['files'].has_key('logfile') :
    casalog.setlogfile(casa['files']['logfile'])

from mpi4casa.MPIEnvironment import MPIEnvironment
processor_origin = MPIEnvironment.processor_origin
casalog.processorOrigin(processor_origin)

casalog.showconsole((MPIEnvironment.is_mpi_enabled and MPIEnvironment.log_to_console) or casa['flags'].log2term)

###
### For reasons unknown, setglobal causes the global logger to steal the
### log sink associated with this casalog object we've created, making
### our casalog object useless for the purposes of configuration. As a
### result, it's best to call this last.
###
casalog.setglobal(True)

try:
    casalog.post("CASA Version " + casa['build']['version'])
except:
    print "Error: the logfile is not writable"
    sys.exit(1)

