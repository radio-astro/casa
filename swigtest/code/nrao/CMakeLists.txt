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
  implement/FITS/GBTACSTable.cc
  implement/FITS/GBTAntennaFile.cc
  implement/FITS/GBTBackendTable.cc
  implement/FITS/GBTDAPFile.cc
  implement/FITS/GBTFITSBase.cc
  implement/FITS/GBTGOFile.cc
  implement/FITS/GBTLO1File.cc
  implement/FITS/GBTMultiACSTable.cc
  implement/FITS/GBTSampler.cc
  implement/FITS/GBTScanLogReader.cc
  implement/FITS/GBTStateTable.cc
  implement/FITS/OldGBTPositionTable.cc
  implement/GBTFillers/GBTACSFiller.cc
  implement/GBTFillers/GBTAntennaDAPFiller.cc
  implement/GBTFillers/GBTBackendFiller.cc
  implement/GBTFillers/GBTCorrelation.cc
  implement/GBTFillers/GBTDAPFillerBase.cc
  implement/GBTFillers/GBTDAPFiller.cc
  implement/GBTFillers/GBTDCRFiller.cc
  implement/GBTFillers/GBTFeed.cc
  implement/GBTFillers/GBTFeedDDFiller.cc
  implement/GBTFillers/GBTFeedInfo.cc
  implement/GBTFillers/GBTGOFiller.cc
  implement/GBTFillers/GBTHoloFiller.cc
  implement/GBTFillers/GBTIF.cc
  implement/GBTFillers/GBTIFFiller.cc
  implement/GBTFillers/GBTIFLOHelper.cc
  implement/GBTFillers/GBTLO1DAPFiller.cc
  implement/GBTFillers/GBTMeanFocusFiller.cc
  implement/GBTFillers/GBTMSAntennaFiller.cc
  implement/GBTFillers/GBTMSDataDescFiller.cc
  implement/GBTFillers/GBTMSFeedFiller.cc
  implement/GBTFillers/GBTMSFieldFiller.cc
  implement/GBTFillers/GBTMSFillOptions.cc
  implement/GBTFillers/GBTMSHistoryFiller.cc
  implement/GBTFillers/GBTMSObservationFiller.cc
  implement/GBTFillers/GBTMSPointingFiller.cc
  implement/GBTFillers/GBTMSPolarizationFiller.cc
  implement/GBTFillers/GBTMSProcessorFiller.cc
  implement/GBTFillers/GBTMSSourceFiller.cc
  implement/GBTFillers/GBTMSSpecWinFiller.cc
  implement/GBTFillers/GBTMSStateFiller.cc
  implement/GBTFillers/GBTMSSysCalFiller.cc
  implement/GBTFillers/GBTMSTypeDataFiller.cc
  implement/GBTFillers/GBTMSTypeFiller.cc
  implement/GBTFillers/GBTMSTypeRecFiller.cc
  implement/GBTFillers/GBTMSTypeStateFiller.cc
  implement/GBTFillers/GBTMSWeatherCopier.cc
  implement/GBTFillers/GBTMSWeatherFiller.cc
  implement/GBTFillers/GBTPointModelFiller.cc
  implement/GBTFillers/GBTPolarization.cc
  implement/GBTFillers/GBTRcvrCalFiller.cc
  implement/GBTFillers/GBTSimpleTable.cc
  implement/GBTFillers/GBTSPFiller.cc
  implement/SDD/SDD12mOnLine.cc
  implement/SDD/SDDBlock.cc
  implement/SDD/SDDBootStrap.cc
  implement/SDD/SDDFile.cc
  implement/SDD/SDDHeader.cc
  implement/SDD/SDDIndex.cc
  implement/SDD/SDDOnLine.cc
  implement/VLA/Get_num_attached.cc
  implement/VLA/nreal.cc
  implement/VLA/VLAADA.cc
  implement/VLA/VLAArchiveInput.cc
  implement/VLA/VLABaselineRecord.cc
  implement/VLA/VLACalibratorFilter.cc
  implement/VLA/VLACDA.cc
  implement/VLA/VLAContinuumRecord.cc
  implement/VLA/VLADiskInput.cc
  implement/VLA/VLAEnum.cc
  implement/VLA/VLAFiller.cc
  implement/VLA/VLAFilter.cc
  implement/VLA/VLAFilterSet.cc
  implement/VLA/VLAFrequencyFilter.cc
  implement/VLA/VLALogicalRecord.cc
  implement/VLA/VLAObsModeFilter.cc
  implement/VLA/VLAOnlineInput.cc
  implement/VLA/VLAProjectFilter.cc
  implement/VLA/VLARCA.cc
  implement/VLA/VLASDA.cc
  implement/VLA/VLASourceFilter.cc
  implement/VLA/VLASpectralLineRecord.cc
  implement/VLA/VLASubarrayFilter.cc
  implement/VLA/VLATapeInput.cc
  implement/VLA/VLATimeFilter.cc
   )

