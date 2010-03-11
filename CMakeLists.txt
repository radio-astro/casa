add_definitions( ${synthesis_DEFINITIONS} )
include_directories( ${synthesis_INCLUDE_DIRS} )

# Dynamic library
add_library( synthesis
  implement/DataSampling/DataSampling.cc
  implement/DataSampling/ImageDataSampling.cc
  implement/DataSampling/PixonProcessor.cc
  implement/DataSampling/SDDataSampling.cc
  implement/DataSampling/SynDataSampling.cc
  implement/IDL/IDL.cc
  implement/MeasurementComponents/AMueller.cc
  implement/MeasurementComponents/BeamCalc.cc
  implement/MeasurementComponents/BeamCalcParams.cc
  implement/MeasurementComponents/BeamSkyJones.cc
  implement/MeasurementComponents/BeamSquint.cc
  implement/MeasurementComponents/BPoly.cc
  implement/MeasurementComponents/CalCorruptor.cc
  implement/MeasurementComponents/Calibrater.cc
  implement/MeasurementComponents/CEMemImageSkyModel.cc
  implement/MeasurementComponents/ClarkCleanAlgorithm.cc
  implement/MeasurementComponents/ClarkCleanImageSkyModel.cc
  implement/MeasurementComponents/CleanImageSkyModel.cc
  implement/MeasurementComponents/ComponentFTMachine.cc
  implement/MeasurementComponents/ConvFuncDiskCache.cc
  implement/MeasurementComponents/CSCleanImageSkyModel.cc
  implement/MeasurementComponents/DBeamSkyJones.cc
  implement/MeasurementComponents/EJones.cc
  implement/MeasurementComponents/EPJones.cc
  implement/MeasurementComponents/FTMachine.cc
  implement/MeasurementComponents/GlobalFTMachineCallbacks.cc
  implement/MeasurementComponents/GridBoth.cc
  implement/MeasurementComponents/GridFT.cc
  implement/MeasurementComponents/GSpline.cc
  implement/MeasurementComponents/HetArrayConvFunc.cc
  implement/MeasurementComponents/HogbomCleanImageSkyModel.cc
  implement/MeasurementComponents/IlluminationConvFunc.cc
  implement/MeasurementComponents/ImageSkyModel.cc
  implement/MeasurementComponents/Jones.cc
  implement/MeasurementComponents/LJJones.cc
  implement/MeasurementComponents/MakeApproxPSFAlgorithm.cc
  implement/MeasurementComponents/MFCEMemImageSkyModel.cc
  implement/MeasurementComponents/MFCleanImageSkyModel.cc
  implement/MeasurementComponents/MFMSCleanImageSkyModel.cc
  implement/MeasurementComponents/MosaicFT.cc
  implement/MeasurementComponents/MSCleanImageSkyModel.cc
  implement/MeasurementComponents/Mueller.cc
  implement/MeasurementComponents/NNLSImageSkyModel.cc
  implement/MeasurementComponents/nPBWProjectFT.cc
  implement/MeasurementComponents/PBMath1DAiry.cc
  implement/MeasurementComponents/PBMath1D.cc
  implement/MeasurementComponents/PBMath1DCosPoly.cc
  implement/MeasurementComponents/PBMath1DGauss.cc
  implement/MeasurementComponents/PBMath1DIPoly.cc
  implement/MeasurementComponents/PBMath1DNumeric.cc
  implement/MeasurementComponents/PBMath1DPoly.cc
  implement/MeasurementComponents/PBMath2D.cc
  implement/MeasurementComponents/PBMath2DImage.cc
  implement/MeasurementComponents/PBMath.cc
  implement/MeasurementComponents/PBMathInterface.cc
  implement/MeasurementComponents/PBMosaicFT.cc
  implement/MeasurementComponents/PClarkCleanImageSkyModel.cc
  implement/MeasurementComponents/PixelatedConvFunc.cc
  implement/MeasurementComponents/PredictAlgorithm.cc
  implement/MeasurementComponents/PWFCleanImageSkyModel.cc
  implement/MeasurementComponents/ReadMSAlgorithm.cc
  implement/MeasurementComponents/ResidualAlgorithm.cc
  implement/MeasurementComponents/SDGrid.cc
  implement/MeasurementComponents/SimACohCalc.cc
  implement/MeasurementComponents/SimACoh.cc
  implement/MeasurementComponents/SimpCompGridMachine.cc
  implement/MeasurementComponents/SimpComponentGridMachine.cc
  implement/MeasurementComponents/SimpleComponentFTMachine.cc
  implement/MeasurementComponents/SimplePBConvFunc.cc
  implement/MeasurementComponents/SkyJones.cc
  implement/MeasurementComponents/SolvableVisCal.cc
  implement/MeasurementComponents/StandardVisCal.cc
  implement/MeasurementComponents/SteepestDescentSolver.cc
  implement/MeasurementComponents/SynthesisError.cc
  implement/MeasurementComponents/Utils.cc
  implement/MeasurementComponents/UVMod.cc
  implement/MeasurementComponents/VisCal.cc
  implement/MeasurementComponents/VisCalGlobals.cc
  implement/MeasurementComponents/VisCalSolver.cc
  implement/MeasurementComponents/VisVector.cc
  implement/MeasurementComponents/VLACalcIlluminationConvFunc.cc
  implement/MeasurementComponents/VLAIlluminationConvFunc.cc
  implement/MeasurementComponents/VPSkyJones.cc
  implement/MeasurementComponents/WBCleanImageSkyModel.cc
  implement/MeasurementComponents/WFCleanImageSkyModel.cc
  implement/MeasurementComponents/WPConvFunc.cc
  implement/MeasurementComponents/WProjectFT.cc
  implement/MeasurementEquations/CCList.cc
  implement/MeasurementEquations/CEMemModel.cc
  implement/MeasurementEquations/CEMemProgress.cc
  implement/MeasurementEquations/ClarkCleanLatModel.cc
  implement/MeasurementEquations/ClarkCleanModel.cc
  implement/MeasurementEquations/ClarkCleanProgress.cc
  implement/MeasurementEquations/ConvolutionEquation.cc
  implement/MeasurementEquations/CubeSkyEquation.cc
  implement/MeasurementEquations/Deconvolver.cc
  implement/MeasurementEquations/Entropy.cc
  implement/MeasurementEquations/Imager.cc
  implement/MeasurementEquations/ImagerMultiMS.cc
  implement/MeasurementEquations/IncCEMemModel.cc
  implement/MeasurementEquations/IncEntropy.cc
  implement/MeasurementEquations/IPLatConvEquation.cc
  implement/MeasurementEquations/LatConvEquation.cc
  implement/MeasurementEquations/LatticeModel.cc
  implement/MeasurementEquations/MosaicSkyEquation.cc
  implement/MeasurementEquations/PSkyEquation.cc
  implement/MeasurementEquations/Qimager.cc
  implement/MeasurementEquations/Simulator.cc
  implement/MeasurementEquations/SkyEquation.cc
  implement/MeasurementEquations/StokesImageUtil.cc
  implement/MeasurementEquations/StokesUtil2.cc
  implement/MeasurementEquations/VisEquation.cc
  implement/MeasurementEquations/VPManager.cc
  implement/Parallel/Applicator.cc
  implement/Parallel/MPIError.cc
  implement/Parallel/MPITransport.cc
  implement/Parallel/PabloIO.cc
  implement/Parallel/SerialTransport.cc
  implement/Utilities/FixVis.cc
  implement/fortran/fgridft.f
  implement/fortran/fmosaic.f
  implement/fortran/fpbmos.f
  implement/fortran/fpbwproj.f
  implement/fortran/leakantso.f
  implement/fortran/wprojgrid.f)

