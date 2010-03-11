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

