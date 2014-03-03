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
  TransformMachines/BeamSkyJones.cc
  TransformMachines/BeamSquint.cc
  TransformMachines/CFCache.cc
  TransformMachines/CFStore.cc
  TransformMachines/CFStore2.cc
  TransformMachines/CFBuffer.cc
  TransformMachines/CFCell.cc
  TransformMachines/PolOuterProduct.cc
  TransformMachines/ComponentFTMachine.cc
  TransformMachines/ConvolutionFunction.cc
  TransformMachines/AWConvFunc.cc
  TransformMachines/AWConvFuncEPJones.cc
  TransformMachines/AWProjectFT.cc
  TransformMachines/AWProjectWBFT.cc
  TransformMachines/AWVisResampler.cc
  TransformMachines/BeamCalc.cc
  TransformMachines/BeamCalcParams.cc
  TransformMachines/DBeamSkyJones.cc
  TransformMachines/FTMachine.cc
  TransformMachines/GridFT.cc
  TransformMachines/SetJyGridFT.cc
  TransformMachines/rGridFT.cc
  TransformMachines/HetArrayConvFunc.cc
  TransformMachines/MosaicFT.cc
  TransformMachines/MultiTermFT.cc	
  TransformMachines/NewMultiTermFT.cc	
  TransformMachines/PBMath1DAiry.cc
  TransformMachines/PBMath1D.cc
  TransformMachines/PBMath1DCosPoly.cc
  TransformMachines/PBMath1DGauss.cc
  TransformMachines/PBMath1DIPoly.cc
  TransformMachines/PBMath1DNumeric.cc
  TransformMachines/PBMath1DPoly.cc
  TransformMachines/PBMath2D.cc
  TransformMachines/PBMath2DImage.cc
  TransformMachines/PBMath.cc
  TransformMachines/PBMathInterface.cc
  TransformMachines/SimpCompGridMachine.cc
  TransformMachines/SimpComponentGridMachine.cc
  TransformMachines/SimpleComponentFTMachine.cc
  TransformMachines/SimplePBConvFunc.cc
  TransformMachines/SkyJones.cc
  TransformMachines/StokesImageUtil.cc
  TransformMachines/SynthesisError.cc
  TransformMachines/Utils.cc
  TransformMachines/VisModelData.cc
  TransformMachines/VPSkyJones.cc
  TransformMachines/WProjectFT.cc
  TransformMachines/WPConvFunc.cc
  TransformMachines/EVLAConvFunc.cc
  TransformMachines/ATerm.cc
  TransformMachines/EVLAAperture.cc
  TransformMachines/AzElAperture.cc
  fortran/fgridft.f		
  fortran/fmosaic.f
  fortran/wprojgrid.f
  fortran/faccumulateToGrid.f
  fortran/faccumulateFromGrid.f
  MSVis/AsynchronousInterface.cc
  MSVis/AsynchronousTools.cc
  MSVis/CalVisBuffer.cc
  MSVis/GroupProcessor.cc
  MSVis/GroupWorker.cc
  MSVis/MeasurementSet2.cc
  MSVis/MSAnalysis.cc
  MSVis/MsAverager.cc
  MSVis/MSCalEnums.cc
  MSVis/MSContinuumSubtractor.cc
  MSVis/MSUVWGenerator.cc
  MSVis/MSUtil.cc
  MSVis/Partition.cc
  MSVis/Reweighter.cc
  MSVis/SelectAverageSpw.cc
  MSVis/SimpleSubMS.cc
  MSVis/SpectralWindow.cc
  MSVis/StokesVector.cc
  MSVis/StatWT.cc
  MSVis/SubMS.cc
  MSVis/UtilJ.cc
  MSVis/VBContinuumSubtractor.cc
  MSVis/VBGContinuumSubtractor.cc
  MSVis/VBRemapper.cc
  MSVis/VisBuffAccumulator.cc
  MSVis/VisBuffer.cc
  MSVis/VisBufferAsync.cc
  MSVis/VisBufferAsyncWrapper.cc
  MSVis/VisBufferUtil.cc
  MSVis/VisBuffGroup.cc
  MSVis/VisBuffGroupAcc.cc
  MSVis/VisibilityIterator.cc
  MSVis/VisibilityIteratorImpl.cc
  MSVis/VisibilityIteratorImplAsync.cc
  MSVis/VisibilityProcessing.cc 
####   MSVis/AsynchronousInterface2.cc
  MSVis/VisBuffer2.cc
  MSVis/VisBufferImpl2.cc
  MSVis/MsRows.cc
  MSVis/Vbi2MsRow.cc
