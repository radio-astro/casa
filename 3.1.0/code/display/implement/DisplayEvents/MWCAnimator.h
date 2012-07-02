//# MWCAnimator.h: Animator for MultiWCHolder class
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

#ifndef TRIALDISPLAY_MWCANIMATOR_H
#define TRIALDISPLAY_MWCANIMATOR_H

#include <casa/aips.h>
#include <casa/Containers/List.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class MultiWCHolder;
class AttributeBuffer;
class RecordInterface;

// <summary>
// Animator for MultiWCHolder class.
// </summary>
// <use visibility=export>
// 
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <etymology>
// </etymology>
//
// <synopsis> 
// </synopsis>
//
// <motivation>
// </motivation>
//
// <todo>
// </todo>

class MWCAnimator {

 public:

  // Constructor which makes an isolated MWCAnimator.
  MWCAnimator();
  
  // Constructor which makes a MWCAnimator which is connected to
  // the specified MultiWCHolder.
  MWCAnimator(MultiWCHolder &mholder);

  // Destructor.
  virtual ~MWCAnimator();
  
  // Add or remove a MultiWCHolder, or remove all MultiWCHolders
  // from this MWCAnimator.  <src>mholder</src> is
  // added at the end of the list.
  // <group>
  virtual void addMWCHolder(MultiWCHolder &mholder);
  virtual void removeMWCHolder(MultiWCHolder &mholder);
  virtual void removeMWCHolders();
  // </group>

  // Install the given restriction, spaced by the given increment, on
  // all MultiWCHolders.  Ultimately this will go private and be
  // called (for example) by next(), prev().  But for now we leave it
  // public for testing purposes.  After being called,
  // <src>restrictions</src> is the set of restrictions which have
  // been placed on the final WorldCanvasHolder of the final
  // MultiWCHolder in the list.
  virtual void setLinearRestrictions(AttributeBuffer &restrictions,
				     const AttributeBuffer &increments);

  // Install a restriction of the given name, and initial value and
  // increment, on all MultiWCHolders.  This is templated and the
  // templates are valid for any type which can be added to an
  // AttributeBuffer.  This function simply sets up a single-element
  // AttributeBuffer and calls <src>setLinearRestrictions</src>.
  template <class T> void setLinearRestriction(const String &name,
					       const T &value,
					       const T &increment,
					       const T& tol);

  // Install a restriction described by the given RecordInterface,
  // on all MultiWCHolders.  This method simply extracts the fields
  // <src>name</src>, <src>value</src>, and <src>increment</src>
  // from <src>rec</src>, and calls the explicit version of 
  // <src>setLinearRestriction</src>.  The <src>value</src> and 
  // <src>increment</src> fields can contain either Int, Float
  // or Double data at the moment.
  virtual void setLinearRestriction(const RecordInterface &rec);

  // Remove a restriction (including a 'linear' (ramped) one).
  // (Can be (and is) used to remove 'bindex' restriction).
  virtual void removeRestriction(const String& name);

 private:
  
  // The list of MultiWCHolders that are connected to this MWCAnimator.
  List<MultiWCHolder *> itsMWCHList;
  
  // Do we already have this MultiWCHolder registered?
  const Bool isAlreadyRegistered(const MultiWCHolder &holder);
  
};

} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/DisplayEvents/MWCAniTemplates.tcc>
#endif

#endif
