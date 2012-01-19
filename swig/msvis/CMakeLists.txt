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

casa_add_library( msvis
  implement/SynthesisUtils/BeamSkyJones.cc
  implement/SynthesisUtils/BeamSquint.cc
  implement/SynthesisUtils/CFCache.cc
  implement/SynthesisUtils/CFStore.cc
  implement/SynthesisUtils/ComponentFTMachine.cc
  implement/SynthesisUtils/ConvolutionFunction.cc
  implement/SynthesisUtils/DBeamSkyJones.cc
  implement/SynthesisUtils/FTMachine.cc
  implement/SynthesisUtils/GridFT.cc
  implement/SynthesisUtils/HetArrayConvFunc.cc
  implement/SynthesisUtils/MosaicFT.cc
  implement/SynthesisUtils/MultiTermFT.cc	
  implement/SynthesisUtils/PBMath1DAiry.cc
  implement/SynthesisUtils/PBMath1D.cc
  implement/SynthesisUtils/PBMath1DCosPoly.cc
  implement/SynthesisUtils/PBMath1DGauss.cc
  implement/SynthesisUtils/PBMath1DIPoly.cc
  implement/SynthesisUtils/PBMath1DNumeric.cc
  implement/SynthesisUtils/PBMath1DPoly.cc
  implement/SynthesisUtils/PBMath2D.cc
  implement/SynthesisUtils/PBMath2DImage.cc
  implement/SynthesisUtils/PBMath.cc
  implement/SynthesisUtils/PBMathInterface.cc
  implement/SynthesisUtils/SimpCompGridMachine.cc
  implement/SynthesisUtils/SimpComponentGridMachine.cc
  implement/SynthesisUtils/SimpleComponentFTMachine.cc
  implement/SynthesisUtils/SimplePBConvFunc.cc
  implement/SynthesisUtils/SkyJones.cc
  implement/SynthesisUtils/StokesImageUtil.cc
  implement/SynthesisUtils/SynthesisError.cc
  implement/SynthesisUtils/Utils.cc
  implement/SynthesisUtils/VisModelData.cc
  implement/SynthesisUtils/VPSkyJones.cc
  implement/SynthesisUtils/WProjectFT.cc
  implement/SynthesisUtils/WPConvFunc.cc
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
   )

casa_add_executable( msvis casasplit apps/casasplit/casasplit.cc )
casa_add_assay( msvis implement/MSVis/test/tHanningSmooth.cc )
casa_add_assay( msvis implement/MSVis/test/tMSCalEnums.cc )
casa_add_assay( msvis implement/MSVis/test/tUVSub.cc )
casa_add_assay( msvis implement/MSVis/test/tVisibilityIterator.cc )
casa_add_assay( msvis implement/MSVis/test/tVisibilityIteratorAsync.cc )
casa_add_assay( msvis implement/MSVis/test/VisibilityProcessing_Test.cc )
casa_add_assay( msvis implement/MSVis/test/tVisIter.cc )
casa_add_assay( msvis implement/SynthesisUtils/test/tVisModelData.cc )
