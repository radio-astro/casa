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


include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/implement/Filling ) 
# because the generated code contain relative paths to the source tree

casa_add_library( miriad
  implement/Filling/DataLoadingBuf.cc
  implement/Filling/DOmirfiller.cc
  implement/Filling/FillMetadata.cc
  implement/Filling/IDIndex.cc
  implement/Filling/MirExceptions.cc
  implement/Filling/MirFiller.cc
  implement/Filling/MirTypeAssert.cc
  implement/Filling/MirVisReader.cc
  )

# importmiriad is supposed to be a more general filler for miriad data
# casa_add_executable( miriad importmiriad apps/importmiriad/importmiriad.cc )

# carmafiller is somewhat limited, and deals only with CARMA (like) data
casa_add_executable( miriad carmafiller apps/carmafiller/carmafiller.cc )
