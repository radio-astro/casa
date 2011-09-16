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

casa_add_assay( nrao implement/VLA/test/dVLAArchiveInput.cc )
casa_add_assay( nrao implement/VLA/test/tVLABuffer.cc )
casa_add_assay( nrao implement/VLA/test/tVLADiskInput.cc )
casa_add_assay( nrao implement/VLA/test/tVLALogicalRecord.cc )
casa_add_assay( nrao implement/VLA/test/tVLATapeInput.cc )
