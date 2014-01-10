//# DisplayData.cc: Base class for display objects in the Display Library
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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

// aips includes:
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/OS/File.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/Directory.h>

// trial includes:

// display library includes:
#include <display/Display/Attribute.h>
#include <display/Display/AttributeBuffer.h>
#include <display/Display/AttValBase.h>
#include <display/DisplayEvents/WCPositionEH.h>
#include <display/DisplayEvents/WCMotionEH.h>
#include <display/DisplayEvents/WCRefreshEH.h>
#include <display/DisplayEvents/WCPositionEvent.h>
#include <display/DisplayEvents/WCMotionEvent.h>
#include <display/DisplayEvents/WCRefreshEvent.h>
#include <display/Display/Colormap.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/DisplayMethod.h>
#include <display/Display/Options.h>

// this include:
#include <display/DisplayDatas/DisplayData.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	const String DisplayData::DATA_MIN = "datamin";
	const String DisplayData::DATA_MAX = "datamax";

	DisplayData::DisplayData() :
		activeZIndex_(0), delTmpData_(False),
		rstrsConformed_(False), csConformed_(False), zIndexConformed_(False),
		displaystate(LIMBO),itsColormap(0), itsColormapWeight(-1.0),
		uiBase_(1) {
		oldWCHolder = NULL;
	};

	DisplayData::~DisplayData()
	{	
		oldWCHolder = NULL;
	};

// Add restrictions from an AttributeBuffer
	void DisplayData::addRestrictions(AttributeBuffer& otherBuf) {
		restrictions.add(otherBuf);
	}

// Add a single restriction
	void DisplayData::addRestriction(Attribute& newRestriction, Bool permanent) {
		restrictions.add(newRestriction, permanent);
	}

// add restrictions to an element
	void DisplayData::addElementRestrictions(const uInt itemNum,
	        AttributeBuffer &at) {
		if (itemNum >= nelements()) {
			return;
		}
		((DisplayMethod *)DDelement[itemNum])->addRestrictions(at);
	}

	Bool DisplayData::conformsToRstrs(const WorldCanvas& wc) {
			rstrsConformed_ = wc.matchesRestrictions(restrictions);
			if ( !rstrsConformed_ ){
				if ( classType( ) != Display::Raster ) {
					//If it isn't a raster, we don't really care about
					//the bIndex;
					rstrsConformed_ = true;
				}
			}
			return rstrsConformed_;
		}

// add a restriction to an element
	void DisplayData::addElementRestriction(const uInt itemNum,
	                                        Attribute &at,
	                                        Bool permanent) {
		if (itemNum >= nelements()) {
			return;
		}
		((DisplayMethod *)DDelement[itemNum])->addRestriction(at, permanent);
	}

// Set a restriction
	void DisplayData::setRestrictions(AttributeBuffer& otherBuf) {
		restrictions.set(otherBuf);
	}

// Set a restriction
	void DisplayData::setRestriction(Attribute& newRestriction) {
		restrictions.set(newRestriction);
	}

// set restrictions for an element
	void DisplayData::setElementRestrictions(const uInt itemNum,
	        AttributeBuffer &at) {
		if (itemNum >= nelements()) {
			return;
		}
		((DisplayMethod *)DDelement[itemNum])->setRestrictions(at);
	}

// set a restriction for an element
	void DisplayData::setElementRestriction(const uInt itemNum,
	                                        Attribute &at) {
		if (itemNum >= nelements()) {
			return;
		}
		((DisplayMethod *)DDelement[itemNum])->setRestriction(at);
	}

// remove a restriction
	void DisplayData::removeRestriction(const String& name) {
		restrictions.remove(name);
	}

// remove an element restriction
	void DisplayData::removeElementRestriction(const uInt itemNum,
	        const String &name) {
		if (itemNum >= nelements()) {
			return;
		}
		((DisplayMethod *)DDelement[itemNum])->removeRestriction(name);
	}

