
# Dynamic library
casa_add_library( flagging
  implement/Flagging/DDMapper.cc
  implement/Flagging/ExtendFlagger.cc
  implement/Flagging/Flagger.cc
  implement/Flagging/FlagIndex.cc
  implement/Flagging/RedFlagger.cc
  implement/Flagging/RFAApplyFlags.cc
  implement/Flagging/RFADiffBase.cc
  implement/Flagging/RFAFlagCubeBase.cc
  implement/Flagging/RFAMedianClip.cc
  implement/Flagging/RFANewMedianClip.cc
  implement/Flagging/RFASelector2.cc
  implement/Flagging/RFASpectralRej.cc
  implement/Flagging/RFAUVBinner.cc
  implement/Flagging/RFChunkStats.cc
  implement/Flagging/RFDataMapper.cc
  implement/Flagging/RFDebugPlot.cc
  implement/Flagging/RFFlagCube.cc
  implement/Flagging/RFRowClipper.cc
  implement/MSPlot/MsPlot.cc
 )

# Executables
# Unit tests
add_executable( tAutoflag EXCLUDE_FROM_ALL implement/Flagging/test/tAutoflag.cc )
target_link_libraries( tAutoflag flagging )
add_test( tAutoflag ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tAutoflag )
add_dependencies( check tAutoflag )

add_executable( tRFASelector EXCLUDE_FROM_ALL implement/Flagging/test/tRFASelector.cc )
target_link_libraries( tRFASelector flagging )
add_test( tRFASelector ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tRFASelector )
add_dependencies( check tRFASelector )

