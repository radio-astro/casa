add_definitions( ${dish_DEFINITIONS} )
include_directories( ${dish_INCLUDE_DIRS} )

# Dynamic library
add_library( dish
  implement/SDIterators/SDFITSDataIterator.cc
  implement/SDIterators/SDIterator.cc
  implement/SDIterators/SDIterAxes.cc
  implement/SDIterators/SDIterFieldCopier.cc
  implement/SDIterators/SDMSIterator.cc
  implement/SDIterators/SDRecord.cc
  implement/SDIterators/SDTableIterator.cc
  implement/SDIterators/SDTabRecCopier.cc
 )

target_link_libraries( dish ${dish_LINK_TO} )

set_target_properties( dish PROPERTIES SOVERSION ${CASA_API_VERSION} )

install( TARGETS dish LIBRARY DESTINATION lib )

# Executables
# Unit tests
add_executable( tSDMSIterator EXCLUDE_FROM_ALL implement/SDIterators/test/tSDMSIterator.cc )
target_link_libraries( tSDMSIterator dish )
add_test( tSDMSIterator ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tSDMSIterator )
add_dependencies( check tSDMSIterator )

add_executable( tSDRecord EXCLUDE_FROM_ALL implement/SDIterators/test/tSDRecord.cc )
target_link_libraries( tSDRecord dish )
add_test( tSDRecord ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tSDRecord )
add_dependencies( check tSDRecord )

add_executable( tSDTableIterator EXCLUDE_FROM_ALL implement/SDIterators/test/tSDTableIterator.cc )
target_link_libraries( tSDTableIterator dish )
add_test( tSDTableIterator ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tSDTableIterator )
add_dependencies( check tSDTableIterator )