install (FILES
	implement/FITS/GBTACSTable.h
	implement/FITS/GBTAntennaFile.h
	implement/FITS/GBTBackendTable.h
	implement/FITS/GBTDAPFile.h
	implement/FITS/GBTFITSBase.h
	implement/FITS/GBTGOFile.h
	implement/FITS/GBTLO1File.h
	implement/FITS/GBTMultiACSTable.h
	implement/FITS/GBTSampler.h
	implement/FITS/GBTScanLogReader.h
	implement/FITS/GBTStateTable.h
	implement/FITS/OldGBTBackendTable.h
	implement/FITS/OldGBTPositionTable.h
	DESTINATION include/casacode/nrao/FITS
	)
install (FILES
	implement/GBTFillers/GBTACSFiller.h
	implement/GBTFillers/GBTAntennaDAPFiller.h
	implement/GBTFillers/GBTBackendFiller.h
	implement/GBTFillers/GBTCorrelation.h
	implement/GBTFillers/GBTDAPFiller.h
	implement/GBTFillers/GBTDAPFillerBase.h
	implement/GBTFillers/GBTDCRFiller.h
	implement/GBTFillers/GBTFeed.h
	implement/GBTFillers/GBTFeedDDFiller.h
	implement/GBTFillers/GBTFeedInfo.h
	implement/GBTFillers/GBTGOFiller.h
	implement/GBTFillers/GBTHoloFiller.h
	implement/GBTFillers/GBTIF.h
	implement/GBTFillers/GBTIFFiller.h
	implement/GBTFillers/GBTIFLOHelper.h
	implement/GBTFillers/GBTLO1DAPFiller.h
	implement/GBTFillers/GBTMSAntennaFiller.h
	implement/GBTFillers/GBTMSDataDescFiller.h
	implement/GBTFillers/GBTMSFeedFiller.h
	implement/GBTFillers/GBTMSFieldFiller.h
	implement/GBTFillers/GBTMSFillOptions.h
	implement/GBTFillers/GBTMSHistoryFiller.h
	implement/GBTFillers/GBTMSObservationFiller.h
	implement/GBTFillers/GBTMSPointingFiller.h
	implement/GBTFillers/GBTMSPolarizationFiller.h
	implement/GBTFillers/GBTMSProcessorFiller.h
	implement/GBTFillers/GBTMSSourceFiller.h
	implement/GBTFillers/GBTMSSpecWinFiller.h
	implement/GBTFillers/GBTMSStateFiller.h
	implement/GBTFillers/GBTMSSysCalFiller.h
	implement/GBTFillers/GBTMSTypeDataFiller.h
	implement/GBTFillers/GBTMSTypeFiller.h
	implement/GBTFillers/GBTMSTypeRecFiller.h
	implement/GBTFillers/GBTMSTypeStateFiller.h
	implement/GBTFillers/GBTMSWeatherCopier.h
	implement/GBTFillers/GBTMSWeatherFiller.h
	implement/GBTFillers/GBTMeanFocusFiller.h
	implement/GBTFillers/GBTPointModelFiller.h
	implement/GBTFillers/GBTPolarization.h
	implement/GBTFillers/GBTRcvrCalFiller.h
	implement/GBTFillers/GBTSPFiller.h
	implement/GBTFillers/GBTSimpleTable.h
	DESTINATION include/casacode/nrao/GBTFillers
	)
install (FILES
	implement/SDD/SDD12mOnLine.h
	implement/SDD/SDDBlock.h
	implement/SDD/SDDBootStrap.h
	implement/SDD/SDDFile.h
	implement/SDD/SDDHeader.h
	implement/SDD/SDDIndex.h
	implement/SDD/SDDOnLine.h
	DESTINATION include/casacode/nrao/SDD
	)
install (FILES
	implement/VLA/VLAADA.h
	implement/VLA/VLAArchiveInput.h
	implement/VLA/VLABaselineRecord.h
	implement/VLA/VLACDA.h
	implement/VLA/VLACalibratorFilter.h
	implement/VLA/VLAContinuumRecord.h
	implement/VLA/VLADiskInput.h
	implement/VLA/VLAEnum.h
	implement/VLA/VLAFiller.h
	implement/VLA/VLAFilter.h
	implement/VLA/VLAFilterSet.h
	implement/VLA/VLAFrequencyFilter.h
	implement/VLA/VLALogicalRecord.h
	implement/VLA/VLAObsModeFilter.h
	implement/VLA/VLAOnlineInput.h
	implement/VLA/VLAProjectFilter.h
	implement/VLA/VLARCA.h
	implement/VLA/VLASDA.h
	implement/VLA/VLASourceFilter.h
	implement/VLA/VLASpectralLineRecord.h
	implement/VLA/VLASubarrayFilter.h
	implement/VLA/VLATapeIO.h
	implement/VLA/VLATapeInput.h
	implement/VLA/VLATimeFilter.h
	implement/VLA/nreal.h
	DESTINATION include/casacode/nrao/VLA
	)

casa_add_assay( nrao implement/VLA/test/dVLAArchiveInput.cc )
casa_add_assay( nrao implement/VLA/test/tVLABuffer.cc )
casa_add_assay( nrao implement/VLA/test/tVLADiskInput.cc )
casa_add_assay( nrao implement/VLA/test/tVLALogicalRecord.cc )
casa_add_assay( nrao implement/VLA/test/tVLATapeInput.cc )
