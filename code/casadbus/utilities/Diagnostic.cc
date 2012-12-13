//# Diagnostic.cc: debugging diagnostic for dbus clients
//# Copyright (C) 2012
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
//# $Id: $

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <casadbus/utilities/Diagnostic.h>

#if defined(__APPLE__)
#include <crt_externs.h>
#else
#include <linux/limits.h>
extern "C" char **environ;
#endif

namespace casa {
	namespace dbus {

		unsigned long init_diagnostic_object_t::count = 0;

		Diagnostic::kernel_t::kernel_t( ) : fptr(0), pid(getpid( )) { }
		Diagnostic::kernel_t::kernel_t( FILE *f ) : fptr(f), pid(getpid( )) { }

		Diagnostic::kernel_t &Diagnostic::lock_kernel( ) {
			// appropriate semaphores can be added when necessary...
			static Diagnostic::kernel_t *global_kernel = 0;
			if ( global_kernel == 0 ) {
				char *outfile = getenv("CASA_DIAGNOSTIC_FILE");
				if ( outfile == NULL )
					global_kernel = new kernel_t(0);
				else
					global_kernel = new kernel_t(fopen(outfile,"a"));
			}
			return *global_kernel;
		}

#define ARGV(TYPE)														\
		void Diagnostic::kernel_t::argv( int argc_, TYPE *argv_[] ) {	\
			if ( fptr ) {												\
				fprintf( fptr, "%5d argv:\t", pid );					\
				for ( int i=0; i < argc_; ++i )							\
					fprintf( fptr, (i==0?"%s":" %s"), argv_[i] );		\
				fprintf( fptr, "\n" );									\
				fflush(fptr);											\
			}															\
			/*** save name for future diagnostic messages... ***/		\
			if ( argc_ > 0 ) name = argv_[0];							\
		}

		ARGV(const char)
		ARGV(char)

		void Diagnostic::verror(kernel_t &k, const char *fmt, va_list argp) {
			fprintf( k.fptr, "%5d error:\t", k.pid );
			vfprintf( k.fptr, fmt, argp);
			fprintf( k.fptr, "\n" );
			fflush( k.fptr );
		}

		void Diagnostic::vinfo(kernel_t &k, const char *fmt, va_list argp) {
			fprintf( k.fptr, "%5d info:\t", k.pid );
			vfprintf( k.fptr, fmt, argp);
			fprintf( k.fptr, "\n" );
			fflush( k.fptr );
		}

		void Diagnostic::output_prologue( ) {
			kernel_t &k = lock_kernel( );
			if ( k.fptr ) {
				char buf[PATH_MAX+1];
				// --- --- current time    --- --- --- --- --- --- --- ---
				struct timeval tv = {0,0};
				gettimeofday( &tv, 0 );
				strftime( buf, PATH_MAX+1, "begin:\t%F %T", localtime(&tv.tv_sec) );
				fprintf( k.fptr, "%5d %s\n", k.pid, buf );
				fflush( k.fptr );
				// --- --- current environment --- --- --- --- --- --- ---
#if defined(__APPLE__)
				char*** envPtr = _NSGetEnviron();	// pointer to the real 'environ'
				char** environ = *envPtr;
#endif
				for ( char **ep=environ; *ep; ++ep )
					fprintf( k.fptr, "%5d env:\t%s\n", k.pid, *ep );
				fflush( k.fptr );
				// --- --- current path    --- --- --- --- --- --- --- ---
				if ( getcwd(buf,PATH_MAX+1) != NULL )
					fprintf( k.fptr, "%5d cwd:\t%s\n", k.pid, buf );
			}
			release_kernel( );
		}

		void Diagnostic::output_epilogue( ) {
			kernel_t &k = lock_kernel( );
			if ( k.fptr ) {
				char buf[2048];
				struct timeval tv = {0,0};
				gettimeofday( &tv, 0 );
				strftime( buf, 2048, "end:\t%F %T", localtime(&tv.tv_sec) );
				fprintf( k.fptr, "%5d %s\n", k.pid, buf );
				fflush( k.fptr );
			}
		}
	}
}

