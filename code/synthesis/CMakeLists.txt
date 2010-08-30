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
  implement/MeasurementEquations/ImageMSCleaner.cc		
  implement/MeasurementEquations/IncCEMemModel.cc
  implement/MeasurementEquations/IncEntropy.cc
  implement/MeasurementEquations/IPLatConvEquation.cc
  implement/MeasurementEquations/LatConvEquation.cc
  implement/MeasurementEquations/LatticeModel.cc
  implement/MeasurementEquations/MatrixCleaner.cc	
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
  implement/fortran/wprojgrid.f  )

casa_add_assay( synthesis implement/DataSampling/test/tPixonProcessor.cc )
casa_add_assay( synthesis implement/IDL/test/tIDL.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tAlgoPClark.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tBeamSquint.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tCalibrater.cc )
casa_add_assay( synthesis implement/MeasurementComponents/test/tPBMath.cc )
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