// clear all clearable restrictions
	void DisplayData::clearRestrictions() {
		restrictions.clear();
	}

// clear element restrictions
	void DisplayData::clearElementRestrictions(const uInt itemNum) {
		if (itemNum >= nelements()) {
			return;
		}
		((DisplayMethod *)DDelement[itemNum])->clearRestrictions();
	}

// check if a restriction with name name exists
	Bool DisplayData::existRestriction(const String& name) {
		return restrictions.exists(name);
	}

	Bool DisplayData::existElementRestriction(const uInt itemNum,
	        const String &name) {
		if (itemNum >= nelements()) {
			return False;
		}
		return ((DisplayMethod *)DDelement[itemNum])->existRestriction(name);
	}

	AttributeBuffer *DisplayData::restrictionBuffer() {
		return  &restrictions;
	}

	AttributeBuffer *DisplayData::elementRestrictionBuffer(const uInt itemNum) {
		if (itemNum >= nelements()) {
			throw(AipsError("DisplayData::elementRestrictionBuffer -"
			                "index out of range"));
		}
		// is this the only use of the friendship between DisplayData and
		// DisplayMethod?  If so, vanquish it...
		return &((DisplayMethod *)DDelement[itemNum])->restrictions;
	}


// add a position handler on the DisplayData
	void DisplayData::addPositionEventHandler(WCPositionEH *newHandler) {

		// check pointer
		if (newHandler == 0) {
			throw(AipsError("DisplayData::addPositionEventHandler - "
			                "null pointer passed"));
		}

		// create iterator
		ListIter<WCPositionEH*> iter(&itsPositionEHList);

		// add to List
		iter.addRight(newHandler);

	}

// add a motion handler on the DisplayData
	void DisplayData::addMotionEventHandler(WCMotionEH *newHandler) {
		// check pointer
		if (newHandler == 0) {
			throw(AipsError("DisplayData::addMotionEventHandler - "
			                "null pointer passed"));
		}

		// create iterator
		ListIter<WCMotionEH*> iter(&itsMotionEHList);

		// add to List
		iter.addRight(newHandler);
	}

// add a refresh handler on the DisplayData
	void DisplayData::addRefreshEventHandler(WCRefreshEH* newHandler) {

		// check pointer
		if (newHandler == 0) {
			throw(AipsError("DisplayData::addRefreshEventHandler - "
			                "null pointer passed"));
		}

		// create iterator
		ListIter<WCRefreshEH*> iter(&itsRefreshEHList);

		// add to List
		iter.addRight(newHandler);

	}
// add a display handler on the DisplayData
	void DisplayData::addDisplayEventHandler(DisplayEH* newHandler) {

		// check pointer
		if (newHandler == 0) {
			throw(AipsError("DisplayData::addDisplayEventHandler - "
			                "null pointer passed"));
		}

		// create iterator
		ListIter<DisplayEH*> iter(&itsDisplayEHList);

		// add to List
		iter.addRight(newHandler);
	}
// remove a position event handler
	void DisplayData::removePositionEventHandler(WCPositionEH& handler) {

		// create iterator
		ListIter<WCPositionEH*> iter(&itsPositionEHList);

		// loop through List
		while (!iter.atEnd()) {
			// if this is the one
			if ( &handler == iter.getRight()) {
				// delete it from list
				iter.removeRight();
				// added 1998/08/21 dgb
				break;
			}
			// addPositionEventHandler does not prevent from add the same event
			// handler twice, so have to keep on looping
			iter++;
		}
	}

// remove a motion event handler
	void DisplayData::removeMotionEventHandler(WCMotionEH& handler) {

		// create iterator
		ListIter<WCMotionEH*> iter(&itsMotionEHList);

		// loop through List
		while (!iter.atEnd()) {
			// if this is the one
			if ( &handler == iter.getRight()) {
				// delete it from list
				iter.removeRight();
				// added 1998/08/21 dgb
				break;
			}
			// addMotionEventHandler does not prevent from add the same event
			// handler twice, so have to keep on looping    // go to next
			iter++;
		}
	}

