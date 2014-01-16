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

#include "ConverterFrequencyVelocity.h"
#include <QDebug>

namespace casa {

	ConverterFrequencyVelocity::ConverterFrequencyVelocity(const QString& oldUnits,
	        const QString& newUnits):
		ConverterFrequency( oldUnits, newUnits) {
	}




	Vector<double> ConverterFrequencyVelocity::convert( const Vector<double>& oldValues,
			SpectralCoordinate spectralCoordinate ) {
		Vector<double> frequencyValues(oldValues.size());
		frequencyValues = oldValues;

		//Find out the frequency units the spectral coordinate is using and
		//compare them to the frequency units we are using.  Transform the
		//data if necessary to the units used by the spectral coordinate.
		Vector<String> spectralUnits = spectralCoordinate.worldAxisUnits();
		String spectralUnit = spectralUnits[0];
		QString spectralUnitStr( spectralUnit.c_str() );
		if ( spectralUnitStr != oldUnits ) {
			ConverterFrequency::convertFrequency( frequencyValues, oldUnits, spectralUnitStr, spectralCoordinate );
		}
		bool unitsUnderstood = spectralCoordinate.setVelocity( newUnits.toStdString() );
		bool successfulConversion = false;
		Vector<double> resultValues( oldValues.size());
		if ( unitsUnderstood ) {
			successfulConversion = spectralCoordinate.frequencyToVelocity( resultValues, frequencyValues );
		}
		if ( !successfulConversion ) {
			resultValues = oldValues;
			qDebug() << "Could not convert frequency to velocity";
		}
		return resultValues;
	}

	ConverterFrequencyVelocity::~ConverterFrequencyVelocity() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
