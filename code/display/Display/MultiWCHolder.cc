//# MultiWCHolder.cc: Holder of multiple WorldCanvasHolders for panelling
//# Copyright (C) 2000,2001,2002,2003
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

#include <casa/aips.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <display/Display/AttributeBuffer.h>
#include <display/Display/AttValTol.h>
#include <display/Display/MultiWCHolder.h>
#include <casa/BasicMath/Math.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Default constructor.
	MultiWCHolder::MultiWCHolder() :
		itsBLength(0), itsBIndex(0),
		itsHoldCount(0),
		itsRefreshHeld(False) {
		setBIndexName();
	}

// Constructor for a single WorldCanvasHolder.
	MultiWCHolder::MultiWCHolder(WorldCanvasHolder &holder) :
		itsHoldCount(0),
		itsRefreshHeld(False) {
		setBIndexName();
		addWCHolder(holder);
	}

// Destructor.
	MultiWCHolder::~MultiWCHolder() {
	}

// Add/remove WorldCanvasHolder/s.
	void MultiWCHolder::addWCHolder(WorldCanvasHolder &holder) {
		if (isAlreadyRegistered(holder)) {
			return;
		}
		ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
		localWCHLI.toEnd();
		localWCHLI.addRight(&holder);
		for (Int i = 0; i < itsHoldCount; i++) {
			holder.worldCanvas()->hold();
		}
		installRestrictions(holder);
		addAllDisplayDatas(holder);
	}
	void MultiWCHolder::removeWCHolder(WorldCanvasHolder &holder) {
		if (!isAlreadyRegistered(holder)) {
			return;
		}
		removeAllDisplayDatas(holder,True);
		ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
		localWCHLI.toStart();
		while (!localWCHLI.atEnd()) {
			if (&holder == localWCHLI.getRight()) {
				localWCHLI.removeRight();
				return;
			} else {
				localWCHLI++;
			}
		}
	}
	void MultiWCHolder::removeWCHolders() {
		ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
		localWCHLI.toStart();
		while (!localWCHLI.atEnd()) {
			localWCHLI.removeRight();
			if (!localWCHLI.atEnd()) {
				localWCHLI++;
			}
		}
	}

