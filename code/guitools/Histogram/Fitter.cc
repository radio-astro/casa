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

#include "Fitter.h"
#include <QDebug>
#include <limits>

namespace casa {

Fitter::Fitter() {
	clearDomainLimits();
}

void Fitter::clearDomainLimits(){
	domainMin = -1*std::numeric_limits<float>::max();
	domainMax = std::numeric_limits<float>::max();
	resetDataWithLimits();
}

void Fitter::setData( Vector<Float> dataValuesX, Vector<Float> dataValuesY ){
	clearFit();

	int xValueSize = dataValuesX.size();
	xValues.resize( xValueSize );
	for ( int i = 0; i < xValueSize; i++ ){
		xValues[i]= dataValuesX[i];
	}
	int yValueSize = dataValuesY.size();
	yValues.resize( yValueSize );
	for ( int i = 0; i < yValueSize; i++ ){
		yValues[i] = dataValuesY[i];
	}
	resetDataWithLimits();

	//For testing a poisson distribution
	//Lambda = 0.61
	/*const int HEIGHT = 10000;
	xValues.resize(7);
	xValues[0] = 0;
	xValues[1] = 1;
	xValues[2] = 2;
	xValues[3] = 3;
	xValues[4] = 4;
	xValues[5] = 5;
	xValues[6] = 6;
	yValues.resize(7);
	yValues[0] = .54335* HEIGHT;
	yValues[1] = 0.33145*HEIGHT;
	yValues[2] = 0.10110*HEIGHT;
	yValues[3] = 0.02055*HEIGHT;
	yValues[4] = 0.00315*HEIGHT;
	yValues[5] = 0.00040*HEIGHT;
	yValues[6] = 0.00005*HEIGHT;
	resetDataWithLimits();*/
}

void Fitter::restrictDomain( double xMin, double xMax ){
	domainMin = xMin;
	domainMax = xMax;
	resetDataWithLimits();
}

void Fitter::setUnits( QString units ){
	this->units = units;
}

void Fitter::resetDataWithLimits(){
	int count = 0;
	Vector<Float>::iterator domainIterator = xValues.begin();
	while ( domainIterator != xValues.end()){
		if ( (*domainIterator)>= domainMin && (*domainIterator)<= domainMax ){
			count++;
		}
		domainIterator++;
	}
	actualXValues.resize( count );
	actualYValues.resize( count );

	int j = 0;
	int xValueCount = xValues.size();
	for( int i = 0; i < xValueCount; i++ ){
		if ( xValues[i]>= domainMin && xValues[i]<= domainMax ){
			actualXValues[j] = xValues[i];
			actualYValues[j] = yValues[i];
			j++;
		}
	}
}

QString Fitter::formatResultLine( QString label, float value, bool endLine ) const {
	QString resultLine( label );
	resultLine.append( "\t");
	if ( label.length() < 15 ){
		resultLine.append( "\t");
	}
	resultLine.append( QString::number( value));
	if ( endLine ){
		resultLine.append( "\n");
	}
	return resultLine;
}


bool Fitter::isFit() const {
	return dataFitted;
}

void Fitter::clearFit(){
	fitValues.resize( 0 );
	errorMsg = "";
	statusMsg = "";
	dataFitted = false;
	solutionConverged = false;
}

Vector<Float> Fitter::getFitValues() const {
	return fitValues;
}

Vector<Float> Fitter::getFitValuesX() const {
	return actualXValues;
}

QString Fitter::getErrorMessage() const {
	return errorMsg;
}

QString Fitter::getStatusMessage() const {
	return statusMsg;
}

void Fitter::toAscii( QTextStream& out ) const {
	const QString LINE_END = "\n";
	out << "#Chi-square:"<<solutionChiSquared<<LINE_END;
	int count = fitValues.size();
	out << "#Degrees of Freedom:"<<count<<LINE_END;
	out << "#Value"<<"Count"<<"Fit Count"<< LINE_END;
	for ( int i = 0; i < count; i++ ){
		out << QString::number( actualXValues[i])
		<< QString::number( actualYValues[i])
		<< QString::number( fitValues[i]) << LINE_END;
	}
}

float Fitter::getMean() const {
	int count = xValues.size();
	float totalCount = 0;
	float weightedMean = 0;
	if ( count > 0 ){
		float sum = 0;
		for ( int i = 0; i < count; i++ ){
			sum = sum + xValues[i] * yValues[i];
			totalCount = totalCount + yValues[i];
		}
		if ( totalCount > 0 ){
			weightedMean = sum / totalCount;
		}
	}
	return weightedMean;
}

Fitter::~Fitter() {
}

} /* namespace casa */
