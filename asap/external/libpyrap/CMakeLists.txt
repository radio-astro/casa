###
# CMakeLists.txt for libpyrap.so
###

# root for libpyrap
set( PYRAPROOT ${ASAP_SOURCE_DIR}/external/libpyrap/pyrap-0.3.2 )
set( PYRAPDIR ${PYRAPROOT}/pyrap )
set( MODULE Converters )

# include path
include_directories( ${PYRAPROOT} )

# link path
link_directories( ${PYRAPROOT} )

# use numpy
add_definitions( -DAIPS_USENUMPY )

# source files for libpyrap
set( PYRAP_SRCS
     ${PYRAPDIR}/${MODULE}/PycArray.cc
     ${PYRAPDIR}/${MODULE}/PycArrayNP.cc
     ${PYRAPDIR}/${MODULE}/PycBasicData.cc
     ${PYRAPDIR}/${MODULE}/PycExcp.cc
     ${PYRAPDIR}/${MODULE}/PycRecord.cc
     ${PYRAPDIR}/${MODULE}/PycValueHolder.cc )

add_library( ${PYRAPLIB} SHARED ${PYRAP_SRCS} )
target_link_libraries( ${PYRAPLIB} 
                       ${Boost_LIBRARIES} 
                       ${PYTHON_LIBRARIES} 
                       ${CASACORE_LIBRARIES} 
                       ${DL_LIBRARIES} 
                       ${BLAS_LIBRARIES} 
                       ${LAPACK_LIBRARIES} )
add_dependencies( inst ${PYRAPLIB} )

if ( DEFINED asap_soversion )
    set_target_properties( ${PYRAPLIB} PROPERTIES SOVERSION ${asap_soversion} )
endif( )

# install directory
install( TARGETS ${PYRAPLIB}
         LIBRARY DESTINATION ${LIB_INSTALL_DIR} )
