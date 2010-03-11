add_definitions( ${msvis_DEFINITIONS} )
include_directories( ${msvis_INCLUDE_DIRS} )

# Dynamic library
add_library( msvis
  implement/MSVis/CalVisBuffer.cc
  implement/MSVis/MsAverager.cc
  implement/MSVis/MSCalEnums.cc
  implement/MSVis/MSContinuumSubtractor.cc
  implement/MSVis/MSUVWGenerator.cc
  implement/MSVis/SelectAverageSpw.cc
  implement/MSVis/StokesVector.cc
  implement/MSVis/SubMS.cc
  implement/MSVis/VisBuffAccumulator.cc
  implement/MSVis/VisBuffer.cc
  implement/MSVis/VisBufferUtil.cc
  implement/MSVis/VisBuffGroupAcc.cc
  implement/MSVis/VisibilityIterator.cc
  implement/MSVis/VisImagingWeight.cc
  implement/MSVis/VisIterator.cc
  implement/MSVis/VisSet.cc
  implement/MSVis/VisSetUtil.cc
  implement/MSVis/VisTimeAverager.cc
 )

target_link_libraries( msvis ${msvis_LINK_TO} )

set_target_properties( msvis PROPERTIES SOVERSION ${CASA_API_VERSION} )

install( TARGETS msvis LIBRARY DESTINATION lib )

# Executables
add_executable( casasplit
apps/casasplit/casasplit.cc)
target_link_libraries( casasplit msvis )

install( TARGETS casasplit RUNTIME DESTINATION bin )

if( APPLE )
  install( TARGETS casasplit RUNTIME DESTINATION apps/casasplit.app/Contents/MacOS )
endif()
# Unit tests
add_executable( tHanningSmooth EXCLUDE_FROM_ALL implement/MSVis/test/tHanningSmooth.cc )
target_link_libraries( tHanningSmooth msvis )
add_test( tHanningSmooth ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tHanningSmooth )
add_dependencies( check tHanningSmooth )

add_executable( tMSCalEnums EXCLUDE_FROM_ALL implement/MSVis/test/tMSCalEnums.cc )
target_link_libraries( tMSCalEnums msvis )
add_test( tMSCalEnums ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tMSCalEnums )
add_dependencies( check tMSCalEnums )

add_executable( tUVSub EXCLUDE_FROM_ALL implement/MSVis/test/tUVSub.cc )
target_link_libraries( tUVSub msvis )
add_test( tUVSub ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tUVSub )
add_dependencies( check tUVSub )

add_executable( tVisibilityIterator EXCLUDE_FROM_ALL implement/MSVis/test/tVisibilityIterator.cc )
target_link_libraries( tVisibilityIterator msvis )
add_test( tVisibilityIterator ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tVisibilityIterator )
add_dependencies( check tVisibilityIterator )

add_executable( tVisIter EXCLUDE_FROM_ALL implement/MSVis/test/tVisIter.cc )
target_link_libraries( tVisIter msvis )
add_test( tVisIter ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tVisIter )
add_dependencies( check tVisIter )

