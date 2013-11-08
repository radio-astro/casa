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

casa_add_dbus_proxy( plotserver/PlotServerProxy.proxy.h xml/PlotServerProxy.interface.xml )
casa_add_dbus_proxy( session/DBusSession.proxy.h xml/DBusSession.interface.xml )
casa_add_dbus_proxy( viewer/ViewerProxy.proxy.h xml/ViewerProxy.interface.xml )
#casa_add_dbus_adaptor( viewer/ViewerAdaptor.adaptor.h xml/ViewerProxy.interface.xml )

###
### who know what is the right way to do this with C-MINUS-MAKE...
###
add_custom_target(install-dbus-session ALL
    COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/../etc/dbus/session.d
    COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/etc/casa.conf ${CMAKE_INSTALL_PREFIX}/../etc/dbus
)

if (INTERACTIVE_ITERATION)
   casa_add_dbus_proxy( interfaces/SynthImager.proxy.h xml/SynthImager.interface.xml )
   casa_add_dbus_adaptor( interfaces/SynthImager.adaptor.h xml/SynthImager.interface.xml )
    install( FILES 
             ${CMAKE_CURRENT_BINARY_DIR}/interfaces/SynthImager.proxy.h
	${CMAKE_CURRENT_BINARY_DIR}/interfaces/SynthImager.adaptor.h
	DESTINATION include/casacode/casadbus/interfaces
	)


else()
   message( WARNING "Not Building Support for Interactive control of imaging")
endif()

install( FILES 
	${CMAKE_CURRENT_BINARY_DIR}/plotserver/PlotServerProxy.proxy.h
	DESTINATION include/casacode/casadbus/plotserver
	)
install( FILES 
	${CMAKE_CURRENT_BINARY_DIR}/viewer/ViewerProxy.proxy.h
	DESTINATION include/casacode/casadbus/viewer
	)
install( FILES 
	${CMAKE_CURRENT_BINARY_DIR}/session/DBusSession.proxy.h
	DESTINATION include/casacode/casadbus/session
	)

if (INTERACTIVE_ITERATION)
casa_add_library( casadbus
  plotserver/PlotServerProxy.cc
  session/DBusSession.cc
  session/Dispatcher.cc
  types/record.cc
  types/variant.cc
  types/nullptr.cc
  utilities/BusAccess.cc
  utilities/Conversion.cc
  utilities/Diagnostic.cc
  utilities/DBusBase.cc
  utilities/io.cc
  viewer/ViewerProxy.cc
  synthesis/ImagerControl.cc
  interfaces/SynthImager.proxy.h
  interfaces/SynthImager.adaptor.h
  plotserver/PlotServerProxy.proxy.h
  session/DBusSession.proxy.h
  viewer/ViewerProxy.proxy.h  
)
else()
casa_add_library( casadbus
  plotserver/PlotServerProxy.cc
  session/DBusSession.cc
  session/Dispatcher.cc
  types/record.cc
  types/variant.cc
  types/nullptr.cc
  utilities/BusAccess.cc
  utilities/Conversion.cc
  utilities/Diagnostic.cc
  utilities/DBusBase.cc
  synthesis/ImagerControl.cc
  utilities/io.cc
  viewer/ViewerProxy.cc
  plotserver/PlotServerProxy.proxy.h
  session/DBusSession.proxy.h
  viewer/ViewerProxy.proxy.h  
)
endif()
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
	synthesis/ImagerControl.h
	DESTINATION include/casacode/casadbus/synthesis
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
	utilities/Diagnostic.h
        utilities/DBusBase.h
	DESTINATION include/casacode/casadbus/utilities
	)
install (FILES
	viewer/ViewerProxy.h
	DESTINATION include/casacode/casadbus/viewer
	)

casa_add_assay( casadbus session/test/dDBusSession.cc )
