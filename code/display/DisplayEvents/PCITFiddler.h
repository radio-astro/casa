//# PCITFiddler.h: Colormap fiddling on the PixelCanvas
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

#ifndef TRIALDISPLAY_PCITFIDDLER_H
#define TRIALDISPLAY_PCITFIDDLER_H

#include <casa/aips.h>
#include <display/DisplayEvents/PCInvisTool.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Colormap fiddling on the PixelCanvas.
// </summary>
//
// <synopsis>
// This class provides colormap fiddling via an invisible
// tool which gets registered with a specific PixelCanvas.
// </synopsis>

	class PCITFiddler : public PCInvisTool {

	public:

		enum FiddleType {
		    StretchAndShift,
		    BrightnessAndContrast
		};

		// Constructor.  Requires a PixelCanvas to fiddle upon, and
		// optional specification of the fiddling type and key to
		// detect.
		PCITFiddler(PixelCanvas *pcanvas,
		            const PCITFiddler::FiddleType &fiddletype =
		                PCITFiddler::StretchAndShift,
		            Display::KeySym keysym = Display::K_Pointer_Button1);

		// Destructor.
		virtual ~PCITFiddler();

		// Set/get the fiddle type.
		// <group>
		virtual void setFiddleType(const PCITFiddler::FiddleType &fiddletype);
		virtual PCITFiddler::FiddleType getFiddleType() {
			return itsFiddleType;
		}
		// </group>

		// This function is called when a new position is ready.  We
		// implement it to fiddle the Colormap in this class.
		virtual void positionReady();

	private:

		PCITFiddler::FiddleType itsFiddleType;

	};


} //# NAMESPACE CASA - END

#endif
