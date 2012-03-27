#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
#
# This file is part of CASA.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

casa_add_library( synthesis
  implement/TransformMachines/BeamSkyJones.cc
  implement/TransformMachines/BeamSquint.cc
  implement/TransformMachines/CFCache.cc
  implement/TransformMachines/CFStore.cc
  implement/TransformMachines/ComponentFTMachine.cc
  implement/TransformMachines/ConvolutionFunction.cc
  implement/TransformMachines/DBeamSkyJones.cc
  implement/TransformMachines/FTMachine.cc
  implement/TransformMachines/GridFT.cc
  implement/TransformMachines/rGridFT.cc
  implement/TransformMachines/HetArrayConvFunc.cc
  implement/TransformMachines/MosaicFT.cc
  implement/TransformMachines/MultiTermFT.cc	
  implement/TransformMachines/PBMath1DAiry.cc
  implement/TransformMachines/PBMath1D.cc
  implement/TransformMachines/PBMath1DCosPoly.cc
  implement/TransformMachines/PBMath1DGauss.cc
  implement/TransformMachines/PBMath1DIPoly.cc
  implement/TransformMachines/PBMath1DNumeric.cc
  implement/TransformMachines/PBMath1DPoly.cc
  implement/TransformMachines/PBMath2D.cc
  implement/TransformMachines/PBMath2DImage.cc
  implement/TransformMachines/PBMath.cc
  implement/TransformMachines/PBMathInterface.cc
  implement/TransformMachines/SimpCompGridMachine.cc
  implement/TransformMachines/SimpComponentGridMachine.cc
  implement/TransformMachines/SimpleComponentFTMachine.cc
  implement/TransformMachines/SimplePBConvFunc.cc
  implement/TransformMachines/SkyJones.cc
  implement/TransformMachines/StokesImageUtil.cc
  implement/TransformMachines/SynthesisError.cc
  implement/TransformMachines/Utils.cc
  implement/TransformMachines/VisModelData.cc
  implement/TransformMachines/VPSkyJones.cc
  implement/TransformMachines/WProjectFT.cc
  implement/TransformMachines/WPConvFunc.cc
  implement/fortran/fgridft.f		
  implement/fortran/fmosaic.f
  implement/fortran/wprojgrid.f
  implement/MSVis/AsynchronousInterface.cc
  implement/MSVis/AsynchronousTools.cc
  implement/MSVis/CalVisBuffer.cc
  implement/MSVis/GroupProcessor.cc
  implement/MSVis/GroupWorker.cc
  implement/MSVis/MSAnalysis.cc
  implement/MSVis/MsAverager.cc
  implement/MSVis/MSCalEnums.cc
  implement/MSVis/MSContinuumSubtractor.cc
  implement/MSVis/MSUVWGenerator.cc
  implement/MSVis/MSUtil.cc
  implement/MSVis/Partition.cc
  implement/MSVis/Reweighter.cc
  implement/MSVis/SelectAverageSpw.cc
  implement/MSVis/SimpleSubMS.cc
  implement/MSVis/StokesVector.cc
  implement/MSVis/StatWT.cc
  implement/MSVis/SubMS.cc
  implement/MSVis/UtilJ.cc
  implement/MSVis/VBContinuumSubtractor.cc
  implement/MSVis/VBGContinuumSubtractor.cc
  implement/MSVis/VBRemapper.cc
  implement/MSVis/VisBuffAccumulator.cc
  implement/MSVis/VisBuffer.cc
  implement/MSVis/VisBufferAsync.cc
  implement/MSVis/VisBufferAsyncWrapper.cc
  implement/MSVis/VisBufferUtil.cc
  implement/MSVis/VisBuffGroup.cc
  implement/MSVis/VisBuffGroupAcc.cc
  implement/MSVis/VisibilityIterator.cc
  implement/MSVis/VisibilityIteratorImpl.cc
  implement/MSVis/VisibilityIteratorImplAsync.cc
  implement/MSVis/VisibilityProcessing.cc 
  implement/MSVis/VisImagingWeight.cc
  implement/MSVis/VisIterator.cc
  implement/MSVis/VisSet.cc
  implement/MSVis/VisSetUtil.cc
  implement/MSVis/VisChunkAverager.cc
  implement/MSVis/VLAT.cc
  implement/MSVis/VWBT.cc
  implement/CalTables/BJonesDesc.cc
  implement/CalTables/BJonesMBuf.cc
  implement/CalTables/BJonesMCol.cc
  implement/CalTables/BJonesMRec.cc
  implement/CalTables/BJonesTable.cc
  implement/CalTables/CalBuffer.cc
  implement/CalTables/CalDescBuffer.cc
  implement/CalTables/CalDescColumns2.cc
  implement/CalTables/CalDescColumns.cc
  implement/CalTables/CalDescRecord.cc
  implement/CalTables/CalHistoryBuffer.cc
  implement/CalTables/CalHistoryColumns.cc
  implement/CalTables/CalHistRecord.cc
  implement/CalTables/CalInterp.cc
  implement/CalTables/CalInterpolation.cc
  implement/CalTables/CalInterpolator.cc
  implement/CalTables/CalIntpMatch.cc
  implement/CalTables/CalIter.cc
  implement/CalTables/CalMainBuffer.cc
  implement/CalTables/CalMainColumns.cc
  implement/CalTables/CalMainRecord.cc
  implement/CalTables/CalSet2.cc
  implement/CalTables/CalSetMetaInfo.cc
  implement/CalTables/CalTable2.cc
  implement/CalTables/CalTable.cc
  implement/CalTables/CalTableDesc2.cc
  implement/CalTables/CalTableDesc.cc
  implement/CalTables/CTColumns.cc
  implement/CalTables/CTDesc.cc
  implement/CalTables/CTEnums.cc
  implement/CalTables/CTGlobals.cc
  implement/CalTables/CTInterface.cc
  implement/CalTables/CTIter.cc
  implement/CalTables/CTMainColumns.cc
  implement/CalTables/CTMainRecord.cc
  implement/CalTables/CTPatchedInterp.cc
  implement/CalTables/CTTimeInterp1.cc
  implement/CalTables/EPointDesc.cc
  implement/CalTables/EPointMBuf.cc
  implement/CalTables/EPointMCol.cc
  implement/CalTables/EPointTable.cc
  implement/CalTables/EPointVJMRec.cc
  implement/CalTables/FVisJonesDesc.cc
  implement/CalTables/FVisJonesMRec.cc
  implement/CalTables/GJonesDesc.cc
  implement/CalTables/GJonesMBuf.cc
  implement/CalTables/GJonesMCol.cc
  implement/CalTables/GJonesTable.cc
  implement/CalTables/MJonesDesc.cc
  implement/CalTables/MJonesMRec.cc
  implement/CalTables/NewCalTable.cc
  implement/CalTables/RIorAParray.cc
  implement/CalTables/SolvableMJDesc.cc
  implement/CalTables/SolvableMJMRec.cc
  implement/CalTables/SolvableVJDesc.cc
  implement/CalTables/SolvableVJMBuf.cc
  implement/CalTables/SolvableVJMCol.cc
  implement/CalTables/SolvableVJMRec.cc
  implement/CalTables/SolvableVJTable.cc
  implement/CalTables/TimeVarMJDesc.cc
  implement/CalTables/TimeVarMJMRec.cc
  implement/CalTables/TimeVarVJDesc.cc
  implement/CalTables/TimeVarVJMBuf.cc
  implement/CalTables/TimeVarVJMCol.cc
  implement/CalTables/TimeVarVJMRec.cc
  implement/CalTables/TimeVarVJTable.cc
  implement/CalTables/TJonesDesc.cc
  implement/CalTables/TJonesMBuf.cc
  implement/CalTables/TJonesMCol.cc
  implement/CalTables/TJonesTable.cc
  implement/CalTables/VisJonesDesc.cc
  implement/CalTables/VisJonesMBuf.cc
  implement/CalTables/VisJonesMCol.cc
  implement/CalTables/VisJonesMRec.cc
  implement/CalTables/VisJonesTable.cc
  implement/Parallel/Applicator.cc
 implement/Parallel/MPIError.cc
 implement/Parallel/MPITransport.cc
 implement/Parallel/PabloIO.cc
 implement/Parallel/SerialTransport.cc
 implement/DataSampling/DataSampling.cc
 implement/DataSampling/ImageDataSampling.cc
 implement/DataSampling/PixonProcessor.cc
 implement/DataSampling/SDDataSampling.cc
 implement/DataSampling/SynDataSampling.cc
 implement/IDL/IDL.cc
 implement/MeasurementComponents/ALMAAperture.cc
 implement/MeasurementComponents/ALMABeamCalcParams.cc
 implement/MeasurementComponents/ALMACalcIlluminationConvFunc.cc
 implement/MeasurementComponents/AMueller.cc
 implement/MeasurementComponents/AWConvFunc.cc
 implement/MeasurementComponents/AWProjectFT.cc
 implement/MeasurementComponents/AWProjectWBFT.cc
 implement/MeasurementComponents/AWVisResampler.cc
 implement/MeasurementComponents/BeamCalc.cc
 implement/MeasurementComponents/BeamCalcParams.cc
 implement/MeasurementComponents/BPoly.cc
 implement/MeasurementComponents/CalCorruptor.cc
 implement/MeasurementComponents/Calibrater.cc
 implement/MeasurementComponents/CEMemImageSkyModel.cc
 implement/MeasurementComponents/ClarkCleanAlgorithm.cc
 implement/MeasurementComponents/ClarkCleanImageSkyModel.cc
 implement/MeasurementComponents/CleanImageSkyModel.cc
 implement/MeasurementComponents/ConvFuncDiskCache.cc
 implement/MeasurementComponents/CSCleanImageSkyModel.cc
 implement/MeasurementComponents/EVLAConvFunc.cc
 implement/MeasurementComponents/EVLAAperture.cc
 implement/MeasurementComponents/EJones.cc
 implement/MeasurementComponents/EPJones.cc
 implement/MeasurementComponents/GlobalFTMachineCallbacks.cc
 implement/MeasurementComponents/GridBoth.cc
 implement/MeasurementComponents/GSpline.cc
 implement/MeasurementComponents/HogbomCleanImageSkyModel.cc
 implement/MeasurementComponents/IlluminationConvFunc.cc
 implement/MeasurementComponents/ImageSkyModel.cc
 implement/MeasurementComponents/Jones.cc
 implement/MeasurementComponents/KJones.cc
 implement/MeasurementComponents/LJJones.cc
 implement/MeasurementComponents/MakeApproxPSFAlgorithm.cc
 implement/MeasurementComponents/MFCEMemImageSkyModel.cc
 implement/MeasurementComponents/MFCleanImageSkyModel.cc
 implement/MeasurementComponents/MFMSCleanImageSkyModel.cc
 implement/MeasurementComponents/MSCleanImageSkyModel.cc
 implement/MeasurementComponents/Mueller.cc
 implement/MeasurementComponents/MultiThreadedVisResampler.cc
 implement/MeasurementComponents/NNLSImageSkyModel.cc
 implement/MeasurementComponents/nPBWProjectFT.cc
 implement/MeasurementComponents/PBMosaicFT.cc
 implement/MeasurementComponents/PClarkCleanImageSkyModel.cc
 implement/MeasurementComponents/PixelatedConvFunc.cc
 implement/MeasurementComponents/PredictAlgorithm.cc
 implement/MeasurementComponents/PSTerm.cc
 implement/MeasurementComponents/PWFCleanImageSkyModel.cc
 implement/MeasurementComponents/ReadMSAlgorithm.cc
 implement/MeasurementComponents/ResidualAlgorithm.cc
 implement/MeasurementComponents/ResamplerWorklet.cc
 implement/MeasurementComponents/SDGrid.cc
 implement/MeasurementComponents/SimACohCalc.cc
 implement/MeasurementComponents/SimACoh.cc
 implement/MeasurementComponents/SolvableVisCal.cc
 implement/MeasurementComponents/StandardVisCal.cc
 implement/MeasurementComponents/SteepestDescentSolver.cc
 implement/MeasurementComponents/TsysGainCal.cc
 implement/MeasurementComponents/UVMod.cc
 implement/MeasurementComponents/VisCal.cc
 implement/MeasurementComponents/VisCalGlobals.cc
 implement/MeasurementComponents/VisCalSolver.cc
 implement/MeasurementComponents/VisVector.cc
 implement/MeasurementComponents/VisibilityResamplerBase.cc
 implement/MeasurementComponents/VisibilityResampler.cc
 implement/MeasurementComponents/VLACalcIlluminationConvFunc.cc
 implement/MeasurementComponents/VLAIlluminationConvFunc.cc
 implement/MeasurementComponents/WBCleanImageSkyModel.cc
 implement/MeasurementComponents/WFCleanImageSkyModel.cc
 implement/MeasurementComponents/WOnlyConvFunc.cc
 implement/MeasurementComponents/WTerm.cc
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
 implement/MeasurementEquations/Feather.cc
 implement/MeasurementEquations/Imager.cc
 implement/MeasurementEquations/Imager2.cc
 implement/MeasurementEquations/ImagerMultiMS.cc
 implement/MeasurementEquations/ImageMSCleaner.cc
 implement/MeasurementEquations/IncCEMemModel.cc
 implement/MeasurementEquations/IncEntropy.cc
 implement/MeasurementEquations/IPLatConvEquation.cc
 implement/MeasurementEquations/LatConvEquation.cc
 implement/MeasurementEquations/LatticeModel.cc
 implement/MeasurementEquations/MatrixCleaner.cc
 implement/MeasurementEquations/MultiTermMatrixCleaner.cc
 implement/MeasurementEquations/Simulator.cc
 implement/MeasurementEquations/SkyEquation.cc
 implement/MeasurementEquations/StokesUtil2.cc
 implement/MeasurementEquations/VisEquation.cc
 implement/MeasurementEquations/VPManager.cc
 implement/Parallel/Applicator.cc
 implement/Parallel/MPIError.cc
 implement/Parallel/MPITransport.cc
 implement/Parallel/PabloIO.cc
 implement/Parallel/SerialTransport.cc
 implement/Utilities/FixVis.cc
 implement/Utilities/ThreadCoordinator.cc
 implement/fortran/fpbmos.f	
 implement/fortran/fpbwproj.f	
 implement/fortran/leakantso.f
  )

