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

casa_add_dbus_interface( plotserver/PlotServerProxy.dbusproxy.h implement/plotserver/PlotServerProxy.dbusproxy.xml )
casa_add_dbus_interface( session/DBusSession.dbusproxy.h implement/session/DBusSession.dbusproxy.xml )
casa_add_dbus_interface( viewer/ViewerProxy.dbusproxy.h implement/viewer/ViewerProxy.dbusproxy.xml )

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
  implement/plotserver/PlotServerProxy.cc
  implement/session/DBusSession.cc
  implement/session/Dispatcher.cc
  implement/types/record.cc
  implement/types/variant.cc
  implement/types/nullptr.cc
  implement/utilities/BusAccess.cc
  implement/utilities/Conversion.cc
  implement/viewer/ViewerProxy.cc
 
plotserver/PlotServerProxy.dbusproxy.h
session/DBusSession.dbusproxy.h
viewer/ViewerProxy.dbusproxy.h  )

install (FILES
	implement/plotserver/PlotServerProxy.h
	DESTINATION include/casacode/casadbus/plotserver
	)
install (FILES
	implement/session/DBusSession.h
	implement/session/Dispatcher.h
	DESTINATION include/casacode/casadbus/session
	)
install (FILES
	implement/types/functor.h
	implement/types/nullptr.h
	implement/types/ptr.h
	implement/types/record.h
	implement/types/variant.h
	DESTINATION include/casacode/casadbus/types
	)
install (FILES
	implement/utilities/BusAccess.h
	implement/utilities/Conversion.h
	DESTINATION include/casacode/casadbus/utilities
	)
install (FILES
	implement/viewer/ViewerProxy.h
	DESTINATION include/casacode/casadbus/viewer
	)
