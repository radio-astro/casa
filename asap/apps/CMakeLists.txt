###
# CMakeLists.txt for asap2to3
###

# source code
set( EXEC_SRC asap2to3.cpp )

# add executable
add_executable( ${ASAP2TO3} ${EXEC_SRC} )

# dependencies
target_link_libraries( ${ASAP2TO3} 
                       ${CASACORE_LIBRARIES} 
                       ${DL_LIBRARIES} 
                       ${BLAS_LIBRARIES}
                       ${LAPACK_LIBRARIES} )
add_dependencies( inst ${ASAP2TO3} )

# install
install( TARGETS ${ASAP2TO3}
         RUNTIME DESTINATION ${EXEC_INSTALL_DIR} )