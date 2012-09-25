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

extern "C" char **environ;

namespace casa {
    namespace dbus {

	unsigned long init_diagnostic_object_t::count = 0;

	Diagnostic::kernel_t::kernel_t( FILE *f ) : fptr(f), pid(getpid( )) { }

#define ARGV(TYPE)								\
	void Diagnostic::kernel_t::argv( int argc_, TYPE *argv_[] ) {		\
	    if ( fptr ) {							\
		fprintf( fptr, "%5d argv:   ", pid );				\
		for ( int i=0; i < argc_; ++i )					\
		    fprintf( fptr, " %s", argv_[i] );				\
		fprintf( fptr, "\n" );						\
		fflush(fptr);							\
	    }									\
	    /*** save name for future diagnostic messages... ***/		\
	    if ( argc_ > 0 ) name = argv_[0];					\
	}

	ARGV(const char)
	ARGV(char)

	void init_diagnostic_object_t::do_initialize( ) {

	    char *outfile = getenv("CASA_DIAGNOSTIC_FILE");
	    if ( outfile == NULL )
		diagnostic.kernel_ = new Diagnostic::kernel_t(0);
	    else
		diagnostic.kernel_ = new Diagnostic::kernel_t(fopen(outfile,"a"));

	    if ( diagnostic.kernel_->fptr ) {
		char buf[PATH_MAX+1];
		// --- --- current time    --- --- --- --- --- --- --- ---
		struct timeval tv = {0,0};
		gettimeofday( &tv, 0 );
		strftime( buf, PATH_MAX+1, "begin:   %F %T", localtime(&tv.tv_sec) );
		fprintf( diagnostic.kernel_->fptr, "%5d %s\n", diagnostic.kernel_->pid, buf );
		fflush( diagnostic.kernel_->fptr );
		// --- --- current environment --- --- --- --- --- --- ---
		for ( char **ep=environ; *ep; ++ep )
		    fprintf( diagnostic.kernel_->fptr, "%5d env:     %s\n", diagnostic.kernel_->pid, *ep );
		fflush( diagnostic.kernel_->fptr );
		// --- --- current path    --- --- --- --- --- --- --- ---
		if ( getcwd(buf,PATH_MAX+1) != NULL )
		    fprintf( diagnostic.kernel_->fptr, "%5d cwd:     %s\n", diagnostic.kernel_->pid, buf );
	    }
	}

	void init_diagnostic_object_t::do_finalize( ) {
	    if ( diagnostic.kernel_ ) {
		if ( diagnostic.kernel_->fptr ) {
		    char buf[2048];
		    struct timeval tv = {0,0};
		    gettimeofday( &tv, 0 );
		    strftime( buf, 2048, "end:     %F %T", localtime(&tv.tv_sec) );
		    fprintf( diagnostic.kernel_->fptr, "%5d %s\n", diagnostic.kernel_->pid, buf );
		    fflush( diagnostic.kernel_->fptr );
		}
		delete diagnostic.kernel_;
	    }
	}
    }
}

