//# DisplayMethod.h: Base class for drawing data in the Display Library
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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

#ifndef TRIALDISPLAY_DISPLAYMETHOD_H
#define TRIALDISPLAY_DISPLAYMETHOD_H

//# aips includes:
#include <casa/aips.h>

//# display library includes:
#include <display/Display/DisplayEnums.h>
#include <display/Display/AttributeBuffer.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# forwards:
	class WorldCanvasHolder;
	class Attribute;
	class DisplayData;

// <summary>
// Base class for drawing a particular element (view) of a DisplayData.
// </summary>
//
// <synopsis>
// This base class defines an interface for drawing a single view of a
// DisplayData object.  The fundamental idea is that a DisplayData
// class will manage one or more DisplayMethod objects, and request
// the individual objects to draw as necessary.
// </synopsis>

	class DisplayMethod {
	public:

		// Constructor.  DisplayMethods need to know who their parent
		// DisplayData is.
		DisplayMethod(DisplayData *parentDisplayData);

		// Destructor.
		virtual ~DisplayMethod();

		// Draw on the provided WorldCanvasHolder.
		virtual void draw(Display::RefreshReason reason,
		                  WorldCanvasHolder &wcHolder) = 0;

		// clear drawlist state.
		// dk note 11/03:  Only used by the PrincipleAxesDM branch, where the
		// implementation has been moved.  Implementation should remain null
		// on the CachingDD/DM branch, which uses different interface (purgeCache)
		// and state for this.
		virtual void cleanup();

		// Set & remove restrictions
		// <group>
		void addRestriction(Attribute& newAt, Bool permanent);
		void addRestrictions(AttributeBuffer& newBuf);
		void setRestriction(Attribute& newAt);
		void setRestrictions(AttributeBuffer& newBuf);
		void removeRestriction(const String& name);
		Bool existRestriction(const String& name);
		void clearRestrictions();
		// </group>

		// match restriction
		// <group>
		Bool matches(Attribute& at);
		Bool matches(AttributeBuffer& atBuf);
		// </group>


		// Allow DisplayData objects to find out about restrictions in
		// DisplayMethod elements...
		friend class DisplayData;

	protected:

		AttributeBuffer  restrictions;

		// (Required) default constructor.
		DisplayMethod();

		// (Required) copy constructor.
		DisplayMethod(const DisplayMethod &other);

		// (Required) copy assignment.
		void operator=(const DisplayMethod &other);

		// Return the parent DisplayData.
		DisplayData *parentDisplayData() {
			return itsParentDisplayData;
		}

	private:

		DisplayData *itsParentDisplayData;

	};


} //# NAMESPACE CASA - END

#endif
