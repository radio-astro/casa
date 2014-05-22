# Copyright (C) 2010
# Associated Universities, Inc. Washington DC, USA.
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be addressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#



casa_add_python( tools tools_mpi4casa_python python/${PYTHONV}/mpi4casa
  __init__.py
  MPIEnvironment.py  
  MPICommunicator.py
  MPICommandClient.py
  MPICommandServer.py
  MPIMonitorClient.py
  MPIMonitorServer.py
  mpi4casapy.py
  task_macros.py
  task_wrappers.py  
)
