from mpi4casa.MPIEnvironment import MPIEnvironment
# jagonzal: MPIClient initialization after watchdog fork
if MPIEnvironment.is_mpi_enabled:
    # Instantiate MPICommunicator singleton in order not to block the clients
    from mpi4casa.MPICommunicator import MPICommunicator
    mpi_comunicator = MPICommunicator()    
    # Post MPI related info
    casalog.post(MPIEnvironment.mpi_info_msg,"INFO","casa" )
