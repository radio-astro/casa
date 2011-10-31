###
# CMakeLists.txt for _asap.so
###

# root for libatnf
set( ASAPROOT ${ASAP_SOURCE_DIR} )
set( SRCDIR ${ASAPROOT}/src )

# include path
include_directories( ${SRCDIR} 
                     ${Boost_INCLUDE_DIR}
                     ${ASAPROOT}/external-alma 
                     ${ASAPROOT}/external/libpyrap/pyrap-0.3.2 )

# link path
link_directories( ${SRCDIR} )

# use libpyrap
add_definitions( -DHAVE_LIBPYRAP )

# source files for libpyrap
set( ASAP_SRCS
     ${SRCDIR}/MathUtils.cpp
     ${SRCDIR}/TableTraverse.cpp
     ${SRCDIR}/RowAccumulator.cpp
     ${SRCDIR}/Logger.cpp
     ${SRCDIR}/LineCatalog.cpp
     ${SRCDIR}/STAttr.cpp
     ${SRCDIR}/STHeader.cpp 
     ${SRCDIR}/STFiller.cpp
     ${SRCDIR}/STMath.cpp
     ${SRCDIR}/STSubTable.cpp
     ${SRCDIR}/STMolecules.cpp
     ${SRCDIR}/STTcal.cpp
     ${SRCDIR}/STFocus.cpp
     ${SRCDIR}/STWeather.cpp
     ${SRCDIR}/STFrequencies.cpp
     ${SRCDIR}/STHistory.cpp
     ${SRCDIR}/STSelector.cpp
     ${SRCDIR}/STLineFinder.cpp
     ${SRCDIR}/STFitter.cpp
     ${SRCDIR}/STFitEntry.cpp
     ${SRCDIR}/STFit.cpp
     ${SRCDIR}/STPol.cpp
     ${SRCDIR}/STPolLinear.cpp
     ${SRCDIR}/STPolCircular.cpp
     ${SRCDIR}/STPolStokes.cpp
     ${SRCDIR}/STWriter.cpp
     ${SRCDIR}/STAsciiWriter.cpp
     ${SRCDIR}/STFITSImageWriter.cpp
     ${SRCDIR}/STAtmosphere.cpp
     ${SRCDIR}/Scantable.cpp
     ${SRCDIR}/FillerBase.cpp
     ${SRCDIR}/NROFiller.cpp
     ${SRCDIR}/PKSFiller.cpp
     ${SRCDIR}/MSFiller.cpp
     ${SRCDIR}/MSWriter.cpp
     ${SRCDIR}/AsapLogSink.cpp
     ${SRCDIR}/Templates.cpp )

set( ASAP_PYSRCS
     ${SRCDIR}/python_Scantable.cpp
     ${SRCDIR}/python_STFiller.cpp
     ${SRCDIR}/python_STSelector.cpp
     ${SRCDIR}/python_STMath.cpp
     ${SRCDIR}/python_Fitter.cpp
     ${SRCDIR}/python_Logger.cpp
     ${SRCDIR}/python_STLineFinder.cpp
     ${SRCDIR}/python_STFitEntry.cpp
     ${SRCDIR}/python_STWriter.cpp
     ${SRCDIR}/python_LineCatalog.cpp
     ${SRCDIR}/python_SrcType.cpp
     ${SRCDIR}/python_STAtmosphere.cpp
     ${SRCDIR}/python_STCoordinate.cpp
     ${SRCDIR}/python_Filler.cpp
     ${SRCDIR}/python_MSFiller.cpp
     ${SRCDIR}/python_MSWriter.cpp
     ${SRCDIR}/python_LogSink.cpp
     ${SRCDIR}/python_asap.cpp )

add_library( ${ASAPLIB} SHARED ${ASAP_SRCS} ${ASAP_PYSRCS} )

# library name must be _asap.${CMAKE_SHARED_LIBRARY_SUFFIX}
set_target_properties( ${ASAPLIB}
                       PROPERTIES
                       PREFIX _ )

target_link_libraries( ${ASAPLIB} 
                       #${Boost_LIBRARIES}
                       ${PYRAPLIB} 
                       ${ATNFLIB} 
                       #${CASACORE_LIBRARIES} 
                       ${WCSLIB} )

add_dependencies( inst ${ASAPLIB} )

# Install ASAPLIB with platform default extension in lib directory.
install( TARGETS ${ASAPLIB}
         LIBRARY DESTINATION ${CMAKE_INSTALL_NAME_DIR} )

#
# Create a symbolic link from LIB_INSTALL_DIR/ASAPLIB. to PYTHON_INSTALL_DIR/ASAPLIB.so
# to work around Python issue loading dylib files on OS X.  On Linux, it appears to be 
# sufficient to copy _asap.so to the Python directory, but handling Linux and OS X the
# same way reduces confusion and does not create any issues.

set ( FULL_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR} ) 
set ( LINK_SOURCE ${FULL_LIB_INSTALL_DIR}/_${ASAPLIB}${CMAKE_SHARED_LIBRARY_SUFFIX} )

set ( FULL_PYTHON_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${PYTHON_INSTALL_DIR} ) 
set ( LINK_TARGET ${FULL_PYTHON_INSTALL_DIR}/_${ASAPLIB}.so ) 

install ( CODE "
    EXECUTE_PROCESS ( COMMAND 
        ln -s ${LINK_SOURCE} ${LINK_TARGET}
    )
" )