####  MSVis/VisBufferImplAsync2.cc
####  MSVis/VisBufferAsyncWrapper2.cc
  MSVis/VisBufferComponents2.cc
  MSVis/ViColumns2.cc
  MSVis/ViFrequencySelection.cc
  MSVis/ViImplementation2.cc
  MSVis/VisibilityIterator2.cc
  MSVis/VisibilityIteratorImpl2.cc
  MSVis/TransformingVi2.cc
  MSVis/AveragingTvi2.cc
  MSVis/AveragingVi2Factory.cc
  MSVis/FinalTvi2.cc
####  MSVis/VisibilityIteratorImplAsync2.cc
  MSVis/VisImagingWeight.cc
  MSVis/VisIterator.cc
  MSVis/VisSet.cc
  MSVis/VisSetUtil.cc
  MSVis/VisChunkAverager.cc
  MSVis/VLAT.cc
  MSVis/VWBT.cc
  CalTables/BJonesDesc.cc
  CalTables/BJonesMBuf.cc
  CalTables/BJonesMCol.cc
  CalTables/BJonesMRec.cc
  CalTables/BJonesTable.cc
  CalTables/CalBuffer.cc
  CalTables/CalDescBuffer.cc
  CalTables/CalDescColumns2.cc
  CalTables/CalDescColumns.cc
  CalTables/CalDescRecord.cc
  CalTables/CalHistoryBuffer.cc
  CalTables/CalHistoryColumns.cc
  CalTables/CalHistRecord.cc
  CalTables/CalInterp.cc
  CalTables/CalInterpolation.cc
  CalTables/CalInterpolator.cc
  CalTables/CalIntpMatch.cc
  CalTables/CalIter.cc
  CalTables/CalMainBuffer.cc
  CalTables/CalMainColumns.cc
  CalTables/CalMainRecord.cc
  CalTables/CalSet2.cc
  CalTables/CalSetMetaInfo.cc
  CalTables/CalTable2.cc
  CalTables/CalTable.cc
  CalTables/CalTableDesc2.cc
  CalTables/CalTableDesc.cc
  CalTables/CLPatchPanel.cc
  CalTables/CTColumns.cc
  CalTables/CTDesc.cc
  CalTables/CTEnums.cc
  CalTables/CTGlobals.cc
  CalTables/CTInterface.cc
  CalTables/CTIter.cc
  CalTables/CTMainColumns.cc
  CalTables/CTMainRecord.cc
  CalTables/CTPatchedInterp.cc
  CalTables/CTTimeInterp1.cc
  CalTables/EPointDesc.cc
  CalTables/EPointMBuf.cc
  CalTables/EPointMCol.cc
  CalTables/EPointTable.cc
  CalTables/EPointVJMRec.cc
  CalTables/FVisJonesDesc.cc
  CalTables/FVisJonesMRec.cc
  CalTables/GJonesDesc.cc
  CalTables/GJonesMBuf.cc
  CalTables/GJonesMCol.cc
  CalTables/GJonesTable.cc
  CalTables/MJonesDesc.cc
  CalTables/MJonesMRec.cc
  CalTables/NewCalTable.cc
  CalTables/RIorAParray.cc
  CalTables/SolvableMJDesc.cc
  CalTables/SolvableMJMRec.cc
  CalTables/SolvableVJDesc.cc
  CalTables/SolvableVJMBuf.cc
  CalTables/SolvableVJMCol.cc
  CalTables/SolvableVJMRec.cc
  CalTables/SolvableVJTable.cc
  CalTables/TimeVarMJDesc.cc
  CalTables/TimeVarMJMRec.cc
  CalTables/TimeVarVJDesc.cc
  CalTables/TimeVarVJMBuf.cc
  CalTables/TimeVarVJMCol.cc
  CalTables/TimeVarVJMRec.cc
  CalTables/TimeVarVJTable.cc
  CalTables/TJonesDesc.cc
  CalTables/TJonesMBuf.cc
  CalTables/TJonesMCol.cc
  CalTables/TJonesTable.cc
  CalTables/VisJonesDesc.cc
  CalTables/VisJonesMBuf.cc
  CalTables/VisJonesMCol.cc
  CalTables/VisJonesMRec.cc
  CalTables/VisJonesTable.cc
  Parallel/Applicator.cc
 Parallel/MPIError.cc
 Parallel/MPITransport.cc
 Parallel/PabloIO.cc
 Parallel/SerialTransport.cc
 DataSampling/DataSampling.cc
 DataSampling/ImageDataSampling.cc
 DataSampling/PixonProcessor.cc
 DataSampling/SDDataSampling.cc
 DataSampling/SynDataSampling.cc
 IDL/IDL.cc
 MeasurementComponents/AMueller.cc
 MeasurementComponents/BPoly.cc
 MeasurementComponents/CalCorruptor.cc
 MeasurementComponents/Calibrater.cc
 MeasurementComponents/CEMemImageSkyModel.cc
 MeasurementComponents/ClarkCleanAlgorithm.cc
 MeasurementComponents/ClarkCleanImageSkyModel.cc
 MeasurementComponents/CleanImageSkyModel.cc
 MeasurementComponents/ConvFuncDiskCache.cc
 MeasurementComponents/CSCleanImageSkyModel.cc
 MeasurementComponents/DJones.cc
 MeasurementComponents/EJones.cc
 MeasurementComponents/EPJones.cc
 MeasurementComponents/EVLASwPow.cc
 MeasurementComponents/FJones.cc
 MeasurementComponents/GlobalFTMachineCallbacks.cc
 MeasurementComponents/GridBoth.cc
 MeasurementComponents/GSpline.cc
 MeasurementComponents/HogbomCleanImageSkyModel.cc
 TransformMachines/IlluminationConvFunc.cc
 MeasurementComponents/ImageSkyModel.cc
 MeasurementComponents/Jones.cc
 MeasurementComponents/KJones.cc
 MeasurementComponents/LJJones.cc
 MeasurementComponents/MakeApproxPSFAlgorithm.cc
 MeasurementComponents/MFCEMemImageSkyModel.cc
 MeasurementComponents/MFCleanImageSkyModel.cc
 MeasurementComponents/MFMSCleanImageSkyModel.cc
 MeasurementComponents/MSCleanImageSkyModel.cc
 MeasurementComponents/Mueller.cc
 MeasurementComponents/MultiThreadedVisResampler.cc
 MeasurementComponents/NNLSImageSkyModel.cc
 MeasurementComponents/nPBWProjectFT.cc
 MeasurementComponents/PBMosaicFT.cc
 MeasurementComponents/PClarkCleanImageSkyModel.cc
 MeasurementComponents/PixelatedConvFunc.cc
 MeasurementComponents/PredictAlgorithm.cc
 TransformMachines/PSTerm.cc
 MeasurementComponents/PWFCleanImageSkyModel.cc
 MeasurementComponents/ReadMSAlgorithm.cc
 MeasurementComponents/ResidualAlgorithm.cc
 MeasurementComponents/ResamplerWorklet.cc
 MeasurementComponents/SDGrid.cc
 MeasurementComponents/SimACohCalc.cc
 MeasurementComponents/SimACoh.cc
 MeasurementComponents/SolvableVisCal.cc
 MeasurementComponents/StandardVisCal.cc
 MeasurementComponents/SteepestDescentSolver.cc
 MeasurementComponents/TsysGainCal.cc
 MeasurementComponents/UVMod.cc
 MeasurementComponents/VisCal.cc
 MeasurementComponents/VisCalGlobals.cc
 MeasurementComponents/VisCalSolver.cc
 MeasurementComponents/VisVector.cc
 TransformMachines/VisibilityResamplerBase.cc
 TransformMachines/VisibilityResampler.cc
 TransformMachines/VLACalcIlluminationConvFunc.cc
 TransformMachines/VLAIlluminationConvFunc.cc
 MeasurementComponents/WBCleanImageSkyModel.cc
 MeasurementComponents/WFCleanImageSkyModel.cc
 TransformMachines/WOnlyConvFunc.cc
 TransformMachines/WTerm.cc
 MeasurementEquations/CCList.cc
 MeasurementEquations/CEMemModel.cc
 MeasurementEquations/CEMemProgress.cc
 MeasurementEquations/ClarkCleanLatModel.cc
 MeasurementEquations/ClarkCleanModel.cc
 MeasurementEquations/ClarkCleanProgress.cc
 MeasurementEquations/ConvolutionEquation.cc
 MeasurementEquations/CubeSkyEquation.cc
 MeasurementEquations/Deconvolver.cc
 MeasurementEquations/Entropy.cc
 MeasurementEquations/Feather.cc
 MeasurementEquations/Imager.cc
 MeasurementEquations/Imager2.cc
 MeasurementEquations/ImagerMultiMS.cc
 MeasurementEquations/ImageMSCleaner.cc
 MeasurementEquations/IncCEMemModel.cc
 MeasurementEquations/IncEntropy.cc
 MeasurementEquations/IPLatConvEquation.cc
 MeasurementEquations/LatConvEquation.cc
 MeasurementEquations/LatticeModel.cc
 MeasurementEquations/MatrixCleaner.cc
 MeasurementEquations/MultiTermMatrixCleaner.cc
 MeasurementEquations/Simulator.cc
 MeasurementEquations/SkyEquation.cc
 MeasurementEquations/StokesUtil2.cc
 MeasurementEquations/VisEquation.cc
 MeasurementEquations/VPManager.cc
 TransformMachines/ALMAAperture.cc
 TransformMachines/ALMABeamCalcParams.cc
 TransformMachines/ALMACalcIlluminationConvFunc.cc
 Parallel/Applicator.cc
 Parallel/MPIError.cc
 Parallel/MPITransport.cc
 Parallel/PabloIO.cc
 Parallel/SerialTransport.cc
 Utilities/FixVis.cc
 Utilities/SigHandler.cc
 Utilities/SpectralImageUtil.cc
 Utilities/ThreadCoordinator.cc
 fortran/fpbmos.f	
 fortran/fpbwproj.f	
 fortran/leakantso.f
 ImagerObjects/SynthesisImager.cc
 ImagerObjects/SIImageStore.cc
 ImagerObjects/SIImageStoreMultiTerm.cc
 ImagerObjects/SIMapperCollection.cc
 ImagerObjects/SIMapperBase.cc
 ImagerObjects/SIMapperSingle.cc
 ImagerObjects/SIMapperMultiTerm.cc
 ImagerObjects/SynthesisIterBot.cc
 ImagerObjects/SIMinorCycleController.cc
 ImagerObjects/SIIterBot.cc
 ImagerObjects/SynthesisDeconvolver.cc
 ImagerObjects/SDAlgorithmBase.cc
 ImagerObjects/SDAlgorithmHogbomClean.cc
 ImagerObjects/SDAlgorithmMSMFS.cc
 ImagerObjects/SDMaskHandler.cc
 ImagerObjects/SynthesisParSync.cc
 ImagerObjects/SynthesisUtilMethods.cc
  )

