//# QtWCBox.h: Class to define a box shaped WC region
//# Copyright (C) 1998,1999,2001
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
//# $Id: QtWCBox.h 20567 2009-04-09 23:12:39Z gervandiepen $



#ifndef QT_WCBOX_H
#define QT_WCBOX_H

//# Includes
#include <casa/aips.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <images/Regions/WCRegion.h>
#include <lattices/LRegions/RegionType.h>
#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>

namespace casacore{

	class LCRegion;
	class TableRecord;
	class IPosition;
}

namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations


	class QtWCBox : public casacore::WCRegion {
	public:
		QtWCBox();

		// Construct from vectors of world coordinates
		// defining the box corners.  It is assumed that the
		// order of the values is in the order of the pixel axes
		// in the given coordinate system.
		// <group>
		QtWCBox(const casacore::Vector<casacore::Quantum<casacore::Double> >& blc,
		        const casacore::Vector<casacore::Quantum<casacore::Double> >& trc,
		        const DisplayCoordinateSystem& cSys,
		        const casacore::Vector<casacore::Int>& absRel);
		// </group>

		// Construct from vectors of world coordinates
		// defining the box corners.   You specify the pixel
		// axis order of the world values.
		// <group>
		QtWCBox(const casacore::Vector<casacore::Quantum<casacore::Double> >& blc,
		        const casacore::Vector<casacore::Quantum<casacore::Double> >& trc,
		        const casacore::IPosition& pixelAxes,
		        const DisplayCoordinateSystem& cSys,
		        const casacore::Vector<casacore::Int>& absRel);
		// </group>

		// Construct from the bounding box of an  <src>casacore::LCRegion</src>.
		QtWCBox(const casacore::LCRegion& region,
		        const DisplayCoordinateSystem& cSys);

		// Copy constructor (reference semantics [except for <src>DisplayCoordinateSystem</src>])
		QtWCBox (const QtWCBox& other);

		// Destructor
		virtual ~QtWCBox();

		// Assignment (copy semantics)
		QtWCBox& operator= (const QtWCBox& other);

		// Comparison
		virtual casacore::Bool operator==(const casacore::WCRegion& other) const;

		// Clone a QtWCBox object.
		virtual casacore::WCRegion* cloneRegion() const;

		// QtWCBox can extend a region.
		virtual casacore::Bool canExtend() const;

		// Make a new box from the given axesin this box.
		QtWCBox splitBox (const casacore::IPosition& axes) const;

		// Convert to an casacore::LCRegion using the supplied <src>DisplayCoordinateSystem</src>
		// and shape.
		virtual casacore::LCRegion* doToLCRegion (const casacore::CoordinateSystem& cSys,
		                                const casacore::IPosition& latticeShape,
		                                const casacore::IPosition& pixelAxesMap,
		                                const casacore::IPosition& outOrder) const;

		// Convert the QtWCBox object to a record.
		// The record can be used to make the object persistent.
		// The <src>tableName</src> argument can be used by derived
		// classes (e.g. casacore::LCPagedMask) to put very large objects.
		virtual casacore::TableRecord toRecord(const casacore::String& tableName) const;

		// Convert to a QtWCBox from a record.
		static QtWCBox* fromRecord (const casacore::TableRecord& rec,
		                            const casacore::String& tableName);

		// Returns QtWCBox
		static casacore::String className();

		// Return region type.  Returns the class name
		virtual casacore::String type() const;

		// Convert from/to boxit format string
		casacore::String toBoxString() const;
		static QtWCBox* fromBoxString(const casacore::String&,
		                              const DisplayCoordinateSystem& cSys, casacore::String& err);

		// FIXME: refactor to someplace more appropriate
		// get/set chan extension
		void setChanExt(const casacore::Double, const casacore::Double);
		casacore::Bool getChanExt(casacore::Double&, casacore::Double&);

		// FIXME: refactor to someplace more appropriate
		// get/set pol extension
		void setPolExt(const casacore::Double, const casacore::Double);
		casacore::Bool getPolExt(casacore::Double&, casacore::Double&);

		static void unitInit();

	private:
		casacore::Vector<casacore::Quantum<casacore::Double> > itsBlc;
		casacore::Vector<casacore::Quantum<casacore::Double> > itsTrc;
		casacore::IPosition itsPixelAxes;
		DisplayCoordinateSystem itsCSys;
		casacore::Vector<casacore::Int> itsAbsRel;
		casacore::Bool itsNull;


// Check units of quanta are consistent with DisplayCoordinateSystem
		void checkUnits (const casacore::IPosition& pixelAxes,
		                 const casacore::Vector<casacore::Quantum<casacore::Double> >& values,
		                 const DisplayCoordinateSystem& cSys);

// Convert relative pixels to absolute or fill in defaults
		void convertPixel(casacore::Double& pixel,
		                  const casacore::Quantum<casacore::Double>& value,
		                  const casacore::Int absRel,
		                  const casacore::Double refPix,
		                  const casacore::Int shape,
		                  const casacore::Bool isBlc) const;

	};



} //# NAMESPACE CASA - END

#endif