// Add/remove DisplayData/s.
	void MultiWCHolder::addDisplayData(DisplayData &displaydata, int position) {
		if (isAlreadyRegistered(displaydata)) {
			return;
		}
		hold();
		ListIter<DisplayData *> localDDLI(itsDDList);
		if ( position < 0 || position >= static_cast<int>(itsDDList.len()) ) {
			localDDLI.toEnd();
		}
		else {
			int i = 0;
			localDDLI.toStart();
			while ( i < position ) {
				localDDLI++;
				i++;
			}
		}
		localDDLI.addRight(&displaydata);
		addToAllWorldCanvasHolders(displaydata, position);

		// Add a 'bIndex' restriction to newly-added DD.  It can be used to
		// alternate display of the various DDs by placing a similar restriction
		// on the WCHs.  The index should reflect its order in the list.  However,
		// contours, vectors, etc are not displayed separately so the index needs
		//to take that into account.
		if ( isBlinkDD(&displaydata) ){

			itsBLength++;
			itsBlinkDDs.resize( itsBLength, True );
		}

		ListIter<DisplayData *> iter(itsDDList);
		iter.toStart();
		int index = 0;
		while (!iter.atEnd()) {
			DisplayData* dd = iter.getRight();
			if ( dd ){
				if ( isBlinkDD( dd) ) {
					itsBlinkDDs[index] = dd;

				}
				Attribute bIndexAtt(itsBIndexName, index );
				dd->setRestriction( bIndexAtt );
				if ( isBlinkDD(dd)){
					index++;
				}
			}
			iter++;
		}

		refresh();
		release();
	}


	void MultiWCHolder::removeDisplayData(DisplayData &displaydata) {

		if (!isAlreadyRegistered(displaydata)) return;

		hold();
		removeFromAllWorldCanvasHolders(displaydata);
		ListIter<DisplayData *> localDDLI(itsDDList);
		localDDLI.toStart();
		DisplayData* dd = &displaydata;
		while (!localDDLI.atEnd()) {
			if (dd == localDDLI.getRight()) {

				// DD found in main list--remove it..

				localDDLI.removeRight();

				if(isBlinkDD(dd)) {

					// Remove from blink DD 'list' (Block) too.  Adjust
					// blink index restrictions as necessary.

					dd->removeRestriction(itsBIndexName);
					// No point in leaving blink restriction hanging on the dd.

					bool found=false;

					for(Int ddBIndex=0; ddBIndex<itsBLength; ddBIndex++) {
						DisplayData* searchDD =
						    static_cast<DisplayData*>(itsBlinkDDs[ddBIndex]);


						if(dd==searchDD) {
							// dd found in blinkDD list--it will be removed.
							found=true;

							if(itsBIndex>ddBIndex){

								itsBIndex--;
							}
						}
						else if(found) {

							// DDs past the one being removed move back in the blinkDD list.
							// Their bIndex restriction must also be decremented.


							Int newddBIndex=ddBIndex-1;

							itsBlinkDDs[newddBIndex]=searchDD;
							Attribute bIndexAtt(itsBIndexName, newddBIndex);
							searchDD->setRestriction(bIndexAtt);

						}

					}
					// itsBIndex is communicated to the animator, and becomes the
					// WCH blink restriction setting.  It should be decremented
					// if it was selecting a DD past the one deleted, in order
					// to continue selecting the same DD.

					if(found) {		// (should be True).
						itsBLength--;
						if ( itsBLength >= 0 ){
							itsBlinkDDs[itsBLength] = NULL;
						}
						itsBIndex = max(0, min(itsBLength-1, itsBIndex));
						// Assure itsBIndex is in proper range
						break;
					}

				}
			}

			else localDDLI++;
		}

		refresh();
		release();
	}


	void MultiWCHolder::removeDisplayDatas() {
		hold();
		ListIter<DisplayData *> localDDLI(itsDDList);
		localDDLI.toStart();
		while (!localDDLI.atEnd()) {
			DisplayData* dd = localDDLI.getRight();
			removeFromAllWorldCanvasHolders(*dd);
			localDDLI.removeRight();
			if(isBlinkDD(dd)) dd->removeRestriction(itsBIndexName);
		}

		itsBLength = itsBIndex = 0;
		refresh();
		release();
	}


// Install/remove restriction/s.
	void MultiWCHolder::setRestriction(const Attribute &restriction) {
		itsAttributes.set(restriction);
		distributeRestrictions();
	}
	void MultiWCHolder::setRestrictions(const AttributeBuffer &restrictions) {
		itsAttributes.set(restrictions);
		distributeRestrictions();
	}
	void MultiWCHolder::removeRestriction(const String &name) {
		String nm = (name=="bIndex")?  itsBIndexName : name;
		itsAttributes.remove(nm);
		ListIter<WorldCanvasHolder*> wchs(itsWCHList);
		for(; !wchs.atEnd(); wchs++) wchs.getRight()->removeRestriction(nm);
	}

	void MultiWCHolder::removeRestrictions() {
		itsAttributes.clear();
		distributeRestrictions();
		// dk note: line above accomplishes nothing; restrictions remain on
		// WCHs at present, i.e. this routine doesn't work.
		// (Implementation was never finished; to be fixed).
	}

