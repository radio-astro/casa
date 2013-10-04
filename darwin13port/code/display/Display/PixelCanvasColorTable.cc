//# PixelCanvasColorTable.cc: base implementation of color resources for X
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001
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

#include <display/Display/PixelCanvasColorTable.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	PixelCanvasColorTable::PixelCanvasColorTable() :
		defaultColormap_(0),
		dcmapMgr_(this) {
		if (defaultColormap_ == 0) {
			//cout << "Creating the default colormap, <default>" << endl;
			// this is the built-in colormap alias <default>
			defaultColormap_ = new Colormap("<default>");
		}
	}

	PixelCanvasColorTable::~PixelCanvasColorTable() {
		delete defaultColormap_;
		defaultColormap_=0;
	}

//
//  RESIZE CALLBACK
//

	void PixelCanvasColorTable::addResizeCallback(PixelCanvasColorTableResizeCB cb, void * clientData) {
		ListIter<void *> it(resizeCBList_);
		ListIter<void *> dit(clientDataList_);
		it.toEnd();
		dit.toEnd();
		it.addRight((void *) cb);
		dit.addRight(clientData);
	}

	void PixelCanvasColorTable::removeResizeCallback(PixelCanvasColorTableResizeCB cb, void * clientData) {
		ListIter<void *> it(resizeCBList_);
		ListIter<void *> dit(clientDataList_);
		while(!it.atEnd() && !dit.atEnd()) {
			if (it.getRight() == (void *) cb && dit.getRight() == clientData) {
				it.removeRight();
				dit.removeRight();
				break;
			}
			if (!it.atEnd()) {
				it++;
			}
			if (!dit.atEnd()) {
				dit++;
			}
		}
	}

	void PixelCanvasColorTable::doResizeCallbacks(const Display::RefreshReason
	        &reason) {
		// commented out: this is NOT part of the resizeCallbacks, this
		// method should be CALLED by redistributeColormaps();
		//cout << "Pcctbl : (builtin) redistributing DataColormaps..." << endl;
		//dcmapMgr_.redistributeColormaps();

		//cout << "Pcctbl : doing resize callbacks..." << endl;
		ListIter<void *> it(resizeCBList_);
		ListIter<void *> dit(clientDataList_);
		while(!it.atEnd()) {
			((PixelCanvasColorTableResizeCB) it.getRight())(this, nColors(), dit.getRight(), reason);
			it++;
			dit++;
		}
		//cout << "Pcctbl : done with resize callbacks." << endl;
	}

	void PixelCanvasColorTable::registerColormap(Colormap * cmap, Float weight) {
		dcmapMgr_.registerColormap(cmap, weight);
	}

	void PixelCanvasColorTable::registerColormap(Colormap *cmap,
	        Colormap *cmapToReplace) {
		dcmapMgr_.registerColormap(cmap, cmapToReplace);
	}

	void PixelCanvasColorTable::unregisterColormap(Colormap * cmap) {
		dcmapMgr_.unregisterColormap(cmap);
	}

	uInt PixelCanvasColorTable::getColormapSize(const Colormap * cmap) const {
		return dcmapMgr_.getColormapSize(cmap);
	}

	Bool PixelCanvasColorTable::member(const Colormap * cmap) const {
		return dcmapMgr_.member(cmap);
	}

	uInt PixelCanvasColorTable::getColorAmount( const uInt* posMatrix, const uInt* endMatrix,
			int shiftAmount, int colorCount )const{
		uInt amountColor = 0;
		const int MAX_COLOR = 255;
		if ( posMatrix < endMatrix ){
			uInt colorIndex = *posMatrix;

			float percentColor = (colorIndex * 1.0f) / colorCount;
			amountColor = static_cast<uInt>(percentColor * MAX_COLOR);

			if ( shiftAmount > 0 ){
				amountColor = amountColor << shiftAmount;
			}
		}
		return amountColor;
	}

	void PixelCanvasColorTable::mapToColorRGB(const Colormap* map, Array<uInt>& outArray,
	                                const Array<uInt>& inArrayRed,
	                                const Array<uInt>& inArrayGreen,
	                                const Array<uInt>& inArrayBlue) const {
		// Figure out the size of the output array based on the sizes of
		// the input arrays.
		uInt redSize = inArrayRed.nelements();
		uInt blueSize = inArrayBlue.nelements();
		uInt greenSize = inArrayGreen.nelements();
		Array<uInt> inArray;
		if ( redSize > 0 ){
			inArray = inArrayRed;
		}
		else if ( blueSize > 0 ){
			inArray = inArrayBlue;
		}
		else {
			inArray = inArrayGreen;
		}
		uInt count = inArray.nelements();
		if(outArray.nelements() < count ) {
			outArray.resize(inArray.shape());
		}

		//Initialize the pointers.
		Bool inDel, outDel;
		const uInt* inRed  =  inArrayRed.getStorage(inDel);
		const uInt* inBlue = inArrayBlue.getStorage(inDel);
		const uInt* inGreen = inArrayGreen.getStorage(inDel );
		uInt* out = outArray.getStorage(outDel);

		const uInt* endRed = inRed + inArrayRed.nelements();
		const uInt* endBlue = inBlue + inArrayBlue.nelements();
		const uInt* endGreen = inGreen + inArrayGreen.nelements();

		uInt cmapsize = getColormapSize(map);
		const uInt* inpRed  = inRed;
		const uInt* inpBlue = inBlue;
		const uInt* inpGreen = inGreen;
		uInt* outp = out;

		//Compute the combined color matrix.
		uInt maxRed =0;
		uInt maxBlue = 0;

		for ( uInt i = 0; i < count; i++ ){
			*outp = 0;
			if ( *inpRed > maxRed ){
				maxRed = *inpRed;
			}
			if ( *inpBlue > maxBlue ){
				maxBlue = *inpBlue;
			}

			//Get the red, blue, and green amount based on the indices in
			//the red, blue, and green matrices.
			uInt amountRed = getColorAmount(inpRed, endRed, 16, cmapsize);
			uInt amountGreen = getColorAmount(inpGreen, endGreen, 8, cmapsize);
			uInt amountBlue = getColorAmount(inpBlue, endBlue, 0, cmapsize);

			//Combine the individual colors.
			*outp = amountRed + amountGreen + amountBlue;

			//Increment the pointers;
			inpRed++;
			inpGreen++;
			inpBlue++;
			outp++;
		}

		//Free the storage
		inArrayRed.freeStorage(inRed, inDel);
		inArrayBlue.freeStorage(inBlue, inDel );
		inArrayGreen.freeStorage(inGreen, inDel );

		//Allocate the storage.
		outArray.putStorage(out, outDel);
	}


} //# NAMESPACE CASA - END

