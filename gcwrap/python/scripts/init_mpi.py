# jagonzal: MPIServer initialization before watchdog fork
from mpi4casa.MPIEnvironment import MPIEnvironment
if MPIEnvironment.is_mpi_enabled and not MPIEnvironment.is_mpi_client:
    import mpi4casa.mpi4casapy as mpi4casapy
    mpi4casapy.run()
    # Servers make sure to exit here
    from init_welcome_helpers import immediate_exit_with_handlers
    immediate_exit_with_handlers(0)

# jagonzal: MPIClient initialization after watchdog fork
if MPIEnvironment.is_mpi_enabled:
    # Instantiate MPICommunicator singleton in order not to block the clients
    from mpi4casa.MPICommunicator import MPICommunicator
    mpi_comunicator = MPICommunicator()    
    # Post MPI related info
    casalog.post(MPIEnvironment.mpi_info_msg,"INFO","casa" )
