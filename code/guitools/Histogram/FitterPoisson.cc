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

#include "FitterPoisson.h"
#include <scimath/Fitting/NonLinearFitLM.h>
#include <QDebug>
#include <assert.h>
#include <QtCore/qmath.h>

namespace casa {

FitterPoisson::FitterPoisson() {
	lambdaSpecified = false;
}

void FitterPoisson::clearFit(){
	dataFitted = false;
	lambdaSpecified = false;
	errorMsg = "";
	statusMsg = "";
}

bool FitterPoisson::isIntegerValue( float val ) const {
	bool valueInt = false;
	if ( qAbs(val - (int)val) < .0000000001 ){
		valueInt = true;
	}
	return valueInt;
}

int FitterPoisson::factorial( int n ) const {
	assert( n >= 0 );
	int factValue = 1;
	if ( n > 1 ){
		factValue = n * factorial( n - 1);
	}
	return factValue;
}

QString FitterPoisson::getSolutionStatistics() const {
	QString result( "Lambda: ");
	result.append( QString::number( lambda ));
	return result;
}

bool FitterPoisson::doFit(){
	if ( !lambdaSpecified ){
		lambda = getMean();
	}
	bool fitSuccessful = true;
	if ( lambda < 0 ){
		fitSuccessful = false;
		QString lambdaStr = QString::number( lambda );
		errorMsg = "Could not fit a Poisson distribution because the lambda value: "+lambdaStr+" was not positive.";
	}
	else {

		fitValues.resize( xValues.size());
		bool allInts = true;
		bool positiveValues = true;
		for( int i = 0; i < static_cast<int>(xValues.size()); i++ ){
			if ( !isIntegerValue( xValues[i])){
				allInts = false;
			}
			int xVal = (int)( xValues[i]);
			if ( xVal < 0 ){
				xVal = 0;
				positiveValues = false;
			}
			fitValues[i] = qPow( lambda, xVal ) * qExp(-1 * lambda ) / factorial( xVal );
			dataFitted = true;
		}
		if ( !positiveValues ){
			statusMsg = "Negative domain values were replaced with 0.";
		}
		if ( !allInts ){
			statusMsg = "Floating point domain values were rounded to the nearest integer.";
		}
	}
	return fitSuccessful;
}

void FitterPoisson::setLambda( double value ){
	lambda = value;
	lambdaSpecified = true;
}

double FitterPoisson::getLambda() const {
	return lambda;
}

void FitterPoisson::toAscii( QTextStream& stream ) const {
	const QString END_LINE( "\n");
	stream << "Poisson Fit" << END_LINE;
	stream << "Lambda: "<<lambda<< END_LINE << END_LINE;
	Fitter::toAscii(stream);
}

FitterPoisson::~FitterPoisson() {
}

} /* namespace casa */
