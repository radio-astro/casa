# Copyright (2008) Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>
# 
# This file is part of AATM
#
# AATM is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# AATM is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU General Public License
# along with AATM.  If not, see <http://www.gnu.org/licenses/>.

bin_PROGRAMS = SMATest dispersive absorption

SMATest_SOURCES = SMATest.cpp

GENFILES = cmdlineiface.cpp cmdlineiface.hpp atmincludes.hpp

dispersive_SOURCES = dispersive.cpp $(GENFILES)
absorption_SOURCES = absorption.cpp $(GENFILES)

LDADD=../src/libaatm.la
INCLUDES=-I../src

EXTRA_DIST=data/SMA_17JUL2006_skydip1.dat

AM_CPPFLAGS= -DBOOST_TEST_DYN_LINK ${BOOST_CPPFLAGS}
AM_LDFLAGS=  ${BOOST_LDFLAGS} ${BOOST_PROGRAM_OPTIONS_LIB} 
