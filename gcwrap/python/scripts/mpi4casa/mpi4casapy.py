#!/usr/bin/env python
import os 
import sys
import time
import signal

# Create a watchdog to kill all processes in the group on exit
# as it seems that OpenMPI 1.10.2 only kills the main process
# but waits until all processes in the group finish
#
# Notice that it is not necessary to finalize MPI in the watchdog
# because it is not initialize either (MPI initialization happens
# when mpi4py is imported for the first time in MPIEnvironment)
if os.fork( ) == 0 :
    
    # Install signal handlers
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    signal.signal(signal.SIGHUP, signal.SIG_IGN)
    
    # Close standard input to avoid terminal interrupts
    sys.stdin.close( )
    sys.stdout.close( )
    sys.stderr.close( )
    os.close(0)
    os.close(1)
    os.close(2)
    
    # Check that parent process is alive
    ppid = os.getppid( )
    while True :
        try:
            os.kill(ppid,0)
        except:
            break
        time.sleep(3)

    # Kill process group
    os.killpg(ppid, signal.SIGTERM)
    sys.exit(1)
        
# CASA dictionary must be initialize here so that it can be found by stack inspection
casa = {}


# Use a function rather than import because with imports the Python import lock 
# is acquired by the main thread (this context) and other threads cannot import
def run():
    
    # Set CASA_ENGINGE env. variable so that:
    # - taskinit does not initialize the viewer
    # - taskmanmager is not initialized
    os.environ['CASA_ENGINE']="YES"

    # Initialize MPI environment
    from mpi4casa.MPIEnvironment import MPIEnvironment

    # Initialize MPICommunicator singleton
    from mpi4casa.MPICommunicator import MPICommunicator
    communicator = MPICommunicator()

    # Wait to receive start service signal from MPI client processor
    start_service_signal_available = False
    while not start_service_signal_available:
        start_service_signal_available = communicator.control_service_request_probe()
        if start_service_signal_available:
            # Receive CASA global dictionary (filtered)
            request = communicator.control_service_request_recv()
        else:
            time.sleep(MPIEnvironment.mpi_start_service_sleep_time)

    # Check if request is start or stop signal
    if request['signal'] == 'start':

        cli_logfile_name, logmode_name = 'casa_filtered', 'logmode'
        for entry_name in [cli_logfile_name, logmode_name]:
            if entry_name not in request:
                raise RuntimeError('A \'start\' MPI request must have a {0} entry but it '
                                   'was not found. Host name: {1}. MPI rank: {2}, '.
                                   format(entry_name, MPIEnvironment.hostname,
                                          MPIEnvironment.mpi_processor_rank))

        # Get CASA environment dictionary
        global casa
        casa_filtered = request['casa_filtered']
        casa.update(casa_filtered)
        global _casa_top_frame_
        _casa_top_frame_ = True

        
        # Re-set log file
        if request[logmode_name] == 'separated' or request[logmode_name] == 'redirect':
            casa['files']['logfile'] = ('{0}-server-{1}-host-{2}-pid-{3}'.
                                        format(casa['files']['logfile'],
                                               MPIEnvironment.mpi_processor_rank,
                                               MPIEnvironment.hostname,
                                               MPIEnvironment.mpi_pid))
    
        # Import logger, logfile is set at taskinit retrieving from the casa dict. from the stack
        from taskinit import casalog
        
        # Set log origin so that the processor origin is updated
        casalog_call_origin = "mpi4casapy"
        casalog.origin(casalog_call_origin)
        
        # If log mode is separated activate showconsole to have all logs sorted by time at the terminal
        if request[logmode_name] == 'redirect':
            casalog.showconsole(True)
            
        # Install filter to remove MSSelectionNullSelection errors
        casalog.filter('NORMAL1')
        casalog.filterMsg('MSSelectionNullSelection')
        casalog.filterMsg('non-existent spw')
        
        # Post MPI welcome msg
        casalog.post(MPIEnvironment.mpi_info_msg,"INFO",casalog_call_origin)
    
        # Initialize MPICommandServer and start serving
        from mpi4casa.MPICommandServer import MPICommandServer
        server = MPICommandServer()
                        
        server.serve()
        
    else:
        
        MPIEnvironment.finalize_mpi_environment()
