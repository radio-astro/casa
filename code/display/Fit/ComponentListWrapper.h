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
namespace casa {

template <class T> class ImageInterface;
class RegionShape;

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
	Quantity getMajorAxis( int i ) const;
	Quantity getMinorAxis( int i ) const;
	Quantity getAngle( int i ) const;
	Quantum< Vector<double> > getLatLong( int i ) const;
	Quantity getFlux( int i ) const;
	void clear();
	void remove( const QVector<int>& indices );
	void fromComponentList( ComponentList list );
	bool fromRecord( String& errorMsg, Record& record );
	bool toEstimateFile( QTextStream& stream,
			ImageInterface<Float>* image, QString& errorMsg ) const;
	QList<RegionShape*> toDrawingDisplay( ImageInterface<Float>* image, const QString& colorName ) const;
	virtual ~ComponentListWrapper();

private:
	void toRecord( Record& record, const Quantity& quantity ) const;
	double getRAValue( int i, const String& unit ) const;
	double getDECValue( int i, const String& unit ) const;
	double radiansToDegrees( double value ) const;
	double degreesToArcSecs( double Value ) const;
	Quantity getAxis( int listIndex, int shapeIndex, bool toArcSecs ) const;

	ComponentList skyList;
	const String RAD;
	const String DEG;
	const String ARC_SEC;
};

} /* namespace casa */
#endif /* COMPONENTLISTWRAPPER_H_ */
