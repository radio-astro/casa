
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

#print 'thelogfile:', thelogfile
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


# Log initialization ###################################################################################################

# IMPORTANT: The following steps must be follow the described order, 
#            otherwise a seg fault occurs when setting the log file.
# 1st Create casalog object, it will be used by tasks when importing taskinit
casalog = casac.logsink()
# 2nd Set log file accessing CASA dictionary of calling context via stack inspection
if casa.has_key('files') and casa['files'].has_key('logfile') :
    casalog.setlogfile(casa['files']['logfile'])

# 3rd Set logger as global
casalog.setglobal(True)

# Set processor origin (normally "casa" but in the MPI case we use the hostname and rank involved)
from mpi4casa.MPIEnvironment import MPIEnvironment
processor_origin = MPIEnvironment.processor_origin
casalog.processorOrigin(processor_origin)

# Log initialization ###################################################################################################

casalog.showconsole(MPIEnvironment.log_to_console or casa['flags'].log2term)
try:
    casalog.post("CASA Version " + casa['build']['version'])
except:
    print "Error: the logfile is not writable"
    sys.exit(1)

