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
     ${SRCDIR}/STSideBandSep.cpp
     ${SRCDIR}/STApplyTable.cpp
     ${SRCDIR}/STCalTsysTable.cpp
     ${SRCDIR}/STCalSkyTable.cpp
     ${SRCDIR}/STCalibration.cpp
     ${SRCDIR}/STCalSkyPSAlma.cpp
     ${SRCDIR}/STCalSkyOtfAlma.cpp
     ${SRCDIR}/STCalTsys.cpp
     ${SRCDIR}/STApplyCal.cpp
     ${SRCDIR}/Calibrator.cpp
     ${SRCDIR}/PSAlmaCalibrator.cpp
     ${SRCDIR}/STBaselineTable.cpp
     ${SRCDIR}/CalibrationManager.cpp )

if ( NOT STANDALONE )
set( ASAP_SRCS
     ${ASAP_SRCS}
     ${SRCDIR}/Plotter2.cpp )
endif()

set( ASAP_PYSRCS
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
     ${SRCDIR}/python_STSideBandSep.cpp
     ${SRCDIR}/python_CalibrationManager.cpp
     ${SRCDIR}/python_asap.cpp )

if ( NOT STANDALONE )
set( ASAP_PYSRCS
     ${ASAP_PYSRCS}
     ${SRCDIR}/python_Plotter2.cpp )
endif()

add_library( ${ASAPLIB} SHARED ${ASAP_SRCS} )
add_library( ${ASAPPYLIB} SHARED ${ASAP_PYSRCS} )

if ( DEFINED asap_soversion )
    set_target_properties( ${ASAPLIB} PROPERTIES SOVERSION ${asap_soversion} )
    set_target_properties( ${ASAPPYLIB} PROPERTIES SOVERSION ${asap_soversion} )
endif( )

# library name must be _asap.so
set_target_properties( ${ASAPPYLIB}
                       PROPERTIES
                       PREFIX ""
                       SUFFIX .so )

target_link_libraries( ${ASAPLIB} 
                       #${Boost_LIBRARIES}
                       ${X11_LIBRARIES} 
                       ${PGPLOT_LIBRARIES} 
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
