add_definitions( ${graphics_DEFINITIONS} )
include_directories( ${graphics_INCLUDE_DIRS} )

# Dynamic library
add_library( graphics
  implement/GenericPlotter/PlotCanvas.cc
  implement/GenericPlotter/PlotCanvasLayout.cc
  implement/GenericPlotter/PlotData.cc
  implement/GenericPlotter/PlotEvent.cc
  implement/GenericPlotter/PlotFactory.cc
  implement/GenericPlotter/PlotItem.cc
  implement/GenericPlotter/PlotLogger.cc
  implement/GenericPlotter/PlotOperation.cc
  implement/GenericPlotter/PlotOptions.cc
  implement/GenericPlotter/Plotter.cc
  implement/GenericPlotter/PlotTool.cc
  implement/GenericPlotter/SimplePlotter.cc
  implement/Graphics/PGPlotterLocal.cc
 )

target_link_libraries( graphics ${graphics_LINK_TO} )

set_target_properties( graphics PROPERTIES SOVERSION ${CASA_API_VERSION} )

install( TARGETS graphics LIBRARY DESTINATION lib )

# Executables
# Unit tests
