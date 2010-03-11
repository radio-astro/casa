add_definitions( ${tableplot_DEFINITIONS} )
include_directories( ${tableplot_INCLUDE_DIRS} )

# Dynamic library
add_library( tableplot
  implement/TablePlot/BasePlot.cc
  implement/TablePlot/CasaPyInterpreter.cc
  implement/TablePlot/CrossPlot.cc
  implement/TablePlot/FlagVersion.cc
  implement/TablePlot/PanelParams.cc
  implement/TablePlot/PlotterGlobals.cc
  implement/TablePlot/SLog.cc
  implement/TablePlot/TablePlot.cc
  implement/TablePlot/TPPlotter.cc
 )

target_link_libraries( tableplot ${tableplot_LINK_TO} )

set_target_properties( tableplot PROPERTIES SOVERSION ${CASA_API_VERSION} )

install( TARGETS tableplot LIBRARY DESTINATION lib )

# Executables
# Unit tests
