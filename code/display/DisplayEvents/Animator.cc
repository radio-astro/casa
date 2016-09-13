//# Animator.cc: Movie control for WorldCanvasHolders
//# Copyright (C) 1996,1997,1998,1999,2000
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

//# aips includes
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/BasicMath/Math.h>

//# trial includes

//# display library includes
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/Display/Attribute.h>
#include <display/Display/AttributeBuffer.h>

//# this include
#include <display/DisplayEvents/Animator.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	AnimatorRefEH::AnimatorRefEH(Animator *animator) :
		itsAnimator(animator) { }
	void AnimatorRefEH::operator()(const WCRefreshEvent &ev) {
		(*itsAnimator)(ev);
	}

// Constructor
// Set parameters to default values
	Animator::Animator()
		: attBufList(0),
		  minCoord(0.0),
		  maxCoord(0.0),
		  movieStep(1.0),
		  coordTolerance(0.1),
		  currentCoord(0.0),
		  matchMode(Animator::MATCH_INDEX),
		  nextMode(Animator::NEXT_FORWARD),
		  updateMode(Animator::UPDATE_DIRECT),
		  updateInterval(0.1),
		  numberInList(0) {
		itsAnimatorRefEH = new AnimatorRefEH(this);
	}

// Destructor
	Animator::~Animator()
	{}   // nothing to do


// Display the next Z coordinate
	void Animator::nextCoord() {
		// increment the coordinate
		computeNextCoord(1);
		// write the Attributes
		writeRestrictions();
		// and refresh
		refresh();
	}

// Display the previous Z coordinate
	void Animator::prevCoord() {
		// decrement the coordinate
		computeNextCoord(-1);
		// write the Attributes
		writeRestrictions();
		// and refresh
		refresh();
	}

// Display Z coordinate newCoord
	void Animator::gotoCoord(Double newCoord) {
		currentCoord = newCoord;
		// should be within limits
		currentCoord = min( max( currentCoord, minCoord ), maxCoord);

		// compute (should always do this???)
		numberInList = Int( (currentCoord-minCoord)/movieStep + 0.5);
		// check limits
		numberInList = min( max( numberInList, 0 ), listLen() );

		// write the Attributes
		writeRestrictions();
		// and refresh
		refresh();

	}



	void Animator::setStep(uInt zIncrement) {
		Double tmpD = zIncrement;
		setStep(tmpD);
	}

// Set the increment (ie. movieStep)
	void Animator::setStep(Double zIncrement) {
		// ensure that movieStep is positive
		if (zIncrement <= 0.0) {
			throw(AipsError("Animator::setStep - non-positive zIncrement"));
		}
		movieStep = zIncrement;
	}

	void Animator::setTolerance(uInt tolerance) {
		Double tmpD = tolerance;
		setTolerance(tmpD);
	}


// Set the tolerance for the coordinate Attribute
	void Animator::setTolerance(Double tolerance) {
		coordTolerance = abs(tolerance);
	}

// Set the minimum and maximum value foor the Z coordinate This routine needs
// both min and max.  Separate setMinCoord() and setMaxCoord() interfaces have
// the problem that it is not possible to keep the min and max ok, since if
// the user wants to give both a new min and a new max, if there are separate
// routines, the new min can only be compared with the old max!!.
	void Animator::setMinAndMaxCoord(Double zMin, Double zMax) {
		if (zMin > zMax) {
			minCoord = zMax;
			maxCoord = zMin;
		} else {
			minCoord = zMin;
			maxCoord = zMax;
		}
	}


// Set the MatchMode
	void Animator::setMatchMode(Animator::MatchMode newMode) {
		matchMode = newMode;
	}

// Set the NextMode
	void Animator::setNextMode(Animator::NextMode newMode) {
		nextMode = newMode;
	}

// Set the UpdateMode
	void Animator::setUpdateMode(Animator::UpdateMode newMode) {
		updateMode = newMode;
	}

// Set the time interval in seconds after which the timer should go off and a
// nextCoord() is invoked
	void Animator::setUpdateInterval(Double interval) {
		updateInterval = max(interval, 0.05);
	}