install (FILES
	CalTables/BJonesDesc.h
	CalTables/BJonesMBuf.h
	CalTables/BJonesMCol.h
	CalTables/BJonesMRec.h
	CalTables/BJonesTable.h
	CalTables/CLPatchPanel.h
	CalTables/CTColumns.h
	CalTables/CTDesc.h
	CalTables/CTEnums.h
	CalTables/CTInterface.h
	CalTables/CTMainColInterface.h
	CalTables/CTIter.h
	CalTables/CTMainColumns.h
	CalTables/CTMainRecord.h
	CalTables/CTPatchedInterp.h
	CalTables/CTTimeInterp1.h
	CalTables/CalBuffer.h
	CalTables/CalDescBuffer.h
	CalTables/CalDescColumns.h
	CalTables/CalDescColumns2.h
	CalTables/CalDescRecord.h
	CalTables/CalHistRecord.h
	CalTables/CalHistoryBuffer.h
	CalTables/CalHistoryColumns.h
	CalTables/CalInterp.h
	CalTables/CalInterpolation.h
	CalTables/CalInterpolator.h
	CalTables/CalIntpMatch.h
	CalTables/CalIter.h
	CalTables/CalIterBase.h
	CalTables/CalMainBuffer.h
	CalTables/CalMainColumns.h
	CalTables/CalMainColumns2.h
	CalTables/CalMainRecord.h
	CalTables/CalSet.h
	CalTables/CalSetMetaInfo.h
	CalTables/CalTable.h
	CalTables/CalTable2.h
	CalTables/CalTableDesc.h
	CalTables/CalTableDesc2.h
	CalTables/CalTableFromAIPS.h
	CalTables/EPointDesc.h
	CalTables/EPointMBuf.h
	CalTables/EPointMCol.h
	CalTables/EPointTable.h
	CalTables/EPointVJMRec.h
	CalTables/FVisJonesDesc.h
	CalTables/FVisJonesMRec.h
	CalTables/GJonesDesc.h
	CalTables/GJonesMBuf.h
	CalTables/GJonesMCol.h
	CalTables/GJonesTable.h
	CalTables/MJonesDesc.h
	CalTables/MJonesMRec.h
	CalTables/NewCalTable.h
	CalTables/RIorAParray.h
	CalTables/ROCalMainColumns2.h
	CalTables/SkyJonesDesc.h
	CalTables/SolvableCalSetMCol.h
	CalTables/SolvableMJDesc.h
	CalTables/SolvableMJMRec.h
	CalTables/SolvableVJDesc.h
	CalTables/SolvableVJMBuf.h
	CalTables/SolvableVJMCol.h
	CalTables/SolvableVJMRec.h
	CalTables/SolvableVJTable.h
	CalTables/TJonesDesc.h
	CalTables/TJonesMBuf.h
	CalTables/TJonesMCol.h
	CalTables/TJonesTable.h
	CalTables/TimeVarMJDesc.h
	CalTables/TimeVarMJMRec.h
	CalTables/TimeVarVJDesc.h
	CalTables/TimeVarVJMBuf.h
	CalTables/TimeVarVJMCol.h
	CalTables/TimeVarVJMRec.h
	CalTables/TimeVarVJTable.h
	CalTables/VisCalEnum.h
	CalTables/VisJonesDesc.h
	CalTables/VisJonesMBuf.h
	CalTables/VisJonesMCol.h
	CalTables/VisJonesMRec.h
	CalTables/VisJonesTable.h
	CalTables/CalMainColumns2.tcc
	CalTables/CalSet.tcc
	CalTables/ROCalMainColumns2.tcc
	CalTables/SolvableCalSetMCol.tcc
	DESTINATION include/casacode/synthesis/CalTables
	)
