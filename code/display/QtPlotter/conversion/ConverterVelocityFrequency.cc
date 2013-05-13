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

#include "ConverterVelocityFrequency.h"
#include <QDebug>
#include <assert.h>

namespace casa {

	ConverterVelocityFrequency::ConverterVelocityFrequency(const QString& oldUnits,const QString& newUnits) :
		ConverterVelocity( oldUnits, newUnits) {
	}


	void ConverterVelocityFrequency::convertFrequency( Vector<double> &resultValues,
	        QString& frequencySourceUnits) {
		//Decide on the multiplier
		int sourceUnitIndex = FREQUENCY_UNITS.indexOf( frequencySourceUnits );
		int destUnitIndex = FREQUENCY_UNITS.indexOf( newUnits );
		Converter::convert( resultValues, sourceUnitIndex, destUnitIndex );
	}

	Vector<double> ConverterVelocityFrequency::convert( const Vector<double>& oldValues ) {
		bool unitsUnderstood = setVelocityUnits( oldUnits );
		Vector<double> resultValues(oldValues.size());
		bool successfulConversion = false;
		if ( unitsUnderstood ) {
			successfulConversion = spectralCoordinate.velocityToFrequency( resultValues, oldValues );
			if ( successfulConversion ) {
				//The frequency unit will be whatever the spectralCoordinate is currently using.
				Vector<String> frequencyUnits = spectralCoordinate.worldAxisUnits();
				assert (frequencyUnits.size() == 1);
				String frequencyUnit = frequencyUnits[0];
				QString freqUnitStr( frequencyUnit.c_str());
				//Now we convert it to appropriate units;
				convertFrequency( resultValues, freqUnitStr );
			}
		}
		if ( !successfulConversion ) {
			resultValues = oldValues;
			qDebug() << "Could not convert velocity to frequency";
		}
		return resultValues;
	}

	ConverterVelocityFrequency::~ConverterVelocityFrequency() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
