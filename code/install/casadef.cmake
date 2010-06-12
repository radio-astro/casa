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

# This CMake script sets the variables SVNREVISION, BUILDTIME and SVNURL.
#
# The script may be invoked from include(...) or using cmake -P
# In practice this dual interface is used to ensure that the values
# of SVNREVISION, BUILDTIME and SVNURL are consistent in the two contexts: 
# generating c++ code, and for generating python code.
#
# On input, the variables casadef_perl and casadef_source_dir must be set to
# the perl executable, and to ${CMAKE_SOURCE_DIR} respectively.
# If and only if the variable casadef_quiet is not set, the values
# of SVNREVISION, BUILDTIME and SVNURL are written to standard output
# as valid python code.

if( NOT casadef_perl )
  message( FATAL_ERROR "casadef_perl is undefined" )
endif()
if( NOT casadef_source_dir )
  message( FATAL_ERROR "casadef_source_dir is undefined" )
endif()

# Temporarily set LC_TIME and LC_MESSAGES to values which cause
# the "date" and "svn info" commands to speak English.
# There used to be a problem with LC_TIME=Japanese causing
# a runtime crash

set( _lc_time_old $ENV{LC_TIME} )
set( _lc_messages_old $ENV{LC_MESSAGES} )

set( ENV{LC_TIME} en_US.UTF-8 )
set( ENV{LC_MESSAGES} en_US )

execute_process( COMMAND
  date -u "+%a %Y/%m/%d %H:%M:%S UTC"
  OUTPUT_VARIABLE BUILDTIME 
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process( COMMAND
  ${casadef_perl} -e "open(INFO, 'svn info |') ; while (<INFO>){ if ( s/^Revision:\\s+//) { print; } }"
  WORKING_DIRECTORY ${casadef_source_dir}
  OUTPUT_VARIABLE SVNREVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process( COMMAND
  ${casadef_perl} -e "open(INFO, 'svn info |') ; while (<INFO>){ if ( s/^URL:\\s+//) { print; } }"
  WORKING_DIRECTORY ${casadef_source_dir}
  OUTPUT_VARIABLE SVNURL
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

set( ENV{LC_TIME} ${_lc_time_old} )
set( ENV{LC_MESSAGES} ${_lc_messages_old} )

if( NOT SVNREVISION )
  message( FATAL_ERROR "Could not read \"Revision\" from the output of \"svn info\"" )
endif()
if( NOT SVNURL )
  message( FATAL_ERROR "Could not read \"URL\" from the output of \"svn info\"" )
endif()

if( NOT casadef_quiet )
  execute_process( COMMAND echo "build_time = \"${BUILDTIME}\"" )
  execute_process( COMMAND echo "subversion_revision = \"${SVNREVISION}\"" )
  execute_process( COMMAND echo "subversion_url = \"${SVNURL}\"" )
endif()


