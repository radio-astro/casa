//# CachingDisplayMethod.cc: assistance for auto-caching DisplayData objects
//# Copyright (C) 1999,2000,2001
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

#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/CachingDisplayData.h>
#include <display/DisplayDatas/CachingDisplayMethod.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor.
	CachingDisplayMethod::CachingDisplayMethod(WorldCanvas *worldCanvas,
	        AttributeBuffer *wchAttributes,
	        AttributeBuffer *ddAttributes,
	        CachingDisplayData *
	        parentDisplayData) :
		DisplayMethod(parentDisplayData),
		itsWorldCanvas(worldCanvas),
		itsHasList(False),
		itsCachedDrawingList(0) {
		addRestrictions(*wchAttributes);
		addRestrictions(*ddAttributes);
	}

// Destructor.
	CachingDisplayMethod::~CachingDisplayMethod() {
		if (itsWorldCanvas) {
			if (itsHasList && itsWorldCanvas->validList(itsCachedDrawingList)) {
				itsWorldCanvas->deleteList(itsCachedDrawingList);
			}
		}
	}

// Draw this slice of data on the supplied WorldCanvasHolder.
	void CachingDisplayMethod::draw(Display::RefreshReason reason,
	                                WorldCanvasHolder &wcHolder) {
		WorldCanvas *wc = wcHolder.worldCanvas();
		if (wc != itsWorldCanvas) {
			throw(AipsError("Invalid WorldCanvas given to "
			                "CachingDisplayMethod::draw"));
		}

		if ( !itsHasList || !wc->validList(itsCachedDrawingList) ) {
			Bool couldDraw;


			itsCachedDrawingList = wc->newList();
			try {
				couldDraw = drawIntoList(reason, wcHolder);
			}

			catch (...) {	// At least clean up drawlist state...
				wc->endList();
				if(wc->validList(itsCachedDrawingList))
					wc->deleteList(itsCachedDrawingList);
				itsHasList = False;
				throw;
			}		//  ...before passing exception on.

			wc->endList();

			// if DM can't draw, do not save the drawlist.
			if(! couldDraw && wc->validList(itsCachedDrawingList) )
				wc->deleteList(itsCachedDrawingList);

			itsHasList = couldDraw && wc->validList(itsCachedDrawingList);
		}


		if(itsHasList) wc->drawList(itsCachedDrawingList);
	}

// (Required) default constructor.
	CachingDisplayMethod::CachingDisplayMethod() {
	}

// (Required) copy constructor.
	CachingDisplayMethod::CachingDisplayMethod(const CachingDisplayMethod &o) : DisplayMethod(o) {
	}

// (Required) copy assignment.
	void CachingDisplayMethod::operator=(const CachingDisplayMethod &) {
	}

} //# NAMESPACE CASA - END

