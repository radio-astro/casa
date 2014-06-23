#!/usr/bin/env python
import os 
import sys
import time

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
            # Receive CASA global dictionary
            request = communicator.control_service_request_recv()
        else:
            time.sleep(MPIEnvironment.mpi_start_service_sleep_time)
      
    # Check if request is start or stop signal
    if request['signal'] == 'start':
    
        # Get CASA environment dictionary
        global casa
        casa = request['casa']
    
        # Import logger
        from taskinit import casalog
        
        # Set log origin so that the processor origin is updated
        casalog_call_origin = "mpi4casapy"
        casalog.origin(casalog_call_origin)
        
        # Post MPI welcome msg
        casalog.post(MPIEnvironment.mpi_info_msg,"INFO",casalog_call_origin)
    
        # Initialize MPICommandServer and start serving
        from mpi4casa.MPICommandServer import MPICommandServer
        server = MPICommandServer()
                        
        server.serve()
        
    else:
        
        MPIEnvironment.finalize_mpi_environment()
    
        