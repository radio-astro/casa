//# PassiveCachingDD.h: passive implementation of a CachingDisplayData
//# Copyright (C) 1999,2000,2001,2002,2003
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

#ifndef TRIALDISPLAY_PASSIVECACHINGDD_H
#define TRIALDISPLAY_PASSIVECACHINGDD_H

#include <casa/aips.h>
#include <casa/Arrays/IPosition.h>
#include <display/DisplayDatas/CachingDisplayData.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Class providing passive behaviour for CachingDisplayDatas.
// </summary>

// <synopsis>
// A "passive" DisplayData is one which can only be used in
// conjunction with another non-passive DisplayData.  As such, it will
// not negotiate coordinates on a WorldCanvas, nor will it provide
// coordinate conversion capabilities.
// </synopsis>

	class PassiveCachingDD : public CachingDisplayData {

	public:

		// Constructor.
		PassiveCachingDD();

		// Destructor.
		virtual ~PassiveCachingDD();

		// Coordinate transformation handlers, called by WorldCanvasHolder.
		// These functions simply return False because this DisplayData is
		// passive.
		// <group>
		virtual Bool linToWorld(Vector<Double> &world, const Vector<Double> &lin);
		virtual Bool worldToLin(Vector<Double> &lin, const Vector<Double> &world);
		// </group>

		// Format a string containing coordinate or value information at the
		// given world coordinate.  They simply return empty Strings because
		// this DisplayData is passive.
		// <group>
		virtual String showPosition(const Vector<Double> &world,
		                            const Bool &displayAxesOnly = False);
		virtual String showValue(const Vector<Double> &world);
		// </group>


		// World axis information suppliers.
		// <group>
		virtual Vector<String> worldAxisNames() const;
		virtual Vector<String> worldAxisUnits() const;
		// </group>

		const Unit dataUnit() const {
			return Unit("_");
		}
		const IPosition dataShape() const {
			return IPosition( );
		}
		const uInt dataDim() const {
			return 0;
		}
		std::vector<int> displayAxes( ) const {
			return std::vector<int>( );
		}

		// Return the number of display elements (ie. drawable images) in
		// this DisplayData.
		// <group>
		virtual const uInt nelements(const WorldCanvasHolder &wcHolder) const;
		virtual const uInt nelements() const;
		// </group>

		// Install the default options for this DisplayData.
		virtual void setDefaultOptions();

		// Apply options stored in <src>rec</src> to the DisplayData.  A
		// return value of <src>True</src> means a refresh is needed.
		// <src>recOut</src> contains any fields which were implicitly
		// changed as a result of the call to this function.
		virtual Bool setOptions(Record &rec, Record &recOut);

		// Retrieve the current and default options and parameter types.
		virtual Record getOptions( bool scrub=false ) const;

		// Negotiatiate WorldCanvas linear coordinate system when asked to
		// do so by the WorldCanvasHolder.  In this implementation, simply
		// return False to indicate that this DisplayData will not negotiate
		// coordinates: it is a passive DisplayData.

		virtual void refreshEH(const WCRefreshEvent &ev);
		virtual Bool sizeControl(WorldCanvasHolder &/*wcHolder*/,
		                         AttributeBuffer &/*holderBuf*/) {
			return False;
		}

		// Tidy up the elements of this DisplayData.
		virtual void cleanup();

	protected:

		// Return the current options of this DisplayData as an
		// AttributeBuffer.  The caller must delete the returned buffer.
		virtual AttributeBuffer optionsAsAttributes();

		// (Required) copy constructor.
		PassiveCachingDD(const PassiveCachingDD &other);

		// (Required) copy assignment.
		void operator=(const PassiveCachingDD &other);

	};


} //# NAMESPACE CASA - END

#endif
