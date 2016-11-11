import os
import sys
if os.environ.has_key('LD_PRELOAD'):
    del os.environ['LD_PRELOAD']

# jagonzal: MPIServer initialization before watchdog fork
from mpi4casa.MPIEnvironment import MPIEnvironment
if MPIEnvironment.is_mpi_enabled and not MPIEnvironment.is_mpi_client:
    import mpi4casa.mpi4casapy as mpi4casapy
    mpi4casapy.run()
    exit()

from IPython import start_ipython

__pylib = os.path.dirname(os.path.realpath(__file__))
__init_scripts = [
    "init_system.py",
    "init_user_pre.py",
    "init_mpi.py",
    "init_dbus.py",
    "init_tools.py",
    "init_logger.py",
    "init_tasks.py",
    "init_funcs.py",
    "init_asap.py",
    "init_testing.py",
    "init_user_post.py",
    "init_welcome.py",
]

##
## this is filled via add_shutdown_hook (from casa_shutdown.py)
##
casa_shutdown_handlers = [ ]

try:
    __startup_scripts = filter( os.path.isfile, map(lambda f: __pylib + '/' + f, __init_scripts ) )
    start_ipython( argv=["-c", "for i in " + str(__startup_scripts) + ": execfile( i )", "-i"] )
except: pass

### this should (perhaps) be placed in an 'atexit' hook...
for handler in casa_shutdown_handlers:
    handler( )