install (FILES
	implement/CalTables/BJonesDesc.h
	implement/CalTables/BJonesMBuf.h
	implement/CalTables/BJonesMCol.h
	implement/CalTables/BJonesMRec.h
	implement/CalTables/BJonesTable.h
	implement/CalTables/CTColumns.h
	implement/CalTables/CTDesc.h
	implement/CalTables/CTEnums.h
	implement/CalTables/CTInterface.h
	implement/CalTables/CTIter.h
	implement/CalTables/CTMainColumns.h
	implement/CalTables/CTMainRecord.h
	implement/CalTables/CTPatchedInterp.h
	implement/CalTables/CTTimeInterp1.h
	implement/CalTables/CalBuffer.h
	implement/CalTables/CalDescBuffer.h
	implement/CalTables/CalDescColumns.h
	implement/CalTables/CalDescColumns2.h
	implement/CalTables/CalDescRecord.h
	implement/CalTables/CalHistRecord.h
	implement/CalTables/CalHistoryBuffer.h
	implement/CalTables/CalHistoryColumns.h
	implement/CalTables/CalInterp.h
	implement/CalTables/CalInterpolation.h
	implement/CalTables/CalInterpolator.h
	implement/CalTables/CalIntpMatch.h
	implement/CalTables/CalIter.h
	implement/CalTables/CalIterBase.h
	implement/CalTables/CalMainBuffer.h
	implement/CalTables/CalMainColumns.h
	implement/CalTables/CalMainColumns2.h
	implement/CalTables/CalMainRecord.h
	implement/CalTables/CalSet.h
	implement/CalTables/CalSetMetaInfo.h
	implement/CalTables/CalTable.h
	implement/CalTables/CalTable2.h
	implement/CalTables/CalTableDesc.h
	implement/CalTables/CalTableDesc2.h
	implement/CalTables/CalTableFromAIPS.h
	implement/CalTables/EPointDesc.h
	implement/CalTables/EPointMBuf.h
	implement/CalTables/EPointMCol.h
	implement/CalTables/EPointTable.h
	implement/CalTables/EPointVJMRec.h
	implement/CalTables/FVisJonesDesc.h
	implement/CalTables/FVisJonesMRec.h
	implement/CalTables/GJonesDesc.h
	implement/CalTables/GJonesMBuf.h
	implement/CalTables/GJonesMCol.h
	implement/CalTables/GJonesTable.h
	implement/CalTables/MJonesDesc.h
	implement/CalTables/MJonesMRec.h
	implement/CalTables/NewCalTable.h
	implement/CalTables/RIorAParray.h
	implement/CalTables/ROCalMainColumns2.h
	implement/CalTables/SkyJonesDesc.h
	implement/CalTables/SolvableCalSetMCol.h
	implement/CalTables/SolvableMJDesc.h
	implement/CalTables/SolvableMJMRec.h
	implement/CalTables/SolvableVJDesc.h
	implement/CalTables/SolvableVJMBuf.h
	implement/CalTables/SolvableVJMCol.h
	implement/CalTables/SolvableVJMRec.h
	implement/CalTables/SolvableVJTable.h
	implement/CalTables/TJonesDesc.h
	implement/CalTables/TJonesMBuf.h
	implement/CalTables/TJonesMCol.h
	implement/CalTables/TJonesTable.h
	implement/CalTables/TimeVarMJDesc.h
	implement/CalTables/TimeVarMJMRec.h
	implement/CalTables/TimeVarVJDesc.h
	implement/CalTables/TimeVarVJMBuf.h
	implement/CalTables/TimeVarVJMCol.h
	implement/CalTables/TimeVarVJMRec.h
	implement/CalTables/TimeVarVJTable.h
	implement/CalTables/VisCalEnum.h
	implement/CalTables/VisJonesDesc.h
	implement/CalTables/VisJonesMBuf.h
	implement/CalTables/VisJonesMCol.h
	implement/CalTables/VisJonesMRec.h
	implement/CalTables/VisJonesTable.h
	implement/CalTables/CalMainColumns2.tcc
	implement/CalTables/CalSet.tcc
	implement/CalTables/ROCalMainColumns2.tcc
	implement/CalTables/SolvableCalSetMCol.tcc
	DESTINATION include/casacode/synthesis/CalTables
	)
