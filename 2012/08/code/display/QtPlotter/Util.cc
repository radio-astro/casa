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
#include <display/QtPlotter/Util.h>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <QMessageBox>
#include <QWidget>

namespace casa {

	const QString Util::ORGANIZATION = "NRAO/CASA";
	const QString Util::APPLICATION = "Spectral Profiler";

	Util::Util() {
	// TODO Auto-generated constructor stub
	}

	Util::~Util() {
	// TODO Auto-generated destructor stub
	}

	const double Util::PI = std::atan(1.0) * 4.0;
	const double Util::TIME_CONV = 60.0;
	const double Util::RAD_DEGREE_CONVERSION = 572.95779513082;

	double Util::degMinSecToRadians( int degrees, int mins, float secs ){
		assert( -90 <= degrees && degrees <=90 );
		assert( 0 <= mins && mins < TIME_CONV );
		assert( 0 <= secs && secs < TIME_CONV );
        double decimalDegrees = toDecimalDegrees( degrees, mins, secs );
		return toRadians( decimalDegrees );
	}

	double Util::hrMinSecToRadians( int hours, int mins, float secs ){
		assert( 0 <= hours && hours <= 24 );
		assert( 0 <= mins && mins < TIME_CONV );
		assert( 0 <= secs && secs < TIME_CONV );

		double decimalHours = toDecimalDegrees( hours, mins, secs );
		double degrees = decimalHours * 15;
		return toRadians( degrees );
	}

	QString Util::toHTML( const QString& baseStr ){
		QString htmlStr( "<font color='black'>");
		htmlStr.append( baseStr );
		htmlStr.append( "</font>");
		return htmlStr;
	}

	double Util::toRadians( double degrees ){
		double rads = degrees *  PI / 180;
		return rads;
	}

	double Util::toDecimalDegrees( int degrees, int mins, float seconds ){
		double deg = degrees + mins / TIME_CONV + seconds / (TIME_CONV * TIME_CONV );
		return deg;
	}

	void Util::showUserMessage( QString& msg, QWidget* parent ){
		QMessageBox msgBox( parent );
		msgBox.setText( msg );
		msgBox.exec();
	}

	void Util::minMax( double& min, double& max, const Vector<Double>& values ){
		if ( values.size() > 0 ){
			min = values[0];
			max = values[0];
			for ( int i = 1; i < static_cast<int>(values.size()); i++ ){
				if ( values[i] < min ){
					min = values[i];
				}
				else if ( values[i] > max ){
					max = values[i];
				}
			}
		}
	}

	int Util::getCenter( const Vector<Double>& values, Double& mean ){
		if ( values.size() == 1 ){
			mean =  values[0];
		}
		else if ( values.size() == 2 ){
			mean =  0.5*(values[0]+values[1]);
		}
		else {
			Double minval;
			Double maxval;
			minMax(minval, maxval, values);
			mean =  0.5*(minval + maxval);
		}
		int pos = static_cast<int>(floor(mean+0.5));
		return pos;
	}

	void Util::getRa(double radians, int& raHour, int& raMin, double& raSec) {
	   double ras = radians * 24 * RAD_DEGREE_CONVERSION;
	   if (ras > 86400) ras = 0;
	   double rah = ras/3600;
	   raHour = (int)floor(rah);
	   double ram = (rah - raHour) * 60;
	   raMin = (int)floor(ram);
	   double raSecond = (ram - raMin) * 60;
	   raSec = floor(1000 * raSecond) / 1000.;
	}

	void Util::getDec(double radians, int& decDeg, int& decMin, double& decSec) {

	   int sign = (radians > 0) ? 1 : -1;
	   double decs = sign * radians * 360 * RAD_DEGREE_CONVERSION;

	   if (decs > 1296000) decs = 0;

	   double decd = decs / 3600;
	   decDeg = (int)floor(decd);
	   double decm = (decd - decDeg) * 60;
	   decMin = (int)floor(decm);
	   double decSeconds = (decm - decMin) * 60;
	   decSec = floor(1000 * decSeconds) / 1000.;

	}


}
