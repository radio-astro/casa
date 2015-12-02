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

casa_add_library( nrao
  VLA/Get_num_attached.cc
  VLA/nreal.cc
  VLA/VLAADA.cc
  VLA/VLAArchiveInput.cc
  VLA/VLABaselineRecord.cc
  VLA/VLACalibratorFilter.cc
  VLA/VLACDA.cc
  VLA/VLAContinuumRecord.cc
  VLA/VLADiskInput.cc
  VLA/VLAEnum.cc
  VLA/VLAFiller.cc
  VLA/VLAFilter.cc
  VLA/VLAFilterSet.cc
  VLA/VLAFrequencyFilter.cc
  VLA/VLALogicalRecord.cc
  VLA/VLAObsModeFilter.cc
  VLA/VLAOnlineInput.cc
  VLA/VLAProjectFilter.cc
  VLA/VLARCA.cc
  VLA/VLASDA.cc
  VLA/VLASourceFilter.cc
  VLA/VLASpectralLineRecord.cc
  VLA/VLASubarrayFilter.cc
  VLA/VLATapeInput.cc
  VLA/VLATimeFilter.cc
   )

install (FILES
	VLA/VLAADA.h
	VLA/VLAArchiveInput.h
	VLA/VLABaselineRecord.h
	VLA/VLACDA.h
	VLA/VLACalibratorFilter.h
	VLA/VLAContinuumRecord.h
	VLA/VLADiskInput.h
	VLA/VLAEnum.h
	VLA/VLAFiller.h
	VLA/VLAFilter.h
	VLA/VLAFilterSet.h
	VLA/VLAFrequencyFilter.h
	VLA/VLALogicalRecord.h
	VLA/VLAObsModeFilter.h
	VLA/VLAOnlineInput.h
	VLA/VLAProjectFilter.h
	VLA/VLARCA.h
	VLA/VLASDA.h
	VLA/VLASourceFilter.h
	VLA/VLASpectralLineRecord.h
	VLA/VLASubarrayFilter.h
	VLA/VLATapeIO.h
	VLA/VLATapeInput.h
	VLA/VLATimeFilter.h
	VLA/nreal.h
	DESTINATION include/casacode/nrao/VLA
	)

casa_add_demo( nrao VLA/test/dVLAArchiveInput.cc )
casa_add_assay( nrao VLA/test/tVLABuffer.cc )
casa_add_assay( nrao VLA/test/tVLADiskInput.cc )
casa_add_assay( nrao VLA/test/tVLALogicalRecord.cc )
casa_add_assay( nrao VLA/test/tVLATapeInput.cc )
