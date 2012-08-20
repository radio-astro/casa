#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
#
# This file is part of CASA.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

casa_add_dbus_interface( plotserver/PlotServerProxy.dbusproxy.h plotserver/PlotServerProxy.dbusproxy.xml )
casa_add_dbus_interface( session/DBusSession.dbusproxy.h session/DBusSession.dbusproxy.xml )
casa_add_dbus_interface( viewer/ViewerProxy.dbusproxy.h viewer/ViewerProxy.dbusproxy.xml )

install( FILES 
	${CMAKE_CURRENT_BINARY_DIR}/plotserver/PlotServerProxy.dbusproxy.h
	DESTINATION include/casacode/casadbus/plotserver
	)
install( FILES 
	${CMAKE_CURRENT_BINARY_DIR}/viewer/ViewerProxy.dbusproxy.h
	DESTINATION include/casacode/casadbus/viewer
	)
install( FILES 
	${CMAKE_CURRENT_BINARY_DIR}/session/DBusSession.dbusproxy.h
	DESTINATION include/casacode/casadbus/session
	)

casa_add_library( casadbus
  plotserver/PlotServerProxy.cc
  session/DBusSession.cc
  session/Dispatcher.cc
  types/record.cc
  types/variant.cc
  types/nullptr.cc
  utilities/BusAccess.cc
  utilities/Conversion.cc
  viewer/ViewerProxy.cc
 
plotserver/PlotServerProxy.dbusproxy.h
session/DBusSession.dbusproxy.h
viewer/ViewerProxy.dbusproxy.h  )

install (FILES
	plotserver/PlotServerProxy.h
	DESTINATION include/casacode/casadbus/plotserver
	)
install (FILES
	session/DBusSession.h
	session/Dispatcher.h
	DESTINATION include/casacode/casadbus/session
	)
install (FILES
	types/functor.h
	types/nullptr.h
	types/ptr.h
	types/record.h
	types/variant.h
	DESTINATION include/casacode/casadbus/types
	)
install (FILES
	utilities/BusAccess.h
	utilities/Conversion.h
	DESTINATION include/casacode/casadbus/utilities
	)
install (FILES
	viewer/ViewerProxy.h
	DESTINATION include/casacode/casadbus/viewer
	)
