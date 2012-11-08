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

#include "ConverterIntensity.h"
#include <display/QtPlotter/conversion/Converter.h>
#include <math.h>
#include <QDebug>

namespace casa {

const QString ConverterIntensity::FRACTION_OF_PEAK = "Fraction of Peak";
const QString ConverterIntensity::JY_BEAM = "Jy/beam";
const QString ConverterIntensity::JY_SR = "MJy/sr";
const QString ConverterIntensity::JY_ARCSEC = "Jy/arcsec^2";
const QString ConverterIntensity::JY = "Jy";
const QString ConverterIntensity::KELVIN = "Kelvin";
const QString ConverterIntensity::ADU = "adu";
const QString ConverterIntensity::TIMES_PIXELS = "*pixels";
const double ConverterIntensity::SPEED_LIGHT_FACTOR = 0.0000000009;
const double ConverterIntensity::FREQUENCY_FACTOR = 2 * 0.0000000000000000000000138;
double ConverterIntensity::beamSolidAngle = 0;

const QList<QString> ConverterIntensity::BEAM_UNITS =
	QList<QString>() << "pJy/beam" <<"10pJy/beam"<<"100pJy/beam"<<
		"nJy/beam"<<"10nJy/beam"<<"100nJy/beam"<<
		"uJy/beam"<<"10uJy/beam"<<"100uJy/beam"<<
		"mJy/beam"<<"10mJy/beam"<<"100mJy/beam"<<
		"Jy/beam"<<"10Jy/beam"<<"100Jy/beam"<<
		"kJy/beam"<<"10kJy/beam"<<"100kJy/beam"<<
		"MJy/beam"<<"10MJy/beam"<<"100MJy/beam"<<
		"GJy/beam";

const QList<QString> ConverterIntensity::JY_UNITS =
	QList<QString>() << "pJy" <<"10pJy"<<"100pJy"<<
		"nJy"<<"10nJy"<<"100nJy"<<
		"uJy"<<"10uJy"<<"100uJy"<<
		"mJy"<<"10mJy"<<"100mJy"<<
		"Jy"<<"10Jy"<<"100Jy"<<
		"kJy"<<"10kJy"<<"100kJy"<<
		"MJy"<<"10MJy"<<"100MJy"<<
		"GJy";


ConverterIntensity::ConverterIntensity() {
}

void ConverterIntensity::setSolidAngle( double angleMeasure ){
	beamSolidAngle = angleMeasure;
}

bool ConverterIntensity::isSupportedUnits( const QString& yUnit ){
	bool acceptable = false;
	if ( yUnit.contains( "Jy") || yUnit.contains( KELVIN )||
			yUnit.contains( FRACTION_OF_PEAK) || yUnit.contains("Jy/arcsec^2") ||
			yUnit.contains( "MJy/sr" ) ){
		acceptable = true;
	}
	return acceptable;
}

QString ConverterIntensity::stripPixels( const QString& units ){
	int pixelIndex = units.indexOf( TIMES_PIXELS );
	QString strippedUnits = units;
	if ( pixelIndex > 0 ){
		strippedUnits = units.left(pixelIndex );
	}
	return strippedUnits;
}

void ConverterIntensity::convert( Vector<float>& values, const Vector<float> hertzValues,
		const QString& oldUnits, const QString& newUnits,
		double maxValue, const QString& maxUnits ){

	bool supportedUnits = isSupportedUnits( oldUnits );
	if ( supportedUnits ){
		supportedUnits = isSupportedUnits( newUnits );
	}
	if ( !supportedUnits ){
		return;
	}

	QString newUnitsBase = stripPixels( newUnits );
	QString oldUnitsBase = stripPixels( oldUnits );
	QString maxUnitsBase = stripPixels( maxUnits );

	//Change fraction of peak back to the original units before converting.  We don't
	//want the current values in fraction of peak going forward.
	QString baseConvertUnits = oldUnitsBase;
	int maxPoints = values.size();
	if ( oldUnitsBase == FRACTION_OF_PEAK && newUnitsBase != FRACTION_OF_PEAK){
		for ( int i = 0; i < maxPoints; i++ ){
			values[i] = percentToValue( values[i], maxValue);
		}
		baseConvertUnits = maxUnitsBase;
	}

	//Exit if we don't have anything to do.
	if ( baseConvertUnits == newUnitsBase ){
			return;
	}

	if ( newUnitsBase == FRACTION_OF_PEAK ){
		//Scale the vector
		for ( int i = 0; i < maxPoints; i++ ){
			values[ i ] = valueToPercent( values[i], maxValue );
		}
	}
	//Converting between Jy/beam.
	else if ( isJansky( baseConvertUnits) && isJansky( newUnitsBase )){
		convertJansky( values, baseConvertUnits, newUnitsBase );
	}
	else {
		//If the original units are in JY or JY_BEAM, strip off a prefix such as
		//mJy and adjust the data.
		QString strippedBase = baseConvertUnits;
		if ( isJansky( baseConvertUnits ) ){
			strippedBase = getJanskyBaseUnits( baseConvertUnits );
			convertJansky( values, baseConvertUnits, strippedBase );
		}
		QString strippedNew = newUnitsBase;
		if ( isJansky( newUnitsBase)){
			strippedNew = getJanskyBaseUnits( newUnitsBase );
		}

		//Use Quanta to convert
		for ( int i = 0; i < maxPoints; i++ ){
			values[ i ] = convertQuantity( values[i], hertzValues[i],
					strippedBase, strippedNew );
		}

		if ( isJansky( newUnitsBase ) ){
			convertJansky( values, strippedNew, newUnitsBase );
		}
	}

}

QString ConverterIntensity::getJanskyBaseUnits( const QString& units ){
	QString baseUnits = units;
	int jyIndex = units.indexOf( JY );
	if ( jyIndex > 0 ){
		baseUnits = units.mid( jyIndex, units.length() - jyIndex );
	}
	return baseUnits;
}

bool ConverterIntensity::isJansky( const QString& units ){
	bool janskyUnits = false;
	if ( units.indexOf( JY ) > 0 ){
		if ( units.indexOf( JY_ARCSEC) < 0 && units.indexOf( JY_SR) < 0 ){
			janskyUnits = true;
		}
	}
	return janskyUnits;
}

void ConverterIntensity::convertJansky( Vector<float>& values, const QString& oldUnits,
		const QString& newUnits ){
	for ( int i = 0; i < static_cast<int>(values.size()); i++ ){
		if ( oldUnits.indexOf( JY_BEAM) > 0 && newUnits.indexOf( JY_BEAM) > 0 ){
			values[i] = convertJyBeams( oldUnits, newUnits, values[i]);
		}
		else {
			values[i] = convertJY( oldUnits, newUnits, values[i]);
		}
	}
}


double ConverterIntensity::valueToPercent( double yValue, double maxValue ) {
	double convertedYValue = yValue / maxValue;
	return convertedYValue;
}


double ConverterIntensity::percentToValue( double yValue, double maxValue ) {
	double convertedYValue = yValue * maxValue;
	return convertedYValue;
}


double ConverterIntensity::convertQuantity( double yValue, double frequencyValue,
		const QString& oldUnits, const QString& newUnits ){

	String oldUnitStr = oldUnits.toStdString();
	String newUnitStr = newUnits.toStdString();

	double convertedYValue = yValue;
	if ( oldUnits != KELVIN && newUnits != KELVIN ){
		Quantity quantity( yValue, oldUnitStr );
		Unit newUnitVal( newUnitStr );
		quantity.convert( newUnitStr );
		convertedYValue = quantity.getValue();
	}
	else if ( oldUnits == KELVIN && newUnits != KELVIN ) {
		if ( beamSolidAngle > 0 ){
			//kelvin * solidAngle * 2 * 1.38 x 10^-23 * freq^2 / (10^-32 x (3 x 10^8)^2)
			double num = yValue * beamSolidAngle * FREQUENCY_FACTOR * pow( frequencyValue, 2);
			double den = SPEED_LIGHT_FACTOR;
			double jyBeamValue = num / den;

			//New units are not JY/BEAM convert them to what we need
			String strUnit(JY_BEAM.toStdString() );
			Quantity quantityJyBeam( jyBeamValue, strUnit );
			quantityJyBeam.convert( newUnitStr );
			convertedYValue = quantityJyBeam.getValue();
		}
		else {
			qDebug() << "Could not convert from Kelvin because the beam solid angle was 0";
		}
	}
	else {
		//oldUnits != KELVIN && newUnits == KELVIN
		if ( beamSolidAngle > 0 ){
			//First convert the value to Jy/Beam
			String jyBeamUnits = JY_BEAM.toStdString();
			Quantity quantity( yValue, oldUnitStr );
			quantity.convert( jyBeamUnits );
			double valueBeam = quantity.getValue();

			//Temperature in Kelvin is now:
			//Jy/beam x 10^(-32) x (3 x 10^8)^2 / ( solidAngle x 2 x 1.38 x 10^-23 x (xvalueinHz)^2 ).
			double num = valueBeam * SPEED_LIGHT_FACTOR;
			double den = beamSolidAngle * FREQUENCY_FACTOR * pow(frequencyValue,2);
			convertedYValue = num / den;
		}
		else {
			qDebug() << "Could not convert to Kelvin because the beamSolidAngle was 0";
		}
	}
	return convertedYValue;
}

double ConverterIntensity::convertJY( const QString& oldUnits,
		const QString& newUnits, double value ){
	int sourceIndex = JY_UNITS.indexOf( oldUnits );
	int destIndex = JY_UNITS.indexOf( newUnits );
	Vector<double> resultValues(1);
	resultValues[0] = value;
	Converter::convert( resultValues, sourceIndex, destIndex );
	return resultValues[0];
}

double ConverterIntensity::convertJyBeams( const QString& oldUnits,
		const QString& newUnits, double value ){
	int sourceIndex = BEAM_UNITS.indexOf( oldUnits );
	int destIndex = BEAM_UNITS.indexOf( newUnits );
	Vector<double> resultValues(1);
	resultValues[0] = value;
	Converter::convert( resultValues, sourceIndex, destIndex );
	return resultValues[0];
}

ConverterIntensity::~ConverterIntensity() {
}

} /* namespace casa */
