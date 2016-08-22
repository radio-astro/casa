###
# CMakeLists.txt for asdm2asap
###

# additional include directories
set( ASAPROOT ${ASAP_SOURCE_DIR} )
include_directories( ${ASDM_INCLUDE_DIR} 
                     ${ASAP_SOURCE_DIR}/src 
                     ${ASAPROOT}/external-alma )

# source code
set( EXEC_SRC ASDMReader.cc
              ASDMFiller.cc
              asdm2ASAP.cc )

# add executable
add_executable( ${ASDM2ASAP} ${EXEC_SRC} )

# dependencies
target_link_libraries( ${ASDM2ASAP} ${Boost_LIBRARIES} ${ASDM_LIBRARY}
                                    ${ASAPLIB} )
add_dependencies( inst ${ASDM2ASAP} )

# install
install( TARGETS ${ASDM2ASAP}
         RUNTIME DESTINATION ${EXEC_INSTALL_DIR} )
