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
  MSVis/AsynchronousInterface.cc
  MSVis/AsynchronousTools.cc
  MSVis/AveragingTvi2.cc
  MSVis/AveragingVi2Factory.cc
  MSVis/CalVisBuffer.cc
  MSVis/FinalTvi2.cc
  MSVis/GroupProcessor.cc
  MSVis/GroupWorker.cc
  MSVis/MSAnalysis.cc
  MSVis/MSCalEnums.cc
  MSVis/MSContinuumSubtractor.cc
  MSVis/MSUVWGenerator.cc
  MSVis/MSUtil.cc
  MSVis/MeasurementSet2.cc
  MSVis/MsAverager.cc
  MSVis/MsRows.cc
  MSVis/Partition.cc
  MSVis/Reweighter.cc
  MSVis/SelectAverageSpw.cc
  MSVis/SimpleSubMS.cc
  MSVis/SpectralWindow.cc
  MSVis/StatWT.cc
  MSVis/StokesVector.cc
  MSVis/SubMS.cc
  MSVis/TransformingVi2.cc
  MSVis/UtilJ.cc
  MSVis/VBContinuumSubtractor.cc
  MSVis/VBGContinuumSubtractor.cc
  MSVis/VBRemapper.cc
  MSVis/VLAT.cc
  MSVis/VWBT.cc
  MSVis/Vbi2MsRow.cc
  MSVis/ViColumns2.cc
  MSVis/ViFrequencySelection.cc
  MSVis/ViImplementation2.cc
  MSVis/VisBuffAccumulator.cc
  MSVis/VisBuffGroup.cc
  MSVis/VisBuffGroupAcc.cc
  MSVis/VisBuffer.cc
  MSVis/VisBuffer2.cc
  MSVis/VisBufferAsync.cc
  MSVis/VisBufferAsyncWrapper.cc
  MSVis/VisBufferComponents2.cc
  MSVis/VisBufferImpl2.cc
  MSVis/VisBufferUtil.cc
  MSVis/VisChunkAverager.cc
  MSVis/VisImagingWeight.cc
  MSVis/VisIterator.cc
  MSVis/VisModelDataI.cc
  MSVis/VisSet.cc
  MSVis/VisSetUtil.cc
  MSVis/VisibilityIterator.cc
  MSVis/VisibilityIterator2.cc
  MSVis/VisibilityIteratorImpl.cc
  MSVis/VisibilityIteratorImpl2.cc
  MSVis/VisibilityIteratorImplAsync.cc
  MSVis/VisibilityProcessing.cc 
####   MSVis/AsynchronousInterface2.cc
####  MSVis/VisBufferAsyncWrapper2.cc
####  MSVis/VisBufferImplAsync2.cc
####  MSVis/VisibilityIteratorImplAsync2.cc
  )

install (FILES
	MSVis/AsynchronousInterface.h
	MSVis/AsynchronousInterface2.h
	MSVis/AsynchronousTools.h
	MSVis/AveragingTvi2.h
	MSVis/AveragingVi2Factory.h
	MSVis/CalVisBuffer.h
	MSVis/FinalTvi2.h
	MSVis/GroupProcessor.h
	MSVis/GroupWorker.h
	MSVis/MSAnalysis.h
	MSVis/MSCalEnums.h
	MSVis/MSContinuumSubtractor.h
	MSVis/MSMoments.h
	MSVis/MSMoments.tcc
	MSVis/MSUVWGenerator.h
	MSVis/MSUtil.h
	MSVis/MsAverager.h
	MSVis/Partition.h
	MSVis/Reweighter.h
	MSVis/SelectAverageSpw.h
	MSVis/SimpleSubMS.h
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
	MSVis/VisBuffer.tcc
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
	MSVis/VisibilityIterator.tcc
	MSVis/VisibilityIteratorImpl.h
	MSVis/VisibilityIteratorImplAsync.h
	MSVis/VisibilityProcessing.h
        MSVis/AsynchronousInterface2.h
        MSVis/MeasurementSet2.h
        MSVis/SpectralWindow.h
        MSVis/TransformingVi2.h
        MSVis/ViColumns2.h
        MSVis/ViFrequencySelection.h
        MSVis/ViImplementation2.h
        MSVis/VisBuffer2.h
        MSVis/VisBuffer2Adapter.h
        MSVis/VisBufferAsyncWrapper2.h
        MSVis/VisBufferComponents2.h
        MSVis/VisBufferImpl2.h
        MSVis/VisBufferImplAsync2.h
        MSVis/VisModelDataI.cc
        MSVis/VisibilityIterator2.h
        MSVis/VisibilityIteratorImpl2.h
        MSVis/VisibilityIteratorImplAsync2.h
	DESTINATION include/casacode/synthesis/MSVis
	)

casa_add_assay( synthesis MSVis/test/tHanningSmooth.cc )
casa_add_assay( synthesis MSVis/test/tMSCalEnums.cc )
casa_add_assay( synthesis MSVis/test/tUVSub.cc )
casa_add_assay( synthesis MSVis/test/tVisibilityIterator.cc )
casa_add_assay( synthesis MSVis/test/tVisibilityIteratorAsync.cc )
casa_add_assay( synthesis MSVis/test/AveragingTvi2_Test.cc MSVis/test/MsFactory.cc )
casa_add_assay( synthesis MSVis/test/VisibilityIterator_Test.cc MSVis/test/MsFactory.cc )
casa_add_assay( synthesis MSVis/test/tVisIter.cc )