install (FILES
	DataSampling/DataSampling.h
	DataSampling/ImageDataSampling.h
	DataSampling/PixonProcessor.h
	DataSampling/SDDataSampling.h
	DataSampling/SynDataSampling.h
	DESTINATION include/casacode/synthesis/DataSampling
	)
install (FILES
	IDL/IDL.h
	DESTINATION include/casacode/synthesis/IDL
	)
install (FILES
	MSVis/AsynchronousInterface.h
	MSVis/AsynchronousInterface2.h
	MSVis/AsynchronousTools.h
	MSVis/CalVisBuffer.h
	MSVis/GroupProcessor.h
	MSVis/GroupWorker.h
        MSVis/MeasurementSet2.h
	MSVis/MSAnalysis.h
	MSVis/MSCalEnums.h
	MSVis/MSContinuumSubtractor.h
	MSVis/MSMoments.h
	MSVis/MSUVWGenerator.h
	MSVis/MSUtil.h
	MSVis/MsAverager.h
	MSVis/Partition.h
	MSVis/Reweighter.h
	MSVis/SelectAverageSpw.h
	MSVis/SimpleSubMS.h
        MSVis/SpectralWindow.h
	MSVis/StatWT.h
	MSVis/StokesVector.h
	MSVis/SubMS.h
	MSVis/UtilJ.h
	MSVis/VBContinuumSubtractor.h
	MSVis/VBGContinuumSubtractor.h
	MSVis/VBRemapper.h
	MSVis/VLAT.h
	MSVis/VWBT.h
	MSVis/VisBuffAccumulator.h
	MSVis/VisBuffGroup.h
	MSVis/VisBuffGroupAcc.h
	MSVis/VisBuffer.h
	MSVis/VisBufferAsync.h
	MSVis/VisBufferAsyncWrapper.h
	MSVis/VisBufferComponents.h
	MSVis/VisBufferUtil.h
	MSVis/VisChunkAverager.h
	MSVis/VisImagingWeight.h
	MSVis/VisIterator.h
	MSVis/VisSet.h
	MSVis/VisSetUtil.h
	MSVis/VisibilityIterator.h
	MSVis/VisibilityIteratorImpl.h
	MSVis/VisibilityIteratorImplAsync.h
        MSVis/AsynchronousInterface2.h
        MSVis/VisBuffer2Adapter.h
        MSVis/VisBuffer2.h
        MSVis/VisBufferAsyncWrapper2.h
        MSVis/VisBufferComponents2.h
        MSVis/VisBufferImpl2.h
        MSVis/VisBufferImplAsync2.h
        MSVis/ViColumns2.h
        MSVis/ViFrequencySelection.h
        MSVis/ViImplementation2.h
        MSVis/VisibilityIterator2.h
        MSVis/VisibilityIteratorImpl2.h
        MSVis/VisibilityIteratorImplAsync2.h
        MSVis/TransformingVi2.h
	MSVis/AveragingVi2Factory.h
	MSVis/AveragingTvi2.h
	MSVis/FinalTvi2.h
	MSVis/VisibilityProcessing.h
	MSVis/MSMoments.tcc
	MSVis/VisBuffer.tcc
	MSVis/VisibilityIterator.tcc
	DESTINATION include/casacode/synthesis/MSVis
	)
