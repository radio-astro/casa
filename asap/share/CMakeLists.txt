###
# CMakeLists.txt for shared files
###

# root for shared
set( SHAREDROOT ${ASAP_SOURCE_DIR} )
set( SHAREDDIR ${PYTHONROOT}/share )

# python modules 
set( SHARED_FILES
     ipythonrc-asap )

install( FILES ${SHARED_FILES} 
         DESTINATION ${SHARED_INSTALL_DIR} )
