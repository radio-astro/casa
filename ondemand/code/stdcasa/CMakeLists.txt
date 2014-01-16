
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

# Make sure that compile time definitions (build time, svn revision number)
# are recomputed in every build.

# This target is executed unconditionally and before any other target that
# depends on the definitions file
add_custom_target( stdcasa_definitions COMMAND ${CMAKE_COMMAND} -E remove -f definitions )


# This is delegated to the cmake script code/install/casadef.cmake in
# order to allow setting environment variables portably.
add_custom_command( OUTPUT definitions
	  COMMAND ${CMAKE_COMMAND} -Dcasadef_perl=${PERL_EXECUTABLE} -Dcasadef_source_dir=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/install/casadef.cmake > definitions_tmp
	    COMMAND ${CMAKE_COMMAND} -E copy definitions_tmp definitions
	    )


# set SVNREVISION and BUILDTIME

set( casadef_quiet TRUE )
set( casadef_perl ${PERL_EXECUTABLE} )
set( casadef_source_dir ${CMAKE_SOURCE_DIR} )
include( ${CMAKE_SOURCE_DIR}/install/casadef.cmake )

add_custom_command( OUTPUT version.cc
	  COMMAND egrep subversion_revision definitions > tmp1.txt
	  COMMAND sed -e 's|[^0-9]*\"||g' tmp1.txt > tmp2.txt
	  COMMAND echo "namespace casa { extern const int   aips_patch_version =" > tmp3.txt
	  COMMAND cat tmp2.txt >> tmp3.txt
	COMMAND
	sed -e 's|MAJOR|${CASA_MAJOR_VERSION}|'
	-e 's|MINOR|${CASA_MINOR_VERSION}|'
	-e 's|PATCH|${CASA_PATCH_VERSION}|'
	-e 's|REVISION|${SVNREVISION}|'
	-e 's|DATE|${TAGGEDTIME}|'
	${CMAKE_CURRENT_SOURCE_DIR}/version.template > tmp4.txt
	COMMAND cat tmp3.txt tmp4.txt > version_tmp.cc
	|| ${PERL_EXECUTABLE} -e 'unlink("version_tmp.cc") \; exit 1 \;'
	COMMAND ${CMAKE_COMMAND} -E copy_if_different version_tmp.cc version.cc
	COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/../share
	COMMAND echo '\# casa version file created from casa code cmake' > ${CMAKE_INSTALL_PREFIX}/../share/version.txt
	COMMAND echo '${CASA_MAJOR_VERSION}.${CASA_MINOR_VERSION}.${CASA_PATCH_VERSION} ${SVNREVISION}' >> ${CMAKE_INSTALL_PREFIX}/../share/version.txt
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/version.template
	DEPENDS definitions
)

casa_add_library( stdcasa
	StdCasa/CasacSupport.cc
	StdCasa/record.cc
	StdCasa/variant.cc
	version2.cc
	version.cc
	)
add_dependencies( libstdcasa_fast stdcasa_definitions )
add_dependencies( libstdcasa      stdcasa_definitions )
install (FILES
	Quantity.h
	array.h
	record.h
	variant.h
	casac.h
	version.h
    cboost_foreach.h
	DESTINATION include/casacode/stdcasa
	)
install (FILES
	StdCasa/CasacSupport.h
	StdCasa/string_conversions
	DESTINATION include/casacode/stdcasa/StdCasa
	)
