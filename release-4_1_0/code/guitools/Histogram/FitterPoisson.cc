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
#include <scimath/Functionals/PoissonFunction.h>
#include <QDebug>
#include <assert.h>
#include <QtCore/qmath.h>

namespace casa {

FitterPoisson::FitterPoisson() {
	clearFit();
}

void FitterPoisson::clearFit(){
	Fitter::clearFit();
	lambdaSpecified = false;

}


float FitterPoisson::getFitCount() const {
	float count = 0;
	for ( int i = 0; i < static_cast<int>(actualYValues.size()); i++ ){
		float binCount = actualYValues[i];
		count = count + binCount;
	}
	return count;
}

void FitterPoisson::restrictDomain( double xMin, double xMax ){
	//Negative values don't make sense for a Poisson distribution.
	domainMin = qMax( 0.0, xMin );
	domainMax = qMax( 0.0, xMax );
	resetDataWithLimits();
}

QString FitterPoisson::getSolutionStatistics() const {
	QString result;
	if ( solutionConverged ){
		result.append( "The following fit was found:\n\n");
		result.append( formatResultLine( "Lambda:", solutionLambda, false)+ " "+ units + "\n");
		result.append( formatResultLine( "Height:", solutionHeight));
		result.append( formatResultLine( "Chi-square:", solutionChiSquared));
		int count = actualXValues.size();
		result.append( formatResultLine( "Degrees of Freedom:", count));
	}
	else {
		result.append( "Fit did not converge.\n");
	}
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
		//Shouldn't change anything if they have already been
		//set, but we only want to fit nonnegative values.
		restrictDomain( domainMin, domainMax );
		fitValues.resize( actualXValues.size());
		NonLinearFitLM<Float> fitter(0);

		fitter.setMaxIter(1024);
		fitter.setCriteria(0.0001);
		float height = getFitCount();
		PoissonFunction<Float> poissonFunction(lambda, height);
		fitter.setFunction(poissonFunction);

		//Initialize the x-data values
		Matrix<Float> components;
		int xCount = actualXValues.size();
		components.resize(xCount,1);
		for ( int i = 0; i < xCount; i++ ){
			components(i,0) = actualXValues[i];
		}

		Vector<Float> sigma(actualYValues.size(), 1);

		try {
			Matrix<Float> solution = fitter.fit(components, actualYValues, sigma);
			solutionConverged = fitter.converged();
			if ( solutionConverged ){
				solutionChiSquared = fitter.chiSquare();
				if (solutionChiSquared < 0) {
					errorMsg= "Unsuccessful Fit: ChiSquare of ";
					errorMsg.append(QString::number( solutionChiSquared ));
					errorMsg.append("is negative.");
				    fitSuccessful = false;
				}
				else if (isNaN(solutionChiSquared)){
				    errorMsg= "Unsuccessful Fit: ChiSquare was a NaN.";
				    fitSuccessful = false;
				}
				else {
					//solutionRMS = solutionChiSquared / actualYValues.size();
					solutionLambda = solution( 0, 0 );
					solutionHeight = solution( 1, 0 );
					PoissonFunction<Float> poissonFit(solutionLambda, solutionHeight );
					for( int i = 0; i < static_cast<int>(actualXValues.size()); i++ ){
						fitValues[i] = poissonFit.eval(&actualXValues[i]);
					}
				}
			}
			else {
				fitSuccessful = false;
			}
			dataFitted = true;
		}

		catch (AipsError& err) {
			qDebug() << "ERROR: " << err.what();
			fitSuccessful = false;
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
	if ( solutionConverged ){
		const QString END_LINE( "\n");
		stream << "#Poisson Fit" << END_LINE;
		stream << "#Lambda: "<<solutionLambda<< END_LINE;
		stream << "#Height: "<<solutionHeight<< END_LINE;
		Fitter::toAscii(stream);
	}
}

FitterPoisson::~FitterPoisson() {
}

} /* namespace casa */
