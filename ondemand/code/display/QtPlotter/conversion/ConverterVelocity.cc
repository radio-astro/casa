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

#include "ConverterVelocity.h"
#include <QDebug>

namespace casa {

	ConverterVelocity::ConverterVelocity(const QString& oldUnits, const QString& newUnits) :
		Converter( oldUnits, newUnits) {

	}

	double ConverterVelocity::toPixel( double value, SpectralCoordinate spectralCoordinate ) {
		Double pixelVal;
		spectralCoordinate.setVelocity( oldUnits.toStdString() );
		spectralCoordinate.velocityToPixel( pixelVal, value );
		return pixelVal;
	}

	Vector<double> ConverterVelocity::convert( const Vector<double>& oldValues, SpectralCoordinate spectralCoordinate ) {
		Vector<double> resultValues( oldValues.size() );
		resultValues = oldValues;
		convertVelocity( resultValues, oldUnits, newUnits, spectralCoordinate );
		return resultValues;
	}

	void ConverterVelocity::convertVelocity( Vector<double> &resultValues,
	        QString& sourceUnits, QString& destUnits, SpectralCoordinate& coord ) {
		int sourceIndex = Converter::VELOCITY_UNITS.indexOf( sourceUnits );
		int destIndex = Converter::VELOCITY_UNITS.indexOf( destUnits );
		Converter::convert( resultValues, sourceIndex, destIndex, coord );
	}


	ConverterVelocity::~ConverterVelocity() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