install (FILES
	MeasurementComponents/ACoh.h
	MeasurementComponents/AMueller.h
	MeasurementComponents/BPoly.h
	TransformMachines/BeamCalc.h
	MeasurementComponents/CEMemImageSkyModel.h
	MeasurementComponents/CExp.h
	MeasurementComponents/CExp.new3.h
#	MeasurementComponents/CFDefs.h
	MeasurementComponents/CSCleanImageSkyModel.h
	MeasurementComponents/CalCorruptor.h
	MeasurementComponents/Calibrater.h
	MeasurementComponents/ClarkCleanAlgorithm.h
	MeasurementComponents/ClarkCleanImageSkyModel.h
	MeasurementComponents/CleanImageSkyModel.h
	MeasurementComponents/ConvFuncDiskCache.h
	MeasurementComponents/EJones.h
	MeasurementComponents/EPJones.h
	MeasurementComponents/ExpCache.h
	MeasurementComponents/FJones.h
	MeasurementComponents/GSpline.h
	MeasurementComponents/GlobalFTMachineCallbacks.h
	MeasurementComponents/GridBoth.h
	MeasurementComponents/HogbomCleanImageSkyModel.h
	MeasurementComponents/ImageSkyModel.h
	MeasurementComponents/Jones.h
	MeasurementComponents/KJones.h
	MeasurementComponents/LJJones.h
	MeasurementComponents/MFCEMemImageSkyModel.h
	MeasurementComponents/MFCleanImageSkyModel.h
	MeasurementComponents/MFMSCleanImageSkyModel.h
	MeasurementComponents/MSCleanImageSkyModel.h
	MeasurementComponents/MThWorkIDEnum.h
	MeasurementComponents/MakeApproxPSFAlgorithm.h
	MeasurementComponents/Mueller.h
	MeasurementComponents/MultiThreadedVisResampler.h
	MeasurementComponents/NNLSImageSkyModel.h
	MeasurementComponents/PBMosaicFT.h
	MeasurementComponents/PClarkCleanImageSkyModel.h
	MeasurementComponents/PWFCleanImageSkyModel.h
	MeasurementComponents/PixelatedConvFunc.h
	MeasurementComponents/PredictAlgorithm.h
	MeasurementComponents/ReadMSAlgorithm.h
	MeasurementComponents/ResamplerWorklet.h
	MeasurementComponents/ResidualAlgorithm.h
	MeasurementComponents/SDGrid.h
	MeasurementComponents/SimACoh.h
	MeasurementComponents/SimACohCalc.h
	MeasurementComponents/SkyModel.h
	MeasurementComponents/SolvableVisCal.h
	MeasurementComponents/StandardVisCal.h
	MeasurementComponents/SteepestDescentSolver.h
	MeasurementComponents/TabulatedConvFunc.h
	MeasurementComponents/TsysGainCal.h
	MeasurementComponents/UVMod.h
	MeasurementComponents/VisCal.h
	MeasurementComponents/VisCalGlobals.h
	MeasurementComponents/VisCalSolver.h
	MeasurementComponents/VisVector.h
	MeasurementComponents/WBCleanImageSkyModel.h
	MeasurementComponents/WFCleanImageSkyModel.h
	MeasurementComponents/WOnlyProjectFT.h
	MeasurementComponents/XCorr.h
	MeasurementComponents/nPBWProjectFT.h
	MeasurementComponents/rGridFT.h
	MeasurementComponents/ImageSkyModel.tcc
	DESTINATION include/casacode/synthesis/MeasurementComponents
	)