install (FILES
	implement/DataSampling/DataSampling.h
	implement/DataSampling/ImageDataSampling.h
	implement/DataSampling/PixonProcessor.h
	implement/DataSampling/SDDataSampling.h
	implement/DataSampling/SynDataSampling.h
	DESTINATION include/casacode/synthesis/DataSampling
	)
install (FILES
	implement/IDL/IDL.h
	DESTINATION include/casacode/synthesis/IDL
	)
install (FILES
	implement/MSVis/AsynchronousInterface.h
	implement/MSVis/AsynchronousTools.h
	implement/MSVis/CalVisBuffer.h
	implement/MSVis/GroupProcessor.h
	implement/MSVis/GroupWorker.h
	implement/MSVis/MSAnalysis.h
	implement/MSVis/MSCalEnums.h
	implement/MSVis/MSContinuumSubtractor.h
	implement/MSVis/MSMoments.h
	implement/MSVis/MSUVWGenerator.h
	implement/MSVis/MSUtil.h
	implement/MSVis/MsAverager.h
	implement/MSVis/Partition.h
	implement/MSVis/Reweighter.h
	implement/MSVis/SelectAverageSpw.h
	implement/MSVis/SimpleSubMS.h
	implement/MSVis/StatWT.h
	implement/MSVis/StokesVector.h
	implement/MSVis/SubMS.h
	implement/MSVis/UtilJ.h
	implement/MSVis/VBContinuumSubtractor.h
	implement/MSVis/VBGContinuumSubtractor.h
	implement/MSVis/VBRemapper.h
	implement/MSVis/VLAT.h
	implement/MSVis/VWBT.h
	implement/MSVis/VisBuffAccumulator.h
	implement/MSVis/VisBuffGroup.h
	implement/MSVis/VisBuffGroupAcc.h
	implement/MSVis/VisBuffer.h
	implement/MSVis/VisBufferAsync.h
	implement/MSVis/VisBufferAsyncWrapper.h
	implement/MSVis/VisBufferComponents.h
	implement/MSVis/VisBufferUtil.h
	implement/MSVis/VisChunkAverager.h
	implement/MSVis/VisImagingWeight.h
	implement/MSVis/VisIterator.h
	implement/MSVis/VisSet.h
	implement/MSVis/VisSetUtil.h
	implement/MSVis/VisibilityIterator.h
	implement/MSVis/VisibilityIteratorImpl.h
	implement/MSVis/VisibilityIteratorImplAsync.h
	implement/MSVis/VisibilityProcessing.h
	implement/MSVis/MSMoments.tcc
	implement/MSVis/VisBuffer.tcc
	implement/MSVis/VisibilityIterator.tcc
	DESTINATION include/casacode/synthesis/MSVis
	)
