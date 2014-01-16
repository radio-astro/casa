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
#include <QDebug>
namespace casa {

	ConverterChannel::ConverterChannel(const QString& oldUnits, const QString& newUnits) :
		Converter( oldUnits, newUnits) {

		//Old units will be channels.
		//New units could be anything.

	}


	double ConverterChannel::toPixel( double value, SpectralCoordinate spectralCoordinate ) {
		Double pixelValue;
		spectralCoordinate.toPixel( pixelValue, value );
		return pixelValue;
	}

	Vector<double> ConverterChannel::convert( const Vector<double>& oldValues, SpectralCoordinate spectralCoordinate ) {
		Vector<double> resultValues( oldValues.size());
		for ( int i = 0; i < static_cast<int>(resultValues.size()); i++ ) {
			Double result;
			bool correct = spectralCoordinate.toWorld( result, oldValues[i]);
			if ( correct ) {
				Vector<String> worldUnitsVector = spectralCoordinate.worldAxisUnits();
				QString worldUnit(worldUnitsVector[0].c_str());
				if ( worldUnit == newUnits ) {
					resultValues[i] = result;
				} else {
					Converter* helper = Converter::getConverter( worldUnit, newUnits);
					resultValues[i] = helper->convert( result, spectralCoordinate );
					delete helper;
				}
			} else {
				qDebug() << "Could not convert channel="<<oldValues[i];
			}
		}
		return resultValues;
	}

	double ConverterChannel::convert ( double oldValue, SpectralCoordinate spectralCoordinate ) {
		Vector<double> oldValues(1);
		oldValues[0] = oldValue;
		Vector<double> newValues = convert( oldValues, spectralCoordinate );
		return newValues[0];
	}

	ConverterChannel::~ConverterChannel() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
