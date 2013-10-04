/*
 * ConverterVelocityWavelength.cc
 *
 *  Created on: Jul 16, 2012
 *      Author: slovelan
 */

#include "ConverterVelocityWavelength.h"
#include <QDebug>
namespace casa {

	ConverterVelocityWavelength::ConverterVelocityWavelength(const QString& oldUnits,const QString& newUnits) :
		ConverterVelocity( oldUnits, newUnits ) {
	}

	Vector<double> ConverterVelocityWavelength::convert( const Vector<double>& oldValues ) {
		Vector<double> resultValues( oldValues.size());
		bool velocitySet = setVelocityUnits( oldUnits );
		bool wavelengthSet = setWavelengthUnits( newUnits );
		bool successfulConversion = false;
		if ( velocitySet && wavelengthSet ) {
			Vector<double> frequencyValues( oldValues.size());
			successfulConversion = spectralCoordinate.velocityToFrequency( frequencyValues, oldValues );
			if ( successfulConversion ) {
				successfulConversion = spectralCoordinate.frequencyToWavelength( resultValues , frequencyValues );
			}
		}
		if ( !successfulConversion ) {
			resultValues = oldValues;
			qDebug() << "Could not convert velocity to wavelength";
		}
		return resultValues;
	}
	ConverterVelocityWavelength::~ConverterVelocityWavelength() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