// Distribute restrictions linearly.
	void MultiWCHolder::setLinearRestrictions(AttributeBuffer &restrictions,
	        const AttributeBuffer &increments) {
		ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
		localWCHLI.toStart();

		AttributeBuffer rstrs=restrictions;
		adjustBIndexName(rstrs);
		AttributeBuffer incrs=increments;
		adjustBIndexName(incrs);
		// Same buffers, except with modified name of 'bIndex' attribute.

		Int bInd;
		Bool BIExists = ( itsBLength>0 &&
		                  rstrs.getValue(itsBIndexName, bInd) &&
		                  bInd>=0 );
		// There are blink DDs to control, and a bIndex
		// restriction (with a reasonable value) exists.

		if(BIExists) itsBIndex=bInd;
		// Maintain internal record of animator bIndex setting.
		// When DDs are removed, its appropriate value may change,
		// and is communicated back to the animator.

		while (!localWCHLI.atEnd()) {
			localWCHLI.getRight()->setRestrictions(rstrs);
			if (!localWCHLI.atEnd()) {
				restrictions += increments;
				// to retain (dubious) semantics of 'restrictions' return value...
				rstrs += incrs;

				if(BIExists) {

					// Do a modulo-length adjustment to blink index, so that there
					// are no empty panels.  (In my opinion, this should be done for
					// zIndex as well.  (dk)).

					rstrs.getValue(itsBIndexName, bInd);
					if(bInd<0 || bInd>=itsBLength) {
						bInd = max(0,bInd) % itsBLength;
						restrictions.set("bIndex", bInd);
						rstrs.set(itsBIndexName, bInd);
					}
				}

			}
			localWCHLI++;
		}
		refresh();
	}

	void MultiWCHolder::hold() {
		itsHoldCount++;
		ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
		localWCHLI.toStart();
		while (!localWCHLI.atEnd()) {
			localWCHLI.getRight()->worldCanvas()->hold();
			localWCHLI++;
		}
	}

	void MultiWCHolder::release() {
		itsHoldCount--;
		if (itsHoldCount <= 0) {
			itsHoldCount = 0;
			if (itsRefreshHeld) {
				refresh(itsHeldReason);
			}
			itsRefreshHeld = False;
		}
		ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
		localWCHLI.toStart();
		while (!localWCHLI.atEnd()) {
			localWCHLI.getRight()->worldCanvas()->release();
			localWCHLI++;
		}
	}

	void MultiWCHolder::refresh(const Display::RefreshReason &reason) {
		if (itsHoldCount) {
			if (!itsRefreshHeld) { // store only first reason
				itsRefreshHeld = True;
				itsHeldReason = reason;
			}
		} else {
			clear();
			ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
			localWCHLI.toStart();
			while (!localWCHLI.atEnd()) {
				localWCHLI.getRight()->refresh(reason);
				localWCHLI++;
			}
		}
	}

// Do we already have this WorldCanvasHolder/DisplayData registered?
	Bool MultiWCHolder::isAlreadyRegistered(const WorldCanvasHolder
	        &holder) {
		ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
		localWCHLI.toStart();
		while (!localWCHLI.atEnd()) {
			if (&holder == localWCHLI.getRight()) {
				return True;
			}
			localWCHLI++;
		}
		return False;
	}
	Bool MultiWCHolder::isAlreadyRegistered(const DisplayData &displaydata) {
		ListIter<DisplayData *> localDDLI(itsDDList);
		localDDLI.toStart();
		while (!localDDLI.atEnd()) {
			if (&displaydata == localDDLI.getRight()) {
				return True;
			}
			localDDLI++;
		}
		return False;
	}

// Add/remove all the DisplayDatas to/from a WorldCanvasHolder.
	void MultiWCHolder::addAllDisplayDatas(WorldCanvasHolder &holder) {
		ListIter<DisplayData *> localDDLI(itsDDList);
		localDDLI.toStart();
		while (!localDDLI.atEnd()) {
			holder.addDisplayData(localDDLI.getRight(), -1);
			localDDLI++;
		}
	}
	void MultiWCHolder::removeAllDisplayDatas(WorldCanvasHolder &holder,
	        const Bool& /*permanent*/) {
		ListIter<DisplayData *> localDDLI(itsDDList);
		localDDLI.toStart();
		while (!localDDLI.atEnd()) {
			holder.removeDisplayData(*(localDDLI.getRight()), True);
			localDDLI++;
		}
	}

// Add/remove a DisplayData to/from all WorldCanvasHolders.
	void MultiWCHolder::addToAllWorldCanvasHolders(DisplayData &displaydata, int position) {
		ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
		localWCHLI.toStart();
		while (!localWCHLI.atEnd()) {
			localWCHLI.getRight()->addDisplayData(&displaydata, position);
			localWCHLI++;
		}
	}
	void MultiWCHolder::removeFromAllWorldCanvasHolders(DisplayData &displaydata) {
		ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
		localWCHLI.toStart();
		while (!localWCHLI.atEnd()) {
			localWCHLI.getRight()->removeDisplayData(displaydata);
			localWCHLI++;
		}
	}
