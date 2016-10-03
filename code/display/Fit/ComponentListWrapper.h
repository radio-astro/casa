//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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

#ifndef COMPONENTLISTWRAPPER_H_
#define COMPONENTLISTWRAPPER_H_

#include <casa/BasicSL/String.h>
#include <components/ComponentModels/ComponentList.h>
#include <QTextStream>
namespace casacore{

	template <class T> class ImageInterface;
}

namespace casa {

	class RegionShape;
	class RegionBox;

	/**
	 * Provides convenient accessors and functionality for a ComponentList.
	 */

	class ComponentListWrapper {
	public:
		ComponentListWrapper();
		int getSize() const;
		string getRA( int i ) const;
		string getDEC( int i ) const;
		string getType( int i ) const;
		const ComponentShape* getShape( int i ) const;
		casacore::Quantity getMajorAxis( int i ) const;
		casacore::Quantity getMinorAxis( int i ) const;
		casacore::Quantity getAngle( int i ) const;
		casacore::Quantum< casacore::Vector<double> > getLatLong( int i ) const;
		casacore::Quantity getFlux( int i ) const;
		//QString getEstimateFixed( int index ) const;
		void clear();
		void remove( QVector<int> indices );
		void fromComponentList( ComponentList list );
		bool fromRecord( casacore::String& errorMsg, casacore::Record& record );
		bool toEstimateFile( QTextStream& stream,
		                     const casacore::ImageInterface<casacore::Float>*const image, QString& errorMsg,
		                     bool screenEstimates = false, RegionBox* screenBox = NULL) const;
		QList<RegionShape*> toDrawingDisplay(const casacore::ImageInterface<casacore::Float>* image, const QString& colorName) const;
		bool toRegionFile(const casacore::ImageInterface<float>* image, int channelIndex, const QString& filePath ) const;
		virtual ~ComponentListWrapper();

	private:
		void toRecord( casacore::Record& record, const casacore::Quantity& quantity ) const;
		double getRAValue( int i, const casacore::String& unit ) const;
		double getDECValue( int i, const casacore::String& unit ) const;
		double radiansToDegrees( double value ) const;
		double degreesToArcSecs( double Value ) const;
		casacore::Quantity getAxis( int listIndex, int shapeIndex, bool toArcSecs ) const;
		double rotateAngle( double value ) const;
		void deconvolve(const casacore::ImageInterface<float>* image, int channel,
		                casacore::Quantity& majorAxis, casacore::Quantity& minorAxis, casacore::Quantity& positionAngle) const;

		ComponentList skyList;
		const casacore::String RAD;
		const casacore::String DEG;
		const casacore::String ARC_SEC;
	};

} // end namespace casa

#endif /* COMPONENTLISTWRAPPER_H_ */
