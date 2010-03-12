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

add_definitions( ${casadbus_DEFINITIONS} )
include_directories( ${casadbus_INCLUDE_DIRS} )

# Dynamic library
add_library( casadbus
  implement/plotserver/PlotServerProxy.cc
  implement/session/DBusSession.cc
  implement/session/Dispatcher.cc
  implement/types/record.cc
  implement/types/variant.cc
  implement/utilities/BusAccess.cc
  implement/utilities/Conversion.cc
  implement/viewer/ViewerProxy.cc
 
plotserver/PlotServerProxy.dbusproxy.h
session/DBusSession.dbusproxy.h
viewer/ViewerProxy.dbusproxy.h)

target_link_libraries( casadbus ${casadbus_LINK_TO} )

set_target_properties( casadbus PROPERTIES SOVERSION ${CASA_API_VERSION} )

install( TARGETS casadbus LIBRARY DESTINATION lib )

# Executables
# Unit tests
#
# XML tools
#

casa_tool( tool_idl tool_sources 
  implement/plotserver/PlotServerProxy.dbusproxy.xml
  implement/session/DBusSession.dbusproxy.xml
  implement/viewer/ViewerProxy.dbusproxy.xml )

