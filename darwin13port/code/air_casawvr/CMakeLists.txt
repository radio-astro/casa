#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2012 by ESO (in the framework of the ALMA collaboration)
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

casa_add_library( air_casawvr
  casawvr/casawvr_main.cpp
  casawvr/mswvrdata.cpp 
  casawvr/msgaintable.cpp
  casawvr/msutils.cpp 
  casawvr/casawvr_errs.cpp
  casawvr/msspec.cpp
  casawvr/msweather.cpp 
  casawvr/msantdata.cpp
  apps/arraygains2.cpp
  )

casa_add_executable( air_casawvr wvrgcal
  cmdline/wvrgcal.cpp  
  cmdline/wvrgcalerrors.cpp  
  cmdline/wvrgcalfeedback.cpp
  )
