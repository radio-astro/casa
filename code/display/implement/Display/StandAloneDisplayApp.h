//# StandAloneDisplayApp.h: define functions needed for stand-alone display
//# Copyright (C) 1999
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

#ifndef TRIALDISPLAY_STANDALONEDISPLAYAPP_H
#define TRIALDISPLAY_STANDALONEDISPLAYAPP_H

// <summary>
// Definitions of functions needed for stand-alone display applications.
// </summary>

// <synopsis>
// Some services (at the moment just PgPlot services) are implemented 
// differently depending on whether the display application being built
// is a stand-alone application, or is bound to some other process
// (eg. Glish/Tk) which already has its own way of providing said 
// service.
// </synopsis>

// <motivation>
// It seems sensible to provide a single include which looks after 
// implementation-specific issues for the programmer.
// </motivation>

// <group name="PgPlot driver definitions for stand-alone display applications"> 

extern "C" {

#if defined(__APPLE__)
  // Prototype for WorldCanvas driver

  extern void (*late_binding_pgplot_driver_1)(int *, float *, int *, char *,
			                      int *, int);
  extern void (*late_binding_pgplot_driver_2)(int *, float *, int *, char *,
			                      int *, int);
  extern void (*late_binding_pgplot_driver_3)(int *, float *, int *, char *,
			                      int *, int);
#if defined(__USE_WS_X11__)
  extern void add_late_binding_pgdriver(void (*driver)(int *, float *, int *, char *, int *, int *, int));
  extern void wcdriv_(int *, float *, int *, char *, int *, int *, int);
#define INITIALIZE_PGPLOT add_late_binding_pgdriver(wcdriv_);
#else
  extern void wcdriv_(int *, float *, int *, char *, int *, int);
  #define INITIALIZE_PGPLOT                                               \
	      if ( ! late_binding_pgplot_driver_1 )                               \
	          late_binding_pgplot_driver_1 = wcdriv_;                         \
	      else if ( ! late_binding_pgplot_driver_2 )                          \
	          late_binding_pgplot_driver_2 = wcdriv_;                         \
	      else if ( ! late_binding_pgplot_driver_3 )                          \
	          late_binding_pgplot_driver_3 = wcdriv_;                         \
	      else                                                                \
	          std::cerr << "no free late-binding driver slots" << std::endl;

#endif


#else
  #define INITIALIZE_PGPLOT


  // Prototype for WorldCanvas driver
  void wcdriv_(int *, float *, int *, char *, int*, int *, int);
  // Null driver
  void nudriv_(int *, float *, int *, char *, int *, int);

  // This global function will over-ride the default PGPlot 
  // function.  It supports only one device, that being the
  // Display Library's WorldCanvas.
  int grexec_(int *idev, int *ifunc, float *rbuf, int *nbuf,
	      char *chr, int *lchr, int len) {
    static int one = 1;
    switch (*idev) 
      {
      case 0:
	rbuf[0] = float(2);
	*nbuf = 1;
	break;
      case 1:
	nudriv_(ifunc, rbuf, nbuf, chr, lchr, len);
	break;
      case 2:
	wcdriv_(ifunc, rbuf, nbuf, chr, lchr, &one, len);
	break;
      default:
	std::cerr << "StandAloneDisplayApp: unknown pgplot device " << *idev 
	     << std::endl;
	break;
      }
    return 0;
  }
#endif

} // extern "C" 

// </group>

#endif
