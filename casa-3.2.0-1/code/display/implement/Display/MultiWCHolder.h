//# MultiWCHolder.h: Holder of multiple WorldCanvasHolders for panelling
//# Copyright (C) 2000,2001,2003
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

#ifndef TRIALDISPLAY_MULTIWCHOLDER_H
#define TRIALDISPLAY_MULTIWCHOLDER_H

#include <casa/aips.h>
#include <casa/Containers/List.h>
#include <display/Display/DisplayEnums.h>
#include <display/Display/AttributeBuffer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class WorldCanvasHolder;
class DisplayData;
class AttributeBuffer;
class String;

// <summary>
// A holder of WorldCanvasHolders to use for panelling applications.
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <prerequisite>
//   <li><linkto class="WorldCanvasHolder">WorldCanvasHolder</linkto>
//   <li><linkto class="DisplayData">DisplayData</linkto>
//   <li><linkto class="Attribute">Attribute</linkto>
//   <li><linkto class="AttributeBuffer">AttributeBuffer</linkto>
// </prerequisite>
//
// <etymology>
// The MultiWCHolder holds a number of WorldCanvasHolders to simplify
// panelling operations.
// </etymology>
//
// <synopsis>
// This class is a holder of <linkto
// class=WorldCanvasHolder>WorldCanvasHolder</linkto> objects.  It
// exists to facilitate the management of multiple WorldCanvasHolders
// having in common a set of restrictions, which for example may be
// for controlling animation, and on which a common set of
// DisplayDatas may be registered and displayed.
//
// The basic interface to the MultiWCHolder provides methods for
// adding and removing individual WorldCanvasHolder objects, for
// globally registering and unregistering DisplayData objects for
// display on the WorldCanvasHolder objects, and for applying a set
// of restrictions to the WorldCanvasHolder objects.
//
// At a more advanced level, functions exist to distribute varying
// restrictions across all the WorldCanvasHolder objects.  For
// example, the programmer can use
// <src>linearDistributeRestriction</src> to apply a restriction to
// each WorldCanvasHolder, where the value of the restriction is
// incremented by a fixed amount between each WorldCanvasHolder.  This
// would be useful, for example, to easily display a set of planes
// from a multi-frequency image on a number of WorldCanvases whose
// WorldCanvasHolders have been added to a MultiWCHolder.
// </synopsis>
//
// <example> 
// </example>
//
// <motivation> 
// </motivation>

class MultiWCHolder {

 public:

  // Constructor which makes an empty MultiWCHolder.
  MultiWCHolder();

  // Constructor which makes a MultiWCHolder containing the single
  // specified WorldCanvasHolder.
  MultiWCHolder(WorldCanvasHolder& holder);

  // Destructor.
  virtual ~MultiWCHolder();

  // Add or remove a WorldCanvasHolder, or remove all
  // WorldCanvasHolders on this MultiWCHolder.  <src>holder</src> is
  // added at the end of the list.  When a new WorldCanvasHolder is 
  // added, it will automatically have all registered DisplayDatas
  // added to it.
  // <group>
  virtual void addWCHolder(WorldCanvasHolder& holder);
  virtual void removeWCHolder(WorldCanvasHolder& holder);
  virtual void removeWCHolders();
  // </group>

  // Add or remove a DisplayData, or remove all DisplayDatas on this
  // MultiWCHolder.  
  // <group>
  virtual void addDisplayData(DisplayData& displaydata);
  virtual void removeDisplayData(DisplayData& displaydata);
  virtual void removeDisplayDatas();
  // </group>

  // Install or remove a single restriction, or a buffer of
  // restrictions, on all of the WorldCanvasHolders managed by this
  // MultiWCHolder.  
  // <group>
  virtual void setRestriction(const Attribute& restriction);
  virtual void setRestrictions(const AttributeBuffer& restrictions);
  virtual void removeRestriction(const String& name);
  virtual void removeRestrictions();
  // </group>

  // Incrementally distribute restrictions to all of the
  // WorldCanvasHolders.  The value of the restrictions are
  // incremented by the given amount for each holder beyond the first
  // one.  The given restrictions may have tolerances, and the base
  // types of each restriction and increment must be identical.  That
  // is, <src>restriction.getType() == increment.getType()</src>.
  // Other methods to distribute restrictions in other ways are easily
  // imagined, eg. a vector of Attributes is given.  After being 
  // called, <src>restrictions</src> are equal to the values set on
  // the final WorldCanvasHolder in the list.

  //# DK note: only this routine and removeRestriction() should be used
  //# with the 'bIndex' restriction; none of the other restriction routines
  //# have yet been modified to add an ID to the 'bIndex' restriction name.
  //# (The routines are normally called simply using the unmodified name
  //# 'bIndex').
  virtual void setLinearRestrictions(AttributeBuffer &restrictions,
				     const AttributeBuffer &increments);
  