// Start the timer
	void Animator::startMovie() {
		// Oops!! AIPS++ does not have a timer!!
	}

// Stop the timer
	void Animator::stopMovie() {
		// Oops!! AIPS++ does not have a timer!!
	}


// Return the length of the movie
	uInt Animator::getMovieLength() {
		switch (updateMode) {
			// in blink mode the kength is determined by the length of the List of
			// AttributeBuffers
		case UPDATE_BLINK :  {
			return uInt(listLen());
			break;
		}
		// otherwise it is determined by the min, max and step
		default :
		case UPDATE_DIRECT :  {
			return uInt( (maxCoord-minCoord)/movieStep ) + 1;
			break;
		}
		}
	}

	Int Animator::getCurrentPosition() {
		// fill the extra Attribute
		if (matchMode == Animator::MATCH_INDEX) {
			return Int(currentCoord+0.5);
		} else {
			throw(AipsError("getCurrentPosition not supported"));
		}
	}

// Add a new WorldCanvasHolder to the list of WorldCanvasHolder that are
// controlled by this Animator
	void  Animator::addWorldCanvasHolder(WorldCanvasHolder *wCnvsHldr) {
		if (wCnvsHldr == 0) {
			throw(AipsError("Animator::addWorldCanvasHolder - "
			                "null pointer passed"));
		}

		// create iterator
		ListIter<void *> it(&holderList);

		// add to List
		it.addRight((void *) wCnvsHldr);

		wCnvsHldr->worldCanvas()->addRefreshEventHandler(*itsAnimatorRefEH);

	}


// remove a WorldCanvasHolder from the buffer
	void Animator::removeWorldCanvasHolder(WorldCanvasHolder& wCnvsHldr) {
		// create iterator
		ListIter<void *> it(&holderList);

		// while something in the list
		while(!it.atEnd()) {
			// check if this is the one
			if (it.getRight() == (void *) &wCnvsHldr) {
				// if so, delete
				it.removeRight();
				wCnvsHldr.worldCanvas()->removeRefreshEventHandler(*itsAnimatorRefEH);
				// addWorldCanvasHolder does not prevent to store the same WorldCanvas
				// twice, so we should continue iterating, so we cannot do here: break;
			} else {
				it++;
			}
		}


	}

// handle a refresh event - ie. see if the resetCoordinates att
// is set on the WorldCanvas - and if so, reset the animator
	void Animator::operator()(const WCRefreshEvent &ev) {
		WorldCanvas *wcanvas = ev.worldCanvas();
		String resetString("resetAnimator");
		if (wcanvas->existsAttribute(resetString)) {
			Bool resetValue = False;
			wcanvas->getAttributeValue(resetString, resetValue);
			wcanvas->removeAttribute(resetString);
			if (resetValue) {
				reset();
			}
		}
	}

// refresh
	void Animator::refresh() {
		// create iterator
		ListIter<void *> it(&holderList);

		// and pointer to a WorldCanvasHolder
		WorldCanvasHolder *wCnvsHldr;

		// while something in the list
		while(!it.atEnd()) {
			// check if this is the one
			wCnvsHldr = (WorldCanvasHolder *) it.getRight();

			// and refresh the WorldCanvas of this WorldCanvasHolder
			(wCnvsHldr->worldCanvas())->refresh();

			// go to next WorldCanvasHolder
			it++;
		}
	}


// Register the List of AttributeBuffers
	void Animator::setBlinkRestrictions(List<void *> *newAttBufList) {
		if (newAttBufList == 0) {
			throw(AipsError("nimator::setBlinkRestrictions - "
			                "null pointer passed"));
		}

		attBufList = newAttBufList;

	}


// Unregister the List of AttributeBuffers
	void Animator::clearBlinkRestrictions() {
		attBufList = 0;
	}


// private functions

