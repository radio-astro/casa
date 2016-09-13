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

#include "ConverterWavelength.h"
#include <QDebug>

namespace casa {

	ConverterWavelength::ConverterWavelength(const QString& oldUnits,const QString& newUnits ) :
		Converter( oldUnits, newUnits ) {

	}

	double ConverterWavelength::toPixel( double value, SpectralCoordinate spectralCoordinate) {
		spectralCoordinate.setWavelengthUnit( oldUnits.toStdString() );
		Vector<Double> frequencyVector(1);
		Vector<Double> wavelengthVector(1);
		wavelengthVector[0] = value;
		spectralCoordinate.wavelengthToFrequency(frequencyVector, wavelengthVector );
		Double pixelValue;
		spectralCoordinate.toPixel( pixelValue, frequencyVector[0]);
		return pixelValue;
	}

	Vector<double> ConverterWavelength::convert( const Vector<double>& oldValues,
			SpectralCoordinate spectralCoordinate) {
		Vector<double> resultValues( oldValues.size() );
		resultValues = oldValues;
		convertWavelength( resultValues, oldUnits, newUnits, spectralCoordinate );
		return resultValues;
	}

	void ConverterWavelength::convertWavelength( Vector<double> &resultValues,
	        QString& sourceUnits, QString& destUnits, SpectralCoordinate& coord) {
		int sourceIndex = Converter::WAVELENGTH_UNITS.indexOf( sourceUnits );
		int destIndex = Converter::WAVELENGTH_UNITS.indexOf( destUnits );
		Converter::convert( resultValues, sourceIndex, destIndex, coord );
	}


	ConverterWavelength::~ConverterWavelength() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
