###
# CMakeLists.txt for asdm2asap
###

# additional include directories
set( ASAPROOT ${ASAP_SOURCE_DIR} )
include_directories( ${ASDM_INCLUDE_DIR_OLD} 
                     ${ASAP_SOURCE_DIR}/src 
                     ${ASAPROOT}/external-alma )

# source code
set( EXEC_SRC OldASDMReader.cc
              OldASDMFiller.cc
              oldasdm2ASAP.cc )

# add executable
add_executable( ${ASDM2ASAP_OLD} ${EXEC_SRC} )

# dependencies
target_link_libraries( ${ASDM2ASAP_OLD} ${ASDM_LIBRARY_OLD}
                                        ${ASAPLIB} )
add_dependencies( inst ${ASDM2ASAP_OLD} )

# install
install( TARGETS ${ASDM2ASAP_OLD}
         RUNTIME DESTINATION ${EXEC_INSTALL_DIR} )