// remove a refresh handler
	void DisplayData::removeRefreshEventHandler(WCRefreshEH& handler) {

		// create iterator
		ListIter<WCRefreshEH*> iter(&itsRefreshEHList);

		// loop through List
		while (!iter.atEnd()) {
			// if this is the one
			if ( &handler == iter.getRight()) {
				// delete it from list
				iter.removeRight();
				break;
			}
			// addRefreshEventHandler does not prevent from add the same event
			// handler twice, so have to keep on looping
			iter++;
		}
	}

// remove a display handler
	void DisplayData::removeDisplayEventHandler(DisplayEH& handler) {

		// create iterator
		ListIter<DisplayEH*> iter(&itsDisplayEHList);

		// loop through List
		while (!iter.atEnd()) {
			// if this is the one
			if ( &handler == iter.getRight()) {
				// delete it from list
				iter.removeRight();
				break;
			}
			// addDisplayEventHandler does not prevent from add the same event
			// handler twice, so have to keep on looping
			iter++;
		}
	}
	void DisplayData::setColormap(Colormap *cmap, Float weight) {
		if (cmap == 0) {
			throw(AipsError("DisplayData::setColormap - "
			                "null pointer passed"));
		}
		if ((cmap == itsColormap) && (weight == itsColormapWeight)) {
			// no change!
			return;
		}

		ConstListIter<WorldCanvasHolder*> it(&itsWCHList);
		WorldCanvasHolder *wcHolder = 0;
		WorldCanvas *wcanvas = 0;
		if (weight != itsColormapWeight) {
			// will have to clear, possibly unregister, then register:
			// for this case, the resize callbacks can be expected to happen...
			it.toStart();
			while (!it.atEnd()) {
				wcHolder = it.getRight();
				wcanvas = wcHolder->worldCanvas();

				wcanvas->clear();
				if (itsColormap) {
					wcanvas->unregisterColormap(itsColormap);
				}
				wcanvas->registerColormap(cmap, weight);
				/*
				if (itsColormap) {
				wcanvas->registerColormap(cmap, itsColormap);
				     } else {
				wcanvas->registerColormap(cmap, weight);
				     }
				     */
				it++;
			}
			itsColormap = cmap;
			itsColormapWeight = weight;
		} else {
			// if we got to here, we have a different cmap, but the same
			// weight, so we'd like to replace the colormap, rather than
			// remove and reinstall - this will save us a refresh...

			// but I can't do this yet, for two reasons:
			// 1. there is no "replaceColormap" method in the infrastructure!
			// 2. it'll require further work for the case of panelling, so
			// for the moment, I just treat it like the previous case:
			it.toStart();
			while (!it.atEnd()) {
				wcHolder = it.getRight();
				wcanvas = wcHolder->worldCanvas();
				/*
				wcanvas->clear();
				if (itsColormap) {
				wcanvas->unregisterColormap(itsColormap);
				     }
				     wcanvas->registerColormap(cmap, weight);
				     */
				if (itsColormap) {
					wcanvas->registerColormap(cmap, itsColormap);
				} else {
					wcanvas->registerColormap(cmap, weight);
				}
				it++;
			}
			itsColormap = cmap;
			itsColormapWeight = weight;
		}

	}

	void DisplayData::removeColormap() {
		if (itsColormap == 0) {
			// should not throw an exception here
			return;
		}

		// create iterator and temporary
		ConstListIter<WorldCanvasHolder*> it(&itsWCHList);
		WorldCanvasHolder *wcHolder = 0;

		// and register the Colormap with all the PixelCanvases
		while (!it.atEnd()) {
			wcHolder = it.getRight();
			(wcHolder->worldCanvas())->unregisterColormap(itsColormap);
			it++;
		}
		// no ColorMap on this DisplayData
		itsColormap = 0;
	}

	void DisplayData::notifyRegister(WorldCanvasHolder *wcHolder) {
		// create iterator
		ListIter<WorldCanvasHolder*> it(&itsWCHList);

		// add to list
		it.toEnd();
		it.addRight(wcHolder);

		WorldCanvas* wc = wcHolder->worldCanvas();

		// if there is a colourmap on this DisplayData, register it with the
		// PixelCanvas (inirectly through the WorldCanvas)
		if (itsColormap !=0) wc->registerColormap(itsColormap, itsColormapWeight);
	}

	void DisplayData::refresh(Bool clean) {
		if (clean) {
			cleanup();
		}

		// create iterator
		ConstListIter<WorldCanvasHolder*> it(&itsWCHList);
		it.toStart();

		// loop
		while (!it.atEnd()) {
			(it.getRight())->refresh();
			it++;
		}
	}

	Bool DisplayData::labelAxes(const WCRefreshEvent &) {
		return False;
	}


	void DisplayData::removeFromAllWCHs() {
		// remove this DD everywhere--will stop any more refresh handling by
		// the DD.  It is a good idea for top-level DDs to call this first
		// in their destructor.

		while (True) {
			// The ListIter is recreated each time, because the WCH it points at
			// is deleted out from under it elsewhere (in notifyUnregister(), below).
			// (If someone knows it is safe to use a single ListIter here anyway,
			// they can change this code if they want to...)
			ListIter<WorldCanvasHolder*> wchs(&itsWCHList);
			if(wchs.atEnd()) return;
			WorldCanvasHolder* wch= wchs.getRight();
			if (wch==0)  {
				return;		// (shouldn't happen).
			}
			wch->removeDisplayData(*this, True);
		}
	}


	Block<Int> DisplayData::allZIndices(Int axZrng) const {
		// Return a sorted Block of all animation frame numbers currently set
		// onto all WCHs where this DD is registered.
		//# (to do: call conformsTo() and check csConformed_ as well; if it passes,
		//# just use activeZIndex_ instead of retrieving the zIndex manually).
		// The frame numbers returned are guaranteed to be in the range
		// 0 <= zIndex < axZrng, where axZrng is the total number of frames
		// on the Z axis.  axZrng can be supplied; the default is nelements().

		if (axZrng==-1) {
			axZrng = nelements();		// default.
		}
		Block<Int> zInds;
		Int size = 0;

		ConstListIter<WorldCanvasHolder*> wchs(&itsWCHList);
		for (wchs.toStart(); !wchs.atEnd(); wchs++) {
			WorldCanvasHolder* wch = wchs.getRight();
			Int zIndex;
			Bool zIndexExists;
			const AttributeBuffer *wchRestrs = wch->restrictionBuffer();
			zIndexExists = wchRestrs->getValue("zIndex",zIndex);
			if (zIndexExists && zIndex>=0 && zIndex<axZrng) {
				Int i=0;
				while(i<size && zIndex>zInds[i]) {
					i++;
				}
				if (i==size || zIndex<zInds[i]) {			  // new unique zindex
					zInds.resize(++size, True, True);		  // to insert at i:
					for (Int j=size-1; j>i; j--) {
						zInds[j]=zInds[j-1];  // make room and
					}
					zInds[i]=zIndex;
				}
			}
		}			  // insert new value.
		return zInds;
	}

	void DisplayData::notifyUnregister(WorldCanvasHolder &wcHolder,
	                                   Bool ignoreRefresh) {
		// create iterator
		ListIter<WorldCanvasHolder*> it(&itsWCHList);
		it.toStart();
		Bool removed = False;
		// loop
		while (!it.atEnd() && !removed) {
			WorldCanvasHolder* holder = it.getRight();
			if ( &wcHolder == holder ) {
				// if this is the one

				WorldCanvas* wc = wcHolder.worldCanvas();

				wc->removeIndexedImage(this);	// attempt to avoid some erroneous
				// reuse (probably unnecessary)....
				if (itsColormap !=0) {
					// if there is a colormap, unregister it
					wc->unregisterColormap(itsColormap);
				}
				// remove WorldCanvasHolder from list
				oldWCHolder = holder;
				it.removeRight();

				removed = True;
			} else {
				it++;
			}
		}

		// now we must clean up this DD, and it'll get redrawn now on any
		// other WCHs it's registered on: need to fix this to properly
		// support multiple WCH.
		if (removed) {
			if (!ignoreRefresh) {
				refresh(True);
			} else {
				// ignoreRefresh indicates that we just want to clean up and not
				// refresh, otherwise we get unneccessary refreshs on multiple
				// WCH
				cleanup();
			}
		}
	}


	Bool DisplayData::isCSmaster(const WorldCanvasHolder* wch) const {

		// Is this DD the CS master of the passed WCH?

		if(wch!=0) return wch->isCSmaster(this);

		// Defaulting wch to zero asks whether this DD is CS master of
		// _some_ WCH on which it is registered.  This option is mostly
		// a kludge, since the DD may be CS master of some WCHs and not
		// others if there is more than one display panel.

		ConstListIter<WorldCanvasHolder*> wchs(&itsWCHList);
		for (wchs.toStart(); !wchs.atEnd(); wchs++) {
			if(wchs.getRight()->isCSmaster(this)) return True;
		}

		return False;
	}


	void DisplayData::setDefaultOptions() {
		// nothing yet
	}

	Bool DisplayData::setOptions(Record &, Record &) {
		// nothing yet
		return False;
	}

	Record DisplayData::getOptions() {
		// nothing yet
		Record rec;
		return rec;
	}

