
# CASA - Common Astronomy Software Applications
# Copyright (C) 2012 by AUI
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

CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/version.cc.in ${CMAKE_CURRENT_BINARY_DIR}/version.cc )

casa_add_library( stdcasa
	StdCasa/CrashReporter.cc
	StdCasa/CasacSupport.cc
	StdCasa/record.cc
	StdCasa/variant.cc
	${CMAKE_CURRENT_BINARY_DIR}/version.cc
	)
if (UseCrashReporter)
    target_include_directories (libstdcasa PUBLIC ${Breakpad_IncludeRoot})
    target_link_libraries (libstdcasa general ${Breakpad_Library})
endif ()

if (UseCrashReporter)
   message ("DEBUG Adding UseCrashReporter to definitions")
    add_definitions( -DUseCrashReporter )
endif ()

add_dependencies( libstdcasa_fast stdcasa_definitions )
add_dependencies( libstdcasa      stdcasa_definitions )
install (FILES
	xerces.h
	Quantity.h
	array.h
	record.h
	variant.h
	casac.h
	version.h
	DESTINATION include/casacode/stdcasa
	)
install (FILES
        StdCasa/CrashReporter.h
	StdCasa/CasacSupport.h
	StdCasa/string_conversions
	DESTINATION include/casacode/stdcasa/StdCasa
	)
install (FILES
    thread/Barrier.h
	DESTINATION include/casacode/stdcasa/thread
	)
