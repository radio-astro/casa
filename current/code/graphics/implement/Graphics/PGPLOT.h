//# PGPLOT.h: AIPS++ interface to PGPLOT routines
//# Copyright (C) 1993,1994,1995,2000,2002
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

#ifndef GRAPHICS_PGPLOT_H
#define GRAPHICS_PGPLOT_H

#include <casa/aips.h>

#if !defined(NEED_FORTRAN_UNDERSCORES)
#define NEED_FORTRAN_UNDERSCORES 1
#endif

#if NEED_FORTRAN_UNDERSCORES
 #define wpgbegin wpgbegin_
 #define wpgenv   wpgenv_
 #define wpgask   wpgask_
 #define wpghist  wpghist_
 #define wpglabel wpglabel_
 #define wpgend   wpgend_
 #define wpgline  wpgline_
 #define wpgpoint wpgpoint_
 #define wpgiden  wpgiden_
 #define wpgtext  wpgtext_
 #define wpgcont  wpgcont_
 #define wpggray  wpggray_
 #define wpgerrx  wpgerrx_
 #define wpgerry  wpgerry_
 #define MAIN   MAIN_
#endif

namespace casa { //# NAMESPACE CASA - BEGIN


// <summary>PGPLOT interface calls</summary>
// <reviewed reviewer="UNKNOWN" date="before2004/08/25" tests="" demos="">
// </reviewed>
// <group name=interface>
extern "C" {
  // PGPLOT routines
  void wpgbegin (int *, const char *, int *, int *, int *);
  void wpgask   (int *);
  void wpghist  (int *, float *, float *, float *, int *, int *);
  void wpglabel (const char *, const char *, const char *, int *, int *, int *);
  void wpgiden  ();
  void wpgenv   (float *, float *, float *, float *, int *, int *);
  void wpgline  (int *, float *, float *);
  void wpgpoint (int *, float *, float *, int *);
  void wpgcont  (float *, int *, int *, int *, int *, int *, int *,
		 float *, int *, float *);
  void wpggray  (float *, int *, int *, int *, int *, int *, int *,
		 float *, float *, float *);
  void wpgerrx  (int *, float *, float *, float *, float *);
  void wpgerry  (int *, float *, float *, float *, float *);
  void wpgend   ();
               
  // called by fortran initialization, may not be needed on all machines,
  // in fact, this might cause problems on some machines.
  void MAIN_(); 
};
// </group>

} //# NAMESPACE CASA - END

#endif
