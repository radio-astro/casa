//# Options.h: base class for storing and parsing parameters
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

#ifndef DISPLAY_OPTIONS_H__
#define DISPLAY_OPTIONS_H__
#include <map>
#include <set>
#include <string>
#include <sys/stat.h>

namespace casa {
	namespace viewer {

		class Options {
		public:
			class Kernel {
			public:
				virtual std::string tmp( ) const = 0;
				virtual ~Kernel( ) { }
			};

			std::string tmp( ) const {
				return kernel->tmp( );
			}

			// this returns a path to be used as a temporary file or directory, and
			// by default, deletes the file when the viewer exits... the "base_name"
			// is just the name to be used as a starting point for finding a unique
			// file name, an example would be "my_tmp_file"... but it could be
			// anything (not including directories, i.e. "/")... this function
			// guarantees that no two returned strings will be identical... and that
			// all will be valid path names...
			std::string temporaryPath( const std::string &base_name, bool remove=true ) {
				return _temporary_path_( base_name, remove );
			}

			Options( ) { }  /*** initialized by options_init_  ***/
			~Options( ) { } /*** finalized by options_init_    ***/


		private:
			friend class options_init_;
			Options( const Options & ) { }
			const Options &operator=(const Options&) {
				return *this;
			}
			void init( Kernel *k ) {
				kernel = k;
				returned_paths = new std::map<std::string,std::pair<std::string,bool> >( );
			}
			void finalize( );
			typedef std::map<std::string,std::pair<std::string,bool> > path_map;
			path_map *returned_paths;
			Kernel *kernel;

			std::string _temporary_path_( const std::string &/*base_dir_name*/, bool /*remove*/ );
		};

		extern Options options;

		static class options_init_ {
		public:
			options_init_( ) {
				if ( count++ == 0 ) do_init( );
			}
			~options_init_( ) {
				if ( --count == 0 ) {
					options.finalize( );
				}
			}
		private:
			static unsigned int count;
			// to be defined in qt (or other windowing library) land....
			void do_init( );
		} _options_init_object_;
	}
}

#endif
