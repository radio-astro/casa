###
# CMakeLists.txt for _asap.so
###

# library name is _asap.so
set( ASAPLIB _asap.so )

# root for libatnf
set( ASAPROOT ${ASAP_SOURCE_DIR} )
set( SRCDIR ${ASAPROOT}/src )

# include path
include_directories( ${SRCDIR} )
include_directories( ${ASAPROOT}/external-alma )

# link path
link_directories( ${SRCDIR} )

# source files for libpyrap
set( ASAP_SRCS
     ${SRCDIR}/MathUtils.cpp
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
     ${SRCDIR}/python_LogSink.cpp
     ${SRCDIR}/python_asap.cpp )

add_library( ${ASAPLIB} SHARED ${ASAP_SRCS} ${ASAP_PYSRCS} )
set_target_properties( ${ASAPLIB}
                       PROPERTIES
                       PREFIX ""
                       SUFFIX "" )
target_link_libraries( ${ASAPLIB} pyrap atnf ${CASACORE_LIBRARIES} )

# install directory
install( TARGETS ${ASAPLIB}
         LIBRARY DESTINATION ${PYTHON_INSTALL_DIR} )
