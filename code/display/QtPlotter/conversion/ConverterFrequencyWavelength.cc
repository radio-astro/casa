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

#include "ConverterFrequencyWavelength.h"
#include <QDebug>
namespace casa {

	ConverterFrequencyWavelength::ConverterFrequencyWavelength(const QString& oldUnits,
	        const QString& newUnits):
		ConverterFrequency( oldUnits, newUnits) {

	}

	Vector<double> ConverterFrequencyWavelength::convert( const Vector<double>& oldValues ) {

		Vector<double> resultValues(oldValues.size());
		resultValues = oldValues;

		bool unitsUnderstood = setWavelengthUnits( newUnits );
		bool successfulConversion = false;
		if ( unitsUnderstood ) {

			successfulConversion = spectralCoordinate.frequencyToWavelength( resultValues, oldValues );

			if ( successfulConversion ) {
				Vector<String> spectralUnits = spectralCoordinate.worldAxisUnits();
				String spectralUnit = spectralUnits[0];
				QString spectralUnitStr( spectralUnit.c_str() );
				if ( spectralUnitStr != oldUnits ) {
					ConverterFrequency::convertFrequency( resultValues, spectralUnitStr, oldUnits );
				}
			}
		}
		if ( !successfulConversion ) {
			qDebug() << "Could not convert frequency to wavelength";
		}
		return resultValues;
	}

	ConverterFrequencyWavelength::~ConverterFrequencyWavelength() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
