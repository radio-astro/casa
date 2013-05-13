//# PSWorldCanvasApp.h: simple PostScript WorldCanvas application
//# Copyright (C) 1999,2000
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

#ifndef TRIALDISPLAY_PSWORLDCANVASAPP_H
#define TRIALDISPLAY_PSWORLDCANVASAPP_H

//# aips includes:
#include <casa/aips.h>

//# trial includes:

//# display library includes:
#include <display/Display/PSPixelCanvas.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PSDriver.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Simple PostScript WorldCanvas application
// </summary>
//
// <prerequisite>
//   <li> <linkto class="PSPixelCanvas">PSPixelCanvas</linkto>
//   <li> <linkto class="WorldCanvas">WorldCanvas</linkto>
// </prerequisite>
//
// <etymology>
// PSWorldCanvasApp stands for PostScript WorldCanvas Application
// </etymology>
//
// <synopsis>
// PSWorldCanvasApp provides a basic infrastructure, including
// a PostScript PixelCanvas and a WorldCanvas, which can be used
// to write a simple application providing PostScript output.
// </synopsis>
//
// <motivation>
// Test class for the PostScript PixelCanvas
// </motivation>

	class PSWorldCanvasApp {

	public:

		// default constructor
		PSWorldCanvasApp(PSDriver *ps);

		// default destructor
		virtual ~PSWorldCanvasApp();

		// retrieve a pointer to the WorldCanvas
		WorldCanvas *worldCanvas() const {
			return itsWorldCanvas;
		}

		// Run the application
		void run();

		PSDriver *getDriver()const {
			return ps_;
		}
		PSPixelCanvas *pixelCanvas()const {
			return itsPixelCanvas;
		}
	protected:

	private:

		// Pointer to the PixelCanvas
		PSPixelCanvas *itsPixelCanvas;

		// Pointer to the WorldCanvas
		WorldCanvas *itsWorldCanvas;

		PSDriver *ps_;
	};


} //# NAMESPACE CASA - END

#endif
