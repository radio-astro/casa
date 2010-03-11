add_definitions( ${calibration_DEFINITIONS} )
include_directories( ${calibration_INCLUDE_DIRS} )

# Dynamic library
add_library( calibration
  implement/CalTables/BJonesDesc.cc
  implement/CalTables/BJonesMBuf.cc
  implement/CalTables/BJonesMCol.cc
  implement/CalTables/BJonesMRec.cc
  implement/CalTables/BJonesTable.cc
  implement/CalTables/CalBuffer.cc
  implement/CalTables/CalDescBuffer.cc
  implement/CalTables/CalDescColumns2.cc
  implement/CalTables/CalDescColumns.cc
  implement/CalTables/CalDescRecord.cc
  implement/CalTables/CalHistoryBuffer.cc
  implement/CalTables/CalHistoryColumns.cc
  implement/CalTables/CalHistRecord.cc
  implement/CalTables/CalInterp.cc
  implement/CalTables/CalInterpolation.cc
  implement/CalTables/CalInterpolator.cc
  implement/CalTables/CalIntpMatch.cc
  implement/CalTables/CalIter.cc
  implement/CalTables/CalMainBuffer.cc
  implement/CalTables/CalMainColumns.cc
  implement/CalTables/CalMainRecord.cc
  implement/CalTables/CalSet2.cc
  implement/CalTables/CalSetMetaInfo.cc
  implement/CalTables/CalTable2.cc
  implement/CalTables/CalTable.cc
  implement/CalTables/CalTableDesc2.cc
  implement/CalTables/CalTableDesc.cc
  implement/CalTables/EPointDesc.cc
  implement/CalTables/EPointMBuf.cc
  implement/CalTables/EPointMCol.cc
  implement/CalTables/EPointTable.cc
  implement/CalTables/EPointVJMRec.cc
  implement/CalTables/FVisJonesDesc.cc
  implement/CalTables/FVisJonesMRec.cc
  implement/CalTables/GJonesDesc.cc
  implement/CalTables/GJonesMBuf.cc
  implement/CalTables/GJonesMCol.cc
  implement/CalTables/GJonesTable.cc
  implement/CalTables/MJonesDesc.cc
  implement/CalTables/MJonesMRec.cc
  implement/CalTables/PlotCal.cc
  implement/CalTables/SolvableMJDesc.cc
  implement/CalTables/SolvableMJMRec.cc
  implement/CalTables/SolvableVJDesc.cc
  implement/CalTables/SolvableVJMBuf.cc
  implement/CalTables/SolvableVJMCol.cc
  implement/CalTables/SolvableVJMRec.cc
  implement/CalTables/SolvableVJTable.cc
  implement/CalTables/TimeVarMJDesc.cc
  implement/CalTables/TimeVarMJMRec.cc
  implement/CalTables/TimeVarVJDesc.cc
  implement/CalTables/TimeVarVJMBuf.cc
  implement/CalTables/TimeVarVJMCol.cc
  implement/CalTables/TimeVarVJMRec.cc
  implement/CalTables/TimeVarVJTable.cc
  implement/CalTables/TJonesDesc.cc
  implement/CalTables/TJonesMBuf.cc
  implement/CalTables/TJonesMCol.cc
  implement/CalTables/TJonesTable.cc
  implement/CalTables/VisJonesDesc.cc
  implement/CalTables/VisJonesMBuf.cc
  implement/CalTables/VisJonesMCol.cc
  implement/CalTables/VisJonesMRec.cc
  implement/CalTables/VisJonesTable.cc
 )

target_link_libraries( calibration ${calibration_LINK_TO} )

set_target_properties( calibration PROPERTIES SOVERSION ${CASA_API_VERSION} )

install( TARGETS calibration LIBRARY DESTINATION lib )

# Executables
# Unit tests
add_executable( tCalInterpolation EXCLUDE_FROM_ALL implement/CalTables/test/tCalInterpolation.cc )
target_link_libraries( tCalInterpolation calibration )
add_test( tCalInterpolation ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tCalInterpolation )
add_dependencies( check tCalInterpolation )

add_executable( tCalIntpMatch EXCLUDE_FROM_ALL implement/CalTables/test/tCalIntpMatch.cc )
target_link_libraries( tCalIntpMatch calibration )
add_test( tCalIntpMatch ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tCalIntpMatch )
add_dependencies( check tCalIntpMatch )

