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
  FITS/GBTACSTable.cc
  FITS/GBTAntennaFile.cc
  FITS/GBTBackendTable.cc
  FITS/GBTDAPFile.cc
  FITS/GBTFITSBase.cc
  FITS/GBTGOFile.cc
  FITS/GBTLO1File.cc
  FITS/GBTMultiACSTable.cc
  FITS/GBTSampler.cc
  FITS/GBTScanLogReader.cc
  FITS/GBTStateTable.cc
  FITS/OldGBTPositionTable.cc
  GBTFillers/GBTACSFiller.cc
  GBTFillers/GBTAntennaDAPFiller.cc
  GBTFillers/GBTBackendFiller.cc
  GBTFillers/GBTCorrelation.cc
  GBTFillers/GBTDAPFillerBase.cc
  GBTFillers/GBTDAPFiller.cc
  GBTFillers/GBTDCRFiller.cc
  GBTFillers/GBTFeed.cc
  GBTFillers/GBTFeedDDFiller.cc
  GBTFillers/GBTFeedInfo.cc
  GBTFillers/GBTGOFiller.cc
  GBTFillers/GBTHoloFiller.cc
  GBTFillers/GBTIF.cc
  GBTFillers/GBTIFFiller.cc
  GBTFillers/GBTIFLOHelper.cc
  GBTFillers/GBTLO1DAPFiller.cc
  GBTFillers/GBTMeanFocusFiller.cc
  GBTFillers/GBTMSAntennaFiller.cc
  GBTFillers/GBTMSDataDescFiller.cc
  GBTFillers/GBTMSFeedFiller.cc
  GBTFillers/GBTMSFieldFiller.cc
  GBTFillers/GBTMSFillOptions.cc
  GBTFillers/GBTMSHistoryFiller.cc
  GBTFillers/GBTMSObservationFiller.cc
  GBTFillers/GBTMSPointingFiller.cc
  GBTFillers/GBTMSPolarizationFiller.cc
  GBTFillers/GBTMSProcessorFiller.cc
  GBTFillers/GBTMSSourceFiller.cc
  GBTFillers/GBTMSSpecWinFiller.cc
  GBTFillers/GBTMSStateFiller.cc
  GBTFillers/GBTMSSysCalFiller.cc
  GBTFillers/GBTMSTypeDataFiller.cc
  GBTFillers/GBTMSTypeFiller.cc
  GBTFillers/GBTMSTypeRecFiller.cc
  GBTFillers/GBTMSTypeStateFiller.cc
  GBTFillers/GBTMSWeatherCopier.cc
  GBTFillers/GBTMSWeatherFiller.cc
  GBTFillers/GBTPointModelFiller.cc
  GBTFillers/GBTPolarization.cc
  GBTFillers/GBTRcvrCalFiller.cc
  GBTFillers/GBTSimpleTable.cc
  GBTFillers/GBTSPFiller.cc
  SDD/SDD12mOnLine.cc
  SDD/SDDBlock.cc
  SDD/SDDBootStrap.cc
  SDD/SDDFile.cc
  SDD/SDDHeader.cc
  SDD/SDDIndex.cc
  SDD/SDDOnLine.cc
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
	FITS/GBTACSTable.h
	FITS/GBTAntennaFile.h
	FITS/GBTBackendTable.h
	FITS/GBTDAPFile.h
	FITS/GBTFITSBase.h
	FITS/GBTGOFile.h
	FITS/GBTLO1File.h
	FITS/GBTMultiACSTable.h
	FITS/GBTSampler.h
	FITS/GBTScanLogReader.h
	FITS/GBTStateTable.h
	FITS/OldGBTBackendTable.h
	FITS/OldGBTPositionTable.h
	DESTINATION include/casacode/nrao/FITS
	)
install (FILES
	GBTFillers/GBTACSFiller.h
	GBTFillers/GBTAntennaDAPFiller.h
	GBTFillers/GBTBackendFiller.h
	GBTFillers/GBTCorrelation.h
	GBTFillers/GBTDAPFiller.h
	GBTFillers/GBTDAPFillerBase.h
	GBTFillers/GBTDCRFiller.h
	GBTFillers/GBTFeed.h
	GBTFillers/GBTFeedDDFiller.h
	GBTFillers/GBTFeedInfo.h
	GBTFillers/GBTGOFiller.h
	GBTFillers/GBTHoloFiller.h
	GBTFillers/GBTIF.h
	GBTFillers/GBTIFFiller.h
	GBTFillers/GBTIFLOHelper.h
	GBTFillers/GBTLO1DAPFiller.h
	GBTFillers/GBTMSAntennaFiller.h
	GBTFillers/GBTMSDataDescFiller.h
	GBTFillers/GBTMSFeedFiller.h
	GBTFillers/GBTMSFieldFiller.h
	GBTFillers/GBTMSFillOptions.h
	GBTFillers/GBTMSHistoryFiller.h
	GBTFillers/GBTMSObservationFiller.h
	GBTFillers/GBTMSPointingFiller.h
	GBTFillers/GBTMSPolarizationFiller.h
	GBTFillers/GBTMSProcessorFiller.h
	GBTFillers/GBTMSSourceFiller.h
	GBTFillers/GBTMSSpecWinFiller.h
	GBTFillers/GBTMSStateFiller.h
	GBTFillers/GBTMSSysCalFiller.h
	GBTFillers/GBTMSTypeDataFiller.h
	GBTFillers/GBTMSTypeFiller.h
	GBTFillers/GBTMSTypeRecFiller.h
	GBTFillers/GBTMSTypeStateFiller.h
	GBTFillers/GBTMSWeatherCopier.h
	GBTFillers/GBTMSWeatherFiller.h
	GBTFillers/GBTMeanFocusFiller.h
	GBTFillers/GBTPointModelFiller.h
	GBTFillers/GBTPolarization.h
	GBTFillers/GBTRcvrCalFiller.h
	GBTFillers/GBTSPFiller.h
	GBTFillers/GBTSimpleTable.h
	DESTINATION include/casacode/nrao/GBTFillers
	)
install (FILES
	SDD/SDD12mOnLine.h
	SDD/SDDBlock.h
	SDD/SDDBootStrap.h
	SDD/SDDFile.h
	SDD/SDDHeader.h
	SDD/SDDIndex.h
	SDD/SDDOnLine.h
	DESTINATION include/casacode/nrao/SDD
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

casa_add_assay( nrao VLA/test/dVLAArchiveInput.cc )
casa_add_assay( nrao VLA/test/tVLABuffer.cc )
casa_add_assay( nrao VLA/test/tVLADiskInput.cc )
casa_add_assay( nrao VLA/test/tVLALogicalRecord.cc )
casa_add_assay( nrao VLA/test/tVLATapeInput.cc )
