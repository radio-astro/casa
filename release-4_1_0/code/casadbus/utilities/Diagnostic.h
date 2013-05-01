//# Diagnostic.h: debugging diagnostic for dbus clents
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
//# $Id$

#ifndef CASA_DBUS_DIAGNOSTIC_H_
#define CASA_DBUS_DIAGNOSTIC_H_
#include <string>
#include <stdarg.h>

namespace casa {
	namespace dbus {

		class Diagnostic {
			public:
				friend class init_diagnostic_object_t;
				void argv( int argc_, const char *argv_[] ) {
					kernel_t &k = lock_kernel( );
					if ( k.do_log( ) ) k.argv(argc_,argv_);
					release_kernel( );
				}
				void argv( int argc_, char *argv_[] ) {
					kernel_t &k = lock_kernel( );
					if ( k.do_log( ) ) k.argv(argc_,argv_);
					release_kernel( );
				}
				Diagnostic( ) { }
				virtual ~Diagnostic( ) { }

				void error(const char *fmt, ...) {
					kernel_t &k = lock_kernel( );
					if ( k.do_log( ) ) {
						va_list argp;
						va_start(argp, fmt);
						verror(k, fmt, argp);
						va_end(argp);
					}
				}

				void info(const char *fmt, ...) {
					kernel_t &k = lock_kernel( );
					if ( k.do_log( ) ) {
						va_list argp;
						va_start(argp, fmt);
						vinfo(k, fmt, argp);
						va_end(argp);
					}
				}

			private:

				struct kernel_t {
					kernel_t( );
					kernel_t( FILE *f );
					bool do_log( ) const { return fptr != 0; }
					~kernel_t( ) { if ( fptr ) fclose(fptr); }
					void argv( int argc_, const char *argv_[] );
					void argv( int argc_, char *argv_[] );
					FILE *fptr;
					pid_t pid;
					std::string name;  /**** any non-argv messages should include 'name' ****/
				};

				/* void verror(Diagnostic::kernel_t &, const char *fmt, va_list argp); */
				/* void vinfo(Diagnostic::kernel_t &, const char *fmt, va_list argp); */
				void verror(kernel_t &, const char *fmt, va_list argp);
				void vinfo(kernel_t &, const char *fmt, va_list argp);

				kernel_t &lock_kernel( );
				void release_kernel( ) { }
				void output_prologue( );
				void output_epilogue( );
		};

		Diagnostic diagnostic;

		static class init_diagnostic_object_t {
			public:
				init_diagnostic_object_t( ) { if ( count++ == 0 ) diagnostic.output_prologue( ); }
				~init_diagnostic_object_t( ) { if ( --count == 0 ) { diagnostic.output_epilogue( ); } }
			private:
				static unsigned long count;
		} init_diagnostic_object_;
	}
}

#endif
