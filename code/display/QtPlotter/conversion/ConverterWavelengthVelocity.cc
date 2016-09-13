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

#include "ConverterWavelengthVelocity.h"
#include <QDebug>
namespace casa {

	ConverterWavelengthVelocity::ConverterWavelengthVelocity(const QString& oldUnits,
	        const QString& newUnits) :
		ConverterWavelength( oldUnits, newUnits) {
	}

	Vector<double> ConverterWavelengthVelocity::convert( const Vector<double>& oldValues,
			SpectralCoordinate spectralCoordinate ) {
		Vector<double> resultValues( oldValues.size());

		bool validWavelength = spectralCoordinate.setWavelengthUnit( oldUnits.toStdString() );
		bool validVelocity = spectralCoordinate.setVelocity( newUnits.toStdString() );

		bool successfulConversion = false;
		if ( validWavelength && validVelocity ) {
			Vector<double> frequencyValues( oldValues.size());
			successfulConversion = spectralCoordinate.wavelengthToFrequency( frequencyValues,oldValues);
			if ( successfulConversion ) {
				successfulConversion = spectralCoordinate.frequencyToVelocity( resultValues, frequencyValues);
			}
		}
		if ( !successfulConversion ) {
			resultValues = oldValues;
			qDebug() << "Could not convert wavelength to velocity";
		}
		return resultValues;
	}
	ConverterWavelengthVelocity::~ConverterWavelengthVelocity() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