// Distribute blinkMode to all WorldCanvasHolders.
	void MultiWCHolder::setBlinkMode( bool mode ) {
		ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
		localWCHLI.toStart();
		while (!localWCHLI.atEnd()) {
			WorldCanvasHolder *holder = localWCHLI.getRight();
			holder->setBlinkMode(mode);
			localWCHLI++;
		}
	}
// Distribute restrictions to all WorldCanvasHolders.
	void MultiWCHolder::distributeRestrictions() {
		ListIter<WorldCanvasHolder *> localWCHLI(itsWCHList);
		localWCHLI.toStart();
		while (!localWCHLI.atEnd()) {
			WorldCanvasHolder *holder = localWCHLI.getRight();
			//holder->removeRestrictions();
			holder->setRestrictions(itsAttributes);
			localWCHLI++;
		}
	}
// Install restrictions on a specific WorldCanvasHolder.
	void MultiWCHolder::installRestrictions(WorldCanvasHolder &holder) {
		if (isAlreadyRegistered(holder)) {
			//holder.removeRestrictions();
			holder.setRestrictions(itsAttributes);
		}
	}
// This will return the maximum 'nelements' (Z axis length) of all
// dds compatible with current canvas coordinates.  (Continuum image
// can be viewed along with selected channel of spectral image, e.g.).
	uInt MultiWCHolder::zLength() {
		uInt length = 0;
		if (itsWCHList.len() > 0) {
			ListIter<WorldCanvasHolder*> wchs(itsWCHList);
			length = wchs.getRight()->nelements();
		}
		// Returns the value of the first wch (should be the same for
		// all of them).
		return length;
	}

// Determines which DDs will be restricted, which are always active.
// May need refinement later; for now, blink Raster PADDs only; do not
// restrict other DDs.  (Contour DDs will always show, e.g.).
// (Note that GTkPanelDisplay assumes that isBlinkDD() is False for
// GTkDrawingDDs, at present).
// (12/04: This should probably be a DD method instead, so MWCH doesn't
// need to know about various DD classes...).
	Bool MultiWCHolder::isBlinkDD(DisplayData *dd) {
		return  dd->classType() == Display::Raster   &&
		        dynamic_cast<PrincipalAxesDD*>(dd) != 0;
	}

// (permanently) sets itsBIndexName (below).  Called only in constructor.
	void MultiWCHolder::setBIndexName() {
		ostringstream os;
		os<<"bIndex"<<this;
		itsBIndexName=String(os);
	}

// Adjust "bIndex" Attribute's name to include ID of this MWCH.
	void MultiWCHolder::adjustBIndexName(AttributeBuffer& rstrs) {
		if(!rstrs.exists("bIndex")) return;
		Attribute bIndexAtt(itsBIndexName, *(rstrs.getAttributeValue("bIndex")));
		rstrs.remove("bIndex");
		rstrs.set(bIndexAtt);
	}

// Return number of blink DDs, current appropriate blink index.  Sent to
// animator (by GtkPanelDisplay, actually) when DDs are added, removed.
// The animator in turn actually orders the bIndex 'LinearRestrictions'
// to be set or removed on the WCHs.
	Int MultiWCHolder::bLength() {
		return itsBLength;
	}
	Int MultiWCHolder::bIndex() {
		return itsBIndex;
	}



	Bool MultiWCHolder::conforms(DisplayData* dd,
	                             Bool testRstrs, Bool testCS, Bool testZ,
	                             Int wchIndex) {
		// Test conformance of a DD to a WCH of this MWCH (by default, test the
		// first one (WCH 0) which always exists).  The three aspects of
		// conformance can be selectively tested.

		WorldCanvasHolder* wch=0;
		ListIter<WorldCanvasHolder*> wchs(itsWCHList);

		for(Int i=0;  ;   i++, wchs++)  {
			if(wchs.atEnd()) return False;
			if(i==wchIndex) break;
		}

		wch=wchs.getRight();
		if(wch==0 || dd==0) return False;

		return (!testZ     || dd->conformsToZIndex(*wch->worldCanvas()))  &&
		       (!testCS    || dd->conformsToCS(*wch->worldCanvas()))      &&
		       (!testRstrs || dd->conformsToRstrs(*wch->worldCanvas()));
	}


} //# NAMESPACE CASA - END

