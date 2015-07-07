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
		if ( actualYValues[i] > count ){
			float binCount = actualYValues[i];
			count = count + binCount;
		}
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
		result.append( formatResultLine( "Lambda:", actualLambda, false)+ " "+ units + "\n");
		//result.append( formatResultLine( "Fit Count:", solutionHeight));
		result.append( formatResultLine( "Chi-square:", solutionChiSquared));
		int count = actualXValues.size();
		result.append( formatResultLine( "Degrees of Freedom:", count));
	}
	else {
		result.append( "Fit did not converge.\n");
	}
	return result;
}

pair<double,double> FitterPoisson::_getMinMax() const {
	int xCount = actualXValues.size();
	pair<double,double> minMax(-1,-1 );
	if ( xCount > 0 ){
		minMax.first = actualXValues[0];
		minMax.second = actualXValues[xCount - 1];
	}
	return minMax;
}


bool FitterPoisson::doFit(){
	bool fitSuccessful = true;
	if ( !lambdaSpecified ){
		lambda = getMean();
	}


	//Shouldn't change anything if they have already been
	//set, but we only want to fit nonnegative values.
	restrictDomain( domainMin, domainMax );
	fitValues.resize( actualXValues.size());
	NonLinearFitLM<Double> fitter(0);

	fitter.setMaxIter(1024);
	fitter.setCriteria(0.0001);
	float height = getFitCount();

	//Change xvalues to numbers between 0 and 10. Note:  we are doing this
	//because of CAS-7531.  A Poisson distribution is an approximation to a
	//binomial distribution for large sample sizes.  But the values that make
	//sense for a Poisson distribution are positive integers 0,1,2,3,....  The
	//histogram, however, may have an x-data range from, say 0.001 to 0.01.  Thus,
	//trying to fit a Poisson distribution to it does not make sense unless we
	//rescale the x-data range to include positive integer values.  The values
	//computed for a Poisson distribution can quickly get out of hand so we rescale
	//with an upper bound of 10.
	int xCount = actualXValues.size();
	const int FIT_X_LOW = 0;
	const int FIT_X_HIGH = 10;

	std::pair<double,double> domainBounds = _getMinMax();
	if ( lambda < domainBounds.first || lambda > domainBounds.second  ){
		fitSuccessful = false;
		QString lambdaStr = QString::number( lambda );
		errorMsg = "Could not fit a Poisson distribution because the lambda value: "+lambdaStr+" was outside data range.";
	}
	else if ( domainBounds.first == domainBounds.second ){
		fitSuccessful = false;
		errorMsg = "There was not enough data to fit a Poisson distribution.";
	}
	else {
		double domainRange = domainBounds.second - domainBounds.first;
		double fitRange = FIT_X_HIGH - FIT_X_LOW;
		Vector<double> fitXValues( xCount );
		Vector<double> fitYValues( xCount );
		//Initialize the x-data values
		Matrix<Double> components;
		components.resize(xCount,1);
		for ( int i = 0; i < xCount; i++ ){
			double percent = (actualXValues[i] - domainBounds.first) / domainRange;
			fitXValues[i] = FIT_X_LOW + fitRange * percent;
			components(i,0) = fitXValues[i];
			fitYValues[i] = actualYValues[i];
		}

		//Scale lambda to fit range.
		double lambdaPercent = (lambda - domainBounds.first ) / domainRange;
		double approx = FIT_X_LOW + fitRange * lambdaPercent;
		PoissonFunction<Double> poissonFunction(/*lambda*/approx, height);
		fitter.setFunction(poissonFunction);

		Vector<Double> sigma(xCount,1);

		try {
			Matrix<Double> solution = fitter.fit(components, fitYValues, sigma);
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
					if ( isNaN( solutionLambda) ){
						errorMsg = "Unsuccessful Fit: Estimated lambda was a NaN.";
						fitSuccessful = false;
					}
					else if ( solutionLambda < FIT_X_LOW || solutionLambda > FIT_X_HIGH ){
						errorMsg = "Unsuccessful Fit: Estimated lambda was outside of data range";
						fitSuccessful = false;
					}
					else {
						PoissonFunction<Double> poissonFit(solutionLambda, solutionHeight );
						int xCount = actualXValues.size();
						for( int i = 0; i < xCount; i++ ){
							fitValues[i] = poissonFit.eval(&fitXValues[i]);
						}
						//Now map lambda back to actualXValues.
						double solutionPercent = (solutionLambda - FIT_X_LOW) / fitRange;
						actualLambda = domainBounds.first + solutionPercent * domainRange;
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
		stream << "#Lambda: "<<actualLambda<< END_LINE;
		stream << "#Height: "<<solutionHeight<< END_LINE;
		Fitter::toAscii(stream);
	}
}

FitterPoisson::~FitterPoisson() {
}

} /* namespace casa */
