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
  implement/MSVis/CalVisBuffer.cc
  implement/MSVis/MsAverager.cc
  implement/MSVis/MSCalEnums.cc
  implement/MSVis/MSContinuumSubtractor.cc
  implement/MSVis/MSUVWGenerator.cc
  implement/MSVis/SelectAverageSpw.cc
  implement/MSVis/StokesVector.cc
  implement/MSVis/SubMS.cc
  implement/MSVis/VisBuffAccumulator.cc
  implement/MSVis/VisBuffer.cc
  implement/MSVis/VisBufferUtil.cc
  implement/MSVis/VisBuffGroupAcc.cc
  implement/MSVis/VisibilityIterator.cc
  implement/MSVis/VisImagingWeight.cc
  implement/MSVis/VisIterator.cc
  implement/MSVis/VisSet.cc
  implement/MSVis/VisSetUtil.cc
  implement/MSVis/VisTimeAverager.cc
   )

casa_add_executable( msvis casasplit apps/casasplit/casasplit.cc )
casa_add_assay( msvis implement/MSVis/test/tHanningSmooth.cc )
casa_add_assay( msvis implement/MSVis/test/tMSCalEnums.cc )
casa_add_assay( msvis implement/MSVis/test/tUVSub.cc )
casa_add_assay( msvis implement/MSVis/test/tVisibilityIterator.cc )
casa_add_assay( msvis implement/MSVis/test/tVisIter.cc )
