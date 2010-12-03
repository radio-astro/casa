###
# CMakeLists.txt for asap2to3
###

# executable name is asap2to3
set( ASAP2TO3 asap2to3 )

# source code
set( EXEC_SRC asap2to3.cpp )

# add executable
add_executable( ${ASAP2TO3} ${EXEC_SRC} )

# dependencies
target_link_libraries( ${ASAP2TO3} ${CASACORE_LIBRARIES} )

# install
install( PROGRAMS ${ASAP2TO3} 
         DESTINATION ${EXEC_INSTALL_DIR} )