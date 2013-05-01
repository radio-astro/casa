###
# CMakeLists.txt for libatnf.so
###

# root for libatnf
set( ATNFROOT ${ASAP_SOURCE_DIR}/external/ )
set( ATNFDIR ${ATNFROOT}/atnf )
set( PKS pks )
set( PKSIO PKSIO )

# include path
include_directories( ${ATNFROOT} )

# link path
link_directories( ${ATNFROOT} )

# source files for libpyrap
set( ATNF_SRCS
     ${ATNFDIR}/${PKS}/pks_maths.cc
     ${ATNFDIR}/${PKSIO}/PKSmsg.cc
     ${ATNFDIR}/${PKSIO}/FITSreader.cc
     ${ATNFDIR}/${PKSIO}/MBFITSreader.cc
     ${ATNFDIR}/${PKSIO}/MBrecord.cc
     ${ATNFDIR}/${PKSIO}/PKSFITSreader.cc
     ${ATNFDIR}/${PKSIO}/PKSMS2reader.cc
     ${ATNFDIR}/${PKSIO}/PKSMS2writer.cc
     ${ATNFDIR}/${PKSIO}/PKSreader.cc
     ${ATNFDIR}/${PKSIO}/PKSSDwriter.cc
     ${ATNFDIR}/${PKSIO}/SDFITSreader.cc
     ${ATNFDIR}/${PKSIO}/SDFITSwriter.cc )
add_library( ${ATNFLIB} SHARED ${ATNF_SRCS} )
target_link_libraries( ${ATNFLIB} 
                       ${CASACORE_LIBRARIES} 
                       ${CFITSIO_LIBRARIES} 
                       ${RPFITS_LIBRARIES} 
                       ${DL_LIBRARIES} 
                       ${BLAS_LIBRARIES} 
                       ${LAPACK_LIBRARIES} )
add_dependencies( inst ${ATNFLIB} )

# install directory
install( TARGETS ${ATNFLIB}
         LIBRARY DESTINATION ${LIB_INSTALL_DIR} )