// Compute the incremeted Z coordinate
	void Animator::computeNextCoord(Int addOrSubtract) {

		// new Z coordinate depend on updateMode
		switch (updateMode) {
			// if blinking, the movie is determined by the AttrubuteBuffers. Z
			// coordinate should not incremented, but the index in the List of
			// AttributeBuffers should increment
		case Animator::UPDATE_BLINK : {
			// how to calculate the next one
			switch (nextMode) {
				//  if going forward: increment index
			default :
			case Animator::NEXT_FORWARD : {
				increment(numberInList, addOrSubtract);
				// and check for bounds
				if ( numberInList >= listLen() ) {
					numberInList = 0;
				}
				if ( numberInList < 0 ) {
					numberInList = listLen()-1;
				}
				break;
			}
			// if going backwards: decrement
			case Animator::NEXT_BACKWARD : {
				decrement(numberInList, addOrSubtract);
				// and check bounds
				if ( numberInList < 0 ) {
					numberInList = listLen()-1;
				}
				if ( numberInList >= listLen() ) {
					numberInList = 0;
				}
				break;
			}
			// rock & roll:
			case Animator::NEXT_ROCKANDROLL : {
				if (movieDirection < 0) {
					// if going down: decrement
					decrement(numberInList, addOrSubtract);
				} else {
					// if going up: increment
					increment(numberInList, addOrSubtract);
				}
				// and check bounds
				if ( numberInList < 0) {
					movieDirection = 1;
					numberInList = 1;
				}
				if ( numberInList >= listLen() ) {
					movieDirection = -1;
					numberInList = listLen() - 2;
				}
				break;
			}
			} // end of switch
			break;
		}
		// for the Z coordinate the same story
		default :
		case Animator::UPDATE_DIRECT : {
			switch (nextMode) {
			default :
			case Animator::NEXT_FORWARD : {
				increment(currentCoord, addOrSubtract);
				if (currentCoord > maxCoord) {
					currentCoord = minCoord;
				}
				if (currentCoord < minCoord) {
					currentCoord = maxCoord;
				}
				break;
			}
			case Animator::NEXT_BACKWARD : {
				decrement(currentCoord, addOrSubtract);
				if (currentCoord > maxCoord) {
					currentCoord = minCoord;
				}
				if (currentCoord < minCoord) {
					currentCoord = maxCoord;
				}
				break;
			}
			case Animator::NEXT_ROCKANDROLL : {
				if (movieDirection < 0) {
					decrement(currentCoord, addOrSubtract);
				} else {
					increment(currentCoord, addOrSubtract);
				}
				if (currentCoord < minCoord) {
					movieDirection = +1;
					currentCoord = minCoord + movieStep;
				}
				if (currentCoord > maxCoord) {
					movieDirection = -1;
					currentCoord = maxCoord - movieStep;
				}
				break;
			}
			} // end switch nextMode
			// now numberInList is calculated
			numberInList = Int( (currentCoord-minCoord)/movieStep + 0.5);
			break;
		}
		} // end switch updateMode

		// numberInList should be inside valid range
		numberInList = max( min( numberInList, listLen()-1), 0);

	}

// reset the animator
	void Animator::reset() {
		uInt nelements = 0;

		// create iterator
		ListIter<void *> it(&holderList);

		// and pointer to a WorldCanvasHolder
		WorldCanvasHolder *wcHolder;

		// commented out dgb 1998/12/17: don't want to renegotiate spatial
		// coords on screen, just movie plane...
		//Attribute coordReset("resetCoordinates", True);

		// while something in the list
		while(!it.atEnd()) {
			// check if this is the one
			wcHolder = (WorldCanvasHolder *) it.getRight();

			// get the nelements  from this WorldCanvasHolder
			nelements = max(nelements, wcHolder->nelements());

			// tell WorldCanvas that coordinates need updating
			//wcHolder->worldCanvas()->setAttribute(coordReset);

			// go to next WorldCanvasHolder
			it++;
		}
		setMinAndMaxCoord(0.0, max(0.0, Double(nelements)-1.0));
		setStep(1.0);
		setTolerance(0.1);
		setMatchMode(Animator::MATCH_INDEX);
		setNextMode(Animator::NEXT_FORWARD);
		setUpdateMode(Animator::UPDATE_DIRECT);
		Bool needToReRefresh = False;
		if ((currentCoord < minCoord) || (currentCoord > maxCoord)) {
			currentCoord = 0.0;
			needToReRefresh = True;
		}
		clearBlinkRestrictions();

		// update the WorldCanvasHolders
		writeRestrictions();

		if (needToReRefresh) {
			refresh();
		}
	}