install (FILES
	MeasurementEquations/ArrayModel.h
	MeasurementEquations/CCList.h
	MeasurementEquations/CEMemModel.h
	MeasurementEquations/CEMemProgress.h
	MeasurementEquations/ClarkCleanLatModel.h
	MeasurementEquations/ClarkCleanModel.h
	MeasurementEquations/ClarkCleanProgress.h
	MeasurementEquations/ConvolutionEquation.h
	MeasurementEquations/CubeSkyEquation.h
	MeasurementEquations/Deconvolver.h
	MeasurementEquations/Entropy.h
	MeasurementEquations/HogbomCleanModel.h
	MeasurementEquations/IPLatConvEquation.h
	MeasurementEquations/ImageMSCleaner.h
	MeasurementEquations/Imager.h
	MeasurementEquations/ImagerMultiMS.h
	MeasurementEquations/IncCEMemModel.h
	MeasurementEquations/IncEntropy.h
	MeasurementEquations/Iterate.h
	MeasurementEquations/LatConvEquation.h
	MeasurementEquations/LatticeModel.h
	MeasurementEquations/LinearEquation.h
	MeasurementEquations/LinearModel.h
	MeasurementEquations/MaskedArrayModel.h
	MeasurementEquations/MaskedHogbomCleanModel.h
	MeasurementEquations/MatrixCleaner.h
	MeasurementEquations/MultiTermMatrixCleaner.h
	MeasurementEquations/ResidualEquation.h
	MeasurementEquations/Simulator.h
	MeasurementEquations/SkyEquation.h
	MeasurementEquations/StokesUtil.h
	MeasurementEquations/VPManager.h
	MeasurementEquations/VisEquation.h
	MeasurementEquations/ArrayModel.tcc
	MeasurementEquations/HogbomCleanModel.tcc
	MeasurementEquations/LinearEquation.tcc
	MeasurementEquations/LinearModel.tcc
	MeasurementEquations/MaskedArrayModel.tcc
	MeasurementEquations/MaskedHogbomCleanModel.tcc
	MeasurementEquations/ResidualEquation.tcc
	MeasurementEquations/StokesUtil.tcc
	DESTINATION include/casacode/synthesis/MeasurementEquations
	)