install (FILES
	implement/MeasurementComponents/ACoh.h
	implement/MeasurementComponents/ALMAAperture.h
	implement/MeasurementComponents/ALMACalcIlluminationConvFunc.h
	implement/MeasurementComponents/AMueller.h
	implement/MeasurementComponents/ATerm.h
	implement/MeasurementComponents/AWConvFunc.h
	implement/MeasurementComponents/AWProjectFT.h
	implement/MeasurementComponents/AWProjectWBFT.h
	implement/MeasurementComponents/AWVisResampler.h
	implement/MeasurementComponents/AzElAperture.h
	implement/MeasurementComponents/BPoly.h
	implement/MeasurementComponents/BeamCalc.h
	implement/MeasurementComponents/CEMemImageSkyModel.h
	implement/MeasurementComponents/CExp.h
	implement/MeasurementComponents/CExp.new3.h
	implement/MeasurementComponents/CFDefs.h
	implement/MeasurementComponents/CFTerms.h
	implement/MeasurementComponents/CSCleanImageSkyModel.h
	implement/MeasurementComponents/CalCorruptor.h
	implement/MeasurementComponents/Calibrater.h
	implement/MeasurementComponents/ClarkCleanAlgorithm.h
	implement/MeasurementComponents/ClarkCleanImageSkyModel.h
	implement/MeasurementComponents/CleanImageSkyModel.h
	implement/MeasurementComponents/ConvFuncDiskCache.h
	implement/MeasurementComponents/EJones.h
	implement/MeasurementComponents/EPJones.h
	implement/MeasurementComponents/EVLAAperture.h
	implement/MeasurementComponents/EVLAConvFunc.h
	implement/MeasurementComponents/ExpCache.h
	implement/MeasurementComponents/GSpline.h
	implement/MeasurementComponents/GlobalFTMachineCallbacks.h
	implement/MeasurementComponents/GridBoth.h
	implement/MeasurementComponents/HogbomCleanImageSkyModel.h
	implement/MeasurementComponents/IlluminationConvFunc.h
	implement/MeasurementComponents/ImageSkyModel.h
	implement/MeasurementComponents/Jones.h
	implement/MeasurementComponents/KJones.h
	implement/MeasurementComponents/LJJones.h
	implement/MeasurementComponents/MFCEMemImageSkyModel.h
	implement/MeasurementComponents/MFCleanImageSkyModel.h
	implement/MeasurementComponents/MFMSCleanImageSkyModel.h
	implement/MeasurementComponents/MSCleanImageSkyModel.h
	implement/MeasurementComponents/MThWorkIDEnum.h
	implement/MeasurementComponents/MakeApproxPSFAlgorithm.h
	implement/MeasurementComponents/Mueller.h
	implement/MeasurementComponents/MultiThreadedVisResampler.h
	implement/MeasurementComponents/NNLSImageSkyModel.h
	implement/MeasurementComponents/PBMosaicFT.h
	implement/MeasurementComponents/PClarkCleanImageSkyModel.h
	implement/MeasurementComponents/PSTerm.h
	implement/MeasurementComponents/PWFCleanImageSkyModel.h
	implement/MeasurementComponents/PixelatedConvFunc.h
	implement/MeasurementComponents/PredictAlgorithm.h
	implement/MeasurementComponents/ReadMSAlgorithm.h
	implement/MeasurementComponents/ResamplerWorklet.h
	implement/MeasurementComponents/ResidualAlgorithm.h
	implement/MeasurementComponents/SDGrid.h
	implement/MeasurementComponents/SimACoh.h
	implement/MeasurementComponents/SimACohCalc.h
	implement/MeasurementComponents/SkyModel.h
	implement/MeasurementComponents/SolvableVisCal.h
	implement/MeasurementComponents/StandardVisCal.h
	implement/MeasurementComponents/SteepestDescentSolver.h
	implement/MeasurementComponents/TabulatedConvFunc.h
	implement/MeasurementComponents/TsysGainCal.h
	implement/MeasurementComponents/UVMod.h
	implement/MeasurementComponents/VBStore.h
	implement/MeasurementComponents/VLACalcIlluminationConvFunc.h
	implement/MeasurementComponents/VLAIlluminationConvFunc.h
	implement/MeasurementComponents/VisCal.h
	implement/MeasurementComponents/VisCalGlobals.h
	implement/MeasurementComponents/VisCalSolver.h
	implement/MeasurementComponents/VisVector.h
	implement/MeasurementComponents/VisibilityResampler.h
	implement/MeasurementComponents/VisibilityResamplerBase.h
	implement/MeasurementComponents/WBCleanImageSkyModel.h
	implement/MeasurementComponents/WFCleanImageSkyModel.h
	implement/MeasurementComponents/WOnlyConvFunc.h
	implement/MeasurementComponents/WOnlyProjectFT.h
	implement/MeasurementComponents/WTerm.h
	implement/MeasurementComponents/XCorr.h
	implement/MeasurementComponents/nPBWProjectFT.h
	implement/MeasurementComponents/rGridFT.h
	implement/MeasurementComponents/ImageSkyModel.tcc
	DESTINATION include/casacode/synthesis/MeasurementComponents
	)
