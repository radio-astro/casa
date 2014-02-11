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

#include "Converter.h"

#include <display/QtPlotter/conversion/ConverterFrequencyVelocity.h>
#include <display/QtPlotter/conversion/ConverterFrequencyWavelength.h>
#include <display/QtPlotter/conversion/ConverterVelocityFrequency.h>
#include <display/QtPlotter/conversion/ConverterVelocityWavelength.h>
#include <display/QtPlotter/conversion/ConverterWavelengthFrequency.h>
#include <display/QtPlotter/conversion/ConverterWavelengthVelocity.h>
#include <display/QtPlotter/conversion/ConverterFrequency.h>
#include <display/QtPlotter/conversion/ConverterVelocity.h>
#include <display/QtPlotter/conversion/ConverterWavelength.h>
#include <display/QtPlotter/conversion/ConverterChannel.h>
#include <QDebug>

namespace casa {

	const QList<QString> Converter::FREQUENCY_UNITS =
	    QList<QString>() << "Hz" << "10Hz"<<"100Hz"<< "KHz" <<
	    "10KHz" << "100KHz" << "MHz" <<
	    "10MHz" << "100MHz" << "GHz";
	const QList<QString> Converter::WAVELENGTH_UNITS =
	    QList<QString>() << "Angstrom" << "nm" << "10nm" << "100nm" << "um" <<
	    "10um" << "100um" << "mm" << "cm" << "dm"<<"m";
	const QList<QString> Converter::VELOCITY_UNITS =
	    QList<QString>() << "m/s" << "10m/s" << "100m/s" << "km/s";


	Converter* Converter::getConverter( const QString& oldUnits,
	                                    const QString& newUnits) {
		Converter* converter = NULL;
		UnitType sourceUnitType = getUnitType( oldUnits );
		UnitType destUnitType = getUnitType( newUnits );
		if ( sourceUnitType == FREQUENCY_UNIT ) {
			if ( destUnitType == WAVELENGTH_UNIT || newUnits.isEmpty() ) {
				converter = new ConverterFrequencyWavelength( oldUnits, newUnits);
			} else if ( destUnitType == VELOCITY_UNIT ) {
				converter = new ConverterFrequencyVelocity( oldUnits, newUnits );
			} else if ( destUnitType == FREQUENCY_UNIT ) {
				converter = new ConverterFrequency( oldUnits, newUnits );
			}
		} else if ( sourceUnitType == VELOCITY_UNIT ) {
			if ( destUnitType == WAVELENGTH_UNIT || newUnits.isEmpty()) {
				converter = new ConverterVelocityWavelength( oldUnits, newUnits);
			} else if ( destUnitType == VELOCITY_UNIT ) {
				converter = new ConverterVelocity( oldUnits, newUnits );
			} else if ( destUnitType == FREQUENCY_UNIT ) {
				converter = new ConverterVelocityFrequency( oldUnits, newUnits );
			}
		} else if ( sourceUnitType == WAVELENGTH_UNIT ) {
			if ( destUnitType == WAVELENGTH_UNIT ) {
				converter = new ConverterWavelength( oldUnits, newUnits );
			} else if ( destUnitType == VELOCITY_UNIT || newUnits.isEmpty() ) {
				converter = new ConverterWavelengthVelocity( oldUnits, newUnits );
			} else if ( destUnitType == FREQUENCY_UNIT ) {
				converter = new ConverterWavelengthFrequency( oldUnits, newUnits );
			}
		} else if ( sourceUnitType == CHANNEL_UNIT ) {
			converter = new ConverterChannel( oldUnits, newUnits );
		}
		return converter;
	}

	Converter::UnitType Converter::getUnitType( const QString& unit ) {
		UnitType unitType = UNRECOGNIZED;
		if ( FREQUENCY_UNITS.contains( unit )) {
			unitType = FREQUENCY_UNIT;
		} else if ( WAVELENGTH_UNITS.contains( unit )) {
			unitType = WAVELENGTH_UNIT;
		} else if ( VELOCITY_UNITS.contains( unit )) {
			unitType = VELOCITY_UNIT;
		} else if ( unit.isEmpty() ) {
			unitType = CHANNEL_UNIT;
		}
		return unitType;
	}

	Converter::Converter( const QString& oldUnitsStr, const QString& newUnitsStr):
		oldUnits( oldUnitsStr), newUnits( newUnitsStr ) {
	}

	QString Converter::getNewUnits() const {
		return newUnits;
	}

	double Converter::convert ( double oldValue, SpectralCoordinate spectralCoordinate ) {
		Vector<double> sourceValues( 1 );
		sourceValues[0] = oldValue;
		Vector<double> destValues = convert( sourceValues, spectralCoordinate );
		double result = destValues[0];
		return result;
	}

	void Converter::convert( Vector<double> &resultValues, int sourceIndex, int destIndex, SpectralCoordinate /*spectralCoordinate*/) {
		if ( sourceIndex >= 0 && destIndex >= 0 ) {
			int diff = qAbs( destIndex - sourceIndex );
			float power = pow( 10, diff );
			if ( destIndex > sourceIndex ) {
				power = 1 / power;
			}
			for ( int i = 0; i < static_cast<int>(resultValues.size()); i++ ) {
				resultValues[i] = resultValues[i] * power;
			}
		} else {
			/*qDebug() <<  "Converter: could not convert sourceIndex=" <<
						sourceIndex << " destIndex=" << destIndex;*/
		}
	}


	Converter::~Converter() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