install (FILES
	ImagerObjects/SynthesisImager.h
	ImagerObjects/SIImageStore.h
	ImagerObjects/SIImageStoreMultiTerm.h
	ImagerObjects/SIMapperCollection.h
	ImagerObjects/SIMapperBase.h
	ImagerObjects/SIMapperSingle.h
	ImagerObjects/SIMapperMultiTerm.h
	ImagerObjects/SynthesisIterBot.h
	ImagerObjects/SIMinorCycleController.h
	ImagerObjects/SIIterBot.h
	ImagerObjects/SynthesisDeconvolver.h
	ImagerObjects/SDMaskHandler.h
	ImagerObjects/SynthesisParSync.h
	ImagerObjects/SynthesisUtilMethods.h
	ImagerObjects/SDAlgorithmBase.h
	ImagerObjects/SDAlgorithmHogbomClean.h
	ImagerObjects/SDAlgorithmMSMFS.h
	DESTINATION include/casacode/synthesis/ImagerObjects
	)
install (FILES
	Parallel/Algorithm.h
	Parallel/Applicator.h
	Parallel/MPIError.h
	Parallel/PTransport.h
	Parallel/PabloIO.h
	DESTINATION include/casacode/synthesis/Parallel
	)
install (FILES
	TransformMachines/BeamSkyJones.h
	TransformMachines/BeamSquint.h
	TransformMachines/CFCache.h
	TransformMachines/CFDefs.h
	TransformMachines/CFStore.h
	TransformMachines/CFStore2.h
	TransformMachines/CFBuffer.h
	TransformMachines/CFCell.h
	TransformMachines/PolOuterProduct.h
	TransformMachines/ComponentFTMachine.h
	TransformMachines/ConvolutionFunction.h
	TransformMachines/DBeamSkyJones.h
	TransformMachines/CFTerms.h
	TransformMachines/PSTerm.h
	TransformMachines/WTerm.h
	TransformMachines/FTMachine.h
	TransformMachines/GridFT.h
	TransformMachines/HetArrayConvFunc.h
	TransformMachines/MosaicFT.h
	TransformMachines/MultiTermFT.h
	TransformMachines/NewMultiTermFT.h
	TransformMachines/PBMath.h
	TransformMachines/PBMath1D.h
	TransformMachines/PBMath1DAiry.h
	TransformMachines/PBMath1DCosPoly.h
	TransformMachines/PBMath1DGauss.h
	TransformMachines/PBMath1DIPoly.h
	TransformMachines/PBMath1DNumeric.h
	TransformMachines/PBMath1DPoly.h
	TransformMachines/PBMath2D.h
	TransformMachines/PBMath2DImage.h
	TransformMachines/PBMathInterface.h
	TransformMachines/SimpCompGridMachine.h
	TransformMachines/SimpComponentGridMachine.h
	TransformMachines/SimpleComponentFTMachine.h
	TransformMachines/SimplePBConvFunc.h
	TransformMachines/SkyJones.h
	TransformMachines/StokesImageUtil.h
	TransformMachines/SynthesisError.h
	TransformMachines/Utils.h
	TransformMachines/VPSkyJones.h
	TransformMachines/VisModelData.h
	TransformMachines/WPConvFunc.h
	TransformMachines/WProjectFT.h
        TransformMachines/ATerm.h
        TransformMachines/NoOpATerm.h
        TransformMachines/AWConvFunc.h
        TransformMachines/AWConvFuncEPJones.h
        TransformMachines/AWProjectFT.h
        TransformMachines/AWProjectWBFT.h
	TransformMachines/AWVisResampler.h
	TransformMachines/AzElAperture.h
	TransformMachines/EVLAAperture.h
	TransformMachines/EVLAConvFunc.h
	TransformMachines/IlluminationConvFunc.h
	TransformMachines/VBStore.h
	TransformMachines/VLACalcIlluminationConvFunc.h
	TransformMachines/VLAIlluminationConvFunc.h
	TransformMachines/VisibilityResampler.h
	TransformMachines/VisibilityResamplerBase.h
	TransformMachines/WOnlyConvFunc.h
	TransformMachines/ALMAAperture.h
	TransformMachines/ALMACalcIlluminationConvFunc.h
	DESTINATION include/casacode/synthesis/TransformMachines
	)
