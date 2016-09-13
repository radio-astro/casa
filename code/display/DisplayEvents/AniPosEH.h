//# AniPosEH.h: Animator position event handler for a WorldCanvas
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

#ifndef TRIALDISPLAY_ANIPOSEH_H
#define TRIALDISPLAY_ANIPOSEH_H

#include <casa/aips.h>
#include <display/DisplayEvents/WCPositionEH.h>
#include <display/DisplayEvents/Animator.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	class WorldCanvasHolder;
	class Attribute;

// <summary>
// WorldCanvas position event handler for Animator.
// </summary>
//
// <synopsis>
// </synopsis>

	class AniPosEH : public WCPositionEH {

	public:

		// Constructor
		AniPosEH();

		// Destrutor
		~AniPosEH();

		// The operator that is given the position events and must process
		// them
		virtual void operator()(const WCPositionEvent& ev);

		// add/remove a WorldCanvasHolder to the handler
		// <group>
		virtual void addWorldCanvasHolder(WorldCanvasHolder *wcHolder);
		virtual void removeWorldCanvasHolder(WorldCanvasHolder& wcHolder);
		// </group>

		// reset the animator - this is typically called by the WorldCanvasHolder
		// when DisplayDatas are registered or unregistered
		virtual void resetAnimator() {
			animator.reset();
		}

	private:

		// the Animator we control
		Animator animator;

		// List of WorldCanvasHolders
		List<void *> holderList;

		// set a restriction on the WorldCanvasHolders
		void setRestriction(Attribute& att);

		// refresh the WorldCanvasHolders
		void refresh();

		// cleanup the DisplayDatas on all WorldCanvasHolders
		void cleanup();
	};


} //# NAMESPACE CASA - END

#endif
