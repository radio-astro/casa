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

casa_add_library( casa_sakura
    SakuraAlignedArray.cc
    SakuraUtils.cc
    SakuraArrayConverter.cc
  )

install (FILES
        SakuraAlignedArray.h
        SakuraAlignedArray.tcc
        SakuraUtils.h
        SakuraArrayConverter.h
	DESTINATION include/casacode/casa_sakura
	)

casa_add_assay( casa_sakura
    test/tSakuraAlignedArray.cc
    test/tSakuraUtils.cc
    )