install (FILES
	Utilities/AppRC.h
	Utilities/FixVis.h
	Utilities/ThreadCoordinator.h
        Utilities/SpectralImageUtil.h
	DESTINATION include/casacode/synthesis/Utilities
	)
casa_add_executable( synthesis casasplit apps/casasplit/casasplit.cc )
casa_add_executable( synthesis imageconcat apps/utils/imageconcat.cc )
casa_add_assay( synthesis ImagerObjects/test/tSIIterBot.cc )
casa_add_assay( synthesis ImagerObjects/test/tSynthesisImager.cc )
casa_add_assay( synthesis MSVis/test/tHanningSmooth.cc )
casa_add_assay( synthesis MSVis/test/tMSCalEnums.cc )
casa_add_assay( synthesis MSVis/test/tUVSub.cc )
casa_add_assay( synthesis MSVis/test/tVisibilityIterator.cc )
casa_add_assay( synthesis MSVis/test/tVisibilityIteratorAsync.cc )
casa_add_assay( synthesis MSVis/test/VisibilityProcessing_Test.cc )
casa_add_assay( synthesis MSVis/test/AveragingTvi2_Test.cc MSVis/test/MsFactory.cc )
casa_add_assay( synthesis MSVis/test/VisibilityIterator_Test.cc MsFactory.cc )
casa_add_assay( synthesis MSVis/test/tVisIter.cc )
casa_add_assay( synthesis TransformMachines/test/tVisModelData.cc )
casa_add_assay( synthesis TransformMachines/test/tStokesImageUtil.cc )
#casa_add_assay( synthesis TransformMachines/test/tCImageRotation.cc )
casa_add_assay( synthesis DataSampling/test/tPixonProcessor.cc )
casa_add_assay( synthesis IDL/test/tIDL.cc )
casa_add_assay( synthesis MeasurementComponents/test/tALMAAperture.cc )
casa_add_assay( synthesis MeasurementComponents/test/tEVLAAperture.cc )
casa_add_assay( synthesis MeasurementComponents/test/tAlgoPClark.cc )
casa_add_assay( synthesis MeasurementComponents/test/tBeamSquint.cc )
casa_add_assay( synthesis MeasurementComponents/test/tCalibrater.cc )
casa_add_assay( synthesis MeasurementComponents/test/tFJones.cc )
casa_add_assay( synthesis MeasurementComponents/test/tKJones.cc )
casa_add_assay( synthesis MeasurementComponents/test/tPBMath.cc )
casa_add_assay( synthesis MeasurementComponents/test/tPBMath2D.cc )
casa_add_assay( synthesis MeasurementComponents/test/tPBTable.cc )
casa_add_assay( synthesis MeasurementComponents/test/tVPSkyJones.cc )
casa_add_assay( synthesis MeasurementEquations/test/dCEMemModel.cc )
casa_add_assay( synthesis MeasurementEquations/test/tCCList.cc )
casa_add_assay( synthesis MeasurementEquations/test/tCEMemModel.cc )
casa_add_assay( synthesis MeasurementEquations/test/tClarkCleanLatModel.cc )
casa_add_assay( synthesis MeasurementEquations/test/tClarkCleanModel.cc )
casa_add_assay( synthesis MeasurementEquations/test/tConvolutionEquation.cc )
casa_add_assay( synthesis MeasurementEquations/test/tFeather.cc )
casa_add_assay( synthesis MeasurementEquations/test/tImager.cc )
casa_add_assay( synthesis MeasurementEquations/test/tIncCEMemModel.cc )
casa_add_assay( synthesis MeasurementEquations/test/tLatConvEquation.cc )
casa_add_assay( synthesis MeasurementEquations/test/tStokesUtil.cc )
casa_add_assay( synthesis Parallel/test/tApplicator.cc )
casa_add_assay( synthesis CalTables/test/tCalInterpolation.cc )
casa_add_assay( synthesis CalTables/test/tCalIntpMatch.cc )
casa_add_assay( synthesis CalTables/test/tNewCalTable.cc )
casa_add_assay( synthesis CalTables/test/tCTIter.cc )
casa_add_assay( synthesis CalTables/test/tRIorAPArray.cc )
casa_add_assay( synthesis CalTables/test/tCLPatchPanel.cc )
casa_add_assay( synthesis CalTables/test/tCTTimeInterp1.cc )
casa_add_assay( synthesis CalTables/test/tCTPatchedInterp.cc )
casa_add_assay( synthesis CalTables/test/tCTSelection.cc )
casa_add_assay( synthesis CalTables/test/tCTGenericFill.cc )
