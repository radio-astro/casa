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

add_definitions( ${flagging_DEFINITIONS} )
include_directories( ${flagging_INCLUDE_DIRS} )

# Dynamic library
add_library( flagging
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
  implement/MSPlot/MsPlot.cc
 )

target_link_libraries( flagging ${flagging_LINK_TO} )

set_target_properties( flagging PROPERTIES SOVERSION ${CASA_API_VERSION} )

install( TARGETS flagging LIBRARY DESTINATION lib )

# Executables
# Unit tests
add_executable( tAutoflag EXCLUDE_FROM_ALL implement/Flagging/test/tAutoflag.cc )
target_link_libraries( tAutoflag flagging )
add_test( tAutoflag ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tAutoflag )
add_dependencies( check tAutoflag )

add_executable( tRFASelector EXCLUDE_FROM_ALL implement/Flagging/test/tRFASelector.cc )
target_link_libraries( tRFASelector flagging )
add_test( tRFASelector ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/tRFASelector )
add_dependencies( check tRFASelector )