// set an Attribute
	void DisplayData::setAttribute(Attribute& at) {
		attributes.set(at);
	}

// set attributes
	void DisplayData::setAttributes(AttributeBuffer& at) {
		attributes.set(at);
	}


/// user interface to the attribute buffer
	Bool DisplayData::getAttributeValue(const String& name, uInt& newValue) {
		return attributes.getValue(name, newValue);
	}

	Bool DisplayData::getAttributeValue(const String& name, Int& newValue) {
		return attributes.getValue(name, newValue);
	}

	Bool DisplayData::getAttributeValue(const String& name, Float& newValue) {
		return attributes.getValue(name, newValue);
	}

	Bool DisplayData::getAttributeValue(const String& name, Double& newValue) {
		return attributes.getValue(name, newValue);
	}

	Bool DisplayData::getAttributeValue(const String& name, Bool& newValue) {
		return attributes.getValue(name, newValue);
	}

	Bool DisplayData::getAttributeValue(const String& name, String& newValue) {
		return attributes.getValue(name, newValue);
	}


	Bool DisplayData::getAttributeValue(const String& name, Vector<uInt>& newValue) {
		return attributes.getValue(name, newValue);
	}
	Bool DisplayData::getAttributeValue(const String& name, Vector<Int>& newValue) {
		return attributes.getValue(name, newValue);
	}
	Bool DisplayData::getAttributeValue(const String& name, Vector<Float>& newValue) {
		return attributes.getValue(name, newValue);
	}
	Bool DisplayData::getAttributeValue(const String& name, Vector<Double>& newValue) {
		return attributes.getValue(name, newValue);
	}
	Bool DisplayData::getAttributeValue(const String& name, Vector<Bool>& newValue) {
		return attributes.getValue(name, newValue);
	}
	Bool DisplayData::getAttributeValue(const String& name, Vector<String>& newValue) {
		return attributes.getValue(name, newValue);
	}


