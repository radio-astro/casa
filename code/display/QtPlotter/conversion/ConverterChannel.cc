//# Copyright (C) 2005
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

#include "ConverterChannel.h"
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <QDebug>
namespace casa {

ConverterChannel::ConverterChannel(SpectralCoordinate* spectralCoordinate) : Converter( "", "", spectralCoordinate){
	Vector<String> newUnitVector = spectralCoordinate->worldAxisUnits();
	newUnits = newUnitVector[0].c_str();
	oldUnits = "channels";
}


double ConverterChannel::toPixel( double value ){
	Double pixelValue;
	spectralCoordinate->toPixel( pixelValue, value );
	return pixelValue;
}

Vector<double> ConverterChannel::convert( const Vector<double>& oldValues ){
	Vector<double> resultValues( oldValues.size());

	for ( int i = 0; i < resultValues.size(); i++ ){
		Double result;
		bool correct = spectralCoordinate->toWorld( result, oldValues[i]);
		if ( correct ){
			resultValues[i] = result;
		}
		else {
			qDebug() << "Could not convert channel="<<oldValues[i];
		}
	}
	return resultValues;
}

double ConverterChannel::convert ( double oldValue ){
	Double worldValue;
	spectralCoordinate->toWorld( worldValue, oldValue );
	return worldValue;
}

ConverterChannel::~ConverterChannel() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