// Helper routine for computeNewCoord()
// If addOrSubtract > 0: add 1 to number
// If addOrSubtract < 0: subtract 1 from number
	void Animator::increment(Int& number, Int addOrSubtract) {
		if (addOrSubtract > 0) {
			number++;
		}
		if (addOrSubtract < 0) {
			number--;
		}
	}

// inverse of above
	void Animator::decrement(Int& number, Int addOrSubtract) {
		if (addOrSubtract > 0) {
			number--;
		}
		if (addOrSubtract < 0) {
			number++;
		}
	}

// as above, but for Doubles, step is movieStep
	void Animator::increment(Double& number, Int addOrSubtract) {
		if (addOrSubtract > 0) {
			number += movieStep;
		}
		if (addOrSubtract < 0) {
			number -= movieStep;
		}
	}

// Inverse of above
	void Animator::decrement(Double& number, Int addOrSubtract) {
		if (addOrSubtract > 0) {
			number -= movieStep;
		}
		if (addOrSubtract < 0) {
			number += movieStep;
		}
	}

// write the correct Attributes on the WorldCanvasHolders
	void Animator::writeRestrictions() {

		// create iterator for the List of WorldCanvasHolders
		ListIter<void *> holderIt(&holderList);

		// and pointer to a WorldCanvasHolder
		WorldCanvasHolder *wCnvsHldr;

		// pointer for the extra Attribute that could be written
		Attribute *extraAtt;
		extraAtt = 0;

		// fill the extra Attribute
		if (matchMode == Animator::MATCH_INDEX) {
			Int cCoord = Int(currentCoord+0.5);
			Int cTol = Int(coordTolerance+0.5);

			extraAtt = new Attribute("zIndex", cCoord, cTol);
		}
		if (matchMode == Animator::MATCH_WORLD) {
			extraAtt = new Attribute("zValue", currentCoord, coordTolerance);
		}

		// create iterator for the List of AttributeBuffers
		ListIter<void *> *atIt;
		atIt = 0;

		// and a pointer to an AttributeBuffer
		AttributeBuffer *attBuf = 0;

		if (attBufList != 0) {
			atIt = new ListIter<void *>(attBufList);

			// go to correct position in List and extract the AttributeBuffer
			numberInList = min(numberInList, Int(atIt->len())-1);
			atIt->pos(numberInList);
			attBuf = (AttributeBuffer *) atIt->getRight();
		}


		// loop the WorldCanvasHolder list
		while(!holderIt.atEnd()) {
			// get the WorldCanvas
			wCnvsHldr = (WorldCanvasHolder *) holderIt.getRight();

			// if needed, write extra Attribute and remove 'the wrong one'
			if (matchMode == Animator::MATCH_INDEX) {
				wCnvsHldr->setRestriction(*extraAtt);
				wCnvsHldr->removeRestriction("zValue");
			}

			if (matchMode == Animator::MATCH_WORLD) {
				wCnvsHldr->setRestriction(*extraAtt);
				wCnvsHldr->removeRestriction("zIndex");
			}

			if (attBufList != 0) {
				// and the AttributeBuffer, is always written (if it exists);
				wCnvsHldr->setRestrictions(*attBuf);
			}

			// iterate
			holderIt++;
		}

		//cleanup
		if (extraAtt != 0) {
			delete extraAtt;
		}
		if (atIt != 0) {
			delete atIt;
		}



	}

// Return the length of the List of AttributeBuffers. Since the pointer to
// this List is stroed, the length of the List can change after registering
// it. So listLen() should be called everytime the length of this List is needed
	Int Animator::listLen() {
		if (attBufList == 0) {
			// if the is no buffer, return 0
			return 0;
		} else {
			// else, find length and return
			ListIter<void *> it(attBufList);
			return Int(it.len());
		}
	}


} //# NAMESPACE CASA - END