// check if a certain Attribute exists
	Bool DisplayData::existsAttribute(String& name) {
		return attributes.exists(name);
	}


// remove an Attribute
	void DisplayData::removeAttribute(String& name) {
		attributes.remove(name);
	}


	AttValue::ValueType DisplayData::attributeType(String& name) {
		return attributes.getDataType(name);
	}


	void DisplayData::setAttributeOnPrimaryWCHs(Attribute &at) {
		// create iterator
		ConstListIter<WorldCanvasHolder*> it(&itsWCHList);
		it.toStart();
		// loop
		WorldCanvasHolder *wch = 0;
		while (!it.atEnd()) {
			wch = it.getRight();
			if (wch->isCSmaster(this)) {
				wch->worldCanvas()->setAttribute(at);
			}
			it++;
		}
	}

	const WorldCanvasHolder *DisplayData::findHolder(const WorldCanvas *wCanvas) const {
		ConstListIter<WorldCanvasHolder*> it(&itsWCHList);
		WorldCanvasHolder *tholder = 0;
		it.toStart();
		while (!it.atEnd() && !tholder) {
			tholder = it.getRight();
			if (tholder->worldCanvas() != wCanvas) {
				tholder = 0;
				it++;
			}
		}
		if ( tholder == NULL && wCanvas->csMaster() == this ){
			tholder = oldWCHolder;
		}
		return tholder;
	}

	WorldCanvasHolder *DisplayData::findHolder(const WorldCanvas *wCanvas) {
		ConstListIter<WorldCanvasHolder*> it(&itsWCHList);
		WorldCanvasHolder *tholder = 0;
		it.toStart();
		while (!it.atEnd() && !tholder) {
			tholder = it.getRight();
			if (tholder->worldCanvas() != wCanvas) {
				tholder = 0;
				it++;
			}
		}
		if ( tholder == NULL && wCanvas->csMaster() == this ){
			tholder = oldWCHolder;
		}
		return tholder;
	}


	void DisplayData::delTmpData(String &tmpData) {
		Path tmpPath(tmpData);
		Path tmpDir(String(viewer::options.tmp( )));
		Path tmpPathDir(tmpPath.dirName());
		String tmpPathDirString(tmpPathDir.absoluteName());
		if (tmpPathDirString.find(tmpDir.absoluteName())!=0) {
			return;
		}
		File tmpFile(tmpData);
		if (tmpFile.exists () && tmpFile.isWritable()) {
			if (tmpFile.isRegular()) {
				RegularFile tmpRegFile(tmpData);
				tmpRegFile.remove();
			} else {
				Directory tmpDir(tmpData);
				tmpDir.removeRecursive(False);
			}
		}
	}


	void DisplayData::positionEH(const WCPositionEvent &ev) {
		WCPositionEH *eh = 0;
		ConstListIter<WCPositionEH*> iter(&itsPositionEHList);
		iter.toStart();
		while (!iter.atEnd()) {
			eh = iter.getRight();
			if (eh) {
				(*eh)(ev);
			}
			iter++;
		}
	}

	void DisplayData::motionEH(const WCMotionEvent &ev) {
		WCMotionEH *eh = 0;
		ConstListIter<WCMotionEH*> iter(&itsMotionEHList);
		iter.toStart();
		while (!iter.atEnd()) {
			eh = iter.getRight();
			if (eh) {
				(*eh)(ev);
			}
			iter++;
		}
	}
// overide DisplayEH::handleEvent to forward DisplayEvents
	void DisplayData::handleEvent(DisplayEvent &ev) {
		DisplayEH *eh = 0;
		ConstListIter<DisplayEH*> iter(&itsDisplayEHList);
		iter.toStart();
		while (!iter.atEnd()) {
			eh = iter.getRight();
			if (eh) {
				eh->handleEvent(ev);
			}
			iter++;
		}
	}

	DisplayData::DisplayData(const DisplayData &other) :
		DisplayOptions(other), DisplayEH(), DlTarget() {
		displaystate = LIMBO;
		oldWCHolder = other.oldWCHolder;
	}

	void DisplayData::operator=(const DisplayData & other) {
		oldWCHolder = other.oldWCHolder;
	}

} //# NAMESPACE CASA - END