target_link_libraries( synthesis ${synthesis_LINK_TO} )

set_target_properties( synthesis PROPERTIES SOVERSION ${CASA_API_VERSION} )

install( TARGETS synthesis LIBRARY DESTINATION lib )

# Executables
# Unit tests
add_executable( tPixonProcessor EXCLUDE_FROM_ALL implement/DataSampling/test/tPixonProcessor.cc )
target_link_libraries( tPixonProcessor synthesis )
add_test( tPixonProcessor ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tPixonProcessor )
add_dependencies( check tPixonProcessor )

add_executable( tIDL EXCLUDE_FROM_ALL implement/IDL/test/tIDL.cc )
target_link_libraries( tIDL synthesis )
add_test( tIDL ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tIDL )
add_dependencies( check tIDL )

add_executable( tAlgoPClark EXCLUDE_FROM_ALL implement/MeasurementComponents/test/tAlgoPClark.cc )
target_link_libraries( tAlgoPClark synthesis )
add_test( tAlgoPClark ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tAlgoPClark )
add_dependencies( check tAlgoPClark )

add_executable( tBeamSquint EXCLUDE_FROM_ALL implement/MeasurementComponents/test/tBeamSquint.cc )
target_link_libraries( tBeamSquint synthesis )
add_test( tBeamSquint ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tBeamSquint )
add_dependencies( check tBeamSquint )

