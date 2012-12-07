###
# CMakeLists.txt for _asap.so
###

# root for libatnf
set( ASAPROOT ${ASAP_SOURCE_DIR} )
set( SRCDIR ${ASAPROOT}/src )

# include QT_USE_FILE
include( ${QT_USE_FILE} )

# include path
include_directories( ${SRCDIR} 
                     ${Boost_INCLUDE_DIR}
                     ${QT4_INCLUDE_DIRS}
                     ${ASAPROOT}/external-alma 
                     ${ASAPROOT}/external/libpyrap/pyrap-0.3.2 )

# link path
link_directories( ${SRCDIR} )

# use libpyrap
add_definitions( -DHAVE_LIBPYRAP )

# source files for libpyrap
set( ASAP_SRCS
#     ${SRCDIR}/Plotter2.cpp
     ${SRCDIR}/concurrent.cpp
     ${SRCDIR}/MathUtils.cpp
     ${SRCDIR}/TableTraverse.cpp
     ${SRCDIR}/RowAccumulator.cpp
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
     ${SRCDIR}/STUpgrade.cpp 
     ${SRCDIR}/STGrid.cpp
     ${SRCDIR}/STIdxIter.cpp
     ${SRCDIR}/EdgeMarker.cpp
     ${SRCDIR}/EdgeDetector.cpp
     ${SRCDIR}/RasterEdgeDetector.cpp
     ${SRCDIR}/GenericEdgeDetector.cpp
     ${SRCDIR}/PlotHelper.cpp
     ${SRCDIR}/Templates.cpp )

set( ASAP_PYSRCS
#     ${SRCDIR}/python_Plotter2.cpp
     ${SRCDIR}/python_Scantable.cpp
     ${SRCDIR}/python_STFiller.cpp
     ${SRCDIR}/python_STSelector.cpp
     ${SRCDIR}/python_STMath.cpp
     ${SRCDIR}/python_Fitter.cpp
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
     ${SRCDIR}/python_STGrid.cpp
     ${SRCDIR}/python_Iterator.cpp
     ${SRCDIR}/python_EdgeMarker.cpp
     ${SRCDIR}/python_PlotHelper.cpp
     ${SRCDIR}/python_asap.cpp )

add_library( ${ASAPLIB} SHARED ${ASAP_SRCS} )
add_library( ${ASAPPYLIB} SHARED ${ASAP_PYSRCS} )

# library name must be _asap.so
set_target_properties( ${ASAPPYLIB}
                       PROPERTIES
                       PREFIX ""
                       SUFFIX .so )

target_link_libraries( ${ASAPLIB} 
                       #${Boost_LIBRARIES}
                       ${QT4_LIBRARIES} 
                       ${PYRAPLIB} 
                       ${ATNFLIB} 
                       #${CASACORE_LIBRARIES} 
                       ${WCSLIB} )
target_link_libraries( ${ASAPPYLIB} 
                       ${ASAPLIB} )

add_dependencies( inst ${ASAPLIB} 
                       ${ASAPPYLIB} )

# install directory
install( TARGETS ${ASAPLIB}
         LIBRARY DESTINATION ${LIB_INSTALL_DIR} )
install( TARGETS ${ASAPPYLIB}
         LIBRARY DESTINATION ${PYTHON_INSTALL_DIR} )
#if( NOT STANDALONE )
#  execute_process(
#    COMMAND ln -Fs ../${PYTHON_INSTALL_DIR}/_${ASAPLIB}.so ${CMAKE_INSTALL_PREFIX}/lib/_${ASAPLIB}.so
#    OUTPUT_VARIABLE stuff)
#endif()
