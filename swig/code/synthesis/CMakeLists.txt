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
 implement/MeasurementComponents/rGridFT.cc
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
