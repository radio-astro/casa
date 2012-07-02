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

casa_add_library( casadbus
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
viewer/ViewerProxy.dbusproxy.h  )

