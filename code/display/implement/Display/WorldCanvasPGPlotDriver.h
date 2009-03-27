//# WorldCanvasPGPlotDriver.h: global functions needed for PgPlot on the WC
//# Copyright (C) 1993,1994,1995,1996,1999,2000
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

#ifndef TRIALDISPLAY_WORLDCANVASPGPLOTDRIVER_H
#define TRIALDISPLAY_WORLDCANVASPGPLOTDRIVER_H

#include <casa/aips.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class WorldCanvas;

// <summary>
// Global driver functions need to support PgPlot on the WorldCanvas.
// </summary>

// <synopsis>
// Global driver functions are required to implement a PgPlot driver for
// the WorldCanvas.  They are called by the FORTRAN part of the PgPlot
// library.
// </synopsis>

// <group name="Driver functions called by FORTRAN PgPlot library">
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__APPLE__)
  void wcdriv_(int * opc, float * rbuf, int * nbuf,
	       char * chr, int * lchr, int len);
#else
  int grexec_(int *idev, int *ifunc, float *rbuf, int *nbuf,
	      char *chr, int *lchr, int len);
  void wcdriv_(int * opc, float * rbuf, int * nbuf,
	       char * chr, int * lchr, int *mode, int len);
#endif

#ifdef __cplusplus
}
#endif
// </group>

// <summary>
// Global utility functions need to support PgPlot on the WorldCanvas.
// </summary>

// <synopsis>
// Global utility functions are required to implement a PgPlot driver for
// the WorldCanvas.  They are provided herein.
// </synopsis>
// <group name="Global functions to attach to existing WorldCanvases">
void pgplot_queueWorldCanvas(WorldCanvas * wc);
WorldCanvas * pgplot_queryWorldCanvas(Int selectID);
void pgplot_setWorldCanvas(Int selectID, WorldCanvas * wc);
// </group>


} //# NAMESPACE CASA - END

#endif