install (FILES
	implement/MeasurementEquations/ArrayModel.h
	implement/MeasurementEquations/CCList.h
	implement/MeasurementEquations/CEMemModel.h
	implement/MeasurementEquations/CEMemProgress.h
	implement/MeasurementEquations/ClarkCleanLatModel.h
	implement/MeasurementEquations/ClarkCleanModel.h
	implement/MeasurementEquations/ClarkCleanProgress.h
	implement/MeasurementEquations/ConvolutionEquation.h
	implement/MeasurementEquations/CubeSkyEquation.h
	implement/MeasurementEquations/Deconvolver.h
	implement/MeasurementEquations/Entropy.h
	implement/MeasurementEquations/HogbomCleanModel.h
	implement/MeasurementEquations/IPLatConvEquation.h
	implement/MeasurementEquations/ImageMSCleaner.h
	implement/MeasurementEquations/Imager.h
	implement/MeasurementEquations/ImagerMultiMS.h
	implement/MeasurementEquations/IncCEMemModel.h
	implement/MeasurementEquations/IncEntropy.h
	implement/MeasurementEquations/Iterate.h
	implement/MeasurementEquations/LatConvEquation.h
	implement/MeasurementEquations/LatticeModel.h
	implement/MeasurementEquations/LinearEquation.h
	implement/MeasurementEquations/LinearModel.h
	implement/MeasurementEquations/MaskedArrayModel.h
	implement/MeasurementEquations/MaskedHogbomCleanModel.h
	implement/MeasurementEquations/MatrixCleaner.h
	implement/MeasurementEquations/MultiTermMatrixCleaner.h
	implement/MeasurementEquations/ResidualEquation.h
	implement/MeasurementEquations/Simulator.h
	implement/MeasurementEquations/SkyEquation.h
	implement/MeasurementEquations/StokesUtil.h
	implement/MeasurementEquations/VPManager.h
	implement/MeasurementEquations/VisEquation.h
	implement/MeasurementEquations/ArrayModel.tcc
	implement/MeasurementEquations/HogbomCleanModel.tcc
	implement/MeasurementEquations/LinearEquation.tcc
	implement/MeasurementEquations/LinearModel.tcc
	implement/MeasurementEquations/MaskedArrayModel.tcc
	implement/MeasurementEquations/MaskedHogbomCleanModel.tcc
	implement/MeasurementEquations/ResidualEquation.tcc
	implement/MeasurementEquations/StokesUtil.tcc
	DESTINATION include/casacode/synthesis/MeasurementEquations
	)