add_executable( tCalibrater EXCLUDE_FROM_ALL implement/MeasurementComponents/test/tCalibrater.cc )
target_link_libraries( tCalibrater synthesis )
add_test( tCalibrater ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tCalibrater )
add_dependencies( check tCalibrater )

add_executable( tPBMath EXCLUDE_FROM_ALL implement/MeasurementComponents/test/tPBMath.cc )
target_link_libraries( tPBMath synthesis )
add_test( tPBMath ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tPBMath )
add_dependencies( check tPBMath )

add_executable( tPBTable EXCLUDE_FROM_ALL implement/MeasurementComponents/test/tPBTable.cc )
target_link_libraries( tPBTable synthesis )
add_test( tPBTable ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tPBTable )
add_dependencies( check tPBTable )

add_executable( tVPSkyJones EXCLUDE_FROM_ALL implement/MeasurementComponents/test/tVPSkyJones.cc )
target_link_libraries( tVPSkyJones synthesis )
add_test( tVPSkyJones ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tVPSkyJones )
add_dependencies( check tVPSkyJones )

add_executable( dCEMemModel EXCLUDE_FROM_ALL implement/MeasurementEquations/test/dCEMemModel.cc )
target_link_libraries( dCEMemModel synthesis )
add_test( dCEMemModel ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/dCEMemModel )
add_dependencies( check dCEMemModel )

add_executable( tCCList EXCLUDE_FROM_ALL implement/MeasurementEquations/test/tCCList.cc )
target_link_libraries( tCCList synthesis )
add_test( tCCList ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tCCList )
add_dependencies( check tCCList )

add_executable( tCEMemModel EXCLUDE_FROM_ALL implement/MeasurementEquations/test/tCEMemModel.cc )
target_link_libraries( tCEMemModel synthesis )
add_test( tCEMemModel ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tCEMemModel )
add_dependencies( check tCEMemModel )

add_executable( tClarkCleanLatModel EXCLUDE_FROM_ALL implement/MeasurementEquations/test/tClarkCleanLatModel.cc )
target_link_libraries( tClarkCleanLatModel synthesis )
add_test( tClarkCleanLatModel ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tClarkCleanLatModel )
add_dependencies( check tClarkCleanLatModel )

add_executable( tClarkCleanModel EXCLUDE_FROM_ALL implement/MeasurementEquations/test/tClarkCleanModel.cc )
target_link_libraries( tClarkCleanModel synthesis )
add_test( tClarkCleanModel ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tClarkCleanModel )
add_dependencies( check tClarkCleanModel )

add_executable( tConvolutionEquation EXCLUDE_FROM_ALL implement/MeasurementEquations/test/tConvolutionEquation.cc )
target_link_libraries( tConvolutionEquation synthesis )
add_test( tConvolutionEquation ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tConvolutionEquation )
add_dependencies( check tConvolutionEquation )

add_executable( tImager EXCLUDE_FROM_ALL implement/MeasurementEquations/test/tImager.cc )
target_link_libraries( tImager synthesis )
add_test( tImager ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tImager )
add_dependencies( check tImager )

add_executable( tIncCEMemModel EXCLUDE_FROM_ALL implement/MeasurementEquations/test/tIncCEMemModel.cc )
target_link_libraries( tIncCEMemModel synthesis )
add_test( tIncCEMemModel ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tIncCEMemModel )
add_dependencies( check tIncCEMemModel )

add_executable( tLatConvEquation EXCLUDE_FROM_ALL implement/MeasurementEquations/test/tLatConvEquation.cc )
target_link_libraries( tLatConvEquation synthesis )
add_test( tLatConvEquation ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tLatConvEquation )
add_dependencies( check tLatConvEquation )

add_executable( tStokesUtil EXCLUDE_FROM_ALL implement/MeasurementEquations/test/tStokesUtil.cc )
target_link_libraries( tStokesUtil synthesis )
add_test( tStokesUtil ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tStokesUtil )
add_dependencies( check tStokesUtil )

