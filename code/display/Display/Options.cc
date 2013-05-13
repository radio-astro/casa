//# Options.cc: class to store and retrieve parameters
//# Copyright (C) 2011
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <display/Display/Options.h>
#include <casa/OS/Directory.h>
#include <sys/stat.h>
#include <iostream>
#include <dirent.h>
#include <string.h>		/***for strlen( )***/

namespace casa {
	namespace viewer {

		Options options;
		unsigned int options_init_::count = 0;

		// arbitrary base numbers...
		static std::string arbitrary( unsigned int val, unsigned int digits=3, const char *cmap="0123456789abcdefghijklmnopqrstuvwxyz" ) {
			char result[digits+1];
			unsigned int base = strlen(cmap);
			if ( base < 2 ) return "";
			// pad with zeros...
			for ( unsigned int i=0; i < digits; ++i ) result[i] = cmap[0];
			result[digits] = '\0';
			for ( unsigned int i=1; i < digits+1 && val > 0; ++i ) {
				unsigned int place_value = val % base;
				result[digits-i] = cmap[place_value];
				val /= base;
			}
			return result;
		}

		// non-const because later it would be nice to have the option of
		// marking temporaries so that the could be automatically deleted
		// upon exit...
		std::string Options::_temporary_path_( const std::string &base_dir_name, bool remove ) {
			std::string result;
			std::string tmppath = tmp( );
			struct stat statbuf;
			if ( base_dir_name.size( ) == 0 ) return "";
			std::string sep = (base_dir_name[base_dir_name.size( )-1] == '.') ? "" : "_";
			for ( unsigned int count=1; count < 46656; ++count ) {
				std::string teststr = tmppath + "/" + base_dir_name + sep + arbitrary(count);
				if ( returned_paths->find(teststr) != returned_paths->end( ) ) continue;
				if ( lstat(teststr.c_str( ), &statbuf) == -1 ) {
					result = teststr;
					break;
				}
			}

			if ( result != std::string( ) )
				returned_paths->insert(path_map::value_type(result,std::pair<std::string,bool>("unused",remove)));

			return result;
		}

		void Options::finalize( ) {
			std::string temporary_path = kernel->tmp( );
			for ( path_map::iterator it=returned_paths->begin( ); it != returned_paths->end( ); ++it ) {
				// removal requested?
				if ( it->second.second ) {
					// does it look like one of our temporaries?
					if ( it->first.compare(0,temporary_path.size(),temporary_path) == 0 &&
					        it->first.size( ) != temporary_path.size( ) ) {
						File tmpfile(it->first);
						if ( tmpfile.exists( ) && tmpfile.isWritable( ) ) {
							if ( tmpfile.isDirectory(false) ) {
								Directory tmpdir(tmpfile);
								tmpdir.removeRecursive(false);
							} else {
								// why hassle with decerning symlinks from block special files from regular files, etc.
								unlink(it->first.c_str( ));
							}
						}
					}
				}
			}
			delete kernel;
			kernel = 0;
			delete returned_paths;
			returned_paths = 0;
		}
	}
}
