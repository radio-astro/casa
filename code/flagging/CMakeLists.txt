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

if( APPLE AND NOT CMAKE_CXX_FLAGS MATCHES x86_64 )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=i586" )
endif()


casa_add_library( flagging
  implement/Flagging/DDMapper.cc
  implement/Flagging/ExtendFlagger.cc
  implement/Flagging/Flagger.cc
  implement/Flagging/FlagIndex.cc
  implement/Flagging/RedFlagger.cc
  implement/Flagging/RFAApplyFlags.cc
  implement/Flagging/RFADiffBase.cc
  implement/Flagging/RFAFlagCubeBase.cc
  implement/Flagging/RFAMedianClip.cc
  implement/Flagging/RFANewMedianClip.cc
  implement/Flagging/RFASelector2.cc
  implement/Flagging/RFASpectralRej.cc
  implement/Flagging/RFAUVBinner.cc
  implement/Flagging/RFChunkStats.cc
  implement/Flagging/RFDataMapper.cc
  implement/Flagging/RFDebugPlot.cc
  implement/Flagging/RFFlagCube.cc
  implement/Flagging/RFRowClipper.cc
  implement/Flagging/RFFloatLattice.cc
  implement/MSPlot/MsPlot.cc
   )

casa_add_test( flagging implement/Flagging/test/tAutoflag.cc )
casa_add_test( flagging implement/Flagging/test/tRFASelector.cc )
casa_add_test( flagging implement/Flagging/test/tFlagger.cc )
casa_add_test( flagging implement/Flagging/test/tRFCubeLattice.cc )
