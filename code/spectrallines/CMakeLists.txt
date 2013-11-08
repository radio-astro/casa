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

casa_add_library( spectrallines
    Splatalogue/ListConverter.cc
    Splatalogue/SearchEngine.cc
    Splatalogue/Searcher.cc 
    Splatalogue/SplatResult.cc
    Splatalogue/SQLiteSearch/SearcherSQLite.cc
    Splatalogue/SQLiteSearch/DatabaseConnector.cc
    Splatalogue/SearcherFactory.cc  
    Splatalogue/SplatalogueTable.cc   
)
install (FILES 
	Splatalogue/ListConverter.h
	Splatalogue/SearchEngine.h
	Splatalogue/Searcher.h
	Splatalogue/SQLiteSearch/SearcherSQLite.h
	Splatalogue/SQLiteSearch/DatabaseConnector.h
	Splatalogue/SearcherFactory.h
	Splatalogue/SplatResult.h
	Splatalogue/SplatalogueTable.h
	DESTINATION include/casacode/spectrallines/Splatalogue
	)
