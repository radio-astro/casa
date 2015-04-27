###
# CMakeLists.txt for executables
###

# executables
set( ASAP_EXEC
     asap
     asap_update_data
     install )

install( PROGRAMS ${ASAP_EXEC} 
         DESTINATION ${EXEC_INSTALL_DIR} )
