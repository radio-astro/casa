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

casa_add_library( flagging
  Flagging/DDMapper.cc
  Flagging/ExtendFlagger.cc
  Flagging/Flagger.cc
  Flagging/AgentFlagger.cc
  Flagging/RFADiffBase.cc
  Flagging/RFAFlagCubeBase.cc
  Flagging/RFAMedianClip.cc
  Flagging/RFANewMedianClip.cc
  Flagging/RFASelector2.cc
  Flagging/RFASpectralRej.cc
  Flagging/RFAUVBinner.cc
  Flagging/RFChunkStats.cc
  Flagging/RFDataMapper.cc
  Flagging/RFFlagCube.cc
  Flagging/RFRowClipper.cc
  Flagging/RFFloatLattice.cc
  Flagging/FlagDataHandler.cc
  Flagging/FlagMSHandler.cc
  Flagging/FlagCalTableHandler.cc
  Flagging/FlagReport.cc
  Flagging/FlagAgentBase.cc
  Flagging/FlagAgentTimeFreqCrop.cc
  Flagging/FlagAgentClipping.cc
  Flagging/FlagAgentSummary.cc
  Flagging/FlagAgentDisplay.cc
  Flagging/FlagAgentManual.cc
  Flagging/FlagAgentElevation.cc
  Flagging/FlagAgentQuack.cc
  Flagging/FlagAgentShadow.cc
  Flagging/FlagAgentExtension.cc
  Flagging/FlagAgentRFlag.cc
  Flagging/FlagVersion.cc
  Flagging/SLog.cc
   )

install (FILES
	Flagging/DDMapper.h
	Flagging/ExtendFlagger.h
	Flagging/FlagAgentBase.h
	Flagging/FlagAgentClipping.h
	Flagging/FlagAgentDisplay.h
	Flagging/FlagAgentElevation.h
	Flagging/FlagAgentExtension.h
	Flagging/FlagAgentManual.h
	Flagging/FlagAgentQuack.h
	Flagging/FlagAgentRFlag.h
	Flagging/FlagAgentShadow.h
	Flagging/FlagAgentSummary.h
	Flagging/FlagAgentTimeFreqCrop.h
	Flagging/FlagCalTableHandler.h
	Flagging/FlagDataHandler.h
	Flagging/FlagMSHandler.h
	Flagging/FlagReport.h
	Flagging/FlagVersion.h
	Flagging/Flagger.h
	Flagging/LFPlotServerProxy.h
	Flagging/RFABase.h
	Flagging/RFADiffBase.h
	Flagging/RFAFlagCubeBase.h
	Flagging/RFAFlagExaminer.h
	Flagging/RFAFlagExaminer.tcc
	Flagging/RFAMedianClip.h
	Flagging/RFANewMedianClip.h
	Flagging/RFASelector.h
	Flagging/RFASelector.tcc
	Flagging/RFASpectralRej.h
	Flagging/RFATimeFreqCrop.h
	Flagging/RFATimeFreqCrop.tcc
	Flagging/RFAUVBinner.h
	Flagging/RFChunkStats.h
	Flagging/RFChunkStats.tcc
	Flagging/RFCommon.h
	Flagging/RFCubeLattice.h
	Flagging/RFCubeLattice.tcc
	Flagging/RFDataMapper.h
	Flagging/RFFlagCube.h
	Flagging/RFFlagCube.tcc
	Flagging/RFFloatLattice.h
	Flagging/RFRowClipper.h
	Flagging/SLog.h
	Flagging/AgentFlagger.h
	DESTINATION include/casacode/flagging/Flagging
	)

casa_add_test( flagging Flagging/test/tFlagAgentRFlag.cc )
casa_add_test( flagging Flagging/test/tFlagAgentExtension.cc )
casa_add_test( flagging Flagging/test/tFlagAgentShadow.cc )
casa_add_test( flagging Flagging/test/tFlagAgentQuack.cc )
casa_add_test( flagging Flagging/test/tFlagAgentElevation.cc )
casa_add_test( flagging Flagging/test/tFlagAgentManual.cc )
casa_add_test( flagging Flagging/test/tFlagAgentSummary.cc )
casa_add_test( flagging Flagging/test/tFlagAgentDisplay.cc )
casa_add_test( flagging Flagging/test/tFlagAgentClipping.cc )
casa_add_test( flagging Flagging/test/tFlagAgentTimeFreqCrop.cc )
casa_add_test( flagging Flagging/test/tFlagAgentBase.cc )
casa_add_test( flagging Flagging/test/tFlagDataHandler.cc )
casa_add_test( flagging Flagging/test/tAutoflag.cc )
casa_add_test( flagging Flagging/test/tRFASelector.cc )
casa_add_test( flagging Flagging/test/tFlagger.cc )
casa_add_test( flagging Flagging/test/tRFCubeLattice.cc )
casa_add_test( flagging Flagging/test/tAgentFlagger.cc )