install (FILES
	implement/Parallel/Algorithm.h
	implement/Parallel/Applicator.h
	implement/Parallel/MPIError.h
	implement/Parallel/PTransport.h
	implement/Parallel/PabloIO.h
	DESTINATION include/casacode/synthesis/Parallel
	)
install (FILES
	implement/TransformMachines/BeamSkyJones.h
	implement/TransformMachines/BeamSquint.h
	implement/TransformMachines/CFCache.h
	implement/TransformMachines/CFDefs.h
	implement/TransformMachines/CFStore.h
	implement/TransformMachines/ComponentFTMachine.h
	implement/TransformMachines/ConvolutionFunction.h
	implement/TransformMachines/DBeamSkyJones.h
	implement/TransformMachines/FTMachine.h
	implement/TransformMachines/GridFT.h
	implement/TransformMachines/HetArrayConvFunc.h
	implement/TransformMachines/MosaicFT.h
	implement/TransformMachines/MultiTermFT.h
	implement/TransformMachines/PBMath.h
	implement/TransformMachines/PBMath1D.h
	implement/TransformMachines/PBMath1DAiry.h
	implement/TransformMachines/PBMath1DCosPoly.h
	implement/TransformMachines/PBMath1DGauss.h
	implement/TransformMachines/PBMath1DIPoly.h
	implement/TransformMachines/PBMath1DNumeric.h
	implement/TransformMachines/PBMath1DPoly.h
	implement/TransformMachines/PBMath2D.h
	implement/TransformMachines/PBMath2DImage.h
	implement/TransformMachines/PBMathInterface.h
	implement/TransformMachines/SimpCompGridMachine.h
	implement/TransformMachines/SimpComponentGridMachine.h
	implement/TransformMachines/SimpleComponentFTMachine.h
	implement/TransformMachines/SimplePBConvFunc.h
	implement/TransformMachines/SkyJones.h
	implement/TransformMachines/StokesImageUtil.h
	implement/TransformMachines/SynthesisError.h
	implement/TransformMachines/Utils.h
	implement/TransformMachines/VPSkyJones.h
	implement/TransformMachines/VisModelData.h
	implement/TransformMachines/WPConvFunc.h
	implement/TransformMachines/WProjectFT.h
	DESTINATION include/casacode/synthesis/TransformMachines
	)