  // Hold and release response to refreshes requested with the
  // <src>refresh()</src> member function.  Multiple calls to
  // <src>hold()</src> can be made, and refreshes will not resume
  // until the same number of calls have been made to
  // <src>release()</src>.  Note that these functions do not affect
  // whether internally (implicitly) generated refreshes continue to
  // occur.  That is, refresh events due to PixelCanvas resize events,
  // or Colormap changes, for example, will continue to be acted upon.
  // <group>
  virtual void hold();
  virtual void release();
  // </group>

  // Distribute a refresh event to all the held WorldCanvasHolders.
  virtual void refresh(const Display::RefreshReason &reason =
		       Display::UserCommand);

  virtual uInt zLength();

  // These orrespond to 'zIndex, zLength' for animator's 'blink mode'.
  virtual Int bLength();
  virtual Int bIndex();

  // Determines which DDs will be restricted, which are always active.
  virtual Bool isBlinkDD(DisplayData *dd);

  // Test conformance of a DD to a WCH of this MWCH (by default, test
  // against the first one (WCH 0), which always exists).
  // The three aspects of conformance can be selectively tested.  
  virtual Bool conforms(DisplayData* dd,
	  Bool testRstrs=True, Bool testCS=True, Bool testZ=True,
	  Int wchIndex=0);

  // Return number of WCHs (subpanels).
  virtual Int nWCHs() { return Int(itsWCHList.len());  } 
  
  // Clear PC in MWCH's area (not implemented on this level).
  virtual void clear() {  } 
 
 protected:

  // Copy constructor - construct a new MultiWCHolder from
  // <src>other</src>.
  // MultiWCHolder(const MultiWCHolder &other);

  // Copy assignment using copy semantics.
  // MultiWCHolder &operator==(const MultiWCHolder &other);

  // (permanently) sets itsBIndexName (below).  Called only in constructor.
  virtual void setBIndexName();

  // Adjust name of "bIndex" Attribute (if any) to indlude ID of this MWCH.
  virtual void adjustBIndexName(AttributeBuffer& rstrs);


 private:
  
  // The list of WorldCanvasHolders that are managed by this
  // MultiWCHolder.
  List<WorldCanvasHolder *> itsWCHList;

  // The list of DisplayDatas that are managed by this MultiWCHolder.
  List<DisplayData *> itsDDList;

  // Subset of above DDs which will have blinking restrictions added
  // (Countour plots, e.g., do not; they always display).  This should
  // actually be a Block<DisplayData*>.
  Block<void*> itsBlinkDDs;

  // Number of DDs above.
  Int itsBLength;

  // Index of DD to show in first WCH when blink mode is active (it is
  // kept up-to-date in any case).
  Int itsBIndex;

  // The (unchanging) name of the blink restriction: "bIndex" followed by
  // an ID unique to this MWCH.  The ID is because DDs can be registered on
  // more than one MWCH and need a different "bIndex" restriction on each one.
  // Note, however, that when setLinearRestrictions() is used externally to
  // set the blink restriction onto the WCHs, it simply uses the name
  // "bIndex".
  String itsBIndexName;

  // The buffer of attributes which are presently installed on the
  // registered WorldCanvasHolders.
  AttributeBuffer itsAttributes;

  // The current hold count of this MultiWCHolder.  Whenever new
  // WCHolders are added, their count is increased to this value.
  // Calls to hold() and release() modify this count as well as
  // those per canvas.
  Int itsHoldCount;

  // The current state of whether a refresh was held up or not.
  Bool itsRefreshHeld;
  
  // The reason for the held-up refresh event
  Display::RefreshReason itsHeldReason;

  // Do we already have this WorldCanvasHolder/DisplayData registered?
  // <group>
  const Bool isAlreadyRegistered(const WorldCanvasHolder &holder);
  const Bool isAlreadyRegistered(const DisplayData &displaydata);
  // </group>

  // Add/remove all the DisplayDatas to/from a WorldCanvasHolder.
  // <group>
  void addAllDisplayDatas(WorldCanvasHolder &holder);
  void removeAllDisplayDatas(WorldCanvasHolder &holder,
			     const Bool& permanent = False);
  // </group>

  // Add/remove a DisplayData to/from all WorldCanvasHolders.
  // <group>
  void addToAllWorldCanvasHolders(DisplayData &displaydata);
  void removeFromAllWorldCanvasHolders(DisplayData &displaydata);
  // </group>

  // Distribute restrictions to all WorldCanvasHolders.
  void distributeRestrictions();
  
  // Set restrictions on a particular WorldCanvasHolder.
  void installRestrictions(WorldCanvasHolder &holder);

};


} //# NAMESPACE CASA - END

#endif
