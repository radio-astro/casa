//# Lowlevel.cc: implementation for low-level tools used by the viewer
//# Copyright (C) 2010
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

#include <display/Utilities/Lowlevel.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <libgen.h>
#include <QApplication>
#include <errno.h>

namespace casa { //# NAMESPACE CASA - BEGIN
	namespace viewer { //# NAMESPACE VIEWER - BEGIN
		//
		//  helper function to create ~/.casa/ipython/security
		//  returns true upon success, false upon failure...
		//
		bool create_dir( const char *path, bool recurse ) {
			struct stat buf;
			if ( stat( path, &buf ) == 0 ) {
				if ( ! S_ISDIR(buf.st_mode) ) {
					char *savepath = (char*) malloc(sizeof(char) * (strlen(path) + 12));
					int count = 0;

					do {
						count += 1;
						sprintf( savepath, "%s_SAV%03d", path, count );
					} while ( stat( savepath, &buf ) == 0 );

					if ( rename( path, savepath ) != 0 ) {
						free(savepath);
						return false;
					}
					if ( mkdir( path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) != 0 ) {
						free(savepath);
						return false;
					}
					free(savepath);
				}
			} else {
				if ( mkdir( path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH ) != 0 ) {
					if ( recurse ) {

						// can't write to the parent directory...
						if ( errno == EACCES ) return false;

						char *orig = strdup(path);
						char *dir = dirname(orig);
						if ( ! strcmp("/",dir) || ! strcmp(".",dir) ) {
							free(orig);
							return true;
						} else if ( ! strcmp("..",dir) ) {
							free(orig);
							char buf[10];
							if ( ! strcmp("/",getcwd(buf,10)) )
								return false;
							else
								return true;
						}
						if ( create_dir(dir,true) ) {
							free( orig );
							return create_dir(path,false);
						} else {
							return false;
						}
					} else {
						return false;
					}
				}
			}
			return true;
		}

		std::string time( ) {
			struct timeval tv = {0,0};
			gettimeofday( &tv, 0 );

			char buf[256];
			strftime( buf, 256, "%T", localtime(&tv.tv_sec) );
			return std::string(buf);
		}

		std::string date( ) {
			struct timeval tv = {0,0};
			gettimeofday( &tv, 0 );

			char buf[256];
			strftime( buf, 256, "%F", localtime(&tv.tv_sec) );
			return std::string(buf);
		}

		static std::string rc_home;
		bool setrcDir( const std::string &home ) {
			if ( ! rc_home.empty( ) ) {
				return false;
			} else if ( create_dir( home + "/viewer", true ) ) {
				rc_home = home + "/viewer";
				return true;
			} else {
				return false;
			}
		}

		Casarc &getrc( ) {
			char *home = getenv("HOME");
			if ( ! rc_home.empty( ) ) {
				return Casarc::instance(rc_home + "/rc");
			}
			if ( home ) {
				std::string rcf(home);
				rcf += "/.casa";
				create_dir( rcf );
				rcf += "/viewer";
				create_dir( rcf );
				rc_home = rcf;
				return Casarc::instance(rc_home + "/rc");
			} else {
				fprintf( stderr, "HOME is not defined... using current directory to store state...\n" );
				return Casarc::instance("casaviewerrc");
			}
		}

		guiwait::guiwait( ) {
			QApplication::setOverrideCursor(Qt::WaitCursor);
		}

		guiwait::~guiwait( ) {
			QApplication::restoreOverrideCursor();
		}

	} //# NAMESPACE VIEWER - BEGIN
} //# NAMESPACE CASA - BEGIN
