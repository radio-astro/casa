###
# CMakeLists.txt for shared files
###

# shared files
set( SHARED_FILES
     ipythonrc-asap )

install( FILES ${SHARED_FILES} 
         DESTINATION ${SHARED_INSTALL_DIR} )