install (FILES
	implement/Utilities/AppRC.h
	implement/Utilities/FixVis.h
	implement/Utilities/ThreadCoordinator.h
	DESTINATION include/casacode/synthesis/Utilities
	)
casa_add_executable( synthesis casasplit apps/casasplit/casasplit.cc )
casa_add_assay( synthesis implement/MSVis/test/tHanningSmooth.cc )
casa_add_assay( synthesis implement/MSVis/test/tMSCalEnums.cc )
casa_add_assay( synthesis implement/MSVis/test/tUVSub.cc )
casa_add_assay( synthesis implement/MSVis/test/tVisibilityIterator.cc )
casa_add_assay( synthesis implement/MSVis/test/tVisibilityIteratorAsync.cc )
casa_add_assay( synthesis implement/MSVis/test/VisibilityProcessing_Test.cc )
casa_add_assay( synthesis implement/MSVis/test/tVisIter.cc )
casa_add_assay( synthesis implement/TransformMachines/test/tVisModelData.cc )
casa_add_assay( synthesis implement/DataSampling/test/tPixonProcessor.cc )
casa_add_assay( synthesis implement/IDL/test/tIDL.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tALMAAperture.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tEVLAAperture.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tAlgoPClark.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tBeamSquint.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tCalibrater.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tPBMath.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tPBMath2D.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tPBTable.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tVPSkyJones.cc )
casa_add_assay( synthesis implement/MeasurementEquations/test/dCEMemModel.cc )
casa_add_assay( synthesis implement/MeasurementEquations/test/tCCList.cc )
casa_add_assay( synthesis implement/MeasurementEquations/test/tCEMemModel.cc )
casa_add_assay( synthesis implement/MeasurementEquations/test/tClarkCleanLatModel.cc )
casa_add_assay( synthesis implement/MeasurementEquations/test/tClarkCleanModel.cc )
casa_add_assay( synthesis implement/MeasurementEquations/test/tConvolutionEquation.cc )
casa_add_assay( synthesis implement/MeasurementEquations/test/tImager.cc )
casa_add_assay( synthesis implement/MeasurementEquations/test/tIncCEMemModel.cc )
casa_add_assay( synthesis implement/MeasurementEquations/test/tLatConvEquation.cc )
casa_add_assay( synthesis implement/MeasurementEquations/test/tStokesUtil.cc )
casa_add_assay( synthesis implement/Parallel/test/tApplicator.cc )
casa_add_assay( synthesis implement/CalTables/test/tCalInterpolation.cc )
casa_add_assay( synthesis implement/CalTables/test/tCalIntpMatch.cc )
casa_add_assay( synthesis implement/CalTables/test/tNewCalTable.cc )
casa_add_assay( synthesis implement/CalTables/test/tCTIter.cc )
casa_add_assay( synthesis implement/CalTables/test/tRIorAPArray.cc )
casa_add_assay( synthesis implement/CalTables/test/tCTTimeInterp1.cc )
casa_add_assay( synthesis implement/CalTables/test/tCTPatchedInterp.cc )
casa_add_assay( synthesis implement/CalTables/test/tCTSelection.cc )
casa_add_assay( synthesis implement/CalTables/test/tCTGenericFill.cc )
