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

if( APPLE AND
    NOT INTEL_CXX_COMPILER MATCHES true  AND
    NOT CMAKE_CXX_FLAGS MATCHES x86_64 AND
    NOT _gcc_machine MATCHES x86_64  AND
    NOT _gcc_machine MATCHES 11.0  AND
    NOT _gcc_machine MATCHES i686-apple-darwin11 )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=i586 -msse3" )
endif()


casa_add_library( flagging
  implement/Flagging/DDMapper.cc
  implement/Flagging/ExtendFlagger.cc
  implement/Flagging/Flagger.cc
  implement/Flagging/TestFlagger.cc
  implement/Flagging/RFADiffBase.cc
  implement/Flagging/RFAFlagCubeBase.cc
  implement/Flagging/RFAMedianClip.cc
  implement/Flagging/RFANewMedianClip.cc
  implement/Flagging/RFASelector2.cc
  implement/Flagging/RFASpectralRej.cc
  implement/Flagging/RFAUVBinner.cc
  implement/Flagging/RFChunkStats.cc
  implement/Flagging/RFDataMapper.cc
  implement/Flagging/RFFlagCube.cc
  implement/Flagging/RFRowClipper.cc
  implement/Flagging/RFFloatLattice.cc
  implement/Flagging/LightFlagger.cc
  implement/Flagging/LFTimeFreqCrop.cc
  implement/Flagging/LFExamineFlags.cc
  implement/Flagging/LFExtendFlags.cc
  implement/Flagging/LFDisplayFlags.cc
  implement/Flagging/FlagDataHandler.cc
  implement/Flagging/FlagMSHandler.cc
  implement/Flagging/FlagCalTableHandler.cc
  implement/Flagging/FlagReport.cc
  implement/Flagging/FlagAgentBase.cc
  implement/Flagging/FlagAgentTimeFreqCrop.cc
  implement/Flagging/FlagAgentClipping.cc
  implement/Flagging/FlagAgentSummary.cc
  implement/Flagging/FlagAgentDisplay.cc
  implement/Flagging/FlagAgentManual.cc
  implement/Flagging/FlagAgentElevation.cc
  implement/Flagging/FlagAgentQuack.cc
  implement/Flagging/FlagAgentShadow.cc
  implement/Flagging/FlagAgentExtension.cc
  implement/Flagging/FlagAgentRFlag.cc
  implement/Flagging/FlagVersion.cc
  implement/Flagging/SLog.cc
   )

install (FILES
	implement/Flagging/DDMapper.h
	implement/Flagging/ExtendFlagger.h
	implement/Flagging/FlagAgentBase.h
	implement/Flagging/FlagAgentClipping.h
	implement/Flagging/FlagAgentDisplay.h
	implement/Flagging/FlagAgentElevation.h
	implement/Flagging/FlagAgentExtension.h
	implement/Flagging/FlagAgentManual.h
	implement/Flagging/FlagAgentQuack.h
	implement/Flagging/FlagAgentRFlag.h
	implement/Flagging/FlagAgentShadow.h
	implement/Flagging/FlagAgentSummary.h
	implement/Flagging/FlagAgentTimeFreqCrop.h
	implement/Flagging/FlagCalTableHandler.h
	implement/Flagging/FlagDataHandler.h
	implement/Flagging/FlagMSHandler.h
	implement/Flagging/FlagReport.h
	implement/Flagging/FlagVersion.h
	implement/Flagging/Flagger.h
	implement/Flagging/LFBase.h
	implement/Flagging/LFDisplayFlags.h
	implement/Flagging/LFExamineFlags.h
	implement/Flagging/LFExtendFlags.h
	implement/Flagging/LFPlotServerProxy.h
	implement/Flagging/LFTimeFreqCrop.h
	implement/Flagging/LightFlagger.h
	implement/Flagging/RFABase.h
	implement/Flagging/RFADiffBase.h
	implement/Flagging/RFAFlagCubeBase.h
	implement/Flagging/RFAFlagExaminer.h
	implement/Flagging/RFAFlagExaminer.tcc
	implement/Flagging/RFAMedianClip.h
	implement/Flagging/RFANewMedianClip.h
	implement/Flagging/RFASelector.h
	implement/Flagging/RFASelector.tcc
	implement/Flagging/RFASpectralRej.h
	implement/Flagging/RFATimeFreqCrop.h
	implement/Flagging/RFATimeFreqCrop.tcc
	implement/Flagging/RFAUVBinner.h
	implement/Flagging/RFChunkStats.h
	implement/Flagging/RFChunkStats.tcc
	implement/Flagging/RFCommon.h
	implement/Flagging/RFCubeLattice.h
	implement/Flagging/RFCubeLattice.tcc
	implement/Flagging/RFDataMapper.h
	implement/Flagging/RFFlagCube.h
	implement/Flagging/RFFlagCube.tcc
	implement/Flagging/RFFloatLattice.h
	implement/Flagging/RFRowClipper.h
	implement/Flagging/SLog.h
	implement/Flagging/TestFlagger.h
	DESTINATION include/casacode/flagging/Flagging
	)

casa_add_test( flagging implement/Flagging/test/tFlagAgentRFlag.cc )
casa_add_test( flagging implement/Flagging/test/tFlagAgentExtension.cc )
casa_add_test( flagging implement/Flagging/test/tFlagAgentShadow.cc )
casa_add_test( flagging implement/Flagging/test/tFlagAgentQuack.cc )
casa_add_test( flagging implement/Flagging/test/tFlagAgentElevation.cc )
casa_add_test( flagging implement/Flagging/test/tFlagAgentManual.cc )
casa_add_test( flagging implement/Flagging/test/tFlagAgentSummary.cc )
casa_add_test( flagging implement/Flagging/test/tFlagAgentDisplay.cc )
casa_add_test( flagging implement/Flagging/test/tFlagAgentClipping.cc )
casa_add_test( flagging implement/Flagging/test/tFlagAgentTimeFreqCrop.cc )
casa_add_test( flagging implement/Flagging/test/tFlagAgentBase.cc )
casa_add_test( flagging implement/Flagging/test/tFlagDataHandler.cc )
casa_add_test( flagging implement/Flagging/test/tAutoflag.cc )
casa_add_test( flagging implement/Flagging/test/tRFASelector.cc )
casa_add_test( flagging implement/Flagging/test/tFlagger.cc )
casa_add_test( flagging implement/Flagging/test/tRFCubeLattice.cc )
casa_add_test( flagging implement/Flagging/test/tTestFlagger.cc )
